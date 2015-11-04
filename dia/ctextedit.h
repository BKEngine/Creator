#ifndef CTEXTEDIT_H
#define CTEXTEDIT_H

#include "bkeSci/qscilexerbkescript.h"
#include "CommonSettingDia.h"
#include <QWidget>
#include <QColorDialog>

namespace Ui {
class CTextEdit;
}

class CTextEdit : public QWidget, public CommonSettingDia
{
	Q_OBJECT

public:
	explicit CTextEdit(QWidget *parent = 0);
	~CTextEdit();

	virtual void save();
	virtual void load();
	virtual void reset();

public slots:
	void onForecolorClicked();
	void onBackcolorClicked();
	void onOKClicked();
	void onSave();
	void onCopy() ;
	void SetCurrentStyle(QString stylename) ;

signals:
	void onOK();
	void onRefreshLexer();

private:
	Ui::CTextEdit *ui;
	QsciLexerBkeScript *lex ;
	QColorDialog *cdia;
	int curindex;

	void upColour(int Row = -1) ;
	bool CheckBtn() ;

private slots:
	void configchange(int ci) ;
	void itemchange(int index) ;
	void upFont() ;

private:
	QColor setcolor;
};

#endif // CTEXTEDIT_H
