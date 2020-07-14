//
// Created by nbaiot@126.com on 2020/7/15.
//

#include "generic_video_stream.h"
#include "video_stream_sink_interface.h"
#include <ffmpeg/av_video_frame_interface.h>

namespace nbaiot {

GenericVideoStream::GenericVideoStream(int id, int fps, int w, int h, VideoFrameType type)
    : id_(id), fps_(fps), w_(w), h_(h), type_(type) {

}

GenericVideoStream::~GenericVideoStream() = default;

void GenericVideoStream::AddVideoStreamSink(VideoStreamSinkInterface* sink) {
  sinks_.Insert(sink);
}

void GenericVideoStream::RemoveVideoStreamSink(VideoStreamSinkInterface* sink) {
  sinks_.Remove(sink);
}

int GenericVideoStream::fps() {
  return fps_;
}

int GenericVideoStream::Id() {
  return id_;
}

int GenericVideoStream::FrameWidth() {
  return w_;
}

int GenericVideoStream::FrameHeight() {
  return h_;
}

VideoFrameType GenericVideoStream::FrameType() {
  return type_;
}

void GenericVideoStream::FeedVideoFrame(const std::shared_ptr<AVVideoFrameInterface>& frame) {
  sinks_.ForEach([frame](VideoStreamSinkInterface* sink) -> bool {
    sink->OnVideoFrame(frame);
    return true;
  });
}


}