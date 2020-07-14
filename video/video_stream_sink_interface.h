//
// Created by nbaiot@126.com on 2020/7/14.
//

#ifndef LEARN_OPENCV_VIDEO_STREAM_SINK_INTERFACE_H
#define LEARN_OPENCV_VIDEO_STREAM_SINK_INTERFACE_H

namespace nbaiot {

class AVVideoFrameInterface;

class VideoStreamSinkInterface {

public:
  virtual ~VideoStreamSinkInterface() = default;

  virtual void OnVideoFrame(const std::shared_ptr<AVVideoFrameInterface>& frame) = 0;

};

}

#endif //LEARN_OPENCV_VIDEO_STREAM_SINK_INTERFACE_H
