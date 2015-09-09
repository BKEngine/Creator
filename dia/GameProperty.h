#ifndef GAMEPROPERTY_H
#define GAMEPROPERTY_H

#include <QWidget>
#include "ui_GameProperty.h"
#include "LangOpt.h"
#include "CommonSettingDia.h"

class GameProperty : public QDialog
{
	Q_OBJECT

public:
	GameProperty(QWidget *parent = 0);
	~GameProperty();

private:
	Ui::GameProperty ui;

	CLangEdit *langOption;

	QList<CommonSettingDia*> set;

public slots:
	void downOK();
	void resetCurrent();
	void resetAll();

};

#endif // GAMEPROPERTY_H
