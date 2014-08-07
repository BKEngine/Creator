#include "topbarwindow.h"

TopBarWindow::TopBarWindow(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout *hs = new QVBoxLayout(this) ;
    CenterWidget = new QWidget(this) ;

    hs->setContentsMargins(0,0,0,0);
    hs->setSpacing(0);
    hs->addWidget(CenterWidget);
    setLayout(hs);

    topx = 0 ;

    this->setFocusPolicy(Qt::StrongFocus );
}

void TopBarWindow::SetCentreWidget(QWidget *childWidget)
{
    childWidget->setParent(this);
    childWidget->setFocusPolicy(Qt::StrongFocus );
    //childWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->layout()->addWidget(childWidget);
    this->layout()->removeWidget(CenterWidget);
    CenterWidget = childWidget ;
}

//往标题栏加控件
void TopBarWindow::addTopBtn( QWidget *btn , int w )
{
    btn->setParent(this);
    if(w>24) btn->setGeometry(topx,0,w,24);

    topx += w+1 ;
}

void TopBarWindow::SetToolBar(QToolBar *bar)
{
    this->layout()->setMenuBar(bar);
}
