#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QStringList>
//csv转maro.api

void startx() ;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    startx();
    return a.exec();
}


void startx()
{
    QString result ;
    QString root ;
    QString child ;
    QStringList list ;

    QFile file("f:/API.csv") ;
    file.open(QFile::ReadOnly) ;
    QTextStream tk(&file) ;

    QStringList pp ;
    QString line ;
    while(!tk.atEnd()){
        pp = tk.readLine().split(",") ;

        if( !pp.at(0).isEmpty() ){
            if( !list.isEmpty() ){
                QString sss = "@macro name=\""+root+"\" " ;
                for(int i = 0 ; i < list.size() ; i++){
                    sss.append(list.at(i)+"# ") ;
                }
                result.append(sss+"\r\n") ;
            }
            root = pp.at(0) ;
            child = pp.at(1) ;
            if( child.isEmpty() ) list.clear();
        }
        if( !pp.at(1).isEmpty() ){
            child = pp.at(1) ;
            list.append(child);
        }

        if( !pp.at(0).isEmpty() || !pp.at(1).isEmpty()){ //子命令
            result.append(line+"\r\n") ;
            line = "@macro name=\"" ;
            if( child.isEmpty() ) line.append(root+"\" ") ;
            else  line.append(root+"#"+child+"\" ") ;
        }

        if( !pp.at(2).isEmpty() ){
            if( pp.at(2).startsWith("*")){
                line.append( pp.at(2).right(pp.at(2).length()-1 )+" ") ;
            }
            else line.append(pp.at(2)+"= ") ;
        }

    }

    file.close();
    file.setFileName("f:/out.txt");
    file.open(QFile::ReadWrite) ;
    tk << result ;
    file.flush() ;
    file.close();
}
