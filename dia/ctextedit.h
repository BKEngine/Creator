#ifndef CTEXTEDIT_H
#define CTEXTEDIT_H

#include "bkeSci/qscilexerbkescript.h"
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
    QsciLexerBkeScript *lex ;

    void upColour() ;

private slots:
    void configchange(int ci) ;
    void itemchange(int index) ;

};

#endif // CTEXTEDIT_H
