#ifndef QSCILEXERBKESCRIPT_H
#define QSCILEXERBKESCRIPT_H

#include <QObject>
#include <Qsci/qscilexer.h>
#include <Qsci/qscistyle.h>
#include "function.h"

class HighlightAttribute{
public:
    QFont font ;
    QColor fc ;
    QColor bc ;
};

class QsciLexerBkeScript : public QsciLexer
{
    Q_OBJECT
public:
    QsciLexerBkeScript(QObject *parent=0);

    enum
    {
        Default = 0 ,
        Command = 1 ,
        Attribute = 2 ,
        String = 3 ,
        Number = 4 ,
        Lable = 5 ,
        Annotate = 6 ,
        Operators = 7 ,
        Text = 8 ,
        Variable = 9 ,
        UntypeA = 10 ,
        UntypeB = 11 ,
        UntypeC = 12 ,
        Error = 13
    };

    const char *language() const ;
    const char * lexer () const ;
    QString description(int style) const ;
    QColor defaultColor (int style) const ;
    QFont font (int style) const ;
    QFont defaultFont (int style) const ;
    QStringList autoCompletionWordSeparators() const ;
    const char * blockStart (int *style=0) const ;

    void ReadConfig() ;
    void ReadConfig(QString hname) ;
private:

    QFont Lfont ;
    HighlightAttribute hlb[32] ;
};

#endif // QSCILEXERBKESCRIPT_H
