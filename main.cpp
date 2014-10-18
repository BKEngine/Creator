#include <QApplication>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QLibrary>
#include "mainwindow.h"
#include "weh.h"
#include "singleapplication.h"

void CheckOpenAL32() ;

int main(int argc, char *argv[])
{
    QApplication::setApplicationName("BKE_Creator");
    QTextCodec *xcodec = QTextCodec::codecForLocale();
#ifdef QT_DEBUG
    BKE_CURRENT_DIR = QString("F:/work/qt/BKE_creator/file") ;
#endif

#ifndef QT_DEBUG
#ifdef WIN32
    wchar_t tmp[MAX_PATH];
    GetModuleFileNameW(NULL, tmp, MAX_PATH);
    QString exeDir = QString::fromWCharArray(tmp);
#else
    QString exeDir = xcodec->toUnicode( QByteArray(argv[0]) ) ;
#endif
    BKE_CURRENT_DIR = QFileInfo( exeDir ).path() ;
    //qt has a bug in 5.2.1(windows)? so I use setLibraryPaths
   QApplication::addLibraryPath( BKE_CURRENT_DIR) ;
#endif

    SingleApplication a(argc, argv);
    if(a.isRunning())
        return -1;
    QTranslator translator;
    if( !translator.load("qt_zh_CN",BKE_CURRENT_DIR,"",".qm") ) QMessageBox::information(0,"错误","加载中文翻译失败",QMessageBox::Ok) ;
    a.installTranslator(&translator);

//启动------->>>>>>>>>>

    //创建文件夹
    QDir temp(BKE_CURRENT_DIR ) ;
    temp.mkdir("temp") ;

    temp.setPath(BKE_CURRENT_DIR+"/stencil");
    isSYSTEMP_LOWDER = temp.exists() ; //检查Stencil是否存在，确定系统是否区分大小写

    LOLI_CLEAR_TEMP(BKE_CURRENT_DIR+"/temp");  //清空临时文件夹

    // 读取api命令
    if( !LOLI::AutoRead(BKE_API_FILE,BKE_CURRENT_DIR+"/command.api") ){
        QMessageBox::information(0,"初始化","读取API列表失败",QMessageBox::Ok) ;
    }

    //读取最近使用列表
    QString ks ;
    LOLI::AutoRead(ks,BKE_CURRENT_DIR+"/projects.txt") ;
    BKE_Recently_Project = ks.split("\r\n") ;
    LOLI::AutoRead(ks,BKE_CURRENT_DIR+"/files.txt") ;
    BKE_Recently_Files = ks.split("\r\n") ;

    //读取默认方法api列表
    BkeCreator::ReadApiList(&SYSlist,BKE_CURRENT_DIR+"/class.api",8) ;
    BkeCreator::ReadApiList(&KEYlist,BKE_CURRENT_DIR+"/parser.api",9) ;

    //setting
    BKE_CLOSE_SETTING = new QSettings(BKE_CURRENT_DIR+"/setting.ini",QSettings::IniFormat) ;
    BKE_USER_SETTING = new QSettings(BKE_CURRENT_DIR+"/user.ini",QSettings::IniFormat) ;

    //颜色配置项
    QFile sa;
    sa.setFileName(BKE_CURRENT_DIR+"/css.json");
    sa.open(QFile::ReadOnly) ;
    BKE_QCSS_OBJECT = QJsonDocument::fromJson(sa.readAll()).object() ;
    sa.close();
    //<<<<<<<---------------


    MainWindow test ;

    a.setActiveWidget(&test);

    if( BKE_CLOSE_SETTING->value("window/ismax").toBool() ) test.showMaximized();
    else test.show();

    if( argc > 1){
        projectedit->OpenProject(xcodec->toUnicode(QByteArray(argv[1])) );
    }

    QObject::connect(&a,&SingleApplication::newApplication,[=](QString args)
        {if(args.count())
            projectedit->OpenProject(args);}
    );

#ifndef QT_DEBUG
    if( !BKE_CLOSE_SETTING->value("update/close").toBool() ) QTimer::singleShot(3000,&test,SLOT(CheckUpdate()) ) ;
#endif
//#ifdef QT_DEBUG
//    if( !BKE_CLOSE_SETTING->value("update/close").toBool() ) QTimer::singleShot(3000,&test,SLOT(CheckUpdate()) ) ;
//#endif

    //使用win32时，检查依赖库
    #ifdef Q_OS_WIN
    CheckOpenAL32();
    #endif

    return a.exec();
}

//检测是否安装了openal32，没有则安装
void CheckOpenAL32()
{
    QLibrary lib("OpenAL32.dll") ;
    if( lib.load() ) return ;
    QMessageBox::information(0,"安装支持库","你的计算机没有安装OpenAL32，Creator将为你安装，\n在接下来的窗口中选择 OK ") ;
    QDesktopServices::openUrl(QUrl::fromLocalFile(BKE_CURRENT_DIR+"/tool/OpenAL.exe")) ;
}
