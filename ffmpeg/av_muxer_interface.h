//
// Created by nbaiot@126.com on 2020/7/2.
//

#ifndef NBAIOT_AV_MUXER_INTERFACE_H
#define NBAIOT_AV_MUXER_INTERFACE_H

#include <memory>
#include <string>
#include <functional>

struct AVPacket;

namespace nbaiot {

using OnMuxerTimeoutCallback = std::function<void()>;

class AVOutputStreamInterface;

class VideoEncoder;

class AVMuxerInterface {

public:
  virtual ~AVMuxerInterface() = default;

  virtual bool Open(std::string fileName, const std::string& mineType) = 0;

  virtual bool Opened() = 0;

  virtual void Close() = 0;

  virtual std::shared_ptr<AVOutputStreamInterface> AddVideoStream(const VideoEncoder* encoder) = 0;

  virtual bool  WriteHeader() = 0;

  virtual bool WritePacket(const std::shared_ptr<AVPacket>& packet) = 0;

  virtual bool WriteTrailer() = 0;

  virtual bool CanWritePacket() = 0;

  virtual void SetTimeoutCallback(int timeoutMS, OnMuxerTimeoutCallback callback) = 0;

};

}

#endif //NBAIOT_AV_MUXER_INTERFACE_H
