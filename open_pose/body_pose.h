//
// Created by nbaiot@126.com on 2020/7/21.
//

#ifndef NBAIOT_BODY_POSE_H
#define NBAIOT_BODY_POSE_H

#include <string>
#include <opencv2/dnn/dnn.hpp>

#include "open_pose_interface.h"

namespace nbaiot {


class BodyPose : public OpenPoseInterface {
public:
  BodyPose(const std::string& model, const std::string& cfg, PoseType type, bool tryUseGpu = true);

  ~BodyPose() override;

  std::tuple<std::vector<cv::Point>, int, int> Detect(cv::Mat img) override;

  void Draw(cv::Mat img, const std::vector<cv::Point>& points, int w, int h) override;

private:
  bool LoadModel(const std::string& model, const std::string& cfg, bool tryUseGpu);

private:
  cv::dnn::Net dnn_;
  int pairs_;
  int parts_;
  int index_;
};

}



#endif //NBAIOT_BODY_POSE_H
