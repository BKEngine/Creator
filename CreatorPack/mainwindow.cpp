#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <quazip/quazip.h>
#include <quazip/JlCompress.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ftpClient = NULL;
    ReadVersion(CURRENT_DIR);
    if(!verJSON.empty())
    {
        ui->lineEdit->setText(CURRENT_DIR);
        ui->lineEdit_2->setText(verJSON.value("version").toString());
        ui->lineEdit_3->setText(verJSON.value("info").toString());
    }
    speedTimer = new QTimer(this);
    connect(speedTimer,SIGNAL(timeout()),this,SLOT(speedTimer_timeout()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete ftpClient;
}

void MainWindow::ReadVersion(QString pdir)
{
    verJSON = QJsonObject();
    QFile file(pdir + "/version.txt") ;
    if( !file.open(QFile::ReadWrite) ) return ;

    QJsonDocument dc = QJsonDocument::fromJson(file.readAll()) ;
    if( !dc.isNull() ){
        verJSON = dc.object() ;
        ofilejs = verJSON.value("files").toObject() ;
    }
    file.close();
}

void MainWindow::WriteVersion(const QByteArray &a)
{
    QString pdir = ui->lineEdit->text();
    QFile file(pdir + "/version.txt") ;
    if( !file.open(QFile::WriteOnly) ) return ;
    file.write(a);
    file.close();
}


//文件md5码，是否相同
bool MainWindow::isFileDiffrent(const QString &name)
{
//    QString oldmd5 = ofilejs.value(name).toString() ;
//    QString newmd5 = nfilejs.value(name).toString() ;
//    return oldmd5 != newmd5 ;
    return false;
}


void MainWindow::on_pushButton_2_clicked()
{
    QString pdir = QFileDialog::getExistingDirectory(this,"选择压缩路径",CURRENT_DIR) ;
    if(pdir.isEmpty())
        return;
    ReadVersion(pdir);
    if(!verJSON.empty())
    {
        ui->lineEdit->setText(pdir);
        ui->lineEdit_2->setText(verJSON.value("version").toString());
        ui->lineEdit_3->setText(verJSON.value("info").toString());
    }
    else
    {
        QMessageBox::warning(this,"警告","选择了一个非bke creator目录。");
    }
}

static bool copyData(QIODevice &inFile, QIODevice &outFile)
{
    while (!inFile.atEnd()) {
        char buf[4096];
        qint64 readLen = inFile.read(buf, 4096);
        if (readLen <= 0)
            return false;
        if (outFile.write(buf, readLen) != readLen)
            return false;
    }
    return true;
}

class MyBuffer : public QBuffer
{
public:
    QString name;
    MyBuffer(const QString &name, QObject *p = NULL):QBuffer(p),name(name){}
    MyBuffer(const QByteArray &a, const QString &name, QObject *p = NULL):QBuffer(p),name(name){setData(a);}
};

bool compressFile(QIODevice &inFile, const QString &file, const QString &name, QuaZip &archive)
{
    QuaZipFile outFile(&archive);
    if(!outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(name, file))) return false;

    // Copio i dati
    if (!copyData(inFile, outFile) || outFile.getZipError()!=UNZ_OK) {
         return false;
    }

    // Chiudo i file

    outFile.close();
    if (outFile.getZipError()!=UNZ_OK) return false;
    return true;
}

bool compressFile(const QString &file, const QString &name, QuaZip &archive)
{
    QFile inFile;
    inFile.setFileName(file);
    if(!inFile.open(QIODevice::ReadOnly)) return false;

    if(!compressFile(inFile, file, name, archive)) return false;
    // Apro il file risulato
    inFile.close();
    return true;
}

