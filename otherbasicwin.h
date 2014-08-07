#ifndef OTHERBASICWIN_H
#define OTHERBASICWIN_H

#include <QtCore>
#include <QAction>
#include <QToolButton>
#include <QCloseEvent>
#include "weh.h"
#include "projectwindow.h"
#include "dia/bkeleftfilewidget.h"


extern ProjectWindow *projectedit ;
extern BkeLeftFileWidget *fileListWidget ;
extern QSplitter *ras[3] ;

class OtherBasicWin :public QWidget
{
    Q_OBJECT
public:
    OtherBasicWin(QWidget *parent = 0);
    QStackedWidget *sk ;

    QComboBox *winlist ;
    QWidget *currentWidget ;


signals:
    void NewOtherBase(const QString &base,int pos) ;
public slots:
    void AddWindow() ;
    void ChangeShow(const QString &base) ;
private:

protected:
    void closeEvent(QCloseEvent *e);
    void SetCurrentWidget( QWidget *w = 0) ;
};

#endif // OTHERBASICWIN_H
