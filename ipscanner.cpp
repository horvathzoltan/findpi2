#include "ipscanner.h"
#include <QNetworkInterface>
#include <QTcpSocket>
#include <iostream>

QMap<QString, QSet<int>> IpScanner::Scan(QHostAddress ip, int i1, int i2, QSet<int> ports)
{
    if(i1<1||i1>255) return {};
    if(i2<1||i2>255) return {};
    if(i1>i2) return {};
    for(auto&port:ports) if(port<1||port>UINT16_MAX) return {};

    QTcpSocket socket;
    QMap<QString, QSet<int>> ipList;
    QHostAddress address;

    qint32 i = ip.toIPv4Address();
    unsigned char* ip2 = reinterpret_cast<unsigned char*>(&i);//mutató az i LSB-re

    QTextStream out(stdout);

    QString w("|/-\\");
    for(unsigned char u=i1;u<i2;u++){
        *ip2=u;
        address.setAddress(i);
        for(auto&port:ports){
            socket.connectToHost(address, port, QIODevice::ReadWrite);
            bool ok = socket.waitForConnected(200);
            if(ok)
            {
                socket.disconnectFromHost();
                auto a = address.toString();
                ipList[a].insert(port);
                out <<"\r"<<a<<"\n";
                out.flush();
                break;
            }
            else{
                out<<"\rsearching "<<w[u%4]<<'\r';
                out.flush();
            }
        }
    }
    out<<"\r";
    return ipList;
}

QList<QHostAddress> IpScanner::GetLocalAddresses()
{
    QList<QHostAddress> result;
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();

    for(auto&eth:allInterfaces) {
        QList<QNetworkAddressEntry> allEntries = eth.addressEntries();        
        for(auto&entry:allEntries) {
            auto ip = entry.ip();
            if (ip.protocol() != QAbstractSocket::IPv4Protocol) continue;
            if(ip == QHostAddress(QHostAddress::LocalHost)) continue;
            if(entry.netmask().isNull()) continue;
            result.append(ip);
        }
    }
    return result;
}
