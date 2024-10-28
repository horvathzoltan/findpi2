#include "downloader.h"
#include "logger.h"

#include <QCoreApplication>
#include <QProcess>

bool Downloader::_verbose = true;

bool Downloader::Wget(const QString &url, const QString &filename)
{
    if(url.isEmpty()) return false;
    if(filename.isEmpty()) return false;
    auto out = Execute("wget",{url, "-O", filename});
    return !out.exitCode;
}

void Downloader::setVerbose(bool newVerbose)
{
    _verbose = newVerbose;
}

Downloader::Output Downloader::Execute(const QString& cmd, const QStringList& args, int timeout){
    QProcess process;
    static QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LD_LIBRARY_PATH", "/usr/lib"); // workaround - https://bugreports.qt.io/browse/QTBUG-2284
    process.setProcessEnvironment(env);
    static auto path = QCoreApplication::applicationDirPath();
    process.setWorkingDirectory(path);

    process.start(cmd,args);

    Output e;

    e.isStarted = process.waitForStarted(-1);
    if(e.isStarted){
        e.isFinished = process.waitForFinished(timeout);
        if(e.isFinished){
            e.stdOut = process.readAllStandardOutput();
            e.stdErr = process.readAllStandardError();
            e.exitCode = process.exitCode();
        }else{
            process.close();
            if(_verbose) zInfo("process cannot finish: "+cmd);
        }
    } else{
        zWarning("process cannot start: "+cmd);
    }

    return e;
}

QString Downloader::AvahiResolve(const QString &ip)
{
    if(ip.isEmpty()) return {};

    QString cmd = "avahi-resolve";
    QStringList cmdParams = {"-a", ip};
    auto out = Execute(cmd, cmdParams, 200);

    if(out.isValid())
    {
        if(out.exitCode==0)
        {
            auto lines = out.stdOut.split('\n');
            if(lines.length()<1) return {};
            auto tokens = lines[0].split('\t');
            if(tokens.length()<2) return {};

            return tokens[1];
        }
    }

    return {};
}
