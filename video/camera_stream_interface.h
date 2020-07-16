//
// Created by nbaiot@126.com on 2020/7/14.
//

#ifndef NBAIOT_CAMERA_STREAM_INTERFACE_H
#define NBAIOT_CAMERA_STREAM_INTERFACE_H

#include <memory>
#include <ffmpeg/av_defines.h>

namespace nbaiot {

class VideoStreamSinkInterface;

class CameraStreamInterface {

public:
  virtual ~CameraStreamInterface() = default;

  virtual void AddVideoStreamSink(VideoStreamSinkInterface* sink) = 0;

  virtual void RemoveVideoStreamSink(VideoStreamSinkInterface* sink) = 0;

  virtual int fps() = 0;

  virtual int Id() = 0;

  virtual int FrameWidth() = 0;

  virtual int FrameHeight() = 0;

  virtual VideoFrameType FrameType() = 0;
};

}

#endif //NBAIOT_CAMERA_STREAM_INTERFACE_H
