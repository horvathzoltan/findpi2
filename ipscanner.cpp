#include "ipscanner.h"
#include "sendicmp.h"
#include <QNetworkInterface>
#include <QTcpSocket>
#include <iostream>
#include <QHostInfo>
#include "gethostname.h"
#include "processhelper.h"

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

QMap<QString, QSet<int>> IpScanner::Scan(QMap<QString,QString> macAddress,
                                         QHostAddress ip, int i1, int i2, QSet<int> ports,
                                         int ptimeout, int pn, int timeout)
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
            QString addst = address.toString();
            mac_str = GetHostName::getMac(addst);
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

            bool hasSSHPort = false;
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

                    if(port==22) hasSSHPort = true;
                }                
            }

            QString vl =  vendor.toLower();
            bool isRasPi = vl.startsWith("ras")&&vl.contains("pi");

            QString msg = mac_str;

            msg += " ("+ ip_str;
            if(!ports_txt.isEmpty())
                msg += ":"+ports_txt;
            msg+=")";

            if(!isRasPi){
                if(!vendor.isEmpty())
                    msg+=" "+vendor;
                if(!hostname_str.isEmpty())
                    msg+=" -> "+hostname_str;
            }

            QMap<QString, QString>::const_iterator kv = macAddress.constFind(mac_str);
            if(kv!=macAddress.constEnd())
                msg+=" <- "+kv.value();


            if(hostname_str.endsWith("(localhost)")){
                msg = "\033[1;33m"+msg+"\e[0m";
            } else if(isRasPi){
                QString projectName;
                QString hwinfo_desc;
                QString piModelName;
                if(hasSSHPort){
                    QString cmd0 = QStringLiteral("ssh pi@%1 cat /sys/firmware/devicetree/base/model").arg(ip_str);
                    ProcessHelper::Output out0 = ProcessHelper::ShellExecute(cmd0, 5000);

                    if(out0.exitCode==0 && !out0.stdOut.isEmpty())
                    {
                        QStringList lines = out0.stdOut.split('\n');
                        if(lines.count()>=1){
                            piModelName = lines[0];
                        }
                    }

                    QString cmd = QStringLiteral("ssh pi@%1 readlink /home/pi/run").arg(ip_str);
                    ProcessHelper::Output out = ProcessHelper::ShellExecute(cmd, 5000);

                    QString projectFolder;
                    QString hwinfoName = "hwinfo.csv";

                    if(out.exitCode==0 && !out.stdOut.isEmpty())
                    {
                        QStringList lines = out.stdOut.split('\n');
                        if(lines.count()>=1){
                            QString line = lines[0];
                            int ix = line.lastIndexOf('/');
                            if(ix>0){
                                projectName = line.mid(ix+1);
                                projectFolder = line.left(ix);
                            }
                         }
                    }

                    if(!projectFolder.isEmpty()){
                        QString hwifoFullPath = projectFolder+'/'+hwinfoName;

                        QString cmd = QStringLiteral("ssh pi@%1 cat %2").arg(ip_str).arg(hwifoFullPath);
                        ProcessHelper::Output out = ProcessHelper::ShellExecute(cmd, 5000);
                        if(out.exitCode==0 && !out.stdOut.isEmpty()){
                            QStringList lines = out.stdOut.split('\n');
                            if(lines.count()>=1){
                                QString line = lines[0];
                                int ix2 = line.indexOf(';');
                                if(ix2>0){
                                    hwinfo_desc = line.mid(ix2+1);
                                }
                            }
                        }
                    }
                }
                if(!piModelName.isEmpty()){
                    msg += " "+piModelName;
                } else{
                    if(!vendor.isEmpty())
                        msg+=" "+vendor;
                    if(!hostname_str.isEmpty())
                        msg+=" -> "+hostname_str;
                }

                if(!projectName.isEmpty()){
                    msg+= ", "+projectName;
                }
                if(!hwinfo_desc.isEmpty()){
                    msg+= " ["+hwinfo_desc+']';
                }

                // Raspberry Pi az ciánkék
                msg = "\033[0;36m"+msg+"\e[0m";
            }
            log(msg+'\n');
        }
    }
    return ipList;
}
/*
export COLOR_NC='\e[0m' # No Color
export COLOR_BLACK='\e[0;30m'
export COLOR_GRAY='\e[1;30m'
export COLOR_RED='\e[0;31m'
export COLOR_LIGHT_RED='\e[1;31m'
export COLOR_GREEN='\e[0;32m'
export COLOR_LIGHT_GREEN='\e[1;32m'
export COLOR_BROWN='\e[0;33m'
export COLOR_YELLOW='\e[1;33m'
export COLOR_BLUE='\e[0;34m'
export COLOR_LIGHT_BLUE='\e[1;34m'
export COLOR_PURPLE='\e[0;35m'
export COLOR_LIGHT_PURPLE='\e[1;35m'
export COLOR_CYAN='\e[0;36m'
export COLOR_LIGHT_CYAN='\e[1;36m'
export COLOR_LIGHT_GRAY='\e[0;37m'
export COLOR_WHITE='\e[1;37m'
*/

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
