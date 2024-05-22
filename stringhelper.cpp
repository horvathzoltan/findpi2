#include "stringhelper.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    const Qt::SplitBehavior StringHelper::SplitBehavior = Qt::SplitBehaviorFlags::SkipEmptyParts;
#else
    const QString::SplitBehavior StringHelper::SplitBehavior = QString::SplitBehavior::SkipEmptyParts;
#endif

