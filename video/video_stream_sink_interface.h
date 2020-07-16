//
// Created by nbaiot@126.com on 2020/7/14.
//

#ifndef NBAIOT_VIDEO_STREAM_SINK_INTERFACE_H
#define NBAIOT_VIDEO_STREAM_SINK_INTERFACE_H

namespace nbaiot {

class AVVideoFrameInterface;

class VideoStreamSinkInterface {

public:
  virtual ~VideoStreamSinkInterface() = default;

  virtual void OnVideoFrame(const std::shared_ptr<AVVideoFrameInterface>& frame) = 0;

};

}

#endif //NBAIOT_VIDEO_STREAM_SINK_INTERFACE_H
