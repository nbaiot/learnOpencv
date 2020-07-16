//
// Created by nbaiot@126.com on 2020/7/16.
//

#include "rgb24_video_preview_widget.h"
#include "ffmpeg/av_video_frame_interface.h"
#include "ffmpeg/av_video_frame_buffer_interface.h"

#include <cstring>
#include <QVideoSurfaceFormat>
#include <QAbstractVideoSurface>
#include <glog/logging.h>

namespace nbaiot {

RGB24VideoPreviewWidget::RGB24VideoPreviewWidget(int w, int h, QWidget* parent)
    : QVideoWidget(parent), width_(w), height_(h) {

  QVideoSurfaceFormat format(QSize(width_, height_), QVideoFrame::PixelFormat::Format_RGB24);

  videoSurface()->start(format);

  setMinimumWidth(width_);

  setMinimumHeight(height_);

}

RGB24VideoPreviewWidget::~RGB24VideoPreviewWidget() {
  LOG(INFO) << ">>>>>>>>>>~~~~~VideoPreviewWidget";
}

void RGB24VideoPreviewWidget::FeedFrame(const std::shared_ptr<AVVideoFrameInterface>& frame) {
  if (frame->FrameBuffer()->Height() != height_ || frame->FrameBuffer()->Width() != width_)
    return;

  QVideoFrame videoFrame(width_ * height_ * 3, QSize(width_, height_), width_ * 3, QVideoFrame::PixelFormat::Format_RGB24);
  if (videoFrame.map(QAbstractVideoBuffer::ReadOnly)) {
    auto rgb24 = frame->FrameBuffer()->ToRGB24();
    std::memcpy(videoFrame.bits(), rgb24->Data(), rgb24->Size());
    videoFrame.unmap();
  }
  videoSurface()->present(videoFrame);
}

void RGB24VideoPreviewWidget::FeedMat(const cv::Mat& mat) {
  if (mat.rows != height_ || mat.cols != width_)
    return;
  QVideoFrame videoFrame(width_ * height_ * 3, QSize(width_, height_), width_ * 3, QVideoFrame::PixelFormat::Format_RGB24);
  if (videoFrame.map(QAbstractVideoBuffer::ReadOnly)) {
    std::memcpy(videoFrame.bits(), mat.data, width_ * height_ * 3);
    videoFrame.unmap();
  }
  videoSurface()->present(videoFrame);
}

}