#ifndef CDIROPTION_H
#define CDIROPTION_H

#include <QWidget>

namespace Ui {
class CDirOption;
}

class CDirOption : public QWidget
{
    Q_OBJECT

public:
    explicit CDirOption(QWidget *parent = 0);
    ~CDirOption();

private:
    Ui::CDirOption *ui;
};

#endif // CDIROPTION_H
