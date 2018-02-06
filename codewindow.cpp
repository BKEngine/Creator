#include <weh.h>
#include <QGuiApplication>
#include "codewindow.h"
#include "dia/lablesuredialog.h"
#include "dia/WaitWindow.h"
#include "dia/openlabeldialog.h"
#include "dia/gotofiledialog.h"
#include "dia/autocompletelist.h"

CodeWindow::CodeWindow(QWidget *parent)
	:QMainWindow(parent)
{
	diasearch = new SearchBox(this);

	//自动补全初始化
	aclist = new AutoCompleteList(this);
	aclist->DefineIcon(1, QIcon(":/auto/auto_function.png"));
	aclist->DefineIcon(3, QIcon(":/auto/auto_normal.png"));
	aclist->DefineIcon(9, QIcon(":/auto/auto_key.png"));
	aclist->DefineIcon(BAGEL_STARTMARK + PromptType::API_KEYWORDS, QIcon(":/auto/auto_key.png"));
	aclist->DefineIcon(BAGEL_STARTMARK + PromptType::API_VARIABLE, QIcon(":/auto/auto_normal.png"));
	aclist->DefineIcon(BAGEL_STARTMARK + PromptType::API_FUNCTION, QIcon(":/auto/auto_function.png"));
	aclist->DefineIcon(BAGEL_STARTMARK + PromptType::API_CLASS, QIcon(":/auto/auto_function.png"));
	aclist->DefineIcon(BAGEL_STARTMARK + PromptType::API_PROPERTY, QIcon(":/auto/auto_normal.png"));
	aclist->hide();
	aclist->SetStops(" ~,./!@#$%^&()+-=\\;'[]{}|:?<>");

	addDockWidget(Qt::BottomDockWidgetArea, diasearch);
	currentedit = nullptr;
	labelbanned = false;

	ks1.setColor(QColor(0x80, 0, 0));
	ks1.setPaper(QColor(0xff, 0xf0, 0xf0));
	ks2.setColor(QColor(0x80, 0, 0x80));
	ks2.setPaper(QColor(0xff, 0xf0, 0xff));
	CreateBtn();

	filewatcher = new QFileSystemWatcher(this);
	connect(filewatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(QfileChange(const QString &)));
	watcherflag = 1 ;

	currentpos = -1;
	currentbase = NULL;
	stackwidget = new QStackedWidget(this);
	setCentralWidget(stackwidget);
	DrawLine(true);

	this->addToolBar(Qt::BottomToolBarArea, toolbar2);


	connect(lablelist, SIGNAL(currentIndexChanged(int)), this, SLOT(SetCurrentEdit(int)));
	connect(stackwidget, SIGNAL(currentChanged(int)), this, SLOT(SetCurrentEdit(int)));
	connect(btnnextact, SIGNAL(triggered()), this, SLOT(NextNavigation()));
	connect(btnlastact, SIGNAL(triggered()), this, SLOT(LastNavigation()));
	connect(btnsaveact, SIGNAL(triggered()), this, SLOT(SaveFile()));
	connect(btncloseact, SIGNAL(triggered()), this, SLOT(CloseFile()));
	connect(btnsaveasact, SIGNAL(triggered()), this, SLOT(SaveAs()));
	//编译
	connect(btncompileact, SIGNAL(triggered()), this, SLOT(CompileAll()));
	connect(btncompilelang, SIGNAL(triggered()), this, SLOT(CompileLang()));
	//编译并运行
	connect(btncompilerunact, SIGNAL(triggered()), this, SLOT(CompileAndRun()));
	connect(btnbookmarkact, SIGNAL(triggered()), this, SLOT(AddBookMark()));
	//查找
	connect(btnfindact, SIGNAL(triggered()), diasearch, SLOT(SearchModel()));
	connect(btnfindactall, SIGNAL(triggered()), diasearch, SLOT(SearchAllModel()));
	connect(btnreplaceact, SIGNAL(triggered()), diasearch, SLOT(ReplaceModel()));
	connect(btnreplaceallact, SIGNAL(triggered()), diasearch, SLOT(ReplaceAllModel()));
	connect(diasearch, SIGNAL(searchOne(const QString &, const QString &, bool, bool, bool)), this, SLOT(searchOneFile(const QString &, const QString &, bool, bool, bool)));
	connect(diasearch, SIGNAL(searchAll(const QString &, bool, bool, bool)), this, SLOT(searchAllFile(const QString &, bool, bool, bool)));
	connect(diasearch, SIGNAL(replaceAll(const QString &, const QString &, bool, bool, bool, bool)), this, SLOT(replaceAllFile(const QString &, const QString &, bool, bool, bool, bool)));
	connect(&comtool, SIGNAL(CompliteFinish()), this, SLOT(CompileFinish()));
	connect(&comtool, SIGNAL(CompliteError(QString)), this, SLOT(CompileError(QString)));
	connect(btnrunact, SIGNAL(triggered()), this, SLOT(RunBKE()));
	connect(btnrunfromlabel, SIGNAL(triggered()), this, SLOT(RunBKEWithArgs()));
	connect(pannote, SIGNAL(triggered()), this, SLOT(AnnotateSelect()));
	connect(btnclearact, SIGNAL(triggered()), this, SLOT(ClearCompileAndSaveData()));
	//编码转换
	connect(btncodeact, SIGNAL(triggered()), this, SLOT(ChangeCodec()));
	connect(btnselectall, SIGNAL(triggered()), this, SLOT(SelectAll()));
	//转到行
	connect(btnfly, SIGNAL(triggered()), this, SLOT(GotoLine()));
	//点击标签
	connect(slablelist, SIGNAL(currentIndexChanged(int)), this, SLOT(GotoLabel(int)));
	//重做
	connect(btnredoact, SIGNAL(triggered()), this, SLOT(ActRedo()));
	//撤销
	connect(btnundoact, SIGNAL(triggered()), this, SLOT(ActUndo()));
	//剪切
	connect(btncutact, SIGNAL(triggered()), this, SLOT(ActCut()));
	//粘贴
	connect(btnpasteact, SIGNAL(triggered()), this, SLOT(ActPaste()));
	//复制
	connect(btncopyact, SIGNAL(triggered()), this, SLOT(ActCopy()));
	//切换折叠
	connect(btnswitchfold, SIGNAL(triggered()), this, SLOT(SwitchFold()));

	btnDisable();
	ignoreflag = false;
	isRun = false;
	isSearLable = false; //是否在查找标签，如果是，刷新文件队列
	ignoreActive = false;

	searchlablelater = nullptr;
	isCompileNotice = _NOTICE_ALWAYS;

	tm.setTimerType(Qt::TimerType::VeryCoarseTimer);
	connect(&tm, SIGNAL(timeout()), this, SLOT(onTimer()));
	float interval = BKE_CLOSE_SETTING->value("autosavetime", 1).toFloat();
	tm.start((int)(60000 * interval));

	//////////////////////DEBUG COMPONENT/////////////////////
	debugServer = new DebugServer(this);
	connect(debugServer, SIGNAL(logReceived(int32_t, QString)), this, SLOT(DebugLogReceived(int32_t, QString)));

	ChangeProject(nullptr);
}

CodeWindow::~CodeWindow()
{
	if (bkeprocess)
	{
		bkeprocess->kill();
	}
}

void CodeWindow::onTimer()
{
	backupAll();
}

