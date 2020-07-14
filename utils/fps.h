//
// Created by nbaiot@126.com on 2020/6/4.
//

#ifndef NBAIOT_FPS_H
#define NBAIOT_FPS_H

#include <chrono>
#include <cstdint>

namespace nbaiot {

class FPS {

public:
  FPS() = default;

  void Start() {
    mean_last_time_ = std::chrono::system_clock::now();
    last_time_ = mean_last_time_;
    frame_num_ = 0;
    mean_frame_num_ = 0;
    current_mean_fps_ = 0;
  }

  int MeanFps() {
    ++mean_frame_num_;
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - mean_last_time_);
    if ( duration >= std::chrono::milliseconds(1000)) {
      current_mean_fps_ = static_cast<int>(mean_frame_num_ * 1000 / (duration.count()));
    }
    return current_mean_fps_;
  }

  int Fps() {
    ++frame_num_;
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time_);
    last_time_ = now;
    return static_cast<int>(1.f / duration.count() * 1000);
  }

private:
  int mean_frame_num_{0};
  int current_mean_fps_{0};
  std::chrono::system_clock::time_point mean_last_time_;

  int frame_num_{0};
  std::chrono::system_clock::time_point last_time_;

};
}

#endif //NBAIOT_FPS_H
