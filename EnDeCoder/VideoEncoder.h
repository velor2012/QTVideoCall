#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H
#include "tool.h"
class VideoEncoder
{
private:

    AVFormatContext *fmtCtx = NULL;
    AVOutputFormat *outFmt = NULL;
    AVStream *vStream = NULL;
    AVCodecContext *codecCtx = NULL;
    AVCodec *codec = NULL;
    AVPacket *pkt;
    AVFrame *picFrame = NULL;

    uint8_t *picture_buf = NULL;
    size_t size;
    int pts_count;
public:
    VideoEncoder();
    int init(int w, int h, int bit_rate = 400000, int fps = 25);
    int encode(char* data, char* out_buff, int& out_len);
    int close();
};

#endif // VIDEOENCODER_H
