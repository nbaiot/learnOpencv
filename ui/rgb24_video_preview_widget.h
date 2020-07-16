//
// Created by nbaiot@126.com on 2020/7/16.
//

#ifndef NBAIOT_VIDEO_PREVIEW_H
#define NBAIOT_VIDEO_PREVIEW_H

#include <QVideoWidget>
#include <QtMultimedia/QVideoFrame>
#include <opencv2/core/mat.hpp>

#include "video/video_stream_sink_interface.h"

namespace nbaiot {

class RGB24VideoPreviewWidget
    : public QVideoWidget {

public:
  RGB24VideoPreviewWidget(int w, int h, QWidget* parent = nullptr);

  ~RGB24VideoPreviewWidget() override;

  void FeedFrame(const std::shared_ptr<AVVideoFrameInterface>& frame);

  void FeedMat(const cv::Mat& mat);

private:
  int width_{-1};
  int height_{-1};

};

}

#endif //NBAIOT_VIDEO_PREVIEW_H
