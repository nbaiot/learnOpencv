//
// Created by nbaiot@126.com on 2020/7/9.
//

#include "av_video_frame_buffer_pool.h"
#include "av_rgb24_frame_buffer.h"
#include "av_bgr24_frame_buffer.h"
#include "av_i420_frame_buffer.h"

namespace nbaiot {

AVVideoFrameBufferPool::AVVideoFrameBufferPool(VideoFrameType type, size_t max_number_of_buffers)
    : type_(type), max_number_of_buffers_(max_number_of_buffers) {

}

std::shared_ptr<AVVideoFrameBufferInterface> AVVideoFrameBufferPool::CreateBuffer(int width, int height) {
  bool needClear = false;
  buffers_.ForEach([&needClear, width, height](const std::shared_ptr<AVVideoFrameBufferInterface>& value) -> bool {
      if (value->Width() != width || value->Height() != height) {
        needClear = true;
        return false;
      }
      return true;
  });

  if (needClear) {
    buffers_.Clear();
  }

  std::shared_ptr<AVVideoFrameBufferInterface> buffer;
  buffers_.ForEach([&buffer, this](const std::shared_ptr<AVVideoFrameBufferInterface>& value) -> bool {
      if (value.unique() && value->Type() == type_) {
        buffer = value;
        return false;
      }
      return true;
  });

  if (buffer) {
    return buffer;
  }

  std::shared_ptr<AVVideoFrameBufferInterface> newBuffer;
  switch (type_) {
    case kYUV_I420:
      newBuffer = AVI420FrameBuffer::Create(width, height);
      break;
    case kYUV_J420:
      newBuffer = AVI420FrameBuffer::Create(width, height, true);
      break;
    case kRGB24:
      newBuffer = AVRGB24FrameBuffer::Create(width, height);
      break;
    case kBGR24:
      newBuffer = AVBGR24FrameBuffer::Create(width, height);
      break;
  }

  if (buffers_.Size() <= max_number_of_buffers_) {
    buffers_.Insert(newBuffer);
  }

  return newBuffer;
}

int AVVideoFrameBufferPool::Size() {
  return buffers_.Size();
}

void AVVideoFrameBufferPool::Release() {
  buffers_.Clear();
}


}