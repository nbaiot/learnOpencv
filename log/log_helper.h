//
// Created by nbaiot@126.com on 2020/6/3.
//

#ifndef NBAIOT_LOG_HELPER_H
#define NBAIOT_LOG_HELPER_H

#include <glog/logging.h>

namespace nbaiot {

void SignalHandle(const char *data, int size) {
  std::string error = std::string(data, size);
  LOG(ERROR) << error;
}

class LogHelper {

public:
  static LogHelper *Instance() {
    static LogHelper INSTANCE;
    return &INSTANCE;
  }

  void Init(const char *program, const char *logDir) {
    if (init_)
      return;

    /// 初始化应用进程名
    google::InitGoogleLogging(program);

    /// 设置错误级别大于等于多少时输出到文件
    /// 参数2为日志存放目录和日志文件前缀
    google::SetLogDestination(google::WARNING, logDir);

    /// 是否将日志输出到标准错误是不是日志文件
    FLAGS_logtostderr = false;

    /// 是否同时将日志发送到标准错误和日志文件中
    FLAGS_alsologtostderr = false;

    /// 当日志级别大于此级别时，自动将此日志输出到标准错误中
    FLAGS_stderrthreshold = google::INFO;

    /// 当日志级别大于此级别时会马上输出，而不缓存
    FLAGS_logbuflevel = google::INFO;

    /// 缓存最久长时间为多久
    /// 0 则为实时输出
    FLAGS_logbufsecs = 0;

    /// 当日志文件达到多少时，进行转存，以M为单位
    FLAGS_max_log_size = 10;

    /// 当磁盘已满时,停止输出日志文件
    FLAGS_stop_logging_if_full_disk = true;

    /// 安装异常处理函数
    google::InstallFailureSignalHandler();

    google::InstallFailureWriter(&SignalHandle);

    init_ = true;
  }

  void Dispose() {
    google::ShutdownGoogleLogging();
    init_ = false;
  }

private:
  LogHelper() = default;

  bool init_{false};

};

}


#endif //NBAIOT_LOG_HELPER_H
