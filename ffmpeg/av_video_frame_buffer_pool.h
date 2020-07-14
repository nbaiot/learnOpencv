//
// Created by nbaiot@126.com on 2020/7/9.
//

#ifndef NBAIOT_AV_VIDEO_FRAME_BUFFER_POOL_H
#define NBAIOT_AV_VIDEO_FRAME_BUFFER_POOL_H

#include <container/safe_unordered_set.h>

#include "av_video_frame_buffer_interface.h"

namespace nbaiot {

class AVVideoFrameBufferPool {

public:
  AVVideoFrameBufferPool() = delete;

  AVVideoFrameBufferPool(VideoFrameType type, size_t max_number_of_buffers = std::numeric_limits<size_t>::max());

  ~AVVideoFrameBufferPool() = default;

  std::shared_ptr<AVVideoFrameBufferInterface> CreateBuffer(int width, int height);

  int Size();

  void Release();

private:
  VideoFrameType type_;
  const size_t max_number_of_buffers_;
  SafeUnorderedSet<std::shared_ptr<AVVideoFrameBufferInterface>> buffers_;
};

}

#endif //NBAIOT_AV_VIDEO_FRAME_BUFFER_POOL_H
