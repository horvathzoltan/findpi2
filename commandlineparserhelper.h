#ifndef COMMANDLINEPARSERHELPER_H
#define COMMANDLINEPARSERHELPER_H

#include <QCommandLineParser>

class CommandLineParserHelper
{
public:
    CommandLineParserHelper();
    static bool addOption(QCommandLineParser *p, const QString& o, const QString& d);
    static bool addOptionBool(QCommandLineParser *p, const QString& o, const QString& d);
};

#endif // COMMANDLINEPARSERHELPER_H
