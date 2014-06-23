#include <QCoreApplication>
#include <QTextStream>
#include <QObject>
#include "parser.h"
#include "debugout.h"

QString ReadFile(const QString &file) ;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    DebugOut dout ;
    BkeParser pa ;

    dout.SetX(&pa,new QTextStream(stdout, QIODevice::WriteOnly));

    pa.SetDir("F:/Game");
    pa.ParserText(ReadFile("F:/Game/text.txt"));

    return a.exec();
}


QString ReadFile(const QString &file)
{
    QFile temp(file) ;
    temp.open(QFile::ReadOnly) ;
    QTextStream tk(&temp) ;
    tk.setCodec(QTextCodec::codecForName("utf8"));
    QString ss = tk.readAll() ;
    temp.close();
    return ss ;
}
