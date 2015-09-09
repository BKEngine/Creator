#ifndef CLANGEDIT_H
#define CLANGEDIT_H

#include "bkeSci/qscilexerbkescript.h"
#include "CommonSettingDia.h"
#include <QWidget>
#include <QColorDialog>

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

public slots:
	void onAddClick();
	void onRemoveClick();
	void onEditClick();
};

#endif // CLangEdit_H
