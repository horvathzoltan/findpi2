#ifndef TEXTFILEHELPER_H
#define TEXTFILEHELPER_H

#include <fileerror.h>
#include <QStringList>
#include <QTextStream>

class TextFileHelper
{
private:
    static bool _verbose;
public:

    void SetVerbose(bool v){ _verbose = v;}

    static void SetUtf8Encoding(QTextStream* st);

    static QString Load(const QString& filename, FileErrors *err);
    static QStringList LoadLines(const QString& filename, FileErrors *err);
    static QStringList LoadLinesContains(const QString& filename, const QStringList& t1, FileErrors *err);

    static bool Save(const QString& txt, const QString& fileName, FileErrors *err, bool isAppend = false);

    static QChar GetLastChar(const QString &fn, FileErrors *err);

};

#endif // TEXTFILEHELPER_H
