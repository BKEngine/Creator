#include "qsearchlineedit.h"
#include "../mainwindow.h"


QSearchLineEdit::QSearchLineEdit(QWidget *parent )
    :QToolBar(parent)
{
    setStyleSheet("QToolBar{ border-top:0px solid #000000}");
    history = BKE_USER_SETTING->value("SearchHistory",QStringList()).toStringList() ;

    btnrecord = new QToolButton(this) ;
    btnrecord->setIcon( QIcon(":/left/source/search1.png") );
    btnrecord->setText("历史记录");

    QToolButton *lks = new QToolButton(this) ;
    lks->setIcon( QIcon(":/left/source/close2.png") );
    lks->setText("清除");
    lks->setStyleSheet( BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT+"/searchclear").toString());

    edittext = new QComboBox(this) ;
    edittext->setEditable(true);
    edittext->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    edittext->setStyleSheet(BKE_SKIN_SETTING->value(BKE_SKIN_CURRENT+"/searchbox").toString());
    edittext->setView(new QListView());
    edittext->addItems(history);


    addWidget(btnrecord) ;
    addWidget(edittext) ;
    btnclose = addWidget(lks) ;
    btnclose->setVisible(false);

    connect(lks,SIGNAL(clicked()),edittext->lineEdit(),SLOT(clear())) ;
    connect(edittext->lineEdit(),SIGNAL(textChanged(QString)),this,SLOT(CheckText(QString))) ;
    connect(edittext->lineEdit(),SIGNAL(returnPressed()),this,SLOT(SearchText())) ;
    connect(btnrecord,SIGNAL(clicked()),this,SLOT(showHistory())) ;

}

void QSearchLineEdit::CheckText(const QString &text)
{
    if( text.isEmpty() ){
        btnclose->setVisible(false);
        projectedit->ClearFindItems();
    }
    else{
        btnclose->setVisible(true);
    }
}

void QSearchLineEdit::SearchText()
{
    projectedit->ShowFindItems(edittext->currentText());
    addHistory(edittext->currentText());
}

void QSearchLineEdit::addHistory(const QString &text)
{
    history.removeOne(text) ;
    history.prepend(text);
    while( history.count() > 15) history.takeLast() ;
    BKE_USER_SETTING->setValue("SearchHistory",history);
}

void QSearchLineEdit::showHistory()
{
    edittext->showPopup();
}
