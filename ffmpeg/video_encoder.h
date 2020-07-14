//
// Created by nbaiot@126.com on 2020/6/29.
//

#ifndef NBAIOT_VIDEO_ENCODER_H
#define NBAIOT_VIDEO_ENCODER_H

#include <memory>
#include <string>
#include <functional>

#include "video_encoder_config.h"

class AVPacket;

namespace nbaiot {

class VideoEncoderImpl;

class AVVideoFrameInterface;

using OnVideoEncodeCallback = std::function<void(const std::shared_ptr<AVPacket>& packet)>;

class VideoEncoder {

public:
  VideoEncoder();

  ~VideoEncoder();

  bool Open(const VideoEncoderConfig& config);

  void Close();

  bool Opened() const;

  VideoEncoderConfig Config();

  bool SetOption(const std::string& key, const std::string& val);

  std::string CodecName();

  /// TODO: add sei info
  void Encode(const std::shared_ptr<AVVideoFrameInterface>& frame);

  void Flush();

  void SetEncodeCallback(OnVideoEncodeCallback callback);

  void* RawAVPtr();

  void* RawAVPtr() const;

private:
  std::unique_ptr<VideoEncoderImpl> impl_;
};

}

#endif //NBAIOT_VIDEO_ENCODER_H
