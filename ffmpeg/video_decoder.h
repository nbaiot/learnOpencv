//
// Created by nbaiot@126.com on 2020/7/1.
//

#ifndef NBAIOT_VIDEO_DECODER_H
#define NBAIOT_VIDEO_DECODER_H

#include <memory>
#include <string>
#include <functional>

#include "av_defines.h"

struct AVCodecParameters;

struct AVFrame;

struct AVPacket;

struct AVCodec;

namespace nbaiot {

class VideoDecoderImpl;

class AVDecodingVideoFrame;

using OnVideoDecodeCallback = std::function<void(const std::shared_ptr<AVFrame>& frame)>;

using OnVideoDecodingFrameCallback = std::function<void(const std::shared_ptr<AVDecodingVideoFrame>& frame)>;

class VideoDecoder {

public:
  VideoDecoder();

  ~VideoDecoder();

  bool Open(AVCodec * codec, AVCodecParameters* params, int decodeThreadCount = 4);

  void Close();

  bool Opened() const;

  bool SetOption(const std::string& key, const std::string& val);

  std::string CodecName();

  void* RawAVPtr();

  void* RawAVPtr() const;

  void SetDecodingFrameType(VideoFrameType decodingFrameType);

  void SetDecodeCallback(OnVideoDecodeCallback callback);

  void SetDecodingFrameCallback(OnVideoDecodingFrameCallback callback);

  void Decode(const std::shared_ptr<AVPacket>& pkt);

private:
  std::unique_ptr<VideoDecoderImpl> impl_;
};

}

#endif //NBAIOT_VIDEO_DECODER_H
