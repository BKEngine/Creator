#ifndef OPENLABELDIALOG_H
#define OPENLABELDIALOG_H

#include <QDialog>
#include <QHash>
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
    explicit OpenLabelDialog(const QStringList &labels, QWidget *parent = 0);
    ~OpenLabelDialog();
	void setLabels(const QStringList &labels);

signals:
	void GotoLabel(QString label);

private slots:
	void LineEditTextChanged(QString);
	void OnOK();

protected:
	bool eventFilter(QObject *, QEvent *) override;

private:
    Ui::OpenLabelDialog *ui;

	QStringList labels;
	QHash<QString, QString> map;
	QStringList matches;
	std::unique_ptr<QFuzzyMatcher> matcher;
	MatcherOptions options;
};

#endif // OPENLABELDIALOG_H
