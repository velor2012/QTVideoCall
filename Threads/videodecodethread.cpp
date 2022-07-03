#include "videodecodethread.h"
#include <QMessageBox>
#include "Config.h"
#include "MyUDPPacketBuilder.h"
#include <iostream>
#include "mysessionmanager.h"
extern MySessionManager* MSM;
extern Config* cfg;

#define MAX_QUEUE_SIZE 200
VideoDecodeThread* cta = NULL;


VideoDecodeThread::VideoDecodeThread(QObject* parent)
    :MyBaseQThread(parent),
    YUV(NULL),
    RGB_buf(NULL),
    YUV_flag(FALSE),
    frame_buf(NULL),
    out_buf(NULL),
    frame_buf_len(0),
    imgHeight(0),
    imgWidth(0)
{

    //创建解码器
    //TODO;
    //这里需要自动判断使用的是什么编码方式，建议客户端建立连接的时候发送编码方式
    decoder = new VideoDecoder();
    int res = decoder->VideoDecodeInit();
    if (res < 0)
    {
        QMessageBox::warning(nullptr,"提示", "解码器加载失败");
        return;
    }

    //分配拼帧缓存
    frame_buf = (unsigned char*)malloc(MAX_FRAME_SIZE * sizeof(char));
    if (NULL == frame_buf)
    {
        QMessageBox::warning(nullptr, "提示", "内存不足");
        return;
    }

    //分配拼帧缓存
    out_buf = (unsigned char*)malloc(MAX_FRAME_SIZE * sizeof(char));
    if (NULL == out_buf)
    {
        QMessageBox::warning(nullptr, "提示", "内存不足");
        return;
    }

    delay = 0;
    video_fps = 0;
    frame_type = nullptr;
    last_paly_time = 0;
    imgHeight = 0;
    imgWidth = 0;

    init = true;

}

void VideoDecodeThread::task()
{
    if(!MSM || MSM->getStat() != 3) return;
    auto pkt = cfg->rec_video_queue.pop_front();

    if (!pkt)
    {
        Sleep(2);
        return;
    }
    if (finish) return;

    cfg->video_last_timestamp = pkt->timestamp;
    //解码
    int out_len = 0;

    memcpy(frame_buf + frame_buf_len, pkt->data, pkt->datalen);

    frame_buf_len += pkt->datalen;
    unsigned char* frame_buf_bk = frame_buf;
    ////qDebug()<< imgWidth;
    int res = decoder->VideoDecodeFrame(&frame_buf, &frame_buf_len, out_buf, &out_len, &imgWidth, &imgHeight, &frame_type, &video_fps);
    //重置frame_buf的位置
    memmove(frame_buf_bk, frame_buf, frame_buf_len);
    //qDebug() << CUtils::generateLog("视频解码一帧完成", this);
    frame_buf = frame_buf_bk;

    if (res < 1 || imgWidth < 1 || imgHeight < 1)
        return;
    if (out_len < 1) return;

    int frame_size2 = imgWidth * imgHeight * 3 / 2;
    int out_frame_nums = out_len / frame_size2;


    if (!YUV_flag)
    {
        //qDebug() << CUtils::generateLog("视频解码初始化YUV", this);
        //VideoPlayControl(nID, &video_left, &video_top, &video_width, &video_height, imgWidth, imgHeight);
        YUV = (unsigned char*)malloc(imgWidth * imgHeight * 3 / 2 * sizeof(char));
        RGB_buf = (unsigned char*)malloc(imgWidth * imgHeight * 3 * sizeof(char));
        YUV_flag = TRUE;

        //获取原始尺寸显示视频时对话框的尺寸
        cfg->imgDecWidth = imgWidth;
        cfg->imgDecHeight = imgHeight;
        //qDebug() << CUtils::generateLog("视频解码初始化YUV结束", this);
    }


    if (cfg->audio_last_timestamp != 0 && cfg->video_last_timestamp != 0) {
        //视频放慢了
        if (cfg->video_last_timestamp < cfg->audio_last_timestamp) {
            delay = video_fps > 0 ? (1000 / video_fps) - 5 : 10; // 改为解码出来的帧率
        }
        //视频放快了
        else if (cfg->video_last_timestamp > cfg->audio_last_timestamp) {
            delay = video_fps > 0 ? (1000 / video_fps) + 5 : 10; // 改为解码出来的帧率
        }
    }
    //qDebug() << CUtils::generateLog("视频解码同步完成", this);
    while (getCurTime() - last_paly_time < delay && !finish) {
        Sleep(1);
    }

    last_paly_time = getCurTime();


    for (int i = 0; i < out_frame_nums; ++i) {

        char* temp_y = (char*)malloc(imgWidth * imgHeight * 3 / 2 * sizeof(char));
        memmove(temp_y, out_buf + i * frame_size2, frame_size2);
        //显示
        cfg->dec_yuv_que.push(temp_y);
    }
    //qDebug() << CUtils::generateLog("视频解码显示完成", this);
}

VideoDecodeThread::~VideoDecodeThread() {

    if (NULL != frame_buf)
    {
        free(frame_buf);
        frame_buf = NULL;
    }
    if (NULL != out_buf)
    {
        free(out_buf);
        out_buf = NULL;
    }
    if (NULL != YUV)
    {
        free(YUV);
        YUV = NULL;
    }
    if (NULL != RGB_buf)
    {
        free(RGB_buf);
        RGB_buf = NULL;
    }
    if (handle > 0)
    {
        decoder->VideoDecodeClose();
        delete decoder;
    }
}
