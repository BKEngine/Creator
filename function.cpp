#include <weh.h>
#include "function.h"
#include "bkeproject.h"

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ShlObj.h>
#include <propvarutil.h>  
#include <propkey.h>  
#include <combaseapi.h>
#endif

QString BKE_CURRENT_DIR ;
QString BKE_PROJECT_NAME("BkeProject.bkp") ;
QString BKE_USE_NAME("BkeUse.bkpuse") ;
QString BKE_PROJECT_WORKPRO ;
//QString BKE_API_FILE ;
QString BKE_PROJECT_DIR ;
QString BKE_CREATOR_VERTION("20160707\t") ;
QStringList BKE_Recently_Project ;
//QStringList BKE_Recently_Files ;
QJsonObject BKE_MARKS_OBJECT ;
QSettings *BKE_CLOSE_SETTING ;
QSettings *BKE_USER_SETTING ;
QSettings *BKE_SKIN_SETTING ;
QString BKE_SKIN_CURRENT ;

//QJsonObject BKE_QCSS_OBJECT ;
bool isSYSTEMP_LOWDER = true ;  //系统是否忽略大小写，默认为真

QString LOLI_MID(const QString &text,const QString from , const QString to,int pos )
{
    int pos1,pos2 ;
    pos1 = text.indexOf(from,pos) ;
    pos2 = text.indexOf(to,pos1+1) ;
    if( pos1 < 0 || pos2 < 0 ) return QString() ;
    return text.mid(pos1+1,pos2-pos1-1) ;
}

QString LOLI_OS_QSTRING(const QString &text)
{
    if( isSYSTEMP_LOWDER ) return text.toLower() ;
    else return text ;
}

//将优先比对模版库中的文件，如果模版库中存在相同文件名的文件则进行拷贝，否则创建空白文件
bool LOLI_MAKE_NULL_FILE(const QString &filename,const QString &stencilname)
{
    QFileInfo temp(filename) ;
    if( temp.exists() )
		return true ;

	QFile t1(BKE_CURRENT_DIR + "/Stencil/" + (stencilname.isEmpty() ? temp.fileName() : stencilname));
    if( t1.exists() )
		return t1.copy(filename) ;
    else
	{
        QDir().mkpath(temp.path()) ;
        QFile abc( filename) ;
        if( !abc.open(QFile::ReadWrite))
			return false ;
        abc.close();
    }
    return true ;
}

void    LOLI_CLEAR_TEMP(const QString &dir,const QString &sfix)
{
    QDir temp(dir) ;
    QFileInfoList list = temp.entryInfoList() ;
    QFileInfo info ;

    QFile abc ;
    for( int i = 0 ; i < list.size() ; i++){
        info = list.at(i) ;
        if( info.fileName() == "." || info.fileName() == "..") continue ;
        else if( info.isDir()){
            LOLI_CLEAR_TEMP(info.filePath(),sfix);
            QDir k( info.path()) ;
            k.rmdir(info.fileName()) ;
        }
        else if( !sfix.isEmpty() && !info.fileName().endsWith(sfix) ) continue ;
        abc.setFileName(info.filePath());
        abc.remove() ;
    }
}


//按大小写顺序插入序列，返回插入的位置
int     LOLI_SORT_INSERT(QStringList &list,const QString &s)
{
    QString s1,s2;
    int i = 0 ;
    for(  ; i < list.size() ; i++){
        s1 = s2 ;
        s2 = list.at(i) ;
        if( QString::compare(s,s1) > 0 && QString::compare(s,s2) < 0){
            list.insert(i,s);
            return i ;
        }
    }
    list.append(s);
    return i ;
}


QString LOLI_AUTONEXT_QSTRING(QString text,int len )
{
    int a = text.length()/len ;
    for( int i = 1 ; i < a ; i++){
        text.insert(i*len+i-1,"\n") ;
    }
    return text ;
}


//拷贝文件夹：
bool copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist,const QString &sfix)
{
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    if(!targetDir.exists()){    //< 如果目标目录不存在，则进行创建
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList){
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if(fileInfo.isDir()){    //< 当为目录时，递归的进行copy
            if(!copyDirectoryFiles(fileInfo.filePath(),
                targetDir.filePath(fileInfo.fileName()),
                coverFileIfExist,sfix))
                return false;
        }
        else if( !fileInfo.fileName().endsWith(sfix) ) continue ;
        else{            // 当允许覆盖操作时，将旧文件进行删除操作
            if(coverFileIfExist && targetDir.exists(fileInfo.fileName())){
                targetDir.remove(fileInfo.fileName());
            }

            // 进行文件copy
            if(!QFile::copy(fileInfo.filePath(),
                targetDir.filePath(fileInfo.fileName()))){
                    return false;
            }
        }
    }
    return true;
}



