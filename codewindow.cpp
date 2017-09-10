#include <weh.h>
#include "codewindow.h"
#include "dia/lablesuredialog.h"
#include "dia/WaitWindow.h"
#include "dia/openlabeldialog.h"
#include "dia/gotofiledialog.h"

CodeWindow::CodeWindow(QWidget *parent)
	:QMainWindow(parent)
{
	diasearch = new SearchBox(this);
	addDockWidget(Qt::BottomDockWidgetArea, diasearch);
	currentedit = 0;
	labelbanned = false;

	QSize fff = parent->size();
	hint.setWidth(fff.width() * 0.8);
	hint.setHeight(fff.height() - 50);

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
	connect(btnreplaceact, SIGNAL(triggered()), diasearch, SLOT(ReplaceModel()));
	connect(diasearch, SIGNAL(searchOne(const QString &, const QString &, bool, bool, bool)), this, SLOT(searchOneFile(const QString &, const QString &, bool, bool, bool)));
	connect(diasearch, SIGNAL(searchAll(const QString &, bool, bool, bool)), this, SLOT(searchAllFile(const QString &, bool, bool, bool)));
	connect(diasearch, SIGNAL(replaceAll(const QString &, const QString &, bool, bool, bool, bool)), this, SLOT(replaceAllFile(const QString &, const QString &, bool, bool, bool, bool)));
	connect(&comtool, SIGNAL(CompliteFinish()), this, SLOT(CompileFinish()));
	connect(&comtool, SIGNAL(CompliteError(QString)), this, SLOT(CompileError(QString)));
	connect(btnrunact, SIGNAL(triggered()), this, SLOT(RunBKE()));
	connect(btnrunfromlabel, SIGNAL(triggered()), this, SLOT(RunBKEWithArgs()));
	connect(pannote, SIGNAL(triggered()), this, SLOT(AnnotateSelect()));
	connect(btnclearact, SIGNAL(triggered()), this, SLOT(ClearCompile()));
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
	ChangeProject(0);
	ignoreflag = false;
	isRun = false;
	isSearLable = false; //是否在查找标签，如果是，刷新文件队列
	ignoreActive = false;

	searchlablelater = 0;
	isCompileNotice = _NOTICE_ALWAYS;

	tm.setTimerType(Qt::TimerType::VeryCoarseTimer);
	connect(&tm, SIGNAL(timeout()), this, SLOT(onTimer()));
	float interval = BKE_CLOSE_SETTING->value("autosavetime", 1).toFloat();
	tm.start((int)(60000 * interval));
}

