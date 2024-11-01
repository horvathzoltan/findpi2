#include <QCommandLineParser>
#include <QCoreApplication>
#include "logger.h"
#include "signalhelper.h"
#include "commandlineparserhelper.h"
#include "work1.h"


/*
#!/bin/bash

./findpi_ -m "b8:27:eb:e3:cc:41=pi_1027 b8:27:eb:96:ca:bf=pi_1010 b8:27:eb:f4:81:bc=pi_1015 b8:27:eb:64:fd:cf=pi_1023 dc:a6:32:74:c8:db=sdcloner f0:2f:74:f7:bb:76=i7_11" -s Aladar123

*/


// Projects/Run/CommanLineArguments
// -m "b8:27:eb:e3:cc:41=pi1 b8:27:eb:e3:cc:42=pi2"
//
// setcap cap_net_raw,cap_net_admin=eip /home/zoli/source/build/build-findpi2-Desktop_Qt_5_15_2_GCC_64bit-Debug/findpi2
auto main(int argc, char *argv[]) -> int
{
    Logger::Init(Logger::ErrLevel::INFO, Logger::DbgLevel::TRACE, true, true);

    SignalHelper::setShutDownSignal(SignalHelper::SIGINT_); // shut down on ctrl-c
    SignalHelper::setShutDownSignal(SignalHelper::SIGTERM_); // shut down on killall

    zInfo(QStringLiteral("started"))

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("test1"));

    QCommandLineParser parser;

    parser.setApplicationDescription(QStringLiteral("command line test1 app."));
    parser.addHelpOption();
    parser.addVersionOption();

//const QString OPTION_IP = QStringLiteral("ip");
  //  const QString OPTION_MAC = QStringLiteral("mac");
//    const QString OPTION_PWD = QStringLiteral("secret");
    //    const QString OPTION_OUT = QStringLiteral("output");
//    const QString OPTION_BACKUP = QStringLiteral("backup");

    CommandLineParserHelper::addOption(&parser, "ip", QStringLiteral("ipaddress"));
    CommandLineParserHelper::addOption(&parser, "mac", QStringLiteral("macaddress"));
    CommandLineParserHelper::addOption(&parser, "secret", QStringLiteral("secret"));
//    CommandLineParserHelper::addOption(&parser, OPTION_OUT, QStringLiteral("csv file as output"));
//    CommandLineParserHelper::addOptionBool(&parser, OPTION_BACKUP, QStringLiteral("set if backup is needed"));

    parser.process(app);

    Work1 w1;


    Work1::Params w1Params = Work1::Params::Parse(parser);
    w1Params.GetHostAddress();

    auto w1Result = w1.doWork(w1Params); // indítás direkt   

    return (w1Result.state==Work1::Result::Ok)?0:-1;
}


