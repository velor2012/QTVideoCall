#ifndef MYUDPPACKETBUILDER_H
#define MYUDPPACKETBUILDER_H
#include "MyUDPPacket.h"
#include<iostream>

class MyUDPPacketBuilder{
public:
     MyUDPPacketBuilder();
     ~MyUDPPacketBuilder();
    //此类主要用于构建与解析udp包
     static char* build(MyUDPPacket* pkt){
        //14字节头部
        char* t = (char*)malloc(pkt->datalen + PKT_HEADER_LEN);
        memcpy(t, &pkt->type, sizeof(uint8_t));
        memcpy(t + sizeof(uint8_t), &pkt->sessiongID, sizeof(uint8_t));
        memcpy(t + 2 * sizeof(uint8_t), &pkt->seq, sizeof(int));
        memcpy(t + 2 * sizeof(uint8_t) +  sizeof(int), &pkt->timestamp, sizeof(long long));
        memcpy(t + 14, pkt->data, pkt->datalen);
        return t;
     }
     static MyUDPPacket* parse(char* data, int len){
        if(len < 14) return nullptr;
        uint8_t a = 0;
        memcpy(&a, data, sizeof(uint8_t));
        MyUDPPacket* pkt = new MyUDPPacket();
        memcpy(&pkt->type, data, sizeof(uint8_t));
        memcpy(&pkt->sessiongID, data + sizeof(uint8_t), sizeof(uint8_t));
        memcpy(&pkt->seq, data + 2 * sizeof(uint8_t), sizeof(int));
        memcpy(&pkt->timestamp, data + 2 * sizeof(uint8_t) + sizeof(int), sizeof(long long));
        memcpy(&pkt->data, data + PKT_HEADER_LEN, len - PKT_HEADER_LEN);
        pkt->datalen = len - PKT_HEADER_LEN;
        return pkt;
     }
     static MyUDPPacket* buildCallPkt(int sID){
         MyUDPPacket* pkt = new MyUDPPacket();
         pkt->datalen = 100;
         pkt->seq = 0;
         pkt->sessiongID = sID;
         pkt->type = 0;
         return pkt;
     }

     static MyUDPPacket* buildAcceptPkt(int sID){
         MyUDPPacket* pkt = new MyUDPPacket();
         pkt->datalen = 100;
         pkt->seq = 0;
         pkt->sessiongID = sID;
         pkt->type = 1;
         return pkt;
     }
     static MyUDPPacket* buildRejectPkt(int sID){
         MyUDPPacket* pkt = new MyUDPPacket();
         pkt->datalen = 100;
         pkt->seq = 0;
         pkt->sessiongID = sID;
         pkt->type = 2;
         return pkt;
     }
     static MyUDPPacket* buildHupPkt(int sID){
         MyUDPPacket* pkt = new MyUDPPacket();
         pkt->datalen = 100;
         pkt->seq = 0;
         pkt->sessiongID = sID;
         pkt->type = 3;
         return pkt;
     }
     static MyUDPPacket* buildHupRespPkt(int sID){
         MyUDPPacket* pkt = new MyUDPPacket();
         pkt->datalen = 100;
         pkt->seq = 0;
         pkt->sessiongID = sID;
         pkt->type = 4;
         return pkt;
     }
};

#endif // MYUDPPACKETBUILDER_H
