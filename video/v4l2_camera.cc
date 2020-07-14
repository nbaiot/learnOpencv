//
// Created by nbaiot@126.com on 2020/7/14.
//

#include "v4l2_camera.h"

#include <glog/logging.h>
#include <ffmpeg/av_v4l2_capture.h>
#include <ffmpeg/av_video_frame_interface.h>

#include "generic_video_stream.h"

namespace nbaiot {

V4l2Camera::V4l2Camera(int id) : id_(id) {
  capture_ = std::make_unique<AVV4l2Capture>();
  capture_->SetDisconnectedErrorCallback([](const std::string& url) {
    LOG(ERROR) << ">>>>>>>>>> V4l2Camera:" << url << " disconnected";
  });
  capture_->SetFrameCallback([this](const std::shared_ptr<AVVideoFrameInterface>& frame) {
    if (video_stream_) {
      video_stream_->FeedVideoFrame(frame);
    }
  });
}

V4l2Camera::~V4l2Camera() {
  Close();
}

int V4l2Camera::Id() {
  return id_;
}

std::string V4l2Camera::Url() {
  return capture_->Device();
}

void V4l2Camera::SetStreamReadyCallback(OnCameraStreamReadyCallback callback) {
  stream_ready_callback_ = std::move(callback);
}

bool V4l2Camera::Open(const std::string& device, int w, int h) {
  auto ret = capture_->Open(device, w, h);
  if (!ret) {
    return false;
  }

  video_stream_ = std::make_shared<GenericVideoStream>(0, capture_->Fps(),
                                                       capture_->Width(),
                                                       capture_->Height(),
                                                       kYUV_I420);
  if (stream_ready_callback_) {
    stream_ready_callback_(video_stream_);
  }
  return true;
}

void V4l2Camera::Close() {
  capture_->Close();
}

bool V4l2Camera::Opened() {
  return capture_->Opened();
}

}