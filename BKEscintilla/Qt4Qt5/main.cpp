#include <QMainWindow>
#include <QApplication>
#include "Qsci/qsciscintilla.h"
#include "bkestyle.h"
#include "loli_island.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QMainWindow w ;
    QsciScintilla pp ;
    BKEstyle ss ;

    w.setCentralWidget(&pp);

    pp.setUtf8(true);
    pp.setLexer(&ss);
    QString temp ;
    LOLI::AutoRead(temp,"F:/明日的世界/makelogo.bkscr") ;
    pp.setText(temp);
    w.show();
    return a.exec();
}
