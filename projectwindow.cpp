#include <weh.h>
#include "projectwindow.h"
#include "dia/newprodia.h"
#include "BKS_info.h"
#include <QSize>
#include "dia/lablesuredialog.h"
#include "mainwindow.h"
#include <QApplication>
#include <QWindow>
#ifdef WIN32
#include <Windows.h>
#endif
#include <stdint.h>
#include "dia/ParserEditor.h"
#include "CmdListLoader.h"

//QList<BkeProject*> projectlist ;

extern uint32_t BKE_hash(const wchar_t *str);

ProjectWindow::ProjectWindow(QWidget *parent)
	:QTreeWidget(parent)
{
	setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT + "/projectlist").toString());
	setHeaderHidden(true);
	setContextMenuPolicy(Qt::CustomContextMenu);

	connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(ItemDoubleClick(QTreeWidgetItem*, int)));
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowRmenu(QPoint)));
	connect(this, SIGNAL(CurrentProChange(BkeProject *)), (MainWindow*)parent, SLOT(CurrentProChange(BkeProject *)));

	QStringList ls = QString("编译脚本 发布游戏 插入路径 预览 新建脚本 新建脚本 添加文件 添加目录 导出剧本 在文件夹中显示 在这里搜索 移除 关闭工程 重命名 从本文件开始运行").split(" ");
	for (int i = 0; i < BTN_COUNT; i++){
		btns[i] = new QAction(ls.at(i), this);
		connect(btns[i], SIGNAL(triggered()), this, SLOT(ActionAdmin()));
	}

	//del键
	action_del = new QAction(this);
	connect(action_del, SIGNAL(triggered()), this, SLOT(DeleteCurrentFile()));
	action_del->setShortcutContext(Qt::WidgetShortcut);
	action_del->setShortcut(Qt::Key_Delete);
	addAction(action_del);

	workpro = NULL;
}

ProjectWindow::~ProjectWindow()
{
	delete workpro;
}

void ProjectWindow::NewProject()
{
	NewProDia use(this);
	use.WaitUser();
	if (use.type<0)
	{
		return;
	}
	if (workpro)
	{
		global_bke_info.save();
		CloseProject();
	}
	workpro = new BkeProject(this);
	if (use.type == 0){
		if (!workpro->NewProject(use.okdir, use.okname))
		{
			delete workpro;
			workpro = nullptr;
			QMessageBox::information(this, "错误", "工程文件创建失败", QMessageBox::Ok);
			return;
		}
	}
	else if (use.type == 1){
		if (!workpro->OpenProject(use.okname))
		{
			delete workpro;
			workpro = nullptr;
			QMessageBox::information(this, "错误", "文件不存在，工程打开失败", QMessageBox::Ok);
			return;
		}
	}

	//projectlist << pro ;
	BkeChangeCurrentProject();
	BkeCreator::AddRecentProject(workpro->ProjectDir() + BKE_PROJECT_NAME);
}

void ProjectWindow::OpenProject()
{
	QString name = QFileDialog::getOpenFileName(this, "打开工程", "", "BKE Creator工程(*.bkp)");
	OpenProject(name);
}

void ProjectWindow::OpenProject(const QString &file)
{
	if (file.isEmpty()) return;
	if (workpro && file == workpro->ProjectFile())
		return;
	CloseProject();
	QString path = QFileInfo(file).path();
	//for(auto it = projectlist.begin();it != projectlist.end();++it)
	//{
	//    if((*it)->FileDir()==path)
	//    {
	//        BkeChangeCurrentProject(*it);
	//        return;
	//    }
	//}

	workpro = new BkeProject(this);
	if (!workpro->OpenProject(file))
	{
		delete workpro;
		workpro = NULL;
		QMessageBox::information(this, "错误", "文件不存在，工程打开失败", QMessageBox::Ok);
		BkeCreator::RemoveRecentProject(file);
		return;
	}
	//projectlist << pro ;
	BkeChangeCurrentProject();

	//    //读取书签
	//    QString text ;
	//    LOLI::AutoRead(text,pro->FileDir()+"/BkeProject.bmk") ;
	//    emit TextToMarks(text,pro->FileDir(),0);
	BkeCreator::AddRecentProject(workpro->ProjectDir() + BKE_PROJECT_NAME);
	//默认展开节点

	emit onProjectOpen(workpro);
}

//读取工程，并发送
bool ProjectWindow::ReadItemInfo(QTreeWidgetItem *dest, ItemInfo &f)
{
	if (dest == NULL) return false;
	BKE_PROJECT_READITEM(dest, f);
	return true;
}

