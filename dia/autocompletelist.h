#ifndef AUTOCOMPLETELIST_H
#define AUTOCOMPLETELIST_H

#include <QFrame>
#include <memory>
#include <QImage>
#include <QIcon>
#include <QMap>
#include <QFont>
#include "QFuzzyMatcher/QFuzzyMatcher.h"

namespace Ui {
class AutoCompleteList;
}
class QListWidgetItem;

class AutoCompleteList : public QFrame
{
    Q_OBJECT

public:
    explicit AutoCompleteList(QWidget *parent = 0);
	~AutoCompleteList();
	void SetList(const QList<QPair<QString, int>> &list);
	void DefineIcon(int id, const QIcon &image);
	void SetFont(const QFont &font);
	void SetStops(const QString &stops);

	bool OnKeyPress(int key);

public slots:
	void Start(const QPoint &pos);
	void Cancel();
	void Match(const QString &str);

signals:
	void OnSelected(QString);
	void OnCanceled();
	void RequestRestart();

private slots:
	void ItemClicked(QListWidgetItem *);

private:
	void UpdateWidgetSize();

private:
    Ui::AutoCompleteList *ui;
	int minWidth;
	int maxHeight;
	QFontMetrics fm;
	QPoint pos;
	QString stops;
	QMap<int, QIcon> icons;
	QStringList alternatives;
	QHash<QString, int> typeList;
	QHash<QString, QString> map;
	QStringList matches;
	std::unique_ptr<QFuzzyMatcher> matcher;
	MatcherOptions options;
};

#endif // AUTOCOMPLETELIST_H
