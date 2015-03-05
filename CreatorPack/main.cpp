#include "mainwindow.h"
#include "weh.h"
#include <QApplication>

//#define MADEUPDATE ;


int main(int argc, char *argv[])
{
    CURRENT_DIR = QDir::currentPath() ;

    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    return a.exec();
}
