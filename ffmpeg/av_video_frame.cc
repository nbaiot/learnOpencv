//
// Created by nbaiot@126.com on 2020/7/9.
//

#include "av_video_frame.h"

#include "av_defines.h"
#include "av_video_frame_buffer_interface.h"

namespace nbaiot {

AVVideoFrame::AVVideoFrame(std::shared_ptr<AVVideoFrameBufferInterface> buffer, int64_t timestamp)
: buffer_(std::move(buffer)), timestamp_(timestamp) {

}

AVVideoFrame::~AVVideoFrame() = default;

int64_t AVVideoFrame::Timestamp() {
  return timestamp_;
}

bool AVVideoFrame::IsDecodingFrame() {
  return false;
}

std::vector<SEIExtendInfo> AVVideoFrame::SEIInfos() {
  return sei_infos_;
}

void AVVideoFrame::AddSEIInfo(const SEIExtendInfo& info) {
  sei_infos_.push_back(info);
}

std::shared_ptr<AVVideoFrameBufferInterface> AVVideoFrame::FrameBuffer() {
  return buffer_;
}

}