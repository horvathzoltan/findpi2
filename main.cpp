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

    const QString OPTION_IN = QStringLiteral("input");
    const QString OPTION_OUT = QStringLiteral("output");
    const QString OPTION_BACKUP = QStringLiteral("backup");

    CommandLineParserHelper::addOption(&parser, OPTION_IN, QStringLiteral("gerber file as input"));
    CommandLineParserHelper::addOption(&parser, OPTION_OUT, QStringLiteral("csv file as output"));
    CommandLineParserHelper::addOptionBool(&parser, OPTION_BACKUP, QStringLiteral("set if backup is needed"));

    parser.process(app);

    bool isEventLoopNeeded = false;

    auto w1 =  new Work1(isEventLoopNeeded);

    bool isok = w1->init({
         parser.value(OPTION_IN),
         parser.value(OPTION_OUT),
         parser.isSet(OPTION_BACKUP)
    });

    if(!isok){
        return 1;
    }

    //Work1::Result a;
    auto a = w1->doWork(); // indítás direkt
    //w1->start(); // indítás szálon
    zInfo(QStringLiteral("waiting..."));

    int e = isEventLoopNeeded?QCoreApplication::exec():0;

    zInfo(a.ToString());

    if(w1->result.state==Work1::Result::State::NotCalculated &&
        !isEventLoopNeeded) zInfo(QStringLiteral("NoEventLoop"));

    if(!e) zInfo(QStringLiteral("Everything went ok."));
    return e;
}


