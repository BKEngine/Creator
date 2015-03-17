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
    //setAttribute(Qt::WA_DeleteOnClose, true);
    connect(ui->listBox,SIGNAL(currentIndexChanged(int)),this,SLOT(configchange(int))) ;
    connect(ui->listWidget,SIGNAL(currentRowChanged(int)),this,SLOT(itemchange(int))) ;

    lex = new QsciLexerBkeScript ;
    QStringList lst = lex->ConfigList() ;
	if (lst.empty() || lst.front()!="默认")
	    lst.prepend("默认");
    ui->listBox->addItems( lst ) ;
	int i = lst.indexOf(lex->ConfigName());
	cdia = new QColorDialog();
	QString curFont = BKE_USER_SETTING->value("sys/HighlightFont", "微软雅黑").toString();
	ui->fontComboBox->setCurrentText(curFont);
	QString curFontSize = BKE_USER_SETTING->value("sys/HighlightFontSize", "13").toString();
	ui->comboBox_2->setCurrentText(curFontSize);

	connect(ui->fcbtn, SIGNAL(clicked()), this, SLOT(onForecolorClicked()));
	connect(ui->bcbtn, SIGNAL(clicked()), this, SLOT(onBackcolorClicked()));
	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(onOKClicked()));
	connect(ui->btnReset, SIGNAL(clicked()), this, SLOT(resetColor()));

	if (codeedit)
	{
		connect(this, SIGNAL(onRefreshLexer()), codeedit, SLOT(resetLexer()));
	}

	if (i >= 0)
		ui->listBox->setCurrentIndex(i);
	else
		ui->listBox->setCurrentIndex(0);

	configchange(0);
	itemchange(0);
}

CTextEdit::~CTextEdit()
{
    delete ui;
}

void CTextEdit::resetColor()
{
	for (int i = 0; i < ui->listWidget->count(); i++)
	{
		setcolor[i] = lex->defaultColor(list_colors[i]);
		setcolor_b[i] = lex->defaultPaper(list_colors[i]);
		QBrush br(setcolor[i]);
		ui->listWidget->item(i)->setForeground(br);
		QBrush br2(setcolor_b[i]);
		ui->listWidget->item(i)->setBackground(br2);
	}
	int r = ui->listWidget->currentRow();
	itemchange(r);
	//auto reset font and font size
	ui->fontComboBox->setCurrentText("微软雅黑");
	ui->comboBox_2->setCurrentText("13");
}

void CTextEdit::configchange(int ci)
{
    QString hname = ui->listBox->currentText() ;
    if( hname == "默认" ) ui->delbtn->setEnabled(false);
    readConfig(hname);
    upColour();
}

void CTextEdit::readConfig(const QString &name)
{
	QStringList v = BKE_USER_SETTING->value("sys/Highlight-" + name, QStringList()).toStringList();
	if (v.empty())
	{
		resetColor();
		onOKClicked();
		return;
	}
	for (int i = 0; i < v.count(); i += 2)
	{
		setcolor[i / 2] = v[i].toUInt();
		setcolor_b[i / 2] = v[i + 1].toUInt();
	}
}

void CTextEdit::upColour()
{
	for (int i = 0; i < ui->listWidget->count(); i++)
	{
		QBrush br(setcolor[i]);
        ui->listWidget->item(i)->setForeground(br);
		QBrush br2(setcolor_b[i]);
		ui->listWidget->item(i)->setBackground(br2);
	}
}

void CTextEdit::itemchange(int index)
{
    if( index < 0 || index > ui->listWidget->count() ) return ;
	curindex = index;
    QString temp ;
	temp.setNum(setcolor[index].rgb(), 16);
    temp = "QPushButton{background-color:#" + temp.right(temp.length()-2) + ";}" ;
    ui->fcbtn->setStyleSheet(temp);
	temp.setNum(setcolor_b[index].rgb(), 16);
    temp = "QPushButton{background-color:#" + temp.right(temp.length()-2) + ";}" ;
    ui->bcbtn->setStyleSheet(temp);
    return ;
}

void CTextEdit::onForecolorClicked()
{
	auto color = cdia->getColor(setcolor[curindex]);
	if (!color.isValid())
		return;
	setcolor[curindex] = color;
	QString temp;
	temp.setNum(color.rgb(), 16);
	temp = "QPushButton{background-color:#" + temp.right(temp.length() - 2) + ";}";
	ui->fcbtn->setStyleSheet(temp);
}

void CTextEdit::onBackcolorClicked()
{
	auto color = cdia->getColor(setcolor_b[curindex]);
	if (!color.isValid())
		return;
	setcolor_b[curindex] = color;
	QString temp;
	temp.setNum(color.rgb(), 16);
	temp = "QPushButton{background-color:#" + temp.right(temp.length() - 2) + ";}";
	ui->bcbtn->setStyleSheet(temp);
}

void CTextEdit::onOKClicked()
{
	emit onOK();
}

void CTextEdit::onSave()
{
	QStringList c;
	for (int i = 0; i < ui->listWidget->count(); i++)
	{
		c.push_back(QString("%1").arg(setcolor[i].rgb()));
		c.push_back(QString("%1").arg(setcolor_b[i].rgb()));
	}
	BKE_USER_SETTING->setValue("sys/Highlight-" + ui->listBox->currentText(), c);
	BKE_USER_SETTING->setValue("sys/Highlight", ui->listBox->currentText());
	BKE_USER_SETTING->setValue("sys/HighlightFont", ui->fontComboBox->currentText());
	BKE_USER_SETTING->setValue("sys/HighlightFontSize", ui->comboBox_2->currentText());

	emit onRefreshLexer();
}