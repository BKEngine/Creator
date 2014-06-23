#include "debugout.h"

DebugOut::DebugOut(QObject *parent) :
    QObject(parent)
{
}

void DebugOut::STDout(int pos ,const QString &info)
{
    QString temp ;
    temp.setNum(pos) ;
    *s << "位置"+temp+"处："+info << endl ;
}

void DebugOut::SetX(BkeParser *p,QTextStream *a)
{
    s = a ;
    connect(p,SIGNAL(ErrorAt(int,QString)),this,SLOT(STDout(int,QString))) ;
}
