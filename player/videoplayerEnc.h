#ifndef VIDEOPLAYERENC_H
#define VIDEOPLAYERENC_H

#include "MyBaseQThread.h"
#include <mutex>
class VideoPlayerEnc  : public MyBaseQThread
{
    Q_OBJECT
public:
    VideoPlayerEnc(QObject* parent = nullptr);
    void task();
    void run();
};

#endif // VIDEOPLAYERENC_H
