
#pragma once
#include "MyBaseQThread.h"
#include <iostream>
#include "EnDeCoder/VideoEncoder.h"
class VideoEncodeThread : public MyBaseQThread
{
    Q_OBJECT
private:
    VideoEncoder* encoder;
    FILE* stream_fp; //码流

public:
    unsigned char* EncYUV;   //原始一帧YUV数据
    unsigned char* EncStream;   //原始一帧YUV数据
    int width;
    int height;
    int frame_count;
    int seq;
    bool getFrame; //通知编码器已经从摄像头获取一帧输出;
    long long entry_time = 0;
    VideoEncodeThread(QObject* parent = nullptr);
    ~VideoEncodeThread();

    //modify by cwy
    bool send_to_queue(unsigned char* Stream, int stream_l, int& seq);

protected:
    void task();

};
