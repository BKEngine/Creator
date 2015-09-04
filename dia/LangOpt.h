#ifndef CLANGEDIT_H
#define CLANGEDIT_H

#include "bkeSci/qscilexerbkescript.h"
#include <QWidget>
#include <QColorDialog>

namespace Ui {
	class CLangEdit;
}

class CLangEdit : public QWidget
{
	Q_OBJECT

public:
	explicit CLangEdit(QWidget *parent = 0);
	~CLangEdit();

	int itemNo;

	public slots:
	void onForecolorClicked();
	void onBackcolorClicked();
	void onOKClicked();
	void onSave();
	void onCopy();
	void SetCurrentStyle(QString stylename);

signals:
	void onOK();
	void onRefreshLexer();

private:
	Ui::CLangEdit *ui;
	QsciLexerBkeScript *lex;
	QColorDialog *cdia;
	int curindex;

	void upColour(int Row = -1);
	bool CheckBtn();

	private slots:
	void configchange(int ci);
	void itemchange(int index);
	void upFont();

private:
	QColor setcolor;
};

#endif // CLangEdit_H