void CodeWindow::CreateBtn()
{
	toolbar = new QToolBar;
	toolbar->setFixedHeight(24);
	toolbar->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT + "/codetoolbar").toString());

	btnlastact = new QAction(QIcon(":/cedit/btnlast.png"), "返回", this);
	btnnextact = new QAction(QIcon(":/cedit/btnnext.png"), "前进", this);
	btnsaveact = new QAction(QIcon(":/cedit/save.png"), "保存", this);
	btnsaveasact = new QAction(QIcon(":/cedit/saveas.png"), "另存为...", this);
	btncodeact = new QAction(QIcon(":/cedit/code.png"), "改变源文件的编码", this);
	btncopyact = new QAction(QIcon(":/cedit/copy.png"), "复制", this);
	btncutact = new QAction(QIcon(":/cedit/edit_cut.png"), "剪切", this);
	btnpasteact = new QAction(QIcon(":/cedit/paste.png"), "粘贴", this);
	btncompileact = new QAction(QIcon(":/cedit/compile.png"), "编译成bkbin脚本", this);
	btncompilelang = new QAction(QIcon(":/cedit/compile.png"), "编译并生成语言文件", this);
	btncompilerunact = new QAction(QIcon(":/cedit/compile_run.png"), "编译并运行", this);
	btnrunact = new QAction(QIcon(":/cedit/run.png"), "运行", this);
	btndebugact = new QAction(QIcon(":/cedit/debug.png"), "调试", this);
	btncloseact = new QAction(QIcon(":/cedit/close.png"), "关闭", this);
	btnfindact = new QAction(QIcon(":/cedit/find.png"), "查找", this);
	btnfindactall = new QAction(QIcon(":/cedit/find.png"), "查找全部", this);
	btnreplaceact = new QAction(QIcon(":/cedit/replace(2).png"), "替换", this);
	btnreplaceallact = new QAction(QIcon(":/cedit/replace(2).png"), "替换全部", this);
    btnbookmarkact = new QAction(QIcon(":/cedit/bookmark.png"), "添加书签", this);
	btnmarkact = new QAction(QIcon(":/cedit/pin.png"), "添加标记", this);
	btnrunfromlabel = new QAction("从本标签处运行", this);
	btnredoact = new QAction(QIcon(":/cedit/redo.png"), "重做", this);
	btnundoact = new QAction(QIcon(":/cedit/undo.png"), "撤销", this);
	btnclearact = new QAction(QIcon(":/cedit/clear.png"), "清理编译工程和存档", this);
	pannote = new QAction("选中部分注释/反注释", this);
	btnselectall = new QAction("全选", this);
	btnfly = new QAction(QIcon(":/cedit/flay.png"), "转到行...", this);
	btngotolabellist = new QAction("转到标签", this);
	btngotofile = new QAction("转到文件", this);
	btnswitchfold = new QAction("全部折叠", this);
	btnautofix = new QAction("自动修正", this);

	//右键菜单
	jumpToDef = new QAction(this);
	jumpToCode = new QAction(this);
	gotoLabel = new QAction(this);

	connect(jumpToDef, SIGNAL(triggered()), this, SLOT(jumpToDefFunc()));
	connect(jumpToCode, SIGNAL(triggered()), this, SLOT(jumpToCodeFunc()));
	connect(gotoLabel, SIGNAL(triggered()), this, SLOT(jumpToLabelFunc()));
	connect(btngotolabellist, SIGNAL(triggered()), this, SLOT(GotoLabelList()));
	connect(btngotofile, SIGNAL(triggered()), this, SLOT(GotoFile()));
	connect(btnautofix, SIGNAL(triggered()), this, SLOT(AutoFix()));

	btnlastact->setShortcut(Qt::ALT + Qt::Key_Left);
	btnnextact->setShortcut(Qt::ALT + Qt::Key_Right);
	btnsaveact->setShortcut(Qt::CTRL + Qt::Key_S);
	//btncopyact->setShortcut(Qt::CTRL + Qt::Key_C);
	//btnpasteact->setShortcut(Qt::CTRL + Qt::Key_V);
	//btncutact->setShortcut(Qt::CTRL + Qt::Key_X);
	//btnundoact->setShortcut(Qt::CTRL + Qt::Key_Z);
	//btnredoact->setShortcut(Qt::CTRL + Qt::Key_Y);
	btnfindact->setShortcut(Qt::CTRL + Qt::Key_F);
	btnfindactall->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_F);
	addAction(btnfindactall);
	btnreplaceact->setShortcut(Qt::CTRL + Qt::Key_H);
	btnreplaceallact->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_H);
	addAction(btnreplaceallact);
	btncompileact->setShortcut(Qt::CTRL + Qt::Key_B);
	//btnselectall->setShortcut(Qt::CTRL + Qt::Key_A);
	btnfly->setShortcut(Qt::CTRL + Qt::Key_G);
	btncompilerunact->setShortcut(Qt::CTRL + Qt::Key_R);
	btngotolabellist->setShortcut(Qt::CTRL + Qt::Key_L);
	btngotofile->setShortcut(Qt::CTRL + Qt::Key_P);
	btnswitchfold->setShortcut(Qt::CTRL + Qt::Key_M);
	btnautofix->setShortcut(Qt::ALT + Qt::Key_Return);
	addAction(btnautofix);

	toolbar->addAction(btnlastact);
	toolbar->addAction(btnnextact);

	lablelist = new QComboBox;
	lablelist->setFixedSize(150, 22);
	lablelist->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT + "/codecombox").toString());
	lablelist->setView(new QListView());
	toolbar->addWidget(lablelist);

	toolbar->addSeparator();
	toolbar->addAction(btnsaveact);
	toolbar->addAction(btnsaveasact);
	toolbar->addAction(btncodeact);
	toolbar->addSeparator();
	toolbar->addAction(btnundoact);
	toolbar->addAction(btnredoact);
	toolbar->addAction(btnfindact);
	toolbar->addAction(btnreplaceact);
	toolbar->addSeparator();
	toolbar->addAction(btncopyact);
	toolbar->addAction(btncutact);
	toolbar->addAction(btnpasteact);
	toolbar->addSeparator();
	toolbar->addAction(btngotolabellist);
	toolbar->addAction(btngotofile);
	toolbar->addSeparator();

	slablelist = new QComboBox;
	slablelist->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	slablelist->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT + "/codecombox").toString());
	slablelist->setView(new QListView());
	toolbar->addWidget(slablelist);
	toolbar->addAction(btncloseact);

	//下边工具栏
	toolbar2 = new QToolBar(this);
	toolbar2->setFixedHeight(24);
	toolbar2->setMovable(false);
	toolbar2->addAction(btncompileact);
	toolbar2->addAction(btncompilerunact);
	toolbar2->addAction(btnrunact);
	toolbar2->addAction(btndebugact);
	toolbar2->addAction(btnclearact);

	QWidget *space = new QWidget(this);
	space->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	toolbar2->addWidget(space);
	kag = new QProgressBar(this);
	kag->setFixedSize(200, 20);

	kag->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT + "/processbar").toString());
	toolbar2->addWidget(kag);

}

void CodeWindow::BindOtherWindow(OtherWindow *win)
{
	othwin = win;
	//定位文件
	connect(win, SIGNAL(Location(BkeMarkerBase*, QString)), this, SLOT(ToLocation(BkeMarkerBase*, QString)));
	connect(this, SIGNAL(searchOne(const QString &, const QString &, int, int, int)), othwin, SLOT(onSearchOne(const QString &, const QString &, int, int, int)));
}

void CodeWindow::BindProjectWindow(ProjectWindow *p)
{
	prowin = p;
	//rename
	connect(p, &ProjectWindow::FileNameChange, this, &CodeWindow::Rename);
	//打开文件
	connect(p, &ProjectWindow::OpenBkscrFile, this, &CodeWindow::AddFile);
	//插入路径
	connect(p, &ProjectWindow::DirWillBeInsert, this, &CodeWindow::TextInsert);
	//编译脚本
	connect(p, &ProjectWindow::Compile, this, (void (QObject::*)())&CodeWindow::CompileAll);
	//当前工程被改变
	connect(p, &ProjectWindow::CurrentProChange, this, &CodeWindow::ChangeProject);
	//改变当前文件
	connect(this, SIGNAL(CurrentFileChange(QString)), p, SLOT(SetCurrentItem(QString)));
	//打开工程时读取书签以及标记
	connect(p, &ProjectWindow::TextToMarks, this, &CodeWindow::TextToMarks);
	//新建空白文档
	//connect(p->btnnewfileact,SIGNAL(triggered()),this,SLOT(NewEmptyFile())) ;
}

void CodeWindow::BindFileListWidget(BkeLeftFileWidget *flist)
{
	filewidget = flist;
	//改变文件
	connect(filewidget, &BkeLeftFileWidget::currentRowChanged, this, (void(QObject::*)(int))&CodeWindow::SetCurrentEdit);
	//关闭文件
	connect(filewidget->btnclose, &QAction::triggered, this, (void(QObject::*)())&CodeWindow::CloseFile);
	//关闭所有
	connect(filewidget->btnCloseAll, &QAction::triggered, this, &CodeWindow::CloseAll);
}

//设置正在编辑工程
void CodeWindow::SetCurrentEdit(int pos)
{
	if (ignoreflag) return;

	ignoreflag = true;
	stackwidget->setCurrentIndex(pos);
	lablelist->setCurrentIndex(pos);
	filewidget->setCurrentRow(pos);
	ignoreflag = false;

	ChangeCurrentEdit(pos);
}

void CodeWindow::resetLexer()
{
	if (currentbase)
	{
		auto lex = currentbase->edit->deflex;
		lex->ReadConfig(lex->ConfigName());
		currentbase->edit->setLexer(currentbase->edit->deflex);
	}
}

void CodeWindow::SetCurrentEdit(QWidget *w)
{
	stackwidget->setCurrentWidget(w);
	SetCurrentEdit(stackwidget->currentIndex());
}

QString CodeWindow::getScenarioTextFromCode(QString text)
{
	static auto le1 = QRegularExpression("(/\\*.+?\\*/)|(^\t*##.+?^\t*##)", QRegularExpression::MultilineOption | QRegularExpression::DontCaptureOption | QRegularExpression::DotMatchesEverythingOption);
	static auto le2 = QRegularExpression("(^\t*#.+$)|(^\t*@.+$)|(\t*\\[.+\\])|(//.+$)|(^\\*.+$)", QRegularExpression::MultilineOption | QRegularExpression::DontCaptureOption);
	static auto le3 = QRegularExpression("^ +$", QRegularExpression::MultilineOption | QRegularExpression::DontCaptureOption);
	return text.remove(le1).remove(le2).remove(le3);
}

void calcWords(QString text)
{
	otheredit->setTextCount(CodeWindow::getScenarioTextFromCode(text).remove(QRegExp("[\t\r\n]")).count());
}

void CodeWindow::AttachCurrentEdit()
{
	CurrentConnect(true);
	ConnectAutoComplete(currentedit);
	currentedit->clearAnnotationsAll();
	currentedit->restoreTopLine();

	AddMarksToEdit();
	refreshLabel(currentedit);
	calcWords(currentedit->text());

	// 导航
	AddNavigation(currentbase->Name(), currentedit->GetCurrentPosition());
	currentedit->Attach();
}

void CodeWindow::DetachCurrentEdit()
{
	aclist->Cancel();
	DisconnectAutoComplete(currentedit);
	CurrentConnect(false);
	leaveClickGotoMode();
	currentedit->saveTopLine();
	currentedit->Detach();
}

//改变正在编辑的文档，文件列表选项是同步改变的
void CodeWindow::ChangeCurrentEdit(int pos)
{
	currentpos = pos;  //当前位置
	if (currentedit == stackwidget->currentWidget()) return;

	btncopyact->setEnabled(false);

	if (pos >= 0)
	{
		currentbase = docWidgetHash.value(stackwidget->currentWidget(), nullptr);
		if (currentbase == nullptr) {
			QMessageBox::critical(this, "", "致命错误：没有找到匹配的QWidget！", QMessageBox::Ok);
			return;
		}
	}
	else
	{
		currentbase = nullptr;
	}
	
	if (currentedit)
	{
		DetachCurrentEdit();
	}
	
	if (currentbase)
	{
		if (!stackwidget->styleSheet().isEmpty()) DrawLine(true);
		//改变工程
		//ChangeProject(prowin->FindProjectFromDir(currentbase->ProjectDir()));
		currentedit = currentbase->edit;
		currentedit->deflex->ReadConfig(currentedit->deflex->ConfigName());
		currentedit->setLexer(currentedit->deflex);

		markadmin.SetFile(currentbase->FullName());  //标记管理器
		btnsaveact->setEnabled(currentedit->isModified());
		btnsaveasact->setEnabled(true);
		btnpasteact->setEnabled(true);
		btnfindact->setEnabled(true);
		btnreplaceact->setEnabled(true);
		btnbookmarkact->setEnabled(true);
		btnmarkact->setEnabled(true);
		btnundoact->setEnabled(currentedit->isUndoAvailable());
		btnredoact->setEnabled(currentedit->isRedoAvailable());
		btncodeact->setEnabled(true);
		btncloseact->setEnabled(stackwidget->count() > 0);
		btnfly->setEnabled(true);

		emit CurrentFileChange(currentbase->FullName());
		emit CurrentFileChange(currentbase->Name(), currentbase->ProjectDir());
		AttachCurrentEdit();
	}
	else
	{
		currentedit = nullptr;
	}
	//reset lexer
	//连接文档改变信号
	diasearch->SetSci(currentedit); //查找管理
}

