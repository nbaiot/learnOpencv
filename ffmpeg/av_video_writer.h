//
// Created by nbaiot@126.com on 2020/6/29.
//

#ifndef NBAIOT_AV_VIDEO_WRITER_H
#define NBAIOT_AV_VIDEO_WRITER_H

#include <memory>
#include <string>
#include <cstdint>

#include "video_encoder_config.h"

namespace nbaiot {

class AVVideoWriterImpl;

class AVVideoFrame;

class AVVideoWriter {

public:
  AVVideoWriter(std::string fileName,
      std::string mineType,
      int w, int h, int fps,
      int64_t bitRate,
      std::string codecName);

  ~AVVideoWriter();

  /// 必须在 open 之前调用
  void SetGOP(int gop);

  void SetMaxBFrames(int bFrames);

  void SetEncodeThreadCount(int count);

  bool Open();

  bool IsOpened();

  void Close();

  void Write(const std::shared_ptr<AVVideoFrame>& frame);

private:
  std::unique_ptr<AVVideoWriterImpl> impl_;
};

}

#endif //NBAIOT_AV_VIDEO_WRITER_H
