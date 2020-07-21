//
// Created by nbaiot@126.com on 2020/7/21.
//

#include "body_pose.h"
#include <opencv2/imgproc.hpp>

namespace nbaiot {

const int POSE_PAIRS[3][20][2] = {
    {   /// COCO body
        {1, 2}, {1, 5}, {2, 3},
        {3, 4}, {5, 6}, {6, 7},
        {1, 8}, {8, 9}, {9, 10},
        {1, 11}, {11, 12}, {12, 13},
        {1, 0},  {0, 14},
        {14, 16}, {0, 15}, {15, 17}
    },
    {   /// MPI body
        {0, 1}, {1, 2}, {2, 3},
        {3, 4}, {1, 5}, {5, 6},
        {6, 7}, {1, 14}, {14, 8}, {8, 9},
        {9, 10}, {14, 11}, {11, 12}, {12, 13}
    },
    {   /// hand
        {0, 1}, {1, 2}, {2, 3}, {3, 4},         /// thumb
        {0, 5}, {5, 6}, {6, 7}, {7, 8},         /// pinkie
        {0, 9}, {9, 10}, {10, 11}, {11, 12},    /// middle
        {0, 13}, {13, 14}, {14, 15}, {15, 16},  /// ring
        {0, 17}, {17, 18}, {18, 19}, {19, 20}   /// small
    }
};


BodyPose::BodyPose(const std::string& model, const std::string& cfg, PoseType type, bool tryUseGpu) {
  LoadModel(model, cfg, tryUseGpu);
  if (type == kCOCOBody) {
    pairs_ = 17;
    parts_ = 18;
    index_ = 0;
  } else if (type == kMPIBody) {
    pairs_ = 14;
    parts_ = 16;
    index_ = 1;
  } else if (type == kHand) {
    pairs_ = 20;
    parts_ = 22;
    index_ = 2;
  }

}

BodyPose::~BodyPose() = default;

bool BodyPose::LoadModel(const std::string& model, const std::string& cfg, bool tryUseGpu) {
  dnn_ = cv::dnn::readNet(model, cfg);
  if (dnn_.empty())
    return false;
  if (tryUseGpu) {
    dnn_.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    dnn_.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
  } else {
    dnn_.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
    auto targets = cv::dnn::getAvailableTargets(cv::dnn::DNN_BACKEND_OPENCV);
    if (std::find(targets.begin(), targets.end(), cv::dnn::DNN_TARGET_OPENCL) != targets.end()) {
      dnn_.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL);
    } else {
      dnn_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }
  }
  return true;
}

std::tuple<std::vector<cv::Point>, int, int> BodyPose::Detect(cv::Mat img) {
  auto blob = cv::dnn::blobFromImage(img, 1 / 255.0, cv::Size(368, 368), cv::Scalar(0, 0, 0), false, false);
  dnn_.setInput(blob);
  auto result = dnn_.forward();
  int H = result.size[2];
  int W = result.size[3];
  std::vector<cv::Point> points(22);
  /// coco 18
  for (int n = 0; n < parts_; n++) {
    cv::Mat heatMap(H, W, CV_32F, result.ptr(0, n));
    cv::Point p(-1, -1), pm;
    double conf;
    cv::minMaxLoc(heatMap, 0, &conf, 0, &pm);
    if (conf > 0.1) {
      p = pm;
    }
    points[n] = p;
  }

  return std::tuple<std::vector<cv::Point>, int, int>(points, W, H);
}

void BodyPose::Draw(cv::Mat img, const std::vector<cv::Point>& points, int w, int h) {
  if (points.empty())
    return;
  float SX = float(img.cols) / w;
  float SY = float(img.rows) / h;
  /// coco 18
  for (int n = 0; n < pairs_; n++) {
    cv::Point2f a = points[POSE_PAIRS[index_][n][0]];
    cv::Point2f b = points[POSE_PAIRS[index_][n][1]];

    if (a.x <= 0 || a.y <= 0 || b.x <= 0 || b.y <= 0) {
      continue;
    }

    a.x *= SX;
    a.y *= SY;
    b.x *= SX;
    b.y *= SY;

    cv::line(img, a, b, cv::Scalar(0, 200, 0), 2);
    cv::circle(img, a, 3, cv::Scalar(0, 0, 200), -1);
    cv::circle(img, b, 3, cv::Scalar(0, 0, 200), -1);
  }

}

}