//工程被双击
void ProjectWindow::ItemDoubleClick(QTreeWidgetItem * item, int column)
{
	if (!ReadItemInfo(item, info)) return;

	OpenProjectFile(info.FullName);
}

void ProjectWindow::OpenProjectFile(QString file)
{
	QString name = workpro->ProjectDir() + file;

	if (file == "config.bkpsr")
	{
		ConfigProject(workpro->config);
		return;
	}

	if (file.endsWith(".bkscr") || file.endsWith(".txt") || file.endsWith(".csv"))
	{
		emit OpenBkscrFile(name);
	}
	else if (file.endsWith(".bkpsr"))
	{
		ParserEditor *edit = new ParserEditor(name);
		edit->load();
		QString error = edit->error();
		if (!error.isEmpty())
		{
			emit OpenBkscrFile(name);
			edit->setParent(NULL);
			delete edit;
		}
		else
			edit->show();
	}
}

//右键菜单
void ProjectWindow::ShowRmenu(const QPoint & pos)
{
	if (!ReadItemInfo(currentItem(), info)) return;

	if (info.Layer == 1 && info.Name == "config.bkpsr")
		return;

	QPoint pt = QCursor::pos();
	QMenu mn;

	if (info.Layer < 1){
		//mn.addAction(btns[btn_active]) ;
		mn.addAction(btns[btn_compile]);
		mn.addAction(btns[btn_release]);
	}
	else if (info.Layer > 1){
		mn.addAction(btns[btn_insertdir]);
		//mn.addAction(btns[btn_preview]);
	}

	mn.addSeparator();

	if (info.Layer == 1 || (info.Layer > 1 && info.IconKey == BkeProject::dirsico.cacheKey()))
	{
		auto info2 = info.getLayer1ItemInfo();
		if (info2.Name == "宏")
			mn.addAction(btns[btn_newimport]);
		if (info2.Name == "脚本")
			mn.addAction(btns[btn_newscript]);
	}

	for (int i = btn_addfile; i <= btn_search; i++){
		if(i == btn_exportscenario && (info.getLayer1ItemInfo().Name != "脚本" || !info.Name.endsWith(".bkscr"))) continue;
		mn.addAction(btns[i]);
		if (i == btn_adddir) mn.addSeparator();
		else if (i == btn_newimport) mn.addSeparator();
	}

	mn.addSeparator();
	if (info.Layer < 1){
		mn.addAction(btns[btn_close]);
	}
	else if (info.Layer > 1){
		mn.addAction(btns[btn_remove]);
		mn.addAction(btns[btn_rename]);

		auto info2 = info.getLayer1ItemInfo();
		if (info2.Name == "脚本")
			mn.addAction(btns[btn_runfromit]);
	}

	pt.setX(pt.x() + 10);
	mn.exec(pt);
	return;

}

//设置选中项
void ProjectWindow::SetCurrentItem(const QString &file)
{
	if (!workpro)
		return;
	//BkeProject *abc;
	QTreeWidgetItem *le = workpro->FindItemAll(file);
	if (le)
	{
		setCurrentItem(le);
	}
	//for (int i = 0; i < projectlist.size(); i++){
	//       abc = projectlist.at(i) ;
	//       le = abc->FindItem(abc->Import,file,false) ;
	//       if( le == 0 ) le = abc->FindItem(abc->Script,file,false ) ;
	//       if( le == 0 ) le = abc->FindItem(abc->Source,file,false ) ;
	//       if( le != 0 ){
	//           setCurrentItem(le);
	//           return ;
	//       }
	//   }
}

void ProjectWindow::NewFile(const ItemInfo &f, int type)
{
	QString name = QInputDialog::getText(this, "新建脚本", "输入脚本名称，如: \r\n   abc \r\n   abc/sence.bkscr");
	if (name.isEmpty())
		return;
	else if (!name.endsWith(".bkscr"))
		name.append(".bkscr");
	name = name.replace("\\", "/");

	bool ismacro = false;
	if (type == 1)
	{
		//macro
		//自动添加一些内容
		ismacro = true;
	}

	BkeProject *p = workpro;
	QFileInfo sk(workpro->ProjectDir() + f.getFullName() + '/' + name);
	if (sk.exists()){
		int sk = QMessageBox::information(this, "", "文件已经存在，是否直接添加文件", QMessageBox::Yes | QMessageBox::No);
		if (sk == QMessageBox::No)
			return;
	}
	else
	{
		LOLI_MAKE_NULL_FILE(sk.filePath(), (ismacro ? "macro.bkscr" : ""));
	}

	workpro->AddFiles(QStringList() << name, f);
	
	//then open
	OpenProjectFile(f.getFullName() + '/' + name);
}


