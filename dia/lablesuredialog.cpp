#include "lablesuredialog.h"

LableSureDialog::LableSureDialog(QWidget *parent) :
    QDialog(parent)
{
    h1 = new QVBoxLayout ;
    h2 = new QHBoxLayout ;
    v1 = new QVBoxLayout ;
    v2 = new QVBoxLayout ;
    v1->setSpacing(10);
    v2->setSpacing(10);
    h1->setSpacing(10);
    h2->setSpacing(10);
    cwidget = 0 ;
    result = -1 ;
    defbtn = 0 ;
}


void LableSureDialog::SetLable(const QString &text)
{
    LableText = text ;
}

void LableSureDialog::SetCheckbox(const QStringList &list)
{
    checklist = list ;
    qchklist.clear();
    for( int i = 0 ; i < checklist.size() ; i++){
        QCheckBox *c = new QCheckBox(checklist.at(i),this) ;
        qchklist << c ;
    }
}

void LableSureDialog::SetBtn(const QStringList &list)
{
    btnlist = list ;
}


//是否被选择
bool LableSureDialog::IsCheckboxChoise(int index)
{
    return qchklist.at(index)->isChecked() ;
}


int  LableSureDialog::WaitUser(int w , int h )
{
    result = -1 ;
    h1->addWidget( new QLabel(LableText,this) );
    if( cwidget != 0) h1->addWidget(cwidget) ;

    for( int i = 0 ; i < btnlist.size() ; i++){
        QPushButton *p = new QPushButton(btnlist.at(i),this) ;
        h2->addWidget(p);
        connect(p,SIGNAL(pressed()),this,SLOT(BtnIsDown())) ;
        connect(p,SIGNAL(clicked()),this,SLOT(BtnIsClick())) ;
        qbtnlist << p ;
    }

    for( int i = 0 ; i < qchklist.size() ; i++){
        v1->addWidget(qchklist.at(i));
    }

    v2->addLayout(h1);
    v2->addLayout(v1);
    v2->addLayout(h2);
    setLayout(v2);
    resize(w,h);

    if( defbtn >= 0 && defbtn < qbtnlist.size() ) qbtnlist.at(defbtn)->setFocus();
    exec() ;
    return result ;
}


void LableSureDialog::BtnIsDown()
{
    for( int i = 0 ; i < qbtnlist.size() ; i++){
        if( qbtnlist.at(i)->isDown() ){
            result = i ;
        }
    }
}

void LableSureDialog::BtnIsClick()
{
    close() ;
}

void LableSureDialog::SetCheckboxAble(int index,bool is)
{
    qchklist.at(index)->setEnabled(is);
}
