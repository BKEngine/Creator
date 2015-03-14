#include <weh.h>
#include "ctextedit.h"
#include "ui_ctextedit.h"

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
    lst.prepend("默认");
    ui->listBox->addItems( lst ) ;

	cdia = new QColorDialog(this);

	connect(ui->fcbtn, SIGNAL(clicked()), this, SLOT(onForecolorClicked()));
	connect(ui->bcbtn, SIGNAL(clicked()), this, SLOT(onBackcolorClicked()));
}
CTextEdit::~CTextEdit()
{
    delete ui;
}

void CTextEdit::configchange(int ci)
{
    QString hname = ui->listBox->currentText() ;
    if( hname == "默认" ) ui->delbtn->setEnabled(false);
    lex->ReadConfig(hname);
    upColour();
}

void CTextEdit::upColour()
{
    for( int i = 0 ; i < 10 ; i++ ){
		QBrush br(QColor(lex->defaultColor(list_colors[i])));
        ui->listWidget->item(i)->setForeground(br);
    }
}

void CTextEdit::itemchange(int index)
{
    if( index < 0 || index > ui->listWidget->count() ) return ;
	curindex = index;
    QString temp ;
	temp.setNum(lex->defaultColor(list_colors[index]).rgb(), 16);
    temp = "QPushButton{background-color:#" + temp.right(temp.length()-2) + ";}" ;
    ui->fcbtn->setStyleSheet(temp);
	temp.setNum(lex->defaultPaper(list_colors[index]).rgb(), 16);
    temp = "QPushButton{background-color:#" + temp.right(temp.length()-2) + ";}" ;
    ui->bcbtn->setStyleSheet(temp);
    return ;
}

void CTextEdit::onForecolorClicked()
{
	auto color = cdia->getColor();
}

void CTextEdit::onBackcolorClicked()
{

}