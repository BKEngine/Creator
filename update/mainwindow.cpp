#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    isSelfUp = false ;
    isOK = false ;
    isDown = false ;
    isBkeUp = false;
    state = STATE_NULL ;
    ui->setupUi(this);
    ui->textEdit->setText("等待连接...");
    ReadVertion();

    netAdmin = new QNetworkAccessManager(this) ;
    autocheck = new QTimer(this) ;
    connect(autocheck,SIGNAL(timeout()),this,SLOT(StartX())) ;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ReadVertion()
{
    QFile file(CURRENT_DIR + "/version.txt") ;
    if( !file.open(QFile::ReadWrite) ) return ;

    QJsonDocument dc = QJsonDocument::fromJson(file.readAll()) ;
    if( !dc.isNull() ){
        verJSON = dc.object() ;
        ofilejs = verJSON.value("files").toObject() ;
    }
    file.close();
}



void MainWindow::StartX()
{
    if( currentREP->isFinished() ){
        downFinish(currentREP);
        Nextfile();
    }
}

//开始下载更新信息
void MainWindow::DownInfo()
{
    currentREP = netAdmin->get( QNetworkRequest(QUrl(HTTP_ADRESS+"bkecreator")) ) ;
    connect(netAdmin,SIGNAL(finished(QNetworkReply*)),this,SLOT(InfodownFinish(QNetworkReply*))) ;
}



void MainWindow::InfodownFinish(QNetworkReply* netfile)
{
    QJsonDocument dc = QJsonDocument::fromJson(netfile->readAll()) ;
    if( !dc.isNull() ){
        newJSON = dc.object() ;
        nfilejs = newJSON.value("files").toObject() ;
        QString tt ;
        tt.append( "版本:"+newJSON.value("version").toString()+"\r\n" ) ;
        tt.append("更新内容：\r\n"+newJSON.value("info").toString()+"\r\n"  ) ;
        ui->textEdit->setText(tt);
    }
    else{
        ui->textEdit->setText("从服务器下载了一个错误的更新文件，请向我们反馈...");
        netfile->deleteLater();
        return ;
    }

    QApplication::processEvents() ;

    //断开信号和槽
    disconnect(netAdmin,SIGNAL(finished(QNetworkReply*)),this,SLOT(InfodownFinish(QNetworkReply*))) ;

    QString fileName;

#if defined(Q_OS_WIN)
    fileName = "update.exe";
#elif defined(Q_OS_LINUX)
    fileName = "update";
#elif defined(Q_OS_MAC)
    fileName = "update";
#endif


    //下载初始化
    if( !dc.isNull() ){

        downlist.clear();
        cmdString.clear();
        if( isFileDiffrent(fileName) )
        {    //优先升级update.exe
#ifdef Q_OS_MAC

#else
            isSelfUp = true ;
            downlist << fileName ;
#endif
        }
        else
            downlist = nfilejs.keys() ;
    }

    netfile->deleteLater();
    startUpdate();
    autocheck->start(1500);
}


//一个个文件下载
void MainWindow::startUpdate()
{
    if( downlist.size() < 1){
        autocheck->stop();
        ui->textEdit->append("没有更新任何数据！");
        return ;
    }

    ui->filesBar->setMaximum(downlist.size());
    //connect(netAdmin,SIGNAL(finished(QNetworkReply*)),this,SLOT(downFinish(QNetworkReply*))) ;

    errorlist.clear();
    ptr = 0 ;

    {
        auto keys = ofilejs.keys();
        auto i = 0;
        while(i < keys.size())
        {
            if(isFileRemoved(keys.at(i)))
            {
#if defined(Q_OS_WIN)
                cmdString.append("del /Q \""+ keys.at(i) +"\"\r\n" ) ;
#elif defined(Q_OS_LINUX)
                cmdString.append("rm -f \""+ keys.at(i) +"\"\n" ) ;
#endif
                ofilejs.remove(keys.at(i));
            }
            i++;
        }
    }

    Nextfile();
}

