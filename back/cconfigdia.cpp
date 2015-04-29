#include <weh.h>
#include "cconfigdia.h"

CConfigdia::CConfigdia(QWidget *parent) :
    QDialog(parent)
{
    resize(700,500);
    //setAttribute(Qt::WA_DeleteOnClose, true);

    itemlist = new QListWidget(this) ;
	stk = new QStackedWidget(this);
	QHBoxLayout *hh = new QHBoxLayout(this);
    hh->addWidget(itemlist,1);
    hh->addWidget(stk,3);
    setLayout(hh);
	cns = new CTextEdit(this);
	stk->addWidget(cns);

    itemlist->addItem("文本编辑器");
    connect(itemlist,SIGNAL(currentRowChanged(int)),this,SLOT(itemChange())) ;
    low = -1 ;
    itemlist->setCurrentRow(0);
}

void CConfigdia::itemChange()
{
    if( low == itemlist->currentRow() ) return ;

    low = itemlist->currentRow() ;

	switch (low)
	{
	case 0:
		connect(cns, SIGNAL(onOK()), this, SLOT(onOK()));
		stk->setCurrentIndex(0);
		cns->resize(stk->size());
		connect(this, SIGNAL(onSave()), cns, SLOT(onSave()));
		break;
    }
}

void CConfigdia::onOK()
{
    //emit onSave();
	close();
}

