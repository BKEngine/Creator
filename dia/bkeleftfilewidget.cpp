#include <weh.h>
#include "bkeleftfilewidget.h"

BkeLeftFileWidget::BkeLeftFileWidget(QWidget *parent) :
    QListWidget(parent)
{
    //setStyleSheet("QListWidget{border:0px}");
    setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT+"/filelist").toString());
    btnCloseAll = new QAction("关闭所有",this);
    btnclose = new QAction("关闭文件",this) ;
    btncloseExp = new QAction("关闭其他所有文件",this)  ;
    mn = new QMenu(this) ;
    mn->addAction(btnclose) ;
    mn->addAction(btnCloseAll) ;
    mn->addAction(btncloseExp) ;

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(ShowRmenu(QPoint))) ;
}

void BkeLeftFileWidget::SetFileList(const QStringList &ls)
{
    clear();
}

void BkeLeftFileWidget::ShowRmenu( const QPoint & pos )
{
    QListWidgetItem *le = currentItem() ;
    if( le == 0) return ;

    QPoint p = QCursor::pos() ;
    p.setX( pos.x()+10);
    mn->exec(p) ;
}
