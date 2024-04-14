#ifndef VideoStreamManager_h
#define VideoStreamManager_h

extern "C" 
{
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_par.h>
#include <libavcodec/packet.h>

#include <libavformat/avformat.h>

#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>

#include <libswscale/swscale.h>
} // extern "C"

#include <string>

class VideoStreamManager {
public:
  enum ManagerState {
    GOT_VIDEO_PKT,
    GOT_AUDIO_PKT,
    GOT_EOF,
    FAILED 
  };

  VideoStreamManager(const std::string& filename, int outBufWidth, int outBufHeight); 
  ~VideoStreamManager();

  VideoStreamManager::ManagerState processNextPacket();
 
  // deleted constructors/assignment/move operators
  VideoStreamManager() = delete;
  VideoStreamManager(VideoStreamManager&) = delete;
  VideoStreamManager& operator=(VideoStreamManager&) = delete;
  VideoStreamManager(VideoStreamManager&&) = delete;
  VideoStreamManager&& operator=(VideoStreamManager&&) = delete;

private:
  // ffmpeg utilities for reading file and decoding video/audio 
  AVFormatContext* fmt_ctx{nullptr};
  AVCodecContext* dec_ctx{nullptr};
  const AVCodec* dec{nullptr};
  AVStream* vid_str{nullptr};
  AVFrame* frame{nullptr};
  AVPacket* packet{nullptr};
  SwsContext *sws_ctx{nullptr};

  // RGB buffer details
  uint8_t* RGB_frame_buf[4];
  int RGB_frame_buf_linesize[4];
  int frame_buf_width;
  int frame_buf_height;
 
  // Source (video) buffer details
  uint8_t* src_data[4];
  int src_linesize[4];
  int src_width;
  int src_height;

  int RGB_frame_buf_size;

  int vid_str_idx;
};

#endif
