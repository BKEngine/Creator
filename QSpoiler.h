#include <QFrame>
#include <QGridLayout>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <QToolButton>
#include <QWidget>

class QSpoiler : public QWidget {
	Q_OBJECT
private:
	QGridLayout mainLayout;
	QToolButton toggleButton;
	//QFrame headerLine;
	QParallelAnimationGroup toggleAnimation;
	QScrollArea contentArea;
	int animationDuration{ 300 };
public:
	explicit QSpoiler(const QString & title = "", const int animationDuration = 300, QWidget *parent = 0);
	void setContentLayout(QLayout *contentLayout);
};