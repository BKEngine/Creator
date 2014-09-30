#ifndef CTEXTEDIT_H
#define CTEXTEDIT_H

#include <QWidget>

namespace Ui {
class CTextEdit;
}

class CTextEdit : public QWidget
{
    Q_OBJECT

public:
    explicit CTextEdit(QWidget *parent = 0);
    ~CTextEdit();

private:
    Ui::CTextEdit *ui;
};

#endif // CTEXTEDIT_H
