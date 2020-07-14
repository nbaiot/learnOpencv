//
// Created by nbaiot@126.com on 2020/7/3.
//

#ifndef NBAIOT_AV_DEMUXER_INTERFACE_H
#define NBAIOT_AV_DEMUXER_INTERFACE_H

#include <memory>
#include <string>
#include <functional>

struct AVDictionary;

struct AVInputFormat;

namespace nbaiot {

class AVInputStreamInterface;

using OnDemuxerTimeoutCallback = std::function<void()>;

using OnDemuxerInputStreamCallback = std::function<void(const std::shared_ptr<AVInputStreamInterface>& stream)>;

using OnDemuxerEOFCallback = std::function<void(const std::string& url)>;

using OnDemuxerConnectionRefuseCallback = std::function<void(const std::string& url)>;

class AVDemuxerInterface {

public:
  virtual ~AVDemuxerInterface() = default;

  virtual bool Open(std::string url) = 0;

  virtual bool Open(std::string url, AVInputFormat* format, std::shared_ptr<AVDictionary> opt) = 0;

  virtual void Close() = 0;

  virtual bool Opened() = 0;

  virtual void EnableReadPacketRateByFPS(bool enable) = 0;

  virtual void SetTimeoutCallback(int timeoutMS, OnDemuxerTimeoutCallback callback) = 0;

  virtual void SetInputStreamCallback(OnDemuxerInputStreamCallback callback) = 0;

  virtual void SetEOFCallback(OnDemuxerEOFCallback callback) = 0;

  virtual void SetConnectionRefuseCallback(OnDemuxerEOFCallback callback) = 0;

};

}

#endif //NBAIOT_AV_DEMUXER_INTERFACE_H
