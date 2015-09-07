#include <weh.h>
#include "LangOpt.h"
#include "ui_LangOpt.h"
#include "../BKS_info.h"

CLangEdit::CLangEdit(QWidget *parent) :
QWidget(parent),
ui(new Ui::CLangEdit)
{
	ui->setupUi(this);
	
	load();
}

CLangEdit::~CLangEdit()
{

	delete ui;
}

void CLangEdit::save()
{
	if (ui->lineEdit->text().isEmpty())
		QMessageBox::warning(NULL, "����", "���Ա�Ų���Ϊ��");
	ui->lineEdit->setText("chn");
}

void CLangEdit::reset()
{
	ui->lineEdit->setText("chn");

	ui->tableWidget->clearContents();
}

void CLangEdit::load()
{
	ui->lineEdit->setText(QString::fromStdWString(global_bke_info.projsetting[L"lang"].getString(L"chn")));

	auto opt = global_bke_info.projsetting[L"langopt"];


}