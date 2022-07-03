#ifndef AudioEncoder_H
#define AudioEncoder_H
#include "tool.h"
class AudioEncoder
{
private:
    AVFormatContext* ofmt_ctx;
    AVOutputFormat* oformat;
    AVCodecContext* pCodecCtx;
    AVStream* out_stream;
    AVFrame* pframe;
    AVPacket* pkt;
    SwrContext* pSwrCtx;
    uint8_t* out_buffer;
    int out_buffer_size;
    uint8_t** data;
    int frame_size;
public:
    AudioEncoder();
    int getFrameSize(){return frame_size;}
    int init(int bit_rate = 0);
    int encode(unsigned char* in, unsigned char* out, int count, int* outbuff_data_len);
    int close();
    int flush(unsigned char* out, int* outbuff_data_len);
};

#endif // AudioEncoder_H
