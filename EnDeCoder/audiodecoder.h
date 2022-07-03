#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include "tool.h"
class AudioDecoder
{
private:
    AVFormatContext* formatContext;
    AVCodecContext* codecContext;
    AVPacket* pkt;
    AVCodecParserContext* parserContext;
    AVFrame* decoded_frame;

   /*********重采样相关********/
   int out_sample_rate; // 采样率
   SwrContext* au_convert_ctx;
   AVSampleFormat out_sample_fmt;
   int out_buffer_size;
   uint8_t* out_buffer;//重采样后的输出，并非原始解码输出
   /**************************/
   /*****sdl播放相关参数*******/
   int out_channels;
   int out_nb_samples;
   double  audio_clk;			// 当前音频时间
   double	audio_pts_duration;	// 记录当前音频帧的持续时间
private:
   void my_swr_init();
   void my_swr_convert(uint8_t* out_buffer, int& out_size);
   void mydecode(AVCodecContext* dec_ctx, AVPacket* pkt, AVFrame* frame, uint8_t* outbuf, int outbuffer_size, int& outbuff_data_len);
public:
    AudioDecoder();
    long AACdecodeInit();

    int AACdecodeFrame(unsigned char** in, unsigned char* out, int* in_data_size, int outbuffer_max_size, int& outbuff_data_len);

    void AACdecodeClose();

    void AACdecodeFlush(unsigned char* out, int outbuffer_max_size, int& outbuff_data_len);
};

#endif // AUDIODECODER_H
