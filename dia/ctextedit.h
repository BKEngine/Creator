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

	int itemNo;

public slots:
	void onForecolorClicked();
	void onBackcolorClicked();
	void onOKClicked();
	void onSave();
	void resetColor();

signals:
	void onOK();
	void onRefreshLexer();

private:
    Ui::CTextEdit *ui;
    QsciLexerBkeScript *lex ;
	QColorDialog *cdia;
	int curindex;

    void upColour() ;
	void readConfig(const QString &name);

private slots:
    void configchange(int ci) ;
    void itemchange(int index) ;

private:
	QColor setcolor[20];
	QColor setcolor_b[20];
};

#endif // CTEXTEDIT_H
