#ifndef OPENLABELDIALOG_H
#define OPENLABELDIALOG_H

#include <QDialog>
#include <QHash>
#include <QSortedSet>
#include <QStandardItemModel>
#include <QPointer>
#include <memory>
#include "QFuzzyMatcher/QFuzzyMatcher.h"

namespace Ui {
class OpenLabelDialog;
}

class OpenLabelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OpenLabelDialog(const QSortedSet<QString> &labels, QWidget *parent = 0);
    ~OpenLabelDialog();
	void setLabels(const QSortedSet<QString> &labels);

signals:
	void GotoLabel(QString label);

private slots:
	void LineEditTextChanged(QString);
	void OnOK();
	void CellDoubleClicked(int row, int);

protected:
	bool eventFilter(QObject *, QEvent *) override;

private:
    Ui::OpenLabelDialog *ui;

	QSortedSet<QString> labels;
	QHash<QString, QString> map;
	std::unique_ptr<QFuzzyMatcher> matcher;
	MatcherOptions options;

	QStringList tableContents;
};

#endif // OPENLABELDIALOG_H
