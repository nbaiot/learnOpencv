//
// Created by nbaiot@126.com on 2020/7/16.
//

#ifndef NBAIOT_DNN_TF_FACE_DETECT_H
#define NBAIOT_DNN_TF_FACE_DETECT_H


#include <string>
#include <memory>
#include <opencv2/dnn/dnn.hpp>
#include "face_detect_interface.h"


namespace nbaiot {

class DNNTFFaceDetect : public FaceDetectInterface {

public:
  DNNTFFaceDetect(const std::string& mode, const std::string& config, bool tryUseGpu = true);

  ~DNNTFFaceDetect() override;

  bool Detect(cv::InputArray img, std::vector<cv::Rect>& faces) override;

private:
  cv::dnn::Net dnn_;
};


}


#endif //NBAIOT_DNN_TF_FACE_DETECT_H
