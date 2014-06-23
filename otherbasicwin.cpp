#include "otherbasicwin.h"
#include "mainwindow.h"

OtherBasicWin::OtherBasicWin(QWidget *parent)
    :QWidget(parent)
{
    QHBoxLayout *hl = new QHBoxLayout ;
    hl->setContentsMargins(0,0,0,0);
    setLayout(hl);

    QToolBar *bar = new QToolBar ;
    bar->setMovable(false);
    bar->setFixedHeight(24);

    winlist = new QComboBox(this) ;

    winlist->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    winlist->addItems(QStringList()<<" 项目"<<" 打开文档"<<" 书签"<<" 标记"<<" 标签区段");
    winlist->setStyleSheet(BKE_QCSS_OBJECT.value("combox").toString());
    winlist->setView(new QListView());

    connect(winlist,SIGNAL(currentTextChanged(QString)),this,SLOT(ChangeShow(QString))) ;

    bar->addWidget(winlist) ;
    QToolButton *llm = new QToolButton(this) ;
    llm->setIcon(QIcon(":/left/source/column.png"));
    llm->setText("分栏");
    llm->setStyleSheet(BKE_QCSS_OBJECT.value("toolbtn").toString());
    bar->addWidget(llm) ;
    connect(llm,SIGNAL(clicked()),this,SLOT(AddWindow())) ;

    llm = new QToolButton(this) ;
    llm->setIcon(QIcon(":/left/source/close2.png"));
    llm->setText("关闭");
    llm->setStyleSheet(BKE_QCSS_OBJECT.value("toolbtn").toString());
    bar->addWidget(llm) ;
    connect( llm,SIGNAL(clicked()),this,SLOT(close())) ;
    bar->setStyleSheet(BKE_QCSS_OBJECT.value("toolbar").toString());

    //setAttribute(Qt::WA_DeleteOnClose);
    hl->setMenuBar(bar);
    currentWidget = 0 ;

}

void OtherBasicWin::AddWindow()
{

    for( int i = 1 ; i < ras[0]->count() ; i++){
        if( ras[0]->widget(i)->isHidden() ){
            ras[0]->widget(i)->show();
            return ;
        }
    }

    OtherBasicWin *llm = new OtherBasicWin ;
    ras[0]->insertWidget(ras[0]->indexOf(this)+1,llm);
}

void OtherBasicWin::ChangeShow(const QString &base)
{
    if( base != winlist->currentText() ){
        winlist->setCurrentText(base);
        return ;
    }

    if( base == " 项目") SetCurrentWidget(projectedit);
    else if( base == " 打开文档") SetCurrentWidget(fileListWidget);
    else if( base == " 书签") SetCurrentWidget(0);
    else if( base == " 标记") SetCurrentWidget(0);
    else if( base == " 标签区段") SetCurrentWidget(0);
}

void OtherBasicWin::closeEvent(QCloseEvent *e)
{
    int k = ras[0]->count() ;
    for( int i = 0 ; i < ras[0]->count() ; i++){
        if( ras[0]->widget(i)->isHidden() ) k-- ;
    }

    if( k < 3){
        ras[0]->hide();
        e->ignore();
    }
}

void OtherBasicWin::SetCurrentWidget( QWidget *w )
{
    if( currentWidget != 0){
        layout()->removeWidget(currentWidget);
    }
    if( w != 0){
        layout()->addWidget(w);
        w->show();
    }
    currentWidget = w ;
}
