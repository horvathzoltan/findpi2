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

QMap<QString, QSet<int>> IpScanner::Scan(QMap<QString,QString> macAddress, QHostAddress ip, int i1, int i2, QSet<int> ports, int ptimeout, int pn, int timeout)
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

        QString ip_str;
        QString mac_str;
        QString hostname_str;
        QString time_str;
        if(lip2!=nullptr)
        {
            ip_str = lip2->ip;
            mac_str = lip2->mac;
            hostname_str = lip2->hostname+"(localhost)";
            time_str = "";
            ok = true;
        }
        else
        {
            Ping::PingResult r = ping.ping(address, ptimeout, pn);
            ip_str = r.fromIp;
            mac_str = GetHostName::getMac(address.toString());
            hostname_str = GetHostName::get(address.toString());

            if(hostname_str.isEmpty())
                hostname_str = Downloader::AvahiResolve(r.fromIp);

            time_str = QString::number(r.time);
            ok = r.ok;
        }

        if(ok)
        {                       
            QString vendor = GetHostName::GetVendor(mac_str);

            QString ports_txt;

            for(auto&port:ports){
                socket.connectToHost(address, port, QIODevice::ReadWrite);
                bool ok = socket.waitForConnected(timeout);

                if(ok)
                {
                    socket.disconnectFromHost();
                    auto a = address.toString();
                    ipList[a].insert(port);
                    if(!ports_txt.isEmpty()) ports_txt+=',';
                    ports_txt+=QString::number(port);
                }                
            }
            QString msg = mac_str;

            msg += " ("+ ip_str;
            if(!ports_txt.isEmpty())
                msg += ":"+ports_txt;
            msg+=")";

            if(!vendor.isEmpty())
                msg+=" "+vendor;

            if(!hostname_str.isEmpty())
                msg+=" -> "+hostname_str;

            QMap<QString, QString>::const_iterator kv = macAddress.constFind(mac_str);
            if(kv!=macAddress.constEnd())
                msg+=" <- "+kv.value();

            log(msg);
        }
    }
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
