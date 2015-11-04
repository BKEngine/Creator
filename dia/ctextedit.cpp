#include <weh.h>
#include "ctextedit.h"
#include "ui_ctextedit.h"
#include "../codewindow.h"

int list_colors[] = { SCE_BKE_DEFAULT, SCE_BKE_COMMAND, SCE_BKE_ATTRIBUTE, SCE_BKE_STRING, SCE_BKE_NUMBER, SCE_BKE_COLOR, SCE_BKE_TRANS,
SCE_BKE_LABEL, SCE_BKE_COMMENT, SCE_BKE_OPERATORS, SCE_BKE_ERROR, SCE_BKE_PARSER_KEYWORD, (1 << 6) };

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
	int begin,end ;
	if( Row < 0){ begin = 0 ; end = ui->listWidget->count() ; }
	else{ begin = Row ; end = Row+1 ; }

	for (;begin < end ; begin++ )
	{
		QBrush br( QColor(lex->hlb[begin].fc) );
		ui->listWidget->item(begin)->setForeground(br);
		QBrush br2(QColor(lex->hlb[begin].bc));
		ui->listWidget->item(begin)->setBackground(br2);
	}
}

void CTextEdit::itemchange(int index)
{
	if( index < 0 || index > ui->listWidget->count() ) return ;
	curindex = index;
	ui->fcbtn->setStyleSheet( "QPushButton{background-color:#" + BkeCreator::IntToRgbString( lex->hlb[index].fc )+ ";}" );
	ui->bcbtn->setStyleSheet("QPushButton{background-color:#" + BkeCreator::IntToRgbString( lex->hlb[index].bc )+ ";}" );
	return ;
}

void CTextEdit::onForecolorClicked()
{
	if( !CheckBtn() ) return ;
	lex->hlb[curindex].fc = setcolor.rgb() ;
	ui->fcbtn->setStyleSheet("QPushButton{background-color:#" + BkeCreator::IntToRgbString(lex->hlb[curindex].fc) + ";}");
	upColour(curindex);
}

void CTextEdit::onBackcolorClicked()
{
	if( !CheckBtn() ) return ;
	lex->hlb[curindex].bc = setcolor.rgb() ;
	ui->bcbtn->setStyleSheet("QPushButton{background-color:#" + BkeCreator::IntToRgbString(lex->hlb[curindex].bc) + ";}");
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

