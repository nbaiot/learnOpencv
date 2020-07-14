//
// Created by nbaiot@126.com on 2020/7/2.
//

#ifndef NBAIOT_AV_OUTPUT_STREAM_INTERFACE_H
#define NBAIOT_AV_OUTPUT_STREAM_INTERFACE_H

#include <memory>

#include "av_defines.h"

struct AVPacket;

namespace nbaiot {

class AVOutputStreamInterface {

public:
  virtual ~AVOutputStreamInterface() = default;

  virtual StreamType Type() = 0;

  virtual int Index() = 0;

  virtual bool Isvalid() = 0;

  virtual void* RawAVPtr() = 0;

  virtual void FeedPacket(std::shared_ptr<AVPacket> packet) = 0;
};

}

#endif //NBAIOT_AV_OUTPUT_STREAM_INTERFACE_H
