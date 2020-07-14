//
// Created by nbaiot@126.com on 2020/7/14.
//

#ifndef NBAIOT_AV_V4L2_CAPTURE_H
#define NBAIOT_AV_V4L2_CAPTURE_H

#include <memory>
#include <string>
#include <functional>

namespace nbaiot {

class AVDemuxer;

class AVVideoFrameInterface;

class InputVideoStream;

using OnV4l2FrameCallback = std::function<void(const std::shared_ptr<AVVideoFrameInterface>&)>;

using OnV4l2DisconnectErrorCallback = std::function<void(const std::string&)>;

class AVV4l2Capture {

public:
  enum Mode {
    kJpeg,
    kRaw,
  };

  AVV4l2Capture();

  ~AVV4l2Capture();

  bool Open(const std::string& device, int w, int h, Mode mode = kJpeg);

  void Close();

  bool Opened();

  std::string Device();

  int Width();

  int Height();

  int Fps();

  void SetFrameCallback(OnV4l2FrameCallback callback);

  void SetDisconnectedErrorCallback(OnV4l2DisconnectErrorCallback callback);

private:
  std::string device_;
  std::unique_ptr<AVDemuxer> demuxer_;
  std::shared_ptr<InputVideoStream> video_stream_;
  OnV4l2FrameCallback frame_callback_;
  OnV4l2DisconnectErrorCallback disconnect_callback_;

};

}

#endif //NBAIOT_AV_V4L2_CAPTURE_H
