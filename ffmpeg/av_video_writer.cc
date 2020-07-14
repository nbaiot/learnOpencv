//
// Created by nbaiot@126.com on 2020/6/29.
//

#include "av_video_writer.h"
#include "ffmpeg.h"
#include "av_muxer.h"
#include "video_encoder.h"
#include "av_output_stream_interface.h"
#include "av_video_frame.h"

#include <thread>
#include <glog/logging.h>


namespace nbaiot {

class AVVideoWriterImpl {

public:
  AVVideoWriterImpl(std::string fileName, std::string mineType, int w, int h, int fps,
                    int64_t bitRate, std::string codecName)
      : file_(std::move(fileName)), mine_type_(std::move(mineType)),
        w_(w), h_(h), fps_(fps), bit_rate_(bitRate),
        codec_name_(std::move(codecName)), gop_size_(fps), max_b_frames_(2) {

    auto cpuCount = std::thread::hardware_concurrency();
    if (w <= 1280) {
      /// 720p
      cpuCount = cpuCount >= 2 ? 2 : cpuCount;
    } if (w <= 1920) {
      /// 1080p
      cpuCount = cpuCount >= 4 ? 4 : cpuCount;
    } if (w <= 4096) {
      /// 4k
      cpuCount = cpuCount >= 6 ? 6 : cpuCount;
    } else {
      /// > 4k
      cpuCount = cpuCount >= 10 ? 10 : cpuCount;
    }
    encode_thread_count_ = cpuCount;
    muxer_ = std::make_unique<AVMuxer>();
  }

  ~AVVideoWriterImpl() {
    Close();
  }

  /// 必须在 open 之前调用
  void SetGOP(int gop) {
    gop_size_ = gop;
  }

  void SetMaxBFrames(int bFrames) {
    max_b_frames_ = bFrames;
  }

  void SetEncodeThreadCount(int count) {
    encode_thread_count_ = count;
  }

  bool Open() {

    if (muxer_->Opened()) {
      LOG(ERROR) << ">>>>>>>>>>>AVVideoWriterImpl already opened :" + file_;
      return false;
    }

    if (!muxer_->Open(file_, mine_type_)) {
      LOG(ERROR) << ">>>>>>>>>>>AVVideoWriterImpl not support format:" + file_;
      return false;
    }

    encoder_ = std::make_unique<VideoEncoder>();

    encoder_->SetEncodeCallback([this](const std::shared_ptr<AVPacket>& packet) {
        OnEncodeCallback(packet);
    });

    VideoEncoderConfig encoderConfig;
    encoderConfig.width = w_;
    encoderConfig.height = h_;
    encoderConfig.gopSize = gop_size_;
    encoderConfig.maxBFrames = max_b_frames_;
    encoderConfig.bitRate = bit_rate_;
    encoderConfig.maxFPS = fps_;
    encoderConfig.globalQuality = -1;
    encoderConfig.threadCount = encode_thread_count_;
    encoderConfig.codecName = codec_name_;
    encoder_->Open(encoderConfig);


    video_stream_ = muxer_->AddVideoStream(encoder_.get());

    if (!video_stream_) {
      LOG(ERROR) << ">>>>>>>>>>>AVVideoWriterImpl open new stream failed";
      return false;
    }

    muxer_->WriteHeader();

    return true;
  }

  void Close() {
    if (!muxer_->Opened())
      return;

    if (encoder_ && encoder_->Opened()) {
      encoder_->Flush();
      encoder_->Close();
    }

    muxer_->WriteTrailer();
    muxer_->Close();
  }

  bool Opened() {
    return muxer_->Opened();
  }

  void Write(const std::shared_ptr<AVVideoFrame>& frame) {
    if (Opened() && encoder_ && encoder_->Opened()) {
      encoder_->Encode(frame);
    }
  }


private:

  void OnEncodeCallback(const std::shared_ptr<AVPacket>& packet) {
    if (muxer_->Opened() && video_stream_ && video_stream_->Isvalid()) {
      video_stream_->FeedPacket(packet);
    }
  }

private:
  std::string file_;
  std::string mine_type_;
  int w_;
  int h_;
  int fps_;
  int64_t bit_rate_;
  std::string codec_name_;
  int gop_size_{fps_};
  int max_b_frames_{2};
  int encode_thread_count_{4};
  std::unique_ptr<AVMuxer> muxer_;
  std::shared_ptr<AVOutputStreamInterface> video_stream_;
  std::unique_ptr<VideoEncoder> encoder_;
};

AVVideoWriter::AVVideoWriter(std::string fileName, std::string mineType, int w, int h, int fps, int64_t bitRate,
                             std::string codecName) {
  impl_ = std::make_unique<AVVideoWriterImpl>(std::move(fileName),
                                              std::move(mineType), w, h, fps, bitRate, std::move(codecName));
}

bool AVVideoWriter::Open() {
  return impl_->Open();
}

bool AVVideoWriter::IsOpened() {
  return impl_->Opened();
}

void AVVideoWriter::Close() {
  impl_->Close();
}

void AVVideoWriter::Write(const std::shared_ptr<AVVideoFrame>& frame) {
  impl_->Write(frame);
}

void AVVideoWriter::SetGOP(int gop) {
  impl_->SetGOP(gop);
}

void AVVideoWriter::SetMaxBFrames(int bFrames) {
  impl_->SetMaxBFrames(bFrames);
}

void AVVideoWriter::SetEncodeThreadCount(int count) {
  impl_->SetEncodeThreadCount(count);
}


AVVideoWriter::~AVVideoWriter() = default;
}