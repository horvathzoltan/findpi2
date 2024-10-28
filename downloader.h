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
private:
    static bool _verbose;
public:
   struct Output
    {
        QString stdOut;
        QString stdErr;
        int exitCode=1;
        QString ToString();
        bool isStarted = false;
        bool isFinished = false;

        bool isValid(){ return isStarted && isFinished;}
    };



    static Output Execute(const QString& cmd, const QStringList& args, int timeout = -1);
    static bool Wget(const QString &url, const QString &filename);
    static QString AvahiResolve(const QString &filename);
    static void setVerbose(bool newVerbose);
};

#endif // DOWNLOADER\_H