void MainWindow::on_pushButton_clicked()
{
    QString pdir = ui->lineEdit->text() ;
    QString info = ui->lineEdit_3->text() ;
    vertxt = ui->lineEdit_2->text() ;

    if( pdir.isEmpty() ){
        QMessageBox::information(this,"错误","必须使用目录！",QMessageBox::Ok) ;
        return ;
    }

    if( vertxt.isEmpty() ){
        QMessageBox::information(this,"错误","请写版本号!",QMessageBox::Ok) ;
        return ;
    }
    if( info.isEmpty() ){
        QMessageBox::information(this,"错误","输入更新信息！",QMessageBox::Ok) ;
        return ;
    }

    //if( !vertxt.endsWith("\t") ) vertxt.append("\t") ;
    MyBuffer *bkecreator = new MyBuffer("bkecreator", this);
    this->bkecreator = MakeMD5(pdir,"",info);
    bkecreator->setData(this->bkecreator);
    QBuffer zipBuffer;
    //QFile zipBuffer("bke_creator.zip");
    QuaZip archive(&zipBuffer);
    archive.open(QuaZip::mdCreate);
    for(auto i = filesjson.begin(); i!=filesjson.end(); i++)
    {
        QString v1 = i.value().toString();
        QString v2 = ofilejs[i.key()].toString();
        if(v1!=v2)
        {
            QByteArray a;
            LOLI::AutoRead(a,pdir + "/" +i.key());
            a = qCompress(a);
            diffs[i.key()] = a;
        }
        if(!compressFile(pdir + "/" +i.key(), i.key(), archive)) continue;
    }
    //version.txt
    {
        bkecreator->open(QIODevice::ReadOnly);
        compressFile(*bkecreator, pdir + "/version.txt", "version.txt", archive);
        bkecreator->close();
    }
    archive.close();
    QFile f("bke_creator.zip");
    f.open(QFile::WriteOnly);
    f.write(zipBuffer.buffer());
    f.close();
    if(!ftpClient)
    {
        ftpClient = new QFtp(this);
        connect(ftpClient,SIGNAL(commandFinished(int,bool)),this,SLOT(cmdfinish(int,bool)));
        connect(ftpClient,SIGNAL(commandStarted(int)),this,SLOT(cmdstart(int)));
        connect(ftpClient,SIGNAL(dataTransferProgress(qint64,qint64)),this,SLOT(progress(qint64,qint64)));
    }
    else
    {
        ftpClient->close();
    }
    ui->progressBar->setValue(0);
    ftpClient->connectToHost("icemaple.info", 21);
    ftpClient->login("ftpuser","ojjlolidaisuki123");
    ftpClient->cd("bkengine");
    ftpClient->cd("update");
    ftpClient->cd("win");
    ftpClient->put(bkecreator,"bkecreator");
    ftpClient->cd("bke_creator");
    //QString npath;
    //int nlayer=0;
    for(auto it = diffs.begin(); it != diffs.end(); it++)
    {
        MyBuffer *b = new MyBuffer(it.key(),this);
        b->setData(it.value());
        ftpClient->put(b,it.key());
    }
    ftpClient->cd("..");
    ftpClient->cd("..");
    ftpClient->cd("..");
    ftpClient->cd("download");
	ftpClient->cd("BKECreator");
    ftpClient->put(new MyBuffer(zipBuffer.buffer(), "bke_creator.zip", this),"bke_creator.zip.new");
    ftpClient->remove("BKECreator.zip");
    ftpClient->rename("bke_creator.zip.new","BKECreator.zip");
    ftpClient->close();
}


static QString getSizeString(size_t s)
{
    char t1[8];
    char t2[8];
    size_t k = s / 1024;
    size_t m = k / 1024;
    if(!m)
    {
        itoa(k, t1, 10);
        return QString(t1) + "KB";
    }
    k = (k - m * 1024) * 1000 / 1024;
    itoa(k, t1, 10);
    itoa(m, t2, 10);
    return QString(t2) + "." + QString(t1) + "MB";
}

void MainWindow::cmdstart(int)
{
    switch(ftpClient->currentCommand())
    {
    case QFtp::Put:
    {
        QIODevice *_dev = ftpClient->currentDevice();
        if(!_dev) break;
        MyBuffer *dev = dynamic_cast<MyBuffer *>(_dev);
        if(!dev) break;
        ui->textEdit->append("当前文件：" + dev->name + " 大小：" + getSizeString(dev->size()));
        startSpeedTimer();
    }
    }
}

