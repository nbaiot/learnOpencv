//
// Created by nbaiot@126.com on 2020/7/9.
//

#ifndef NBAIOT_AV_VIDEO_FRAME_H
#define NBAIOT_AV_VIDEO_FRAME_H

#include "av_video_frame_interface.h"

namespace nbaiot {

class AVVideoFrame : public AVVideoFrameInterface {

public:
  AVVideoFrame(std::shared_ptr<AVVideoFrameBufferInterface> buffer, int64_t timestamp);

  ~AVVideoFrame() override;

  int64_t Timestamp() override;

  bool IsDecodingFrame() override;

  std::vector<SEIExtendInfo> SEIInfos() override;

  std::shared_ptr<AVVideoFrameBufferInterface> FrameBuffer() override;

  void AddSEIInfo(const SEIExtendInfo& info);

private:
  int64_t timestamp_;
  std::vector<SEIExtendInfo> sei_infos_;
  std::shared_ptr<AVVideoFrameBufferInterface> buffer_;
};

}

#endif //NBAIOT_AV_VIDEO_FRAME_H
