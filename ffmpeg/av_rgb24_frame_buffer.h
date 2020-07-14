//
// Created by nbaiot@126.com on 2020/7/9.
//

#ifndef NBAIOT_AV_RGB24_FRAME_BUFFER_H
#define NBAIOT_AV_RGB24_FRAME_BUFFER_H

#include "av_video_frame_buffer_interface.h"

namespace nbaiot {

class AVRGB24FrameBufferImpl;

class AVRGB24FrameBuffer : public AVRGB24BufferInterface {

public:
  static std::shared_ptr<AVRGB24FrameBuffer> Create(int w, int h);

  AVRGB24FrameBuffer(int w, int h);

  ~AVRGB24FrameBuffer() override;

  int32_t Width() override;

  int32_t Height() override;

  VideoFrameType Type() override;

  int32_t Size() override;

  void* RawAVPtr() override;

  std::shared_ptr<AVI420BufferInterface> ToI420() override;

  std::shared_ptr<AVBGR24BufferInterface> ToBGR24() override;

  std::shared_ptr<AVRGB24BufferInterface> ToRGB24() override;

  const uint8_t* Data() const override;

  int Stride() const override;

private:
  std::shared_ptr<AVRGB24FrameBufferImpl> impl_;
};

}

#endif //NBAIOT_AV_RGB24_FRAME_BUFFER_H