void MainWindow::cmdfinish(int , bool error)
{
    switch(ftpClient->currentCommand())
    {
    case QFtp::ConnectToHost:
        if(error)
        {
            ui->textEdit->append("无法连接主机。");
            ui->textEdit->append(ftpClient->errorString());
        }
        else
            ui->textEdit->append("主机连接成功。");
        break;
    case QFtp::Login:
        if(error)
        {
            ui->textEdit->append("登陆失败。");
            ui->textEdit->append(ftpClient->errorString());
        }
        else
            ui->textEdit->append("登陆成功。");
        break;
    case QFtp::Put:
    {
        //QString s = ((QFile *)ftpClient->currentDevice())->fileName();
        //s = s.endsWith("bkecreator") ? s.right(s.size() - CURRENT_DIR.size() - 1) : s.right(s.size() - CURRENT_DIR.size() - 5);
        if(error)
        {
            ui->textEdit->append("上传失败。");
            ui->textEdit->append(ftpClient->errorString());
        }
        else{
            ui->textEdit->append("上传成功。");
            ui->progressBar->setValue(0);
            stopSpeedTimer();
            QIODevice *_dev = ftpClient->currentDevice();
            if(!_dev) break;
            MyBuffer *dev = dynamic_cast<MyBuffer *>(_dev);
            if(!dev) break;
            if(!filesjson.contains(dev->name)) break;
            ofilejs[dev->name] = filesjson[dev->name];
            affineJson();
            WriteVersion(versionjson);
        }
        break;
    }
    case QFtp::Close:
    {
        WriteVersion(this->bkecreator);
        QMessageBox::information(this,"Packer","处理完成。");
        break;
    }
    default:
        break;
    }
}

void MainWindow::progress(qint64 n, qint64 m)
{
    bytesDownload +=  n - ui->progressBar->value();
    ui->progressBar->setMaximum(m);
    ui->progressBar->setValue(n);
}

QByteArray MainWindow::MakeMD5(QString &dir,QString rdir,QString &info)
{
    MD5FILE.clear();
    filesjson = QJsonObject() ;

    QSet<QString> exceptions = {
        dir + "/version.txt",
        dir + "/setting.ini",
		dir + "/user.ini",
        dir + "/files.txt",
        dir + "/projects.txt",
        dir + "/up.bat",
        dir + "/update_pack.exe",
        dir + "/temp/",
        "mht",
        "html",
        "rar",
        "zip",
        "new"
    };
    simpleMD5(dir, rdir, exceptions);

    QJsonObject verjs ;
    verjs.insert("files",filesjson) ;
    verjs.insert("version",vertxt) ;
    verjs.insert("info",info) ;
    QJsonDocument dc(verjs) ;
    return dc.toJson();
}

void MainWindow::affineJson()
{
    QJsonObject verjs ;
    verjs.insert("files", ofilejs) ;
    verjs.insert("version",vertxt) ;
    verjs.insert("info", ui->lineEdit_3->text()) ;
    QJsonDocument dc(verjs) ;
    versionjson = dc.toJson();
}


void MainWindow::simpleMD5(QString dir, QString rdir, const QSet<QString> exceptions)
{

    QDir eas(dir) ;
    if( !eas.exists() ) return ;

    QFileInfoList ls = eas.entryInfoList() ;
    QFileInfo info ;
    QByteArray dest ;
    QDir et ;
    QCryptographicHash hashmk(QCryptographicHash::Md5) ;
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(ls.size());
    QApplication::processEvents();

    for( int i = 0 ; i < ls.size() ; i++){
        info = ls.at(i) ;
        if( info.fileName() == "." || info.fileName() == ".." ) continue ;
        else if( info.isDir() && !exceptions.contains(dir+"/"+info.fileName()+"/")) simpleMD5(dir+"/"+info.fileName(),rdir.isEmpty()?info.fileName():rdir+"/"+info.fileName(),exceptions);
        else if( !exceptions.contains(dir+"/"+info.fileName()) && !exceptions.contains(info.suffix()) && LOLI::AutoRead(dest,dir+"/"+info.fileName()) ){
            hashmk.reset();
            hashmk.addData(dest);
            QString okh = rdir.isEmpty()?info.fileName():rdir+"/"+info.fileName() ;
            QString emd = QString( hashmk.result().toHex() ) ;
            filesjson.insert(okh,emd) ;
            ui->textEdit->append(okh+" : "+ emd );

            //dest = qCompress(dest) ;
            //QFileInfo oinfo(CURRENT_DIR+"/out/"+okh) ;
            //et.mkpath(oinfo.path()) ;

            //LOLI::AutoWrite(oinfo.filePath(),dest) ;
            ui->progressBar->setValue(i+1);
            QApplication::processEvents();
        }
    }
}

void MainWindow::stopSpeedTimer()
{
    bytesDownload = 0;
    speedTimer->stop();
    ui->label_3->setText("0KB/S");
}

void MainWindow::startSpeedTimer()
{
    bytesDownload = 0;
    speedTimer->start(500);
}

void MainWindow::speedTimer_timeout()
{
    ui->label_3->setText(getSizeString(bytesDownload * 2) + "/S");
    bytesDownload = 0;
}
