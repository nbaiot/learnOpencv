//
// Created by nbaiot@126.com on 2020/7/2.
//

#include "input_video_stream.h"

#include <thread>
#include <glog/logging.h>

#include "ffmpeg.h"
#include "video_decoder.h"
#include "av_decoding_video_frame.h"

namespace nbaiot {

class InputVideoStreamImpl : public AVInputStreamInterface {

public:
  InputVideoStreamImpl(AVStream *stream,
                       AVDemuxerInterface *muxer,
                       const std::shared_ptr<char>& monitor)
      : stream_(stream), muxer_(muxer), parent_monitor_(monitor) {

    video_decoder_ = std::make_unique<VideoDecoder>();
    auto codec = avcodec_find_decoder(stream_->codecpar->codec_id);
    if (!codec) {
      LOG(ERROR) << "InputVideoStream not find decoder:" << avcodec_get_name(stream_->codecpar->codec_id);
      return;
    }

    auto decodeThreadCount = std::thread::hardware_concurrency();
    if (stream->codecpar->width <= 1280) {
      /// 720p
      decodeThreadCount = 1;
    } if (stream->codecpar->width <= 1920) {
      /// 1080p
      decodeThreadCount = decodeThreadCount >= 2 ? 2 : decodeThreadCount;
    } if (stream->codecpar->width <= 4096) {
      /// 4k
      decodeThreadCount = decodeThreadCount >= 4 ? 4 : decodeThreadCount;
    } else {
      /// > 4k
      decodeThreadCount = decodeThreadCount >= 8 ? 8 : decodeThreadCount;
    }

    if (!video_decoder_->Open(codec, stream->codecpar, decodeThreadCount)) {
      LOG(ERROR) << "InputVideoStream error: open decoder failed";
    }
    video_decoder_->SetDecodeCallback([this](const std::shared_ptr<AVFrame>& frame) {
        if (avframe_callback_) {
          avframe_callback_(frame);
        }
    });

    video_decoder_->SetDecodingFrameCallback([this](const std::shared_ptr<AVDecodingVideoFrame>& frame){
      if (video_frame_callback_) {
        video_frame_callback_(frame);
      }
    });
  }

  ~InputVideoStreamImpl() override {
    video_decoder_->Close();
  }

  StreamType Type() override {
    return kVideoStream;
  }

  int Index() override {
    if (stream_) {
      return stream_->index;
    }
    return -1;
  }

  bool Isvalid() override {
    return !parent_monitor_.expired() && stream_;
  }

  void *RawAVPtr() override {
    return stream_;
  }

  void SetFrameType(VideoFrameType frameType) override {
    frame_type_ = frameType;
    if (video_decoder_) {
      video_decoder_->SetDecodingFrameType(frame_type_);
    }
  }

  VideoFrameType FrameType() override {
    return frame_type_;
  }

  void SetAVFrameCallback(OnAVInputStreamAVFrameCallback callback) override {
    avframe_callback_ = std::move(callback);
  }

  void SetAVVideoFrameCallback(OnAVVideoFrameCallback callback) override {
    video_frame_callback_ = std::move(callback);
  }

  void FeedAVPacket(const std::shared_ptr<AVPacket>& packet) override {
    if (video_decoder_) {
      video_decoder_->Decode(packet);
    }
  }

  int Fps() {
    if (Isvalid())
      return stream_->avg_frame_rate.num / stream_->avg_frame_rate.den;
    return -1;
  }

  int64_t Bitrate() {
    if (Isvalid())
      return stream_->codecpar->bit_rate;
    return 0;
  }

  int Width() {
    if (Isvalid())
      return stream_->codecpar->width;
    return -1;
  }

  int Height() {
    if (Isvalid())
      return stream_->codecpar->height;
    return -1;
  }

  int PixelFormat() {
    if (Isvalid())
      return stream_->codecpar->format;
    return AV_PIX_FMT_NONE;
  }


private:
  std::weak_ptr<char> parent_monitor_;
  AVStream *stream_;
  AVDemuxerInterface *muxer_;
  VideoFrameType frame_type_{kYUV_I420};
  std::unique_ptr<VideoDecoder> video_decoder_;
  OnAVInputStreamAVFrameCallback avframe_callback_;
  OnAVVideoFrameCallback video_frame_callback_;

};

InputVideoStream::InputVideoStream(AVStream *stream,
                                   AVDemuxerInterface *muxer,
                                   const std::shared_ptr<char>& monitor) {
  impl_ = std::make_unique<InputVideoStreamImpl>(stream, muxer, monitor);
}

InputVideoStream::~InputVideoStream() = default;

StreamType InputVideoStream::Type() {
  return impl_->Type();
}

int InputVideoStream::Index() {
  return impl_->Index();
}

bool InputVideoStream::Isvalid() {
  return impl_->Isvalid();
}

void *InputVideoStream::RawAVPtr() {
  return impl_->RawAVPtr();
}

void InputVideoStream::SetFrameType(VideoFrameType frameType) {
  impl_->SetFrameType(frameType);
}

VideoFrameType InputVideoStream::FrameType() {
  return impl_->FrameType();
}

void InputVideoStream::SetAVFrameCallback(OnAVInputStreamAVFrameCallback callback) {
  impl_->SetAVFrameCallback(std::move(callback));
}

void InputVideoStream::SetAVVideoFrameCallback(OnAVVideoFrameCallback callback) {
  impl_->SetAVVideoFrameCallback(std::move(callback));
}

void InputVideoStream::FeedAVPacket(const std::shared_ptr<AVPacket>& packet) {
  impl_->FeedAVPacket(packet);
}

int InputVideoStream::Fps() {
  return impl_->Fps();
}

int64_t InputVideoStream::Bitrate() {
  return impl_->Bitrate();
}

int InputVideoStream::Width() {
  return impl_->Width();
}

int InputVideoStream::Height() {
  return impl_->Height();
}

int InputVideoStream::PixelFormat() {
  return impl_->PixelFormat();
}

}