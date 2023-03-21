#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QDebug>

class Downloader
{
public:
   struct Output
    {
        QString stdOut;
        QString stdErr;
        int exitCode;
        QString ToString();
    };

    static Output Execute(const QString& cmd, const QStringList& args, int timeout = -1);
    static bool Wget(const QString &url, const QString &filename);
    static QString AvahiResolve(const QString &filename);
};

#endif // DOWNLOADER\_H
