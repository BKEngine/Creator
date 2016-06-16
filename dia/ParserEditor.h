#ifndef PARSEREDITOR_H
#define PARSEREDITOR_H

#include <QMainWindow>

#include "ParserEditor.h"
#include "ParserEditorTreeModel.h"

namespace Ui {
class ParserEditor;
}

class QToolButton;
class ParserEditor : public QMainWindow
{
    Q_OBJECT
private:
	Ui::ParserEditor *ui;
	ParserEditorTreeModel *model;
	QString filepath;
	QString _error;
	QModelIndex _lastEnteredIndex;

	void removeButtons();
	void addButtons(const QModelIndex &index);

	QToolButton *x;
	QToolButton *a;
	QToolButton *e;
	void buildToolButtons();

private slots:
	void onTreeViewRClick(const QPoint &pt);
	void onIndexEntered(const QModelIndex &index);
	void onViewportEntered();
	void onXClicked();
	void onAClicked();
	void onEClicked();

public:
    explicit ParserEditor(const QString &filepath, QWidget *parent = 0);
	void save();
	void load();
	QString error() { return _error; }
};

#endif // NEWVERSIONDATAWIZARD_H
