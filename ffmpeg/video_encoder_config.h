//
// Created by nbaiot@126.com on 2020/6/29.
//

#ifndef NBAIOT_VIDEO_ENCODER_CONFIG_H
#define NBAIOT_VIDEO_ENCODER_CONFIG_H

namespace nbaiot {

struct VideoEncoderConfig {
  int width{0};
  int height{0};
  int gopSize{0};
  int maxBFrames{0}; /// 高压缩率，但会造成编码延时
  int globalQuality{-1};
  int threadCount{2};
  int64_t bitRate{0};
  bool globalHeader{false};
  int maxFPS{25};
  std::string codecName{"libx264"};

  bool operator==(const VideoEncoderConfig& rhs) const {
    return width == rhs.width &&
           height == rhs.height &&
           gopSize == rhs.gopSize &&
           maxBFrames == rhs.maxBFrames &&
           globalQuality == rhs.globalQuality &&
           threadCount == rhs.threadCount &&
           bitRate == rhs.bitRate &&
           globalHeader == rhs.globalHeader &&
           maxFPS == rhs.maxFPS &&
           codecName == rhs.codecName;
  }

  bool operator!=(const VideoEncoderConfig& rhs) const {
    return !(rhs == *this);
  }
};

}

#endif //NBAIOT_VIDEO_ENCODER_CONFIG_H
