#ifndef QSCILEXERBKESCRIPT_H
#define QSCILEXERBKESCRIPT_H

#include <QObject>
#include <Qsci/qscilexer.h>
#include <Qsci/qscistyle.h>
#include "function.h"

class HighlightAttribute{
public:
    QFont font ;
    unsigned int fc ;
    unsigned int bc ;
};

class QsciLexerBkeScript : public QsciLexer
{
    Q_OBJECT
public:
    QsciLexerBkeScript(QObject *parent=0);

    const char *language() const ;
    const char * lexer () const ;
	virtual QString description(int style) const;
	virtual QColor defaultColor(int style) const;
	virtual QColor defaultPaper(int style) const;
	QFont font(int style) const;
    QFont defaultFont (int style) const ;
    QStringList autoCompletionWordSeparators() const ;
    const char * blockStart (int *style=0) const ;
    void ReadConfig(QString hname) ;
    QString ConfigName() ;
    QStringList ConfigList() ;
	virtual const char *keywords(int set) const;

    HighlightAttribute hlb[32] ;
private:

    QFont Lfont ;
};

#endif // QSCILEXERBKESCRIPT_H
