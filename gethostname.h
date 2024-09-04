#ifndef GETHOSTNAME_H
#define GETHOSTNAME_H

#include "downloader.h"

#include <QString>



class GetHostName
{
    struct in_addr
    {
        unsigned long s_addr;    // load with inet_pton()
    };

    struct OuiModel{
        QString oui;
        QString vendor;
    };



    static Downloader* _downloader;
    static quint32 _downloader_status;
public:
    struct LocalIpModel{
        QString hostname;
        QString mac;
        QString ip;
    };

    static QString get(const QString& addr);
    static QString getMac(const QString& addr);
    static QString GetVendor(const QString &mac);
    static bool Download(const QString &addr, const QString& filename);
    static QList<GetHostName::OuiModel> LoadData(const QString& filename);
    static void readFile();
    static QList<LocalIpModel> GetAllLocalIp();
//private:
//static void zInfo(const char *);
};

#endif // GETHOSTNAME_H
