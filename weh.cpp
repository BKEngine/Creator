#include "weh.h"

QString BKE_CURRENT_DIR ;
QString BKE_PROJECT_NAME("BkeProject.bkp") ;
QString BKE_PROJECT_WORKPRO ;

QString LOLI_MID(const QString &text,const QString from , const QString to,int pos )
{
    int pos1,pos2 ;
    pos1 = text.indexOf(from,pos) ;
    pos2 = text.indexOf(to,pos1+1) ;
    if( pos1 < 0 || pos2 < 0 ) return QString() ;
    return text.mid(pos1+1,pos2-pos1-1) ;
}

QString LOLI_OS_QSTRING(const QString &text)
{
    #ifdef Q_OS_WIN32
    return text.toLower() ;
    #endif
    #ifndef Q_OS_WIN32
    return text ;
    #endif

}

bool    LOLI_MAKE_NULL_FILE(const QString &filename)
{
    QFileInfo temp(filename) ;
    if( !temp.exists()){
        QDir().mkpath(temp.path()) ;
        QFile abc( filename) ;
        if( !abc.open(QFile::ReadWrite)) return false ;
        abc.close();
    }
    return true ;
}

void LOLI_CLEAR_TEMP2(const QString &dir)
{
    QDir temp(dir) ;
    QFileInfoList list = temp.entryInfoList() ;
    QFileInfo info ;

    QFile abc ;
    for( int i = 0 ; i < list.size() ; i++){
        info = list.at(i) ;
        if( info.fileName() == "." || info.fileName() == "..") continue ;
        else if( info.isDir()){
            LOLI_CLEAR_TEMP2(info.filePath());
            QDir k( info.path()) ;
            k.rmdir(info.fileName()) ;
        }
        abc.setFileName(info.filePath());
        abc.remove() ;
    }
}

//=========================class=============
