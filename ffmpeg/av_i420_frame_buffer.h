//
// Created by nbaiot@126.com on 2020/7/9.
//

#ifndef NBAIOT_AV_I420_FRAME_BUFFER_H
#define NBAIOT_AV_I420_FRAME_BUFFER_H

#include "av_video_frame_buffer_interface.h"

namespace nbaiot {

class AVI420FrameBufferImpl;

class AVI420FrameBuffer : public AVI420BufferInterface {

public:
  static std::shared_ptr<AVI420FrameBuffer> Create(int w, int h, bool isJ420p = false);

  static std::shared_ptr<AVI420FrameBuffer> Copy(int w,
                                          int h,
                                          const uint8_t* dataY,
                                          int strideY,
                                          const uint8_t* dataU,
                                          int strideU,
                                          const uint8_t* dataV,
                                          int strideV);

  AVI420FrameBuffer(int w, int h, bool isJ420p);

  ~AVI420FrameBuffer() override;

  int32_t Width() override;

  int32_t Height() override;

  VideoFrameType Type() override;

  int32_t Size() override;

  void* RawAVPtr() override;

  std::shared_ptr<AVI420BufferInterface> ToI420() override;

  std::shared_ptr<AVBGR24BufferInterface> ToBGR24() override;

  std::shared_ptr<AVRGB24BufferInterface> ToRGB24() override;

  int StrideY() const override;

  int StrideU() const override;

  int StrideV() const override;

  const uint8_t* DataY() const override;

  const uint8_t* DataU() const override;

  const uint8_t* DataV() const override;

private:
  std::shared_ptr<AVI420FrameBufferImpl> impl_;
};

}

#endif //NBAIOT_AV_I420_FRAME_BUFFER_H
