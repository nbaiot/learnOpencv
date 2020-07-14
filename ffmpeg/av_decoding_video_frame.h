//
// Created by nbaiot@126.com on 2020/7/9.
//

#ifndef NBAIOT_AV_DECODING_VIDEO_FRAME_H
#define NBAIOT_AV_DECODING_VIDEO_FRAME_H

#include "av_video_frame_interface.h"

struct AVFrame;

namespace nbaiot {

class AVDecodingVideoFrame : public AVVideoFrameInterface {

public:
  AVDecodingVideoFrame(std::shared_ptr<AVVideoFrameBufferInterface> buffer,
      int64_t timestamp, int pts, bool isKeyFrame);

  ~AVDecodingVideoFrame() override;

  int64_t Timestamp() override;

  bool IsDecodingFrame() override;

  std::vector<SEIExtendInfo> SEIInfos() override;

  std::shared_ptr<AVVideoFrameBufferInterface> FrameBuffer() override;

  void AddSEIInfo(const SEIExtendInfo& info);

  int64_t dts();

  int64_t pts();

  bool isKeyFrame();

  static std::shared_ptr<AVDecodingVideoFrame> Create(const std::shared_ptr<AVVideoFrameBufferInterface>& buffer,
                                                      int64_t timestamp,
                                                      const std::shared_ptr<AVFrame>& avFrame);

private:
  int64_t pts_{-1};
  int64_t timestamp_{-1};
  bool is_key_frame_{false};
  std::vector<SEIExtendInfo> sei_infos_;
  std::shared_ptr<AVVideoFrameBufferInterface> buffer_;
};

}

#endif //NBAIOT_AV_DECODING_VIDEO_FRAME_H
