#include "VideoStreamManager.h"

#include <iostream>
#include <libavutil/frame.h>

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
  vid_str = fmt_ctx->streams[vid_str_idx];
  
  // Filling out decoder context and decoder with appropriate data
  dec = avcodec_find_decoder(vid_str->codecpar->codec_id);
  dec_ctx = avcodec_alloc_context3(dec);
  ret = avcodec_parameters_to_context(dec_ctx, vid_str->codecpar);
  ret = avcodec_open2(dec_ctx, dec, NULL);

  // Set video params for later use
  int src_width = dec_ctx->width;
  int src_height = dec_ctx->height; 

  frame = av_frame_alloc();
  packet = av_packet_alloc();
 
  // allocate buffers to store video data
  RGB_frame_buf_size = av_image_get_buffer_size(dec_ctx->pix_fmt, frame_buf_width, frame_buf_height, 16);
  av_image_alloc(src_data, src_linesize, src_width, src_height, dec_ctx->pix_fmt, 16);
  av_image_alloc(RGB_frame_buf, RGB_frame_buf_linesize, frame_buf_width, frame_buf_height, AV_PIX_FMT_RGB24, 1); 
}

VideoStreamManager::~VideoStreamManager() {
  av_freep(&RGB_frame_buf[0]); 
  av_freep(&src_data[0]);
  av_frame_free(&frame);
  av_packet_free(&packet);
  avcodec_free_context(&dec_ctx);
  avformat_close_input(&fmt_ctx);
}

inline int VideoStreamManager::readAndDecodeNextPacket() {
  int ret = 0; 
  if((ret = av_read_frame(fmt_ctx, packet)) < 0) {
    std::cout << "Error reading frame\n";
    return ret;
  }
  if ((ret = avcodec_send_packet(dec_ctx, packet)) < 0) {
    std::cout << "Error submitting packet for decoding\n";
    return ret;
  }
  return packet->stream_index;
}

inline void VideoStreamManager::unrefPacket() {
  av_packet_unref(packet); 
}

inline int VideoStreamManager::nextVideoFrame() {
  int ret = avcodec_receive_frame(dec_ctx, frame);
  if (ret == AVERROR_EOF | ret == AVERROR(EAGAIN) | ret < 0) {
    return ret;
  }
   
  sws_scale(sws_ctx, 
            const_cast<const uint8_t * const *>(frame->data),
            src_linesize,
            0,
            src_height,
            RGB_frame_buf,
            RGB_frame_buf_linesize);

  return 0;
}

inline void VideoStreamManager::unrefFrame() {
  av_frame_unref(frame);
}
