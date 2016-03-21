#include <weh.h>
#include "LangOpt.h"
#include "ui_LangOpt.h"
#include "../BKS_info.h"
#include "doubleinput.h"

CLangEdit::CLangEdit(QWidget *parent) :
QWidget(parent),
ui(new Ui::CLangEdit),
langs({ "chs", "cht", "eng", "jpn", "kor" })
{
	ui->setupUi(this);

	connect(ui->addButton, SIGNAL(clicked()), this, SLOT(onAddClick()));
	connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(onRemoveClick()));

	connect(ui->tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(onDoubleClick(QTableWidgetItem*)));
	
	ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

	ui->comboBox->insertItems(0, langs);

	load();
}

CLangEdit::~CLangEdit()
{
	delete ui;
}

void CLangEdit::save()
{
	if (ui->comboBox->currentText().isEmpty())
	{
		QMessageBox::warning(NULL, "警告", "语言编号不能为空");
		ui->comboBox->setCurrentIndex(0);
	}
	global_bke_info.projsetting[L"lang"] = ui->comboBox->currentText().toStdWString();

	BKE_Variable tempset = new BKE_VarDic();

	int row = ui->tableWidget->rowCount();

	for (int i = 0; i < row; i++)
	{
		auto t1 = ui->tableWidget->item(i, 0)->text().toStdWString();
		auto t2 = ui->tableWidget->item(i, 1)->text();

		if (t2.isEmpty())
			tempset[t1] = 1;
		else
		{
			auto tl = t2.split(' ');
			BKE_Variable t = new BKE_VarDic();
			for (auto &ii : tl)
			{
				auto prop = ii.trimmed();
				if (!prop.isEmpty())
				{
					t[prop.toStdWString()] = 1;
				}
			}
			tempset[t1] = t;
		}
	}

	global_bke_info.projsetting[L"langopt"] = tempset;

	global_bke_info.save();
}

void CLangEdit::reset()
{
	ui->comboBox->clear();
	ui->comboBox->insertItems(0, langs);
	ui->comboBox->setCurrentIndex(0);

	ui->tableWidget->clearContents();

	ui->tableWidget->setRowCount(1);

	QTableWidgetItem *item = new QTableWidgetItem("textsprite");
	ui->tableWidget->setItem(0, 0, item);
	item = new QTableWidgetItem("text");
	ui->tableWidget->setItem(0, 1, item);
}

void CLangEdit::load()
{
	QString lng = QString::fromStdWString(global_bke_info.projsetting[L"lang"].getString(L"chs"));
	if (langs.contains(lng))
	{
		ui->comboBox->setCurrentText(lng);
	}
	else
	{
		ui->comboBox->setCurrentIndex(0);
	}

	auto opt = global_bke_info.projsetting[L"langopt"];

	if (opt.getType() != VAR_DIC)
		opt = new BKE_VarDic();
	auto &vmap = opt.forceAsDic()->varmap;
	int row = 0;
	for (auto &it : vmap)
	{
		auto &v = it.second;
		if (v.getType() == VAR_NUM && v.asBoolean())
		{
			ui->tableWidget->setRowCount(row + 1);
			QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdWString(it.first.getConstStr()));
			ui->tableWidget->setItem(row, 0, item);
			item = new QTableWidgetItem("");
			ui->tableWidget->setItem(row, 1, item);
			sortLast();
			row++;
		}
		else
		{
			//part
			ui->tableWidget->setRowCount(row + 1);
			QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdWString(it.first.getConstStr()));
			ui->tableWidget->setItem(row, 0, item);
			item = new QTableWidgetItem("");
			ui->tableWidget->setItem(row, 1, item);
			if (v.getType() == VAR_DIC)
			{
				QString res;
				auto &vvmap = v.forceAsDic()->varmap;
				for (auto &it2 : vvmap)
				{
					if (it2.second.asBoolean())
					{
						if (!res.isEmpty())
							res += " ";
						res += QString::fromStdWString(it2.first.getConstStr());
					}
				}
				item = new QTableWidgetItem(res);
				ui->tableWidget->setItem(row, 1, item);
			}
			sortLast();
			row++;
		}
	}
}

