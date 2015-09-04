#ifndef SETOPTIONDIA_H
#define SETOPTIONDIA_H

#include <QDialog>
#include "dia/ctextedit.h"
#include "dia/cdiroption.h"
#include "dia/cskinoption.h"
#include "dia/CommonSettingDia.h"

namespace Ui {
class SetOptionDia;
}

class SetOptionDia : public QDialog
{
	Q_OBJECT

public:
	explicit SetOptionDia(QWidget *parent = 0);
	~SetOptionDia();

public slots:
	void downOK() ;
	void resetCurrent();
	void resetAll();
//    void ItemChange(int i) ;

private:
	Ui::SetOptionDia *ui;
	CTextEdit *CEditOption ;
	CDirOption *dirOption ;
	CSkinOption *skinOption ;

	QList<CommonSettingDia*> set;
};

#endif // SETOPTIONDIA_H
