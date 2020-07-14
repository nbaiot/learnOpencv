//
// Created by nbaiot@126.com on 2020/7/14.
//

#include "av_v4l2_capture.h"

#include "ffmpeg.h"
#include "av_deleter.h"
#include "av_demuxer.h"
#include "av_decoding_video_frame.h"
#include "input_video_stream.h"
#include <glog/logging.h>

namespace nbaiot {

AVV4l2Capture::AVV4l2Capture() {
  demuxer_ = std::make_unique<AVDemuxer>();
  demuxer_->SetEOFCallback([this](const std::string& url) {
      if (disconnect_callback_) {
        disconnect_callback_(url);
      }
  });

  demuxer_->SetInputStreamCallback([this](const std::shared_ptr<AVInputStreamInterface>& stream) {
      if (stream->Type() != kVideoStream) {
        return;
      }
      video_stream_ = std::dynamic_pointer_cast<InputVideoStream>(stream);
      video_stream_->SetAVVideoFrameCallback([this](const std::shared_ptr<AVDecodingVideoFrame>& frame) {
        if (frame_callback_) {
          frame_callback_(frame);
        }
      });
  });
}

AVV4l2Capture::~AVV4l2Capture() {
  Close();
}

bool AVV4l2Capture::Open(const std::string& device, int w, int h, Mode mode) {
  if (demuxer_->Opened()) {
    LOG(ERROR) << ">>>>>>>>>> AVV4l2Capture already opened.";
    return false;
  }

  device_ = device;

  auto v4l2Format = av_find_input_format("v4l2");

  if (!v4l2Format) {
    LOG(ERROR) << ">>>>>>>>>> AVV4l2Capture open failed: current system not support v4l2";
    return false;
  }

  std::string codeName;
  if (mode == kJpeg) {
    codeName = "mjpeg";
  } else {
    codeName = "rawvideo";
  }

  std::string videoSize = std::to_string(w) + "x" + std::to_string(h);
  AVDictionary* rawOpts = nullptr;
  av_dict_set(&rawOpts, "input_format", codeName.c_str(), 0);
  av_dict_set(&rawOpts, "video_size", videoSize.c_str(), 0);
  auto opts = std::shared_ptr<AVDictionary>(rawOpts, AVDeleter::AVDictionaryDeleter());
  return demuxer_->Open(device_, v4l2Format, opts);
}

void AVV4l2Capture::Close() {
  demuxer_->Close();
  device_.clear();
  video_stream_.reset();
}

bool AVV4l2Capture::Opened() {
  return demuxer_->Opened();
}

std::string AVV4l2Capture::Device() {
  return device_;
}

int AVV4l2Capture::Width() {
  if (!video_stream_)
    return -1;
  return video_stream_->Width();
}

int AVV4l2Capture::Height() {
  if (!video_stream_)
    return -1;
  return video_stream_->Height();
}

int AVV4l2Capture::Fps() {
  if (!video_stream_)
    return -1;
  return video_stream_->Fps();
}

void AVV4l2Capture::SetFrameCallback(OnV4l2FrameCallback callback) {
  frame_callback_ = std::move(callback);
}

void AVV4l2Capture::SetDisconnectedErrorCallback(OnV4l2DisconnectErrorCallback callback) {
  disconnect_callback_ = std::move(callback);
}

}