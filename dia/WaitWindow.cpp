#include <weh.h>

#include "WaitWindow.h"
#include "ui_WaitWindow.h"

WaitWindow::WaitWindow(QWidget *parent)
	:QDialog(parent),
	ui(new Ui::Dialog)
{
	ui->setupUi(this);
}

WaitWindow::~WaitWindow()
{
	delete ui;
}

void WaitWindow::setInfo(const QString &s, int max)
{
	in = s;
	m_max = max;
	ui->pb->setMaximum(m_max);
	setNum(1);
}

void WaitWindow::setNum(int n)
{
	m_cur = n;
	QString tmp, tmp2;
	tmp.setNum(m_max, 10);
	tmp2.setNum(m_cur, 10);
	ui->info->setText(in + "\t" + tmp2 + "/" + tmp);
	ui->pb->setValue(n);
	update();
}

void WaitWindow::addNum()
{
	setNum(m_cur + 1);
}