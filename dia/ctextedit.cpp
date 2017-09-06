#include <weh.h>
#include "ctextedit.h"
#include "ui_ctextedit.h"
#include "../codewindow.h"

struct ColorDesc
{
	QString desc;
	vector<int> colors;
};
static vector<ColorDesc> desc=
{
	{"普通的文字", { SCE_BKE_DEFAULT, SCE_BKE_PARSER_DEFAULT, SCE_BKE_PARSER, SCE_BKE_TEXT, SCE_BKE_PARSER_VAR }},
	{"命令（@action）", { SCE_BKE_COMMAND, SCE_BKE_COMMAND2 }},
	{"属性名（time=100）", { SCE_BKE_ATTRIBUTE }},
	{"字符串（\"面包工坊\"）", { SCE_BKE_STRING, SCE_BKE_STRING2 }},
	{ "数字（123456789）",{ SCE_BKE_NUMBER } },
	{ "颜色（#FFFFFF）",{ SCE_BKE_COLOR } },
	{ "转义字符（'\\r\\n'）",{ SCE_BKE_TRANS } },
	{ "标签（*main）",{ SCE_BKE_LABEL_DEFINE } },
	{ "代码中的标签（*main）",{ SCE_BKE_LABEL_IN_PARSER } },
	{ "注释（//设置）",{ SCE_BKE_COMMENT, SCE_BKE_ANNOTATE } },
	{ "运算符（+-*/）",{ SCE_BKE_OPERATORS } },
	{ "错误（&^$%%^#）",{ SCE_BKE_ERROR } },
	{ "关键字（class,false,if）",{ SCE_BKE_PARSER_KEYWORD } },
	{ "Bagel脚本内容",{ SCE_BKE_PARSER_BG } },
};

CTextEdit::CTextEdit(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CTextEdit)
{
	ui->setupUi(this);
	lex = new QsciLexerBkeScript ;

	ui->listBox->addItems( lex->ConfigList() ) ;

	connect(ui->listWidget,SIGNAL(currentRowChanged(int)),this,SLOT(itemchange(int))) ;
	connect(ui->listBox,SIGNAL(currentIndexChanged(int)),this,SLOT(configchange(int))) ;
	connect(ui->fcbtn, SIGNAL(clicked()), this, SLOT(onForecolorClicked()));
	connect(ui->bcbtn, SIGNAL(clicked()), this, SLOT(onBackcolorClicked()));
	connect(ui->cpybtn,SIGNAL(clicked()),this,SLOT(onCopy())) ;
	connect(ui->fontComboBox,SIGNAL(currentTextChanged(QString)),this,SLOT(upFont())) ;
	connect(ui->comboBox_2,SIGNAL(currentTextChanged(QString)),this,SLOT(upFont())) ;

	ui->listBox->setCurrentText( lex->ConfigName() );
	curindex = ui->listBox->currentIndex();

	//create widgets
	for (auto &s : desc)
	{
		auto item = new QListWidgetItem(ui->listWidget);
		item->setText(s.desc);
	}

	load();
}

CTextEdit::~CTextEdit()
{
	delete ui;
}

void CTextEdit::save()
{
	QString stylename = ui->listBox->currentText();
	if (stylename.isEmpty() || stylename == "默认") return;

	QStringList c;
	for (int i = 0; i < ui->listWidget->count(); i++)
	{
		c.push_back(QString("%1").arg(lex->hlb[i].fc));
		c.push_back(QString("%1").arg(lex->hlb[i].bc));
	}

	BKE_USER_SETTING->setValue(stylename + "/colour", c);
	BKE_USER_SETTING->setValue(stylename + "/font", ui->fontComboBox->currentText());
	BKE_USER_SETTING->setValue(stylename + "/size", ui->comboBox_2->currentText());
	BKE_USER_SETTING->setValue(stylename + "/valid", QVariant(true));
}

void CTextEdit::load()
{
	upFont();
	upColour();
}

void CTextEdit::reset()
{
	lex->ReadConfig("默认");
	load();
}

