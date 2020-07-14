//
// Created by nbaiot@126.com on 2020/6/4.
//

#ifndef NBAIOT_ELAPSED_TIME_H
#define NBAIOT_ELAPSED_TIME_H

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/timer/timer.hpp>

namespace nbaiot {

class ElapsedTime : public boost::noncopyable {
public:
  ElapsedTime() = default;

  uint64_t ElapsedWallMS() {
    return cpu_timer_.elapsed().wall / 1000000;
  }

  ~ElapsedTime() {
    cpu_timer_.stop();
  }

private:
  boost::timer::auto_cpu_timer cpu_timer_;
};

}


#endif //NBAIOT_ELAPSED_TIME_H
