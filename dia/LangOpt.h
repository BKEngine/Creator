#ifndef CLANGEDIT_H
#define CLANGEDIT_H

#include "bkeSci/qscilexerbkescript.h"
#include <QWidget>
#include <QColorDialog>

namespace Ui {
	class CLangEdit;
}

class CLangEdit : public QWidget
{
	Q_OBJECT

public:
	explicit CLangEdit(QWidget *parent = 0);
	~CLangEdit();

private:
	Ui::CLangEdit *ui;

};

#endif // CLangEdit_H