QTreeWidgetItem *ProjectWindow::findFileInProject(const QString &name)
{
	//没有工程返回0
	if (!workpro) return 0;
	return workpro->FindItemAll(name);
	//temppro = workpro ;
	//QTreeWidgetItem *le = temppro->FindItemAll(name) ;
	//if( projectlist.size() == 1) return le ;

	//for( int i = 0 ; i < projectlist.size() ; i++){
	//    temppro = projectlist.at(i) ;
	//    if( temppro != workpro) le = temppro->FindItemAll(name) ;
	//    if( le != 0) break ;
	//}
	//return le ;
}

void ProjectWindow::RunBKEFromFile(const ItemInfo & f)
{
	QStringList args;
	args << "-startscript" << f.Name;
	codeedit->CompileAndRun(args);
}

#undef DeleteFile
void ProjectWindow::DeleteCurrentFile()
{
	ItemInfo info;
	if (!ReadItemInfo(currentItem(), info)) return;
	DeleteFile(info);
}

void ProjectWindow::DeleteFile(const ItemInfo &f)
{
	//是文件的话将询问是否移除文件
	QString sk = workpro->ProjectDir() + f.FullName;
	LableSureDialog msg;
	msg.SetLable("要移除文件" + sk + "吗？");
	msg.SetCheckbox(QStringList() << "彻底删除");
	msg.SetBtn(QStringList() << "移除" << "取消");
	if (msg.WaitUser(240, 130) == 1) return;

	if (msg.IsCheckboxChoise(0)){
		if (workpro->CheckIsDir(f))
			QDir(sk).removeRecursively();
		else
			QFile(sk).remove();
	}
	//移除文件
	if (!workpro->RemoveItem(f)){
		QMessageBox::information(this, "错误", "移除工程过程中出了一个错误", QMessageBox::Ok);
		return;
	}
	//写出结果
	workpro->WriteBkpFile();
}

