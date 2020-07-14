//
// Created by nbaiot@126.com on 2020/7/14.
//

#ifndef NBAIOT_V4L2_CAMERA_H
#define NBAIOT_V4L2_CAMERA_H

#include "base_camera_interface.h"

namespace nbaiot {

class AVV4l2Capture;

class GenericVideoStream;

class V4l2Camera : public BaseCameraInterface {

public:
  explicit V4l2Camera(int id);

  ~V4l2Camera() override;

  int Id() override;

  std::string Url() override;

  bool Open(const std::string& device, int w, int h);

  void Close();

  bool Opened();

  void SetStreamReadyCallback(OnCameraStreamReadyCallback callback) override;

private:
  const int id_;
  std::unique_ptr<AVV4l2Capture> capture_;
  std::shared_ptr<GenericVideoStream> video_stream_;
  OnCameraStreamReadyCallback stream_ready_callback_;

};

}


#endif //NBAIOT_V4L2_CAMERA_H
