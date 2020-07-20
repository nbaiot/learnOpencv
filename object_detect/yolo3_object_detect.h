//
// Created by nbaiot@126.com on 2020/7/20.
//

#ifndef NBAIOT_YOLO3_OBJECT_DETECT_H
#define NBAIOT_YOLO3_OBJECT_DETECT_H

#include <vector>
#include <string>
#include <opencv2/dnn/dnn.hpp>

namespace nbaiot {

class Yolo3ObjectDetect {

public:

  bool LoadModel(const std::string& cocoFile,
                 const std::string& configFile,
                 const std::string& weightFile,
                 bool tryUseGpu = true);

  void Detect(cv::Mat img, std::vector<cv::Mat>& outs);

  std::vector<std::tuple<std::string, float, cv::Rect>> Postprocess(cv::Mat& frame, const std::vector<cv::Mat>& outs);

  static void DrawPred(const std::string& classLabel, float conf, const cv::Rect& rect, cv::Mat& frame);

private:
  bool LoadClassesFile(const std::string& cocoFile);

  bool LoadYoloModel(const std::string& configFile, const std::string& weightFile, bool tryUseGpu);

  ///取得输出层的名字
  std::vector<cv::String> GetOutputNames(const cv::dnn::Net& net);

private:
  std::vector<std::string> classes_;
  float conf_threshold_{0.5};
  float nms_threshold_{0.4};
  int input_width_{416};
  int input_height_{416};
  cv::dnn::Net dnn_;
  std::vector<cv::String> output_layer_names_;
};

}

#endif //NBAIOT_YOLO3_OBJECT_DETECT_H
