//
// Created by nbaiot@126.com on 2020/7/16.
//

#include "haar_face_detect.h"

#include <opencv2/objdetect.hpp>

namespace nbaiot {

HaarFaceDetect::HaarFaceDetect(const std::string& modelXml) {
  detector_ = std::make_unique<cv::CascadeClassifier>(modelXml);
}

HaarFaceDetect::~HaarFaceDetect() = default;

bool HaarFaceDetect::Detect(cv::InputArray img, std::vector<cv::Rect>& faces) {
  if (!img.isMat())
    return false;

  auto mat = img.getMat();

  if (mat.channels() != 3)
    return false;

  faces.clear();
  detector_->detectMultiScale(img, faces);


  return true;
}


}