#ifndef MYSESSIONMANAGER_H
#define MYSESSIONMANAGER_H
#include<QtCore>
#include <QUdpSocket>
#include "Threads/udphandler.h"
#include <unordered_set>
#include "MyUDPPacket.h"
#include "cammicloader.h"
#include <QMessageBox>
#include "mainwindow.h"
#include "threads/videoencodethread.h"
#include "threads/videodecodethread.h"
#include "threads/audioencodethread.h"
#include "threads/audiodecodethread.h"
//此类用于启动和管理会话
//包括呼叫的两次握手
class MySessionManager: public QObject
{
    Q_OBJECT
private:
    int sessionID;
    volatile int stat; //标记会话状态 0：未通话 1正在呼叫并等待回复 2正在发送挂断包并等待回复 3正在通话
    QUdpSocket *m_udpSocket;
    bool init;
    UdpHanlder* uh;
    VideoEncodeThread* video_enc_thread;
    VideoDecodeThread* video_dec_thread;
    AudioEncodeThread* audio_enc_thread;
    AudioDecodeThread* audio_dec_thread;

    std::unordered_set<int> sessionRecord;
    QTimer* m_pushTimer;
    long long last_time_rec;
    QMessageBox* cancelBox;
    QWidget* parent;

    void setSessionID(int sID){sessionID = sID;}
    void setStat(int s){stat = s;}
public:
    MySessionManager(QWidget* parent);

    int call(const QString sHostAddr,const int sHostPort);
    int sendControlPkt(const QString sHostAddr,const int sHostPort, MyUDPPacket* (*func)(int sID));
    int sendAccept(const QString sHostAddr,const int sHostPort);
    int sendReject(const QString sHostAddr,const int sHostPort);
    int sendHup(const QString sHostAddr,const int sHostPort);
    int sendHupResp(const QString sHostAddr,const int sHostPort);
    void recCall(MyUDPPacket* pkt);
    int close();
    void closeSock();

    void hupAccepted();
    void callReject();
    void callAccepted();
    void stopSendUDP();
    void stopSession(bool clear_sid = false);
    int hupSession();
    int updateUDP();
    int getSessionID(){return sessionID;}
    int getStat(){return stat;}
    bool getInit(){return init;}

    void setWindowTitle(const char* s);
private slots:
    void recUDP();
    void cancelCall(int v);
};

#endif // MYSESSIONMANAGER_H
