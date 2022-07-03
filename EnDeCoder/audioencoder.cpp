#include "audioencoder.h"


//AAC有两种封装格式，分别是ADIF ADTS，多与流媒体一般使用ADTS格式。见：
//http://www.jianshu.com/p/839b11e0638b aac freqIdx

char aac_adts_header[7] = { 0 };
int chanCfg = 2;            //MPEG-4 Audio Channel Configuration. 1 Channel front-center

static int init_aac_header() {
    int profile = 2;   //AAC SSR
    int freqIdx = 4;   //44100HZ

    aac_adts_header[0] = (char)0xFF;      // 11111111     = syncword
    aac_adts_header[1] = (char)0xF1;      // 1111 1 00 1  = syncword MPEG-2 Layer CRC
    aac_adts_header[2] = (char)(((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
    aac_adts_header[6] = (char)0xFC;

    return 0;
}

static int write_aac_header(uint8_t* buff, AVPacket* pkt) {
    aac_adts_header[3] = (char)(((chanCfg & 3) << 6) + ((7 + pkt->size) >> 11));
    aac_adts_header[4] = (char)(((7 + pkt->size) & 0x7FF) >> 3);
    aac_adts_header[5] = (char)((((7 + pkt->size) & 7) << 5) + 0x1F);

    memcpy(buff, aac_adts_header, 7);
    return 0;
}

AudioEncoder::AudioEncoder()
{

}
int AudioEncoder::init(int bit_rate){
    init_aac_header();

    const char* outputfilename = ".\\test.aac";
    //初始化输出环境

    ofmt_ctx = avformat_alloc_context();
    oformat = av_guess_format(NULL, outputfilename, NULL);

    if (oformat == NULL) {
        av_log(NULL, AV_LOG_ERROR, "fail to find the output format\n");
        return -1;
    }
    if (avformat_alloc_output_context2(&ofmt_ctx, oformat, oformat->name, outputfilename) < 0) {
        av_log(NULL, AV_LOG_ERROR, "fail to alloc output context\n");
        return -1;
    }
    out_stream = avformat_new_stream(ofmt_ctx, NULL);
    if (out_stream == NULL) {
        av_log(NULL, AV_LOG_ERROR, "fail to create new stream\n");
        return -1;
    }
    out_stream = out_stream;

    //设置AAC编码格式：

    pCodecCtx = out_stream->codec;

    pCodecCtx->codec_id = oformat->audio_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
    pCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP; //其他会出错
    pCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);
    pCodecCtx->sample_rate = 44100;
    if(bit_rate != 0) pCodecCtx->bit_rate = bit_rate;

    //打开编码器并向输出文件中写入文件头信息
    AVCodec* pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        av_log(NULL, AV_LOG_ERROR, "fail to find codec\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "fail to open codec\n");
        return -1;
    }
    av_dump_format(ofmt_ctx, 0, outputfilename, 1);
    if (avio_open(&ofmt_ctx->pb, outputfilename, AVIO_FLAG_WRITE) < 0) {
        av_log(NULL, AV_LOG_ERROR, "fail to open output\n");
        return -1;
    }
    if (avformat_write_header(ofmt_ctx, NULL) < 0) {
        av_log(NULL, AV_LOG_ERROR, "fail to write header");
        return -1;
    }

    //设置一些参数，需要将pcm raw data压缩为aac格式
    pframe = av_frame_alloc();
    pframe->channels = pCodecCtx->channels;
    pframe->format = pCodecCtx->sample_fmt;
    pframe->nb_samples = pCodecCtx->frame_size;
    //从文件中读取原始数据，缓冲区
    out_buffer_size = av_samples_get_buffer_size(NULL, pCodecCtx->channels, pCodecCtx->frame_size, pCodecCtx->sample_fmt, 1);
    out_buffer = (uint8_t*)av_malloc(out_buffer_size);


    avcodec_fill_audio_frame(pframe, pCodecCtx->channels, pCodecCtx->sample_fmt, (const uint8_t*)out_buffer, out_buffer_size, 1);

    //新版本需要使用到转换参数，将读取的数据转换成输出的编码格式
    data = (uint8_t**)av_calloc(pCodecCtx->channels, sizeof(*data));
    int res =av_samples_alloc(data, NULL, pCodecCtx->channels, pCodecCtx->frame_size, pCodecCtx->sample_fmt, 1);

    pSwrCtx = swr_alloc();
    swr_alloc_set_opts(pSwrCtx, pCodecCtx->channel_layout, pCodecCtx->sample_fmt, pCodecCtx->sample_rate,
        pCodecCtx->channel_layout, AV_SAMPLE_FMT_S16, 44100, 0, NULL);
    int rt = swr_init(pSwrCtx);
    //需要使用AVPacket进行压缩储存
    pkt = av_packet_alloc();
    av_new_packet(pkt, out_buffer_size);
    pkt->data = NULL;
    pkt->size = 0;

    frame_size = pframe->nb_samples * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * pframe->channels;

    return 0;
}

int AudioEncoder::encode(unsigned char* in, unsigned char* out, int count, int* outbuff_data_len){

    //读取的长度要 和原始数据的采样率，采样格式以及通道有关 如果size设置的不对，会导致音频错误
    memcpy(out_buffer, in, frame_size);

    swr_convert(pSwrCtx, data, pCodecCtx->frame_size, (const uint8_t**)pframe->data, pframe->nb_samples);
    //转换后的数据大小与采样率和采样格式有关
    int size = pCodecCtx->frame_size * av_get_bytes_per_sample(pCodecCtx->sample_fmt);
    memcpy(pframe->data[0], data[0], size);
    memcpy(pframe->data[1], data[1], size);
    pframe->pts = count * 100;
    //编码写入
    if (avcodec_send_frame(pCodecCtx, pframe) < 0) {
        printf("fail to send frame\n");
        return -1;
    }
    //读取编码好的数据
    if (avcodec_receive_packet(pCodecCtx, pkt) >= 0) {
        pkt->stream_index = out_stream->index;
        av_log(NULL, AV_LOG_INFO, "write %d frame\n", count);
        av_write_frame(ofmt_ctx, pkt);
    }
    if (pkt->size > 0) {
        write_aac_header(out, pkt);
        memcpy(out + 7, pkt->data, pkt->size);
        *outbuff_data_len += pkt->size + 7;
    }
    av_packet_unref(pkt);

    return 0;
}

int AudioEncoder::flush(unsigned char* out, int* outbuff_data_len) {
    int stream_index = out_stream->index;
    if (!(ofmt_ctx->streams[stream_index]->codec->codec->capabilities & AV_CODEC_CAP_DELAY)) {
        return 0;
    }
    int got_fame = 0;
    AVPacket* pkt = av_packet_alloc();


    pkt->data = NULL;
    pkt->size = 0;
    av_init_packet(pkt);
    AVCodecContext* tmpAvCodecCtx = ofmt_ctx->streams[stream_index]->codec;
    int ret = (avcodec_send_frame(tmpAvCodecCtx, NULL) < 0 || (got_fame = avcodec_receive_packet(tmpAvCodecCtx, pkt)) < 0);
    //int ret = avcodec_encode_audio2(ofmt_ctx->streams[stream_index]->codec, pkt, NULL, &got_fame);
    if (ret < 0) {
        return -1;
    }
    if (got_fame == 0) {
        return -1;
    }
    ret = av_write_frame(ofmt_ctx, pkt);
    if (ret < 0) {
        return -1;
    }
    if (pkt->size > 0) {
        write_aac_header(out, pkt);
        memcpy(out + 7, pkt->data, pkt->size);
        *outbuff_data_len += pkt->size + 7;
    }
    av_packet_free(&pkt);
    return 1;
}

int AudioEncoder::close(){
    av_write_trailer(ofmt_ctx);
    av_packet_free(&pkt);
    swr_free(&pSwrCtx);
    av_free(out_buffer);
    av_frame_free(&pframe);

    avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    return 0;
}