//根据给定的文件夹路径，使得绝对路径变为相对路径，如果不是以给定的文件夹开头，返回空
QString BkeFullnameToName(const QString &fullname,const QString &dir)
{
    QString name = LOLI_OS_QSTRING( fullname ).replace(QRegExp("\\"),"/") ; //把 \ 替换为 /
    QString ed = LOLI_OS_QSTRING( dir ).replace(QRegExp("\\"),"/") ;
     if( !name.startsWith( ed ) ) return QString();

     name = fullname.right( name.length() - ed.length() ) ;
     while( name.startsWith("/") || name.startsWith("\\")) name = name.right(name.length()-1) ;  //开头不带/
     return name ;
}

bool LOLI_simple_copy(const QString fromname,const QString &toname)
{
    QFile file( fromname ) ;
    return file.copy(toname) ;
}

//同个给定的全路径列表，起始的文件路径，新的文件夹进行复制，返回复制失败的文件列表
QStringList ListDirsCopy(QStringList &list,const QString &dir,const QString &newdir)
{
    QFile file ;
    QFileInfo info ;
    QDir k ;
    QStringList ls ;
    QString temp ;
    for( int i = 0 ; i < list.size() ; i++){
        temp = list.at(i);

        temp.prepend( newdir ) ;
        //新建路径
        info.setFile(temp);
        k.mkpath( info.path() ) ;
        file.setFileName( dir + list.at(i) );

        //拷贝
        if( !file.copy(temp) ) ls.append( list.at(i) );
    }

    return ls ;
}

#ifdef Q_OS_WIN
static IShellLink* CreateShortCut(const QString &proj)
{
	QString infotip = "打开工程：" + proj;
	HRESULT hr;
	IShellLink *pLink = NULL;
	QJsonObject doc = BkeProject::LoadProject(proj);
	if (doc.isEmpty())
		return NULL;
	hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pLink));
	if (SUCCEEDED(hr))
	{
		wchar_t exe[MAX_PATH];
		GetModuleFileNameW(NULL, exe, MAX_PATH);
		pLink->SetPath(exe);
		QString desc = proj;
		desc.replace('/', '\\');
		pLink->SetDescription(desc.toStdWString().c_str());
		pLink->SetArguments(("\"" + desc + "\"").toStdWString().c_str());
		pLink->SetShowCmd(SW_SHOW);
		// Explicitly setting icon file,
		// without this icons are not shown at least in Windows 7 jumplist
		pLink->SetIconLocation(exe, 0);

		// this is necessary for Windows 7 taskbar jump list links
		IPropertyStore * PropertyStore;
		if (SUCCEEDED(pLink->QueryInterface(IID_IPropertyStore, (void**)&PropertyStore)))
		{
			std::wstring name = doc.value("name").toString().toStdWString();
			PROPVARIANT Prop;
			Prop.vt = VT_LPWSTR;
			Prop.pwszVal = (wchar_t *)name.c_str();
			PropertyStore->SetValue(PKEY_Title, Prop);
			PropertyStore->Commit();
			PropertyStore->Release();
		}
	}
	return pLink;
}


static void WinUpdateRecentList()
{
	HRESULT hr;
	//创建List  
	ICustomDestinationList *pCDL = NULL;
	hr = CoCreateInstance(CLSID_DestinationList, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pCDL));
	if (SUCCEEDED(hr))
	{
		//BeginList  
		UINT uMaxSlots;
		IObjectArray *pOARemoved = NULL;
		hr = pCDL->BeginList(&uMaxSlots, IID_PPV_ARGS(&pOARemoved));
		if (SUCCEEDED(hr))
		{
			QStringList removed;
			unsigned int removedCount;
			if (FAILED(pOARemoved->GetCount(&removedCount)))
			{
				removedCount = 0;
			}

			for (unsigned int i = 0; i < removedCount; i++)
			{
				IShellLink * link;
				wchar_t desc[2048];
				if (SUCCEEDED(pOARemoved->GetAt(i, IID_IShellLink, (void**)&link)) &&
					SUCCEEDED(link->GetDescription(desc, sizeof(desc) - 1)))
				{
					BKE_Recently_Project.removeAll(QString::fromWCharArray(desc).replace('\\','/'));
				}
			}

			//ObjectCollection  
			IObjectCollection *pOC = NULL;
			hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL,
				CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pOC));
			if (SUCCEEDED(hr))
			{
				//每个文件分别创建ShellItem  
				for (auto && p : BKE_Recently_Project)
				{
					IShellLink *pSI = NULL;
					pSI = CreateShortCut(p);
					if (pSI)
					{
						pOC->AddObject(pSI);
						pSI->Release();
					}
				}
				IObjectArray *pOA = NULL;
				hr = pOC->QueryInterface(IID_PPV_ARGS(&pOA));
				if (SUCCEEDED(hr))
				{
					//将自定义Category加入JumpList  
					pCDL->AppendCategory(TEXT("最近的工程"), pOA);
					pOA->Release();
				}
				hr = pCDL->CommitList();
				pOC->Release();
			}
			pOARemoved->Release();
		}
		pCDL->Release();
	}
}
#endif