//断开、连接当前文档信号
void CodeWindow::CurrentConnect(bool c)
{
	if (c){
		connect(currentedit, SIGNAL(copyAvailable(bool)), btncopyact, SLOT(setEnabled(bool)));
		connect(currentedit, SIGNAL(copyAvailable(bool)), btncutact, SLOT(setEnabled(bool)));
		//工程被改变，需要从下层传递信号
		connect(currentedit, SIGNAL(textChanged()), this, SLOT(ActCurrentChange()));
		connect(currentedit, SIGNAL(refreshLabel(BkeScintilla *)), this, SLOT(refreshLabel(BkeScintilla *)));
		connect(currentedit, SIGNAL(refreshLabel(QSortedSet<QString> &)), this, SLOT(refreshLabel(QSortedSet<QString> &)));
		connect(currentedit, SIGNAL(ShouldAddToNavigation()), this, SLOT(ShouldAddToNavigation()));
		connect(currentedit, SIGNAL(indicatorReleased(int, int, Qt::KeyboardModifiers)), this, SLOT(indicatorReleased(int, int, Qt::KeyboardModifiers)));
	}
	else{
		disconnect(currentedit, SIGNAL(copyAvailable(bool)), btncopyact, SLOT(setEnabled(bool)));
		disconnect(currentedit, SIGNAL(copyAvailable(bool)), btncutact, SLOT(setEnabled(bool)));
		disconnect(currentedit, SIGNAL(textChanged()), this, SLOT(ActCurrentChange()));
		disconnect(currentedit, SIGNAL(refreshLabel(BkeScintilla *)), this, SLOT(refreshLabel(BkeScintilla *)));
		disconnect(currentedit, SIGNAL(refreshLabel(QSortedSet<QString> &)), this, SLOT(refreshLabel(QSortedSet<QString> &)));
		disconnect(currentedit, SIGNAL(ShouldAddToNavigation()), this, SLOT(ShouldAddToNavigation()));
		disconnect(currentedit, SIGNAL(indicatorReleased(int, int, Qt::KeyboardModifiers)), this, SLOT(indicatorReleased(int, int, Qt::KeyboardModifiers)));
	}
}

void CodeWindow::btnDisable()
{
	btnnextact->setEnabled(false);
	btnlastact->setEnabled(false);
	btnsaveact->setEnabled(false);
	btnsaveasact->setEnabled(false);
	btncopyact->setEnabled(false);
	btnpasteact->setEnabled(false);
	//btndebugact->setEnabled(false) ;
	btncloseact->setEnabled(false);
	btnfindact->setEnabled(false);
	btnreplaceact->setEnabled(false);
	btnbookmarkact->setEnabled(false);
	btnmarkact->setEnabled(false);
	btncutact->setEnabled(false);
	btnredoact->setEnabled(false);
	btnundoact->setEnabled(false);
	btncodeact->setEnabled(false);
	btnfly->setEnabled(false);
}

void CodeWindow::Rename(const QString &old, const QString &now)
{
	BkeDocBase* loli = docStrHash.value(LOLI_OS_QSTRING(old), nullptr);
	docStrHash.remove(LOLI_OS_QSTRING(old));
	docStrHash.insert(LOLI_OS_QSTRING(now), loli);
	if (loli)
	{
		SetCurrentEdit(loli->edit);
		loli->SetFileName(now);
		//BkeCreator::ReNameRecentFile(old, now);
		int pos = ItemTextList.indexOf(LOLI_OS_QSTRING(chopFileName(old)));
		filewidget->item(pos)->setText(chopFileName(now));
		lablelist->setItemText(pos, chopFileName(now));
	}
}

void CodeWindow::searchOneFile(const QString &file, const QString &searchstr, bool iscase, bool isregular, bool isword)
{
	if (!workpro)
		return;
	BkeDocBase* loli = docStrHash.value(LOLI_OS_QSTRING(file), nullptr);
	bool close = !loli;
	if (!loli)
	{
		loli = new BkeDocBase();
		loli->SetFileName(file);
		if (!loli->File()->exists())
		{
			delete loli;
			return;
		}
		QString en;
		if (!LOLI::AutoRead(en, loli->File()))
		{
			delete loli;
			return;
		}
		loli->SetProjectDir(prowin->workpro->ProjectDir());
		//新的编辑窗口
		QDir d(loli->ProjectDir());
		QString shortname = d.relativeFilePath(file);
		loli->edit = new BkeScintilla(this);
		loli->edit->basedoc = loli;
		loli->edit->setText(en);
		loli->edit->setModified(false);
		if (shortname.startsWith("..") || shortname[1] == ':')
			loli->edit->FileName = file;
		else
			loli->edit->FileName = shortname;
	}
	loli->edit->findFirst1(searchstr, iscase, isregular, isword);
	for (auto &&p : loli->edit->testlist)
	{
		int line = loli->edit->SendScintilla(QsciScintilla::SCI_LINEFROMPOSITION, p.Start());
		int linestart = loli->edit->SendScintilla(QsciScintilla::SCI_POSITIONFROMLINE, line);
		int linelen = loli->edit->SendScintilla(QsciScintilla::SCI_LINELENGTH, line);
		auto *buf = new char[linelen + 1];
		loli->edit->SendScintilla(QsciScintilla::SCI_GETLINE, line, buf);
		buf[linelen] = 0;
		linelen--;
		while (buf[linelen] == '\r' || buf[linelen] == '\n')
		{
			buf[linelen--] = 0;
		}
		int offset = p.Start() - linestart;
		//实际行号从1开始
		othwin->AddSearchItem(loli->edit->FileName + QString("(%1, %2)").arg(line + 1).arg(offset) + ":\t" + buf);
		emit searchOne(loli->edit->FileName, loli->FullName(), line, p.Start(), p.End());
		delete[] buf;
	}
	if (close)
	{
		loli->edit->close();
		loli->edit->deleteLater();
		delete loli;
	}
}

void CodeWindow::replaceOneFile(const QString &file, const QString &searchstr, const QString &replacestr, bool iscase, bool isregular, bool isword, bool stayopen)
{
	if (!workpro)
		return;
	BkeDocBase* loli = docStrHash.value(LOLI_OS_QSTRING(file), nullptr);
	bool close = !loli;
	if (!loli)
	{
		loli = new BkeDocBase();
		loli->SetFileName(file);
		if (!loli->File()->exists())
		{
			delete loli;
			return;
		}
		QString en;
		if (!LOLI::AutoRead(en, loli->File()))
		{
			delete loli;
			return;
		}
		loli->SetProjectDir(prowin->workpro->ProjectDir());
		//新的编辑窗口
		QDir d(loli->ProjectDir());
		QString shortname = d.relativeFilePath(file);
		loli->edit = new BkeScintilla(this);
		loli->edit->basedoc = loli;
		loli->edit->setText(en);
		loli->edit->setModified(false);
		if (shortname.startsWith("..") || shortname[1] == ':')
			loli->edit->FileName = file;
		else
			loli->edit->FileName = shortname;
	}
	loli->edit->ReplaceAllText(searchstr, replacestr, iscase, isregular, isword);
	if (!loli->edit->testlist.empty())
	{
		bool m = loli->edit->isModified();
		if (!stayopen && close)
		{
			LOLI::AutoWrite(loli->File(), loli->edit->text(), "UTF-8");
			loli->edit->close();
			loli->edit->deleteLater();
			delete loli;
		}
		else
		{
			//加到各种列表里去
			//BkeCreator::AddRecentFile(loli->FullName());
			//为了恢复当前文档的currentpos
			loli->edit->installEventFilter(this);
			QString curopenfile = ItemTextList.at(currentpos);
			int pos = LOLI_SORT_INSERT(ItemTextList, loli->Name() + "*");
			currentpos = ItemTextList.indexOf(curopenfile);
			filewidget->insertItem(pos, loli->Name() + "*");
			lablelist->insertItem(pos, loli->Name() + "*");
			stackwidget->insertWidget(pos, loli->edit);
			docStrHash[LOLI_OS_QSTRING(loli->FullName())] = loli;
			docWidgetHash[loli->edit] = loli;
			//文件被改变
			connect(loli->edit, SIGNAL(modificationChanged(bool)), this, SLOT(DocChange(bool)));
			//右键菜单
			connect(loli->edit, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowRmenu(QPoint)));
			//文件有改动
			connect(loli->edit, SIGNAL(textChanged()), diasearch, SLOT(onDocChanged()));
			connect(loli->edit, SIGNAL(selectionChanged()), diasearch, SLOT(onSelectionChanged()));
		}
	}
}

/*
class SearchThread :public QThread
{
public:
CodeWindow *cw;

virtual void run()
{
QStringList ls;
ls.append(cw->currentproject->ListFiles(1));
ls.append(cw->currentproject->ListFiles(2));
QString base = cw->currentproject->FileDir();
if (ls.empty())
return;
WaitWindow *w = new WaitWindow();
w->setInfo("正在工程中搜索", ls.count());
w->show();
w->update();
for (auto &&it : ls)
{
cw->searchOneFile(base + it, searchstr, iscase, isregular, isword);
w->addNum();
}
}
};
*/

void CodeWindow::searchAllFile(const QString &searchstr, bool iscase, bool isregular, bool isword)
{
	if (!workpro)
	{
		return;
	}
	QStringList ls;
	ls.append(workpro->ListFiles(1));
	ls.append(workpro->ListFiles(2));
	QString base = workpro->ProjectDir();
	if (ls.empty())
		return;
	//WaitWindow *w = new WaitWindow();
	//w->setInfo("正在工程中搜索", ls.count());
	//w->show();
	//w->update();
	for (auto &&it : ls)
	{
		searchOneFile(base + it, searchstr, iscase, isregular, isword);
		//w->addNum();
	}
	//w->hide();
	//delete w;
}

