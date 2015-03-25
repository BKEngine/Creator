#include <weh.h>
#include "searchbox.h"
#include "../mainwindow.h"

SearchBox::SearchBox(QWidget *parent) :
QDockWidget(parent)
{
	QWidget *w = new QWidget();
	h1 = new QVBoxLayout;
	h2 = new QVBoxLayout;
	v1 = new QHBoxLayout;
	btnsearchlast = new QPushButton("查找上一个", this);
	btnsearchnext = new QPushButton("查找下一个", this);
	btnsearchall = new QPushButton("查找全部", this);
	btnreplacemodel = new QPushButton("替换>>", this);
	btnreplace = new QPushButton("替换", this);
	btnreplaceall = new QPushButton("替换全部", this);
	lable1 = new QLabel("查找：", this);
	lable2 = new QLabel("替换：", this);
	edit = new QLineEdit(this);
	edit1 = new QLineEdit(this);
	iscase = new QCheckBox("区分大小写", this);
	isregular = new QCheckBox("使用正则表达式", this);
	isword = new QCheckBox("作为单词完整匹配", this);
	findallpro = new QCheckBox("在整个工程里搜索", this);
	isalwaysbegin = new QCheckBox("总是从文档开头(结尾)开始查找", this);
	isalwaysbegin->setChecked(true);
	sciedit = 0;
	firstshow = true;

	connect(findallpro, SIGNAL(stateChanged(int)), this, SLOT(onSearcAllConditionChange()));
	connect(iscase, SIGNAL(stateChanged(int)), this, SLOT(onFindConditionChange()));
	connect(isregular, SIGNAL(stateChanged(int)), this, SLOT(onFindConditionChange()));
	connect(isword, SIGNAL(stateChanged(int)), this, SLOT(onFindConditionChange()));

	h1->addWidget(lable1);
	h1->addWidget(edit);
	h1->addWidget(lable2);
	h1->addWidget(edit1);
	h1->addWidget(iscase);
	h1->addWidget(isregular);
	h1->addWidget(isword);
	h1->addWidget(findallpro);
	h1->addWidget(isalwaysbegin);
	h1->addStretch(1);

	h2->addWidget(btnsearchlast);
	h2->addWidget(btnsearchnext);
	h2->addWidget(btnsearchall);
	h2->addWidget(btnreplacemodel);
	h2->addWidget(btnreplace);
	h2->addWidget(btnreplaceall);
	h2->addStretch(2);

	v1->addLayout(h1);
	v1->addLayout(h2);
	v1->setSpacing(30);
	h1->setSpacing(5);
	h2->setSpacing(5);

	w->setLayout(v1);
	this->setWidget(w);
	this->setFixedSize(315, 195);
	connect(btnsearchnext, SIGNAL(clicked()), this, SLOT(FindNext()));
	connect(btnsearchlast, SIGNAL(clicked()), this, SLOT(FindLast()));
	connect(btnsearchall, SIGNAL(clicked()), this, SLOT(FindAll()));
	connect(btnreplacemodel, SIGNAL(clicked()), this, SLOT(ChangeModel()));
	connect(btnreplace, SIGNAL(clicked()), this, SLOT(ReplaceText()));
	connect(btnreplaceall, SIGNAL(clicked()), this, SLOT(ReplaceAllText()));
	this->hide();
}

void SearchBox::onDocChanged()
{
	if (this->isVisible() && edit->text() == fstr && !(sciedit->ChangeStateFlag & BkeScintilla::BKE_CHANGE_REPLACE))
	{
		sciedit->findFirst1(fstr, iscase->isChecked(), isregular->isChecked(), isword->isChecked());
	}
}

void SearchBox::onSelectionChanged()
{
	if (this->isVisible() && edit->text() == fstr && !sciedit->findlast.IsNull())
	{
		sciedit->SetIndicator(BkeScintilla::BKE_INDICATOR_FIND, sciedit->findlast);
		sciedit->findlast.Clear();
	}
}

void SearchBox::onFindConditionChange()
{
	sciedit->refind = true;
}

void SearchBox::onSearcAllConditionChange()
{
	if (findallpro->isChecked())
	{
		btnsearchlast->setEnabled(false);
		btnsearchnext->setEnabled(false);
		btnreplacemodel->setEnabled(false);
		btnreplace->setEnabled(false);
		//btnreplaceall->setEnabled(false);
	}
	else
	{
		btnsearchlast->setEnabled(true);
		btnsearchnext->setEnabled(true);
		btnreplacemodel->setEnabled(true);
		btnreplace->setEnabled(true);
		btnreplaceall->setEnabled(true);
		if (this->windowTitle() == "替换")
		{
			ReplaceModel();
		}
		else
		{
			SearchModel();
		}
	}
}

void SearchBox::FindAll()
{
	otheredit->clearSearch();
	bool all = findallpro->isChecked();
	if (!all)
	{
		fstr = edit->text();
		emit searchOne(sciedit->basedoc->FullName(), fstr, iscase->isChecked(), isregular->isChecked(), isword->isChecked());
		otheredit->IfShow(otheredit->btnsearch, true);
	}
	else
	{
		fstr = edit->text();
		emit searchAll(fstr, iscase->isChecked(), isregular->isChecked(), isword->isChecked());
		otheredit->IfShow(otheredit->btnsearch, true);
	}
	close();
}

