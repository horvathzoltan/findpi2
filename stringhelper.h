#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <QString>

class StringHelper
{
public:
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    static const Qt::SplitBehavior SplitBehavior;
#elif QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    static const QString::SplitBehavior SplitBehavior;
#endif
};

#endif // STRINGHELPER_H
