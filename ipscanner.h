#ifndef IPSCANNER_H
#define IPSCANNER_H

#include <QHostAddress>
#include <QList>
#include <QMap>
#include <QSet>

class IpScanner
{
public:        
    static QMap<QString, QSet<int>> Scan(QHostAddress ip, int i1, int i2, QSet<int> p);
    static QList<QHostAddress> GetLocalAddresses();
};

#endif // IPSCANNER_H
