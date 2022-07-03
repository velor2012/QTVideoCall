#include "VideoPlayerDec.h"
#include "Config.h"
#include "mainwindow.h"
#include "tool.h"
extern Config* cfg;
extern MySessionManager* MSM;
VideoPlayerDec::VideoPlayerDec(QObject* parent):MyBaseQThread(parent)
{

}
void VideoPlayerDec::task(){
    if(!MSM || MSM->getStat() != 3) return;
    auto yuv = cfg->dec_yuv_que.pop_front();
    if(yuv){
        MainWindow* mainwindow = (MainWindow*)this->parent;
        YUV2RGB((uint8_t*)yuv, cfg->imgDecWidth, cfg->imgDecHeight, mainwindow->rgb_dec_buf, mainwindow->cur_dec_lb_w, mainwindow->cur_dec_lb_h);
         //跨线程，刷新界面送往UI主线程，此时必须捕获data数组，如果只捕获data分量指针，有可能由于导致跨线程读写问题引起crash
        int r = QMetaObject::invokeMethod(mainwindow, "show_decode_buf");
        free(yuv);
    }
}
void VideoPlayerDec::run(){
    while (!finish && !isInterruptionRequested()) {
        task();
    }
}
