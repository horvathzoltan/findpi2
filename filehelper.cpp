#include "filehelper.h"

#include <QFileInfo>

bool FileHelper::_verbose = false;

void FileHelper::setVerbose(bool v)
{
    _verbose = v;
}

QStringList FileHelper::LoadLines(const QString& filename) {
    QFileInfo fi(filename);
    if(!fi.isAbsolute())
    {
        if(_verbose)
            qDebug()<<QStringLiteral("path is not absolute: %1").arg(filename);
        return QStringList();
    }

    if(!fi.exists())
    {
        if(_verbose)
            qDebug()<<QStringLiteral("file not exist: %1").arg(filename);
        return QStringList();
    }

    QFile f(filename);
    QStringList e;

    // TODO ha relatív a filename, akkor abszolúttá kell tenni
    // egyébként megnyitható azaz

    if (f.open(QIODevice::ReadOnly))  {
        if(_verbose)
            qDebug()<<QStringLiteral("loaded: %1").arg(filename);
        QTextStream st(&f);
        st.setEncoding(QStringConverter::Utf8);
        do
        {
            e << st.readLine();
        } while (!st.atEnd());
        f.close();
    }
    else{
        if(_verbose)
            qDebug()<<QStringLiteral("cannot read file (%1): %2").arg(f.errorString()).arg(filename);
        e= QStringList();
    }
    return e;
}

QStringList FileHelper::LoadLinesContains(const QString& filename, const QStringList& t1) {
    QFileInfo fi(filename);
    if(!fi.isAbsolute())
    {
        if(_verbose)
            qDebug()<<QStringLiteral("path is not absolute: %1").arg(filename);
        return QStringList();
    }

    if(!fi.exists())
    {
        if(_verbose)
            qDebug()<<QStringLiteral("file not exist: %1").arg(filename);
        return QStringList();
    }

    QFile f(filename);
    QStringList e;

    // TODO ha relatív a filename, akkor abszolúttá kell tenni
    // egyébként megnyitható azaz

    QStringList t2;
    for(auto&t:t1) t2<<t.toUpper();
    if (f.open(QIODevice::ReadOnly))  {
        if(_verbose)
            qDebug()<<QStringLiteral("loaded: %1").arg(filename);
        QTextStream st(&f);
        st.setEncoding(QStringConverter::Utf8);
        do
        {
            auto line = st.readLine();
            auto l2 = line.toUpper();
            bool minden_van = true;
            for(auto&t:t2){
                if(!l2.contains(t))
                {
                    minden_van = false;
                    break;
                }
            }
            if(minden_van) e<<line;

        } while (!st.atEnd());
        f.close();
    }
    else{
        if(_verbose)
            qDebug()<<QStringLiteral("cannot read file (%1): %2").arg(f.errorString()).arg(filename);
        e= QStringList();
    }
    return e;
}
