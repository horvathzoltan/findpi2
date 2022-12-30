#include "work1.h"
#include "logger.h"
#include "nameof.h"

auto Work1::Params::IsValid() -> bool
{
    QStringList err;

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

auto Work1::doWork(Params params) -> Result
{
    zInfo(QStringLiteral("params: %1, %2, %3").arg(params.ipAddress));

    for(int i=0;i<10;i++)
    {
        QThread::msleep(500);
            zInfo(QStringLiteral("Work1: ")+QString::number(i));
    }
    return {Result::State::Ok, 55};
}

