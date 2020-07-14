//
// Created by nbaiot@126.com on 2020/7/3.
//

#include "av_demuxer.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include <glog/logging.h>
#include <utils/fps.h>
#include <container/safe_map.h>

#include "av_deleter.h"
#include "av_utils.h"
#include "input_video_stream.h"
#include "ffmpeg.h"

namespace nbaiot {

class AVDemuxerImpl : public AVDemuxerInterface {

public:
  AVDemuxerImpl() = default;

  ~AVDemuxerImpl() override {
    Close();
  }

  bool Open(std::string url) override {
    return Open(std::move(url), nullptr, nullptr);
  }

  bool Open(std::string url, AVInputFormat* format, std::shared_ptr<AVDictionary> opt) override {
    if (Opened()) {
      LOG(WARNING) << ">>>>>>>>>>>AVDemuxer already opened:" << url_;
      return true;
    }

    auto rawPtr = avformat_alloc_context();
    if (rawPtr) {
      rawPtr->interrupt_callback.opaque = this;
      rawPtr->interrupt_callback.callback = AVDemuxerImpl::AVInterruptCallback;
    } else {
      LOG(ERROR) << ">>>>>>>>>>>AVDemuxer alloc context failed";
      return false;
    }

    url_ = std::move(url);

    last_socket_access_ = std::chrono::system_clock::now();
    int ret = 0;
    if (opt) {
      AVDictionary *dict = nullptr;
      av_dict_copy(&dict, opt.get(), 0);
      ret = avformat_open_input(&rawPtr, url_.c_str(), format, &dict);
    } else {
      ret = avformat_open_input(&rawPtr, url_.c_str(), format, nullptr);
    }

    if (ret != 0) {
      LOG(ERROR) << "AVDemuxer open:" << url_ << " failed," << av_err2str(ret);
      if (ret == ECONNREFUSED && refuse_callback_) {
        refuse_callback_(url_);
      }
      return false;
    }

    format_context_ = std::shared_ptr<AVFormatContext>(rawPtr, AVDeleter::AVFormatContextDeleter());

    opened_ = true;

    AVUtils::ScopedValue<int64_t>
        scopedTimeoutDisable(socket_timeout_ms_, -1, socket_timeout_ms_);

    FindBestVideoStream();

    demux_thread_ = std::make_unique<std::thread>([this]() {
        DemuxRun();
    });

    return true;
  }

  void Close() override {
    if (!Opened())
      return;

    stop_ = true;

    if (demux_thread_ && demux_thread_->joinable()) {
      demux_thread_->join();
    }

    last_socket_access_ = std::chrono::system_clock::now();

    if (format_context_) {
      format_context_->interrupt_callback.opaque = nullptr;
      format_context_->interrupt_callback.callback = nullptr;
      format_context_.reset();
    }

    monitor_.reset(new char);
    streams_.Clear();
    stop_ = false;
    opened_ = false;
  }

  bool Opened() override {
    return opened_;
  }

  void EnableReadPacketRateByFPS(bool enable) override {
    read_packet_rate_by_fps_ = enable;
  }

  void SetTimeoutCallback(int timeoutMS, OnDemuxerTimeoutCallback callback) override {
    socket_timeout_ms_ = timeoutMS;
    timeout_callback_ = std::move(callback);
  }

  void SetInputStreamCallback(OnDemuxerInputStreamCallback callback) override {
    get_input_stream_callback_ = std::move(callback);
  }

  void SetEOFCallback(OnDemuxerEOFCallback callback) override {
    eof_callback_ = std::move(callback);
  }

  void SetConnectionRefuseCallback(OnDemuxerEOFCallback callback) override {
    refuse_callback_ = std::move(callback);
  }

private:
  static int AVInterruptCallback(void *opaque) {
    if (!opaque)
      return 0;
    return ((AVDemuxerImpl *) opaque)->InterruptCallback();
  }

  int InterruptCallback() {
    std::chrono::time_point<std::chrono::system_clock> currentTime = std::chrono::system_clock::now();
    auto elapsedTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - last_socket_access_).count();
    if (elapsedTime > socket_timeout_ms_ && socket_timeout_ms_ > -1) {
      OnTimeoutCallback();
      return 1;
    }
    return 0;
  }

  void FindBestVideoStream() {
    auto ret = avformat_find_stream_info(format_context_.get(), nullptr);
    if (ret < 0) {
      LOG(ERROR) << "AVDemuxer find stream info for:" << url_ << " failed," << av_err2str(ret);
      return;
    }
    for (int i = 0; i < format_context_->nb_streams; i++) {
      auto stream = format_context_->streams[i];
      if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        video_stream_index_ = i;
        break;
      }
    }
    if (video_stream_index_ == -1) {
      LOG(ERROR) << "AVDemuxer not find video stream for:" << url_;
      return;
    }

