#include "udphandler.h"
#include <QWidget>
#include "Config.h"
#include "MyUDPPacketBuilder.h"
#include "mysessionmanager.h"

extern Config* cfg;
extern MySessionManager* MSM;

UdpHanlder::UdpHanlder(QUdpSocket* q, QString ip, int port, QObject* parent):
    MyBaseQThread(parent),
    sock(q)
{
    setIP(ip);
    setPort(port);
}

void UdpHanlder::task()
{
    if(!MSM || !MSM->getInit()) return;
    //取数据
    auto pkt = cfg->send_que.pop_front();
    if(!pkt) return;

    char* data = MyUDPPacketBuilder::build(pkt);
    QByteArray bytGram(data, pkt->datalen + PKT_HEADER_LEN);

    //发送
    sock->writeDatagram(bytGram, bytGram.size(), QHostAddress(ip), port);

    delete pkt;
    free(data);
}

UdpHanlder::~UdpHanlder(){};
