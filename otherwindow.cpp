#include <weh.h>
#include <QToolBar>
#include "otherwindow.h"

OtherWindow::OtherWindow(QWidget *parent)
	:TopBarWindow(parent)
{
	bc1 = QColor(0xc7, 0xed, 0xcc);
	bc2 = QColor(0xdc, 0xe2, 0xf1);

	stackw = new QStackedWidget(this);
	problemList = new QListWidget(this);
	searchlist = new QListWidget(this);
	compileedit = new QsciScintilla(this);
	bookmarklist = new QListWidget(this);
	marklist = new QListWidget(this);
	runtimeProblemList = new QListWidget(this);
	compileedit->setUtf8(true);

	stackw->addWidget(problemList);
	stackw->addWidget(searchlist);
	stackw->addWidget(compileedit);
	stackw->addWidget(bookmarklist);
	stackw->addWidget(marklist);
	stackw->addWidget(runtimeProblemList);
	SetCentreWidget(stackw);
	lastbtn = 0;

	icoerror = new QIcon(":/info/source/error.png");
	icowarning = new QIcon(":/info/source/alert.png");
	iconlog = new QIcon(":/info/source/log.png");
	icobookmark = new QIcon(":/cedit/source/Bookmark.png");

	btnproblem = new QPushButton("问题", this);
	btnsearch = new QPushButton("搜索结果", this);
	btncompiletext = new QPushButton("编译输出", this);
	btnbookmark = new QPushButton("书签", this);
	btnmark = new QPushButton("标记", this);
	btnruntimeproblem = new QPushButton("运行时问题", this);
	lewords = new QLineEdit(this);
	lewords->setReadOnly(true);
	lewords->setFixedWidth(150);
	lewords->setAlignment(Qt::AlignHCenter);

	connect(btnproblem, SIGNAL(clicked()), this, SLOT(switchToProblemWidget()));
	connect(btnsearch, SIGNAL(clicked()), this, SLOT(switchToSearchWidget()));
	connect(btncompiletext, SIGNAL(clicked()), this, SLOT(switchToCompileWidget()));
	connect(btnbookmark, SIGNAL(clicked()), this, SLOT(switchToBookmarkWidget()));
	connect(btnmark, SIGNAL(clicked()), this, SLOT(switchToMarkWidget()));
	connect(btnruntimeproblem, SIGNAL(clicked()), this, SLOT(switchToRuntimeProblemWidget()));
	connect(searchlist, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(SearchDoubleClicked(QListWidgetItem*)));
	connect(problemList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(ProblemDoubleClicked(QListWidgetItem*)));
	connect(bookmarklist, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(BookMarkDoubleClicked(QListWidgetItem*)));
	connect(marklist, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(MarkDoubleClicked(QListWidgetItem*)));
	connect(runtimeProblemList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(RuntimeProblemDoubleClicked(QListWidgetItem*)));

	emap[btnproblem] = problemList;
	emap[btnsearch] = searchlist;
	emap[btncompiletext] = compileedit;
	emap[btnbookmark] = bookmarklist;
	emap[btnmark] = marklist;
	emap[btnruntimeproblem] = runtimeProblemList;
}

OtherWindow::~OtherWindow()
{
	delete icoerror;
	delete icowarning;
	delete icobookmark;
}

void OtherWindow::switchToProblemWidget(bool force/* = false*/) { checkShow(btnproblem, force); }
void OtherWindow::switchToSearchWidget(bool force/* = false*/) { checkShow(btnsearch, force); }
void OtherWindow::switchToCompileWidget(bool force/* = false*/) { checkShow(btncompiletext, force); }
void OtherWindow::switchToBookmarkWidget(bool force/* = false*/) { checkShow(btnbookmark, force); }
void OtherWindow::switchToMarkWidget(bool force/* = false*/) { checkShow(btnmark, force); }
void OtherWindow::switchToRuntimeProblemWidget(bool force/* = false*/) { checkShow(btnruntimeproblem, force); }

void OtherWindow::checkShow(QPushButton *btn, bool must)
{
	if (lastbtn != 0) lastbtn->setStyleSheet("");
	if (must) lastbtn = 0;

	if (btn != lastbtn && this->isHidden()) { //切换不同的状态，并且窗口是隐藏的，则显示
		stackw->setCurrentWidget(emap.value(btn));
		this->show();
	}
	else if (btn != lastbtn) {         //切换不同的状态，并且窗口不是隐藏的，切换当前页
		stackw->setCurrentWidget(emap.value(btn));
	}
	else {
		this->hide();  //同一个状态按钮再点一次，则隐藏
		lastbtn = 0;
		return;
	}

	lastbtn = btn;
	btn->setStyleSheet("background: #808080");

	//if (btn == btnproblem);
	//else if (btn == btnbookmark);
	//else if (btn == btnmark);
}

void OtherWindow::ClearSearch()
{
	searchlist->clear();
	for (auto &&t : memsearch)
	{
		delete t;
	}
	memsearch.clear();
}

