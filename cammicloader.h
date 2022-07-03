#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "libavutil/time.h"
#include "libavutil/pixfmt.h"
#include "libavutil/common.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "libavutil/pixfmt.h"
#include "libavutil/imgutils.h"
#ifdef __cplusplus
}
#endif
//#include <minwindef.h>
//#include <stringapiset.h>
#include <atltrace.h>
#include <string>
#include "comutil.h"
#include <mutex>


typedef LRESULT(CALLBACK* VideoCaptureCB)(uint8_t* out_buff, INT64 lTimeStamp);
typedef LRESULT(CALLBACK* AudioCaptureCB)(uint8_t* out_buff, int audio_out_buffer_size, INT64 lTimeStamp);
class CamMicLoader
{
private:
    AVFormatContext* m_pVidFmtCtx;
    AVFormatContext* m_pAudFmtCtx;
    AVInputFormat* m_pInputFormat;
    AVCodecContext* pVideoCodecCtx;
    AVCodecContext* pAudioCodecCtx;
    AVFrame* pFrameYUV;
    SwsContext* img_convert_ctx;
    SwrContext* audio_convert_ctx;
    AVPacket* dec_pkt;
    uint8_t* m_out_buffer;
    uint8_t* m_audio_out_buffer;
    std::wstring  m_video_device; //摄像头名称;
    std::wstring  m_audio_device; //麦克风名称;

    int     m_videoindex;
    int     m_audioindex;

    HANDLE m_hCapVideoThread, m_hCapAudioThread; //线程句柄;
    bool   m_exit_thread; //退出线程的标志变量;

    VideoCaptureCB  m_pVideoCBFunc; //视频数据回调函数指针;
    AudioCaptureCB  m_pAudioCBFunc; //音频数据回调函数指针;

    std::mutex lock;

    int decodeVideo(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt);//底层解码函数;
    int decodeAudio(AVCodecContext* dec_ctx, AVFrame* frame, AVPacket* pkt);

protected:
    static DWORD WINAPI CaptureVideoThreadFunc(LPVOID lParam); //获取视频数据的线程;
    static DWORD WINAPI CaptureAudioThreadFunc(LPVOID lParam);

    int  ReadVideoPackets();
    int  ReadAudioPackets();

public:
    int64_t     m_start_time; //采集的起点时间;
    int out_width, out_height; //输出的yuv长宽;
    int src_width, src_height, fps; //摄像头采集的长宽fps;
    AVPixelFormat pixel_fmt;
    uint8_t* out_buff; //存储摄像头采集的，经过重采样的yuv;

    AVSampleFormat sample_fmt;//麦克风音频的采样格式，采样率，通道数;
    int sample_rate;
    int channel;
    int audio_nsamples;
    int audio_out_buffer_size;
    uint8_t* audio_out_buffer;
    int count;

    CamMicLoader(int out_width, int out_height);
    ~CamMicLoader();
public:
    bool initLoader(std::wstring video_device_name, std::wstring audio_device_name);
    void  CloseInputStream();
    bool StartCapture();//开始获取数据，内部开了两个线程分别接受音频视频数据;
    bool  GetVideoInputInfo(int& width, int& height, int& framerate, AVPixelFormat& pixFmt);
    bool  GetAudioInputInfo(AVSampleFormat& sample_fmt, int& sample_rate, int& channels);

    void  SetVideoCaptureCB(VideoCaptureCB pFuncCB);//获取一帧数据之后的回调函数，返回的东西看VideoCaptureCB的参数;
    void  SetAudioCaptureCB(AudioCaptureCB pFuncCB);
};
