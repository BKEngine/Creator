#ifndef CTEXTEDIT_H
#define CTEXTEDIT_H

#include "bkeSci/qscilexerbkescript.h"
#include <QWidget>
#include <QColorDialog>

namespace Ui {
class CTextEdit;
}

class CTextEdit : public QWidget
{
    Q_OBJECT

public:
    explicit CTextEdit(QWidget *parent = 0);
    ~CTextEdit();

public slots:
	void onForecolorClicked();
	void onBackcolorClicked();

private:
    Ui::CTextEdit *ui;
    QsciLexerBkeScript *lex ;
	QColorDialog *cdia;
	int curindex;

    void upColour() ;

private slots:
    void configchange(int ci) ;
    void itemchange(int index) ;

};

#endif // CTEXTEDIT_H
