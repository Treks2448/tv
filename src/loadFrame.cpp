extern "C" {
  #include <libavcodec/packet.h>
  #include <libavutil/error.h>
  #include <libavutil/frame.h>
  #include <libavformat/avformat.h>
  #include <libavcodec/avcodec.h>
  #include <libavutil/avutil.h>
  #include <libavcodec/codec_par.h>
  #include <libavutil/imgutils.h>
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
  const AVMediaType type{AVMEDIA_TYPE_VIDEO};
  //const AVPixelFormat pix_fmt{AV_PIX_FMT_RGBA};
  int vid_str_idx = 0; 
  
  // Get format context
  ret = avformat_open_input(&fmt_ctx, filename.c_str(), NULL, NULL);
  ret = avformat_find_stream_info(fmt_ctx, NULL);

  //open_codec_context(&vid_str_idx, dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO);

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
  //*RGB_frame_buf = new uint8_t[static_cast<size_t>(RGB_frame_buf_size)];
  *RGB_frame_buf = reinterpret_cast<uint8_t*>(av_malloc(static_cast<size_t>(RGB_frame_buf_size)));
  
  // Repeatedly read video packets/frames from file
  ret = 0;
  while (ret >= 0) {
    // send packet
    // if packet is video packet
    //  while (no issue with receiving/decoding frames) 
    //    receive frame
    //    if valid video frame
    //      copy frame to buffer 
    //    else if end of file 
    //      break out of whole loop
    //    else if EAGAIN
    //      break out of inner loop
    // unref packet
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

      ret = av_image_copy_to_buffer(*RGB_frame_buf,
                                    RGB_frame_buf_size,
                                    const_cast<const uint8_t * const *>(frame->data),
                                    const_cast<const int*>(frame->linesize), 
                                    dec_ctx->pix_fmt, 
                                    frame->width, 
                                    frame->height,
                                    1);
      
      if (ret < 0) {
        std::cout << "Error copying frame to buffer. Error: " << ret << "\n";
        return 1;
      }
      av_frame_unref(frame);
    }
    
    //if (packet->stream_index == vid_str_idx){
    //  ret = avcodec_send_packet(dec_ctx, packet);
    //  while (ret >= 0) {
    //    ret = avcodec_receive_frame(dec_ctx, frame);
    //    switch (ret) {
    //      case 0:
    //        ret = av_image_copy_to_buffer(*RGB_frame_buf,
    //                                      RGB_frame_buf_size,
    //                                      const_cast<const uint8_t * const *>(frame->data),
    //                                      frame->linesize, 
    //                                      dec_ctx->pix_fmt, 
    //                                      frame->width, 
    //                                      frame->height,
    //                                      1);
    //        av_frame_unref(frame);
    //        continue;
    //      case AVERROR_EOF:
    //        //goto finish
    //      case AVERROR(EAGAIN):
    //        ret = 0;
    //        av_frame_unref(frame);
    //        break;
    //      default:
    //        std::cout << "oops";
    //        return 1;
    //    }
    //    break;
    //  }
    //}
    //av_packet_unref(packet);
  }

finish:
 
  // TODO: these need to be moved to a different function than init
  // av_freep(RGB_frame_buf);
  avcodec_free_context(&dec_ctx);
  avformat_close_input(&fmt_ctx);
  av_packet_free(&packet);
  av_frame_free(&frame);
  
  return 0;
}
