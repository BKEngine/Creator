#ifndef AUTOLEX_H
#define AUTOLEX_H

#include <QHash>
#include <Qsci/qsciscintilla.h>
#include <QTextStream>
#include "function.h"


class AutoLex
{
public:
    AutoLex();

    QString RootCommand ;
    QString LastRootCom ;

    const char* WordListOf(const QString &startword) ;
    bool SetApiListOf(const QString &startword) ;
    void BuildRootHash() ;
    QString ReadMacroFile(const QString &dir,const QString &name) ;
    bool RemoveWordFromList(const QString &word) ;
    int  TheListCountIs(){ return WordList.size() ;}

    static QStringList LineToWords(QString &text) ;

private:
    QHash<QString,QString> ApiHash ;
    QString TypeFormat(const QString &text) ;

    bool empty ;
    QByteArray WordListData ;
    QStringList WordList ;

};

#endif // AUTOLEX_H
