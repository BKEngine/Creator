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

    QString platform;

#ifdef Q_OS_WIN
    platform = "win/";
#endif
#ifdef Q_OS_LINUX
    platform = "";
#endif
#ifdef Q_OS_Mac
    platform = "mac/";
#endif


    if( argc < 3 ) HTTP_ADRESS = "http://bke.bakerist.info/update/"+platform ;
    else HTTP_ADRESS = "http://bke.bakerist.info/update/test/"+platform ;

    w.DownInfo();

    return a.exec();
}
