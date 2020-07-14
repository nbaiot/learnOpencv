//
// Created by nbaiot@126.com on 2020/7/9.
//

#ifndef NBAIOT_AV_VIDEO_FRAME_BUFFER_FACTORY_H
#define NBAIOT_AV_VIDEO_FRAME_BUFFER_FACTORY_H

#include <map>
#include <memory>
#include <limits>
#include "av_defines.h"

namespace nbaiot {

class AVVideoFrameBufferInterface;

class AVVideoFrameBufferPool;

class AVVideoFrameBufferFactory {
public:
  explicit AVVideoFrameBufferFactory(size_t poolSize = std::numeric_limits<size_t>::max());

  ~AVVideoFrameBufferFactory() = default;

  std::shared_ptr<AVVideoFrameBufferInterface> Create(int w, int h, VideoFrameType type);

private:
  std::map<VideoFrameType, std::shared_ptr<AVVideoFrameBufferPool>> pools_;
};

}

#endif //NBAIOT_AV_VIDEO_FRAME_BUFFER_FACTORY_H
