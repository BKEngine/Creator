#include "loli_island.h"

bool LOLI::AutoWrite(const QString &file,const QString &text)
{
    QFile f( file ) ;
    bool s = LOLI::AutoWrite(&f,text,"UTF-8") ;
    f.close();
    return s ;
}

bool LOLI::P_OpenFile(QFile *file)
{
    if( !file->isOpen() && !file->open(QFile::ReadWrite) ) return false ;
    //增加写属性
    else if( !(file->openMode()&QFile::WriteOnly) ){
        file->close();
        file->open(QFile::ReadWrite) ;
    }
    return true ;
}

bool LOLI::AutoWrite(QFile *file,const QString &text,const char *codecname)
{
    if( !P_OpenFile(file) ) return false ;

    file->resize(0) ;
    QTextStream strem ;
    strem.setDevice(file);
    strem.setCodec(QTextCodec::codecForName(codecname));
    bom(strem) ;
    strem << text ;
    strem.flush();
    return true ;
}

bool LOLI::AutoWrite(const QString &name,QByteArray data)
{
    QFile file( name ) ;
    if( !P_OpenFile(&file) ) return false ;

    file.resize(0) ;
    file.write(data) ;
    file.close();
    return true ;
}

bool LOLI::AutoRead(QString &text,const QString &name)
{
    QFile file( name ) ;
    return LOLI::AutoRead(text,&file) ;
}

bool LOLI::AutoRead(QString &text,QFile *file)
{
    if( !file->exists() ){ //打开失败
        text.clear();
        return false ;
    }
    else if( !file->isOpen() && !file->open(QFile::ReadWrite) ){ //文件不存在
        text.clear();
        return false ;
    }
    else if( !(file->openMode()&QFile::ReadOnly) ){//没有读属性时，增加读属性
        file->close();
        file->open(QFile::ReadOnly) ;
    }

    file->seek(0) ;  //重置读写位置
    QTextStream strem ;
    //检测utf字节序列，默认使用本地8位码
    QTextCodec *codec = QTextCodec::codecForUtfText(file->read(16),QTextCodec::codecForLocale());
    file->seek(0) ;//重置读写位置

    if( codec == QTextCodec::codecForLocale() ){
        text = isValidUTF8(file) ;  //是否为不带bom的utf8文件
        if( !text.isEmpty() ){
            file->close();
            return true ;
        }
        file->seek(0) ;
    }

    strem.setDevice(file);
    strem.setCodec(codec);
    text = strem.readAll() ;
    file->close();
    return true ;
}

QString LOLI::isValidUTF8(QFile *file)
{
    QByteArray byte = file->readAll() ;
    const uchar *unsigned_buffer= (const uchar *)byte.constData() ;
    for (ulong a = 0 ; a < byte.length() ; a++ ){
        ulong char_len;
        if (!(*unsigned_buffer&128))
            char_len=1;
        else if ((*unsigned_buffer&224)==192)
            char_len=2;
        else if ((*unsigned_buffer&240)==224)
            char_len=3;
        else if ((*unsigned_buffer&248)==240)
            char_len=4;
        else
            return QString() ;
        unsigned_buffer++;
        if (char_len<2)
            continue;
        a++;
        for (ulong b=1; b < char_len; b++ , a++ , unsigned_buffer++)
            if (*unsigned_buffer<0x80 || (*unsigned_buffer&0xC0)!=0x80)
                return QString() ;
    }
    QTextCodec *codec = QTextCodec::codecForName("UTF-8") ;
    return codec->toUnicode(byte) ;
}

void LOLI::makeNullFile(const QStringList &list,const QString &dir )
{
    QStringList ls ;
    if( !dir.isEmpty() ){
        for( int i = 0 ; i < list.size() ; i++){
            ls.append( dir+"/" + list.at(i) );
        }
    }
    else ls = list ;

    QFile f ;
    QFileInfo info ;
    QDir llm ;
    for( int i = 0 ; i < ls.size() ; i++){
        info.setFile( ls.at(i) );
        llm.mkpath( info.path() ) ;
        f.setFileName( ls.at(i) );
        f.open( QFile::ReadWrite ) ;
        f.close();
    }

}

bool LOLI::AutoRead(QByteArray &dest,const QString &name)
{
    QFile file(name) ;
    if( !file.exists() ) return false ;
    if( !file.open(QFile::ReadOnly) ) return false ;

    if( file.size() < 50000000 ) dest = file.readAll() ;
    else dest = file.read( 50000000 ) ;
    file.close();
    return true ;
}

