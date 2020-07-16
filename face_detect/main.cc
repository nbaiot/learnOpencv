//
// Created by nbaiot@126.com on 2020/7/15.
//

#include <mutex>
#include <condition_variable>

#include <glog/logging.h>
#include <video/v4l2_camera.h>
#include <video/video_stream_sink_interface.h>
#include <ffmpeg/av_utils.h>
#include <ffmpeg/av_video_frame_interface.h>
#include <ffmpeg/av_video_frame_buffer_interface.h>
#include <log/log_helper.h>
#include <utils/fps.h>
#include <utils/elapsed_time.h>
#include <opencv2/opencv.hpp>

#include <QApplication>
#include <QMainWindow>
#include <ui/rgb24_video_preview_widget.h>

#include "haar_face_detect.h"
#include "dnn_tf_face_detect.h"

using namespace nbaiot;

class FaceDetect : public VideoStreamSinkInterface {
public:
  FaceDetect(RGB24VideoPreviewWidget* widget) : widget_(widget) {
    cv::cuda::printShortCudaDeviceInfo(cv::cuda::getDevice());
//    face_detector_ = std::make_unique<HaarFaceDetect>("haarcascade_frontalface_alt2.xml");
    face_detector_ = std::make_unique<DNNTFFaceDetect>(
        "opencv_face_detector_uint8.pb",
        "opencv_face_detector.pbtxt");
  }

  void OnVideoFrame(const std::shared_ptr<AVVideoFrameInterface>& frame) override {
    ElapsedTime time;
    auto bgr24 = frame->FrameBuffer()->ToBGR24();
    cv::Mat img(cv::Size(frame->FrameBuffer()->Width(), frame->FrameBuffer()->Height()), CV_8UC3);
    std::memcpy(img.data, bgr24->Data(), bgr24->Size());
    auto t1 = time.ElapsedWallMS();
    std::vector<cv::Rect> objects;
    face_detector_->Detect(img, objects);
    auto t2 = time.ElapsedWallMS();
    cv::Mat rgb24(img.size(), CV_8UC3);
    cv::cvtColor(img, rgb24, cv::COLOR_BGR2RGB);
    auto t3 = time.ElapsedWallMS();
    for (int i = 0; i < objects.size(); i++) {
      cv::rectangle(rgb24, objects[i], cv::Scalar(255, 0, 0));
    }
    auto t4 = time.ElapsedWallMS();

    widget_->FeedMat(rgb24);

    auto t5 = time.ElapsedWallMS();
    LOG(INFO) << ">>>>>>>>>>>>>>>>>>fps:" << fps_.Fps();
    LOG(INFO) << ">>>>>>>>>>>>>>>>>>face count:" << objects.size();
    LOG(INFO) << ">>>>>>>>>>>>>>>>>>t1:" << t1 << "ms, t2:" << (t2 - t1) << "ms, t3:" << (t3 - t2) << "ms, t4:"
              << (t4 - t3) << "ms, t5:" << (t5 - t4) << "ms";
  }

private:
  FPS fps_;
  RGB24VideoPreviewWidget* widget_;
  std::unique_ptr<FaceDetectInterface> face_detector_;

};


int main(int argc, char** argv) {

  QApplication a(argc, argv);

  LogHelper::Instance()->Init("face_detect", "log/face_detect_");

  AVUtils::AVInit();

  /// init ui
  QMainWindow mainWindow;
  auto preview = new RGB24VideoPreviewWidget(1280, 720, &mainWindow);
  mainWindow.setCentralWidget(preview);

  auto camera = std::make_shared<V4l2Camera>(0);
  auto detect = std::make_shared<FaceDetect>(preview);

  camera->SetStreamReadyCallback([detect](const std::shared_ptr<CameraStreamInterface>& stream) {
    stream->AddVideoStreamSink(detect.get());
  });

  camera->Open("/dev/video0", 1280, 720);

  mainWindow.show();
  return a.exec();


}
