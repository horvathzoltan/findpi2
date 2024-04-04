#include "textfilehelper.h"
#include "logger.h"

#include <fileerror.h>
#include <QFileInfo>

bool TextFileHelper::_verbose = false;

void TextFileHelper::SetUtf8Encoding(QTextStream* st)
{
    if(st==nullptr) return;
#if QT_VERSION >= 0x050000 && QT_VERSION < 0x060000
    st->setCodec("UTF-8"); //5.15.2
#elif QT_VERSION >= 0x06
    st->setEncoding(QStringConverter::Utf8);
#endif
}


QString TextFileHelper::Load(const QString& filename, FileErrors *err) {

    bool valid = FnValidator::Validate_Load(filename, err);
    if(!valid) return QString();

    QFile f(filename);

    bool ok = f.open(QFile::ReadOnly | QFile::Text);

    if (!ok){
        if(_verbose) zInfo(QStringLiteral("cannot read file (%1): %2").arg(f.errorString(),filename));
        if(err != nullptr) *err= FileErrors::PathIsNotAbsolute;
        return QString();
    }

    if(_verbose) zInfo(QStringLiteral("Load: %1").arg(filename));
    QTextStream st(&f);
    SetUtf8Encoding(&st);
    QString e = st.readAll();

    if(err != nullptr) *err= FileErrors::Ok;
    return e;
}

QStringList TextFileHelper::LoadLines(const QString& filename,FileErrors *err) {
    bool valid = FnValidator::Validate_Load(filename, err);
    if(!valid) return QStringList();

    //QFileInfo fi(filename);

    QFile f(filename);
    QStringList e;

    bool ok = f.open(QFile::ReadOnly | QFile::Text);

    if(!ok){
        if(_verbose) zInfo(QStringLiteral("cannot read file (%1): %2").arg(f.errorString(),filename));
        if(err != nullptr) *err= FileErrors::CannotRead;
        return QStringList();
    }

    if(_verbose) zInfo(QStringLiteral("LoadLines: %1").arg(filename));
    QTextStream st(&f);
    SetUtf8Encoding(&st);

    while (!st.atEnd())
    {
        e << st.readLine();
    }
    f.close();

    if(err != nullptr) *err= FileErrors::Ok;
    return e;
}


bool TextFileHelper::Save(const QString& txt, const QString& filename, FileErrors *err, bool isAppend) {
    bool valid = FnValidator::Validate_Save(filename, err);
    if(!valid) return false;

    QFile f(filename);

    auto om = QIODevice::WriteOnly | QIODevice::Text; // openmode
    if(isAppend) om |= QIODevice::Append;

    bool opened = f.open(om);

    if (!opened){
        if(_verbose) zInfo(QStringLiteral("cannot write file (%1): %2").arg(f.errorString(),filename));
        if(err != nullptr) *err= FileErrors::CannotWrite;
        return false;
    }

    if(_verbose) zInfo(QStringLiteral("Save: %1").arg(filename));
    QTextStream out(&f);
    SetUtf8Encoding(&out);
    out << txt;
    f.close();

    if(err != nullptr) *err= FileErrors::Ok;
    return true;
}

QStringList TextFileHelper::LoadLinesContains(const QString& filename,
                                              const QStringList& t1,
                                              FileErrors *err) {
    bool valid = FnValidator::Validate_Load(filename, err);
    if(!valid) return QStringList();

    QFile f(filename);

    QStringList t2;
    for(auto&t:t1) t2<<t.toUpper();
    bool opened = f.open(QIODevice::ReadOnly);

    if(!opened){
        if(_verbose) zInfo(QStringLiteral("cannot read file (%1): %2").arg(f.errorString(),filename));
        if(err != nullptr) *err= FileErrors::CannotRead;
        return QStringList();
    }

    if(_verbose) zInfo(QStringLiteral("LoadLinesContains: %1").arg(filename));
    QTextStream st(&f);
    SetUtf8Encoding(&st);

    QStringList e;
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
        if(minden_van) e.append(line);

    } while (!st.atEnd());
    f.close();
    if(err != nullptr) *err= FileErrors::Ok;
    return e;
}

QChar TextFileHelper::GetLastChar(const QString& filename, FileErrors *err){
    bool valid = FnValidator::Validate_Load(filename, err);
    if(!valid) return QChar();

    QFileInfo fi(filename);

    QFile f(filename);

    bool ok = f.open(QIODevice::ReadOnly);
    if(!ok){
        if(_verbose) zInfo(QStringLiteral("cannot read file (%1): %2").arg(f.errorString(),filename));
        if(err != nullptr) *err= FileErrors::CannotRead;
        return QChar();
    }

    //_lastError.clear();
    if(_verbose) zInfo(QStringLiteral("GetLastChar: %1").arg(filename));
    qint64 size = f.size();
    f.seek(size-1);
    QByteArray a = f.read(1);
    f.close();
    QChar e((a.isEmpty())?'\0':a[0]);

    if(err != nullptr) *err= FileErrors::Ok;
    return e;
}
