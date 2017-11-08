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

}

void AutoCompleteList::SetList(const QList<QPair<QString, int>> &list)
{
	typeList.clear();
	map.clear();
	QStringList alternatives;
	this->alternatives = alternatives;
	for (auto &&pair : list)
	{
		alternatives << pair.first;
		typeList.insert(pair.first, pair.second);
	}
	QPinyin::ExtractPinyinToMap(alternatives, map);
	matcher.reset(new QFuzzyMatcher(map.keys()));
}

void AutoCompleteList::DefineIcon(int id, const QIcon & icon)
{
	icons.insert(id, icon);
}

void AutoCompleteList::Cancel()
{
	this->hide();
}

void AutoCompleteList::Start(const QPoint &pos)
{
	this->move(pos);
	this->show();
}

void AutoCompleteList::Match(const QString &str)
{
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
		}
		int type = typeList.value(qs[i]);
		if (type >= 0)
			item->setIcon(icons.value(type));
		else
			item->setIcon(QIcon());
		item->setText(qs[i]);
	}
	ui->listWidget->setCurrentRow(0);
	matches = qs;
}
