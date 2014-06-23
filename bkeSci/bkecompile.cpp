#include "bkecompile.h"
#include "weh.h"

BkeCompile::BkeCompile(QObject *parent) :
    QObject(parent)
{
    codec = QTextCodec::codecForLocale();
}

void BkeCompile::Compile(const QString dir)
{
    result.clear();
    cmd = new QProcess(this) ;
    connect(cmd,SIGNAL(readyReadStandardOutput()),this,SLOT(StandardOutput())) ;
    connect(cmd,SIGNAL(finished(int)),this,SLOT(finished(int))) ;
    list.clear();
    cmd->start(BKE_CURRENT_DIR+"/tool/BKCompiler_Dev.exe",QStringList() << dir << "-nopause");
}

void BkeCompile::StandardOutput()
{
    QByteArray temp = cmd->readAll() ;
    result.append(temp) ;
    QString name = codec->toUnicode(temp) ;
    if( name.endsWith(".bkscr") && list.indexOf(name) < 0 ){
        list.append(name);
        emit NewFileReady(list.size());
    }
}

void BkeCompile::finished(int exitCode )
{
    emit CompliteFinish();
    delete cmd ;
    cmd = 0 ;
}


QString BkeCompile::Result()
{
    text.clear();
    text = codec->toUnicode(result) ;
    return text ;
}
