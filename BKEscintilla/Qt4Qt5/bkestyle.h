#ifndef BKESTYLE_H
#define BKESTYLE_H
#include "Qsci/qscilexer.h"

class BKEstyle : public QsciLexer
{
public:
    enum{
        S_DEFAULT = 0 ,
        S_COMMAND = 1 ,
        S_ATTRIBUTE = 2 ,
        S_STRING = 3 ,
        S_NUMBER = 4 ,
        S_LABEL = 5 ,
        S_ANNOTATE = 6 ,
        S_OPERATORS = 7 ,
        S_TEXT = 8 ,
        S_VARIABLE = 9 ,
        S_UNTYPEA = 10 ,
        S_UNTYPEB = 11 ,
        S_UNTYPEC = 12 ,
        S_ERROR = 13 ,
        MAXCOUNT = 14
    };
    BKEstyle(QObject *parent=0);
    const char* language() const;
    const char* lexer() const ;
    QString description	(int style ) const ;
    QColor defaultColor (int style) const ;
};

#endif // BKESTYLE_H
