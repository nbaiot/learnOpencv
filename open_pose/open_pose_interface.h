//
// Created by nbaiot@126.com on 2020/7/21.
//

#ifndef NBAIOT_OPEN_POSE_INTERFACE_H
#define NBAIOT_OPEN_POSE_INTERFACE_H

#include <tuple>
#include <vector>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>

namespace nbaiot {

class OpenPoseInterface {

public:
  enum PoseType{
    kCOCOBody,
    kMPIBody,
    kHand,
  };

  virtual ~OpenPoseInterface() = default;

  virtual std::tuple<std::vector<cv::Point>, int, int> Detect(cv::Mat img) = 0;

  virtual void Draw(cv::Mat img, const std::vector<cv::Point>& points, int w, int h) = 0;
};


}

#endif //NBAIOT_OPEN_POSE_INTERFACE_H
