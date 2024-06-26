#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <QString>

class StringHelper
{
public:
#if QT_VERSION > QT_VERSION_CHECK(5, 15, 0)
    static const Qt::SplitBehavior SplitBehavior;
#else
    static const QString::SplitBehavior SplitBehavior;
#endif
};

#endif // STRINGHELPER_H
