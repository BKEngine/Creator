#include <weh.h>
#include "ctextedit.h"
#include "ui_ctextedit.h"

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
        QBrush br( QColor(lex->hlb[i].fc) ) ;
        ui->listWidget->item(i)->setForeground(br);
    }
}

void CTextEdit::itemchange(int index)
{
    if( index < 0 || index > ui->listWidget->count() ) return ;
    QString temp ;
    temp.setNum( lex->hlb[index].fc,16) ;
    temp = "QPushButton{background-color:#" + temp.right(temp.length()-2) + ";}" ;
    ui->fcbtn->setStyleSheet(temp);
    temp.setNum( lex->hlb[index].bc,16) ;
    temp = "QPushButton{background-color:#" + temp.right(temp.length()-2) + ";}" ;
    ui->bcbtn->setStyleSheet(temp);
    return ;
}

