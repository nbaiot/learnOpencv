//
// Created by nbaiot@126.com on 2020/7/3.
//

#ifndef NBAIOT_AV_DEMUXER_H
#define NBAIOT_AV_DEMUXER_H

#include "av_demuxer_interface.h"

namespace nbaiot {

class AVDemuxerImpl;

class AVDemuxer : public AVDemuxerInterface {

public:
  AVDemuxer();

  ~AVDemuxer() override;

  bool Open(std::string url) override;

  bool Open(std::string url, AVInputFormat* format, std::shared_ptr<AVDictionary> opt) override;

  void Close() override;

  bool Opened() override;

  void EnableReadPacketRateByFPS(bool enable) override;

  void SetTimeoutCallback(int timeoutMS, OnDemuxerTimeoutCallback callback) override;

  void SetInputStreamCallback(OnDemuxerInputStreamCallback callback) override;

  void SetEOFCallback(OnDemuxerEOFCallback callback) override;

  void SetConnectionRefuseCallback(OnDemuxerEOFCallback callback) override;

private:
  std::unique_ptr<AVDemuxerImpl> impl_;
};

}

#endif //NBAIOT_AV_DEMUXER_H