void CodeWindow::replaceAllFile(const QString &searchstr, const QString &replacestr, bool iscase, bool isregular, bool isword, bool stayopen)
{
	if (!workpro)
	{
		return;
	}
	QStringList ls;
	ls.append(workpro->ListFiles(1));
	ls.append(workpro->ListFiles(2));
	QString base = workpro->ProjectDir();
	for (auto &&it : ls)
	{
		replaceOneFile(base + '/' + it, searchstr, replacestr, iscase, isregular, isword, stayopen);
	}
}

//打开文件，文件列表是自动维护的
void CodeWindow::AddFile(const QString &file)
{
	BkeDocBase* loli = docStrHash.value(LOLI_OS_QSTRING(file), nullptr);

	//如果为空，则创建
	if (loli == nullptr){
		loli = new BkeDocBase;
		loli->SetFileName(file);
		if (!loli->File()->exists()){
			int i = QMessageBox::warning(this, "打开错误", "文件：\r\n  " + file + " 不存在！", QMessageBox::Ok | QMessageBox::Ignore);
			if (i == QMessageBox::Ok) return;
		}

		QString en;
		if (!LOLI::AutoRead(en, loli->File())){
			QMessageBox::critical(this, "打开错误", "文件：\r\n  " + file + " 读取失败！", QMessageBox::Ok);
			return;
		}

		loli->SetProjectDir(workpro->ProjectDir());
		//新的编辑窗口
		simpleNew(loli, en);
		loli->edit->FileName = workpro->AllNameToName(file);
		//BkeProject *tpro = prowin->FindProjectFromDir(prodir);
		//if (prodir != 0) loli->edit->setParser(tpro->lex);

		//BkeCreator::AddRecentFile(loli->FullName());

		//添加文件监视
		filewatcher->addPath(loli->FullName());
		//        bool ks = filewatcher->addPath(loli->FullName()) ;
		//        ks = false ;
	}

	//从当前文档的附近路径中寻找工程，失败返回0
	//ChangeProject(prowin->FindProjectFromDir(loli->ProjectDir()));
	//改变当前显示项
	SetCurrentEdit(loli->edit);
	//QfileChange("");
}

void CodeWindow::ConnectAutoComplete(BkeScintilla *edit)
{
	aclist->SetFont(edit->deflex->font(0));
	connect(edit, &BkeScintilla::AutoCompleteStart, edit, [this, edit](auto v1, auto v2) {
		aclist->Start(v1, v2, edit->mapToGlobal(edit->PointByPosition(edit->GetCurrentPosition() - v2.toUtf8().length())));
	});
	connect(edit, &BkeScintilla::AutoCompleteCancel, aclist, &AutoCompleteList::Cancel);
	connect(edit, &BkeScintilla::AutoCompleteMatch, aclist, &AutoCompleteList::Match);
	connect(aclist, &AutoCompleteList::OnCanceled, edit, &BkeScintilla::OnAutoCompleteCanceled);
	connect(aclist, &AutoCompleteList::OnSelected, edit, &BkeScintilla::OnAutoCompleteSelected);
	connect(aclist, &AutoCompleteList::RequestRestart, edit, [edit]() {
		edit->UpdateAutoComplete();
	});
}

void CodeWindow::DisconnectAutoComplete(BkeScintilla *edit)
{
	aclist->disconnect(edit);
	edit->disconnect(aclist);
}

void CodeWindow::simpleNew(BkeDocBase *loli, const QString &t)
{
	ignoreflag = true; //忽略改变，在所有准备工作完成以后才改变

	loli->edit = new BkeScintilla(this);
	loli->edit->ChangeIgnore++;
	loli->edit->installEventFilter(this);
	if (workpro)
		loli->edit->analysis = workpro->analysis;
	loli->edit->workpro = workpro;
	loli->edit->basedoc = loli;
	int pos = LOLI_SORT_INSERT(ItemTextList, loli->Name());
	filewidget->insertItem(pos, loli->Name());
	lablelist->insertItem(pos, loli->Name());
	stackwidget->insertWidget(pos, loli->edit);
	docStrHash[LOLI_OS_QSTRING(loli->FullName())] = loli;
	docWidgetHash[loli->edit] = loli;

	//文字
	loli->edit->setText(t);
	loli->edit->setModified(false);  //没有改变

	//文件被改变
	connect(loli->edit, SIGNAL(modificationChanged(bool)), this, SLOT(DocChange(bool)));
	//右键菜单
	connect(loli->edit, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ShowRmenu(QPoint)));
	//文件有改动
	connect(loli->edit, SIGNAL(textChanged()), diasearch, SLOT(onDocChanged()));
	connect(loli->edit, SIGNAL(selectionChanged()), diasearch, SLOT(onSelectionChanged()));
	loli->edit->ChangeIgnore--;
	ignoreflag = false;
}

//文档被改变，标题加*
void CodeWindow::DocChange(bool m)
{
	if (currentpos > lablelist->count() - 1 || currentpos < 0) return;
	QString title = ItemTextList.at(currentpos);

	if (m){
		title.append('*');
		lablelist->setItemText(currentpos, title);
		filewidget->item(currentpos)->setText(title);
		btnsaveact->setEnabled(true);
	}
	else{
		lablelist->setItemText(currentpos, title);
		filewidget->item(currentpos)->setText(title);
		btnsaveact->setEnabled(false);
	}
}

void CodeWindow::NavigateTo(const QPair<QString, int> &target)
{
	for (int i = 0; i < ItemTextList.size(); i++)
	{
		if (ItemTextList[i] == target.first)
		{
			navigationLocker++;
			SetCurrentEdit(i);
			currentedit->SetCurrentPosition(target.second);
			navigationLocker--;
		}
	}
}

void CodeWindow::NextNavigation()
{
	if (currentNavigation < navigationList.size() - 1)
	{
		QPair<QString, int> target = navigationList[++currentNavigation];
		NavigateTo(target);
		RefreshNavigation();
	}
}

void CodeWindow::LastNavigation()
{
	if (currentNavigation > 0)
	{
		QPair<QString, int> target = navigationList[--currentNavigation];
		NavigateTo(target);
		RefreshNavigation();
	}
}

void CodeWindow::SaveALL()
{
	BkeDocBase *llm;
	for (int i = 0; i < stackwidget->count(); i++){
		llm = docWidgetHash.value(stackwidget->widget(i));
		if (llm == NULL) continue;
		else simpleSave(llm);
	}
}

void CodeWindow::backupAll()
{
	static QDir qd;
	auto userdir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/.BKE_Creator/";
	if (!qd.exists(userdir))
		qd.mkdir(userdir);
	static QJsonObject jo;
	static QStringList files;
	static QJsonDocument jd;
	if (workpro)
		jo.insert("project", workpro->ProjectFile());
	files.clear();
	BkeDocBase *llm;
	for (int i = 0; i < stackwidget->count(); i++){
		llm = docWidgetHash.value(stackwidget->widget(i));
		if (llm == NULL)
			continue;
		else
		{
			if (workpro)
				files.push_back(workpro->ProjectDir() + llm->edit->FileName);
			simpleBackup(llm);
		}
	}
	jo.insert("files", QJsonArray::fromStringList(files));
	jd.setObject(jo);
	LOLI::AutoWrite(userdir + "pro", jd.toJson());
}

void CodeWindow::SaveFile()
{
	simpleSave(currentbase);
}

bool CodeWindow::simpleBackup(BkeDocBase *loli)
{
	auto userdir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/.BKE_Creator/";
	QFileInfo fi(loli->FullName());
	return LOLI::AutoWrite(userdir + fi.fileName(), loli->edit->text());
}

void CodeWindow::simpleSave(BkeDocBase *loli)
{
	filewatcher->removePath(loli->FullName());
	if (loli->FullName() == "New"){
		QString temp = QFileDialog::getSaveFileName(this, "保存文件");
		if (temp.isEmpty()) return;
		else if (!temp.endsWith(".bkscr") && !temp.endsWith(".bkpsr"))
			temp.append(".bkscr");
		loli->SetFileName(temp);
	}

	if (!loli->edit->isModified())
		return;

	//delete bkbin
	QFileInfo finfo(*loli->File());
	QDir bin(finfo.filePath());
	bin.remove(finfo.completeBaseName() + ".bkbin");

	if (!LOLI::AutoWrite(loli->File(), loli->edit->text(), "UTF-8")){
		QMessageBox::critical(this, "", "文件：\r\n" + currentbase->FullName() + " 写出失败！");
		return;
	}
	loli->edit->setModified(false);
	//更新文件被改写的时间
	loli->upFileTime();
	filewatcher->addPath(loli->FullName());
	//同时更新自动备份
	simpleBackup(loli);
}

//另存为
void CodeWindow::SaveAs()
{
	QString name = QFileDialog::getSaveFileName(this, "另存为", "", "bkscr脚本(*.bkscr)");
	if (name.isEmpty()) return;
	else if (!name.endsWith(".bkscr")) name.append(".bkscr");

	//改变文件监视
	currentbase->SetFileName(name);
	simpleSave(currentbase);
}

void CodeWindow::CloseFile()
{
	if (currentedit->isModified()){
		auto *msg = new QMessageBox(this);
		msg->addButton("保存", QMessageBox::AcceptRole);
		msg->addButton("关闭", QMessageBox::RejectRole);
		msg->addButton("取消", QMessageBox::DestructiveRole);

		msg->setText(currentbase->FullName() + "\r\n已经被修改，是否保存？");
		int i = msg->exec();
		if (i == QMessageBox::AcceptRole)
			SaveFile();
		else if (i == QMessageBox::DestructiveRole)
			return;
	}

	simpleClose(currentbase);
}

void CodeWindow::CloseFile(int pos)
{
	if (pos < 0 || pos > ItemTextList.size()) return;
	BkeDocBase *llm = docStrHash.value(LOLI_OS_QSTRING(ItemTextList.at(pos)));
	simpleClose(llm);
}

