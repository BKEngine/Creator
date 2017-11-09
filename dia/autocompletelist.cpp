#include "autocompletelist.h"
#include "ui_autocompletelist.h"
#include <QPinyin/QPinyin.h>

AutoCompleteList::AutoCompleteList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AutoCompleteList)
{
    ui->setupUi(this);
	connect(ui->listWidget, &QListWidget::itemClicked, this, &AutoCompleteList::ItemClicked);
}

AutoCompleteList::~AutoCompleteList()
{
    delete ui;
}

void AutoCompleteList::ItemClicked(QListWidgetItem *)
{
	emit OnSelected(matches[ui->listWidget->currentRow()]);
	this->hide();
}

void AutoCompleteList::SetList(const QList<QPair<QString, int>> &list)
{
	typeList.clear();
	map.clear();
	QStringList alternatives;
	for (auto &&pair : list)
	{
		alternatives << pair.first;
		typeList.insert(pair.first, pair.second);
	}
	QPinyin::ExtractPinyinToMap(alternatives, map);
	matcher.reset(new QFuzzyMatcher(map.keys()));
	this->alternatives = alternatives;
}

void AutoCompleteList::DefineIcon(int id, const QIcon & icon)
{
	icons.insert(id, icon);
}

void AutoCompleteList::SetFont(const QFont & font)
{
	ui->listWidget->setFont(font);
}

void AutoCompleteList::SetStops(const QString & stops)
{
	this->stops = stops;
}

bool AutoCompleteList::OnKeyPress(int key)
{
	if (key == Qt::Key_Escape)
	{
		Cancel();
	}
	else if (key == Qt::Key_Return || key == Qt::Key_Enter)
	{
		emit OnSelected(matches[ui->listWidget->currentRow()]);
		this->hide();
	}
	else if (key == Qt::Key_Down)
	{
		ui->listWidget->setCurrentRow(ui->listWidget->currentRow() + 1);
	}
	else if (key == Qt::Key_Up)
	{
		ui->listWidget->setCurrentRow(ui->listWidget->currentRow() -1);
	}
	else
	{
		return false;
	}
	return true;
}

void AutoCompleteList::Cancel()
{
	this->hide();
	emit OnCanceled();
}

void AutoCompleteList::Start(const QPoint &pos)
{
	this->move(pos);
	this->show();
}

void AutoCompleteList::Match(const QString &str)
{
	if (!str.isEmpty() && stops.contains(str.right(1)))
	{
		this->hide();
		emit OnCanceled();
		return;
	}
	QStringList qs;
	if (str.isEmpty())
	{
		qs = alternatives;
	}
	else
	{
		QVector<MatchResult> results = matcher->findMatches(str, options);
		for (auto && result : results)
		{
			auto &&ss = map[*result.value];
			if (!qs.contains(ss))
			{
				qs.append(ss);
			}
		}
	}
	if (qs.empty())
		this->hide();
	else if (qs.size() == 1 && qs[0] == str)
		this->hide();
	else
	{
		this->show();
		int rows = ui->listWidget->count();
		for (int i = 0; i < qs.length(); i++)
		{
			QListWidgetItem *item;
			if (i < rows)
			{
				item = ui->listWidget->item(i);
			}
			else
			{
				item = new QListWidgetItem();
				ui->listWidget->addItem(item);
			}
			int type = typeList.value(qs[i]);
			if (type > 0)
				item->setIcon(icons.value(type));
			else
				item->setIcon(QIcon());
			item->setText(qs[i]);
		}
		for (int i = qs.length(); i < rows; i++)
		{
			delete ui->listWidget->takeItem(qs.length());
		}
		ui->listWidget->setCurrentRow(0);
	}
	matches = qs;
}