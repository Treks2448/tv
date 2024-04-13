#include <libavutil/pixfmt.h>
extern "C" {
  #include <libavcodec/packet.h>
  #include <libavutil/error.h>
  #include <libavutil/frame.h>
  #include <libavformat/avformat.h>
  #include <libavcodec/avcodec.h>
  #include <libavutil/avutil.h>
  #include <libavcodec/codec_par.h>
  #include <libavutil/imgutils.h>
  #include <libswscale/swscale.h>
}

#include <string>
#include <cerrno>
#include <iostream>

int init(std::string filename, int& width, int& height, uint8_t** RGB_frame_buf, int& RGB_frame_buf_size) {
  int ret = 0; 

  AVFormatContext* fmt_ctx = nullptr;
  AVCodecContext* dec_ctx = nullptr;
  const AVCodec* dec = nullptr;
  AVStream* vid_str = nullptr;
  AVFrame* frame = nullptr;
  AVPacket* packet = nullptr;
  SwsContext *sws_ctx = nullptr;
  uint8_t *src_data[4], *dst_data[4];
  int src_linesize[4], dst_linesize[4];
  const AVMediaType type{AVMEDIA_TYPE_VIDEO};
  AVPixelFormat dst_pix_fmt{AV_PIX_FMT_RGB24};
  int vid_str_idx = 0;  
  // Get format context
  ret = avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL);
  ret = avformat_find_stream_info(fmt_ctx, NULL);

  // get video stream and respective index
  vid_str_idx = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
  vid_str = fmt_ctx->streams[vid_str_idx];
  
  // Filling out decoder context and decoder with appropriate data
  dec = avcodec_find_decoder(vid_str->codecpar->codec_id);
  dec_ctx = avcodec_alloc_context3(dec);
  ret = avcodec_parameters_to_context(dec_ctx, vid_str->codecpar);
  ret = avcodec_open2(dec_ctx, dec, NULL);

  // Set video params for later use
  width = dec_ctx->width;
  height = dec_ctx->height; 

  frame = av_frame_alloc();
  packet = av_packet_alloc();

  RGB_frame_buf_size = av_image_get_buffer_size(dec_ctx->pix_fmt, width, height, 16);
  av_image_alloc(src_data, src_linesize, width, height, dec_ctx->pix_fmt, 16);
  av_image_alloc(dst_data, dst_linesize, width, height, dst_pix_fmt, 1);
  
  *RGB_frame_buf = dst_data[0];

  sws_ctx = sws_getContext(width, 
                           height, 
                           dec_ctx->pix_fmt, 
                           width, 
                           height, 
                           dst_pix_fmt, 
                           SWS_BILINEAR, 
                           NULL, 
                           NULL, 
                           NULL);

  // Repeatedly read video packets/frames from file
  ret = 0;
  while (ret >= 0) {
    ret = av_read_frame(fmt_ctx, packet);
    if (ret >= 0 && packet->stream_index != vid_str_idx) {
      av_packet_unref(packet);
      continue;
    }
    else if (ret < 0) {
      ret = avcodec_send_packet(dec_ctx, NULL);
    }
    else {
      ret = avcodec_send_packet(dec_ctx, packet);
    }   
    av_packet_unref(packet);
  
    if (ret < 0) {
      std::cout << "Error submitting packet for decoding\n";
      return 1;
    }
     
    while (ret >= 0) {
      ret = avcodec_receive_frame(dec_ctx, frame);
      if (ret == AVERROR_EOF) {
        goto finish;
      }
      else if (ret == AVERROR(EAGAIN)) {
        ret = 0;
        break;
      }
      else if (ret < 0) {
        std::cout << "Error decoding frame. Error: " << ret << "\n";
        break;
      }
       
      sws_scale(sws_ctx, 
                const_cast<const uint8_t * const *>(frame->data),
                src_linesize,
                0,
                height,
                dst_data,
                dst_linesize);
      
      av_frame_unref(frame);
    }  
  }

finish:
 
  // TODO: these need to be moved to a different function than init
  // av_freep(src_data);
  // av_freep(dst_data);
  avcodec_free_context(&dec_ctx);
  avformat_close_input(&fmt_ctx);
  av_packet_free(&packet);
  av_frame_free(&frame);
  
  return 0;
}
