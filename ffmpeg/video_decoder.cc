//
// Created by nbaiot@126.com on 2020/7/1.
//

#include "video_decoder.h"

#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <boost/lockfree/queue.hpp>
#include <glog/logging.h>

#include <utils/elapsed_time.h>
#include <utils/fps.h>

#include "ffmpeg.h"
#include "av_deleter.h"
#include "av_codec_global_mutex.h"
#include "av_utils.h"
#include "av_h264_sei_extend.h"
#include "av_video_frame_buffer_pool.h"
#include "av_decoding_video_frame.h"
#include "av_video_frame_buffer_factory.h"

namespace nbaiot {

class VideoDecoderImpl {

public:
  struct DecodeTask {
    std::shared_ptr<AVPacket> pkt;
  };

  VideoDecoderImpl() = default;

  ~VideoDecoderImpl() {
    Close();
  }

  bool Open(AVCodec *codec, AVCodecParameters *params, int decodeThreadCount) {
    if (!codec || !params)
      return false;

    if (Opened()) {
      LOG(ERROR) << ">>>>>>>>>>>>>>>>>VideoDecoder is already opened";
      return false;
    }

    LOG(INFO) << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>codec name:"
              << avcodec_get_name(codec->id);
    LOG(INFO) << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>frame format:"
              << av_get_pix_fmt_name(static_cast<AVPixelFormat>(params->format));
    LOG(INFO) << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>frame size:"
              << params->width << "*" << params->height;

    auto rawPtr = avcodec_alloc_context3(codec);
    if (!rawPtr) {
      LOG(ERROR) << "VideoDecoder alloc codex context failed";
      return false;
    }

    avcodec_parameters_to_context(rawPtr, params);
    if (decodeThreadCount > 1) {
      rawPtr->thread_count = decodeThreadCount;
    }

    codec_context_ = std::shared_ptr<AVCodecContext>(rawPtr, AVDeleter::AVCodecContextDeleter());

    int ret = 0;
    {
      std::unique_lock<std::mutex> lk(CodecMutex::global_codec_open_mutex);
      ret = avcodec_open2(codec_context_.get(), codec, nullptr);
    }

    if (ret != 0) {
      codec_context_.reset();
      LOG(ERROR) << "VideoDecoder cannot open decoder:" << codec->name;
      return false;
    }

    decode_thread_ = std::make_unique<std::thread>([this]() {
        DecodingRun();
    });
    return true;
  }

  void Close() {
    if (!Opened())
      return;

    stop_ = true;
    cv_.notify_all();

    if (decode_thread_ && decode_thread_->joinable()) {
      decode_thread_->join();
    }

    codec_context_.reset();

    DecodeTask* task;
    while (queue_pkg_.pop(task)) {
      delete task;
    }

    stop_ = false;
  }

  bool Opened() const {
    if (codec_context_)
      return avcodec_is_open(codec_context_.get());
    return false;
  }

  bool SetOption(const std::string& key, const std::string& val) {
    if (Opened()) {
      LOG(ERROR) << ">>>>>>>>>>codec option must be set before open.";
      return false;
    }
    return true;
  }

  std::string CodecName() {
    if (codec_context_)
      return avcodec_get_name(codec_context_->codec_id);
    return "";
  }

  void *RawAVPtr() {
    return codec_context_.get();
  }

  void *RawAVPtr() const {
    return codec_context_.get();
  }

  void SetDecodingFrameType(VideoFrameType decodingFrameType) {
    decoding_frame_type_ = decodingFrameType;
  }

  void SetDecodeCallback(OnVideoDecodeCallback callback) {
    decode_callback_ = std::move(callback);
  }

  void SetDecodingFrameCallback(OnVideoDecodingFrameCallback callback) {
    decoding_frame_callback_ = std::move(callback);
  }

  void Decode(const std::shared_ptr<AVPacket>& pkt) {
    if (!Opened())
      return;

    if (decode_task_count_ > 12) {
      LOG_EVERY_N(WARNING, 10) << ">>>>>>>>>>>>>>>>>>>>>>>>decode thread too slow";
    }

    auto task = new DecodeTask;
    task->pkt = pkt;

    if (queue_pkg_.push(task)) {
      ++decode_task_count_;
    } else {
      delete task;
    }

    cv_.notify_all();
  }

private:

  void DecodingRun() {
    decode_fps_counter_.Start();
    while (ProcessDecoding()) {}
  }

  bool ProcessDecoding() {
    if (stop_)
      return false;

    DecodeTask* task = nullptr;
    while (queue_pkg_.pop(task)) {
      --decode_task_count_;
      DecodePacketInner(task->pkt);
      delete task;
      if (stop_)
        return false;
    }

    std::unique_lock<std::mutex> lk(cv_mutex_);
    cv_.wait_for(lk, std::chrono::milliseconds(40), [&]() { return !queue_pkg_.empty() && !stop_; });

    return true;
  }

