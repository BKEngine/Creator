#ifndef CSKINOPTION_H
#define CSKINOPTION_H

#include <QWidget>
#include <QInputDialog>
#include <QMessageBox>
#include "function.h"
#include "CommonSettingDia.h"

namespace Ui {
	class CSkinOption;
}

class CSkinOption : public QWidget, public CommonSettingDia
{
	Q_OBJECT

public:
	explicit CSkinOption(QWidget *parent = 0);
	~CSkinOption();

	QStringList StyleList();
	QStringList StyleUnit(QString stylename);
	QString TypeText(QString text);

	bool isChange;

	virtual void save();
	virtual void reset();
	virtual void load();

	public slots:
	void StyleChange(const QString &stylename);
	void UnitChange(const QString &part);
	void SaveStyle();
	void NewStyle();
	void DelStyle();
private:
	Ui::CSkinOption *ui;
};

#endif // CSKINOPTION_H
