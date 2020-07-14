//
// Created by nbaiot@126.com on 2020/6/29.
//

#include "video_encoder.h"


#include <thread>
#include <cstring>

#include <libyuv.h>
#include <glog/logging.h>
#include <condition_variable>
#include <boost/lockfree/queue.hpp>

#include "ffmpeg.h"
#include "av_deleter.h"
#include "av_codec_global_mutex.h"
#include "av_video_frame.h"
#include "av_i420_frame_buffer.h"

#include "av_h264_sei_extend.h"

namespace nbaiot {

class VideoEncoderImpl {

public:
  struct EncodeTask {
    std::shared_ptr<AVVideoFrameInterface> frame;
  };

  VideoEncoderImpl() = default;

  ~VideoEncoderImpl() {
    Close();
    if (codec_context_) {
      avcodec_free_context(&codec_context_);
    }
    codec_ = nullptr;
  }

  bool Open(const VideoEncoderConfig& config) {

    if (Opened()) {
      if (config_ != config) {
        Close();
      } else {
        return true;
      }
    }

    config_ = config;

    codec_ = avcodec_find_encoder_by_name(config.codecName.c_str());
    if (codec_) {
      codec_context_ = avcodec_alloc_context3(codec_);
    } else {
      codec_context_ = nullptr;
    }

    if (!codec_ || !codec_context_) {
      LOG(ERROR) << ">>>>>>>>>encoder not support";
      return false;
    }

    codec_context_->width = config.width;
    codec_context_->height = config.height;
    codec_context_->pix_fmt = AV_PIX_FMT_YUV420P;
    if (codec_context_->pix_fmt == AV_PIX_FMT_NONE) {
      LOG(ERROR) << ">>>>>>>>>>encoder pix fmt invalid";
      return false;
    }
    codec_context_->gop_size = config.gopSize;
    codec_context_->time_base = AVRational{1, config.maxFPS};
    if (config.bitRate > 0) {
      codec_context_->bit_rate = config.bitRate;
    }
    if (config.maxBFrames > 0) {
      codec_context_->max_b_frames = config.maxBFrames;
    }
    if (config.globalQuality > 0) {
      codec_context_->global_quality = config.globalQuality;
    }
    if (config_.threadCount > 1) {
      codec_context_->thread_count = config_.threadCount;
    }

    if (config.globalHeader) {
      codec_context_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (config_.codecName == "libx264") {
      SetOption("preset", "faster");
      SetOption("tune", "zerolatency");
      SetOption("profile", "high");
    } else if (config_.codecName == "libx265") {
      SetOption("preset", "ultrafast");
      SetOption("tune", "zero-latency");
      SetOption("profile", "main10");
      SetOption("level", "10");
    }

    int ret = 0;
    {
      std::unique_lock<std::mutex> lk(CodecMutex::global_codec_open_mutex);
      ret = avcodec_open2(codec_context_, codec_, nullptr);
    }


    if (ret != 0) {
      LOG(ERROR) << ">>>>>>>>>VideoEncoderImpl open codec failed" << av_err2str(ret);
      return false;
    }

    encoder_thread_ = std::make_shared<std::thread>([this]() {
      DecodingRun();
    });

    return true;
  }

  void Close() {
    if (!Opened())
      return;

    stop_ = true;

    if (encoder_thread_ && encoder_thread_->joinable()) {
      encoder_thread_->join();
    }

    if (codec_context_) {
      avcodec_free_context(&codec_context_);
      codec_context_ = avcodec_alloc_context3(codec_);
    }

    EncodeTask* task;
    while (queue_frame_.pop(task)) {
      delete task;
    }

    stop_ = false;
  }

  bool Opened() {
    if (!codec_context_)
      return false;
    return avcodec_is_open(codec_context_);
  }

  VideoEncoderConfig Config() {
    return config_;
  }

  std::string CodecName() {
    if (codec_)
      return avcodec_get_name(codec_->id);
    return "";
  }

  bool SetOption(const std::string& key, const std::string& val) {
    if (Opened()) {
      LOG(ERROR) << ">>>>>>>>>>codec option must be set before open.";
      return false;
    }
    if (codec_context_) {
      auto ret = av_opt_set(codec_context_->priv_data, key.c_str(), val.c_str(), 0);
      if (ret != 0) {
        LOG(ERROR) << ">>>>>>>>>>> encoder set option failed:" << av_err2str(ret);
        return false;
      }
    }
    return true;
  }

  void Encode(const std::shared_ptr<AVVideoFrameInterface>& frame) {
    if (!Opened() || stop_)
      return;

    if (encode_task_count_ > 12) {
      LOG_EVERY_N(WARNING, 10) << ">>>>>>>>>>>>>>>>>>>>>>>>encode thread too slow";
    }

    auto task = new EncodeTask();
    task->frame = frame;

    if (queue_frame_.push(task)) {
      ++encode_task_count_;
    } else {
      delete task;
    }

    cv_.notify_all();

  }

  void SetEncodeCallback(OnVideoEncodeCallback callback) {
    callback_ = std::move(callback);
  }

  void Flush() {
   /* std::packaged_task<int()> task([this]() {
        InnerFlush();
        return 0;
    });

    boost::asio::post(ioc_, std::ref(task));
    auto feature = task.get_future();
    feature.get();*/
  }

  void* RawAVPtr() {
    return codec_context_;
  }

private:
  void DecodingRun() {
    while (ProcessEncoding()) {}
  }

  bool ProcessEncoding() {
    if (stop_) {
      InnerFlush();
      return false;
    }

    EncodeTask* task = nullptr;
    while (queue_frame_.pop(task)) {
      --encode_task_count_;
      EncodeInner(task->frame);
      delete task;
    }

    std::unique_lock<std::mutex> lk(cv_mutex_);
    cv_.wait_for(lk, std::chrono::milliseconds(40), [&]() { return !queue_frame_.empty() && !stop_; });

    return true;
  }

  void EncodeInner(const std::shared_ptr<AVVideoFrameInterface>& frame) {
    auto i420 = frame->FrameBuffer()->ToI420();
    auto avFrame = (AVFrame*) i420->RawAVPtr();
    avFrame->pict_type = AV_PICTURE_TYPE_NONE;
    auto ret = avcodec_send_frame(codec_context_, avFrame);
    if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
      return;
    } else if (ret < 0) {
      LOG(ERROR) << ">>>>>>>>>>>>>>>>>>>>>>>>>>Encode failed:" << av_err2str(ret);
      return;
    }
    auto packet = std::shared_ptr<AVPacket>(av_packet_alloc(), AVDeleter::AVPacketDeleter());
    ret = avcodec_receive_packet(codec_context_, packet.get());
    if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
      return;
    } else if (ret < 0) {
      LOG(ERROR) << ">>>>>>>>>>>>>>>>>>>>>>>>>>Encode failed:" << av_err2str(ret);
      return;
    }
    if (CodecName() == "libx264" || CodecName() == "h264") {
      packet = AddSEIInfo(packet, frame->SEIInfos());
    }
    if (callback_) {
      callback_(packet);
    }
  }


