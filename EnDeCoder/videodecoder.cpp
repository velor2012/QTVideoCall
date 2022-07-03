#include "videodecoder.h"

VideoDecoder::VideoDecoder()
{

}

#include "EnDeCoder/videodecoder.h"
#include <string.h>

// 0 h264
// 1 h265
long VideoDecoder::VideoDecodeInit()
{

    // 查找所需解码器;
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        return -1;
    }
    // 配置上下文;
    c = avcodec_alloc_context3(codec);
    if (!c) {
        return -1;
    }
    //add by cwy 2020/9/28
    codecParameters = avcodec_parameters_alloc();
    if (avcodec_parameters_from_context(codecParameters, c) < 0) {
        printf("Failed to copy avcodec parameters from codec context.\n");
        avcodec_parameters_free(&codecParameters);
        avcodec_free_context(&c);
        return -1;
    }
    //end add
        // 初始化解析器;
    pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264);
    if (!pCodecParserCtx) {
        return -1;
    }
    // 开启解码器;
    if (avcodec_open2(c, codec, NULL) < 0) {
        return -1;
    }
    // 分配帧内存;
    picture = av_frame_alloc();
    // 初始化数据包;
    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);

    //const char* outfilename = ".\\decodeh264.yuv";
    //file = fopen(outfilename, "wb");
    return 0;
}

static int copyFrame(AVFrame* frame, unsigned char* output_buff) {

    int luma_size = frame->width * frame->height;
    int chroma_size = luma_size / 4;
    for (int i = 0; i < frame->height; i++) {
        memcpy(output_buff + i * frame->width, frame->data[0] + i * frame->linesize[0], frame->width);
    }

    int loop = frame->height / 2;
    int len_uv = frame->width / 2;

    for (int i = 0; i < loop; i++) {
        memcpy(output_buff + luma_size + i * len_uv, frame->data[1] + i * frame->linesize[1], len_uv);

    }

    for (int i = 0; i < loop; i++) {
        memcpy(output_buff + luma_size + chroma_size + i * len_uv, frame->data[2] + i * frame->linesize[2], len_uv);

    }
    return 1;

}

static int decode(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt, unsigned char* output_buff, int* output_len)
{
    int ret;
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding\n");
        return -1;
    }
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return -1;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            return -1;
        }
        //printf("saving frame %3d\n", dec_ctx->frame_number);
        //fflush(stdout);

        *output_len = frame->width * frame->height * 3 / 2;
        copyFrame(frame, (unsigned char* )output_buff);
        //fwrite(output_buff, 1, *output_len, file);
        return 1;
    }
}


int VideoDecoder::VideoDecodeFrame(unsigned char** in, int* nallen, unsigned char* output_buff, int* output_len,
    int* width, int* height, char** frame_type, int* fps)
{
    int consume = -1;
    while ((*nallen) > 0) {
        //consumed_bytes = avcodec_decode_video2(c, picture, &got_picture, in, nallen);
        int ret = av_parser_parse2(pCodecParserCtx, c, &pkt->data, &pkt->size,
            *in, *nallen, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        *nallen -= ret;
        *in += ret;
        if (pkt->size) {
            if (decode(c, picture, pkt, output_buff, output_len)) {
                *fps = c->framerate.num / c->framerate.den;
                *width = c->width;
                *height = c->height;
                switch (picture->pict_type)
                {
                case(AV_PICTURE_TYPE_I):
                    *frame_type = (char*)"I";
                    break;
                case(AV_PICTURE_TYPE_P):
                    *frame_type =  (char*)"P";
                    break;
                default:
                    *frame_type =  (char*)"other";
                    break;
                }
                //2022 3 /27添加
                //modified by cwy
                consume = 1;
                return consume;
            }
            else {
                return 2;
            }
        }
    }
    return consume;
}

int VideoDecoder::VideoDecodeClose()
{

    // 解码器相关;
    if (c) {
        // pCodec
        avcodec_close(c);
        av_free(c);
    }
    if (pCodecParserCtx) {
        av_parser_close(pCodecParserCtx);
    }
    if (picture) {
        av_frame_free(&picture);
    }
    if (codecParameters) {
        avcodec_parameters_free(&codecParameters);
    }
    //fclose(file);
    return 1;
}