void MainWindow::Nextfile()
{

    //只更新md5值不同的文件
    while( ptr < downlist.size() && !isFileDiffrent(downlist.at(ptr)) ) ptr++ ;

    //已经历遍文件，结束循环
    if( ptr >= downlist.size() ){
        simpleOK();
        return ;
    }

    QString fileName;

#if defined(Q_OS_WIN)
    fileName = "tool/bkengine_dev.exe";
#elif defined(Q_OS_LINUX)
    fileName = "tool/BKEngine_Dev";
#elif defined(Q_OS_MAC)
#endif


    //绑定新文件
    currentfile = downlist.at(ptr) ;
    if(currentfile.toLower() == fileName)
        isBkeUp = true;
    state = STATE_NEXT ;
    ptr++ ;

    ui->filesBar->setValue(ui->filesBar->value()+1);

    ui->textEdit->append( currentfile+" 准备更新...\n");
    currentREP = netAdmin->get(QNetworkRequest(QUrl( HTTP_ADRESS+"bke_creator/"+currentfile))) ;
    connect(currentREP,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(updateDataReadProgress(qint64,qint64))) ;
    ui->filepreBar->setValue(ptr);
    //指向下一个文件
    return ;
}

void MainWindow::downFinish(QNetworkReply* netfile)
{
    QByteArray dest ;
    QCryptographicHash hashmk(QCryptographicHash::Md5) ;
    QDir outd ;
    QString dmd5 ;

    if( netfile == 0 ){
        addError("连接失败...");
        return ;
    }

    dest = qUncompress( netfile->readAll() ) ;

    hashmk.addData(dest);
    dmd5 = hashmk.result().toHex() ;

    if( dmd5 != nfilejs.value(currentfile).toString() ) addError("md5校验失败...");
    else{
#if defined(Q_OS_WIN)
        QFileInfo oinfo( CURRENT_DIR+"/"+currentfile+".new" ) ;
#elif defined(Q_OS_LINUX)
        QFileInfo oinfo( CURRENT_DIR+"/"+currentfile+".new" ) ;
#elif defined(Q_OS_MAC)
        QFileInfo oinfo( CURRENT_DIR+"/"+currentfile ) ;
#endif
        outd.mkpath( oinfo.path() ) ;
        if( !LOLI::AutoWrite( oinfo.filePath(),dest ) ){
            addError("写出失败");
        }
        else{
            QString ename = currentfile ;
#if defined(Q_OS_WIN)
            ename.replace(QRegExp("/"),"\\" ) ;
            cmdString.append("copy /Y \""+ ename+".new\" \""+ ename +"\"\r\n" ) ;
#elif defined(Q_OS_LINUX)
            ename.replace(QRegExp("\\"),"/" ) ;
            cmdString.append("cp -f \""+ ename+".new\" \""+ ename +"\"\r\n"  ) ;
#endif

            ofilejs.insert(currentfile,dmd5) ;  //同时更新md5的值
            ui->textEdit->append("完成");
        }
    }

    ui->textEdit->moveCursor(QTextCursor::End);
    //QApplication::processEvents() ;

    state = STATE_NEXT ;
    if( netfile != 0) netfile->deleteLater();

}

void MainWindow::updateDataReadProgress(qint64 from,qint64 to)
{
    ui->filepreBar->setMaximum(to);
    ui->filepreBar->setValue(from);
}


//打包程序
void MainWindow::MakeMD5()
{
    MD5FILE.clear();
    simpleMD5(MAKEDIR);

    QJsonDocument dc(filesjson) ;
    LOLI::AutoWrite(CURRENT_DIR+ "/out.txt", dc.toJson() ) ;
    ui->textEdit->append("处理完成");
}