void CLangEdit::onAddClick()
{
	DoubleInput input;
	input.setText("输入指令的名称", "", "输入要参与翻译的属性名，用空格隔开，留空表示全部", "");
	QString r1, r2;
	input.waitResult(r1, r2);

	if (r1.isEmpty())
	{
		QMessageBox::information(this, "注意", "命令名不能为空");
		return;
	}

	int row = ui->tableWidget->rowCount();
	QTableWidgetItem *item;

	int res = findRow(r1);

	if (res < 0)
	{
		ui->tableWidget->setRowCount(row + 1);
		item = new QTableWidgetItem(r1);
		ui->tableWidget->setItem(row, 0, item);
		item = new QTableWidgetItem(r2);
		ui->tableWidget->setItem(row, 1, item);
		sortLast();
		return;
	}
	else
	{
		QMessageBox msg(QMessageBox::Icon::Warning, "警告", "该命令已存在，是否覆盖现有的设置？");
		msg.addButton("保存", QMessageBox::AcceptRole);
		msg.addButton("关闭", QMessageBox::RejectRole);
		int i = msg.exec();
		if (i == QMessageBox::AcceptRole)
		{
			ui->tableWidget->item(res, 1)->setText(r2);
			return;
		}
		else if (i == QMessageBox::DestructiveRole)
		{
			return;
		}
	}
}

void CLangEdit::removeRow(const QString &tag)
{
	int row = ui->tableWidget->rowCount();
	for (int i = 0; i < row; i++)
	{
		if (ui->tableWidget->item(i, 0)->text() == tag)
		{
			ui->tableWidget->removeRow(i);
			i--;
		}
	}
}

int CLangEdit::findRow(const QString &tag)
{
	int row = ui->tableWidget->rowCount();
	for (int i = 0; i < row; i++)
	{
		if (ui->tableWidget->item(i, 0)->text() == tag)
			return i;
	}
	return -1;
}

void CLangEdit::sortLast()
{
	int row = ui->tableWidget->rowCount();
	row--;
	auto *item = ui->tableWidget->item(row, 0);
	int i = row - 1;
	while (i>=0)
	{
		if (ui->tableWidget->item(i, 0)->text() < item->text())
		{
			break;
		}
		i--;
	}
	i++;
	if (i == row)
		return;
	ui->tableWidget->insertRow(i);
	ui->tableWidget->setItem(i, 0, new QTableWidgetItem(item->text()));
	ui->tableWidget->setItem(i, 1, new QTableWidgetItem(ui->tableWidget->item(row + 1, 1)->text()));
	ui->tableWidget->removeRow(row + 1);
}

void CLangEdit::onRemoveClick()
{
	int r = ui->tableWidget->currentRow();
	ui->tableWidget->removeRow(r);
}

void CLangEdit::onDoubleClick(QTableWidgetItem* clickitem)
{
	int r = ui->tableWidget->row(clickitem);
	auto i1 = ui->tableWidget->item(r, 0);
	auto i2 = ui->tableWidget->item(r, 1);
	DoubleInput input;
	input.setText("输入指令的名称", i1->text(), "输入要参与翻译的属性名，用空格隔开，留空表示全部", i2->text());
	QString r1, r2;
	input.waitResult(r1, r2);

	if (r1.isEmpty())
	{
		QMessageBox::information(this, "注意", "命令名不能为空");
		return;
	}

	int row = ui->tableWidget->rowCount();
	QTableWidgetItem *item;

	int res = findRow(r1);

	if (res < 0)
	{
		ui->tableWidget->setRowCount(row + 1);
		item = new QTableWidgetItem(r1);
		ui->tableWidget->setItem(row, 0, item);
		item = new QTableWidgetItem(r2);
		ui->tableWidget->setItem(row, 1, item);
		ui->tableWidget->removeRow(r);
		sortLast();
		return;
	}
	else if (res == r)
	{
		ui->tableWidget->item(res, 1)->setText(r2);
		return;
	}
	else
	{
		QMessageBox msg(QMessageBox::Icon::Warning, "警告", "该命令已存在，是否覆盖现有的设置？");
		msg.addButton("保存", QMessageBox::AcceptRole);
		msg.addButton("关闭", QMessageBox::RejectRole);
		int i = msg.exec();
		if (i == QMessageBox::AcceptRole)
		{
			ui->tableWidget->item(res, 1)->setText(r2);
			ui->tableWidget->removeRow(r);
			return;
		}
		else if (i == QMessageBox::DestructiveRole)
		{
			return;
		}
	}

}
