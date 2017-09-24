#ifndef OPENLABELDIALOG_H
#define OPENLABELDIALOG_H

#include <QDialog>
#include <QHash>
#include <set>
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
    explicit OpenLabelDialog(const std::set<QString> &labels, QWidget *parent = 0);
    ~OpenLabelDialog();
	void setLabels(const std::set<QString> &labels);

signals:
	void GotoLabel(QString label);

private slots:
	void LineEditTextChanged(QString);
	void OnOK();

protected:
	bool eventFilter(QObject *, QEvent *) override;

private:
    Ui::OpenLabelDialog *ui;

	std::set<QString> labels;
	QHash<QString, QString> map;
	std::unique_ptr<QFuzzyMatcher> matcher;
	MatcherOptions options;

	QStringList tableContents;
};

#endif // OPENLABELDIALOG_H
