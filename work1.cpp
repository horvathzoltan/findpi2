#include "work1.h"
#include "logger.h"
#include "nameof.h"
#include "ipscanner.h"
#include "processhelper.h"
#include "gethostname.h"
#include <QDebug>

void Work1::Params::GetHostAddress(){
    ipAddress = QHostAddress(_ip);
    if(ipAddress.isNull())
    {
        auto addresses = IpScanner::GetLocalAddresses();
        if(!addresses.isEmpty()) {
            ipAddress = addresses.first();
        }
    }

    if(!_mac.isEmpty())
    {
        QStringList lines = _mac.split(' ');
        if(!lines.isEmpty())
        {
            for(auto&l:lines)
            {
                QStringList tokens = l.split('=');
                if(tokens.length()<2) continue;
                macAddress.insert(tokens[0], tokens[1]);
            }
        }
    }
}

// auto Work1::Params::IsValid() -> bool
// {
//     QStringList err;

//     if(!err.isEmpty()) zInfo(err)
//     return err.isEmpty();
// }

Work1::Params Work1::Params::Parse(const QCommandLineParser &parser){
    Params m;
    m._ip = parser.value("ip");
    m._mac = parser.value("mac");
    m._pwd = parser.value("secret");
    return m;
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

    ProcessHelper::SetPassword(params._pwd);

    GetHostName::Download("https://standards-oui.ieee.org/oui/oui.txt", "oui.txt");
    IpScanner::setVerbose(false);
    QMap<QString, QSet<int>> result =
            IpScanner::Scan(params.macAddress, params.ipAddress, 1, 254, {22, 135, 445, 1997, 8080}, 1000, 3, 1000);

//    QList<QString> keys = result.keys();
//    for (auto&key : keys)
//    {
//        QSet<int> values = result[key];
//        QString str;//ports
//        for(auto&v:values){
//            if(!str.isEmpty()) str+=',';
//            str+=QString::number(v);
//        };
//        zInfo("ip:" + key + ":" +str);
//    }
    return {Result::State::Ok, 55};
}

