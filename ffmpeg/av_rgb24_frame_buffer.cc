//
// Created by nbaiot@126.com on 2020/7/9.
//

#include "av_rgb24_frame_buffer.h"

#include <libyuv.h>

#include "ffmpeg.h"
#include "av_i420_frame_buffer.h"
#include "av_bgr24_frame_buffer.h"

namespace nbaiot {

class AVRGB24FrameBufferImpl : public AVRGB24BufferInterface {
public:
  AVRGB24FrameBufferImpl(int w, int h) {
    av_frame_ = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* frame) {
        if (frame) {
          av_freep(&frame->data[0]);
          av_frame_free(&frame);
        }
    });

    av_frame_->format = AV_PIX_FMT_RGB24;
    av_frame_->width = w;
    av_frame_->height = h;

    size_ = av_image_alloc(av_frame_->data, av_frame_->linesize,
                           av_frame_->width, av_frame_->height, AV_PIX_FMT_RGB24, 1);
  }

  ~AVRGB24FrameBufferImpl() override = default;

  int32_t Width() override {
    return av_frame_->width;
  }

  int32_t Height() override {
    return av_frame_->height;
  }

  VideoFrameType Type() override {
    return kRGB24;
  }

  int32_t Size() override {
    return size_;
  }

  void* RawAVPtr() override {
    return av_frame_.get();
  }

  std::shared_ptr<AVI420BufferInterface> ToI420() override {
    auto i420 = AVI420FrameBuffer::Create(Width(), Height());

    /// libyuv raw is RGB
    libyuv::RAWToI420(Data(), Stride(),
                        const_cast<uint8_t*>(i420->DataY()), i420->StrideY(),
                        const_cast<uint8_t*>(i420->DataU()), i420->StrideU(),
                        const_cast<uint8_t*>(i420->DataV()), i420->StrideV(),
                        Width(), Height());

    return i420;
  }

  std::shared_ptr<AVBGR24BufferInterface> ToBGR24() override {
    auto bgr24 = AVBGR24FrameBuffer::Create(Width(), Height());
    libyuv::RAWToRGB24(Data(), Stride(),
                       const_cast<uint8_t*>(bgr24->Data()), bgr24->Stride(),
                       Width(), Height());
    return bgr24;
  }

  std::shared_ptr<AVRGB24BufferInterface> ToRGB24() override {
    return std::dynamic_pointer_cast<AVRGB24BufferInterface>(shared_from_this());
  }

  const uint8_t* Data() const override {
    return av_frame_->data[0];
  }

  int Stride() const override {
    return av_frame_->linesize[0];
  }

private:
  int size_;
  std::shared_ptr<AVFrame> av_frame_;
};

std::shared_ptr<AVRGB24FrameBuffer> AVRGB24FrameBuffer::Create(int w, int h) {
  return std::make_shared<AVRGB24FrameBuffer>(w, h);
}

AVRGB24FrameBuffer::~AVRGB24FrameBuffer() = default;

AVRGB24FrameBuffer::AVRGB24FrameBuffer(int w, int h) {
  impl_ = std::make_shared<AVRGB24FrameBufferImpl>(w, h);
}

int32_t AVRGB24FrameBuffer::Width() {
  return impl_->Width();
}

int32_t AVRGB24FrameBuffer::Height() {
  return impl_->Height();
}

VideoFrameType AVRGB24FrameBuffer::Type() {
  return kRGB24;
}

int32_t AVRGB24FrameBuffer::Size() {
  return impl_->Size();
}

void* AVRGB24FrameBuffer::RawAVPtr() {
  return impl_->RawAVPtr();
}

std::shared_ptr<AVI420BufferInterface> AVRGB24FrameBuffer::ToI420() {
  return impl_->ToI420();
}

std::shared_ptr<AVBGR24BufferInterface> AVRGB24FrameBuffer::ToBGR24() {
  return impl_->ToBGR24();
}

std::shared_ptr<AVRGB24BufferInterface> AVRGB24FrameBuffer::ToRGB24() {
  return impl_->ToRGB24();
}

const uint8_t* AVRGB24FrameBuffer::Data() const {
  return impl_->Data();
}

int AVRGB24FrameBuffer::Stride() const {
  return impl_->Stride();
}
}