bool CodeWindow::CloseAll()
{
	QList<BkeDocBase*> ls = docWidgetHash.values();

	for (auto ptr : ls){
		if (ptr->edit->isModified()){
			SetCurrentEdit(ptr->edit);
			auto btn = QMessageBox::information(this, "", "文件:\r\n" + ptr->FullName() + "\r\n已经修改，是否保存？", QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			if (btn == QMessageBox::Yes)
				simpleSave(ptr);
			else if (btn == QMessageBox::No)
				simpleClose(ptr);
			else
				return false;
		}
		else{
			simpleClose(ptr);
		}
	}
	return true;
}

QSize CodeWindow::sizeHint() const
{
	auto fff = ((QWidget *)this->parent())->size();
	QSize hint;
	hint.setWidth(fff.width() * 0.8);
	hint.setHeight(fff.height() - 50);
	return hint;
}



void CodeWindow::TextInsert(const QString &text)
{
	if (stackwidget->count() < 1) return;
	if (!text.isEmpty() && (text.left(1) == "\\" || text.left(1) == "/"))
		currentedit->insert("\"" + text.mid(1) + "\"");
	else
		currentedit->insert("\"" + text + "\"");
}


void CodeWindow::ImportBeChange(const QString &text, int type)
{

}

//文件将被删除
void CodeWindow::FileWillBeDel(const QString &file)
{
}

void CodeWindow::FileIOclose(const QStringList &list)
{
	QStringList ls;
	BkeDocBase *llm;
	QList<BkeDocBase*> doclist;
	for (int i = 0; i < list.size(); i++){
		llm = docStrHash.value(list.at(i), nullptr);
		//已经被打开的文件
		if (llm != nullptr){
			ls.append(list.at(i));
			doclist.append(llm);
		}
	}
}


void CodeWindow::Compile()
{
	LableSureDialog abc(this);
	abc.SetLable("标记是通过在main.bkscr中的第一个@jump插入一个选项实现的，\r\n为保证标记正确启用，建议在main.bkscr中只进行变量初始化工作。");
	abc.SetCheckbox(QStringList() << "运行" << "启用标记");
	if (currentedit->hasSelectedText()) abc.SetBtn(QStringList() << "编译全部脚本" << "编译当前脚本" << "编译选择部分");
	else abc.SetBtn(QStringList() << "编译全部脚本" << "编译当前脚本");
	int i = abc.WaitUser(330, 200);

	if (i == 0)
	{
		SaveALL();
		CompileAll();
	}
}

void CodeWindow::CompileLang(bool release /*= false*/)
{
	LOLI_CLEAR_TEMP(BKE_CURRENT_DIR + "/temp");
	//当前编辑项不是工程的话，什么也不会发生
	if (workpro == nullptr) return;

	//设置按钮
	btncompileact->setEnabled(false);
	btncompilerunact->setEnabled(false);
	btnrunact->setEnabled(false);
	////清理上次编译的工程
	//deleteCompileFile(ComList, cosdir);

	////拷贝文件
	//if( !WriteOpenFile(currentproject->FileDir()) ){
	//    btncompileact->setEnabled(true);
	//    btncompilerunact->setEnabled(true);
	//    btnrunact->setEnabled(true);
	//    return ;
	//}
	//QStringList ls = ListDirsCopy(ComList,cosdir,BKE_CURRENT_DIR+"/temp") ;

	//if( ls.size() > 0){
	//    QMessageBox msg ;
	//    msg.setText("以下文件复制失败：\r\n" + ls.join("\r\n") );
	//    msg.addButton(QMessageBox::Ok);
	//    msg.exec() ;
	//    return ;
	//}
	QStringList ls = workpro->AllScriptFiles();
	kag->setMaximum(ls.size());
	kag->setValue(0);
	kag->show();
	//开始编译
	//comtool.Compile(BKE_CURRENT_DIR+"/temp");
	SaveALL();
	comtool.CompileLang(workpro->ProjectDir(), release);
}

void CodeWindow::CompileLang()
{
	CompileLang(false);
}

void CodeWindow::CompileAll(bool release /*= false*/)
{
	LOLI_CLEAR_TEMP(BKE_CURRENT_DIR + "/temp");
	//当前编辑项不是工程的话，什么也不会发生
	if (workpro == nullptr) return;

	markadmin.ClearRuntimeProblems();
	if (currentedit)
		CheckRuntimeProblemMarks(currentedit, nullptr);
	othwin->SetRuntimeProblemList(nullptr, workpro->ProjectDir());

	//设置按钮
	btncompileact->setEnabled(false);
	btncompilerunact->setEnabled(false);
	btnrunact->setEnabled(false);
	
	QStringList ls = workpro->AllScriptFiles();
	kag->setMaximum(ls.size());
	kag->setValue(0);
	kag->show();
	//开始编译
	//comtool.Compile(BKE_CURRENT_DIR+"/temp");
	SaveALL();
	comtool.Compile(workpro->ProjectDir(), release);
}

void CodeWindow::CompileAll()
{
	CompileAll(false);
}

bool CodeWindow::WriteOpenFile(const QString &dir)
{
	//总是不保存
	if (isCompileNotice == _NOTICE_NOSAVE) return true;

	QList<BkeDocBase*> ls = docStrHash.values();
	QList<BkeDocBase*> ks;
	QStringList ns;

	for (auto l : ls){
		if (LOLI_OS_QSTRING(l->ProjectDir()) != LOLI_OS_QSTRING(dir)) continue;
		if (!l->edit->isModified()) continue;
		ks.append(l);
		ns.append(l->Name());
	}

	if (ns.empty()) return true;

	if (isCompileNotice == _NOTICE_ALWAYS){

		LableSureDialog msg;
		msg.SetLable("以下文件已经修改，是否保存？");
		msg.SetBtn(QStringList() << "取消" << "不要保存" << "全部保存");
		msg.SetCheckbox(QStringList() << "不再提示");
		auto *tls = new QListWidget(&msg);
		tls->addItems(ns);
		msg.SetCenterWidget(tls);
		msg.SetDefaultBtn(2);
		int k = msg.WaitUser();

		if (msg.IsCheckboxChoise(0) && k == 1) isCompileNotice = _NOTICE_NOSAVE;
		if (msg.IsCheckboxChoise(0) && k == 2) isCompileNotice = _NOTICE_ALLSAVE;

		if (k == 0) return false;
		else if (k == 1) return true;
		else if (k != 2) return true;

	}

	for (auto k : ks) simpleSave(k);

	return true;
}

/*
//拷贝编译过的脚本
void CodeWindow::copyCompileFile(QStringList &list)
{
QString fn ,ft ;
QString pna ;
QFile temp ;
for( int i = 0 ; i < list.size() ; i++){
pna = list.at(i) ;
if( pna.endsWith(".bkscr") ) pna = pna.left(pna.length() - 6 ) + ".bkbin" ;
fn = BKE_CURRENT_DIR + "/temp/"+pna ;
ft = cosdir + "/" + pna ;

if( fn.endsWith(".bkbin") ) {
temp.setFileName( fn );
temp.copy(ft) ;
}
}
}*/

QStringList fileEntries(const QString &dir, const QStringList &suffixes)
{
	QStringList l;
	QDir d(dir);
	QFileInfoList a = d.entryInfoList();
	for (QFileInfo &info : a)
	{
		if (info.isDir())
		{
			if (info.fileName() != ".." && info.fileName() != ".")
				l.append(fileEntries(info.absoluteFilePath(), suffixes));
		}
		else
		{
			for (auto s : suffixes)
			{
				if (info.fileName().endsWith(s))
				{
					l.append(info.absoluteFilePath());
				}
			}
		}
	}
	return l;
}

//删除编译过的文件
void CodeWindow::DeleteCompileFile()
{
	if (workpro == nullptr)
		return;
	QStringList l = fileEntries(workpro->ProjectDir(), QStringList() << ".bkbin");
	for (auto &&i : l)
	{
		QFile::remove(i);
	}
}

void CodeWindow::DeleteSaveData()
{
	if (workpro == nullptr)
		return;
	workpro->DeleteSaveData();
}

/*
bool CodeWindow::ReadyCompile(const QString &file)
{
//不复制觉得路径
if( file.indexOf("\\") >= 0) return true;
int s = currentproject->FileDir().length() ;

QFileInfo info( BKE_CURRENT_DIR + "/temp/" + file.right(file.length()- s - 1) ) ;
QDir abc(info.path()) ;
if( !abc.exists() ) abc.mkpath( info.path() ) ;  //创建路径

BkeDocBase *ak = docStrHash.value( LOLI_OS_QSTRING(file),0 ) ;
if( ak == 0 ){  //文件没有被打开，直接复制
QFile llm(file) ;
if( !llm.copy(info.filePath())) return false ;
else return true ;
}
else{  //已经打开则写到文件
return LOLI::AutoWrite(info.filePath(),ak->edit->text()) ;
}

}*/


void CodeWindow::CompileError(QString s)
{
	btncompileact->setEnabled(true);
	kag->setValue(kag->maximum());
	btncompilerunact->setEnabled(true);
	btnrunact->setEnabled(true);
	QTimer::singleShot(8 * 1000, kag, SLOT(reset())); //8秒之后隐藏
	isRun = false;
	QMessageBox::warning(this, "编译失败", "BKCompiler执行失败，错误：" + s + "\n\n1.请检查creator目录下tool/BKCompiler_Dev.exe是否存在，如不存在请重新下载本程序或者运行update.exe -fixup进行修复。\n2.请关闭关闭所有“杀毒软件”或者“安全卫士”。");
}

//编译完成
void CodeWindow::CompileFinish()
{
	btncompileact->setEnabled(true);
	kag->setValue(kag->maximum());

    QString comResult = comtool.Result();
    markadmin.ProblemsFromText(workpro->ProjectDir(), comResult);

	BkeMarkList *problemslist;
	if (stackwidget->count() > 0){
		//给当前文件标记错误信息
		BkeMarkList templs = *(markadmin.GetProblemMark(currentbase->FullName(), false));
		problemslist = markadmin.GetProblemMark(currentbase->FullName(), true);
		CheckProblemMarks(currentedit, &templs);
		currentedit->update();
	}
	else{
		problemslist = markadmin.GetProblemMark("", true);
	}

    othwin->SetCompileResult(comResult);
	othwin->SetProblemList(problemslist, workpro->ProjectDir());

	//按钮可用
	btncompileact->setEnabled(true);
	btncompilerunact->setEnabled(true);
	QTimer::singleShot(8 * 1000, kag, SLOT(reset())); //8秒之后隐藏
	if (markadmin.errorcount < 1)
		btnrunact->setEnabled(true);  //编译完成并且没有问题运行按钮才可用
	if (markadmin.errorcount < 1 && isRun)
		RunBKEWithArgs();
	emit CompileFinish(markadmin.errorcount);
	isRun = false;
}

void CodeWindow::CompileAndRun(const QStringList &extraArgs)
{
	isRun = true;
	BKE_extraArgs = extraArgs;
	CompileAll();
}

void CodeWindow::FileNameChange(const QString &oldname, const QString &newname, bool &c)
{

}

//转到文件
void CodeWindow::ToLocation(BkeMarkerBase *p, const QString &prodir)
{
	AddFile(p->FullName);
	//    BKEproject *pro = prowin->FindFileProject(p->FullName) ;
	//    if( pro == 0){
	//        addFile(p->FullName,0);
	//    }
	//    else  addFile(p->FullName,pro->FileDir());
	currentedit->setFirstVisibleDocumentLine(p->Atpos >= 1 ? p->Atpos - 1 : 0);

	//if (p->Atpos > 1)
	//	currentedit->setFirstVisibleLine(p->Atpos - 1);
	//else
	//	currentedit->setFirstVisibleLine(p->Atpos);

	if (p->Type == BkeMarkSupport::BKE_MARK_SEARCH)
	{
		BkeIndicatorBase in;
		in.SetStart(p->start);
		in.SetEnd(p->end);
		//currentedit->setSelection(in);
		currentedit->SetIndicator(BkeScintilla::BKE_INDICATOR_FIND, in);
	}
}

void CodeWindow::ShowRmenu(const QPoint & pos)
{
	QMenu menu(this);
	//在文件内容上右键菜单
	int n_pos = currentedit->SendScintilla(BkeScintilla::SCI_POSITIONFROMPOINT, pos.x(), pos.y());

	auto node = currentedit->analysis->findNode(currentedit->FileName, n_pos);

	if (node)
	{
		if (node->isCommand())
		{
			QString name = node->name;
			BKEMacros m;
			bool f = currentedit->analysis->findMacro(name, &m);
			if (f)
			{
				QString d;
				d.setNum(m.pos);
				jumpToDef->setText("转到宏" + m.name + "的声明");
				jumpToDef->setData(m.definefile + "|" + d);

				jumpToCode->setText("转到宏" + m.name + "的实现");
				jumpToCode->setData(m.definefile + "|" + m.name);

				menu.addAction(jumpToDef);
				menu.addAction(jumpToCode);
			}
		}
	}

	menu.addAction(btncopyact);
	menu.addAction(btncutact);
	menu.addAction(btnpasteact);
	menu.addSeparator();
	menu.addAction(btnfindact);
	menu.addAction(btnreplaceact);
	menu.addAction(btnfly);
	menu.addSeparator();
	menu.addAction(pannote);
	menu.addAction(btnbookmarkact);
	menu.addAction(btnmarkact);

	auto labelnode = currentedit->analysis->findLastLabelNode(currentedit->FileName, n_pos);

	if (labelnode)
	{
		BKE_extraArgs.clear();
		BKE_extraArgs << "-startscript" << currentedit->FileName << "-startlabel" << "*" + labelnode->name;
		menu.addAction(btnrunfromlabel);
	}

	QPoint temp = QCursor::pos();
	temp.setX(temp.x() + 10);
	QAction *result = menu.exec(temp);

}

//添加书签
void CodeWindow::AddBookMark()
{
	QString info = QInputDialog::getText(this, "新建书签", "输入书签的标记信息，\r\n如果为空，书签不会创建");
	if (info.isEmpty()) return;

	int line = currentedit->GetCurrentLine();
	if (currentbase != nullptr){
		//markadmin.AddBookMark(info, line ,BkeFullnameToName(currentbase->fullname,currentproject->FileDir()) );
		workpro->WriteMarkFile(&markadmin);
	}
	else{
		markadmin.AddBookMark(info, line, "");
	}

	currentedit->markerAdd(line, 3);
	othwin->RefreshBookMark(markadmin.GetBookMark(currentbase->FullName(), true));
}

//更新所有标记信息
void CodeWindow::AddMarksToEdit()
{
	CheckBookMarks(currentedit, markadmin.GetBookMark(currentbase->FullName(), false));
	CheckMarks(currentedit, markadmin.GetMarks(currentbase->FullName(), false));
	CheckProblemMarks(currentedit, markadmin.GetProblemMark(currentbase->FullName(), false));
	CheckRuntimeProblemMarks(currentedit, markadmin.GetRuntimeProblemMark(currentbase->FullName(), false));

	currentedit->update();
}

//标记更新完后使用 updata
void CodeWindow::CheckProblemMarks(BkeScintilla *edit, BkeMarkList *list)
{
	edit->clearAnnotations(BkeScintilla::PROBLEM);
	edit->markerDeleteAll(1);
	edit->markerDeleteAll(2);

	if (list->isEmpty()) return;

	BkeMarkerBase *abc;
	QString info;

	for (auto i : *list){
		abc = i;
		info = edit->annotation(abc->Atpos - 1);

		if (info.isEmpty()) info = abc->Information;
		else info += "\n\n" + abc->Information;

		if (abc->Type == 1){
			edit->annotate(abc->Atpos - 1, info, ks1, BkeScintilla::PROBLEM);
			edit->markerAdd(abc->Atpos - 1, 1);
		}
		else{
			edit->annotate(abc->Atpos - 1, info, ks2, BkeScintilla::PROBLEM);
			edit->markerAdd(abc->Atpos - 1, 2);
		}
	}
}

void CodeWindow::CheckBookMarks(BkeScintilla *edit, BkeMarkList *list)
{
	edit->markerDeleteAll(3);

	if (list->isEmpty()) return;

	BkeMarkerBase *abc;
	for (auto i : *list){
		abc = i;
		edit->markerAdd(abc->Atpos, 3);
	}
}

void CodeWindow::CheckMarks(BkeScintilla *edit, BkeMarkList *list)
{
	edit->markerDeleteAll(4);

	if (list->isEmpty()) return;

	BkeMarkerBase *abc;
	for (auto i : *list){
		abc = i;
		edit->markerAdd(abc->Atpos, 4);
	}
}

//标记更新完后使用 updata
void CodeWindow::CheckRuntimeProblemMarks(BkeScintilla *edit, BkeMarkList *list)
{
	edit->clearAnnotations(BkeScintilla::RUNTIME_PROBLEM);
	edit->markerDeleteAll(5);
	edit->markerDeleteAll(6);

	if (list == nullptr || list->isEmpty()) return;

	for (auto i : *list) {
		BkeMarkerBase *abc = i;
		if(abc->Atpos > 0)
		{
			QString info = edit->annotation(abc->Atpos - 1);

			if (info.isEmpty()) info = abc->Information;
			else info += "\n\n" + abc->Information;
			if (abc->Type == 1) {
				edit->annotate(abc->Atpos - 1, info, ks1, BkeScintilla::RUNTIME_PROBLEM);
				edit->markerAdd(abc->Atpos - 1, 5);
			}
			else {
				edit->annotate(abc->Atpos - 1, info, ks2, BkeScintilla::RUNTIME_PROBLEM);
				edit->markerAdd(abc->Atpos - 1, 6);
			}
		}
	}
}


static QString qt_create_commandline(const QString &program, const QStringList &arguments)
{
	QString args;
	if (!program.isEmpty()) {
		QString programName = program;
		if (!programName.startsWith(QLatin1Char('\"')) && !programName.endsWith(QLatin1Char('\"')) && programName.contains(QLatin1Char(' ')))
			programName = QLatin1Char('\"') + programName + QLatin1Char('\"');
		programName.replace(QLatin1Char('/'), QLatin1Char('\\'));

		// add the prgram as the first arg ... it works better
		args = programName + QLatin1Char(' ');
	}

	for (int i = 0; i < arguments.size(); ++i) {
		QString tmp = arguments.at(i);
		// Quotes are escaped and their preceding backslashes are doubled.
		tmp.replace(QRegExp(QLatin1String("(\\\\*)\"")), QLatin1String("\\1\\1\\\""));
		if (tmp.isEmpty() || tmp.contains(QLatin1Char(' ')) || tmp.contains(QLatin1Char('\t'))) {
			// The argument must not end with a \ since this would be interpreted
			// as escaping the quote -- rather put the \ behind the quote: e.g.
			// rather use "foo"\ than "foo\"
			int i = tmp.length();
			while (i > 0 && tmp.at(i - 1) == QLatin1Char('\\'))
				--i;
			tmp.insert(i, QLatin1Char('"'));
			tmp.prepend(QLatin1Char('"'));
		}
		args += QLatin1Char(' ') + tmp;
	}
	return args;
}

void CodeWindow::StartBKEProcess(const QStringList &args)
{
	if(bkeprocess)
	{
		bkeprocess->kill();
	}
	QString ndir;
#ifdef Q_OS_WIN
	ndir = BKE_CURRENT_DIR + "/tool/BKEngine_Dev.exe";
#elif defined(Q_OS_MAC)
	ndir = BKE_CURRENT_DIR + "/BKEngine_Dev.app";
#else
	ndir = BKE_CURRENT_DIR + "/tool/BKEngine_Dev";
#endif

	QString command;
	QString path;
#ifdef Q_OS_MAC
	command = qt_create_commandline("open", QStringList() << ndir << "--args" << workpro->ProjectDir() << args);
#else
	command = qt_create_commandline(ndir, args);
	path = workpro->ProjectDir();
#endif
#ifdef _WIN32
	bkeprocess = new TinyProcessLib::Process(command.toStdWString(), path.toStdWString());
#else
	bkeprocess = new TinyProcessLib::Process(command.toStdString(), path.toStdString());
#endif
	BKE_extraArgs.clear();
}

//运行BKEngine.exe
void CodeWindow::RunBKE()
{
	auto args = QStringList() << "-nologo" << BKE_extraArgs;
	if (envMobile)
		args << "-envmobile";
	StartBKEProcess(args);
}

void CodeWindow::RunBKEWithArgs()
{
	auto args = QStringList() << "-nologo" << BKE_extraArgs;
	if (envMobile)
		args << "-envmobile";
	StartBKEProcess(args);
}

void CodeWindow::AnnotateSelect()
{
	currentedit->BkeAnnotateSelect();
}

void CodeWindow::ClearCompile()
{
	DeleteCompileFile();
	btnrunact->setEnabled(false); //清理后运行按钮不可用
	btndebugact->setEnabled(false); //debug按钮也不可用
}

void CodeWindow::ClearCompileAndSaveData()
{
	DeleteCompileFile();
	DeleteSaveData();
	btnrunact->setEnabled(false); //清理后运行按钮不可用
	btndebugact->setEnabled(false); //debug按钮也不可用
}

void CodeWindow::ChangeCodec()
{
	QMessageBox msg;
	QComboBox akb(&msg);
	akb.setGeometry(65, 65, 118, 24);
	akb.addItems(QStringList() << "UTF-8" << "Big5" << "GB18030-0" << "JShift-JIS" << "UTF-16BE" << "UTF-16LE" << "UTF-32BE" << "UTF-32LE");
	msg.setWindowTitle("选择原文件编码");
	msg.resize(240, 300);
	msg.setText("当文件出现乱码时，请选择原文件编码，\r\n在选择正确的编码之前不要保存，\r\n否则会导致文件不可逆损坏！\r\n\r\n\r\n");
	msg.addButton(QMessageBox::Yes);
	if (msg.exec() == QMessageBox::Yes){
		QFile *f = currentbase->File();
		if (!f->isOpen() && !f->open(QFile::ReadOnly)) return;
		f->seek(0);
		QByteArray temp = f->readAll();
		QTextCodec *codec = QTextCodec::codecForName(akb.currentText().toLatin1());
		currentedit->setText(codec->toUnicode(temp));
	}
}

void CodeWindow::refreshLabel(BkeScintilla *sci)
{
	QSortedSet<QString> l;
	sci->analysis->getLabels(sci->FileName, l);
	refreshLabel(l);
}

void CodeWindow::refreshLabel(QSortedSet<QString> &l)
{
	slablelist->clear();
	labelbanned = true;
	for (auto &it : l)
	{
		slablelist->addItem("*" + it);
	}
	labelbanned = false;
}

void CodeWindow::SwitchFold()
{
	if (!currentedit)
		return;
	if (btnswitchfold->text() == "全部折叠")
	{
		btnswitchfold->setText("全部展开");
	}
	else
	{
		btnswitchfold->setText("全部折叠");
	}
	currentedit->foldAll(true);
}

void CodeWindow::FileReadyToCompile(int i)
{
	kag->setValue(i);
}

//当前工程被改变
void CodeWindow::ChangeProject(BkeProject *p)
{
	if (p != workpro)
	{
		navigationList.clear();
		currentNavigation = -1;

		if (p == nullptr)
		{
			btncompileact->setEnabled(false);  //编译按钮只有当工程出现时才可用
			btncompilerunact->setEnabled(false);
			btnrunact->setEnabled(false);
			btndebugact->setEnabled(false);
		}
		else 
		{
			btncompileact->setEnabled(true);  //工程出现后编译按钮都是可用的
			btncompilerunact->setEnabled(true);
		}

		workpro = p;
		debugServer->WorkproChanged(p);
	}
}

void CodeWindow::TextToMarks(const QString &text, const QString &dir, int type)
{
	if (type == 0){
		markadmin.BookMarksFromText(text, dir);
	}
	othwin->RefreshBookMark(markadmin.GetBookMark("", true));
}

void CodeWindow::NewEmptyFile()
{
	auto *llm = new BkeDocBase;
	llm->SetFileName("New");
	simpleNew(llm, "");
	llm->edit->FileName = "New";
	SetCurrentEdit(llm->edit);
}

void CodeWindow::SelectAll()
{
	if (stackwidget->count() < 1) return;
	currentedit->selectAll();
}

void CodeWindow::QfileChange(const QString &path)
{
	if (stackwidget->count() < 1) return;

	BkeDocBase *tempbase = currentbase;

	if (!path.isEmpty()){
		tempbase = docStrHash.value(LOLI_OS_QSTRING(path), nullptr);
		if (tempbase == nullptr) return;
		SetCurrentEdit(tempbase->edit);
	}

	if (!tempbase->isFileChange()) return;
	ignoreActive = true;

	QMessageBox msg(this);

	if (!tempbase->File()->exists()){
		msg.setText("文件：\r\n" + tempbase->Name() + "\r\n已经被从外部删除，是否重新保存？");
		msg.addButton(QMessageBox::Save);
		msg.addButton(QMessageBox::Close);
		msg.addButton(QMessageBox::Cancel);
		int i = msg.exec();
		if (i == QMessageBox::Close){
			//BkeProject *pro = prowin->FindProjectFromDir(tempbase->ProjectDir());
			BkeProject *pro = workpro;
			if (pro != nullptr) pro->RemoveItem(tempbase->FullName());
			simpleClose(tempbase);
		}
		else if (i == QMessageBox::Save)
			simpleSave(tempbase);
		return;
	}

	msg.setText("文件：\r\n" + tempbase->Name() + "\r\n已经被改变，是否重新载入？");
	msg.addButton(QMessageBox::Yes);
	msg.addButton(QMessageBox::No);
	if (msg.exec() == QMessageBox::No) return;

	if (tempbase->edit->isModified()){
		msg.setText("文件：\r\n" + tempbase->Name() + "\r\n已经修改，是否放弃修改？");
		msg.addButton(QMessageBox::Yes);
		msg.addButton(QMessageBox::No);
		if (msg.exec() == QMessageBox::No) SaveAs();
	}

	QString en;
	LOLI::AutoRead(en, tempbase->FullName());
	tempbase->edit->setText(en);
	tempbase->edit->setModified(false);
}

void CodeWindow::simpleClose(BkeDocBase *loli)
{
	int tpos = stackwidget->indexOf(loli->edit);
	if (tpos < 0) return;

	ignoreflag = true;
	lablelist->removeItem(tpos);
	filewidget->takeItem(tpos);
	ItemTextList.removeAt(tpos);
	stackwidget->removeWidget(loli->edit);
	docStrHash.remove(LOLI_OS_QSTRING(loli->FullName()));
	docWidgetHash.remove(loli->edit);
	//移除文件监视
	filewatcher->removePath(currentbase->FullName()) ;

	RemoveNavigation(loli->Name());

	loli->edit->close();
	loli->edit->analysis = NULL;
	loli->edit->deleteLater();
	lablelist->setCurrentIndex(-1);
	delete loli;
	ignoreflag = false;

	if (stackwidget->count() < 1){
		btnDisable();
		filewatcher->removePaths(filewatcher->files()) ;
		DrawLine(false);
		
	}
	SetCurrentEdit(stackwidget->currentIndex());
}

//跳转到指定行
void CodeWindow::GotoLine()
{
	if (stackwidget->count() < 1) return;

	bool ok;
	int i = QInputDialog::getInt(this, "跳转", "输入行号", 0, -1, 999999, 1, &ok);

	if (!ok) return;
	else if (i < 1) i = 1;
	else if (i > currentedit->lines()) i = currentedit->lines();
	currentedit->setFirstVisibleLine(i - 1);
}

void CodeWindow::GotoLabel(int i)
{
	if (i < 0 || labelbanned)
		return;
	//currentedit->setFirstVisibleLine(slablelist->itemData(i).toInt());
	QString l = slablelist->itemText(i);
	GotoLabel(l);
	//currentedit->SendScintilla(QsciScintilla::SCI_GOTOLINE, line);
}

void CodeWindow::GotoLabel(QString l)
{
	if (l.startsWith("*"))
	{
		l = l.right(l.length() - 1);
	}
	int pos = currentedit->analysis->findLabel(currentedit->FileName, l);
	if (pos < 0)
		return;
	int line, index;
	currentedit->lineIndexFromPositionByte(pos, &line, &index);
	currentedit->setFirstVisibleDocumentLine(line);
}

void CodeWindow::GotoOrCreateLabel(QString l)
{
	if (l.startsWith("*"))
	{
		l = l.right(l.length() - 1);
	}
	int pos = currentedit->analysis->findLabel(currentedit->FileName, l);
	if (pos < 0)
	{
		CreateLabel(l);
		return;
	}
	int line, index;
	currentedit->lineIndexFromPositionByte(pos, &line, &index);
	currentedit->setFirstVisibleLine(line);
}

void CodeWindow::CreateLabel(QString l)
{
	if (l.startsWith("*"))
	{
		l = l.right(l.length() - 1);
	}
	QString text = "\n*" + l + "\n\n[return]\n";
	int line = currentedit->lines();
	QByteArray data = currentedit->TextAsBytes(text);
	currentedit->AppendText(data);
	currentedit->SetCurrentPosition(currentedit->PositionByLine(line + 1));
}

void CodeWindow::GotoLabelList()
{
	if (currentedit == nullptr || currentedit->analysis == nullptr)
	{
		return;
	}
	QSortedSet<QString> qs;
	currentedit->analysis->getLabels(currentedit->FileName, qs);
	OpenLabelDialog *dialog = new OpenLabelDialog(qs, this);
	dialog->setModal(true);
	connect(dialog, &OpenLabelDialog::GotoLabel, this, (void(QObject::*)(QString))&CodeWindow::GotoLabel);
	dialog->show();
}

void CodeWindow::GotoFile()
{
	if (workpro == nullptr)
	{
		return;
	}
	QStringList qs = workpro->AllScriptFiles();
	auto *dialog = new GotoFileDialog(qs, this);
	dialog->setModal(true);
	connect(dialog, &GotoFileDialog::GotoFile, projectedit, &ProjectWindow::OpenProjectFile);
	dialog->show();
}

void CodeWindow::DrawLine(bool isClear)
{
	if (isClear){
		stackwidget->setStyleSheet("");
	}
	else{

		stackwidget->setStyleSheet("QWidget{border-left:1px solid #313131;border-top:1px solid #313131 }");
	}
}

//撤销
void CodeWindow::ActUndo()
{
	currentedit->undo();
}

//重做
void CodeWindow::ActRedo()
{
	currentedit->redo();
}

void CodeWindow::ActCurrentChange()
{
	btnundoact->setEnabled(currentedit->isUndoAvailable());
	btnredoact->setEnabled(currentedit->isRedoAvailable());
	calcWords(currentedit->text());
}

//剪切
void CodeWindow::ActCut()
{
	currentedit->cut();
}

//粘贴
void CodeWindow::ActPaste()
{
	currentedit->paste();
}

void CodeWindow::ActCopy()
{
	currentedit->copy();
}

void CodeWindow::jumpToDefFunc()
{
	auto *act = (QAction*)sender();
	QStringList ls = act->data().toString().split('|');
	int pos = ls[1].toInt();
	AddFile(workpro->ProjectDir() + ls[0]);
	if (currentedit->FileName != ls[0])
		return;
	int line, xpos;
	currentedit->lineIndexFromPositionByte(pos, &line, &xpos);
	currentedit->setFirstVisibleDocumentLine(line);
}

void CodeWindow::jumpToCodeFunc()
{
	auto *act = (QAction*)sender();
	QStringList ls = act->data().toString().split('|');
	AddFile(workpro->ProjectDir() + ls[0]);
	if (currentedit->FileName != ls[0])
		return;
	int pos = currentedit->analysis->findLabel(currentedit->FileName, ls[1]);
	int line, index;
	currentedit->lineIndexFromPositionByte(pos, &line, &index);
	currentedit->setFirstVisibleDocumentLine(line);
}

void CodeWindow::jumpToLabelFunc()
{

}

void CodeWindow::ShouldAddToNavigation()
{
	AddNavigation(currentbase->Name(), currentedit->GetCurrentPosition());
}

void CodeWindow::AddNavigation(const QString &file, int pos)
{
	if (navigationLocker)
		return;
	if (!navigationList.empty() && navigationList[currentNavigation].first == file && navigationList[currentNavigation].second == pos)
		return;
	if (currentNavigation != navigationList.size() - 1)
	{
		int c = navigationList.size() - currentNavigation - 1;
		while (c--)
		{
			navigationList.removeLast();
		}
	}
	navigationList.push_back({ file, pos });
	currentNavigation = navigationList.size() - 1;
	RefreshNavigation();
}

void CodeWindow::RemoveNavigation(const QString &file)
{
	int offset = 0;
	for (int i = 0; i < navigationList.size(); i++)
	{
		if (navigationList[i].first == file)
		{
			if (i <= currentNavigation)
			{
				offset++;
			}
			navigationList.removeAt(i);
			i--;
		}
	}
	currentNavigation -= offset;
	/*if (currentNavigation < 0 && navigationList.size())
	{
		currentNavigation = 0;
	}*/
	RefreshNavigation();
}

void CodeWindow::CreateAndGotoLabel(QString label)
{
	if (label.startsWith("*"))
		label = label.right(label.size() - 1);
	if (currentedit->analysis->findLabel(currentedit->FileName, label) < 0)
	{
		int line, index;
		currentedit->lineIndexFromPosition(currentedit->GetTextLength(), &line, &index);
		QString content1 = "\n*" + label + "\n";
		QString content2 = "\n[return]\n";
		currentedit->insertAt(content1, line, index);
		currentedit->SetCurrentPosition(currentedit->GetTextLength());
		currentedit->insert(content2);
	}
	else
	{
		GotoLabel(label);
	}
}

void CodeWindow::AutoFix()
{
	if (currentedit == nullptr)
		return;
	QMenu menu;
	QAction *firstAction = nullptr;
	//测试Label
	{
		BkeIndicatorBase base = currentedit->GetRangeForStyle(currentedit->GetCurrentPosition(), SCE_BKE_LABEL_IN_PARSER);
		if (base)
		{
			QString content = currentedit->TextForRange(base);
			QAction *action = menu.addAction(QIcon(":/auto/hint.png"), "跳转或创建标签");
			if (firstAction == nullptr)
				firstAction = action;
			connect(action, &QAction::triggered, [this, content]() {
				GotoOrCreateLabel(content);
			});
		}
	}
	if (firstAction != nullptr)
	{
		menu.setActiveAction(firstAction);
		QPoint pos = currentedit->mapToGlobal(currentedit->PointByPosition(currentedit->GetCurrentPosition()));
		menu.exec(pos + QPoint(0, 20));
	}
}

void CodeWindow::RefreshNavigation()
{
	btnlastact->setEnabled(currentNavigation > 0);
	btnnextact->setEnabled(currentNavigation < navigationList.size() - 1);
}

void CodeWindow::DebugLogReceived(int32_t level, QString log)
{
	if (!workpro)
		return;
	markadmin.AddRuntimeProblem(workpro->ProjectDir(), level, log);

	BkeMarkList *runtimeproblemslist;
	if (stackwidget->count() > 0) {
		//给当前文件标记错误信息
		BkeMarkList templs = *(markadmin.GetRuntimeProblemMark(currentbase->FullName(), false));
		runtimeproblemslist = markadmin.GetRuntimeProblemMark(currentbase->FullName(), true);
		CheckRuntimeProblemMarks(currentedit, &templs);
		currentedit->update();
	}
	else {
		runtimeproblemslist = markadmin.GetRuntimeProblemMark("", true);
	}

	othwin->SetRuntimeProblemList(runtimeproblemslist, workpro->ProjectDir());
}

bool CodeWindow::eventFilter(QObject * watched, QEvent *e)
{
	if (watched != currentedit)
		return false;
	if (e->type() == QEvent::KeyPress)
	{
		auto *event = (QKeyEvent *)e;
		if (aclist->isVisible() && aclist->OnKeyPress(event->key()))
		{
			return true;
		}
		if (event->key() == Qt::Key_Control && !event->isAutoRepeat())
		{
			enterClickGotoMode();
		}
	}
	else if (e->type() == QEvent::KeyRelease)
	{
		auto *event = (QKeyEvent *)e;
		if (event->key() == Qt::Key_Control && !event->isAutoRepeat())
		{
			leaveClickGotoMode();
		}
	}
	else if (e->type() == QEvent::HoverMove)
	{
		auto *event = (QHoverEvent *)e;
		if (event->modifiers() & Qt::ControlModifier)
		{
			onHoverMove(event->pos());
		}
	}
	else if (e->type() == QEvent::MouseButtonPress)
	{
		auto *event = (QMouseEvent *)e;
		if (event->button() == Qt::LeftButton)
		{
			if(aclist->isVisible())
				aclist->Cancel();
		}
	}
	return false;
}

void CodeWindow::enterClickGotoMode()
{
	if (clickGotoMode)
		return;
	clickGotoMode = true;
	//currentedit->setMouseTracking(true);
	currentedit->setAttribute(Qt::WA_Hover, true);
	onHoverMove(currentedit->mapFromGlobal(QCursor::pos()));
	QMouseEvent event(QEvent::MouseMove, QPointF(currentedit->viewport()->mapFromGlobal(QCursor::pos())), Qt::NoButton, nullptr, nullptr);
	QGuiApplication::sendEvent(currentedit->viewport(), &event);
}

void CodeWindow::leaveClickGotoMode()
{
	if (!clickGotoMode)
		return;
	clickGotoMode = false;
	currentedit->ClearIndicator(lastClickIndicatorType, lastClickIndicator);
	lastClickIndicator.Clear();
	lastClickIndicatorType = 0;
	currentedit->setAttribute(Qt::WA_Hover, false);
	QMouseEvent event(QEvent::MouseMove, QPointF(currentedit->viewport()->mapFromGlobal(QCursor::pos())), Qt::NoButton, nullptr, nullptr);
	QGuiApplication::sendEvent(currentedit->viewport(), &event);
}

void CodeWindow::setClickIndicator(const BkeIndicatorBase &indicator, int id)
{
	if (lastClickIndicator != indicator || id != lastClickIndicatorType)
	{
		currentedit->ClearIndicator(lastClickIndicatorType, lastClickIndicator);
		lastClickIndicator = indicator;
		lastClickIndicatorType = id;
		currentedit->SetIndicator(id, indicator);
	}
}

void CodeWindow::SetEnvMobile(bool enabled)
{
	envMobile = enabled;
}

void CodeWindow::onHoverMove(QPoint pos)
{
	int position = currentedit->PositionAt(pos);
	if (position < 0)
		return;
	BkeIndicatorBase indicator = currentedit->GetRangeForStyle(position, SCE_BKE_COMMAND);
	if (!indicator.IsNull())
	{
		if (currentedit->GetByte(indicator.Start()) == '@')
		{
			indicator.SetStart(indicator.Start() + 1);
		}
		QString cmd = currentedit->TextForRange(indicator);
		BKEMacros m;
		if (currentedit->analysis->findMacro(cmd, &m))
		{
			setClickIndicator(indicator, BkeScintilla::BKE_INDICATOR_CLICK_COMMAND);
			return;
		}
		goto end;
	}
	indicator = currentedit->GetRangeForStyle(position, SCE_BKE_COMMAND2);
	if (!indicator.IsNull())
	{
		if (currentedit->GetByte(indicator.Start()) == '[')
		{
			indicator.SetStart(indicator.Start() + 1);
		}
		else if (currentedit->GetByte(indicator.Start()) == ']')
		{
			return;
		}
		if (currentedit->GetByte(indicator.End() - 1) == ']')
		{
			indicator.SetEnd(indicator.End() - 1);
		}
		QString cmd = currentedit->TextForRange(indicator);
		BKEMacros m;
		if (currentedit->analysis->findMacro(cmd, &m))
		{
			setClickIndicator(indicator, BkeScintilla::BKE_INDICATOR_CLICK_COMMAND);
			return;
		}
		goto end;
	}
	indicator = currentedit->GetRangeForStyle(position, SCE_BKE_LABEL_IN_PARSER);
	if (!indicator.IsNull())
	{
		setClickIndicator(indicator, BkeScintilla::BKE_INDICATOR_CLICK_LABEL);
		return;
	}
end:
	setClickIndicator(BkeIndicatorBase(), 0);
}

void CodeWindow::indicatorReleased(int line, int index, Qt::KeyboardModifiers state)
{
	auto *edit = (BkeScintilla *)sender();
	auto pos = edit->positionFromLineIndex(line, index);
	if (lastClickIndicatorType != 0 && edit->IsIndicator(lastClickIndicatorType, pos))
	{
		QString content = edit->TextForRange(lastClickIndicator);
		QTimer::singleShot(0, [content, this, edit]() {
			switch (lastClickIndicatorType)
			{
				case BkeScintilla::BKE_INDICATOR_CLICK_COMMAND:
				{
					BKEMacros macro;
					if (edit->analysis->findMacro(content, &macro))
					{
						AddFile(workpro->ProjectDir() + macro.definefile);
						if (currentedit->FileName != macro.definefile)
							return;
						emit GotoLabel(macro.name);
					}
					break;
				}
				case BkeScintilla::BKE_INDICATOR_CLICK_LABEL:
				{
					emit GotoLabel(content);
					break;
				}
				default:
					break;
			}
		});
	}
}