void SearchBox::FindNext()
{
	if (edit->text().isEmpty()){
		QMessageBox::information(this, "查找", "请输入需要查找的内容！", QMessageBox::Ok);
		return;
	}
	else if (sciedit->refind || edit->text() != fstr){
		fstr = edit->text();
		sciedit->findFirst1(fstr, iscase->isChecked(), isregular->isChecked(), isword->isChecked());
		if (isalwaysbegin->isChecked()) sciedit->FindForward(0);
		else sciedit->FindForward(sciedit->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS));
	}
	else if (!sciedit->FindForward(sciedit->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS))){
		if (sciedit->findcount > 0)
			QMessageBox::information(this, "查找", "再往后没有了");
	}
}

void SearchBox::FindLast()
{
	if (edit->text().isEmpty()){
		QMessageBox::information(this, "查找", "请输入需要查找的内容！", QMessageBox::Ok);
		return;
	}
	else if (edit->text() != fstr){
		fstr = edit->text();
		sciedit->findFirst1(fstr, iscase->isChecked(), isregular->isChecked(), isword->isChecked());
		if (isalwaysbegin->isChecked()) sciedit->FindBack();
		else sciedit->FindBack(sciedit->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS));
	}
	else{
		sciedit->FindBack(sciedit->SendScintilla(QsciScintilla::SCI_GETCURRENTPOS));
	}
}


void SearchBox::SetSci(BkeScintilla *sci)
{
	sciedit = sci;
}

void SearchBox::closeEvent(QCloseEvent *event)
{
	if (sciedit == 0) return;
	fstr.clear();
	sciedit->ClearIndicators(BkeScintilla::BKE_INDICATOR_FIND);
}

void SearchBox::SearchModel()
{
	btnreplacemodel->setText("替换>>");
	this->setWindowTitle("查找");
	lable2->setEnabled(false);
	btnreplace->setEnabled(false);
	btnreplaceall->setEnabled(false);
	edit1->setEnabled(false);
	//如果有选中部分，自动填入edit
	edit->setText(sciedit->selectedText());
	if (sciedit == 0) return;
	Show_();
}

void SearchBox::ReplaceModel()
{
	btnreplacemodel->setText("查找>>");
	this->setWindowTitle("替换");
	lable2->setEnabled(true);
	btnreplace->setEnabled(true);
	btnreplaceall->setEnabled(true);
	edit1->setEnabled(true);
	edit->setText(sciedit->selectedText());
	if (sciedit == 0) return;
	Show_();
}

void SearchBox::ChangeModel()
{
	if (btnreplacemodel->text() == "替换>>") ReplaceModel();
	else SearchModel();
}

void SearchBox::ReplaceText()
{
	fstr = edit->text();
	bool res = sciedit->ReplaceText(fstr, edit1->text(), iscase->isChecked(), isregular->isChecked(), isword->isChecked());

	if (!res)
	{
		QMessageBox::information(this, "替换", "哼，再往前，没有可以替换的内容啦~", QMessageBox::Ok);
		return;
	}

	sciedit->findFirst1(fstr, iscase->isChecked(), isregular->isChecked(), isword->isChecked());
	//if (!sciedit->hasSelectedText()){
	//	QMessageBox::information(this, "替换", "没有可以替换的位置!", QMessageBox::Ok);
	//	return;
	//}

	//BkeIndicatorBase abc = sciedit->ReplaceFind(edit1->text());
	//if (abc.IsNull()){
	//	QMessageBox::information(this, "替换", "o(︶︿︶)o ，替换似乎出现了一个错误。", QMessageBox::Ok);
	//	return;
	//}
	//if (!sciedit->FindForward(abc.End())){
	//	QMessageBox::information(this, "替换", "哼，再往前，没有需要替换的内容啦~", QMessageBox::Ok);
	//}
}

void SearchBox::ReplaceAllText()
{
	if (!findallpro->isChecked())
	{
		sciedit->ClearIndicators(BkeScintilla::BKE_INDICATOR_FIND);
		fstr = edit->text();
		sciedit->ReplaceAllText(fstr, edit1->text(), iscase->isChecked(), isregular->isChecked(), isword->isChecked());
	}
	else
	{
		QMessageBox b;
		b.setWindowTitle("警告");
		b.setText("要启用撤销功能，必须打开所有受到影响的文件，确定打开吗？");
		b.addButton("是", QMessageBox::AcceptRole);
		b.addButton("否", QMessageBox::RejectRole);
		b.addButton("放弃替换操作", QMessageBox::DestructiveRole);
		auto res = b.exec();
		if (res == QMessageBox::DestructiveRole)
			return;
		emit replaceAll(edit->text(), edit1->text(), iscase->isChecked(), isregular->isChecked(), isword->isChecked(), res == QMessageBox::AcceptRole);
	}
}

void SearchBox::Show_()
{
	if (!isFloating()) setFloating(true);
	/*
		if( firstshow ){  //首次显示处于左下角
		QWidget *pw = this->parentWidget() ;
		if( pw != 0){
		QPoint pt = pw->pos() ;
		pt.setX(pt.x()+pw->width()-350) ;
		pt.setY(pt.y()+pw->height()-300);
		move(pw->mapToGlobal(pt));
		}
		firstshow = false ;
		}*/

	this->show();
	activateWindow();
}
