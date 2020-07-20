//
// Created by nbaiot@126.com on 2020/7/20.
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

#include "yolo3_object_detect.h"

using namespace nbaiot;

class ObjectDetect : public VideoStreamSinkInterface {
public:
  ObjectDetect(RGB24VideoPreviewWidget* widget, const std::string& cocoFile,
               const std::string& configFile, const std::string& weightFile)
      : widget_(widget) {

    detector_ = std::make_unique<Yolo3ObjectDetect>();
    if (!detector_->LoadModel(cocoFile, configFile, weightFile)) {
      LOG(ERROR) << ">>>>>>>>>> Load YOLO3 model failed";
    }
  }

  void OnVideoFrame(const std::shared_ptr<AVVideoFrameInterface>& frame) override {
    ElapsedTime time;
    auto bgr24 = frame->FrameBuffer()->ToBGR24();
    cv::Mat img(cv::Size(frame->FrameBuffer()->Width(), frame->FrameBuffer()->Height()), CV_8UC3);
    std::memcpy(img.data, bgr24->Data(), bgr24->Size());
    auto t1 = time.ElapsedWallMS();

    std::vector<cv::Mat> outs;
    detector_->Detect(img, outs);
    auto t2 = time.ElapsedWallMS();
    auto results = detector_->Postprocess(img, outs);
    auto t3 = time.ElapsedWallMS();
    for (const auto& result : results) {
      detector_->DrawPred(std::get<0>(result), std::get<1>(result), std::get<2>(result), img);
    }

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
  FPS fps_;
  RGB24VideoPreviewWidget* widget_;
  std::unique_ptr<Yolo3ObjectDetect> detector_;
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
    auto detect = std::make_shared<ObjectDetect>(preview, "coco.names",
        "yolov3.cfg",
        "yolov3.weights");

    camera->SetStreamReadyCallback([detect](const std::shared_ptr<CameraStreamInterface>& stream) {
      stream->AddVideoStreamSink(detect.get());
    });

    camera->Open("/dev/video0", 1280, 720);

    mainWindow.show();
    return a.exec();
  }