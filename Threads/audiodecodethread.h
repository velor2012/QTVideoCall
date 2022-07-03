#ifndef AUDIODECODETHREAD_H
#define AUDIODECODETHREAD_H

#include <deque>
#include "MyBaseQThread.h"
#include <QFile>
#include <QString>
#include "EnDeCoder/audiodecoder.h"
//ffmpeg默认以1024样本进行解码，但播放需要882个样本，需要解码时与解码后处理
class AudioDecodeThread : public MyBaseQThread
{
    Q_OBJECT

public:
    AudioDecodeThread(QObject* parent = nullptr);
    ~AudioDecodeThread();
    void closeFiles();

private:

    QFile* rec_fp; //保存重建的音频PCM
    QString save_pcm_path;

    int frame_size;
    int payloadlen;
    int pts;
    unsigned short sequence;
    unsigned char* stream_buff;
    unsigned char* out_buff;
    int streambuff_len;
    int outbuff_len;
    bool initSDL;

    bool pkt_flag;
    bool init_decoder;
    long decoder_handle;

    AudioDecoder* decoder;

protected:
    void task();
};


#endif // AUDIODECODETHREAD_H
