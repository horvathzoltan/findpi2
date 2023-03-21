#ifndef WORK1_H
#define WORK1_H

#include <QHostAddress>
#include <QString>
#include <QThread>

class Work1
{
public:
    struct Params{
        explicit Params(const QString& ipAddress, const QString& mac);
        QHostAddress ipAddress;
        QMap<QString,QString> macAddress;

        bool IsValid();
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