  void DecodePacketInner(const std::shared_ptr<AVPacket>& pkt) {
    if (!Opened())
      return;
    ElapsedTime time;
    auto ret = avcodec_send_packet(codec_context_.get(), pkt.get());
    if (ret != 0) {
      LOG(ERROR) << "VideoDecoder decode video failed:" << av_err2str(ret);
      return;
    }

    for (;;) {
      auto frame = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* ptr){
        if (ptr) {
          av_frame_free(&ptr);
        }
      });

      ret = avcodec_receive_frame(codec_context_.get(), frame.get());
      if (ret != 0) {
        if (ret != AVERROR(EAGAIN)) {
          LOG(ERROR) << ">>>>>>>>>> VideoDecoder decode frame failed:" << av_err2str(ret);
        }
        break;
      }

      auto t1 = time.ElapsedWallMS();
      if (decode_callback_) {
        decode_callback_(frame);
      }

      if (decoding_frame_callback_) {
        /// 提取 sei
        std::vector<SEIExtendInfo> seiExtendInfos;
        ExtractSEIContent(pkt, seiExtendInfos);
        auto frameBuffer = video_frame_buffer_factory_.Create(frame->width, frame->height, decoding_frame_type_);
        auto decodingFrame = AVDecodingVideoFrame::Create(frameBuffer, AVUtils::TimestampMS(), frame);
        for (const auto& it : seiExtendInfos) {
          decodingFrame->AddSEIInfo(it);
        }
        decoding_frame_callback_(decodingFrame);
      }
      auto t2 = time.ElapsedWallMS();
      auto fps = decode_fps_counter_.Fps();
      LOG_EVERY_N(INFO, 250) << ">>>>>>>>>>>>>>>>>decode video fps:" << fps
                             << ", decode time:" << t1 << ", callback time:" << t2 - t1;
    }

  }

  void ExtractSEIContent(const std::shared_ptr<AVPacket>& pkt, std::vector<SEIExtendInfo>& seiExtendInfos) {
    if (CodecName() != "libx264" && CodecName() != "h264") {
      return;
    }
    auto isAnnexb = H264SEI::CheckIsAnnexb(pkt->data, pkt->size);
    std::vector<std::vector<uint8_t>> uuidList;
    std::vector<std::string> seiPayloads;
    if (isAnnexb) {
      H264SEI::GetAnnexbAllSEIContent(pkt->data, pkt->size, uuidList, seiPayloads);
    } else {
      H264SEI::GetAVCCAllSEIContent(pkt->data, pkt->size, uuidList, seiPayloads);
    }
    seiExtendInfos.reserve(uuidList.size());
    for (int i = 0; i < uuidList.size(); ++i) {
      SEIExtendInfo seiExtendInfo;
      seiExtendInfo.uuid = uuidList[i];
      seiExtendInfo.sei = seiPayloads[i];
      seiExtendInfos.push_back(seiExtendInfo);
    }
  }

private:
  FPS decode_fps_counter_;
  std::shared_ptr<AVCodecContext> codec_context_{nullptr};
  std::unique_ptr<std::thread> decode_thread_;
  std::atomic<bool> stop_{false};
  OnVideoDecodeCallback decode_callback_;
  OnVideoDecodingFrameCallback decoding_frame_callback_;
  std::mutex cv_mutex_;
  std::condition_variable cv_;
  std::atomic<int32_t> decode_task_count_{0};
  boost::lockfree::queue<DecodeTask*> queue_pkg_{25};
  VideoFrameType decoding_frame_type_{kYUV_I420};
  AVVideoFrameBufferFactory video_frame_buffer_factory_{5};
};

VideoDecoder::VideoDecoder() {
  impl_ = std::make_unique<VideoDecoderImpl>();
}

VideoDecoder::~VideoDecoder() {
  impl_->Close();
}

bool VideoDecoder::Open(AVCodec *codec, AVCodecParameters *params, int decodeThreadCount) {
  return impl_->Open(codec, params, decodeThreadCount);
}

void VideoDecoder::Close() {
  impl_->Close();
}

bool VideoDecoder::Opened() const {
  return impl_->Opened();
}

bool VideoDecoder::SetOption(const std::string& key, const std::string& val) {
  return impl_->SetOption(key, val);
}

std::string VideoDecoder::CodecName() {
  return impl_->CodecName();
}

void VideoDecoder::SetDecodeCallback(OnVideoDecodeCallback callback) {
  impl_->SetDecodeCallback(std::move(callback));
}

void VideoDecoder::SetDecodingFrameCallback(OnVideoDecodingFrameCallback callback) {
  impl_->SetDecodingFrameCallback(std::move(callback));
}

void *VideoDecoder::RawAVPtr() {
  return impl_->RawAVPtr();
}

void *VideoDecoder::RawAVPtr() const {
  return impl_->RawAVPtr();
}

void VideoDecoder::SetDecodingFrameType(VideoFrameType decodingFrameType) {
  impl_->SetDecodingFrameType(decodingFrameType);
}

void VideoDecoder::Decode(const std::shared_ptr<AVPacket>& pkt) {
  impl_->Decode(pkt);
}


}