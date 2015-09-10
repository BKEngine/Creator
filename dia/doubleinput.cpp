#include "doubleinput.h"
#include "ui_doubleinput.h"

DoubleInput::DoubleInput(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DoubleInput)
{
	ui->setupUi(this);
}

DoubleInput::~DoubleInput()
{
	delete ui;
}

void DoubleInput::setText(const QString &l1, const QString &t1, const QString &l2, const QString &t2)
{
	ui->label->setText(l1);
	ui->label_2->setText(l2);
	ui->lineEdit->setText(t1);
	ui->lineEdit_2->setText(t2);
}

void DoubleInput::waitResult(QString &r1, QString &r2)
{
	this->exec();
	r1 = ui->lineEdit->text();
	r2 = ui->lineEdit_2->text();
	this->close();
}