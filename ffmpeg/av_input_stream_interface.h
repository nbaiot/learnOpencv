//
// Created by nbaiot@126.com on 2020/7/3.
//

#ifndef NBAIOT_AV_INPUT_STREAM_INTERFACE_H
#define NBAIOT_AV_INPUT_STREAM_INTERFACE_H

#include <memory>
#include <functional>

#include "av_defines.h"

struct AVFrame;

struct AVPacket;

namespace nbaiot {

class AVDecodingVideoFrame;

using OnAVInputStreamAVFrameCallback = std::function<void(const std::shared_ptr<AVFrame>& frame)>;

using OnAVVideoFrameCallback = std::function<void(const std::shared_ptr<AVDecodingVideoFrame>& frame)>;

class AVInputStreamInterface {
public:
  virtual ~AVInputStreamInterface() = default;

  virtual StreamType Type() = 0;

  virtual int Index() = 0;

  virtual bool Isvalid() = 0;

  virtual void* RawAVPtr() = 0;

  virtual void SetFrameType(VideoFrameType frameType) = 0;

  virtual VideoFrameType FrameType() = 0;

  virtual void SetAVFrameCallback(OnAVInputStreamAVFrameCallback callback) = 0;

  virtual void SetAVVideoFrameCallback(OnAVVideoFrameCallback callback) = 0;

  virtual void FeedAVPacket(const std::shared_ptr<AVPacket>& packet) = 0;
};

}

#endif //NBAIOT_AV_INPUT_STREAM_INTERFACE_H
