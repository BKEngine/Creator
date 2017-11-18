#ifndef GOTOFILEDIALOG_H
#define GOTOFILEDIALOG_H

#include <QDialog>
#include <QHash>
#include <QStandardItemModel>
#include <memory>
#include "QFuzzyMatcher/QFuzzyMatcher.h"

namespace Ui {
class GotoFileDialog;
}

class GotoFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GotoFileDialog(const QStringList &files, QWidget *parent = 0);
    ~GotoFileDialog();
	void setFiles(const QStringList &files);

signals:
	void GotoFile(QString file);

private slots:
	void LineEditTextChanged(QString);
	void OnOK();
	void CellDoubleClicked(int row, int);

protected:
	bool eventFilter(QObject *, QEvent *) override;

private:
    Ui::GotoFileDialog *ui;

	QStringList files;
	QHash<QString, QString> map;
	QStringList matches;
	std::unique_ptr<QFuzzyMatcher> matcher;
	MatcherOptions options;

	QStringList tableContents;
};

#endif // GOTOFILEDIALOG_H
