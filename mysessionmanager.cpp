#include "mysessionmanager.h"
#include "Config.h"
#include "QMessageBox"
#include <QtCore>
#include "MyUDPPacketBuilder.h"
#include "mainwindow.h"
extern Config* cfg;
MySessionManager::MySessionManager(QWidget* parent):
    stat(0),
    sessionID(0),
    m_udpSocket(nullptr),
    init(false),
    video_enc_thread(nullptr),
    m_pushTimer(new QTimer(this)),
    cancelBox(nullptr),
    video_dec_thread(nullptr),
    audio_dec_thread(nullptr),
    audio_enc_thread(nullptr),
    last_time_rec(0),
    parent(parent)
{
    uh = new UdpHanlder(m_udpSocket, cfg->target_ip, cfg->target_port);


    connect(m_pushTimer, &QTimer::timeout, this, [=]()mutable{
        if(getStat() != 3) return;
        auto cur_time = getCurTime();
        // 10s没数据就关闭
        if(last_time_rec != 0 && cur_time - last_time_rec > 10000){
            stopSession(true);
            QMessageBox::warning(nullptr, "提示", "超时，已断开会话");
        }
     }, Qt::DirectConnection);
    m_pushTimer->start(500);

//    CamMicLoader*& camloader = cfg->CML;
//    if(!camloader){
//        camloader = new CamMicLoader(cfg->w,cfg->h);
//        auto v_device = cfg->cur_cam;
//        auto a_device = cfg->cur_mic;
//        int tt = camloader->initLoader(v_device, a_device);

//        camloader->StartCapture();

//    }
//    audio_enc_thread = new AudioEncodeThread(this);
//    audio_enc_thread->start();
//    audio_dec_thread = new AudioDecodeThread(this);
//    audio_dec_thread->start();
    updateUDP();
    uh->start();
}
int MySessionManager::updateUDP(){
    qDebug() << "stat:" + QString::number(stat);
    if(stat != 0) return false;

    auto sock_new = new QUdpSocket(this);
    if(!sock_new->bind(cfg->local_port)){
        QMessageBox::warning(nullptr, "提示", "绑定新udp端口失败");
        return false;
    }

    closeSock();
    m_udpSocket = sock_new;
    uh->setSock(sock_new);

    //当服务端收到客户端发来的消息时会触发readyRead,信号
    //onReciveMsg是我自定义的槽函数，用来处理从客户端接受到的消息
    connect(m_udpSocket,&QUdpSocket::readyRead, this, &MySessionManager::recUDP);
    init = true;
    return true;
}
void MySessionManager::closeSock(){
    if(m_udpSocket){
        m_udpSocket->disconnect();
        if(m_udpSocket->isOpen()) m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = nullptr;
    }
}
void MySessionManager::recUDP(){
    //定义QByteArray类型变量
    QByteArray datagram;
    //设置data的大小为等待处理的数据报的大小，这样才能接收到完整的数据
    datagram.resize(m_udpSocket->pendingDatagramSize());
    // 接收数据报，将其存放到datagram中
    QHostAddress addr;
    quint16 port;
    m_udpSocket->readDatagram(datagram.data(), datagram.size(), &addr, &port);
    cfg->target_ip = addr.toString();
    cfg->target_port = port;
    //处理接收数据
    char* data = datagram.data();

    MyUDPPacket* pkt = MyUDPPacketBuilder::parse(data, datagram.size());

    if(!pkt || (stat != 0 && pkt->sessiongID != sessionID)) return;

//    qDebug() << "pkt->type: " + QString::number(pkt->type);
//    qDebug() << "stat: " + QString::number(stat);
    //会话状态转移
    switch (pkt->type) {
    case 0:
        if(stat == 0){
           recCall(pkt);
        }
        break;
    case 1:
        if(stat == 1){
           callAccepted();
           if(cancelBox) cancelBox->hide();
        }
        break;
    case 2:
        if(stat == 1){
           callReject();
           setStat(0);
           if(cancelBox)  cancelBox->hide();
        }
        break;
    case 3:
        if(stat == 3){
            hupAccepted();
        }
        setStat(0);
        break;
    case 4:
        if(stat == 2){
            stopSession(true);
        }
        setStat(0);
        break;
    case 9:
        if(stat == 3){
          //方到队列中
            cfg->rec_video_queue.push(pkt);
            last_time_rec = getCurTime();
        }
        break;
    case 10:
        if(stat == 3){
          //方到队列中
            cfg->rec_audio_queue.push(pkt);
            last_time_rec = getCurTime();
        }
        break;
    default:
        break;
    }
    if(pkt->type != 9 && pkt->type != 10){
        delete pkt;
    }
}


void MySessionManager::hupAccepted()
{

    stopSession();

    sendControlPkt(cfg->target_ip, cfg->target_port,  MyUDPPacketBuilder::buildHupRespPkt);

    setWindowTitle("正在发送挂断回应...");
    setStat(0);
    int res = sendHup(cfg->target_ip, cfg->target_port);

    setWindowTitle("未开始会话");
}

