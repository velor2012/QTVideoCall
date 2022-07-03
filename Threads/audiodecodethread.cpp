#include "audiodecodethread.h"

#include <QDebug>
#include "Config.h"
#include <QMessageBox>
#define MAX_QUEUE_SIZE 200
#define MAX_SIZE 102400
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096
#define BUFFER_PADDING_SIZE 64

#include "mysessionmanager.h"
extern MySessionManager* MSM;
extern Config* cfg;
AudioDecodeThread::AudioDecodeThread(QObject* parent)
    :MyBaseQThread(parent),
    decoder(nullptr),
    rec_fp(NULL)
{

    //创建解码器
    decoder = new AudioDecoder();
    int res = decoder->AACdecodeInit();
    if (res < 0)
    {
        //dstDlg->MessageBox("解码器加载失败", "提示", MB_ICONWARNING | MB_OK);
        QMessageBox::warning(nullptr, "提示", "解码器加载失败");
        return;
    }
    streambuff_len = 0;
    outbuff_len = 0;
    stream_buff = (unsigned char*)malloc(AUDIO_INBUF_SIZE);
    out_buff = (unsigned char*)malloc(MAX_SIZE);

    init = true;
}

void AudioDecodeThread::task()
{

//    if(!MSM || MSM->getStat() != 3) return;
    //取数据包
    pkt_flag = FALSE;
    auto pkt = cfg->rec_audio_queue.pop_front();
    if(!pkt || pkt->datalen == 0) return;
    cfg->audio_last_timestamp = pkt->timestamp;

    //防止一直没有解码成功导致程序内存崩溃
    if(streambuff_len + pkt->datalen > AUDIO_INBUF_SIZE){
        memcpy(stream_buff, pkt->data, pkt->datalen);
        streambuff_len = pkt->datalen;
    }else{
        memcpy(stream_buff + streambuff_len, pkt->data, pkt->datalen);
        streambuff_len += pkt->datalen;
    }


    unsigned char* stream_buff_bk = stream_buff;
    int cur_out_len = 0;
    int res = decoder->AACdecodeFrame(&stream_buff, out_buff + outbuff_len, &streambuff_len, MAX_SIZE, cur_out_len);
    //重置frame_buf的位置
    memmove(stream_buff_bk, stream_buff, streambuff_len);
    //qDebug() << CUtils::generateLog("视频解码一帧完成", this);
    stream_buff = stream_buff_bk;

    constexpr int put_size = PLAY_SAMPLE_RATE * 4;
    int cur_data_l = outbuff_len + cur_out_len;
    while (cur_data_l > put_size) {

        char* data = (char*)malloc(put_size);
        memcpy(data, out_buff, put_size);
        cfg->pcm_que.push(data);
        cfg->pcm_pkt_l = put_size;
        memmove(out_buff, out_buff + put_size,  cur_data_l - put_size );
        cur_data_l -= put_size;
        outbuff_len = cur_data_l;
    }
    delete pkt;
}
AudioDecodeThread::~AudioDecodeThread() {
    if (decoder > 0)
    {
        decoder->AACdecodeClose();
        delete decoder;
    }
}
