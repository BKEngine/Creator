#include <QMainWindow>
#include <QApplication>
#include "Qsci/qsciscintilla.h"
#include "bkestyle.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMainWindow w ;
    QsciScintilla pp ;
    BKEstyle ss ;

    w.setCentralWidget(&pp);
    pp.setLexer(&ss);
    pp.setText("##\r\nif(abc)\r\nreturn\r\n##");
    w.show();
    return a.exec();
}
