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

using namespace nbaiot;

class VideoSink : public VideoStreamSinkInterface {
public:
  void OnVideoFrame(const std::shared_ptr<AVVideoFrameInterface>& frame) override {
    LOG(INFO) << ">>>>>>>>>>>>>>>>>>frame:" << frame->FrameBuffer()->Width() << "*" << frame->FrameBuffer()->Height();
  }
};

int main() {

  AVUtils::AVInit();

  auto camera = std::make_shared<V4l2Camera>(0);

  camera->SetStreamReadyCallback([](const std::shared_ptr<CameraStreamInterface>& stream){
    stream->AddVideoStreamSink(new VideoSink());
  });

  camera->Open("/dev/video0", 1280, 720);

  std::mutex mutex;
  std::unique_lock<std::mutex> lk(mutex);
  std::condition_variable cv;
  cv.wait(lk);

  return 0;
}
