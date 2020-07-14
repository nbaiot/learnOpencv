//
// Created by nbaiot@126.com on 2020/7/15.
//

#ifndef LEARN_OPENCV_GENERIC_VIDEO_STREAM_H
#define LEARN_OPENCV_GENERIC_VIDEO_STREAM_H

#include "camera_stream_interface.h"

#include <container/safe_unordered_set.h>

namespace nbaiot {

class AVVideoFrameInterface;

class GenericVideoStream : public CameraStreamInterface {
public:
  GenericVideoStream(int id, int fps, int w, int h, VideoFrameType type);

  ~GenericVideoStream() override;

  void AddVideoStreamSink(VideoStreamSinkInterface* sink) override;

  void RemoveVideoStreamSink(VideoStreamSinkInterface* sink) override;

  int fps() override;

  int Id() override;

  int FrameWidth() override;

  int FrameHeight() override;

  VideoFrameType FrameType() override;

  void FeedVideoFrame(const std::shared_ptr<AVVideoFrameInterface>& frame);

private:
  const int id_;
  const int w_;
  const int h_;
  const int fps_;
  const VideoFrameType type_;
  SafeUnorderedSet<VideoStreamSinkInterface*> sinks_;

};

}


#endif //LEARN_OPENCV_GENERIC_VIDEO_STREAM_H
