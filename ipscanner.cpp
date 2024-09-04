#include "ipscanner.h"
#include "logger.h"
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
// aaa
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

    QList<int> opened_ports;

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
            for (const int &port : ports) {
                socket.connectToHost(address, port, QIODevice::ReadWrite);
                bool ok = socket.waitForConnected(timeout);

                if (ok) {
                    socket.disconnectFromHost();
                    auto a = address.toString();
                    ipList[a].insert(port);
                    if (!ports_txt.isEmpty())
                        ports_txt += ',';

                    opened_ports.append(port);
                    ports_txt += QString::number(port);;

                    if (port == 22)
                        hasSSHPort = true;
                }
            }

            QString vl =  vendor.toLower();
            bool isRasPi = vl.startsWith("ras")&&vl.contains("pi");

            QString msg0 = mac_str;

            msg0 += " ("+ ip_str;
            if(!ports_txt.isEmpty())
                msg0 += ":"+ports_txt;
            if(!hostname_str.isEmpty())
                msg0+=" "+hostname_str;
            msg0+=")";

            if(!isRasPi){                
                if(!vendor.isEmpty())
                    msg0+=' '+vendor;
            }



// ssh-keygen -R '172.16.1.232'
// ssh pi@172.16.1.232 -o StrictHostKeyChecking=no cat /sys/firmware/devicetree/base/model

            // if(ip_str == "172.16.1.44"){
            //     zInfo("rpi");
            // }
            if(hostname_str.endsWith("(localhost)"))
            {
                msg0 = "\033[1;33m"+msg0+"\e[0m";
            }
            else if(isRasPi)
            {
                int lenmeg1 = mac_str.length()+1;
                QString msg1;
                QString msg2;

                QString projectName;
                QString hwinfo_desc;
                QString piModelName;
                QString osName;
                bool hasSMBPort = opened_ports.contains(445);
                bool hasRPCPort = opened_ports.contains(135);
                bool hasHTTPPort = opened_ports.contains(8080);

                bool isLin = false;
                bool isWin = false;
                if(hasSSHPort){
                    //QString cmd1 = QStringLiteral("ssh-keygen -R '%1'").arg(ip_str);
                    //ProcessHelper::Output out1 = ProcessHelper::ShellExecute(cmd1, 5000);
                    if(true){//out1.exitCode==0){
                        //sshpass -p qw ssh pi@172.16.1.110 cat /sys/firmware/devicetree/base/model
                        QString sshpass = QStringLiteral("sshpass -p qw");
                        QString cmd0 = QStringLiteral("ssh pi@%1 cat /sys/firmware/devicetree/base/model").arg(ip_str);
                        ProcessHelper::Output out0 = ProcessHelper::ShellExecute(sshpass+" "+cmd0, 5000);

                        if(out0.exitCode==0 && !out0.stdOut.isEmpty())
                        {
                            isLin = true;
                            QStringList lines = out0.stdOut.split('\n');
                            if(lines.count()>=1){
                                piModelName = lines[0];
                            }                            
                        } else{
                            if(hasRPCPort || hasSMBPort){
                                isWin = true;
                            }
                        }

                        if(isLin)
                        {

                            QString cmd3 = QStringLiteral("ssh pi@%1 cat /etc/os-release | egrep \"PRETTY_NAME\" | cut -d = -f 2 | tr -d '\"'").arg(ip_str);
                            ProcessHelper::Output out3 = ProcessHelper::ShellExecute(sshpass+" "+cmd3, 5000);

                            if(out3.exitCode==0 && !out3.stdOut.isEmpty())
                            {
                                QStringList lines = out3.stdOut.split('\n');
                                if(lines.count()>=1){
                                    osName = lines[0];
                                }
                            }

                            QString cmd1 = QStringLiteral("ssh pi@%1 readlink /home/pi/run").arg(ip_str);
                            ProcessHelper::Output out1 = ProcessHelper::ShellExecute(sshpass+" "+cmd1, 5000);

                            QString projectFolder;
                            if(out1.exitCode==0 && !out1.stdOut.isEmpty())
                            {
                                QStringList lines = out1.stdOut.split('\n');
                                if(lines.count()>=1){
                                    QString line = lines[0];
                                    int ix = line.lastIndexOf('/');
                                    if(ix>0){
                                        projectName = line.mid(ix+1);
                                        projectFolder = line.left(ix);
                                    }
                                }
                            }

                            QString hwinfoName = "hwinfo.csv";
                            if(!projectFolder.isEmpty()){
                                QString hwifoFullPath = projectFolder+'/'+hwinfoName;

                                QString cmd2 = QStringLiteral("ssh pi@%1 cat %2").arg(ip_str).arg(hwifoFullPath);
                                ProcessHelper::Output out2 = ProcessHelper::ShellExecute(sshpass+ " " +cmd2, 5000);
                                if(out2.exitCode==0 && !out2.stdOut.isEmpty()){
                                    QStringList lines = out2.stdOut.split('\n');
                                    if(lines.count()>=1){
                                        QString line = lines[0];
                                        int ix2 = line.indexOf(';');
                                        if(ix2>0){
                                            hwinfo_desc = line.mid(ix2+1);
                                        }
                                    }
                                }
                            }
                        } else if(isWin){
                            osName = "win";
                        }
                    }
                }
                if(!piModelName.isEmpty()){
                    msg0 += " "+piModelName;
                } else{
                    if(!vendor.isEmpty())
                        msg0+=" "+vendor;
                    // if(!hostname_str.isEmpty())
                    //     msg+=","+hostname_str;
                }

                if(!osName.isEmpty()){
                    msg1+= osName;
                }

                if(!projectName.isEmpty()){
                    msg2+= projectName;
                }
                if(!hwinfo_desc.isEmpty()){
                    msg2+= " ["+hwinfo_desc+']';
                }


                if(!msg1.isEmpty()) msg0 += '\n'+QString(lenmeg1, ' ')+msg1;
                if(!msg2.isEmpty()) msg0 += '\n'+QString(lenmeg1, ' ')+msg2;

                // Raspberry Pi az ciánkék
                msg0 = "\033[0;36m"+msg0+"\e[0m";
            }

            QMap<QString, QString>::const_iterator kv = macAddress.constFind(mac_str);
            if(kv!=macAddress.constEnd())
                msg0+=" <- "+kv.value();

            log(msg0+'\n');
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
