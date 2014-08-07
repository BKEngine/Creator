#ifndef DEBUGOUT_H
#define DEBUGOUT_H

#include <QObject>
#include <QTextStream>
#include "parser.h"

class DebugOut : public QObject
{
    Q_OBJECT
public:
    explicit DebugOut(QObject *parent = 0);
    void SetX(BkeParser *p,QTextStream *a) ;

signals:

public slots:
    void STDout(int pos ,const QString &info) ;
private:
    QTextStream *s ;

};

#endif // DEBUGOUT_H