void MySessionManager::callAccepted()
{
    setStat(3);
    setWindowTitle("正在通话...");
    CamMicLoader*& camloader = cfg->CML;
    if(!camloader){
        camloader = new CamMicLoader(cfg->w,cfg->h);
        auto v_device = cfg->cur_cam;
        auto a_device = cfg->cur_mic;
        int tt = camloader->initLoader(v_device, a_device);

        camloader->StartCapture();

    }

    video_enc_thread = new VideoEncodeThread(this);
    video_enc_thread->start();
    video_dec_thread = new VideoDecodeThread(this);
    video_dec_thread->start();
    audio_enc_thread = new AudioEncodeThread(this);
    audio_enc_thread->start();
    audio_dec_thread = new AudioDecodeThread(this);
    audio_dec_thread->start();
}
void MySessionManager::callReject()
{

}
void MySessionManager::stopSendUDP()
{
    cfg->send_que.clear_use_delete();
}

int MySessionManager::call(const QString sHostAddr,const int sHostPort)
{
    if(stat != 0) return -1;
    setSessionID(rand() % 255);
    int res = sendControlPkt(sHostAddr, sHostPort,  MyUDPPacketBuilder::buildCallPkt);
    if(res >= 0){
        setStat(1);
    }

    if(cancelBox) delete cancelBox;
    cancelBox = new QMessageBox(QMessageBox::Information, "提示", "正在呼叫...",
                      QMessageBox::Cancel);
    cancelBox->setButtonText(QMessageBox::Cancel,QString("取 消"));
    connect(cancelBox, SIGNAL(finished(int)), this, SLOT(cancelCall(int)));
    cancelBox->show();

   return res;
}
void MySessionManager::cancelCall(int v){
    if(stat == 1){
       setStat(0);
    }
}
int MySessionManager::sendControlPkt(const QString sHostAddr,const int sHostPort, MyUDPPacket* (*func)(int sID))
{
    if(!init) return -1;


    uh->setIP(sHostAddr);
    uh->setPort(sHostPort);

    int temp_id = getSessionID();
    MyUDPPacket* pkt = func(temp_id);
    cfg->send_que.push(pkt);
    return 0;
}

//挂断回话
int MySessionManager::hupSession(){
    if(stat != 3) return -1;

    setStat(2);
//    stopSession(false);

    int res = sendHup(cfg->target_ip, cfg->target_port);

    setWindowTitle("正在挂断...");
    return res;
}

int MySessionManager::sendHup(const QString sHostAddr,const int sHostPort){
    return sendControlPkt(sHostAddr, sHostPort,  MyUDPPacketBuilder::buildHupPkt);
}
int MySessionManager::sendAccept(const QString sHostAddr,const int sHostPort)
{
    return sendControlPkt(sHostAddr, sHostPort,  MyUDPPacketBuilder::buildAcceptPkt);
}

int MySessionManager::sendReject(const QString sHostAddr,const int sHostPort)
{
    return sendControlPkt(sHostAddr, sHostPort,  MyUDPPacketBuilder::buildRejectPkt);
}

void MySessionManager::recCall(MyUDPPacket* pkt)
{
    if(!init || sessionRecord.find(pkt->sessiongID) != sessionRecord.end()) return;
    sessionRecord.insert(pkt->sessiongID);

    QMessageBox box(QMessageBox::Question, "提示", "是否接受连接",
                      QMessageBox::Yes|QMessageBox::No);
    box.setButtonText(QMessageBox::Yes,QString("接 受"));
    box.setButtonText(QMessageBox::No,QString("拒 绝"));
    // 弹出对话框
    int ret = box.exec();

    switch (ret) {
    case QMessageBox::Yes:
        qDebug() << "接受连接";
        setStat(3);
        setSessionID(pkt->sessiongID);
        sendAccept(cfg->target_ip, cfg->target_port);
        callAccepted();
        break;
    case QMessageBox::No:
    default:
        qDebug() << "拒绝连接";
        sendReject(cfg->target_ip, cfg->target_port);
        setStat(0);
        break;
    }
}

void MySessionManager::stopSession(bool clear_sid){
    if(clear_sid) setSessionID(0);
    if(getStat() != 0) setStat(0);
    CamMicLoader*& camloader = cfg->CML;
    if(camloader){
        camloader->CloseInputStream();
        delete camloader;
        camloader = nullptr;
    }

    if(video_dec_thread){
       video_dec_thread->stop();
       video_dec_thread = nullptr;
    }
    if(video_enc_thread){
       video_enc_thread->stop();
       video_enc_thread = nullptr;
    }
    if(audio_enc_thread){
       audio_enc_thread->stop();
       audio_enc_thread = nullptr;
    }
    if(audio_dec_thread){
       audio_dec_thread->stop();
       audio_dec_thread = nullptr;
    }
    cfg->rec_audio_queue.clear_use_delete();
    cfg->rec_video_queue.clear_use_delete();
    cfg->send_que.clear_use_delete();

    last_time_rec = 0;

    setWindowTitle("未开始会话");
}

void MySessionManager::setWindowTitle(const char* s){
    //跨线程，刷新界面送往UI主线程，此时必须捕获data数组，如果只捕获data分量指针，有可能由于导致跨线程读写问题引起crash
    QMetaObject::invokeMethod(this, [this, s]() mutable {
            ((MainWindow*)parent)->setTitle(s);
          ((MainWindow*)parent)->updateStartButtion();
        });
}
