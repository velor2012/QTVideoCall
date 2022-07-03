#ifndef VIDEODECODER_H
#define VIDEODECODER_H
#include "tool.h"

class VideoDecoder
{
private:
    struct AVCodec* codec;
    struct AVCodecContext* c;
    struct AVFrame* picture;
    struct AVPacket* pkt;
    struct AVCodecParserContext* pCodecParserCtx;
    struct AVCodecParameters* codecParameters;
public:
    VideoDecoder();

    long VideoDecodeInit();

    int VideoDecodeFrame(unsigned char** in, int* nallen, unsigned char* output_buff, int* output_len,
        int* width, int* height, char** frame_type, int* fps);


    int VideoDecodeClose();
};

#endif // VIDEODECODER_H
