#include <weh.h>

#include <QDialog>

namespace Ui {
	class Dialog;
}

class WaitWindow :public QDialog
{
	Q_OBJECT
public:
	WaitWindow(QWidget *parent = 0);

	~WaitWindow();

	void setInfo(const QString &s, int max);

	void setNum(int n);

	void addNum();

private:
	Ui::Dialog *ui;

	int m_cur;
	int m_max;
	QString in;
};