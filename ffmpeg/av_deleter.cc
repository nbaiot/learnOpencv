//
// Created by nbaiot@126.com on 2020/7/2.
//

#include "av_deleter.h"

#include "ffmpeg.h"

namespace nbaiot {

namespace AVDeleter {


template<>
void AVDeleter<AVDictionary>::operator()(AVDictionary *objPtr) {
  if (objPtr) {
    av_dict_free(&objPtr);
  }
}

template<>
void AVDeleter<AVCodecContext>::operator()(AVCodecContext *objPtr) {
  if (objPtr) {
    avcodec_free_context(&objPtr);
  }
}

template<>
void AVDeleter<AVFormatContext>::operator()(AVFormatContext *objPtr) {
  if (objPtr) {
    avformat_close_input(&objPtr);
  }
}

template<>
void AVDeleter<AVPacket>::operator()(AVPacket *objPtr) {
  if (objPtr) {
    av_packet_free(&objPtr);
  }
}

template<>
void AVDeleter<AVFrame>::operator()(AVFrame *objPtr) {
  if (objPtr) {
    av_freep(&objPtr->data[0]);
    av_frame_free(&objPtr);
  }
}

}

}