#ifndef UDPHANDLER_H
#define UDPHANDLER_H

//用于处理udp发送的事情
#include "MyBaseQThread.h"
#include <QUdpSocket>
#include <mutex>
class UdpHanlder : public MyBaseQThread
{
    Q_OBJECT
private:
    QUdpSocket* sock;
    int port;
    QString ip;
    std::mutex mtx;
public:
    UdpHanlder(QUdpSocket* q, QString ip, int port, QObject* parent = nullptr);
    void setIP(QString _ip){ip = _ip;}
    void setPort(int _port) {port = _port;}
    void setSock(QUdpSocket* q){sock = q;}
    ~UdpHanlder();
protected:
    void task();

};

#endif // UDPHANDLER_H