CodeWindow::~CodeWindow()
{
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

	btnlastact = new QAction(QIcon(":/cedit/source/btnlast.png"), "返回", this);
	btnnextact = new QAction(QIcon(":/cedit/source/btnnext.png"), "前进", this);
	btnsaveact = new QAction(QIcon(":/cedit/source/save.png"), "保存", this);
	btnsaveasact = new QAction(QIcon(":/cedit/source/saveas.png"), "另存为...", this);
	btncodeact = new QAction(QIcon(":/cedit/source/code.png"), "改变源文件的编码", this);
	btncopyact = new QAction(QIcon(":/cedit/source/copy.png"), "复制", this);
	btncutact = new QAction(QIcon(":/cedit/source/edit_cut.png"), "剪切", this);
	btnpasteact = new QAction(QIcon(":/cedit/source/paste.png"), "粘贴", this);
	btncompileact = new QAction(QIcon(":/cedit/source/compile.png"), "编译成bkbin脚本", this);
	btncompilelang = new QAction(QIcon(":/cedit/source/compile.png"), "编译并生成语言文件", this);
	btncompilerunact = new QAction(QIcon(":/cedit/source/compile_run.png"), "编译并运行", this);
	btnrunact = new QAction(QIcon(":/cedit/source/run.png"), "运行", this);
	btndebugact = new QAction(QIcon(":/cedit/source/debug.png"), "调试", this);
	btncloseact = new QAction(QIcon(":/cedit/source/close.png"), "关闭", this);
	btnfindact = new QAction(QIcon(":/cedit/source/find.png"), "查找", this);
	btnreplaceact = new QAction(QIcon(":/cedit/source/replace(2).png"), "替换", this);
	btnbookmarkact = new QAction(QIcon(":/cedit/source/Bookmark.png"), "添加书签", this);
	btnmarkact = new QAction(QIcon(":/cedit/source/pin.png"), "添加标记", this);
	btnrunfromlabel = new QAction("从本标签处运行", this);
	btnredoact = new QAction(QIcon(":/cedit/source/redo.png"), "重做", this);
	btnundoact = new QAction(QIcon(":/cedit/source/undo.png"), "撤销", this);
	btnclearact = new QAction(QIcon(":/cedit/source/clear.png"), "清理编译工程", this);
	pannote = new QAction("选中部分注释/反注释", this);
	btnselectall = new QAction("全选", this);
	btnfly = new QAction(QIcon(":/cedit/source/flay.png"), "转到行...", this);
	btngotolabellist = new QAction("转到标签", this);
	btngotofile = new QAction("转到文件", this);
	btnswitchfold = new QAction("全部折叠", this);

	//右键菜单
	jumpToDef = new QAction(this);
	jumpToCode = new QAction(this);
	gotoLabel = new QAction(this);

	connect(jumpToDef, SIGNAL(triggered()), this, SLOT(jumpToDefFunc()));
	connect(jumpToCode, SIGNAL(triggered()), this, SLOT(jumpToCodeFunc()));
	connect(gotoLabel, SIGNAL(triggered()), this, SLOT(jumpToLabelFunc()));
	connect(btngotolabellist, SIGNAL(triggered()), this, SLOT(GotoLabelList()));
	connect(btngotofile, SIGNAL(triggered()), this, SLOT(GotoFile()));


	btnsaveact->setShortcut(Qt::CTRL + Qt::Key_S);
	//btncopyact->setShortcut(Qt::CTRL + Qt::Key_C);
	//btnpasteact->setShortcut(Qt::CTRL + Qt::Key_V);
	//btncutact->setShortcut(Qt::CTRL + Qt::Key_X);
	//btnundoact->setShortcut(Qt::CTRL + Qt::Key_Z);
	//btnredoact->setShortcut(Qt::CTRL + Qt::Key_Y);
	btnfindact->setShortcut(Qt::CTRL + Qt::Key_F);
	btnreplaceact->setShortcut(Qt::CTRL + Qt::Key_H);
	btncompileact->setShortcut(Qt::CTRL + Qt::Key_B);
	//btnselectall->setShortcut(Qt::CTRL + Qt::Key_A);
	btnfly->setShortcut(Qt::CTRL + Qt::Key_G);
	btncompilerunact->setShortcut(Qt::CTRL + Qt::Key_R);
	btngotolabellist->setShortcut(Qt::CTRL + Qt::Key_L);
	btngotofile->setShortcut(Qt::CTRL + Qt::Key_P);
	btnswitchfold->setShortcut(Qt::CTRL + Qt::Key_M);

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

void CodeWindow::OtherWinOtherwin(OtherWindow *win)
{
	othwin = win;
	//定位文件
	connect(win, SIGNAL(Location(BkeMarkerBase*, QString)), this, SLOT(ToLocation(BkeMarkerBase*, QString)));
	connect(this, SIGNAL(searchOne(const QString &, const QString &, int, int, int)), othwin, SLOT(onSearchOne(const QString &, const QString &, int, int, int)));
}

void CodeWindow::OtherWinProject(ProjectWindow *p)
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

void CodeWindow::OtherwinFileList(BkeLeftFileWidget *flist)
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
	else if (pos == stackwidget->currentIndex() && pos == lablelist->currentIndex()
		&& pos == filewidget->currentRow()) return;

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

void calcWords(QString text, QLineEdit *edit)
{
	edit->setText("文本字数：" + QString::number(CodeWindow::getScenarioTextFromCode(text).remove(QRegExp("[\r\n]")).count()));
}

//改变正在编辑的文档，文件列表选项是同步改变的
void CodeWindow::ChangeCurrentEdit(int pos)
{
	if (!stackwidget->styleSheet().isEmpty()) DrawLine(true);

	currentpos = pos;  //当前位置
	if (currentedit == stackwidget->currentWidget()) return;

	currentpos = pos;  //当前位置
	btncopyact->setEnabled(false);
	

	//释放文档改变信号
	if (currentpos > 0) CurrentConnect(false);

	currentbase = docWidgetHash.value(stackwidget->currentWidget(), 0);
	if (currentbase == 0){
		QMessageBox::critical(this, "", "致命错误：没有找到匹配的QWidget！", QMessageBox::Ok);
		return;
	}

	//改变工程
	//ChangeProject(prowin->FindProjectFromDir(currentbase->ProjectDir()));
	if(currentedit)
		currentedit->saveTopLine();
	currentedit = currentbase->edit;
	//reset lexer
	currentedit->deflex->ReadConfig(currentedit->deflex->ConfigName());
	currentedit->setLexer(currentedit->deflex);
	//连接文档改变信号
	CurrentConnect(true);
	currentedit->clearAnnotations();
	currentedit->restoreTopLine();

	diasearch->SetSci(currentedit); //查找管理
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

	AddMarksToEdit();
	refreshLabel(currentedit);
	calcWords(currentedit->text(), this->othwin->lewords);
	//BackstageSearchLable(currentedit);

	// 导航
	AddNavigation(currentbase->Name(), currentedit->GetCurrentPosition());
}

//断开、连接当前文档信号
void CodeWindow::CurrentConnect(bool c)
{
	if (c){
		connect(currentedit, SIGNAL(copyAvailable(bool)), btncopyact, SLOT(setEnabled(bool)));
		connect(currentedit, SIGNAL(copyAvailable(bool)), btncutact, SLOT(setEnabled(bool)));
		//        connect(currentedit,SIGNAL(Undoready(bool)),btnundoact,SLOT(setEnabled(bool))) ;
		//        connect(currentedit,SIGNAL(Redoready(bool)),btnredoact,SLOT(setEnabled(bool))) ;
		//        connect(btncopyact,SIGNAL(triggered()),currentedit,SLOT(copy())) ;
		//        connect(btncutact,SIGNAL(triggered()),currentedit,SLOT(cut())) ;
		//        connect(btnpasteact,SIGNAL(triggered()),currentedit,SLOT(paste())) ;
		//        connect(btnredoact,SIGNAL(triggered()),currentedit,SLOT(redo())) ;
		//        connect(btnundoact,SIGNAL(triggered()),currentedit,SLOT(undo())) ;
		//工程被改变，需要从下层传递信号
		connect(currentedit, SIGNAL(textChanged()), this, SLOT(ActCurrentChange()));
		connect(currentedit, SIGNAL(refreshLabel(BkeScintilla *)), this, SLOT(refreshLabel(BkeScintilla *)));
		connect(currentedit, SIGNAL(refreshLabel(QStringList &)), this, SLOT(refreshLabel(QStringList &)));
		connect(currentedit, SIGNAL(ShouldAddToNavigation()), this, SLOT(ShouldAddToNavigation()));
	}
	else{
		disconnect(currentedit, SIGNAL(copyAvailable(bool)), btncopyact, SLOT(setEnabled(bool)));
		disconnect(currentedit, SIGNAL(copyAvailable(bool)), btncutact, SLOT(setEnabled(bool)));
		//        disconnect(currentedit,SIGNAL(Undoready(bool)),btnundoact,SLOT(setEnabled(bool))) ;
		//        disconnect(currentedit,SIGNAL(Redoready(bool)),btnredoact,SLOT(setEnabled(bool))) ;
		//        disconnect(btncopyact,SIGNAL(triggered()),currentedit,SLOT(copy())) ;
		//        disconnect(btncutact,SIGNAL(triggered()),currentedit,SLOT(cut())) ;
		//        disconnect(btnpasteact,SIGNAL(triggered()),currentedit,SLOT(paste())) ;
		//        disconnect(btnredoact,SIGNAL(triggered()),currentedit,SLOT(redo())) ;
		//        disconnect(btnundoact,SIGNAL(triggered()),currentedit,SLOT(undo())) ;
		disconnect(currentedit, SIGNAL(textChanged()), this, SLOT(ActCurrentChange()));
		disconnect(currentedit, SIGNAL(refreshLabel(BkeScintilla *)), this, SLOT(refreshLabel(BkeScintilla *)));
		disconnect(currentedit, SIGNAL(refreshLabel(QStringList &)), this, SLOT(refreshLabel(QStringList &)));
		disconnect(currentedit, SIGNAL(ShouldAddToNavigation()), this, SLOT(ShouldAddToNavigation()));
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
	BkeDocBase* loli = docStrHash.value(LOLI_OS_QSTRING(old), 0);
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
	BkeDocBase* loli = docStrHash.value(LOLI_OS_QSTRING(file), 0);
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
		char *buf = new char[linelen + 1];
		loli->edit->SendScintilla(QsciScintilla::SCI_GETLINE, line, buf);
		buf[linelen] = 0;
		linelen--;
		while (buf[linelen] == '\r' || buf[linelen] == '\n')
		{
			buf[linelen--] = 0;
		}
		int offset = p.Start() - linestart;
		//实际行号从1开始
		othwin->searchlist->addItem(loli->edit->FileName + QString("(%1, %2)").arg(line + 1).arg(offset) + ":\t" + buf);
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
	BkeDocBase* loli = docStrHash.value(LOLI_OS_QSTRING(file), 0);
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
	BkeDocBase* loli = docStrHash.value(LOLI_OS_QSTRING(file), 0);

	//如果为空，则创建
	if (loli == 0){
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

void CodeWindow::simpleNew(BkeDocBase *loli, const QString &t)
{
	ignoreflag = true; //忽略改变，在所有准备工作完成以后才改变

	loli->edit = new BkeScintilla(this);
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
	return;
}

void CodeWindow::SaveFile()
{
	simpleSave(currentbase);
	return;
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
		QMessageBox *msg = new QMessageBox(this);
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

	BkeDocBase *ptr;
	for (int i = 0; i < ls.size(); i++){
		ptr = ls.at(i);
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
		llm = docStrHash.value(list.at(i), 0);
		//已经被打开的文件
		if (llm != 0){
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
	if (workpro == 0) return;

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
	if (workpro == 0) return;

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

	for (int i = 0; i < ls.size(); i++){
		if (LOLI_OS_QSTRING(ls.at(i)->ProjectDir()) != LOLI_OS_QSTRING(dir)) continue;
		if (!ls.at(i)->edit->isModified()) continue;
		ks.append(ls.at(i));
		ns.append(ls.at(i)->Name());
	}

	if (ns.size() < 1) return true;

	if (isCompileNotice == _NOTICE_ALWAYS){

		LableSureDialog msg;
		msg.SetLable("以下文件已经修改，是否保存？");
		msg.SetBtn(QStringList() << "取消" << "不要保存" << "全部保存");
		msg.SetCheckbox(QStringList() << "不再提示");
		QListWidget *tls = new QListWidget(&msg);
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

	for (int i = 0; i < ks.size(); i++) simpleSave(ks.at(i));

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
	for (int i = 0; i < a.size(); i++)
	{
		QFileInfo &info = a[i];
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
void CodeWindow::deleteCompileFile()
{
	if (workpro == 0)
		return;
	QStringList l = fileEntries(workpro->ProjectDir(), QStringList() << ".bkbin");
	for (auto i : l)
	{
		QFile(i).remove();
	}
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
		BkeMarkList templs = *(markadmin.GetPrombleMark(currentbase->FullName(), false));
		problemslist = markadmin.GetPrombleMark(currentbase->FullName(), true);
		CheckProblemMarks(currentedit, &templs);
		currentedit->update();
	}
	else{
		problemslist = markadmin.GetPrombleMark("", true);
	}

    othwin->compileedit->setText(comResult);
	othwin->compileedit->setReadOnly(true);
	othwin->ShowProblem(problemslist, workpro->ProjectDir());

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

	if (p->Atpos > 1)
		currentedit->setFirstVisibleLine(p->Atpos - 1);
	else
		currentedit->setFirstVisibleLine(p->Atpos);
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
	if (currentbase != 0){
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
	CheckProblemMarks(currentedit, markadmin.GetPrombleMark(currentbase->FullName(), false));
	currentedit->update();
}

//标记更新完后使用 updata
void CodeWindow::CheckProblemMarks(BkeScintilla *edit, BkeMarkList *list)
{
	edit->clearAnnotations();
	edit->markerDeleteAll(1);
	edit->markerDeleteAll(2);

	if (list->isEmpty()) return;

	BkeMarkerBase *abc;
	QString info;

	for (int i = 0; i < list->size(); i++){
		abc = list->at(i);
		info = edit->annotation(abc->Atpos - 1);

		if (info.isEmpty()) info = abc->Information;
		else info += "\r\n" + LOLI_AUTONEXT_QSTRING(abc->Information);

		if (abc->Type == 1){
			edit->annotate(abc->Atpos - 1, info, ks1);
			edit->markerAdd(abc->Atpos - 1, 1);
		}
		else{
			edit->annotate(abc->Atpos - 1, info, ks2);
			edit->markerAdd(abc->Atpos - 1, 2);
		}
	}
}

void CodeWindow::CheckBookMarks(BkeScintilla *edit, BkeMarkList *list)
{
	edit->markerDeleteAll(3);

	if (list->isEmpty()) return;

	BkeMarkerBase *abc;
	for (int i = 0; i < list->size(); i++){
		abc = list->at(i);
		edit->markerAdd(abc->Atpos, 3);
	}
}

void CodeWindow::CheckMarks(BkeScintilla *edit, BkeMarkList *list)
{
	edit->markerDeleteAll(4);

	if (list->isEmpty()) return;

	BkeMarkerBase *abc;
	for (int i = 0; i < list->size(); i++){
		abc = list->at(i);
		edit->markerAdd(abc->Atpos, 4);
	}
}


//运行BKEngine.exe
void CodeWindow::RunBKE()
{
	QString ndir;
	/*if (!currentproject->config->live2DKey.isEmpty())
#ifdef Q_OS_WIN
		ndir = BKE_CURRENT_DIR + "/tool/BKEngine_Live2D_Dev.exe";
#elif defined(Q_OS_MAC)
		ndir = BKE_CURRENT_DIR+"/BKEngine_Dev.app"; // Mac上没有Live2D
#else
		ndir = BKE_CURRENT_DIR+"/tool/BKEngine_Dev"; // Linux上没有Live2D
#endif
	else*/
#ifdef Q_OS_WIN
		ndir = BKE_CURRENT_DIR + "/tool/BKEngine_Dev.exe";
#elif defined(Q_OS_MAC)
		ndir = BKE_CURRENT_DIR+"/BKEngine_Dev.app";
#else
		ndir = BKE_CURRENT_DIR+"/tool/BKEngine_Dev";
#endif
#ifdef Q_OS_MAC
	QProcess::startDetached( "open",QStringList() << ndir << "--args" << "-dir" << workpro->ProjectDir() << "-nologo") ;
#else
	QProcess::startDetached(ndir, QStringList() << "-nologo", workpro->ProjectDir());
#endif
}

void CodeWindow::RunBKEWithArgs()
{
	QString ndir;
	/*if (!currentproject->config->live2DKey.isEmpty())
	#ifdef Q_OS_WIN
	ndir = BKE_CURRENT_DIR + "/tool/BKEngine_Live2D_Dev.exe";
	#elif defined(Q_OS_MAC)
	ndir = BKE_CURRENT_DIR+"/BKEngine_Dev.app"; // Mac上没有Live2D
	#else
	ndir = BKE_CURRENT_DIR+"/tool/BKEngine_Dev"; // Linux上没有Live2D
	#endif
	else*/
#ifdef Q_OS_WIN
	ndir = BKE_CURRENT_DIR + "/tool/BKEngine_Dev.exe";
#elif defined(Q_OS_MAC)
	ndir = BKE_CURRENT_DIR + "/BKEngine_Dev.app";
#else
	ndir = BKE_CURRENT_DIR + "/tool/BKEngine_Dev";
#endif
	QStringList args;
#ifdef Q_OS_MAC
	args << ndir << "--args" << "-dir" << workpro->ProjectDir() << "-nologo";
	for (auto &s : BKE_extraArgs)
		args << s;
	QProcess::startDetached("open", args);
#else
	args << "-nologo";
	for (auto &s : BKE_extraArgs)
		args << s;
	QProcess::startDetached(ndir, args, workpro->ProjectDir());
#endif
}

void CodeWindow::AnnotateSelect()
{
	currentedit->BkeAnnotateSelect();
}

void CodeWindow::ClearCompile()
{
	deleteCompileFile();
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
	QStringList l;
	sci->analysis->getLabels(sci->FileName, l);
	refreshLabel(l);
}

void CodeWindow::refreshLabel(QStringList &l)
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

		if (p == 0) {
			btncompileact->setEnabled(false);  //编译按钮只有当工程出现时才可用
			btncompilerunact->setEnabled(false);
			btnrunact->setEnabled(false);
			btndebugact->setEnabled(false);
			workpro = 0;
			return;
		}

		workpro = p;
		btncompileact->setEnabled(true);  //工程出现后编译按钮都是可用的
		btncompilerunact->setEnabled(true);
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
	BkeDocBase *llm = new BkeDocBase;
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
		tempbase = docStrHash.value(LOLI_OS_QSTRING(path), 0);
		if (tempbase == 0) return;
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
			if (pro != 0) pro->RemoveItem(tempbase->FullName());
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
		currentedit = NULL;
		filewatcher->removePaths(filewatcher->files()) ;
		DrawLine(false);
	}
	else{
		SetCurrentEdit(stackwidget->currentIndex());
	}

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
	int line, index;
	currentedit->lineIndexFromPositionByte(pos, &line, &index);
	if (line < 0)
		return;
	currentedit->setFirstVisibleLine(line);
}

void CodeWindow::GotoLabelList()
{
	if (currentedit == nullptr || currentedit->analysis == nullptr)
	{
		return;
	}
	QStringList qs;
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
	GotoFileDialog *dialog = new GotoFileDialog(qs, this);
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
	calcWords(currentedit->text(), this->othwin->lewords);
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
	QAction *act = (QAction*)sender();
	QStringList ls = act->data().toString().split('|');
	int pos = ls[1].toInt();
	AddFile(workpro->ProjectDir() + ls[0]);
	if (currentedit->FileName != ls[0])
		return;
	int line, xpos;
	currentedit->lineIndexFromPositionByte(pos, &line, &xpos);
	currentedit->setFirstVisibleLine(line);
}

void CodeWindow::jumpToCodeFunc()
{
	QAction *act = (QAction*)sender();
	QStringList ls = act->data().toString().split('|');
	AddFile(workpro->ProjectDir() + ls[0]);
	if (currentedit->FileName != ls[0])
		return;
	int pos = currentedit->analysis->findLabel(currentedit->FileName, ls[1]);
	int line, index;
	currentedit->lineIndexFromPositionByte(pos, &line, &index);
	currentedit->setFirstVisibleLine(line);
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
	if (navigationList.size() && navigationList[currentNavigation].first == file && navigationList[currentNavigation].second == pos)
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

void CodeWindow::RefreshNavigation()
{
	btnlastact->setEnabled(currentNavigation > 0);
	btnnextact->setEnabled(currentNavigation < navigationList.size() - 1);
}
