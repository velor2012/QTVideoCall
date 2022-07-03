#include "videoplayerEnc.h"
#include "Config.h"
#include "mainwindow.h"
#include "tool.h"
extern Config* cfg;
extern MySessionManager* MSM;
VideoPlayerEnc::VideoPlayerEnc(QObject* parent):MyBaseQThread(parent)
{

}
void VideoPlayerEnc::task(){
    if(!MSM || MSM->getStat() != 3) return;
    char* yuv = cfg->enc_yuv_que.pop_front();
    if(yuv){
        MainWindow* mainwindow = (MainWindow*)this->parent;
        YUV2RGB((uint8_t*)yuv, cfg->w, cfg->h, mainwindow->rgb_dis_buf, mainwindow->cur_enc_lb_w, mainwindow->cur_enc_lb_h);
         //跨线程，刷新界面送往UI主线程，此时必须捕获data数组，如果只捕获data分量指针，有可能由于导致跨线程读写问题引起crash
        int r = QMetaObject::invokeMethod(mainwindow, "show_encode_buf");
        free(yuv);
    }

}
void VideoPlayerEnc::run(){
    while (!finish && !isInterruptionRequested()) {
        task();
    }
}
