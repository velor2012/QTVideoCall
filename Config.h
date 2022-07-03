#ifndef CONFIG_H
#define CONFIG_H
#include<iostream>
#include <vector>
#include<mutex>
#include <QString>
#include <QtCore>
#include <QtMultimedia/QCameraDevice>
#include <QtMultimedia/QAudioDevice>
#include <QtMultimedia/QMediaDevices>
#include "tool.h"
#include <queue>
#include <condition_variable>
#include "MyUDPPacket.h"
#include "MyQueue.h"
#include "cammicloader.h"
#define MAX_FRAME_SIZE 3110400
#define PLAY_SAMPLE_RATE 882
class Config {
public:
    static Config* GetInstance() {
        if (mcfg == nullptr) {//lock外的判空，
            //是因为获取锁，是很浪费时间的，
            //获取锁之外还有一层判断，
            //那么在第二次获取单例对象的时候，
            //lock外的if判断发现指针已经非空，就不会再获取锁了，
            //直接返回了对应的对象
            mutex_t.lock();
            if (mcfg == nullptr) {//lock里面判断一次，
            //因为可能有多个线程在lock处等待，
            //一个成功之后，会将m_psl设置为非空，
            //这样下个线程就算拿到lock资源，
            //再进去发现指针非空就离开了
                mcfg = new Config();
            }
            //这样双层检测，即保证了对象创建的唯一性，又减少了获取锁浪费的时间和资源
            mutex_t.unlock();
        }
        return mcfg;
    }

private:
    static Config* mcfg; //定义单例对象指针
    static std::mutex mutex_t; //定义锁

private:
//将其构造拷贝构造和赋值运算符重载全部私有化
    Config():
    h(288),
    w(352),
    fps(25),
    pcm_pkt_l(0),
    local_port(30000),
      bitrate(0),
      mute(false),
      CML(nullptr)
    {
       show_device(videolist, audiolist);
       cur_cam = videolist[0].toStdWString();
       cur_mic = audiolist[0].toStdWString();
//       cur_mic = L"";
//       cur_cam = L"";
    }
    Config(Config&) = delete;
    Config& operator=(Config&) = delete;


public:
    std::vector<QString> videolist;
    std::vector<QString> audiolist;
    std::wstring cur_mic;
    std::wstring cur_cam;
    int h;
    int w;
    int fps;
    int local_port;
    int bitrate;

    bool mute; //静音选项是否勾选

    int imgDecWidth;
    int imgDecHeight;

    long long audio_last_timestamp;
    long long video_last_timestamp;

    QString target_ip;
    int target_port;
    MyQueue<char*, 1> pcm_que; //存放需要播放的pcm音频数据
    MyQueue<char*, 1> enc_yuv_que; //存放需要播放的yuv视频数据
    MyQueue<char*, 1> dec_yuv_que; //存放需要播放的yuv视频数据
    int pcm_pkt_l;
    MyQueue<MyUDPPacket*> send_que; //存放需要发送的udp包数据
    MyQueue<MyUDPPacket*> rec_video_queue; //存放接收到的的视频包数据
    MyQueue<MyUDPPacket*> rec_audio_queue; //存放接收到的的音频包数据
    CamMicLoader* CML;
};
////类外对其静态成员变量进行一个初始化
//Config* Config::mcfg = nullptr;
//mutex Config::mutex_t;

#endif // CONFIG_H
