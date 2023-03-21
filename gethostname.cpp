#include "gethostname.h"
#include "filehelper.h"
#include "downloader.h"

#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <QEventLoop>
#include <QFile>
#include <netdb.h>
#include <QTimer>
#include <QNetworkInterface>
#include <QHostInfo>

Downloader* GetHostName::_downloader = nullptr;
quint32 GetHostName::_downloader_status = 0;

QString GetHostName::get(const QString& addr)
{
//    struct sockaddr_in sa;    /* input */
//       socklen_t len;         /* input */
//       char hbuf[NI_MAXHOST];

//       memset(&sa, 0, sizeof(struct sockaddr_in));

//       /* For IPv4*/
//       sa.sin_family = AF_INET;
//       sa.sin_addr.s_addr = inet_addr(addr.toLocal8Bit());
//       len = sizeof(struct sockaddr_in);

//       bool ok = getnameinfo((struct sockaddr *) &sa, len, hbuf, sizeof(hbuf), NULL, 0, NI_NAMEREQD);

//       QString hn;
//       if (ok)
//       {
//           //printf("could not resolve hostname\n");
//       }
//       else {
//           //printf("host=%s\n", hbuf);
//           hn= QString::fromStdString(hbuf);
//       }

       struct hostent *h;
       struct sockaddr_in sin;
       //char domain[512];
       sin.sin_addr.s_addr=inet_addr(addr.toLocal8Bit());
       h = gethostbyaddr((char *)&sin.sin_addr.s_addr,
       sizeof(struct in_addr), AF_INET);

       QString hn;

       if (h!=(struct hostent *)0)
       {
          //strcpy(domain,h->h_name);
          hn= QString::fromStdString(h->h_name);
          //printf("gethostbyaddr was successful\n");
       }
    //   else
       //   printf("gethostbyaddr failed\n");

       return hn;
}

QString GetHostName::getMac(const QString &addr)
{
    auto lines = FileHelper::LoadLines("/proc/net/arp");

    for(auto&line:lines){
        QStringList tokens = line.split(' ', Qt::SplitBehaviorFlags::SkipEmptyParts);
        if(tokens[0]==addr){
            return tokens[3];
        }
    }
    return {};
}

//https://standards-oui.ieee.org/oui/oui.txt
//QtDownload dl;
QString GetHostName::GetVendor(const QString &mac){
    if(mac.isEmpty()) return {};
    static QList<GetHostName::OuiModel> ouiModels =
            Download("https://standards-oui.ieee.org/oui/oui.txt");

    QString mac2 = mac.toUpper();
    for(auto&o:ouiModels){
        if(mac2.startsWith(o.oui)) return o.vendor;
    }
    return {};
}

QList<GetHostName::OuiModel> GetHostName::Download(const QString& url){
     _downloader = new Downloader();
     _downloader_status = 0;
     QObject::connect(_downloader, &Downloader::onReady, &GetHostName::readFile);
     _downloader->getData(url);
     qDebug()<<"file letöltés...";

     for(quint32 i = 0;i<10;i++)
     {
        if(_downloader_status==1) break;
        QEventLoop loop;
        QTimer::singleShot(10 * 1000, &loop, SLOT(quit()));
        loop.exec();
     };

     QList<GetHostName::OuiModel> m;
     QStringList lines = FileHelper::LoadLinesContains("/home/zoli/oui.txt", {"hex"});//,"raspberry"
     int i=0;
     for(auto&line:lines){         
         QStringList tokens1 = line.split('\t', Qt::SplitBehaviorFlags::SkipEmptyParts);         
         if(tokens1.length()<2) continue;
         QStringList tokens2 = tokens1[0].split(' ', Qt::SplitBehaviorFlags::SkipEmptyParts);
         if(tokens2.length()<2) continue;
         if(tokens2[0].isEmpty()) continue;
         if(tokens1[1].isEmpty()) continue;
         OuiModel oui{ .oui=tokens2[0].replace('-', ':').toUpper(), .vendor=tokens1[1]};
         m<<oui;
         i++;
     }
     return m;
}

void GetHostName::readFile(){
    qDebug()<<"file lent van";
    _downloader_status = 1;
}


QList<GetHostName::LocalIpModel> GetHostName::GetAllLocalIp() {
    QList<LocalIpModel> list2;
    QString localhostname =  QHostInfo::localHostName();

    foreach (const QNetworkInterface& networkInterface, QNetworkInterface::allInterfaces()) {
       foreach (const QNetworkAddressEntry& entry, networkInterface.addressEntries()) {
           if(entry.ip().isLoopback()) continue;
           if(entry.ip().protocol() != QAbstractSocket::IPv4Protocol) continue;
           LocalIpModel m{.hostname=localhostname,
                         .mac=networkInterface.hardwareAddress(),
                         .ip=entry.ip().toString()};
           list2<<m;
       }
    }
    return list2;
}


