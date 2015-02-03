#include "mainwindow.h"
#include "weh.h"
#include <QApplication>



int main(int argc, char *argv[])
{
//#ifndef QT_DEBUG
    //取得exe的运行路径
    QTextCodec *gbkcd = QTextCodec::codecForLocale() ;
    QFileInfo info(gbkcd->toUnicode(QByteArray(argv[0]))) ;
    CURRENT_DIR = info.path() ;
    QApplication::setLibraryPaths( QApplication::libraryPaths() << CURRENT_DIR ) ;
//#endif
//#ifdef QT_DEBUG
//    CURRENT_DIR = "F:/work/qt/update/file" ;
//#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    if( argc < 3 ) HTTP_ADRESS = "http://bke.bakerist.info/update/" ;
    else HTTP_ADRESS = "http://bke.bakerist.info/update/test/" ;

    w.DownInfo();

    return a.exec();
}
