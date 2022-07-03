#ifndef TOOL_H
#define TOOL_H
#include <iostream>
#include <vector>
#include <QString>
#include <QtCore>
#include <QtMultimedia/QCameraDevice>
#include <QtMultimedia/QAudioDevice>
#include <QtMultimedia/QMediaDevices>
#include <chrono>

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

using std::vector;
static void show_device(vector<QString>& videolist, vector<QString>& audiolist){
    //获取输入音频设备名称
    const QList<QAudioDevice> mics = QMediaDevices::audioInputs();
//    videolist.push_back("");
//    audiolist.push_back("");
    for (const QAudioDevice &micDevice : mics) {
        qDebug()<<micDevice.description();
        audiolist.push_back(micDevice.description());
    }


    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();

    for (const QCameraDevice &cameraDevice : cameras) {
        qDebug()<<cameraDevice.description();
        videolist.push_back(cameraDevice.description());
    }


}

static int YUV2RGB(uint8_t* data, int nSrcW, int nSrcH, uint8_t* out_buffer, int nDstW, int nDstH )
{
    nDstW = nDstW < 1 ? nSrcW : nDstW;
    nDstH = nDstH < 1 ? nSrcH : nDstH;
    // 分配空间
    auto Input_pFrame = av_frame_alloc();  //存放RGB数据的缓冲区
    auto Ouput_pFrame = av_frame_alloc();  //存放RGB数据的缓冲区

    struct SwsContext* m_pSwsContext = sws_getContext(nSrcW, nSrcH, AV_PIX_FMT_YUV420P, nDstW, nDstH, AV_PIX_FMT_RGB32, SWS_BILINEAR, NULL, NULL, NULL);
    if (NULL == m_pSwsContext)
    {
        //std::cout << "ffmpeg get context error!" << endl;
        return -1;
    }

    /*4. 设置转码的源数据地址*/
    avpicture_fill((AVPicture*)Ouput_pFrame, out_buffer, AV_PIX_FMT_RGB32, nDstW, nDstH);
    avpicture_fill((AVPicture*)Input_pFrame, data, AV_PIX_FMT_YUV420P, nSrcW, nSrcH);

    int res = sws_scale(m_pSwsContext, (const uint8_t* const*)Input_pFrame->data, Input_pFrame->linesize, 0, nSrcH, Ouput_pFrame->data, Ouput_pFrame->linesize);

    if (Input_pFrame)av_free(Input_pFrame);
    if (Ouput_pFrame)av_free(Ouput_pFrame);
    if (m_pSwsContext)sws_freeContext(m_pSwsContext);
    return 0;
}

const static int8_t permutation[33] =
    {0,1,2,3,4,4,5,5,5,5,6,6,6,6,6,7,7,7,7,8,8,8,9,9,9,9,9,9,9,9,9,9,9};

static int16_t WebRtcSpl_MaxAbsValueW16C(const int16_t* vector, size_t length)
{
    size_t i = 0;
    int absolute = 0, maximum = 0;
    for (i = 0; i < length; i++) {
        absolute = abs((int)vector[i]);
        if (absolute > maximum) {
            maximum = absolute;
        }
    }
    if (maximum > 32767) {
        maximum = 32767;
    }
    return (int16_t)maximum;
}

static int8_t ComputeLevel(const int16_t* data, size_t length)
{
    int16_t _absMax = 0;
    int16_t _count = 0;
    int8_t _currentLevel = 0;
    int16_t absValue(0);
    absValue = WebRtcSpl_MaxAbsValueW16C(data,length);
    if (absValue > _absMax)
        _absMax = absValue;
    if (_count++ == 10) {
        _count = 0;
        int32_t position = _absMax/1000;
        if ((position == 0) && (_absMax > 250)){
            position = 1;
        }
        _currentLevel = permutation[position];
        _absMax >>= 2;
    }
    return _currentLevel;
}

//参数为数据，采样个数
//返回值为分贝
static int SimpleCalculate_DB(short* pcmData, int sample)
{
    signed short ret = 0;
    if (sample > 0){
        int sum = 0;
        signed short* pos = (signed short *)pcmData;
        for (int i = 0; i < sample; i++){
            sum += abs(*pos);
            pos++;
        }
        ret = sum * 500.0 / (sample * 32767);
        if (ret >= 100){
            ret = 100;
        }
   }
   return ret;
}

static long long getCurTime(){
    std::chrono::milliseconds ms = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    );

    return ms.count();
}

#endif // TOOL_H
