#include "videoencodethread.h"
#include "Config.h"
#include <QMessageBox>
#include "MyUDPPacketBuilder.h"
#include "mysessionmanager.h"
extern MySessionManager* MSM;
VideoEncodeThread* ct1 = NULL;
extern Config* cfg;

//采集到的视频图像回调
LRESULT CALLBACK VideoCaptureCallback1(uint8_t* buff, INT64 lTimeStamp)
{
    //qDebug() << CUtils::generateLog("进入视频回调", ct1);
    if (!ct1) return -1;
    //qDebug() << CUtils::generateLog("获取一帧数据 ", ct1);
    ct1->getFrame = true;
    memcpy(ct1->EncYUV, buff, ct1->width * ct1->height * 3 / 2);
    return 0;
}
VideoEncodeThread::VideoEncodeThread(QObject* parent)
    :MyBaseQThread(parent),
    EncYUV(NULL),
    EncStream(NULL),
    frame_count(0),
    seq(0),
    entry_time(0)
{
    ct1 = this;
    width = cfg->w;
    height = cfg->h;

    //分配yuv缓存
    EncYUV = (unsigned char*)malloc(MAX_FRAME_SIZE * sizeof(char));

    //分配码流缓存
    EncStream = (unsigned char*)malloc(MAX_FRAME_SIZE * sizeof(char));

    //创建编码器
    encoder = new VideoEncoder();
    int res = encoder->init(width, height, cfg->bitrate, cfg->fps);
    if (res < 0)
    {
        QMessageBox::warning(nullptr, "提示", "构建编码器失败");
        qDebug() << "构建编码器失败";
        init = false;
        return;
    }

    if(!cfg->CML){
        QMessageBox::warning(nullptr,"提示", "摄像头设备尚未打开");
        qDebug() << "摄像头设备尚未打开";
        init = false;
        return;
    }
    cfg->CML->SetVideoCaptureCB(VideoCaptureCallback1);
    init = true;
}


void VideoEncodeThread::task() {
    if(!MSM || MSM->getStat() != 3) return;
    double delay = 1000 / (double)cfg->fps;
    //帧率控制
    if (entry_time != 0 && (getCurTime() - entry_time < frame_count * delay))
    {
        Sleep(10);
        return;
    }
    //qDebug() << CUtils::generateLog("帧率控制结束", this);
    if (entry_time == 0) entry_time = getCurTime();

    //编码并传输

    if (!getFrame) {
        return;
    }
    getFrame = false;


    //帧号累加
    frame_count++;




    //显示原始视频
    char* display_yuv = (char*)malloc(width * height * 3 / 2 * sizeof(char));


    memmove(display_yuv, EncYUV, width * height * 3 / 2);
    cfg->enc_yuv_que.push(display_yuv);

    int stream_l = 0;
    //modified by cwy
    encoder->encode((char*)EncYUV, (char*)EncStream, stream_l);
    if (stream_l <= 0) return;

    //放入队列中
    send_to_queue(EncStream, stream_l, seq);
    Sleep(20);
}

bool VideoEncodeThread::send_to_queue(unsigned char* Stream, int stream_l, int& seq)
{
    long long t = getCurTime();
    while(stream_l > 0){
        int cp_l = stream_l > MAX_PKT_SIZE - PKT_HEADER_LEN ? MAX_PKT_SIZE - PKT_HEADER_LEN : stream_l;
        MyUDPPacket *pkt = new MyUDPPacket;
        memcpy(pkt->data, Stream, cp_l);  //拷贝码流分片数据
        pkt->datalen = cp_l;
        pkt->seq = seq;
        pkt->type = 9;
        pkt->sessiongID = MSM->getSessionID();
        pkt->timestamp = t;
        seq++;
        cfg->send_que.push(pkt);
        Stream += cp_l;
        stream_l -= cp_l;
    }
    return true;
}

VideoEncodeThread::~VideoEncodeThread() {

    //关闭编码器
    if (encoder)
    {
        encoder->close();
        delete(encoder);
    }

    //缓存释放
    if (NULL != EncYUV)
    {
        free(EncYUV);
        EncYUV = NULL;
    }
    if (NULL != EncStream)
    {
        free(EncStream);
        EncStream = NULL;
    }

    ct1 = nullptr;

}
