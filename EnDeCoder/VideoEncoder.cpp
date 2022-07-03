#include "VideoEncoder.h"

VideoEncoder::VideoEncoder():
    pts_count(0),
    fmtCtx(nullptr),
    codecCtx(nullptr),
    codec(nullptr),
    vStream(nullptr)
{

}
int VideoEncoder::init(int w, int h, int bit_rate, int fps){

    //初始化输出环境

    const char *outFile = ".\enc.h264";
    if(avformat_alloc_output_context2(&fmtCtx,NULL,NULL,outFile)<0){
        printf("Cannot alloc output file context.\n");
        return -1;
    }
    pkt = av_packet_alloc(); //创建已编码帧
    outFmt=fmtCtx->oformat;

    //[3]!打开输出文件
    if(avio_open(&fmtCtx->pb,outFile,AVIO_FLAG_READ_WRITE)<0){
        printf("output file open failed.\n");
        return -1;
    }

    //[4]!创建h264视频流，并设置参数
       vStream = avformat_new_stream(fmtCtx,codec);
        if(vStream ==NULL){
            printf("failed create new video stream.\n");
            return -1;
        }
        vStream->time_base.den=25;
        vStream->time_base.num=1;
        //[4]!

        //[5]!编码参数相关
        AVCodecParameters *codecPara= fmtCtx->streams[vStream->index]->codecpar;
        codecPara->codec_type=AVMEDIA_TYPE_VIDEO;
        codecPara->width=w;
        codecPara->height=h;
        //[5]!

        //[6]!查找编码器
        codec = avcodec_find_encoder(outFmt->video_codec);
        if(codec == NULL){
            printf("Cannot find any endcoder.\n");
            return -1;
        }
        //[6]!

        //[7]!设置编码器内容
        codecCtx = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(codecCtx,codecPara);
        if(codecCtx==NULL){
            printf("Cannot alloc context.\n");
            return -1;
        }

        codecCtx->codec_id      = outFmt->video_codec;
        codecCtx->codec_type    = AVMEDIA_TYPE_VIDEO;
        codecCtx->pix_fmt       = AV_PIX_FMT_YUV420P;
        codecCtx->width         = w;
        codecCtx->height        = h;
        codecCtx->time_base.num = 1;
        codecCtx->time_base.den = fps;
        if(bit_rate > 0) codecCtx->bit_rate      = bit_rate;
        codecCtx->gop_size      = 12;

        if (codecCtx->codec_id == AV_CODEC_ID_H264) {
            codecCtx->qmin      = 10;
            codecCtx->qmax      = 51;
            codecCtx->qcompress = (float)0.6;
        }

        av_opt_set(codecCtx->priv_data, "preset", "fast", 0);
        av_opt_set(codecCtx->priv_data, "tune", "zerolatency", 0);

        //[8]!打开编码器
        if(avcodec_open2(codecCtx,codec,NULL)<0){
            printf("Open encoder failed.\n");
            return -1;
        }
        //[8]!

        av_dump_format(fmtCtx,0,outFile,1);//输出 输出文件流信息

        //初始化帧
        picFrame         = av_frame_alloc();
        picFrame->width  = codecCtx->width;
        picFrame->height = codecCtx->height;
        picFrame->format = codecCtx->pix_fmt;
        size            = (size_t)av_image_get_buffer_size(codecCtx->pix_fmt,codecCtx->width,codecCtx->height,1);
        picture_buf     = (uint8_t *)av_malloc(size);
        av_image_fill_arrays(picFrame->data,picFrame->linesize,
                             picture_buf,codecCtx->pix_fmt,
                             codecCtx->width,codecCtx->height,1);

        //[9] --写头文件
        int ret = avformat_write_header(fmtCtx, NULL);
        //[9]

        int      y_size = codecCtx->width * codecCtx->height;
        av_new_packet(pkt, (int)(size * 3));
    return 0;
}

int VideoEncoder::encode(char* data, char* out_buff, int& out_len){
    //fwrite(yuv_frame, 1, malloc_usable_size(yuv_frame), fp_yuv);


    picFrame->data[0] = (uint8_t*)data;
    picFrame->data[1] = (uint8_t*)data + picFrame->width * picFrame->height;
    picFrame->data[2] = (uint8_t*)data + picFrame->width * picFrame->height * 5 / 4;
    // AVFrame pts;

    picFrame->pts = pts_count++;
    // ����;
    int ret;
    ret = avcodec_send_frame(codecCtx, picFrame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        return -1;
    }

    int count = 0;
    while (ret >= 0) {
        ret = avcodec_receive_packet(codecCtx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return 0;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            return -1;
        }

        memcpy(out_buff + count * pkt->size, pkt->data, pkt->size);
        out_len += pkt->size;
        count++;

        // parpare packet for muxing
          pkt->stream_index = vStream->index;
          av_packet_rescale_ts(pkt, codecCtx->time_base, vStream->time_base);
          pkt->pos = -1;
          int ret     = av_interleaved_write_frame(fmtCtx, pkt);


        av_packet_unref(pkt);
    }
    return 0;
}

int VideoEncoder::close(){
    /* flush the encoder */
    /* add sequence end code to have a real MPEG file */
    //[12] --写文件尾
    av_write_trailer(fmtCtx);

    //释放内存
    av_packet_free(&pkt);
    avcodec_close(codecCtx);
    av_free(picFrame);
    av_free(picture_buf);

    if(fmtCtx){
        avio_close(fmtCtx->pb);
        avformat_free_context(fmtCtx);
    }

    return 0;
}
