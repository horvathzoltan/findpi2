#include "sendicmp.h"
#include "qcoreapplication.h"
#include <unistd.h>

#include <errno.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/time.h>
#include <arpa/inet.h>
#include <QDebug>
#include <QElapsedTimer>
#include <QProcessEnvironment>

// https://stackoverflow.com/questions/9913661/what-is-the-proper-process-for-icmp-echo-request-reply-on-unreachable-destinatio
// https://github.com/rocwangp/ping/blob/master/ping.cpp

/*--------------------------------------------------------------------*/
/*--- checksum - standard 1s complement checksum                   ---*/
/*--------------------------------------------------------------------*/
quint16 Ping::checksum(char *b, quint32 len)
{
    quint16 *buf = (quint16 *)b;
    quint32 sum=0;
    quint16 result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(quint8*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void Ping::setVerbose(bool v){
    _verbose = v;
}

Ping::PingResult Ping::ping2(const QHostAddress& host, quint32 timeoutMillis, quint32 loopMax)
{
    static QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    QProcess process;
    QString hostName = host.toString();

    Ping::PingResult r;
    r.ok = false;
    r.fromIp = hostName;
    r.packSize = 0;
    r.icmpSeq = 0;
    r.ttl = 0;

    // process beállítása
    // workaround - https://bugreports.qt.io/browse/QTBUG-2284
    env.insert("LD_LIBRARY_PATH", "/usr/lib");
    process.setProcessEnvironment(env);
    QString path = qApp->applicationDirPath();
    process.setWorkingDirectory(path);


    QStringList l = {hostName,"-c1", "-w3"};
    QString cmd("ping");
    // process indítása
    QElapsedTimer t;
    t.start();
    process.start(cmd, l);
    if(!process.waitForStarted()) return r;
    process.waitForFinished();//timeoutMillis);

    r.time = t.elapsed();
    //ProcessHelper::Output o;
    //auto elapsedMillis = t.elapsed();
    //auto stdOut  = process.readAllStandardOutput();
    //auto stdErr = process.readAllStandardError();
    auto exitCode = process.exitCode();

    r.ok = exitCode==0;

    return r;
}

/*--------------------------------------------------------------------*/
/*--- ping - Create message and send it.                           ---*/
/*    return 0 is ping Ok, return 1 is ping not OK.                ---*/
/*--------------------------------------------------------------------*/
Ping::PingResult Ping::ping(const QHostAddress& host, quint32 timeoutMillis, quint32 loopMax)
{
    _nPacketNoLimit=0;
    _cnt=0;
    PingResult result;
    const qint32 val=255;
    qint32 sd;
    struct icmp* pckt;
    char packet[PACKETSIZE];
    char rpacket[PACKETSIZE];
    struct sockaddr_in r_addr;
    struct hostent *hname;
    //struct sockaddr_in addr_ping;

    _pid = htons(getpid() & 0xFFFF);
    _proto = getprotobyname("ICMP");

    QString hostName = host.toString();
    hname = gethostbyname(hostName.toLocal8Bit());

    memset(&_addrPing, 1, sizeof(_addrPing));
    _addrPing.sin_family = hname->h_addrtype;
    _addrPing.sin_port = 0;
    _addrPing.sin_addr.s_addr = *(in_addr_t*)hname->h_addr;

//    memset(&addr_ping, 1, sizeof(addr_ping));
//    addr_ping.sin_family = host.;
//    addr_ping.sin_port = 0;
//    addr_ping.sin_addr.s_addr = htonl(host.toIPv4Address());

    sd = socket(PF_INET, SOCK_DGRAM, _proto->p_proto);//SOCK_DGRAM
    if ( sd < 0 )
    {
        if(_verbose) qDebug()<<"socket errno:"+ QString::number(errno);
        result.packSize = 0;
        return result;
    }
    if ( setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
    {
        if(_verbose) qDebug()<<"Set TTL option";
        return result;
    }
    if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
    {
        if(_verbose) qDebug()<<"Request nonblocking I/O";
        return result;
    }

    quint64 r_addr_len = sizeof(r_addr);

    _data = "majom";

    _dataOffset = 8+sizeof(timeval);
    _dataLength = sizeof(timeval)+_data.length();
    _packetLength = sizeof(icmp)+_dataLength;
    memset(&pckt, 0, sizeof(pckt));
    pckt = (icmp*)packet;

    pckt->icmp_type = ICMP_ECHO;
    pckt->icmp_code = 0;
    pckt->icmp_id = _pid;

    struct timeval *tvsend = (struct timeval*)pckt->icmp_data;

    auto d8 = _data.toLocal8Bit();
    strncpy(packet+_dataOffset,d8,_data.length());

    //select  timeout
     _timeoutMillis = timeoutMillis;
    // struct timeval timeout;
    // timeout.tv_sec = 0;
    // timeout.tv_usec = _timeoutMillis*1000;

     auto tms = (_timeoutMillis*1000)/loopMax;

    fd_set recvfd;
    qint32 maxfd = sd + 1;

    //bool pingOk=false;
    for (quint32 loop=0; loop < loopMax; loop++)
    {
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = tms*(loop+1);

        pckt->icmp_cksum = 0;
        //qDebug()<<"cnt"<<_cnt;
        pckt->icmp_seq = _cnt++;        
        pckt->icmp_cksum = checksum(packet, _packetLength);
        gettimeofday(tvsend, NULL);

        ssize_t s = sendto(sd, pckt, _packetLength, 0,
                           (struct sockaddr *)&_addrPing, sizeof(_addrPing));

        if(s <= 0)
        {
            if(_verbose) qDebug()<<"sendto";
        }

        FD_ZERO(&recvfd);
        FD_SET(sd, &recvfd);

        qint32 n = select(maxfd, &recvfd, NULL, NULL, &timeout);

        if(n < 0)
        {
            if(_verbose) qDebug()<<"Receive_Packet: select error...\n";
            continue;
        }
        //if timeout, then needn't receive packet again, so set m_nCnt to m_nSend and break
        else if(n == 0)
        {
            if(_verbose) qDebug() << "Request Timeout...";
            continue;
        }

        ssize_t packSize = recvfrom(sd, &rpacket, PACKETSIZE, 0,
                                    (struct sockaddr *)&r_addr, (socklen_t *)&r_addr_len);

        if(packSize < 0)
        {
            if(_verbose) qDebug()<< "recvfrom error...\n";
            continue;
        }

        result = unpack(rpacket, packSize);
        if (result.ok)
        {
            break;
        }        
    }

    return result;
}

Ping::PingResult Ping::unpack(char* packet, quint32 packSize)
{
    PingResult r;
    r.packSize = packSize;
    char* brr = inet_ntoa(_addrPing.sin_addr);
    r.fromIp = QString::fromLocal8Bit(brr);

    icmp* m_recvpack = (icmp*)packet;
    //the packet received is IP_packet
    struct ip* recv_ip = (struct ip*)m_recvpack;

    r.ttl = recv_ip->ip_ttl;
    //IP_header : ip_hl is 4 bit , need to multi 4  //why?    
    qint32 iphdrlen = recv_ip->ip_hl << 2;
    //last len is ICMP_packet_len
    packSize -= iphdrlen;

    //remove the IP_Header then get ICMP_PACKET
    struct icmp *r_pckt = (struct icmp*)(m_recvpack + iphdrlen);

    r.icmpSeq = r_pckt->icmp_seq;
    //ICMP_header len is 8, if the packet len less than 8, then the packet is error
    if(packSize < 8) return r;

    if(r_pckt->icmp_type == ICMP_DEST_UNREACH)
    {
        qDebug()<<"ICMP_DEST_UNREACH";
        return r;
    }

    //the packet is not REPLY_packet
    if(r_pckt->icmp_type != ICMP_ECHOREPLY) return r;   

    // icmphdr * pkt = (icmphdr *)(recv_icmp);
    //the packet_id is not the id which is set when send
  //  uint16_t id = pid;//ntohs(pkt->un.echo.id);
  //  uint16_t id2 = htons(r_pckt->icmp_id & 0xFFFF);

//    if(id2 != id)
//        return -1;

    //ICMP_data is the send_time which is set when send
    struct timeval* sendtime = (struct timeval *)r_pckt->icmp_data;
    struct timeval recvtime;
    gettimeofday(&recvtime, NULL);
    tv_sub(&recvtime, sendtime);

    qreal send_to_recv_time = recvtime.tv_sec * 1000 + (qreal)recvtime.tv_usec / 1000;

    r.time = send_to_recv_time;

    //the packet maybe the previous timeout packet, need to throw
    if(send_to_recv_time > _timeoutMillis * 1000) return r;

    QString m = QString::fromLocal8Bit(&packet[_dataOffset], _data.length());
    if(m!=_data) return r;

    if(_verbose)
    {
        qDebug()<< packSize<<"bytes from"<<brr<<"icmp_seq="<<r_pckt->icmp_seq<<
               "ttl="<<recv_ip->ip_ttl<<"time="<<send_to_recv_time<<"ms"<<m;
    }

    //m_bPacketNo used to judge this ICMP_PACKET whether is repetive or not
    //if icmp_seq more than the size of m_bPacketNo, icmp_seq reduce m_nPacketNoLimit
//    int n = r_pckt->icmp_seq - _nPacketNoLimit;
//    if(n >= MAX_PACKET_NO)
//    {
//        Init_PacketNoArray();
//        _nPacketNoLimit += MAX_PACKET_NO;
//    }
////    //judge whether there exist loop on the way to destination
//    if(_bPacketNo[n] == true)
//    {
//        if(_verbose) qDebug() << " DUP";
//        //return r;
//    }
//    else
//    {
//        if(_verbose) qDebug() << "hutty";
//        _bPacketNo[n] = true;
//        //return 1;
//        r.ok = true;
//    }

    r.ok = true;
    return r;
}

void Ping::tv_sub(struct timeval *in, struct timeval *out)
{
    qint64 sec = in->tv_sec - out->tv_sec;
    qint64 usec = in->tv_usec - out->tv_usec;
    if(usec < 0)
    {
        in->tv_sec = sec - 1;
        in->tv_usec = 1000000 + usec;
    }
    else
    {
        in->tv_sec = sec;
        in->tv_usec = usec;
    }
}

QString Ping::PingResult::ToString()
{
    QString a = QStringLiteral("%1 icmp_seq=%2 ttl=%3 time=%4ms")//%1 bytes from
            .arg(fromIp).arg(icmpSeq).arg(ttl).arg(time);//.arg(packSize)
    return a;
}

//void Ping::Init_PacketNoArray()
//{
//    for(int i = 0; i < MAX_PACKET_NO; ++i)
//        _bPacketNo[i] = false;
//}
