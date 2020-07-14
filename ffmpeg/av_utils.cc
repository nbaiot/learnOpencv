//
// Created by nbaiot@126.com on 2020/7/6.
//

#include "av_utils.h"

#include <chrono>
#include "ffmpeg.h"

namespace nbaiot {

namespace AVUtils {

int64_t TimestampMS() {
  auto now = std::chrono::system_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

void AVInit() {
  avdevice_register_all();
  avformat_network_init();
}

void AVUnInit() {
avformat_network_deinit();
}

}

}