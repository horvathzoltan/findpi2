#include "work1.h"
#include "logger.h"
#include "nameof.h"
#include "ipscanner.h"

Work1::Params::Params(QString _ipAddress){
    ipAddress = QHostAddress(_ipAddress);
    if(ipAddress.isNull())
    {
        auto addresses = IpScanner::GetLocalAddresses();
        if(!addresses.isEmpty()) {
            ipAddress = addresses.first();
        }
    }
}

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
    zInfo(QStringLiteral("params: %1").arg(params.ipAddress.toString()));

    IpScanner::setVerbose(false);
    QMap<QString, QSet<int>> result =
            IpScanner::Scan(params.ipAddress, 1, 254, {22, 1997, 8080}, 30);

    for(auto&key:result.keys())
    {
        QSet<int> values = result[key];
        QString str;//ports
        for(auto&v:values){
            if(!str.isEmpty()) str+=',';
            str+=QString::number(v);
        };
        zInfo("ip:" + key + ":" +str);
    }
    return {Result::State::Ok, 55};
}

