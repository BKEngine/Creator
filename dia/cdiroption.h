#ifndef CDIROPTION_H
#define CDIROPTION_H

#include <QWidget>
#include "CommonSettingDia.h"

namespace Ui {
class CDirOption;
}

class CDirOption : public QWidget, public CommonSettingDia
{
    Q_OBJECT

public:
    explicit CDirOption(QWidget *parent = 0);
    ~CDirOption();

	virtual void save();
	virtual void load();
	virtual void reset();

private:
    Ui::CDirOption *ui;
};

#endif // CDIROPTION_H
