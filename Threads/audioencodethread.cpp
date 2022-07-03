#include "audioencodethread.h"
#include "AudioEncodeThread.h"
#include <QDebug>
#include <mainwindow.h>
#include "Config.h"
#include "mysessionmanager.h"
#include "MyUDPPacket.h"
extern Config* cfg;
extern MySessionManager* MSM;
#define MAX_SIZE 1024000
FILE* audiofile;
AudioEncodeThread* cta = NULL;

//采集到的视频图像回调
LRESULT CALLBACK AudioCaptureCallback1(uint8_t* buff, int audio_out_buffer_size, INT64 lTimeStamp)
{
    if (!cta) return -1;

//    char* t = (char*)malloc(audio_out_buffer_size);
//    memcpy(t, buff, audio_out_buffer_size);


//    cfg->pcm_que.push(t);

//    cfg->pcm_pkt_l = audio_out_buffer_size;

    uint8_t* queue_buff = (unsigned char*)malloc(audio_out_buffer_size);
    memcpy(queue_buff, buff, audio_out_buffer_size);

    if (cta->audio_buff_remain_size == -1) {
        cta->audio_buff_remain_size = audio_out_buffer_size;
    }

    cta->mquue.push(queue_buff);


    return 0;
}

AudioEncodeThread::AudioEncodeThread(QObject* parent)
    :MyBaseQThread(parent),
    sequenceNumber_audio(0),
    mic_stream_buf(nullptr),
    encoder(nullptr),
    audio_buff_remain_size(-1),
    audio_out_buff(nullptr),
    audio_stream_buff(nullptr),
    audio_stream_buff_l(0)
{
    //...
    cta = this;


    //edited by cwy
    //创建音频编码器
    encoder = new AudioEncoder();
    int res = encoder->init(128000);
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


    audio_buffer_size = encoder->getFrameSize();
//    qDebug() << audio_buffer_size;
    audio_stream_buff = (unsigned char*)malloc(MAX_SIZE);
    audio_out_buff = (unsigned char*)malloc(MAX_SIZE);
    audio_encode_count = 0;

    audio_buff_remain_size = -1;

    cfg->CML->SetAudioCaptureCB(AudioCaptureCallback1);
    init = true;

//    audiofile = fopen("D:\\haisi\\VideoCalling\\VideoCall\\out.aac", "wb+");
//    if(!audiofile){
//            return;
//        }
}

void AudioEncodeThread::task()
{
//    if(!MSM || MSM->getStat() != 3) return;
    mic_stream_buf = mquue.pop_front();
    if(!mic_stream_buf) return;


    int total_put_in_size = 0;
//                        882                      600
    int remain = audio_buff_remain_size + audio_stream_buff_l;
    memcpy(audio_stream_buff + audio_stream_buff_l, mic_stream_buf, audio_buff_remain_size);

    int offset = 0;
    int out_buffer_len = 0;
    while (remain > audio_buffer_size - 1) {
        int encode_len = 0;

        encoder->encode(audio_stream_buff + offset, audio_out_buff + out_buffer_len, audio_encode_count, &encode_len);

        out_buffer_len += encode_len;
        offset += audio_buffer_size;
        remain -= audio_buffer_size;

    }

    memmove(audio_stream_buff, audio_stream_buff + offset, remain);
    audio_stream_buff_l = remain;
    free(mic_stream_buf);
    mic_stream_buf = nullptr;

    if (out_buffer_len > 0) {
        //RTP发送
        //qDebug() << CUtils::generateLog("音频帧发送开始", this);
        send_to_queue(audio_out_buff, out_buffer_len, sequenceNumber_audio);

        audio_encode_count++;
    }

}
bool AudioEncodeThread::send_to_queue(unsigned char* Stream, int stream_l, int& seq) {
    long long t = getCurTime();
    while(stream_l > 0){
        int cp_l = stream_l > MAX_PKT_SIZE - PKT_HEADER_LEN ? MAX_PKT_SIZE - PKT_HEADER_LEN : stream_l;
//        fwrite(Stream, 1, cp_l, audiofile);
        MyUDPPacket *pkt = new MyUDPPacket;
        if(!cfg->mute){
            memcpy(pkt->data, Stream, cp_l);  //拷贝码流分片数据
            pkt->datalen = cp_l;
        }else{
            pkt->datalen = 0;
        }
        pkt->seq = seq;
        pkt->type = 10;
        pkt->sessiongID = MSM->getSessionID();
        pkt->timestamp = t;
        seq++;
        cfg->send_que.push(pkt);
//        cfg->rec_audio_queue.push(pkt);
        Stream += cp_l;
        stream_l -= cp_l;
    }
    return true;

}
AudioEncodeThread::~AudioEncodeThread() {
//    fclose(audiofile);
    if (encoder){
        encoder->close();
        delete encoder;
    }

    if (audio_stream_buff) {
        free(audio_stream_buff);
        audio_stream_buff = nullptr;
    }
    if (audio_out_buff) {
        free(audio_out_buff);
        audio_out_buff = nullptr;
    }
    cta = nullptr;
}
