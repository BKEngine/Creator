#include "cconfigdia.h"

CConfigdia::CConfigdia(QWidget *parent) :
    QDialog(parent)
{
    resize(700,500);
    //setAttribute(Qt::WA_DeleteOnClose, true);

    itemlist = new QListWidget(this) ;
    stk      = new QStackedWidget(this) ;
    QHBoxLayout *hh = new QHBoxLayout ;
    hh->addWidget(itemlist,1);
    hh->addWidget(stk,3);
    setLayout(hh);

    itemlist->addItem("文本编辑器");
    connect(itemlist,SIGNAL(currentRowChanged(int)),this,SLOT(itemChange())) ;
    low = -1 ;
    itemlist->setCurrentRow(0);
}

void CConfigdia::itemChange()
{
    if( low == itemlist->currentRow() ) return ;

    low = itemlist->currentRow() ;
    if( low == 0 ){
        CTextEdit *cns = new CTextEdit ;
        stk->addWidget(cns) ;
        stk->setCurrentIndex(0);
        cns->resize(stk->size());
    }
}
