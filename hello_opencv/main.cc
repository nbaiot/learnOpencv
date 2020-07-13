//
// Created by nbaiot@126.com on 2020/7/13.
//

#include <glog/logging.h>
#include<opencv2/core/version.hpp>

#ifdef USE_GPU
#include <opencv2/core/cuda.hpp>
#endif

int main() {
  LOG(INFO) << ">>>>>>>>>> hello opencv!!!";

  LOG(INFO) << "Major version : " << CV_MAJOR_VERSION;
  LOG(INFO) << "Minor version : " << CV_MINOR_VERSION;
  LOG(INFO) << "Subminor version : " << CV_SUBMINOR_VERSION;
  LOG(INFO) << "OpenCV version : " << CV_VERSION;


#ifdef USE_GPU
  auto gpuCount = cv::cuda::getCudaEnabledDeviceCount();
  for (int i = 0; i < gpuCount; ++i) {
    cv::cuda::printShortCudaDeviceInfo(i);
  }
#endif

  return 0;
}