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
#include <iostream>

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

  inline VideoStreamManager::ManagerState processNextPacket() {
    int ret = 0;
    while (ret >= 0) {
      ret = av_read_frame(fmt_ctx, packet);
      if (ret >= 0 && packet->stream_index != vid_str_idx) {
        av_packet_unref(packet);
        continue;
      }
      if (ret < 0) {
        ret = avcodec_send_packet(dec_ctx, NULL);
      }
      else {
        ret = avcodec_send_packet(dec_ctx, packet);
      }
      av_packet_unref(packet);
      
      if (ret < 0) {
        std::cout << "Error [" << ret << "]: failed to send packet to decoding.\n";
        return ManagerState::FAILED;
      }
      
      // move this while loop out of this if statement, excluding the part readingg
      // the frame buffer, if possible
      while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        std::cout << ret << "\n"; // debugging
        if (ret == AVERROR_EOF) {
          return ManagerState::GOT_EOF;
        }
        else if (ret == AVERROR(EAGAIN)) {
          ret = 0;
          break;
        }
        else if (ret < 0) {
          std::cout << "Error [" << ret << "]: failed to receive frame";
          return ManagerState::FAILED;
        }
      
        sws_scale(sws_ctx, 
                  const_cast<const uint8_t * const *>(frame->data),
                  src_linesize,
                  0,
                  src_height,
                  RGB_frame_buf,
                  RGB_frame_buf_linesize);

        av_frame_unref(frame);
        //return ManagerState::GOT_VIDEO_PKT;
      }
    }
    return ManagerState::FAILED;
  }

  inline int readAndDecodeNextPacket() {
    int ret = 0; 
    if((ret = av_read_frame(fmt_ctx, packet)) < 0) {
      avcodec_send_packet(dec_ctx, NULL);
      std::cout << "Error reading frame\n";
      return ret;
    }
    if ((ret = avcodec_send_packet(dec_ctx, packet)) < 0) {
      std::cout << "Error submitting packet for decoding\n";
      return ret;
    }  
    return packet->stream_index;
  }

  inline void unrefPacket() {
    av_packet_unref(packet); 
  }
  
  inline int nextVideoFrame() {
    int ret = avcodec_receive_frame(dec_ctx, frame);
    if ((ret == AVERROR_EOF) || (ret == AVERROR(EAGAIN)) || (ret < 0)) {
      return ret;
    }
     
    sws_scale(sws_ctx, 
              const_cast<const uint8_t * const *>(frame->data),
              src_linesize,
              0,
              src_height,
              RGB_frame_buf,
              RGB_frame_buf_linesize);
  
    return ret;
  }
  
  inline void unrefFrame() {
    av_frame_unref(frame);
  }
  
  
  inline int getVidStreamIdx() {
    return vid_str_idx;
  }


  // deleted constructors/assignment/move operators
  VideoStreamManager() = delete;
  VideoStreamManager(VideoStreamManager&) = delete;
  VideoStreamManager& operator=(VideoStreamManager&) = delete;
  VideoStreamManager(VideoStreamManager&&) = delete;
  VideoStreamManager&& operator=(VideoStreamManager&&) = delete;

public:

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
