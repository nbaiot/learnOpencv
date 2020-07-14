//
// Created by nbaiot@126.com on 2020/7/6.
//

#ifndef NBAIOT_AV_UTILS_H
#define NBAIOT_AV_UTILS_H

#include <cstdint>

namespace nbaiot {

namespace AVUtils {

/// TODO: add init ffmpeg and set log level

void AVInit();

void AVUnInit();

int64_t TimestampMS();

template<typename T, typename V = T>
class ScopedValue {

public:
  ScopedValue(T& var, const V& outValue)
      : var(var),
        outValue(outValue) {
  }

  ScopedValue(T& var, const V& inValue, const V& outValue)
      : var(var),
        outValue(outValue) {
    this->var = inValue;
  }

  ~ScopedValue() {
    var = outValue;
  }

private:
  T& var;
  V outValue;
};

}

}

#endif //NBAIOT_AV_UTILS_H
