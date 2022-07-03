#ifndef VIDEODECODETHREAD_H
#define VIDEODECODETHREAD_H

#include <deque>
#include "MyBaseQThread.h"
#include <QFile>
#include "EnDeCoder/videodecoder.h"

class VideoDecodeThread : public MyBaseQThread
{
    Q_OBJECT

public:
    VideoDecodeThread(QObject* parent = nullptr);
    ~VideoDecodeThread();
private:

    int frame_size;
    unsigned char* frame_buf;
    int frame_buf_len;
    unsigned char* out_buf;
    VideoDecoder* decoder;

    int imgWidth;
    int imgHeight;
    bool YUV_flag;
    unsigned char* YUV;
    unsigned char* RGB_buf;
    int video_fps;
    char* frame_type;

    int delay;
    long long last_paly_time;

protected:
    void task();
};


#endif // VIDEODECODETHREAD_H
