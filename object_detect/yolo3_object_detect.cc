//
// Created by nbaiot@126.com on 2020/7/20.
//

#include "yolo3_object_detect.h"

#include <fstream>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>

namespace nbaiot {

bool Yolo3ObjectDetect::LoadClassesFile(const std::string& cocoFile) {
  std::ifstream ifs(cocoFile);
  std::string line;
  while (getline(ifs, line)) {
    classes_.push_back(line);
  }
  return !classes_.empty();
}

bool Yolo3ObjectDetect::LoadYoloModel(const std::string& configFile, const std::string& weightFile, bool tryUseGpu) {
  output_layer_names_.clear();
  dnn_ = cv::dnn::readNetFromDarknet(configFile, weightFile);
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

bool
Yolo3ObjectDetect::LoadModel(const std::string& cocoFile, const std::string& configFile, const std::string& weightFile,
                             bool tryUseGpu) {
  return LoadClassesFile(cocoFile) && LoadYoloModel(configFile, weightFile, tryUseGpu);
}

void Yolo3ObjectDetect::Detect(cv::Mat img, std::vector<cv::Mat>& outs) {
  if (dnn_.empty())
    return;
  auto blob = cv::dnn::blobFromImage(img, 1 / 255.0, cv::Size(input_width_, input_height_));
  dnn_.setInput(blob);
  dnn_.forward(outs, GetOutputNames(dnn_));
}

std::vector<std::tuple<std::string, float, cv::Rect>> Yolo3ObjectDetect::Postprocess(cv::Mat& frame, const std::vector<cv::Mat>& outs) {
  /// 储存识别类的索引
  std::vector<int> classIds;
  /// 储存置信度
  std::vector<float> confidences;
  /// 储存边框
  std::vector<cv::Rect> boxes;

  for (const auto& out : outs) {
    /// 从网络输出中扫描所有边界框
    /// 保留高置信度选框
    /// 目标数据data:x,y,w,h为百分比，x,y为目标中心点坐标
    auto data = (float*) out.data;
    for (int j = 0; j < out.rows; j++, data += out.cols) {
      cv::Mat scores = out.row(j).colRange(5, out.cols);
      cv::Point classIdPoint;
      ///置信度
      double confidence;
      ///取得最大分数值与索引
      cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
      if (confidence > conf_threshold_) {
        int centerX = (int) (data[0] * frame.cols);
        int centerY = (int) (data[1] * frame.rows);
        int width = (int) (data[2] * frame.cols);
        int height = (int) (data[3] * frame.rows);
        int left = centerX - width / 2;
        int top = centerY - height / 2;

        classIds.push_back(classIdPoint.x);
        confidences.push_back((float) confidence);
        boxes.emplace_back(left, top, width, height);
      }

    }

  }

  /// 保存没有重叠边框的索引
  std::vector<int> indices;
  /// 该函数用于抑制重叠边框
  cv::dnn::NMSBoxes(boxes, confidences, conf_threshold_, nms_threshold_, indices);
  std::vector<std::tuple<std::string, float, cv::Rect>> results;
  results.reserve(boxes.size());
  for (int & indice : indices) {
    results.emplace_back(classes_[classIds[indice]], confidences[indice], boxes[indice]);
  }
  return results;
}

void Yolo3ObjectDetect::DrawPred(const std::string& classLabel, float conf, const cv::Rect& rect, cv::Mat& frame) {
  ///绘制边界框
  cv::rectangle(frame, rect.tl(), rect.br(), cv::Scalar(255, 0, 0), 3);
  std::string label = cv::format("%.2f", conf);
  ///边框上的类别标签与置信度
  label = classLabel + ":" + label;

  /// 绘制边界框上的标签
  int baseLine;
  cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
  auto top = std::max(rect.y, labelSize.height);
  cv::rectangle(frame, cv::Point(rect.x, top - round(1.5 * labelSize.height)),
                cv::Point(rect.x + round(1.5 * labelSize.width), top + baseLine), cv::Scalar(255, 255, 255), cv::FILLED);
  cv::putText(frame, label, cv::Point(rect.x, top), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0), 1);
}

std::vector<cv::String> Yolo3ObjectDetect::GetOutputNames(const cv::dnn::Net& net) {
  /// 只获取一次
  if (output_layer_names_.empty()) {
    /// 取得输出层指标
    std::vector<int> outLayers = net.getUnconnectedOutLayers();
    std::vector<cv::String> layersNames = net.getLayerNames();
    /// 取得输出层名字
    output_layer_names_.resize(outLayers.size());
    for (size_t i = 0; i < outLayers.size(); i++) {
      output_layer_names_[i] = layersNames[outLayers[i] - 1];
    }
  }
  return output_layer_names_;
}


}