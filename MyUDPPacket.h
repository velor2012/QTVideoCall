#ifndef MYUDPPACKET_H
#define MYUDPPACKET_H
#define MAX_PKT_SIZE 3000
#define PKT_HEADER_LEN 14
#include<iostream>
struct MyUDPPacket
{
    uint8_t type;  //0呼叫 1呼叫返回接收 2呼叫返回拒绝 3挂断 4挂断回复报文 与tcp三次握手4次挥手类似，因为有会话id，不需要三次握手，因为不需要确保每个包都收到，挥手只需要两次
    //9 视频包， 10音频包
    uint8_t sessiongID;
    int seq;
    long long timestamp;
    char data[MAX_PKT_SIZE];
    int datalen;
};
#endif // MYUDPPACKET_H
