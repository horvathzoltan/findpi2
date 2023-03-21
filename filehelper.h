#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <QStringList>



class FileHelper
{
    static bool _verbose;
public:
    static void setVerbose(bool v);
    static QStringList LoadLines(const QString& filename);
    static QStringList LoadLinesContains(const QString &filename, const QStringList &t1);
};

#endif // FILEHELPER_H
