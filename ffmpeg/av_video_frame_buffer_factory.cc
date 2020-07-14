//
// Created by nbaiot@126.com on 2020/7/9.
//

#include "av_video_frame_buffer_factory.h"
#include "av_video_frame_buffer_pool.h"
namespace nbaiot {

AVVideoFrameBufferFactory::AVVideoFrameBufferFactory(size_t poolSize) {
  pools_[kYUV_I420] = std::make_shared<AVVideoFrameBufferPool>(kYUV_I420, poolSize);
  pools_[kYUV_J420] = std::make_shared<AVVideoFrameBufferPool>(kYUV_J420, poolSize);
  pools_[kRGB24] = std::make_shared<AVVideoFrameBufferPool>(kRGB24, poolSize);
  pools_[kBGR24] = std::make_shared<AVVideoFrameBufferPool>(kBGR24, poolSize);
}

std::shared_ptr<AVVideoFrameBufferInterface> AVVideoFrameBufferFactory::Create(int w, int h, VideoFrameType type) {
  return  pools_[type]->CreateBuffer(w, h);
}

}