void CTextEdit::configchange(int ci)
{
	QString hname = ui->listBox->currentText() ;
	if( hname == "默认" ) ui->delbtn->setEnabled(false);
	else ui->delbtn->setEnabled(true);
	lex->ReadConfig(ui->listBox->currentText());

	ui->fontComboBox->setCurrentText(lex->Lfont.family());
	ui->comboBox_2->setCurrentText( QString("%1").arg(lex->Lfont.pointSize()));
	upColour();
}

void CTextEdit::upColour(int Row)
{
	int begin, end ;
	if( Row < 0)
	{ 
		begin = 0 ;
		end = ui->listWidget->count() ;
	}
	else 
	{
		begin = Row; end = Row + 1;
	}


	for (;begin < end ; begin++ )
	{
		int coloridx = desc[begin].colors[0];
		QBrush br( QColor(lex->hlb[coloridx].fc) );
		ui->listWidget->item(begin)->setForeground(br);
		QBrush br2(QColor(lex->hlb[coloridx].bc));
		ui->listWidget->item(begin)->setBackground(br2);
	}
}

void CTextEdit::itemchange(int index)
{
	if( index < 0 || index > ui->listWidget->count() ) return ;
	curindex = index;
	int coloridx = desc[index].colors[0];
	ui->fcbtn->setStyleSheet( "QPushButton{background-color:#" + BkeCreator::IntToRgbString( lex->hlb[coloridx].fc )+ ";}" );
	ui->bcbtn->setStyleSheet("QPushButton{background-color:#" + BkeCreator::IntToRgbString( lex->hlb[coloridx].bc )+ ";}" );
	return ;
}

void CTextEdit::onForecolorClicked()
{
	if( !CheckBtn() ) return ;
	for (auto &idx : desc[curindex].colors)
	{
		lex->hlb[idx].fc = setcolor.rgb();
	};
	ui->fcbtn->setStyleSheet("QPushButton{background-color:#" + BkeCreator::IntToRgbString(lex->hlb[desc[curindex].colors[0]].fc) + ";}");
	upColour(curindex);
}

void CTextEdit::onBackcolorClicked()
{
	if( !CheckBtn() ) return ;
	for (auto &idx : desc[curindex].colors)
	{
		lex->hlb[idx].bc = setcolor.rgb();
	};
	ui->bcbtn->setStyleSheet("QPushButton{background-color:#" + BkeCreator::IntToRgbString(lex->hlb[desc[curindex].colors[0]].bc) + ";}");
	upColour(curindex);
}

bool CTextEdit::CheckBtn()
{
	if( ui->listBox->currentText() == "默认" ){
		QMessageBox::information(this,"","默认的颜色方案不可以更改！请新建一个新的方案或更改其他方案") ;
		return false ;
	}
	setcolor = QColorDialog::getColor() ;
	if (!setcolor.isValid()) return false;
	return true ;
}

void CTextEdit::onOKClicked()
{
	emit onOK();
}

void CTextEdit::onSave()
{
	save();
}

void CTextEdit::SetCurrentStyle(QString stylename)
{
	if( stylename.isEmpty() ) stylename = ui->listBox->currentText() ;
	BKE_USER_SETTING->setValue("SyleCurrent",stylename);
}


//复制当前样式
void CTextEdit::onCopy()
{
	QString NewStyleName ;
	bool gww ;
	do{
		NewStyleName = QInputDialog::getText(this,"","请输入新样式的名字",QLineEdit::Normal,"",&gww) ;
		if( !gww ) return  ;
		if( ui->listBox->findText(NewStyleName) >= 0 ){
			QMessageBox::information(this,"","重复的类型名字") ;
			NewStyleName.clear();
		}
	} while( NewStyleName.isEmpty() ) ;

	ui->listBox->insertItem(0,NewStyleName);
	ui->listBox->setCurrentText(NewStyleName);
}


void CTextEdit::upFont()
{
	QFont ss ;
	ss.setFamily(ui->fontComboBox->currentText());
	ss.setPointSize(ui->comboBox_2->currentText().toInt()-2);
	ui->listWidget->setFont(ss);
}

