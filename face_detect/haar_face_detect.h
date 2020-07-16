//
// Created by nbaiot@126.com on 2020/7/16.
//

#ifndef NBAIOT_HAAR_FACE_DETECT_H
#define NBAIOT_HAAR_FACE_DETECT_H

#include <string>
#include <memory>
#include "face_detect_interface.h"

namespace cv {

class CascadeClassifier;

}

namespace nbaiot {

class HaarFaceDetect
    : public FaceDetectInterface {

public:
  explicit HaarFaceDetect(const std::string& modelXml);

  ~HaarFaceDetect() override;

  bool Detect(cv::InputArray img, std::vector<cv::Rect>& faces) override;

private:
  std::unique_ptr<cv::CascadeClassifier> detector_;
};

}

#endif //NBAIOT_HAAR_FACE_DETECT_H
