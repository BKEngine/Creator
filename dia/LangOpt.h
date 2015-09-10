#ifndef CLANGEDIT_H
#define CLANGEDIT_H

#include "bkeSci/qscilexerbkescript.h"
#include "CommonSettingDia.h"
#include <QWidget>
#include <QColorDialog>
#include <QTableWidgetItem>

namespace Ui {
	class CLangEdit;
}

class CLangEdit : public QWidget, public CommonSettingDia
{
	Q_OBJECT

public:
	explicit CLangEdit(QWidget *parent = 0);
	~CLangEdit();

	virtual void save();
	virtual void reset();
	virtual void load();

private:
	Ui::CLangEdit *ui;

	void removeRow(const QString &tag);
	int findRow(const QString &tag);
	void sortLast();

public slots:
	void onAddClick();
	void onRemoveClick();

	void onDoubleClick(QTableWidgetItem*);
};

#endif // CLangEdit_H