void MainWindow::simpleMD5(QString dir)
{
    QDir eas(dir) ;
    if( !eas.exists() ) return ;

    QFileInfoList ls = eas.entryInfoList() ;
    QFileInfo info ;
    QByteArray dest ;
    QDir et ;
    QCryptographicHash hashmk(QCryptographicHash::Md5) ;

    for( int i = 0 ; i < ls.size() ; i++){
        info = ls.at(i) ;
        if( info.fileName() == "." || info.fileName() == ".." ) continue ;
        else if( info.isDir() ) simpleMD5(dir+"/"+info.fileName());
        else if( LOLI::AutoRead(dest,dir+"/"+info.fileName()) ){
            hashmk.reset();
            hashmk.addData(dest);
            QString okh = info.filePath().right(info.filePath().length()-MAKEDIR.length()-1) ;
            QString emd = QString( hashmk.result().toHex() ) ;
            filesjson.insert(okh,emd) ;
            ui->textEdit->append(okh+" : "+ emd );

            dest = qCompress(dest) ;
            QFileInfo oinfo(CURRENT_DIR+"/out/"+okh) ;
            et.mkpath(oinfo.path()) ;

            LOLI::AutoWrite(oinfo.filePath(),dest) ;
        }
    }
}

void MainWindow::simpleOK()
{
    if( errorlist.size() < 1 ){
        autocheck->stop();
        ui->filesBar->setValue( ui->filesBar->maximum() );
        ui->textEdit->append("完成！请等待脚本运行...");
#if defined(Q_OS_WIN)
        if( !cmdString.isEmpty() ){
            cmdString.prepend("@echo off\r\necho 准备覆盖....\r\nping -n 2 127.1>nul\r\necho 【 3 】\r\nping -n 2 127.1>nul\r\n"
                              "echo 【 2 】\r\n"
                              "ping -n 2 127.1>nul\r\n"
                              "echo 【 1 】\r\n" ) ;
            cmdString.append("del /f/s/q *.new\r\n") ;
        }
        cmdString.prepend("@echo off\r\n"
                          "for %%i in (%0) do set aa=%%~dpi \r\n"
                          "cd /d %aa%\r\n\r\n");

        //如果升级程序需要更新，那么升级自身
        if( isSelfUp ) cmdString.append("start update.exe\r\n") ;
        //否则启动主程序
        else
        {
            cmdString.append("start BKE_creator.exe\r\n") ;
            if(isBkeUp)
                cmdString.append("start updatelog.txt");
        }


        QFile temp( CURRENT_DIR+"/up.bat" ) ;
        temp.remove();
        LOLI::AutoWrite(&temp,cmdString,"GBK" ) ;
        temp.close();

        QDesktopServices::openUrl(QUrl::fromLocalFile( CURRENT_DIR+"/up.bat" )) ;
#elif defined(Q_OS_LINUX)
//        QDesktopServices::openUrl(QUrl::fromLocalFile( CURRENT_DIR+"/BKE_creator" )) ;
        QMessageBox::information(this,"在线更新","更新成功！请重启BKE_Creator！");
#elif defined(Q_OS_MAC)
#endif
        newJSON.insert("files",ofilejs) ; //替换新的文件列表为旧的，成功的文件md5被改变
        QJsonDocument dc(newJSON) ;
        LOLI::AutoWrite(CURRENT_DIR + "/version.txt",dc.toJson()) ;
        close() ;
    }

    else{
        downlist = errorlist ;
        state = STATE_STARTUP ;
        QObject *otk = new QObject ;
        connect(otk,SIGNAL(destroyed()),this,SLOT(startUpdate()),Qt::QueuedConnection) ;
        otk->deleteLater();
    }
}

//文件md5码，是否相同
bool MainWindow::isFileDiffrent(const QString &name)
{
    QCryptographicHash hashmk(QCryptographicHash::Md5) ;
    QFile f(CURRENT_DIR + "/" + name);
    hashmk.addData(&f);

    QString newmd5 = nfilejs.value(name).toString() ;
    return hashmk.result().toHex() != newmd5 ;
}

bool MainWindow::isFileRemoved(const QString &name)
{
    return !nfilejs.contains(name);
}

//添加错误文件
void MainWindow::addError(const QString &info)
{
    if( errorlist.indexOf( currentfile ) < 0) errorlist.append(currentfile);
    ui->textEdit->append(currentfile+":"+info);
}