  void InnerFlush() {
    auto ret = avcodec_send_frame(codec_context_, nullptr);
    if (ret == AVERROR(EAGAIN))
      return;
    do {
      auto packet = std::shared_ptr<AVPacket>(av_packet_alloc(), AVDeleter::AVPacketDeleter());
      ret = avcodec_receive_packet(codec_context_, packet.get());
      if (ret == 0) {
        if (callback_) {
          callback_(packet);
        }
      }
    } while (ret != AVERROR_EOF);
  }

  std::shared_ptr<AVPacket> AddSEIInfo(const std::shared_ptr<AVPacket>& pkt,
                                       const std::vector<SEIExtendInfo>& seiInfos) {

    auto annexb_type = H264SEI::GetAnnexbType(pkt->data, pkt->size);
    LOG_EVERY_N(INFO, 125) << ">>>>>>>>>>>>>>>>>>> AddSEIInfo annexb_type:" << annexb_type;

    int totalSEIPktSize = 0;
    std::vector<int32_t> seiSize;
    seiSize.reserve(seiInfos.size());
    for (const auto& sei : seiInfos) {
      auto size = H264SEI::GetSEIPacketSize((const uint8_t*) sei.sei.c_str(), sei.sei.length(), annexb_type);;
      totalSEIPktSize += size;
      seiSize.push_back(size);
    }

    auto seiPkt = std::shared_ptr<AVPacket>(av_packet_alloc(), AVDeleter::AVPacketDeleter());
    av_init_packet(seiPkt.get());
    av_new_packet(seiPkt.get(), pkt->size + totalSEIPktSize);
    std::memset(seiPkt->data, 0, pkt->size + totalSEIPktSize);
    av_packet_copy_props(seiPkt.get(), pkt.get());

    uint8_t* data = pkt->data;
    int size = pkt->size;

    bool isAnnexb = annexb_type != H264SEI::kAnnexbType_0;
    /// 追加到尾部
    std::memcpy(seiPkt->data, pkt->data, pkt->size);
    auto seiStart = (uint8_t*) seiPkt->data + pkt->size;
    for (int i = 0; i < seiInfos.size(); ++i) {
      uint8_t* sei = nullptr;
      if (i == 0) {
        sei = seiStart;
      } else {
        sei = seiStart + seiSize[i - 1];
      }
      if (seiInfos[i].uuid.size() != 16) {
        LOG(ERROR) << ">>>>>>>>>>>>>>>>>>>>>>sei uuid size must be 16 bytes";
        continue;
      } else {
        H264SEI::FillSEIPacket(sei, annexb_type, seiInfos[i].uuid.data(),
                               (const uint8_t*) seiInfos[i].sei.c_str(), seiInfos[i].sei.length());
      }
    }

    return seiPkt;
  }

private:
  VideoEncoderConfig config_;
  AVCodec* codec_{nullptr};
  AVCodecContext* codec_context_{nullptr};
  std::shared_ptr<std::thread> encoder_thread_;
  std::atomic<bool> stop_{false};
  std::mutex cv_mutex_;
  std::condition_variable cv_;
  std::atomic<int32_t> encode_task_count_{0};
  boost::lockfree::queue<EncodeTask*> queue_frame_{25};
  OnVideoEncodeCallback callback_;
};

VideoEncoder::VideoEncoder() {
  impl_ = std::make_unique<VideoEncoderImpl>();
}

bool VideoEncoder::Open(const VideoEncoderConfig& config) {
  return impl_->Open(config);
}

void VideoEncoder::Close() {
  impl_->Close();
}

bool VideoEncoder::Opened() const {
  return impl_->Opened();
}

VideoEncoderConfig VideoEncoder::Config() {
  return impl_->Config();
}

bool VideoEncoder::SetOption(const std::string& key, const std::string& val) {
  return impl_->SetOption(key, val);
}

std::string VideoEncoder::CodecName() {
  return impl_->CodecName();
}

void VideoEncoder::Encode(const std::shared_ptr<AVVideoFrameInterface>& frame) {
  impl_->Encode(frame);
}

void VideoEncoder::Flush() {
  impl_->Flush();
}

void VideoEncoder::SetEncodeCallback(OnVideoEncodeCallback callback) {
  impl_->SetEncodeCallback(std::move(callback));
}

void* VideoEncoder::RawAVPtr() {
  return impl_->RawAVPtr();
}

void* VideoEncoder::RawAVPtr() const {
  return impl_->RawAVPtr();
}

VideoEncoder::~VideoEncoder() = default;
}
