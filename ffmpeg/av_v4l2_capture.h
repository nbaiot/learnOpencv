//
// Created by nbaiot@126.com on 2020/7/14.
//

#ifndef NBAIOT_AV_V4L2_CAPTURE_H
#define NBAIOT_AV_V4L2_CAPTURE_H

#include <memory>
#include <string>
#include <functional>

namespace nbaiot {

class AVVideoFrameInterface;

using OnV4l2FrameCallback = std::function<void(const std::shared_ptr<AVVideoFrameInterface>&)>;

using OnV4l2DisconnectErrorCallback = std::function<void(const std::string&)>;

class AVV4l2CaptureImpl;

class AVV4l2Capture {

public:

  AVV4l2Capture();

  ~AVV4l2Capture();

  bool Open(const std::string& device, int w, int h);

  void Close();

  bool Opened();

  std::string Device();

  int Width();

  int Height();

  int Fps();

  void SetFrameCallback(OnV4l2FrameCallback callback);

  void SetDisconnectedErrorCallback(OnV4l2DisconnectErrorCallback callback);

private:
  std::unique_ptr<AVV4l2CaptureImpl> impl_;

};

}

#endif //NBAIOT_AV_V4L2_CAPTURE_H
