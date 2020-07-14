//
// Created by nbaiot@126.com on 2020/7/9.
//

#ifndef NBAIOT_AV_VIDEO_FRAME_BUFFER_INTERFACE_H
#define NBAIOT_AV_VIDEO_FRAME_BUFFER_INTERFACE_H

#include <memory>
#include <cstdint>

#include "av_defines.h"

namespace nbaiot {

class AVI420BufferInterface;

class AVBGR24BufferInterface;

class AVRGB24BufferInterface;

class AVVideoFrameBufferInterface :  public std::enable_shared_from_this<AVVideoFrameBufferInterface> {

public:
  virtual ~AVVideoFrameBufferInterface() = default;

  virtual int32_t Width() = 0;

  virtual int32_t Height() = 0;

  virtual VideoFrameType Type() = 0;

  virtual int32_t Size() = 0;

  virtual void* RawAVPtr() = 0;

  virtual std::shared_ptr<AVI420BufferInterface> ToI420() = 0;

  virtual std::shared_ptr<AVBGR24BufferInterface> ToBGR24() = 0;

  virtual std::shared_ptr<AVRGB24BufferInterface> ToRGB24() = 0;

};

class AVI420BufferInterface : public AVVideoFrameBufferInterface {
public:
  ~AVI420BufferInterface() override = default;

  virtual int StrideY() const = 0;

  virtual int StrideU() const = 0;

  virtual int StrideV() const = 0;

  virtual const uint8_t* DataY() const = 0;

  virtual const uint8_t* DataU() const = 0;

  virtual const uint8_t* DataV() const = 0;
};

class AVBGR24BufferInterface : public AVVideoFrameBufferInterface {
public:
  ~AVBGR24BufferInterface() override = default;

  virtual const uint8_t* Data() const = 0;

  virtual int Stride() const = 0;

};


class AVRGB24BufferInterface : public AVVideoFrameBufferInterface {
public:
  ~AVRGB24BufferInterface() override = default;

  virtual const uint8_t* Data() const = 0;

  virtual int Stride() const = 0;
};

}

#endif //NBAIOT_AV_VIDEO_FRAME_BUFFER_INTERFACE_H