void OtherWindow::feedDownBar(QToolBar *downbar)
{
	downbar->addWidget(btnproblem);
	downbar->addWidget(btnsearch);
	downbar->addWidget(btncompiletext);
	downbar->addWidget(btnbookmark);
	downbar->addWidget(btnmark);
	downbar->addWidget(btnruntimeproblem);
	downbar->addSeparator();
	downbar->addWidget(lewords);
}

void OtherWindow::setTextCount(int count)
{
	lewords->setText("文本字数：" + QString::number(count));
}

void OtherWindow::AddSearchItem(const QString & label)
{
	searchlist->addItem(label);
}

void OtherWindow::onSearchOne(const QString &file, const QString &fullfile, int line, int start, int end)
{
	auto bm = new BkeMarkerBase();
	bm->Name = file;
	bm->FullName = fullfile;
	bm->Type = BkeMarkSupport::BKE_MARK_SEARCH;
	bm->Atpos = line;
	bm->start = start;
	bm->end = end;
	bm->Information = searchlist->item(memsearch.size())->text();
	memsearch.push_back(bm);
}

void OtherWindow::SearchDoubleClicked(QListWidgetItem * item)
{
	emit Location(memsearch.at(searchlist->currentRow()), "");
}

//定位问题处
void OtherWindow::ProblemDoubleClicked(QListWidgetItem * item)
{
	emit Location(memproblem.at(problemList->currentRow()), problemdir);
}

void OtherWindow::BookMarkDoubleClicked(QListWidgetItem * item)
{
	emit Location(membookmark.at(bookmarklist->currentRow()), "");
}

void OtherWindow::MarkDoubleClicked(QListWidgetItem * item)
{
	emit Location(memmark.at(marklist->currentRow()), "");
}

void OtherWindow::RuntimeProblemDoubleClicked(QListWidgetItem * item)
{
	emit Location(memruntimeproblem.at(runtimeProblemList->currentRow()), runtimeproblemdir);
}

void OtherWindow::SetProblemList(BkeMarkList *list, const QString &dir)
{
	problemdir = dir;
	problemList->clear();

	BkeMarkerBase *akb;
	QString temp;

	errorcount = list->size();
	for (int i = 0; i < list->size(); i++) {
		akb = list->at(i);
		if (akb->Type == 1) AddItem(akb, icoerror, problemList);
		else AddItem(akb, icowarning, problemList);
	}

	temp.setNum(errorcount);
	btnproblem->setText("问题(" + temp + ")");

	//复制一个拷贝，以免原列表被删除
	memproblem = *list;
	stackw->setCurrentWidget(problemList);
	if (errorcount > 0) checkShow(btnproblem, true);
}

void OtherWindow::SetRuntimeProblemList(BkeMarkList * list, const QString & dir)
{
	runtimeproblemdir = dir;
	runtimeProblemList->clear();

	BkeMarkerBase *akb;
	QString temp;
	if (list)
	{
		runtimeerrorcount = list->size();
		for (int i = 0; i < list->size(); i++) {
			akb = list->at(i);
			if (akb->Type == 1) AddItem(akb, icoerror, runtimeProblemList);
			else if(akb->Type == 0) AddItem(akb, iconlog, runtimeProblemList);
			else AddItem(akb, icowarning, runtimeProblemList);
		}

		temp.setNum(runtimeerrorcount);
		btnruntimeproblem->setText("运行时问题(" + temp + ")");

		//复制一个拷贝，以免原列表被删除
		memruntimeproblem = *list;
		stackw->setCurrentWidget(runtimeProblemList);
		if (runtimeerrorcount > 0) checkShow(btnruntimeproblem, true);
	}
	else
	{
		runtimeerrorcount = 0;
		temp.setNum(runtimeerrorcount);
		btnruntimeproblem->setText("运行时问题(" + temp + ")");
		runtimeProblemList->clear();
		memruntimeproblem.clear();
	}
}

void OtherWindow::SetCompileResult(const QString & label)
{
	compileedit->setText(label);
	compileedit->setReadOnly(true);
}

void OtherWindow::AddItem(BkeMarkerBase *marker, QIcon *ico, QListWidget* w)
{
	QListWidgetItem *le = new QListWidgetItem;
	le->setIcon(*ico);
	QString temp;
	if (!marker->Name.isEmpty()) {
		temp.append("【" + marker->Name + "】");
	}
	if (marker->Atpos >= 0) {
		temp.append("第" + QString::number(marker->Atpos) +  "行：");
	}
	temp.append(marker->Information);
	le->setText(temp);
	w->addItem(le);
}

void OtherWindow::RefreshBookMark(BkeMarkList *b)
{
	membookmark.clear();
	membookmark = *b;
	bookmarklist->clear();
	for (int i = 0; i < membookmark.size(); i++) {
		AddItem(membookmark.at(i), icobookmark, bookmarklist);
	}
}



