#ifndef NAMEOF_H
#define NAMEOF_H

#include <QRegularExpression>
#include <QString>

#define nameof(x) _nameof<0>(#x, sizeof(x))

template<int a>
QString _nameof(const char* y, std::size_t)
{
    QString x(y);
    // [\w]+(?:::|->|.)([\w]+)\(\)
    // (?:::|->|\.)?([\w]+)\(\)$
    static const QRegularExpression refn(QStringLiteral(R"((?:::|->|\.)?([\w]+)\(\)$)"));
    //^&([_a-zA-Z]\w*)\s*(->|\.|::)?\s*([_a-zA-Z]\w*)?$
    //(?:(?:[_a-zA-Z]\w*)(?:->|\.|::))*([_a-zA-Z]\w*)
    //(?:->|\.|::)?([_a-zA-Z]\w*)$
    static const QRegularExpression re(QStringLiteral(R"((?:->|\.|::)?([_a-zA-Z]\w*)$)"));
    QRegularExpressionMatch m;

    if(x.endsWith(QStringLiteral("()"))) m = refn.match(x);
    else m = re.match(x);

    if (m.hasMatch()) return m.captured(m.lastCapturedIndex());

    return QStringLiteral("A bad expression in nameof(%1)").arg(x);
}

#endif // NAMEOF_H
