//
// Created by nbaiot@126.com on 2020/7/2.
//

#ifndef NBAIOT_AV_DELETER_H
#define NBAIOT_AV_DELETER_H

struct AVDictionary;
struct AVCodecContext;
struct AVFormatContext;
struct AVPacket;
struct AVFrame;

namespace nbaiot {

namespace AVDeleter {

template<typename AVObj>
class AVDeleter {
public:
  void operator()(AVObj *objPtr);
};

using AVDictionaryDeleter = AVDeleter<AVDictionary>;

using AVCodecContextDeleter = AVDeleter<AVCodecContext>;

using AVFormatContextDeleter = AVDeleter<AVFormatContext>;

using AVPacketDeleter = AVDeleter<AVPacket>;

using AVFrameDeleter = AVDeleter<AVFrame>;

}

}

#endif //NBAIOT_AV_DELETER_H
