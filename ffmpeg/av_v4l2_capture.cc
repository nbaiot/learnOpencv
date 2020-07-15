//
// Created by nbaiot@126.com on 2020/7/14.
//

#include "av_v4l2_capture.h"

#include "ffmpeg.h"
#include "av_deleter.h"
#include "av_decoding_video_frame.h"
#include "input_video_stream.h"
#include "av_video_frame_buffer_pool.h"
#include <atomic>
#include <thread>
#include <memory>
#include <libyuv.h>
#include <glog/logging.h>

namespace nbaiot {


class AVV4l2CaptureImpl {

public:
  AVV4l2CaptureImpl() = default;

  ~AVV4l2CaptureImpl() = default;

  bool Open(const std::string& device, int w, int h) {
    if (Opened()) {
      LOG(WARNING) << ">>>>>>>>>> AVV4l2Capture already opened:" << device_;
      return false;
    }

    av_format_context_ =
        std::unique_ptr<AVFormatContext, AVDeleter::AVFormatContextDeleter>(avformat_alloc_context(),
                                                                            AVDeleter::AVFormatContextDeleter());

    auto v4l2Format = av_find_input_format("v4l2");

    if (!v4l2Format) {
      LOG(ERROR) << ">>>>>>>>>>AVV4l2Capture opened failed: current system not support v4l2";
      return false;
    }

    device_ = device;

    std::string codeName = "mjpeg";
    std::string videoSize = std::to_string(w) + "x" + std::to_string(h);
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "input_format", codeName.c_str(), 0);
    av_dict_set(&opts, "video_size", videoSize.c_str(), 0);

    auto rawInputContextPtr = av_format_context_.get();
    auto ret = avformat_open_input(&rawInputContextPtr, device_.c_str(), v4l2Format, &opts);

    if (ret != 0) {
      av_format_context_.reset();
      device_.clear();
      LOG(ERROR) << ">>>>>>>>>> AVV4l2Capture opened failed:" << av_err2str(ret);
      return false;
    }

    if (av_format_context_->nb_streams <= 0) {
      av_format_context_.reset();
      LOG(ERROR) << ">>>>>>>>>>  AVV4l2Capture opened failed:" + device_ + " no find stream";
      device_.clear();
      return false;
    }

    capture_thread_ = std::make_unique<std::thread>([this]() {
        Run();
    });

    auto stream = av_format_context_->streams[0];
    auto codecpar = stream->codecpar;

    width_ = codecpar->width;
    height_ = codecpar->height;
    if (stream->avg_frame_rate.den != 0) {
      fps_ = stream->avg_frame_rate.num / stream->avg_frame_rate.den;
    }
    opened_ = true;
    return true;
  }

  void Close() {
    if (!Opened())
      return;

    stop_ = true;

    if (capture_thread_ && capture_thread_->joinable()) {
      capture_thread_->join();
    }

    av_format_context_.reset();

    width_ = -1;
    height_ = -1;
    fps_ = -1;
    device_.clear();

    stop_ = false;
  }

  bool Opened() {
    return opened_;
  }

  std::string Device() {
    return device_;
  }

  int Width() {
    return width_;
  }

  int Height() {
    return height_;
  }

  int Fps() {
    return fps_;
  }

  void SetFrameCallback(OnV4l2FrameCallback callback) {
    frame_callback_ = std::move(callback);
  }

  void SetDisconnectedErrorCallback(OnV4l2DisconnectErrorCallback callback) {
    disconnect_callback_ = std::move(callback);
  }

private:
  void Run() {
    while (!stop_) {
      int ret = 0;
      AVPacket* avPacket = av_packet_alloc();
      do {
        ret = av_read_frame(av_format_context_.get(), avPacket);
        if (ret != 0) {
          av_packet_free(&avPacket);
          avPacket = av_packet_alloc();
        }
      } while (ret == AVERROR(EAGAIN));

      if (ret == AVERROR_EOF) {
        ProcessEOF();
        return;
      }

      ProcessPacket(avPacket);

      av_packet_free(&avPacket);
    }
  }

  void ProcessPacket(AVPacket* packet) {
    if (packet->size <= 0)
      return;
    auto i420Buffer = i420_buffer_pool_.CreateBuffer(width_, height_)->ToI420();
    auto ret = libyuv::ConvertToI420(packet->data, packet->size,
                                     const_cast<uint8_t*>(i420Buffer->DataY()), i420Buffer->StrideY(),
                                     const_cast<uint8_t*>(i420Buffer->DataU()), i420Buffer->StrideU(),
                                     const_cast<uint8_t*>(i420Buffer->DataV()), i420Buffer->StrideV(),
                                     0, 0,
                                     width_, height_,
                                     width_, height_,
                                     libyuv::kRotate0, libyuv::FOURCC_MJPG);
    auto videoFrame = std::make_shared<AVDecodingVideoFrame>(i420Buffer, packet->pts, packet->pts, true);

    if (frame_callback_) {
      frame_callback_(videoFrame);
    }

  }

  void ProcessEOF() {
    if (disconnect_callback_) {
      disconnect_callback_(device_);
    }
  }

private:
  std::string device_;
  int width_{-1};
  int height_{-1};
  int fps_{-1};
  bool opened_{false};
  std::atomic<bool> stop_{false};
  VideoFrameType frame_type_{kYUV_I420};
  OnV4l2FrameCallback frame_callback_;
  OnV4l2DisconnectErrorCallback disconnect_callback_;
  AVVideoFrameBufferPool i420_buffer_pool_{kYUV_I420, 5};
  std::unique_ptr<std::thread> capture_thread_;
  std::unique_ptr<AVFormatContext, AVDeleter::AVFormatContextDeleter> av_format_context_;
};

AVV4l2Capture::AVV4l2Capture() {
  impl_ = std::make_unique<AVV4l2CaptureImpl>();
}

AVV4l2Capture::~AVV4l2Capture() {
  impl_->Close();
}

bool AVV4l2Capture::Open(const std::string& device, int w, int h) {
  return impl_->Open(device, w, h);
}

void AVV4l2Capture::Close() {
  impl_->Close();
}

bool AVV4l2Capture::Opened() {
  return impl_->Opened();
}

std::string AVV4l2Capture::Device() {
  return impl_->Device();
}

int AVV4l2Capture::Width() {
  return impl_->Width();
}

int AVV4l2Capture::Height() {
  return impl_->Height();
}

int AVV4l2Capture::Fps() {
  return impl_->Fps();
}

void AVV4l2Capture::SetFrameCallback(OnV4l2FrameCallback callback) {
  impl_->SetFrameCallback(std::move(callback));
}

void AVV4l2Capture::SetDisconnectedErrorCallback(OnV4l2DisconnectErrorCallback callback) {
  impl_->SetDisconnectedErrorCallback(std::move(callback));
}

}