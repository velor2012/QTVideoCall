#include "audiodecoder.h"
#define MAX_AUDIO_FRAME_SIZE 192000 //采样率：1 second of 48khz 32bit audio
AudioDecoder::AudioDecoder():
    codecContext(nullptr),
    parserContext(nullptr),
    formatContext(nullptr),
   decoded_frame(nullptr),
   au_convert_ctx(nullptr)
{

}

long AudioDecoder::AACdecodeInit()
{
    const AVCodec* codec;


    decoded_frame = av_frame_alloc();
    //decoded_frame->format = AV_CODEC_ID_PCM_S16LE;
    pkt = av_packet_alloc();
    codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        return -1;
    }
    parserContext = av_parser_init(codec->id);
    if (!parserContext) {
        fprintf(stderr, "Parser not found\n");
        return -1;
    }

    codecContext = avcodec_alloc_context3(codec);
    if (!codecContext) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        return -1;
    }
    /* open it */
    if (avcodec_open2(codecContext, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return -1;
    }

    return 0;
}

//初始化重采样器以及SDL需要的参数
void AudioDecoder::my_swr_init() {


    out_sample_fmt = AV_SAMPLE_FMT_S16; // 输出的音频格式
    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO; // 双声道输出
    out_channels = av_get_channel_layout_nb_channels(out_channel_layout);

    out_sample_rate = 44100; // 采样率
    int64_t in_channel_layout = av_get_default_channel_layout(codecContext->channels); //输入通道数
    au_convert_ctx = swr_alloc(); // 初始化重采样结构体
    au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, out_channel_layout, AV_SAMPLE_FMT_S16, out_sample_rate,
        in_channel_layout, codecContext->sample_fmt, codecContext->sample_rate, 0, NULL); //配置重采样率
    swr_init(au_convert_ctx); // 初始化重采样率

    out_nb_samples = codecContext->frame_size;
    out_buffer_size = av_samples_get_buffer_size(NULL, codecContext->channels, codecContext->frame_size, out_sample_fmt, 1);
//    out_buffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);

}
void AudioDecoder::my_swr_convert(uint8_t* out_buffer, int& out_size) {
    if (au_convert_ctx == NULL) {
        my_swr_init();
    }

    swr_convert(au_convert_ctx, &out_buffer, out_buffer_size, // 对音频的采样率进行转换
        (const uint8_t**)decoded_frame->data, codecContext->frame_size);
    out_size = out_buffer_size;
    // -------------------------------- 更新音频时间戳 ----------------------------------- //
    if (decoded_frame->pts != AV_NOPTS_VALUE) {
        audio_clk = av_q2d(codecContext->time_base) * decoded_frame->pts; /* time_base 是一个分数 av_q2d是一个将分数转换成小数的函数。
                                                                                            在此处是用pts乘以time_base得到时间戳 */
    }
    audio_pts_duration = av_q2d(codecContext->time_base) * decoded_frame->pkt_duration; // 更新音频帧的持续时间
    // ------------------------------ end ------------------------------------- //
}

void AudioDecoder::mydecode(AVCodecContext* dec_ctx, AVPacket* pkt, AVFrame* frame, uint8_t* outbuf, int outbuffer_size, int& outbuff_data_len) {
    int i, ch;
    int ret, data_size;

    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error submitting the packet to the decoder\n");
        return;

    }

    /* read all the output frames (in general there may be any number of them */
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            return;
        }
        data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        if (data_size < 0) {
            /* This should not occur, checking just for paranoia */
            fprintf(stderr, "Failed to calculate data size\n");
            return;

        }
        if (outbuff_data_len + frame->nb_samples * dec_ctx->channels * data_size > outbuffer_size) {
            fprintf(stderr, "Error! outBuffer too small wihle decoding audio!\n");
            return ;
        }
//        int offset = 0;
        //实时播放不需要这段
        //for (i = 0; i < frame->nb_samples; i++)
        //	for (ch = 0; ch < dec_ctx->channels; ch++) {
        //		memcpy(outbuf + offset, frame->data[ch] + data_size * i, data_size);
        //		offset += data_size;
        //	}
        my_swr_convert(outbuf, outbuff_data_len);
//        outbuff_data_len += frame->nb_samples * dec_ctx->channels * data_size;
    }
}

int AudioDecoder::AACdecodeFrame(unsigned char** in, unsigned char* out, int* in_data_size, int outbuffer_max_size, int& outbuff_data_len)
{
    AVCodecParserContext* parser = parserContext;
    AVCodecContext* c = codecContext;
    int ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
        *in, *in_data_size,
        AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
    if (ret < 0) {
        fprintf(stderr, "Error while parsing\n");
        return -1;

    }
    *in += ret;
    *in_data_size -= ret;

    if (pkt->size) {
        mydecode(c, pkt, decoded_frame, out, outbuffer_max_size, outbuff_data_len);
    }
    return 0;
}

void AudioDecoder::AACdecodeClose()
{
    pkt->data = NULL;
    pkt->size = 0;
    if(codecContext) avcodec_free_context(&codecContext);
    if(parserContext) av_parser_close(parserContext);
    if(decoded_frame) av_frame_free(&decoded_frame);
    if(pkt) av_packet_free(&pkt);
}

void AudioDecoder::AACdecodeFlush(unsigned char* out, int outbuffer_max_size, int& outbuff_data_len) {
    AVCodecParserContext* parser = parserContext;
    AVCodecContext* c = codecContext;
    AVPacket* pkt = pkt;
    /* flush the decoder */
    pkt->data = NULL;
    pkt->size = 0;
    mydecode(c, pkt, decoded_frame, out, outbuffer_max_size, outbuff_data_len);
}
