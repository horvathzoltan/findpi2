#ifndef SENDICMP_H
#define SENDICMP_H

#include <QtGlobal>
#include <fcntl.h>
#include <netinet/ip_icmp.h>
#include <QHostAddress>
#include <QString>
#include <string>

#define PACKETSIZE  64
#define MAX_PACKET_NO 50

class Ping
{
    uint16_t _pid=0;
    struct protoent *_proto=nullptr;
    quint32 _cnt=1;
    quint64 _packetLength;
    quint64 _dataLength;
    struct sockaddr_in _addrPing;
    QString _data;
    quint64 _dataOffset;
    quint32 _timeoutMillis;
    bool _verbose;
    int _nPacketNoLimit;
    bool _bPacketNo[MAX_PACKET_NO];

    void Init_PacketNoArray();
public:
    struct PingResult{
        quint32 packSize;
        QString fromIp;
        quint16 icmpSeq;
        quint8 ttl;
        qreal time;
        bool ok = false;

        QString ToString();
    };
    quint16 checksum(char*, quint32 len);
    PingResult ping(const QHostAddress& host, quint32 timeoutMillis=1000, quint32 loopMax=1);
    PingResult unpack(char* pckt, quint32);
    void tv_sub(timeval *in, timeval *out);
    void setVerbose(bool);
};

#endif // SENDICMP_H
