#include "ctextedit.h"
#include "ui_ctextedit.h"

CTextEdit::CTextEdit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CTextEdit)
{
    ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose, true);

}

CTextEdit::~CTextEdit()
{
    delete ui;
}
