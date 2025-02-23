#include "VideoStreamManager.h"

extern "C" 
{
#include <libavutil/pixfmt.h>
#include <libavutil/frame.h>
} // extern "C"

#include <iostream>

VideoStreamManager::VideoStreamManager(const std::string& filename, int outBufWidth, int outBufHeight)
  :
  frame_buf_width(outBufWidth),
  frame_buf_height(outBufHeight)
{
  int ret = 0;    
  // Get format context
  ret = avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL);
  ret = avformat_find_stream_info(fmt_ctx, NULL);

  // get video stream and respective index
  vid_str_idx = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);

  // Filling out decoder context with appropriate data
  open_codec_context(vid_str_idx, &dec_ctx, AVMEDIA_TYPE_VIDEO);

  // Set video params for later use
  src_width = dec_ctx->width;
  src_height = dec_ctx->height; 
 
  frame = av_frame_alloc();
  packet = av_packet_alloc();
  
  // allocate buffers to store video data
  // src_linesize is not correctly calculated for some reason - frame->linesize gives the correct linesize
  // might be to do with the last parameter to this function
  av_image_alloc(src_data, 
                 src_linesize, 
                 src_width, 
                 src_height, 
                 dec_ctx->pix_fmt, 
                 32);
  av_image_alloc(RGB_frame_buf, 
                 RGB_frame_buf_linesize, 
                 frame_buf_width, 
                 frame_buf_height, 
                 AV_PIX_FMT_RGB24, 
                 1); 
  RGB_frame_buf_size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, 
                                                frame_buf_width, 
                                                frame_buf_height, 
                                                RGB_frame_buf_linesize[0]);

  sws_ctx = sws_getContext(src_width, 
                           src_height, 
                           dec_ctx->pix_fmt, 
                           frame_buf_width, 
                           frame_buf_height, 
                           AV_PIX_FMT_RGB24, 
                           SWS_BILINEAR, 
                           NULL, 
                           NULL, 
                           NULL);

}

VideoStreamManager::~VideoStreamManager() {
  av_freep(&RGB_frame_buf[0]); 
  av_freep(&src_data[0]);
  av_frame_free(&frame);
  av_packet_free(&packet);
  avcodec_free_context(&dec_ctx);
  avformat_close_input(&fmt_ctx);
  sws_freeContext(sws_ctx);
}

VideoStreamManager::ManagerState VideoStreamManager::processNextPacket() {
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
      
      while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
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
                  // src_linesize is not correctly calculated for some reason - use linesize from frame directly
                  //src_linesize,
                  frame->linesize,
                  0,
                  src_height,
                  RGB_frame_buf,
                  RGB_frame_buf_linesize);

        av_frame_unref(frame);
        return ManagerState::GOT_VIDEO_PKT;
      }
    }
    return ManagerState::FAILED;
  }

void VideoStreamManager::open_codec_context(
  int str_idx,
  AVCodecContext **dec_ctx,
  enum AVMediaType type)
{
  int ret{0};
  AVStream* str = fmt_ctx->streams[str_idx];
  const AVCodec* dec = avcodec_find_decoder(str->codecpar->codec_id);

  (void)type; // TODO: temporary cast to void to avoid unused-parameter warning

  *dec_ctx = avcodec_alloc_context3(dec);
  ret = avcodec_parameters_to_context(*dec_ctx, str->codecpar);
  ret = avcodec_open2(*dec_ctx, dec, NULL);
}