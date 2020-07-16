//
// Created by nbaiot@126.com on 2020/7/16.
//

#ifndef NBAIOT_FACE_DETECT_INTERFACE_H
#define NBAIOT_FACE_DETECT_INTERFACE_H

#include <vector>
#include <opencv2/core/mat.hpp>

namespace nbaiot {

class FaceDetectInterface {

public:
  virtual ~FaceDetectInterface() = default;

  virtual bool Detect(cv::InputArray img, std::vector<cv::Rect>& faces) = 0;

};

}


#endif //NBAIOT_FACE_DETECT_INTERFACE_H