void BkeCreator::LoadRecentProject()
{
	//读取最近使用列表
	QString ks;
	LOLI::AutoRead(ks, BKE_CURRENT_DIR + "/projects.txt");
	BKE_Recently_Project = ks.split("\r\n");
	for (auto &s : BKE_Recently_Project)
	{
		s.replace('\\', '/');
	}
	BKE_Recently_Project.removeDuplicates();
#ifdef Q_OS_WIN
	WinUpdateRecentList();
#endif
}

void BkeCreator::AddRecentProject(const QString &file)
{
    if( file == "##" ){
        BKE_Recently_Project.clear();
        LOLI::AutoWrite(BKE_CURRENT_DIR+"/projects.txt",QString()) ;
        return ;
    }
    //QRegExp exp(file) ;
    //if( isSYSTEMP_LOWDER ) exp.setCaseSensitivity(Qt::CaseInsensitive);

    int i = BKE_Recently_Project.indexOf(file) ;
    if( i > 0 ) BKE_Recently_Project.takeAt(i) ;
    else if( i == 0 ) return ;
    BKE_Recently_Project.prepend( file );
    while( BKE_Recently_Project.size() > 10 )BKE_Recently_Project.takeLast();
#ifdef Q_OS_WIN
	WinUpdateRecentList();
#endif
    LOLI::AutoWrite(BKE_CURRENT_DIR+"/projects.txt",BKE_Recently_Project.join("\r\n")) ;
}

void BkeCreator::RemoveRecentProject(const QString &file)
{
	QString path = file;
	path.replace('\\', '/');
	BKE_Recently_Project.removeOne(file);
#ifdef Q_OS_WIN
	WinUpdateRecentList();
#endif
	LOLI::AutoWrite(BKE_CURRENT_DIR + "/projects.txt", BKE_Recently_Project.join("\r\n"));
}

/*
void BkeCreator::AddRecentFile(const QString &file)
{
    if( file == "##" ){
        BKE_Recently_Files.clear();
        LOLI::AutoWrite(BKE_CURRENT_DIR+"/files.txt",QString()) ;
        return ;
    }

    int i = BKE_Recently_Files.indexOf(file) ;
    if( i > 0 ) BKE_Recently_Files.takeAt(i) ;
    else if( i == 0 ) return ;
    BKE_Recently_Files.prepend( file );
    while( BKE_Recently_Files.size() > 10 )BKE_Recently_Files.takeLast() ;
    LOLI::AutoWrite(BKE_CURRENT_DIR+"/files.txt",BKE_Recently_Files.join("\r\n")) ;
}*/

/*
void BkeCreator::ReNameRecentFile(const QString &old, const QString &now)
{
	int i = BKE_Recently_Files.indexOf(old);
	if (i >= 0)
		BKE_Recently_Files[i] = now;
}*/

QStringList BkeCreator::CopyStencil(const QString &dir,const QStringList &ls)
{
    QFile f;
    QStringList list ;
    for( int i = 0 ; i < ls.size() ; i++){
        f.setFileName(BKE_CURRENT_DIR+"/Stencil/"+ls.at(i));
        if( !f.copy(dir+"/"+ls.at(i)) ) list.append(ls.at(i));
    }
    return list ;
}

//读取api列表
/*
void BkeCreator::ReadApiList(QStringList *ls,const QString &name,int type)
{
    ls->clear();
    QString text ;
    LOLI::AutoRead(text,name) ;
    QStringList ks = text.split("\r\n") ;
    for( int i = 0 ; i < ks.size() ; i++){
        ls->append( QString(ks.at(i)+"?%1").arg(type) );
    }

    ls->sort(Qt::CaseInsensitive);
    return ;
}
*/


QString BkeCreator::IntToRgbString(unsigned int rgb)
{
    QString temp ;
    temp.setNum(rgb,16) ;
    while( temp.length() < 6) temp.prepend("0" ) ;
    while( temp.length() > 6) return temp.right(6) ;
    return temp ;
}

bool BkeCopyDirRecursively(QString fromDir, QString toDir, bool replaceOnConflict)
{
	QDir dir;
	dir.setPath(fromDir);

	fromDir += QDir::separator();
	toDir += QDir::separator();

	foreach(QString copy_file, dir.entryList(QDir::Files))
	{
		QString from = fromDir + copy_file;
		QString to = toDir + copy_file;

		if (QFile::exists(to))
		{
			if (replaceOnConflict)
			{
				if (QFile::remove(to) == false)
				{
					return false;
				}
			}
			else
			{
				continue;
			}
		}

		if (QFile::copy(from, to) == false)
		{
			return false;
		}
	}

	foreach(QString copy_dir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
	{
		QString from = fromDir + copy_dir;
		QString to = toDir + copy_dir;

		if (dir.mkpath(to) == false)
		{
			return false;
		}

		if (BkeCopyDirRecursively(from, to, replaceOnConflict) == false)
		{
			return false;
		}
	}

	return true;
}
