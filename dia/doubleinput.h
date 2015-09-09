#ifndef DOUBLEINPUT_H
#define DOUBLEINPUT_H

#include <QDialog>

namespace Ui {
class DoubleInput;
}

class DoubleInput : public QDialog
{
	Q_OBJECT

public:
	explicit DoubleInput(QWidget *parent = 0);
	~DoubleInput();

	void setText(const QString &l1, const QString &t1, const QString &l2, const QString &t2);

	void waitResult(QString &r1, QString &r2);

private:
	Ui::DoubleInput *ui;
};

#endif // DOUBLEINPUT_H
