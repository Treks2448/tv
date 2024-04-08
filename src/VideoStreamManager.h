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
  VideoStreamManager(const std::string& filename, int outBufWidth, int outBufHeight); 
  ~VideoStreamManager();

  inline int readAndDecodeNextPacket(); 
  inline void unrefPacket();
  inline int nextVideoFrame();
  inline void unrefFrame();

  // deleted constructors/assignment operators
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
  uint8_t* RGB_frame_buf[4]{nullptr, nullptr, nullptr, nullptr};
  int RGB_frame_buf_linesize[4]{-1,-1,-1,-1};
  int frame_buf_width{-1};
  int frame_buf_height{-1};
 
  // Source (video) buffer details
  uint8_t* src_data[4]{nullptr, nullptr, nullptr, nullptr};
  int src_linesize[4]{-1,-1,-1,-1};
  int src_width{-1};
  int src_height{-1};

  int RGB_frame_buf_size{-1};

  int vid_str_idx{-1};
};

#endif
