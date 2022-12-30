#include <QCommandLineParser>
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include "logger.h"
#include "signalhelper.h"
#include "commandlineparserhelper.h"
//#include "common/coreappworker/coreappworker.h"
#include "work1.h"
//#include "coreappworker2.h"

auto main(int argc, char *argv[]) -> int
{
    Logger::Init(Logger::ErrLevel::INFO, Logger::DbgLevel::TRACE, true, true);

    SignalHelper::setShutDownSignal(SignalHelper::SIGINT_); // shut down on ctrl-c
    SignalHelper::setShutDownSignal(SignalHelper::SIGTERM_); // shut down on killall

    zInfo(QStringLiteral("started"));

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("test1"));

    QCommandLineParser parser;

    parser.setApplicationDescription(QStringLiteral("command line test1 app."));
    parser.addHelpOption();
    parser.addVersionOption();  

    const QString OPTION_IP = QStringLiteral("ip");
//    const QString OPTION_OUT = QStringLiteral("output");
//    const QString OPTION_BACKUP = QStringLiteral("backup");

    CommandLineParserHelper::addOption(&parser, OPTION_IP, QStringLiteral("ipaddress"));
//    CommandLineParserHelper::addOption(&parser, OPTION_OUT, QStringLiteral("csv file as output"));
//    CommandLineParserHelper::addOptionBool(&parser, OPTION_BACKUP, QStringLiteral("set if backup is needed"));

    parser.process(app);

    Work1 w1;

    Work1::Params w1Params(parser.value(OPTION_IP));
    auto w1Result = w1.doWork(w1Params); // indítás direkt

    zInfo(QStringLiteral("wa iting..."));

    zInfo(w1Result.ToString());

    return (w1Result.state==Work1::Result::Ok)?0:-1;
}


