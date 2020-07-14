//
// Created by nbaiot@126.com on 2020/7/2.
//

#ifndef NBAIOT_INPUT_VIDEO_STREAM_H
#define NBAIOT_INPUT_VIDEO_STREAM_H

#include "av_input_stream_interface.h"

struct AVStream;

namespace nbaiot {

class AVDemuxerInterface;

class InputVideoStreamImpl;

class InputVideoStream : public AVInputStreamInterface {

public:
  InputVideoStream(AVStream* stream,
                   AVDemuxerInterface* muxer,
                   const std::shared_ptr<char>& monitor);

  ~InputVideoStream() override;

  StreamType Type() override;

  int Index() override;

  bool Isvalid() override;

  void *RawAVPtr() override;

  void SetFrameType(VideoFrameType frameType) override;

  VideoFrameType FrameType() override;

  void SetAVFrameCallback(OnAVInputStreamAVFrameCallback callback) override;

  void SetAVVideoFrameCallback(OnAVVideoFrameCallback callback) override;

  void FeedAVPacket(const std::shared_ptr<AVPacket>& packet) override;

  int Fps();

  int64_t Bitrate();

  int Width();

  int Height();

  int PixelFormat();

private:
  std::unique_ptr<InputVideoStreamImpl> impl_;
};

}

#endif //NBAIOT_INPUT_VIDEO_STREAM_H
