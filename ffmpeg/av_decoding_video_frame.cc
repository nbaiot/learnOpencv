//
// Created by nbaiot@126.com on 2020/7/9.
//

#include "av_decoding_video_frame.h"

#include <libyuv.h>
#include <glog/logging.h>

#include "av_defines.h"
#include "ffmpeg.h"
#include "av_video_frame_buffer_interface.h"

namespace nbaiot {

AVDecodingVideoFrame::AVDecodingVideoFrame(std::shared_ptr<AVVideoFrameBufferInterface> buffer,
                                           int64_t timestamp, int pts, bool isKeyFrame)
    : buffer_(std::move(buffer)), timestamp_(timestamp), pts_(pts), is_key_frame_(isKeyFrame) {

}

AVDecodingVideoFrame::~AVDecodingVideoFrame() = default;

int64_t AVDecodingVideoFrame::Timestamp() {
  return timestamp_;
}

bool AVDecodingVideoFrame::IsDecodingFrame() {
  return true;
}

std::vector<SEIExtendInfo> AVDecodingVideoFrame::SEIInfos() {
  return sei_infos_;
}

std::shared_ptr<AVVideoFrameBufferInterface> AVDecodingVideoFrame::FrameBuffer() {
  return buffer_;
}

void AVDecodingVideoFrame::AddSEIInfo(const SEIExtendInfo& info) {
  sei_infos_.push_back(info);
}

int64_t AVDecodingVideoFrame::dts() {
  return timestamp_;
}

int64_t AVDecodingVideoFrame::pts() {
  return pts_;
}

bool AVDecodingVideoFrame::isKeyFrame() {
  return is_key_frame_;
}

std::shared_ptr<AVDecodingVideoFrame>
AVDecodingVideoFrame::Create(const std::shared_ptr<AVVideoFrameBufferInterface>& buffer, int64_t timestamp,
                             const std::shared_ptr<AVFrame>& avFrame) {

  auto type = buffer->Type();
  if (type == kBGR24) {
    auto bgr24 = std::dynamic_pointer_cast<AVBGR24BufferInterface>(buffer);
    if (avFrame->format == AV_PIX_FMT_YUVJ420P) {
      libyuv::J420ToRGB24(avFrame->data[0], avFrame->linesize[0],
                          avFrame->data[1], avFrame->linesize[1],
                          avFrame->data[2], avFrame->linesize[2],
                          const_cast<uint8_t*>(bgr24->Data()), bgr24->Stride(),
                          avFrame->width, avFrame->height);
    } else if (avFrame->format == AV_PIX_FMT_YUV420P) {
      libyuv::I420ToRGB24(avFrame->data[0], avFrame->linesize[0],
                          avFrame->data[1], avFrame->linesize[1],
                          avFrame->data[2], avFrame->linesize[2],
                          const_cast<uint8_t*>(bgr24->Data()), bgr24->Stride(),
                          avFrame->width, avFrame->height);
    } else {
      LOG(ERROR) << ">>>>>>>>>>>>>>>>>>> AVDecodingVideoFrame pixel format not support";
      return nullptr;
    }
  } else if (type == kRGB24) {
    auto rgb24 = std::dynamic_pointer_cast<AVRGB24BufferInterface>(buffer);
    if (avFrame->format == AV_PIX_FMT_YUVJ420P) {
      libyuv::J420ToRAW(avFrame->data[0], avFrame->linesize[0],
                        avFrame->data[1], avFrame->linesize[1],
                        avFrame->data[2], avFrame->linesize[2],
                        const_cast<uint8_t*>(rgb24->Data()), rgb24->Stride(),
                        avFrame->width, avFrame->height);
    } else if (avFrame->format == AV_PIX_FMT_YUV420P) {
      libyuv::I420ToRAW(avFrame->data[0], avFrame->linesize[0],
                        avFrame->data[1], avFrame->linesize[1],
                        avFrame->data[2], avFrame->linesize[2],
                        const_cast<uint8_t*>(rgb24->Data()), rgb24->Stride(),
                        avFrame->width, avFrame->height);
    } else {
      LOG(ERROR) << ">>>>>>>>>>>>>>>>>>> AVDecodingVideoFrame pixel format not support";
      return nullptr;
    }
  } else {
    /// TODO: fixme J420 ==> I420
    auto i420 = std::dynamic_pointer_cast<AVI420BufferInterface>(buffer);
    if (avFrame->format == AV_PIX_FMT_YUV420P || avFrame->format == AV_PIX_FMT_YUVJ420P) {
      libyuv::I420Copy(avFrame->data[0], avFrame->linesize[0],
                       avFrame->data[1], avFrame->linesize[1],
                       avFrame->data[2], avFrame->linesize[2],
                       const_cast<uint8_t*>(i420->DataY()), i420->StrideY(),
                       const_cast<uint8_t*>(i420->DataU()), i420->StrideU(),
                       const_cast<uint8_t*>(i420->DataV()), i420->StrideV(),
                       avFrame->width, avFrame->height);
    } else {
      return nullptr;
    }
  }

  return std::make_shared<AVDecodingVideoFrame>(buffer, timestamp, avFrame->pts, avFrame->key_frame);
}
}