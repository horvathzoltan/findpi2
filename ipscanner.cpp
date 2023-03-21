#include "ipscanner.h"
#include "sendicmp.h"
#include <QNetworkInterface>
#include <QTcpSocket>
#include <iostream>
#include <QHostInfo>
#include "gethostname.h"

bool IpScanner::_verbose;

void IpScanner::setVerbose(bool v)
{
    _verbose = v;
}

void IpScanner::log(const QString &msg)
{
    static QTextStream out(stdout);

    out<<msg;
    out.flush();
}

QMap<QString, QSet<int>> IpScanner::Scan(QHostAddress ip, int i1, int i2, QSet<int> ports, int ptimeout, int pn, int timeout)
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
    static const QString w("|/-\\");

    Ping ping;
    ping.setVerbose(false);

    auto localIps = GetHostName::GetAllLocalIp();
    for(unsigned char u=i1;u<i2;u++){
        *ip2=u;
        address.setAddress(i);

        bool ok;

        GetHostName::LocalIpModel* lip2=nullptr;
        for(GetHostName::LocalIpModel &lip : localIps) {
            if (lip.ip == address.toString()) {
                lip2 = &lip;
                break;
            }
        }

        QString l;
        QString mac;
        QString hostname;
        if(lip2!=nullptr)
        {
            l = lip2->ip+"(localhost)";
            mac = lip2->mac;
            hostname = lip2->hostname;
            ok = true;
        }
        else
        {
            Ping::PingResult r = ping.ping(address, ptimeout, pn);
            l = r.ToString();
            mac = GetHostName::getMac(address.toString());
            hostname = GetHostName::get(address.toString());
            ok = r.ok;
        }

        if(ok)
        {                       
            QString vendor = GetHostName::GetVendor(mac);

            QString msg = l;

            if(!hostname.isEmpty())
                msg+=" "+hostname;
            if(!vendor.isEmpty())
                msg+=" "+vendor;

            log("\r"+msg+"\n");
            for(auto&port:ports){
                socket.connectToHost(address, port, QIODevice::ReadWrite);
                bool ok = socket.waitForConnected(timeout);
                //QString a = address.toString();
                if(ok)
                {
                    socket.disconnectFromHost();
                    auto a = address.toString();
                    ipList[a].insert(port);
                    if(_verbose)
                        log("\r"+a+":"+QString::number(port)+"\n");
                }
                else{
                    if(_verbose)
                        log(QStringLiteral("\rsearching ")+w[u%4]+'\r');
                }
            }
        }
    }
    if(_verbose) log("\r");
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
