//
// Created by nbaiot@126.com on 2020/6/3.
//

#ifndef NBAIOT_AV_CODEC_GLOBAL_MUTEX_H
#define NBAIOT_AV_CODEC_GLOBAL_MUTEX_H

#include <mutex>

namespace nbaiot::CodecMutex {

extern std::mutex global_codec_open_mutex;

}


#endif //NBAIOT_AV_CODEC_GLOBAL_MUTEX_H
