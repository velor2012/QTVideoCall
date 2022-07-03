#ifndef AUDIOENCODETHREAD_H
#define AUDIOENCODETHREAD_H
#include <QThread>
#include <queue>
#include <iostream>
#include <mutex>
#include "MyBaseQThread.h"
#include "EnDeCoder/audioencoder.h"
#include "MyQueue.h"
//音频包以20ms采样，1s为44100hz，因此每次采样得到的PCM样本为882个
class AudioEncodeThread : public MyBaseQThread
{
    Q_OBJECT

public:
    AudioEncodeThread(QObject* parent = nullptr);
    ~AudioEncodeThread();
    bool send_to_queue(unsigned char* Stream, int stream_l, int& seq);
    MyQueue<std::uint8_t*> mquue;
    int audio_buff_remain_size;
private:
    AudioEncoder* encoder;
    //add by cwy
    int sequenceNumber_audio;
    unsigned char* audio_stream_buff;
    int audio_stream_buff_l;
    unsigned char* audio_out_buff;
    int audio_buffer_size;
    int audio_encode_count;
    unsigned char* stream_flag_buf;

    //add by cwy
    bool get_frame;//从摄像头中获取了一帧
    unsigned char* mic_stream_buf;


    //多少个音频包合成一个发
    int compress_nums;
protected:
    void task();
};
#endif // AUDIOENCODETHREAD_H
