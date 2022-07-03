#ifndef VideoPlayerDec_H
#define VideoPlayerDec_H

#include "MyBaseQThread.h"
#include <mutex>
class VideoPlayerDec  : public MyBaseQThread
{
    Q_OBJECT
public:
    VideoPlayerDec(QObject* parent = nullptr);
    void task();
    void run();
};

#endif // VideoPlayerDec_H
