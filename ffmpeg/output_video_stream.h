//
// Created by nbaiot@126.com on 2020/7/2.
//

#ifndef NBAIOT_OUTPUT_VIDEO_STREAM_H
#define NBAIOT_OUTPUT_VIDEO_STREAM_H

#include "av_output_stream_interface.h"

struct AVStream;

namespace nbaiot {

class OutputVideoStreamImpl;

class AVMuxerInterface;

class OutputVideoStream : public  AVOutputStreamInterface {

public:
  OutputVideoStream(AVStream* stream,
      int fps,
      AVMuxerInterface* muxer,
      const std::shared_ptr<char>& monitor);

  ~OutputVideoStream() override;

  StreamType Type() override;

  int Index() override;

  bool Isvalid() override;

  void *RawAVPtr() override;

  void FeedPacket(std::shared_ptr<AVPacket> packet) override;

private:
  std::unique_ptr<OutputVideoStreamImpl> impl_;
};

}

#endif //NBAIOT_OUTPUT_VIDEO_STREAM_H
