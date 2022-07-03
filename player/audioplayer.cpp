#include "audioplayer.h"

#include <QDebug>
#include <QFile>
#include "Config.h"
extern "C" {
#include <include/SDL.h>
#include <include/SDL_thread.h>
//#include <Utils.h>
}
#include "tool.h"
//采样率
#define SAMPLE_RATE 44100
// 采样格式
#define SAMPLE_FORMAT AUDIO_S16LSB
// 采样大小，等同于SAMPLE_FORMAT & 0xFF
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT)
// 声道数
#define CHANNELS 2
// 音频缓冲区的样本数量，必须是2的幂次方
#define SAMPLES 882
// 每个样本占用多少个字节 ，向右移三位表示除以8，效率更高
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) >> 3)
// 定义缓冲区大小，SAMPLES*CHANNELS*SAMPLE_FORMAT/8
#define BUFFER_SIZE (SAMPLES * BYTES_PER_SAMPLE)
uint8_t volume = SDL_MIX_MAXVOLUME;
 struct AudioBuffer{
    int len = 0;
    int pullLen = 0;
    Uint8 *data = nullptr;
};
extern Config* cfg;
// userdata：SDL_AudioSpec.userdata
// stream：音频缓冲区（需要将音频数据填充到这个缓冲区）
// len：音频缓冲区的大小（SDL_AudioSpec.samples * 每个样本的大小
void pull_audio_data(void *userdata, Uint8 *stream, int len)
{
    //清空缓存stream
    SDL_memset(stream,0,len);

    // 取出缓冲信息
    AudioBuffer *buffer=(AudioBuffer *)userdata;
    if(!buffer->data || buffer->len==0){
        return;
    }

    // 取len、bufferLen的最小值（为了保证数据安全，防止指针越界）
    buffer->pullLen = (len > buffer->len) ? buffer->len : len;

    // 填充数据
    SDL_MixAudio(stream,buffer->data,buffer->pullLen,volume);

    buffer->data+=buffer->pullLen;
    buffer->len-=buffer->pullLen;
}

AudioPlayeThread::AudioPlayeThread(QObject *parent) : QThread(parent)
{
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this,&AudioPlayeThread::finished,this,[=](){
        this->deleteLater();
        qDebug()<<"线程结束";
    });
}

AudioPlayeThread::~AudioPlayeThread()
{
    //强制关闭窗口时，线程也能安全关闭
    requestInterruption();
    wait();
    qDebug()<<"析构函数";
}

void AudioPlayeThread::run()
{
    if(SDL_Init(SDL_INIT_AUDIO)){
        qDebug()<<"初始化子系统失败";
        SDL_Quit();
        return;
    }

    //配置音频设备的参数
    SDL_AudioSpec audio_spec;
    audio_spec.freq=SAMPLE_RATE;
    audio_spec.format=AUDIO_S16LSB;
    audio_spec.channels=CHANNELS;
    audio_spec.samples=SAMPLES;
    audio_spec.callback= pull_audio_data;
    // 传递给回调的参数
    AudioBuffer buffer;
    audio_spec.userdata = &buffer;

    if(SDL_OpenAudio(&audio_spec,nullptr)){
        qDebug()<<"打开设备失败";
        SDL_Quit();
        return;
    }

    // 存放文件数据
    Uint8 data[BUFFER_SIZE];
    //0表示播放，其他数字表示暂停，只要开启就会自动调用回调函数
    SDL_PauseAudio(0);

    while(!isInterruptionRequested()){//当没发出中断请求时，执行循环体
        // 只要从文件中读取的音频数据，还没有填充完毕，就跳过
        if(buffer.len>0){
            continue;
        }

        //取数据

        char* pcm_data = cfg->pcm_que.pop_front();
        if(pcm_data == nullptr) continue;

        memcpy(data, pcm_data, cfg->pcm_pkt_l);
        free(pcm_data);
        // 读取到了文件数据
        buffer.data = (uint8_t*)data;
        buffer.len = cfg->pcm_pkt_l;

//        qDebug() << "paly: " + QString::number(getCurTime());
        // 文件数据已经读取完毕，防止数据还没读完，线程就结束了
        if (buffer.len <= 0) {
            // 剩余的样本数量
            int samples = buffer.pullLen / BYTES_PER_SAMPLE;
            int ms = samples * 1000 / SAMPLE_RATE;
            SDL_Delay(ms);
          qDebug() << "stop: " + QString::number(getCurTime());
            break;
        }

    }

    // 关闭音频设备
    SDL_CloseAudio();
    //清除所有子系统
    SDL_Quit();
}
