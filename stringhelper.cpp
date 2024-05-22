#include "stringhelper.h"

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
    const Qt::SplitBehavior StringHelper::SplitBehavior = Qt::SplitBehaviorFlags::SkipEmptyParts,
#elif QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
    const QString::SplitBehavior StringHelper::SplitBehavior = QString::SplitBehavior::SkipEmptyParts;
#endif

