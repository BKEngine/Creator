#ifndef TOPBARWINDOW_H
#define TOPBARWINDOW_H

#include "weh.h"

class TopBarWindow : public QWidget
{
    Q_OBJECT
public:
    explicit TopBarWindow(QWidget *parent = 0);

signals:

public slots:
    void SetCentreWidget(QWidget *childWidget) ;
    void addTopBtn( QWidget *btn, int w = 24 ) ;
    void SetToolBar(QToolBar *bar) ;

private:
    QWidget *Topbar ;
    QWidget *CenterWidget ;

    int topx ;

};

#endif // TOPBARWINDOW_H
