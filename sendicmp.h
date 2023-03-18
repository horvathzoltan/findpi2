#ifndef SENDICMP_H
#define SENDICMP_H

#include <fcntl.h>
#include <netinet/ip_icmp.h>
#include <string>

#define PACKETSIZE  64

class Ping
{
//    struct packet
//    {
//        struct icmphdr hdr;
//        char msg[PACKETSIZE-sizeof(struct icmphdr)];
//    };
    uint16_t pid=0;
    struct protoent *proto=nullptr;
    int cnt=1;
    int packet_length;
    int data_length;
    struct sockaddr_in addr_ping;
    std::string data;
public:
    unsigned short checksum(char *, int len);
    bool ping(char*);
    int unpack(char* pckt, int);
    void tv_sub(timeval *in, timeval *out);
};

#endif // SENDICMP_H
