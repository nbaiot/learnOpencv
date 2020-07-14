//
// Created by nbaiot@126.com on 2020/7/2.
//

#ifndef NBAIOT_AV_MUXER_H
#define NBAIOT_AV_MUXER_H

#include <memory>

#include "av_muxer_interface.h"

namespace nbaiot {

class AVMuxerImpl;

class AVMuxer : public AVMuxerInterface {

public:
  AVMuxer();

  ~AVMuxer() override;

  bool Open(std::string fileName, const std::string& mineType) override;

  bool Opened() override;

  void Close() override;

  std::shared_ptr<AVOutputStreamInterface> AddVideoStream(const VideoEncoder *encoder) override;

  bool WriteHeader() override;

  bool WritePacket(const std::shared_ptr<AVPacket>& packet) override;

  bool WriteTrailer() override;

  bool CanWritePacket() override;

  void SetTimeoutCallback(int timeoutMS, OnMuxerTimeoutCallback callback) override;

private:
  std::unique_ptr<AVMuxerImpl> impl_;
};

}

#endif //NBAIOT_AV_MUXER_H
