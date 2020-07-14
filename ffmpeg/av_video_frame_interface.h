//
// Created by nbaiot@126.com on 2020/7/9.
//

#ifndef NBAIOT_AV_VIDEO_FRAME_INTERFACE_H
#define NBAIOT_AV_VIDEO_FRAME_INTERFACE_H

#include <vector>
#include <memory>

namespace nbaiot {

class AVVideoFrameBufferInterface;

struct SEIExtendInfo;

class AVVideoFrameInterface {

public:
  virtual ~AVVideoFrameInterface() = default;

  virtual int64_t Timestamp() = 0;

  virtual bool IsDecodingFrame() = 0;

  virtual std::vector<SEIExtendInfo> SEIInfos() = 0;

  virtual std::shared_ptr<AVVideoFrameBufferInterface> FrameBuffer() = 0;
};

}

#endif //NBAIOT_AV_VIDEO_FRAME_INTERFACE_H
