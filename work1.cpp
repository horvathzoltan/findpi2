#include "work1.h"
#include "logger.h"
#include "nameof.h"

#include <QCoreApplication>
#include <QThread>

Work1::Work1(bool isEventLoopNeeded)
{
    _isEventLoopNeeded = isEventLoopNeeded;
    //connect(this, &Work1::finished, this, &QObject::deleteLater);
    if(isEventLoopNeeded) QObject::connect(this, &Work1::finished, QCoreApplication::instance(), QCoreApplication::quit, Qt::QueuedConnection);
    //else {
    //connect(this, &Work1::resultReady, this, &Work1::handleResults);

    //connect(this, &Work1::finished, this, &QObject::deleteLater);
    //}
}

auto Work1::init(Work1::Params p) -> bool
{
    _isInited = false;
    if(!p.IsValid()) return false;
    params = p;
    result = { Result::State::NotCalculated, -1};
    _isInited = true;
    return true;
}

auto Work1::Params::IsValid() -> bool
{
    QStringList err;
//    if(inFile.isEmpty())
//    {
//        err.append(QStringLiteral("inFile is empty"));
//    }
//    if(inFile.isEmpty())
//    {
//        err.append(QStringLiteral("outFile is empty"));
//    }
    if(!err.isEmpty()) zInfo(err)
    return err.isEmpty();
}

auto Work1::Result::ToString() -> QString
{
    if(state==Ok) return QStringLiteral("a: ")+QString::number(value);
    if(state==NoResult) return nameof(NoResult);
    if(state==NotCalculated) return nameof(NotCalculated);
    return QStringLiteral("unknown");
}

auto Work1::doWork() -> Result
{
    if(!_isInited) return {Result::State::NoResult, 0};
    auto e = doWork2();
    if(_isEventLoopNeeded) emit finished();
    return e;
}

void Work1::run() {
    if(!_isInited) return;
    if(!_isEventLoopNeeded) return;
    result = doWork2();
    if(_isEventLoopNeeded) emit finished();
}

auto Work1::doWork2() -> Result
{
    zInfo(QStringLiteral("params: %1, %2, %3").arg(params.inFile).arg(params.outFile).arg(params.isBackup));

    for(int i=0;i<10;i++)
    {
        QThread::msleep(500);
            zInfo(QStringLiteral("Work1: ")+QString::number(i));
    }
    return {Result::State::Ok, 55};
}

