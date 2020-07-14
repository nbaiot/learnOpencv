//
// Created by nbaiot@126.com on 2020/7/2.
//

#include "av_muxer.h"

#include <chrono>
#include <glog/logging.h>
#include <container/safe_map.h>

#include "ffmpeg.h"
#include "av_deleter.h"
#include "video_encoder.h"
#include "output_video_stream.h"


namespace nbaiot {

class AVMuxerImpl : public AVMuxerInterface {

public:
  AVMuxerImpl() = default;

  ~AVMuxerImpl() override {
    Close();
  }

  bool Open(std::string fileName, const std::string& mineType) override {
    if (Opened()) {
      LOG(ERROR) << ">>>>>>>>>>>AVMuxer already opened:" << url_;
      return false;
    }

    url_ = std::move(fileName);
    format_ = av_guess_format(mineType.empty() ? nullptr : mineType.c_str(),
                              url_.empty() ? nullptr : url_.c_str(),
                              nullptr);

    if (!format_) {
      LOG(ERROR) << ">>>>>>>>>>>AVMuxer not support format:" << url_;
      return false;
    }

    AVFormatContext* rawPtr = nullptr;
    auto ret = avformat_alloc_output_context2(&rawPtr, format_, nullptr, url_.c_str());
    format_context_ = std::shared_ptr<AVFormatContext>(rawPtr, AVDeleter::AVFormatContextDeleter());
    if (format_context_) {
      format_context_->interrupt_callback.opaque = this;
      format_context_->interrupt_callback.callback = AVMuxerImpl::AVInterruptCallback;
    }

    if (ret < 0) {
      LOG(ERROR) << ">>>>>>>>>>>AVMuxer alloc failed:" << av_err2str(ret);
      return false;
    }

    last_socket_access_ = std::chrono::system_clock::now();
    ret = avio_open2(&format_context_->pb, url_.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);

    if (ret < 0) {
      LOG(ERROR) << ">>>>>>>>>>>AVMuxer open :" + url_ << " failed," << av_err2str(ret);
      format_context_.reset();
      return false;
    }

    opened_ = true;

    return true;
  }

  bool Opened() override {
    return opened_;
  }

  void Close() override {
    if (!Opened())
      return;

    /// encoder must flush
    if (write_header_ && !write_trailer_) {
      WriteTrailer();
    }

    monitor_.reset(new char);
    streams_.Clear();
    format_context_.reset();
    opened_ = false;
    write_header_ = false;
    write_trailer_ = false;
  }

  std::shared_ptr<AVOutputStreamInterface> AddVideoStream(const VideoEncoder *encoder) override {
    if (!encoder || !encoder->Opened()) {
      LOG(ERROR) << ">>>>>>>>>> AVMuxer add stream failed: codec must open before add stream to muxer";
      return nullptr;
    }

    if (!Opened()) {
      LOG(ERROR) << ">>>>>>>>>> AVMuxer not opened, so cannot add video stream";
      return nullptr;
    }

    if (write_header_) {
      LOG(ERROR) << ">>>>>>>>>> AVMuxer add stream must before write header";
      return nullptr;
    }

    auto stream = avformat_new_stream(format_context_.get(), ((AVCodecContext*) encoder->RawAVPtr())->codec);
    if (!stream) {
      LOG(ERROR) << ">>>>>>>>>> AVMuxer new stream failed";
      return nullptr;
    }

    avcodec_parameters_from_context(stream->codecpar, (AVCodecContext*) encoder->RawAVPtr());
    int fps = const_cast<VideoEncoder*>(encoder)->Config().maxFPS;
    auto videoStream = std::make_shared<OutputVideoStream>(stream, fps, this, monitor_);
    streams_.Insert(stream->index, videoStream);

    return videoStream;
  }

  bool WriteHeader() override {
    if (write_header_)
      return true;

    if (!opened_) {
      LOG(ERROR) << ">>>>>>>>>>>AVMuxer write header failed: not opened";
      return false;
    }

    last_socket_access_ = std::chrono::system_clock::now();
    auto ret = avformat_write_header(format_context_.get(), nullptr);

    if (ret < 0) {
      LOG(ERROR) << ">>>>>>>>>>>AVMuxer write header failed:" << av_err2str(ret);
      return false;
    }

    write_header_ = true;

    return true;
  }

