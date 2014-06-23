#include "qsearchlineedit.h"
#include "mainwindow.h"


QSearchLineEdit::QSearchLineEdit(const QString &backtext , QWidget *parent) :
    QLineEdit(parent)
{

    BackText = backtext ;

    btns = new QToolButton(this) ;
    btns->setIcon(QIcon(":/left/source/search1.png"));
    btns->setText("历史记录");
    setStyleSheet(BKE_QCSS_OBJECT.value("lineedit").toString());
    connect(this,SIGNAL(returnPressed()),this,SLOT(CheckText())) ;
}

void QSearchLineEdit::CheckText()
{
    QString tt = this->text() ;

    ils.clear();
    ils = projectedit->findItems(tt,Qt::MatchRecursive|Qt::MatchWildcard|Qt::MatchFixedString) ;

    if( ils.isEmpty() ) return ;

    listwidget = new QListWidget ;
    listwidget->setAttribute(Qt::WA_DeleteOnClose);
    for( int i = 0 ; i < ils.size() ; i++){
        QListWidgetItem *le = new QListWidgetItem(ils.at(i)->icon(0),ils.at(i)->text(0)) ;
        le->setData(Qt::UserRole,i);
        listwidget->addItem(le);
    }

    connect(listwidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(ItemDClick(QListWidgetItem*))) ;
    listwidget->setWindowTitle("查找结果");
    listwidget->show();
}


void QSearchLineEdit::ItemDClick(QListWidgetItem* le)
{
    projectedit->setCurrentItem(ils.at(le->data(Qt::UserRole).toInt()));
}
