//
// Created by nbaiot@126.com on 2020/7/16.
//

#include "dnn_tf_face_detect.h"

#include <opencv2/core/cuda.hpp>
#include <glog/logging.h>

namespace nbaiot {

DNNTFFaceDetect::DNNTFFaceDetect(const std::string& mode, const std::string& config, bool tryUseGpu) {
  if (cv::cuda::getCudaEnabledDeviceCount() == 0) {
    tryUseGpu = false;
  }

  dnn_ = cv::dnn::readNetFromTensorflow(mode, config);

  if (dnn_.empty())
    return;

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
}

DNNTFFaceDetect::~DNNTFFaceDetect() {

}

bool DNNTFFaceDetect::Detect(cv::InputArray img, std::vector<cv::Rect>& faces) {
  if (dnn_.empty())
    return false;

  if (!img.isMat())
    return false;

  auto mat = img.getMat();

  if (mat.channels() != 3)
    return false;

  cv::Mat inputBlob = cv::dnn::blobFromImage(mat, 0.5,
                                             cv::Size(300, 300), cv::Scalar(0, 0, 255), false, false);

  dnn_.setInput(inputBlob, "data");

  cv::Mat detection = dnn_.forward("detection_out");

  cv::Mat detectionMat(detection.size[2], detection.size[3],
                       CV_32F, detection.ptr<float>());

  faces.clear();
  faces.reserve(detectionMat.rows);
  for (int i = 0; i < detectionMat.rows; i++) {
    ///置值度获取
    float confidence = detectionMat.at<float>(i, 2);
    ///如果大于阈值说明检测到人脸
    if (confidence > 0.3) {
      ///计算矩形
      int xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * mat.cols);
      int yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * mat.rows);
      int xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * mat.cols);
      int yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * mat.rows);
      ///生成矩形
      cv::Rect rect((int) xLeftBottom, (int) yLeftBottom,
                    (int) (xRightTop - xLeftBottom),
                    (int) (yRightTop - yLeftBottom));
      faces.push_back(rect);

    }

  }
  return true;
}
}