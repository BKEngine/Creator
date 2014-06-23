#ifndef LOLI_ISLAND_H
#define LOLI_ISLAND_H

#include <QtCore>


namespace LOLI{
    bool AutoWrite(const QString &file,const QString &text) ;
    bool AutoWrite(const QString &name,QByteArray data) ;
    bool AutoWrite(QFile *file,const QString &text,const char *codecname) ;
    bool AutoRead(QString &text,const QString &name) ;
    bool AutoRead(QString &text,QFile *file) ;
    QString isValidUTF8(QFile *file) ;
    void makeNullFile(const QStringList &list,const QString &dir ) ;
    bool P_OpenFile(QFile *file) ;
}








#endif // LOLI_ISLAND_H
