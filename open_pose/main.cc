//
// Created by nbaiot@126.com on 2020/7/21.
//

#include <glog/logging.h>
#include <video/v4l2_camera.h>
#include <video/video_stream_sink_interface.h>
#include <ffmpeg/av_utils.h>
#include <ffmpeg/av_video_frame_interface.h>
#include <ffmpeg/av_video_frame_buffer_interface.h>
#include <log/log_helper.h>
#include <opencv2/opencv.hpp>
#include <utils/elapsed_time.h>
#include <utils/fps.h>
#include <QApplication>
#include <QMainWindow>
#include <ui/rgb24_video_preview_widget.h>

#include "body_pose.h"

using namespace nbaiot;

class PoseDetect : public VideoStreamSinkInterface {
public:
  PoseDetect(RGB24VideoPreviewWidget* widget,
      const std::string& cfg, const std::string& mode,
      OpenPoseInterface::PoseType type)
      : widget_(widget) {
    body_pose_ = std::make_unique<BodyPose>(cfg, mode, type, true);
  }

  void OnVideoFrame(const std::shared_ptr<AVVideoFrameInterface>& frame) override {
    ++count_;
    if (count_ % 2 == 0)
      return;
    ElapsedTime time;
    auto bgr24 = frame->FrameBuffer()->ToBGR24();
    cv::Mat img(cv::Size(frame->FrameBuffer()->Width(), frame->FrameBuffer()->Height()), CV_8UC3);
    std::memcpy(img.data, bgr24->Data(), bgr24->Size());
    auto t1 = time.ElapsedWallMS();

    auto result = body_pose_->Detect(img);
    auto t2 = time.ElapsedWallMS();

    body_pose_->Draw(img, std::get<0>(result), std::get<1>(result), std::get<2>(result));
    auto t3 = time.ElapsedWallMS();

    cv::Mat rgb24(img.size(), CV_8UC3);
    cv::cvtColor(img, rgb24, cv::COLOR_BGR2RGB);
    auto t4 = time.ElapsedWallMS();
    widget_->FeedMat(rgb24);

    auto t5 = time.ElapsedWallMS();
    LOG(INFO) << ">>>>>>>>>>>>>>>>>>fps:" << fps_.Fps();
    LOG(INFO) << ">>>>>>>>>>>>>>>>>>t1:" << t1 << "ms, t2:" << (t2 - t1) << "ms, t3:" << (t3 - t2) << "ms, t4:"
              << (t4 - t3) << "ms, t5:" << (t5 - t4) << "ms";
  }

private:
  int count_{0};
  FPS fps_;
  RGB24VideoPreviewWidget* widget_;
  std::unique_ptr<BodyPose> body_pose_;
};

int main(int argc, char** argv) {
  QApplication a(argc, argv);

  LogHelper::Instance()->Init("face_detect", "log/object_detect_");

  AVUtils::AVInit();

  /// init ui
  QMainWindow mainWindow;
  auto preview = new RGB24VideoPreviewWidget(1280, 720, &mainWindow);
  mainWindow.setCentralWidget(preview);

  auto camera = std::make_shared<V4l2Camera>(0);
  auto detect = std::make_shared<PoseDetect>(preview,
                                               "hand-pose/pose_deploy.prototxt",
                                               "hand-pose/pose_iter_102000.caffemodel",
                                               OpenPoseInterface::PoseType::kHand);

  camera->SetStreamReadyCallback([detect](const std::shared_ptr<CameraStreamInterface>& stream) {
    stream->AddVideoStreamSink(detect.get());
  });

  camera->Open("/dev/video0", 1280, 720);

  mainWindow.show();
  return a.exec();
}