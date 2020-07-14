//
// Created by nbaiot@126.com on 2020/7/2.
//

#ifndef NBAIOT_AV_DEFINES_H
#define NBAIOT_AV_DEFINES_H

#include <vector>
#include <string>
#include <cstdint>

namespace nbaiot {

enum StreamType {
  kVideoStream,
  kAudioStream,
};

enum VideoFrameType {
  kYUV_I420,
  kYUV_J420,
  kRGB24,
  kBGR24,
};

struct SEIExtendInfo {
  std::vector<uint8_t> uuid;
  std::string sei;
};

using StreamType = enum StreamType;

}

#endif //NBAIOT_AV_DEFINES_H
