#ifndef WORK1_H
#define WORK1_H

#include <QCommandLineParser>
#include <QHostAddress>
#include <QString>
#include <QThread>

class Work1
{
public:
    struct Params{
        QString _ip;
        QString _mac;

        QHostAddress ipAddress;
        QMap<QString,QString> macAddress;
        QString _pwd;

        //bool IsValid();

        void GetHostAddress();
        static Params Parse(const QCommandLineParser& parser);
    };

    struct Result{
        enum State:int {NotCalculated = -3, NoResult=-1, Ok=1};
        State state;
        int value;

        QString ToString();
    };

    Result doWork(Params params);
};

#endif // WORK1_H
