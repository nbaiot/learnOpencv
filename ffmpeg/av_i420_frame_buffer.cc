//
// Created by nbaiot@126.com on 2020/7/9.
//

#include "av_i420_frame_buffer.h"

#include <libyuv.h>
#include <glog/logging.h>

#include "ffmpeg.h"
#include "av_bgr24_frame_buffer.h"
#include "av_rgb24_frame_buffer.h"

namespace nbaiot {

class AVI420FrameBufferImpl : public AVI420BufferInterface {

public:
  AVI420FrameBufferImpl(int w, int h, bool isJ420p) : is_j420p_(isJ420p) {

    av_frame_ = std::shared_ptr<AVFrame>(av_frame_alloc(), [](AVFrame* frame) {
        if (frame) {
          av_freep(&frame->data[0]);
          av_frame_free(&frame);
        }
    });

    if (!is_j420p_) {
      av_frame_->format = AV_PIX_FMT_YUV420P;
    } else {
      av_frame_->format = AV_PIX_FMT_YUVJ420P;
    }


    av_frame_->width = w;
    av_frame_->height = h;

    size_ = av_image_alloc(av_frame_->data, av_frame_->linesize,
                           av_frame_->width, av_frame_->height, static_cast<AVPixelFormat>(av_frame_->format), 1);
  }

  ~AVI420FrameBufferImpl() override = default;

  int32_t Width() override {
    return av_frame_->width;
  }

  int32_t Height() override {
    return av_frame_->height;
  }

  VideoFrameType Type() override {
    return kYUV_I420;
  }

  int32_t Size() override {
    return size_;
  }

  void* RawAVPtr() override {
    return av_frame_.get();
  }

  std::shared_ptr<AVI420BufferInterface> ToI420() override {
    return std::dynamic_pointer_cast<AVI420BufferInterface>(shared_from_this());
  }

  std::shared_ptr<AVBGR24BufferInterface> ToBGR24() override {
    auto bgr24 = AVBGR24FrameBuffer::Create(Width(), Height());
    if (!is_j420p_) {
      libyuv::I420ToRGB24(DataY(), StrideY(),
                          DataU(), StrideU(),
                          DataV(), StrideV(),
                          const_cast<uint8_t*>(bgr24->Data()), bgr24->Stride(),
                          Width(), Height());
    } else {
      libyuv::J420ToRGB24(DataY(), StrideY(),
                          DataU(), StrideU(),
                          DataV(), StrideV(),
                          const_cast<uint8_t*>(bgr24->Data()), bgr24->Stride(),
                          Width(), Height());
    }

    return bgr24;
  }

  std::shared_ptr<AVRGB24BufferInterface> ToRGB24() override {
    auto rgb24 = AVRGB24FrameBuffer::Create(Width(), Height());
    if (!is_j420p_) {
      libyuv::I420ToRAW(DataY(), StrideY(),
                        DataU(), StrideU(),
                        DataV(), StrideV(),
                        const_cast<uint8_t*>(rgb24->Data()), rgb24->Stride(),
                        Width(), Height());
    } else {
      libyuv::J420ToRAW(DataY(), StrideY(),
                        DataU(), StrideU(),
                        DataV(), StrideV(),
                        const_cast<uint8_t*>(rgb24->Data()), rgb24->Stride(),
                        Width(), Height());
    }
    return rgb24;
  }

  int StrideY() const override {
    return av_frame_->linesize[0];
  }

  int StrideU() const override {
    return av_frame_->linesize[1];
  }

  int StrideV() const override {
    return av_frame_->linesize[2];
  }

  const uint8_t* DataY() const override {
    return av_frame_->data[0];
  }

  const uint8_t* DataU() const override {
    return av_frame_->data[1];
  }

  const uint8_t* DataV() const override {
    return av_frame_->data[2];
  }

private:
  int size_;
  bool is_j420p_;
  std::shared_ptr<AVFrame> av_frame_;
};

std::shared_ptr<AVI420FrameBuffer> AVI420FrameBuffer::Create(int w, int h, bool isJ420p) {
  return std::make_shared<AVI420FrameBuffer>(w, h, isJ420p);
}


std::shared_ptr<AVI420FrameBuffer>
AVI420FrameBuffer::Copy(int w, int h, const uint8_t* dataY, int strideY, const uint8_t* dataU, int strideU,
                        const uint8_t* dataV, int strideV) {
  auto buffer = Create(w, h);
  libyuv::I420Copy(dataY, strideY,
                   dataU, strideU,
                   dataV, strideV,
                   const_cast<uint8_t*>(buffer->DataY()), buffer->StrideY(),
                   const_cast<uint8_t*>(buffer->DataU()), buffer->StrideU(),
                   const_cast<uint8_t*>(buffer->DataV()), buffer->StrideV(),
                   w, h);
  return buffer;
}

AVI420FrameBuffer::~AVI420FrameBuffer() = default;

AVI420FrameBuffer::AVI420FrameBuffer(int w, int h, bool isJ420p) {
  impl_ = std::make_shared<AVI420FrameBufferImpl>(w, h, isJ420p);
}


int32_t AVI420FrameBuffer::Width() {
  return impl_->Width();
}

int32_t AVI420FrameBuffer::Height() {
  return impl_->Height();
}

VideoFrameType AVI420FrameBuffer::Type() {
  return impl_->Type();
}

int32_t AVI420FrameBuffer::Size() {
  return impl_->Size();
}

void* AVI420FrameBuffer::RawAVPtr() {
  return impl_->RawAVPtr();
}

std::shared_ptr<AVI420BufferInterface> AVI420FrameBuffer::ToI420() {
  return impl_->ToI420();
}

std::shared_ptr<AVBGR24BufferInterface> AVI420FrameBuffer::ToBGR24() {
  return impl_->ToBGR24();
}

std::shared_ptr<AVRGB24BufferInterface> AVI420FrameBuffer::ToRGB24() {
  return impl_->ToRGB24();
}

int AVI420FrameBuffer::StrideY() const {
  return impl_->StrideY();
}

int AVI420FrameBuffer::StrideU() const {
  return impl_->StrideU();
}

int AVI420FrameBuffer::StrideV() const {
  return impl_->StrideV();
}

const uint8_t* AVI420FrameBuffer::DataY() const {
  return impl_->DataY();
}

const uint8_t* AVI420FrameBuffer::DataU() const {
  return impl_->DataU();
}

const uint8_t* AVI420FrameBuffer::DataV() const {
  return impl_->DataV();
}
}