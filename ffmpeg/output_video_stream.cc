//
// Created by nbaiot@126.com on 2020/7/2.
//

#include <memory>
#include <atomic>
#include "ffmpeg.h"
#include "av_muxer.h"
#include "output_video_stream.h"

namespace nbaiot {

class OutputVideoStreamImpl : public AVOutputStreamInterface {

public:
  OutputVideoStreamImpl(AVStream *stream, int fps,
                        AVMuxerInterface *muxer,
                        const std::shared_ptr<char>& monitor)
                        :stream_(stream), fps_(fps),
                          muxer_(muxer), parent_monitor_(monitor){}

  ~OutputVideoStreamImpl() override = default;

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

  void FeedPacket(std::shared_ptr<AVPacket> packet) override {
    if (Isvalid() && packet->size > 0 && muxer_ && muxer_->CanWritePacket()) {
      ++pkg_count_;
      packet->stream_index = stream_->index;
      packet->pts = packet->dts = pkg_count_ * (stream_->time_base.den) / stream_->time_base.num / fps_;
      muxer_->WritePacket(packet);
    }
  }

private:
  std::weak_ptr<char> parent_monitor_;
  int fps_;
  AVStream* stream_;
  AVMuxerInterface* muxer_;
  std::atomic<uint64_t> pkg_count_{0};
};

OutputVideoStream::OutputVideoStream(AVStream *stream, int fps,
                                     AVMuxerInterface *muxer,
                                     const std::shared_ptr<char>& monitor) {
  impl_ = std::make_unique<OutputVideoStreamImpl>(stream, fps, muxer, monitor);
}

OutputVideoStream::~OutputVideoStream() = default;

StreamType OutputVideoStream::Type() {
  return impl_->Type();
}

int OutputVideoStream::Index() {
  return impl_->Index();
}

bool OutputVideoStream::Isvalid() {
  return impl_->Isvalid();
}

void *OutputVideoStream::RawAVPtr() {
  return impl_->RawAVPtr();
}

void OutputVideoStream::FeedPacket(std::shared_ptr<AVPacket> packet) {
  impl_->FeedPacket(std::move(packet));
}
}