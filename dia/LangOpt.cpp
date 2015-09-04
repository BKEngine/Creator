#include <weh.h>
#include "LangOpt.h"
#include "ui_LangOpt.h"

CLangEdit::CLangEdit(QWidget *parent) :
QWidget(parent),
ui(new Ui::CLangEdit)
{
	ui->setupUi(this);
	
	ui->lineEdit->setText(BKE_USER_SETTING->value("lang", "chn").toString());
}

CLangEdit::~CLangEdit()
{
	delete ui;
}
