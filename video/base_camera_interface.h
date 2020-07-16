//
// Created by nbaiot@126.com on 2020/7/14.
//

#ifndef NBAIOT_BASE_CAMERA_INTERFACE_H
#define NBAIOT_BASE_CAMERA_INTERFACE_H

#include <string>
#include <memory>
#include <functional>
#include "camera_stream_interface.h"

namespace nbaiot {

class CameraStreamInterface;

using OnCameraStreamReadyCallback = std::function<void(const std::shared_ptr<CameraStreamInterface>&)>;

class BaseCameraInterface {

public:
  virtual ~BaseCameraInterface() = default;

  virtual int Id() = 0;

  virtual std::string Url() = 0;

  virtual void SetStreamReadyCallback(OnCameraStreamReadyCallback callback) = 0;

};

}

#endif //NBAIOT_BASE_CAMERA_INTERFACE_H