  bool WritePacket(const std::shared_ptr<AVPacket>& packet) override {
    if (!CanWritePacket())
      return false;

    last_socket_access_ = std::chrono::system_clock::now();

    auto ret = av_interleaved_write_frame(format_context_.get(), packet.get());
    if (ret != 0) {
      LOG(ERROR) << ">>>>>>>>>>>>>>>>>>>>AVMuxer write packet failed:" << av_err2str(ret);
    }
    return true;
  }

  bool WriteTrailer() override {
    if (!opened_) {
      LOG(ERROR) << ">>>>>>>>>>>AVMuxer write trailer failed: not opened";
      return false;
    }

    if (!write_header_) {
      LOG(ERROR) << ">>>>>>>>>>>AVMuxer write trailer failed: not write header";
      return false;
    }

    if (write_trailer_) {
      LOG(ERROR) << ">>>>>>>>>>>AVMuxer already write trailer";
      return false;
    }

    auto ret = av_write_trailer(format_context_.get());

    if (ret != 0) {
      LOG(ERROR) << ">>>>>>>>>>>AVVideoWriterImpl write header trailer failed:" << av_err2str(ret);
      return false;
    }

    write_trailer_ = true;

    return true;
  }

  bool CanWritePacket() override {
    if (!Opened())
      return false;

    if (!write_header_) {
      LOG(ERROR) << ">>>>>>>>>>AVMuxer not writer header";
      return false;
    }

    if (write_trailer_) {
      LOG(ERROR) << ">>>>>>>>>>AVMuxer already writer trailer, so can't write packet";
      return false;
    }

    return true;
  }

  void SetTimeoutCallback(int timeoutMS, OnMuxerTimeoutCallback callback) override {
    socket_timeout_ms_ = timeoutMS;
    timeout_callback_ = std::move(callback);
  }

private:
  static int AVInterruptCallback(void *opaque) {
    if (!opaque)
      return 0;
    return ((AVMuxerImpl *) opaque)->InterruptCallback();
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

  void OnTimeoutCallback() {
    LOG(INFO) << ">>>>>>>>>>>>>>>>>>>>>>>> timeout:" << url_;
    if (timeout_callback_) {
      timeout_callback_();
    }
    last_socket_access_ = std::chrono::system_clock::now();
  }

private:
  std::shared_ptr<char> monitor_{new char};
  std::string url_;
  bool opened_{false};
  AVOutputFormat *format_{nullptr};
  std::shared_ptr<AVFormatContext> format_context_;
  bool write_header_{false};
  bool write_trailer_{false};
  int64_t socket_timeout_ms_{3000};
  OnMuxerTimeoutCallback  timeout_callback_;
  std::chrono::time_point<std::chrono::system_clock> last_socket_access_;
  SafeMap<int , std::shared_ptr<AVOutputStreamInterface>> streams_;
};

AVMuxer::AVMuxer() {
  impl_ = std::make_unique<AVMuxerImpl>();
}

AVMuxer::~AVMuxer() {
  impl_->Close();
}

bool AVMuxer::Open(std::string fileName, const std::string& mineType) {
  return impl_->Open(std::move(fileName), mineType);
}

bool AVMuxer::Opened() {
  return impl_->Opened();
}

void AVMuxer::Close() {
  impl_->Close();
}

std::shared_ptr<AVOutputStreamInterface> AVMuxer::AddVideoStream(const VideoEncoder *encoder) {
  return impl_->AddVideoStream(encoder);
}

bool AVMuxer::WriteHeader() {
  return impl_->WriteHeader();
}

bool AVMuxer::WritePacket(const std::shared_ptr<AVPacket>& packet) {
  return impl_->WritePacket(packet);
}


bool AVMuxer::WriteTrailer() {
  return impl_->WriteTrailer();
}

void AVMuxer::SetTimeoutCallback(int timeoutMS, OnMuxerTimeoutCallback callback) {
  impl_->SetTimeoutCallback(timeoutMS, std::move(callback));
}

bool AVMuxer::CanWritePacket() {
  return impl_->CanWritePacket();
}

}