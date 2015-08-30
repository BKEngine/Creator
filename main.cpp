#include <weh.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QLibrary>
#include "mainwindow.h"
#include "singleapplication.h"
#include <stdint.h>
#include "quazip/JlCompress.h"

QString title = "BKE Creator - ";
uint32_t titlehash = 0;

uint32_t BKE_hash(const wchar_t *str)
{
	if (!*str)
		return 0;
	const uint32_t _FNV_offset_basis = 2166136261U;
	const uint32_t _FNV_prime = 16777619U;
	const wchar_t *c = str;
	uint32_t ret = _FNV_offset_basis;
	while (*c)
	{
		ret ^= (uint32_t)*c;
        ret *= _FNV_prime;
		c++;
	}
	return ret;
}

#ifdef WIN32
#include <Windows.h>

BOOL CALLBACK EnumWndProc(HWND hwnd, LPARAM lParam)
{
	uint32_t h = (uint32_t)GetProp(hwnd, L"title");
	if (h == titlehash)
	{
		*(HWND*)lParam = hwnd;
		return false;
	}
	return true;
}
#endif

void CheckOpenAL32() ;
void CheckFileAssociation();

int main(int argc, char *argv[])
{
    QApplication::setApplicationName("BKE_Creator");
    QTextCodec *xcodec = QTextCodec::codecForLocale();
#ifdef QT_DEBUG
#ifdef WIN32
    BKE_CURRENT_DIR = QString("F:/work/qt/BKE_creator/file") ;
#endif
#endif

#ifndef QT_DEBUG
#ifdef WIN32
    wchar_t tmp[MAX_PATH];
    GetModuleFileNameW(NULL, tmp, MAX_PATH);
    QString exeDir = QString::fromWCharArray(tmp);
#else
    QString exeDir = xcodec->toUnicode( QByteArray(argv[0]) ) ;
#endif
#ifndef Q_OS_MAC
    BKE_CURRENT_DIR = QFileInfo( exeDir ).path() ;
    //qt has a bug in 5.2.1(windows)? so I use setLibraryPaths
    QApplication::addLibraryPath( BKE_CURRENT_DIR) ;
#else
    {
        QDir d = QFileInfo( exeDir ).dir();
        d.cdUp();
        d.cd("PlugIns");
        QApplication::setLibraryPaths(QStringList() << d.absolutePath());
        qDebug() << QApplication::libraryPaths();
    }
#endif
#endif

   QApplication a(argc, argv);

#ifdef Q_OS_MAC
   BKE_CURRENT_DIR = QDir::homePath() + "/Documents/BKE_Creator";
   QDir dir(BKE_CURRENT_DIR);
   if(!dir.exists())
   {
       QDir d(qApp->applicationDirPath());
       d.cdUp();
       d.cd("Resources");
       if(JlCompress::extractDir(d.filePath("data.compress"), dir.absolutePath()).count()==0)
       {
           QMessageBox::information(0, "Error", "Cannot uncompress the resources. :( \nPlease unzip .app/Resources/data.compress to ~/Documents/BKE_Creator yourself and restart the application.");
           exit(0);
       }
   }
#endif
    //SingleApplication a(argc, argv);

#ifndef WIN32
#else
	if (argc > 1)
		title += argv[1];
	titlehash = BKE_hash(title.toStdWString().c_str());
	HWND oldHWnd = NULL;
	EnumWindows(EnumWndProc, (LPARAM)&oldHWnd);
	if (oldHWnd != NULL)
	{
		ShowWindow(oldHWnd, SW_SHOWNORMAL);
		SetWindowPos(oldHWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		SetWindowPos(oldHWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		return 0;
	}
#endif
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
    BKE_SKIN_SETTING = new QSettings(BKE_CURRENT_DIR+"/skin.ini",QSettings::IniFormat) ;
    BKE_SKIN_CURRENT = BKE_SKIN_SETTING->value("StyleName","默认").toString() ;	//颜色配置项

    //<<<<<<<---------------


    MainWindow test ;

//    a.setActiveWidget(&test);
    a.setActiveWindow(&test);
#ifdef Q_OS_WIN
    if( BKE_CLOSE_SETTING->value("window/ismax").toBool() ) test.showMaximized();
    else test.show();
#else
    test.show();
#endif

    if( argc > 1){
        projectedit->OpenProject(xcodec->toUnicode(QByteArray(argv[1])) );
    }

    //QObject::connect(&a,&SingleApplication::newApplication,[=](QString args)
    //    {if(args.count())
    //        projectedit->OpenProject(args);}
    //);

#ifndef QT_DEBUG
    if( !BKE_CLOSE_SETTING->value("update/close").toBool() ) QTimer::singleShot(3000,&test,SLOT(CheckUpdate()) ) ;
#endif
//#ifdef QT_DEBUG
//    if( !BKE_CLOSE_SETTING->value("update/close").toBool() ) QTimer::singleShot(3000,&test,SLOT(CheckUpdate()) ) ;
//#endif

    //使用win32时，检查依赖库
    #ifdef Q_OS_WIN
    CheckOpenAL32();
    CheckFileAssociation();
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

void doFileAssociation()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(BKE_CURRENT_DIR+"/FileAssociation.exe")) ;
}

void CheckFileAssociation()
{
    QSettings *ukenvFileReg = new QSettings("HKEY_CLASSES_ROOT\\.bkp", QSettings::NativeFormat);   //

    //判断UKEnv类型是否已在注册表中，并关联了正确的打开方式（程序打开方式），没有则写入
    QString currentValue = ukenvFileReg->value("Default").toString();

    if (currentValue.isEmpty() ||
      currentValue != "BKE_Creator")
    {
        if(QMessageBox::question(0,"提示","检测到工程文件尚未关联。是否关联工程文件？")==QMessageBox::Yes)
        {
            doFileAssociation();
        }
    }
}