    auto ss = std::make_shared<InputVideoStream>(format_context_->streams[video_stream_index_], this, monitor_);
    streams_.Insert(video_stream_index_, ss);

    if (get_input_stream_callback_) {
      get_input_stream_callback_(ss);
    }
  }

  void DemuxRun() {
    demux_fps_counter_.Start();
    while (ProcessDemux()) {}
  }

  bool ProcessDemux() {

    if (stop_) {
      return false;
    }

    auto start = std::chrono::system_clock::now();

    int ret = 0;
    auto pkt = std::shared_ptr<AVPacket>(av_packet_alloc(), AVDeleter::AVPacketDeleter());
    const int retryCount = 5;
    int tryCount = 0;
    do {
      last_socket_access_ = std::chrono::system_clock::now();
      ret = av_read_frame(format_context_.get(), pkt.get());
      ++tryCount;
    } while (ret == AVERROR(EAGAIN) && tryCount <= retryCount);

    if (ret == AVERROR_EOF) {
      ProcessEOF();
      return false;
    }

    std::shared_ptr<AVInputStreamInterface> stream;
    if (!streams_.Lookup(pkt->stream_index, stream)) {
      return true;
    }

    if (pkt->size <= 0) {
      return true;
    }

    if (stream) {
      stream->FeedAVPacket(pkt);
    }
    demux_fps_counter_.Fps();
    LOG_EVERY_N(INFO, 250) << "AVDemuxer:" << url_ << "\n"
                           << ">>>>>>>>>>>>>>>>>receive video fps:" << demux_fps_counter_.Fps();

    /// TODO: fixme, 对于有音频和视频等处理不太恰当
    if (read_packet_rate_by_fps_ && stream->Type() == kVideoStream) {
      auto end = std::chrono::system_clock::now();
      auto elapsedTime =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      auto es = (1000 / std::dynamic_pointer_cast<InputVideoStream>(stream)->Fps()) - elapsedTime;
      if (es > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(es));
      }
    }
    return true;
  }

  void ProcessEOF() {
    if (eof_callback_) {
      eof_callback_(url_);
    }
  }

  void OnTimeoutCallback() {
    if (timeout_callback_) {
      timeout_callback_();
    }
    last_socket_access_ = std::chrono::system_clock::now();
  }


private:
  std::shared_ptr<char> monitor_{new char};
  std::string url_;
  std::atomic<bool> stop_{false};
  std::atomic<bool> opened_{false};
  int64_t socket_timeout_ms_{3000};
  int video_stream_index_{-1};
  bool read_packet_rate_by_fps_{false};
  std::chrono::time_point<std::chrono::system_clock> last_socket_access_;
  std::shared_ptr<AVFormatContext> format_context_;
  std::unique_ptr<std::thread> demux_thread_;
  FPS demux_fps_counter_;
  OnDemuxerTimeoutCallback timeout_callback_;
  OnDemuxerEOFCallback eof_callback_;
  OnDemuxerInputStreamCallback get_input_stream_callback_;
  OnDemuxerConnectionRefuseCallback  refuse_callback_;
  SafeMap<int, std::shared_ptr<AVInputStreamInterface>> streams_;
};

AVDemuxer::AVDemuxer() {
  impl_ = std::make_unique<AVDemuxerImpl>();
}

AVDemuxer::~AVDemuxer() {
  Close();
}

bool AVDemuxer::Open(std::string url) {
  return Open(url, nullptr, nullptr);
}

bool AVDemuxer::Open(std::string url, AVInputFormat* format, std::shared_ptr<AVDictionary> opt) {
  return impl_->Open(std::move(url), format, std::move(opt));
}

void AVDemuxer::Close() {
  impl_->Close();
}

bool AVDemuxer::Opened() {
  return impl_->Opened();
}

void AVDemuxer::EnableReadPacketRateByFPS(bool enable) {
  impl_->EnableReadPacketRateByFPS(enable);
}

void AVDemuxer::SetTimeoutCallback(int timeoutMS, OnDemuxerTimeoutCallback callback) {
  impl_->SetTimeoutCallback(timeoutMS, std::move(callback));
}

void AVDemuxer::SetInputStreamCallback(OnDemuxerInputStreamCallback callback) {
  impl_->SetInputStreamCallback(std::move(callback));
}

void AVDemuxer::SetEOFCallback(OnDemuxerEOFCallback callback) {
  impl_->SetEOFCallback(std::move(callback));
}

void AVDemuxer::SetConnectionRefuseCallback(OnDemuxerEOFCallback callback) {
  impl_->SetConnectionRefuseCallback(std::move(callback));
}

}