void ProjectWindow::AddFiles(const ItemInfo &f)
{
	QStringList ls;
	if (f.Layer > 1 && !workpro->CheckIsDir(f))
	{
		ReadItemInfo(f.Root->parent(), info);
	}
	QString path = workpro->ProjectDir() + f.getFullName();
	if (f.RootName == "宏" || f.RootName == "脚本"){
		ls = QFileDialog::getOpenFileNames(this, "添加文件", path, "bkscr脚本(*.bkscr);;Bagel脚本(*.bkpsr);;文本文件(*.txt *.csv)");
	}
	else ls = QFileDialog::getOpenFileNames(this, "添加文件", path, "所有文件(*.*)");

	if (ls.isEmpty()) return;	

	QStringList errors;
	auto it = ls.begin();
	while (it != ls.end())
	{
		QString rfile = BkeFullnameToName(*it, path);
		if (rfile.isEmpty())
		{
			errors.append(*it);
			it = ls.erase(it);
		}
		else
		{
			*it = rfile;
			it++;
		}
	}
	if (!errors.isEmpty())
	{
		if (QMessageBox::information(this, "警告", "选中了选定目录以外的文件：\n" + errors.join('\n') + "是否拷贝到" + f.getFullName() + "下？", QMessageBox::Yes | QMessageBox::No) == QDialogButtonBox::Yes)
		{
			for (auto &&file : errors)
			{
				QString filename = QFileInfo(file).fileName();
				QString dest = path + "/" + filename;
				if (QFile::exists(dest))
				{
					if (QMessageBox::warning(this, "警告", "该文件夹下已存在文件：" + filename + "，是否覆盖？", QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
					{
						continue;
					}
					else
					{
						if (!QFile::remove(dest))
						{
							QMessageBox::warning(this, "警告", "文件覆盖失败，该文件正在被使用：" + filename + "。");
							continue;
						}
					}
					if (!QFile::copy(file, dest))
					{
						QMessageBox::warning(this, "警告", "文件覆盖失败，该文件正在被使用：" + filename + "。");
						continue;
					}
					ls.append(filename);
				}
			}
		}
	}
	workpro->AddFiles(ls, f);
}

//
void ProjectWindow::AddDir(const ItemInfo &f)
{
	if (f.Layer > 1 && !workpro->CheckIsDir(f))
	{
		ReadItemInfo(f.Root->parent(), info);
	}
	QString path = workpro->ProjectDir() + f.getDir();
	QString d = QFileDialog::getExistingDirectory(this, "添加目录", path);
	if (d.isEmpty())
		return;
	QString tmp = BkeFullnameToName(d, path);
	if (tmp.isEmpty()){
		if (QMessageBox::information(this, "警告", "选中了选定目录以外的文件夹：\n" + tmp + "是否拷贝到" + f.getFullName() + "下？", QMessageBox::Yes | QMessageBox::No) == QDialogButtonBox::Yes)
		{
			QString dir = QDir(d).dirName();
			QString dest = path + "/" + dir;
			if (QDir(dest).exists())
			{
				if (QMessageBox::warning(this, "警告", "该文件夹下已存在文件夹：" + dir + "，是否覆盖？", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
				{
					BkeCopyDirRecursively(d, dest, true);
					tmp = dir;
				}
			}
			else
			{
				BkeCopyDirRecursively(d, dest, true);
				tmp = dir;
			}
		}
	}
	if (!tmp.isEmpty()) {
		workpro->AddDir(tmp, f);
	}
}

//void ProjectWindow::ReName()
//{
//    QString name = QInputDialog::getText(this,"重命名","将要改变到的名称，后缀名可以省略。不能含有\/:*?\"<>|") ;
//    if( name.isEmpty() ) return ;
//    else if( !name.endsWith(".bkscr")) name.append(".bkscr") ;
//
//    QString oldname = workpro->FileDir()+"/"+info.FullName ;
//    QString newname = oldname.left(oldname.lastIndexOf("/")) + "/" + name ;
//
//    QFile temp(newname) ;
//    if( temp.exists() ){
//        QMessageBox::information(this,"重命名","文件已存在，不能再使用这个名称。",QMessageBox::Ok) ;
//        return ;
//    }
//
//    bool result ;
//    emit FileNameChange(oldname,newname,result);
//    if( !result){
//        QMessageBox::warning(this,"重命名","重命名失败") ;
//        return ;
//    }
//
//    QTreeWidgetItem *le = currentItem() ;
//    le->setText(0,name);
//    workpro->WriteBkpFile() ;
//}


void ProjectWindow::OpenFile()
{
	QString filename = QFileDialog::getOpenFileName(this, "选择要打开的文件", "", "bksfile(*.bkscr);;Allfile(*.*)");
	if (filename.isNull()) return;

	QTreeWidgetItem *llm = findFileInProject(filename);
	if (llm == 0) 
		return;
	OpenBkscrFile(filename);
}

void ProjectWindow::BkeChangeCurrentProject(/*BkeProject *p*/)
{
	workpro->SetTopLeveBold(true);
	emit CurrentProChange(workpro);
	
	if (!CmdListLoader::load())
	{
        QMessageBox::warning(0, "警告", "BKECmdList库不存在，智能提示和自动补全功能将禁用。");
	}
}

void ProjectWindow::ConfigProject(BkeProjectConfig *config)
{
	QBkeConfigDialog kag;
	kag.StartConfig(config);
}

void ProjectWindow::ActionAdmin()
{
	QAction *p = dynamic_cast<QAction*>(sender());

	if (p == NULL) return;
	//else if( p == btns[btn_active] ) Active(info);
	else if (p == btns[btn_compile]) emit Compile();
	else if (p == btns[btn_release]);
	else if (p == btns[btn_insertdir]) emit DirWillBeInsert(info.FullName);
	else if (p == btns[btn_preview]) PreviewFile(info);
	else if (p == btns[btn_newscript]) NewFile(info, 2);
	else if (p == btns[btn_newimport]) NewFile(info, 1);
	else if (p == btns[btn_addfile]) AddFiles(info);
	else if (p == btns[btn_adddir]) AddDir(info);
	else if (p == btns[btn_exportscenario]) ExportScenario(info);
	else if (p == btns[btn_showindir]) ShowInDir(info);
	else if (p == btns[btn_search]);
	else if (p == btns[btn_remove]) DeleteFile(info);
	else if (p == btns[btn_close]) CloseProject(info);
	else if (p == btns[btn_rename]) RenameFile(info);
	else if (p == btns[btn_runfromit]) RunBKEFromFile(info);
}

void ProjectWindow::RenameFile(const ItemInfo &f)
{
	if (f.Layer <= 1)
		return;
	if (!workpro->CheckIsDir(f))
	{
		//file
		bool change;
		QString name = QInputDialog::getText(this, "重命名", "输入新名称(文件路径和文件类型不可更改)", QLineEdit::Normal, chopFileNameWithoutExt(f.FullName), &change);
		if (!change)
			return;
		QString ext = chopFileExt(f.Name);
		name = chopFileNameWithoutExt(name) + ext;
		if (name == ".bkscr")
			return;
		QString rawname = workpro->ProjectDir() + f.FullName;
		QString newname = workpro->ProjectDir() + f.Dirs + name;
		QFile temp(newname);
		if (temp.exists()){
			QMessageBox::information(this, "重命名", "文件已存在，不能再使用这个名称。", QMessageBox::Ok);
			return;
		}
		auto res = QFile(rawname).rename(newname);
		if (!res)
		{
			QMessageBox::warning(this, "重命名", "重命名失败");
			return;
		}
		f.Root->setText(0, name);
		if (newname.endsWith(".bkscr"))
		{
			emit FileNameChange(rawname, newname);
		}
	}
	else
	{
		QString name = QInputDialog::getText(this, "重命名", "输入新文件夹名(不可新建子文件夹)");
		name = chopFileName(name);
		QString rawname = workpro->ProjectDir() + f.FullName;
		QString newname = workpro->ProjectDir() + f.Dirs + name;
		QDir temp(newname);
		if (temp.exists()){
			QMessageBox::information(this, "重命名", "文件已存在，不能再使用这个名称。", QMessageBox::Ok);
			return;
		}
		auto res = temp.rename(rawname, newname);
		if (!res)
		{
			QMessageBox::warning(this, "重命名", "重命名失败");
			return;
		}
		f.Root->setText(0, name);
		workpro->config->removeScriptDir(f.FullName);
		workpro->config->addScriptDir(f.Dirs + name);
	}
	workpro->WriteBkpFile();
}

void ProjectWindow::PreviewFile(const ItemInfo &f)
{
	QString n = workpro->ProjectDir() + info.FullName;
	if (info.FullName.endsWith(".bkscr") || info.FullName.endsWith(".bkpsr"))
		emit OpenBkscrFile(n);
	else
	{
		QDesktopServices::openUrl(QUrl::fromLocalFile(n));
	}
}

void ProjectWindow::CloseProject()
{
	if (!workpro)
		return;
	//workpro->deleteLater();
	//先closeALl脚本窗口
	emit onProjectClose();
	delete workpro;
	workpro = NULL;
	emit CurrentProChange(workpro);
}

void ProjectWindow::CloseProject(const ItemInfo &f)
{
	CloseProject();
}

//void ProjectWindow::Active(const ItemInfo &f)
//{
//    BkeProject *p = FindPro(f.ProName);
//    if(p)
//    {
//        BkeChangeCurrentProject(p);
//    }
//}

#if defined(Q_OS_WIN)
#include <windows.h>
#include <shellapi.h>
#pragma comment(lib,"shell32.lib")
#endif

void ProjectWindow::ShowInDir(const ItemInfo &f)
{
	QString n = workpro->ProjectDir();
	if (info.Layer > 1)
		n += info.FullName;
	else
		n += '/';
#if defined(Q_OS_WIN)
	n.replace('/', '\\');
	QByteArray a = ("/select," + n).toLocal8Bit();
	ShellExecuteA(NULL, "open", "explorer.exe", a.data(), NULL, true);
#elif defined(Q_OS_MAC)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                     .arg(n);
    QProcess::execute("/usr/bin/osascript", scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
               << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute("/usr/bin/osascript", scriptArgs);
#else
    const QFileInfo fileInfo(n);;
    const QString app = QLatin1String("xdg-open ") + fileInfo.path();
    QProcess::startDetached(app);
#endif
}

void ProjectWindow::ReleaseGame(const ItemInfo &f)
{
	workpro->ReleaseGame();
}

void ProjectWindow::ExportScenario(const ItemInfo &f)
{
	if (workpro)
	{
		QString text;
		QString path = workpro->ProjectDir() + f.FullName;
		if (LOLI::AutoRead(text, path))
		{
			text = CodeWindow::getScenarioTextFromCode(text).replace("\n\n","\n");
			path.truncate(path.lastIndexOf('.'));
			if (LOLI::AutoWrite(path + ".scenario.txt", text))
			{
				if (QMessageBox::question(NULL, "导出文本", "文本导出成功！是否打开导出目录？") == QMessageBox::Yes)
				{
					ShowInDir(f.getParentInfo());
				}
				return;
			}
		}
		QMessageBox::warning(NULL, "导出文本", "导出失败！");
	}
}

void ProjectWindow::ReleaseCurrentGame()
{
	if (workpro)
	{
		workpro->ReleaseGame();
	}
}
