#include "sendicmp.h"

#include <errno.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/time.h>
#include <arpa/inet.h>
#include <QDebug>

// https://stackoverflow.com/questions/9913661/what-is-the-proper-process-for-icmp-echo-request-reply-on-unreachable-destinatio


/*--------------------------------------------------------------------*/
/*--- checksum - standard 1s complement checksum                   ---*/
/*--------------------------------------------------------------------*/
unsigned short Ping::checksum(char *b, int len)
{
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}


/*--------------------------------------------------------------------*/
/*--- ping - Create message and send it.                           ---*/
/*    return 0 is ping Ok, return 1 is ping not OK.                ---*/
/*--------------------------------------------------------------------*/
bool Ping::ping(char *adress)
{
    const int val=255;
    int sd;
    struct icmp* pckt;
    char packet[PACKETSIZE];
    char rpacket[PACKETSIZE];
    struct sockaddr_in r_addr;
    int loop;
    struct hostent *hname;
    //struct sockaddr_in addr_ping;

    pid = htons(getpid() & 0xFFFF);
    proto = getprotobyname("ICMP");
    hname = gethostbyname(adress);
    memset(&addr_ping, 1, sizeof(addr_ping));
    addr_ping.sin_family = hname->h_addrtype;
    addr_ping.sin_port = 0;
    addr_ping.sin_addr.s_addr = *(long*)hname->h_addr;

    //addr = &addr_ping;

    sd = socket(PF_INET, SOCK_DGRAM, proto->p_proto);
    if ( sd < 0 )
    {
        perror("socket");
        return false;
    }
    if ( setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
    {
        perror("Set TTL option");
        return false;
    }
    if ( fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
    {
        perror("Request nonblocking I/O");
        return false;
    }

    int r_addr_len = sizeof(r_addr);
    data = "majom";

    data_length = sizeof(timeval)+data.length();
    packet_length = sizeof(icmp)+data_length;
    memset(&pckt, 0, sizeof(pckt));
    pckt = (icmp*)packet;

    pckt->icmp_type = ICMP_ECHO;
    pckt->icmp_code = 0;
    pckt->icmp_id = pid;

    struct timeval *tvsend = (struct timeval*)pckt->icmp_data;

    strncpy(&packet[8+sizeof(timeval)],data.c_str(),data.length());

    //select  timeout
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 300*1000;

    fd_set recvfd;
    int maxfd = sd + 1;

    bool pingOk=false;
    for (loop=0;loop < 10; loop++)
    {
        pckt->icmp_cksum = 0;
        pckt->icmp_seq = cnt++;
        pckt->icmp_cksum = checksum(packet, packet_length);
        gettimeofday(tvsend, NULL);

        int s = sendto(sd, pckt, packet_length, 0, (struct sockaddr*)&addr_ping, sizeof(addr_ping));

        if(s <= 0)
            qDebug()<<"sendto";

        FD_ZERO(&recvfd);
        FD_SET(sd, &recvfd);

        int n = select(maxfd, &recvfd, NULL, NULL, &timeout);

        if(n < 0)
        {
            qDebug()<<"Receive_Packet: select error...\n";
        }
        //if timeout, then needn't receive packet again, so set m_nCnt to m_nSend and break
        else if(n == 0)
        {
            qDebug() << "Request Timeout...";
        }

        int packSize = recvfrom(sd, &rpacket, PACKETSIZE,
                                    0, (struct sockaddr*)&r_addr,(socklen_t*) &r_addr_len);

        if(packSize < 0)
        {
            qDebug()<< "recvfrom error...\n";
        }
        else
        {
            int u_size = unpack(rpacket, packSize);
            if (u_size > 0)
            {
                pingOk=true;
                //continue;
            }
        }
        usleep(timeout.tv_usec);
    }

    return pingOk;
}

int Ping::unpack(char* packet, int packSize)
{

    icmp* m_recvpack = (icmp*)packet;
    //the packet received is IP_packet
    struct ip* recv_ip = (struct ip*)m_recvpack;
    //IP_header : ip_hl is 4 bit , need to multi 4  //why?
    int iphdrlen = recv_ip->ip_hl << 2;
    //last len is ICMP_packet_len
    packSize -= iphdrlen;

    //remove the IP_Header then get ICMP_PACKET
    struct icmp *r_pckt = (struct icmp*)(m_recvpack + iphdrlen);

    //ICMP_header len is 8, if the packet len less than 8, then the packet is error
    if(packSize < 8)
        return -1;

    //the packet is not REPLY_packet
    if(r_pckt->icmp_type != ICMP_ECHOREPLY)
        return -1;


    // icmphdr * pkt = (icmphdr *)(recv_icmp);
    //the packet_id is not the id which is set when send
  //  uint16_t id = pid;//ntohs(pkt->un.echo.id);
  //  uint16_t id2 = htons(r_pckt->icmp_id & 0xFFFF);

//    if(id2 != id)
//        return -1;


    struct timeval *sendtime, recvtime;
    //ICMP_data is the send_time which is set when send
    sendtime = (struct timeval*)r_pckt->icmp_data;
    gettimeofday(&recvtime, NULL);
    tv_sub(&recvtime, sendtime);

    double send_to_recv_time = recvtime.tv_sec * 1000 + (double)recvtime.tv_usec / 1000;

    //the packet maybe the previous timeout packet, need to throw
   // if(send_to_recv_time > m_nMaxTimeWait * 1000)
   //     return -1;

    std::string m(&packet[8+sizeof(timeval)]);
    if(m!=data)
        return -1;

    QString br = QString::fromStdString(m);

    auto brr = inet_ntoa(addr_ping.sin_addr);
    qDebug()<< packSize<<"bytes from"<<brr<<"icmp_seq="<<r_pckt->icmp_seq<<
               "ttl="<<recv_ip->ip_ttl<<"time="<<send_to_recv_time<<"ms"<<br;

    //m_bPacketNo used to judge this ICMP_PACKET whether is repetive or not
    //if icmp_seq more than the size of m_bPacketNo, icmp_seq reduce m_nPacketNoLimit
//    if(recv_icmp->icmp_seq - m_nPacketNoLimit >= MAX_PACKET_NO)
//    {
//        Init_PacketNoArray();
//        m_nPacketNoLimit += MAX_PACKET_NO;
//    }
//    //judge whether there exist loop on the way to destination
//    if(m_bPacketNo[recv_icmp->icmp_seq - m_nPacketNoLimit] == true)
//    {
//        std::cout << " DUP!\n";
//        return -1;
//    }
//    else
//    {
//        std::cout << "\n";
//        m_bPacketNo[recv_icmp->icmp_seq - m_nPacketNoLimit] = true;
//        return 1;
//    }
    return packSize;
}

void Ping::tv_sub(struct timeval *in, struct timeval *out)
{
    int sec = in->tv_sec - out->tv_sec;
    int usec = in->tv_usec - out->tv_usec;
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
