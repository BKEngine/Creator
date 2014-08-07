#include "bkedoc.h"
#include "function.h"
#include <QFileInfo>

BKEdoc::BKEdoc()
{
}

//返回一个编辑文件
BkeScintilla *BKEdoc::WorkingFile(const QString &file,bool mkNull )
{
    errors.clear();
    QString name = LOLI_OS_QSTRING( file ) ;
    BkeScintilla *temp = sciHash.value(name,0) ;
    if( !mkNull || temp != 0) return temp ;

    sciHash[name] = temp ;  //双向key，value
    winHash[temp] = file ;

    FileList << file ;
    FileList.sort(Qt::CaseInsensitive);
    currentfile = file ;  //置当前文件
    return temp ;
}

//设置文件所属的项目
void BKEdoc::SetFileProject(const QString &file, BKEproject *pro)
{
    QString name = LOLI_OS_QSTRING( file ) ;
    BkeScintilla *temp = sciHash.value(name,0) ;
    if( temp == 0 ) return ; //没有被打开的文件，设置是无效的

    proHash[name] = pro ;
}

//文件所属项目，没有返回0
BKEproject* BKEdoc::FileProject(const QString &file)
{
    QString name = LOLI_OS_QSTRING( file ) ;
    return proHash.value( name,0) ;
}

//关闭窗口
bool BKEdoc::CloseFile(QWidget *win)
{
    QString file = winHash.value(win,QString()) ;
    QString name = LOLI_OS_QSTRING( file ) ;
    if( name.isEmpty() ) return false ;

    BkeScintilla *temp = sciHash.take(name) ;
    winHash.remove(win) ;
    FileList.removeOne(file) ;

    delete temp ;
}

//对应窗口的文件名
QString BKEdoc::WinOfFile(QWidget *win)
{
    return winHash.value(win,QString()) ;
}


void BKEdoc::SetCurrentFile(const QString &file)
{
    currentfile = file ;
}


QStringList BKEdoc::WorkFileList( int &currentpos)
{
    QStringList result ;
    currentpos = FileList.indexOf(currentfile) ;

    int pos ;
    QString temp ;
    for( int i = 1 ; i < FileList.size() ; i++){
        temp = FileList.at(i) ;
        pos  = temp.lastIndexOf("/") ;
        result << temp.right( temp.length() - pos ) ;
    }
    return result ;
}
