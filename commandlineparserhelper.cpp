#include "commandlineparserhelper.h"
#include <QStringList>

CommandLineParserHelper::CommandLineParserHelper()= default;

// option with its value
bool CommandLineParserHelper::addOption(QCommandLineParser *p, const QString& o, const QString& d){
    return p->addOption({QStringList{o.left(1),o},d,o});
}

// option without value
bool CommandLineParserHelper::addOptionBool(QCommandLineParser *p, const QString& o, const QString& d){
    ////parser.addOption(QCommandLineOption{QStringList {OPTION_BACKUP[0], OPTION_BACKUP},QStringLiteral("set if backup is needed")});
    return p->addOption({QStringList{o.left(1),o},d});
}
