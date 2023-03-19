#ifndef IPSCANNER_H
#define IPSCANNER_H

#include <QHostAddress>
#include <QList>
#include <QMap>
#include <QSet>

class IpScanner
{
private:
    static bool _verbose;

    static void log(const QString& mag);
public:                
    static QMap<QString, QSet<int>> Scan(QHostAddress ip, int i1, int i2, QSet<int> p, int ptimeout, int pn, int timeout);
    static QList<QHostAddress> GetLocalAddresses();
    static void setVerbose(bool v);
};

#endif // IPSCANNER_H
