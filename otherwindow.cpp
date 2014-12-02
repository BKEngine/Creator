#include "otherwindow.h"

OtherWindow::OtherWindow(QWidget *parent)
    :TopBarWindow(parent)
{ 
    bc1 = QColor(0xc7,0xed,0xcc) ;
    bc2 = QColor(0xdc,0xe2,0xf1) ;

    stackw = new QStackedWidget(this) ;
    ProblemList = new QListWidget(this) ;
    searchlist =  new QListWidget(this)  ;
    compileedit =  new QsciScintilla(this)  ;
    bookmarklist =  new QListWidget(this)  ;
    marklist = new QListWidget(this) ;
    compileedit->setUtf8(true);

    stackw->addWidget(ProblemList) ;
    stackw->addWidget(searchlist) ;
    stackw->addWidget(compileedit) ;
    stackw->addWidget(bookmarklist) ;
    stackw->addWidget(marklist) ;
    SetCentreWidget(stackw);
    lastbtn = 0 ;

    icoerror = new QIcon(":/info/source/error.png") ;
    icowarning = new QIcon(":/info/source/alert.png") ;
    icobookmark = new QIcon(":/cedit/source/Bookmark.png") ;

    btnproblem = new QPushButton("问题",this) ;
    btnsearch = new QPushButton("搜索结果",this) ;
	btncompiletext = new QPushButton("编译输出", this);
	btnbookmark = new QPushButton("书签", this);
	btnmark = new QPushButton("标记", this);

    connect(btnproblem,SIGNAL(clicked()),this,SLOT(WINproblem())) ;
    connect(btnsearch,SIGNAL(clicked()),this,SLOT(WINsearch())) ;
    connect(btncompiletext,SIGNAL(clicked()),this,SLOT(WINcomile())) ;
    connect(btnbookmark,SIGNAL(clicked()),this,SLOT(WINbookmark())) ;
    connect(btnmark,SIGNAL(clicked()),this,SLOT(WINmark())) ;
    connect(ProblemList,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(ProblemDoubleClicked(QListWidgetItem*))) ;
    connect(bookmarklist,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(BookMarkDoubleClicked(QListWidgetItem*))) ;
    connect(marklist,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(MarkDoubleClicked(QListWidgetItem*))) ;

    emap[btnproblem] = ProblemList ;
    emap[btnsearch] = searchlist ;
    emap[btncompiletext] = compileedit ;
    emap[btnbookmark] = bookmarklist ;
    emap[btnmark] = marklist ;
}

void OtherWindow::WINproblem(){ IfShow(btnproblem) ;}
void OtherWindow::WINsearch(){ IfShow(btnsearch) ;}
void OtherWindow::WINcomile(){ IfShow(btncompiletext) ;}
void OtherWindow::WINbookmark(){ IfShow(btnbookmark) ;}
void OtherWindow::WINmark(){ IfShow(btnmark) ;}

void OtherWindow::IfShow(QPushButton *btn,bool must)
{
    if( lastbtn != 0) lastbtn->setStyleSheet("");
    if( must ) lastbtn = 0 ;

    if( btn != lastbtn && this->isHidden()){ //切换不同的状态，并且窗口是隐藏的，则显示
        stackw->setCurrentWidget(emap.value(btn));
        this->show();
    }
    else if( btn != lastbtn ){         //切换不同的状态，并且窗口不是隐藏的，切换当前页
        stackw->setCurrentWidget(emap.value(btn));
    }
    else{
        this->hide();  //同一个状态按钮再点一次，则隐藏
        lastbtn = 0 ;
        return ;
    }

    lastbtn = btn ;
    btn->setStyleSheet("background: #808080");

    if( btn == btnproblem )   ;
    else if( btn == btnbookmark ) ;
    else if( btn == btnmark )   ;
}



//定位问题处
void OtherWindow::ProblemDoubleClicked(QListWidgetItem * item)
{
    emit Location( memproblem.at(ProblemList->currentRow()),problemdir );
}

void OtherWindow::BookMarkDoubleClicked(QListWidgetItem * item)
{
    emit Location( membookmark.at(bookmarklist->currentRow()),"" );
}

void OtherWindow::MarkDoubleClicked(QListWidgetItem * item)
{
    emit Location( memmark.at(marklist->currentRow()),"" );
}


void OtherWindow::ShowProblem(BkeMarkList *list,const QString &dir)
{
    problemdir = dir ;
    ProblemList->clear();

    BkeMarkerBase *akb ;
    QString temp ;

    errorcount = list->size() ;
    for( int i = 0 ; i < list->size() ; i++){
        akb = list->at(i) ;
        if( akb->Type == 1) AddItem(akb,icoerror,ProblemList);
        else AddItem(akb,icowarning,ProblemList);
    }

     temp.setNum(errorcount) ;
     btnproblem->setText("问题("+temp+")");

     //复制一个拷贝，以免原列表被删除
    memproblem = *list ;
    stackw->setCurrentWidget(ProblemList);
    if( errorcount > 0 ) IfShow(btnproblem,true);
}


void OtherWindow::AddItem(BkeMarkerBase *marker, QIcon *ico,QListWidget* w)
{
    QString temp ;
    QListWidgetItem *le = new QListWidgetItem ;
    le->setIcon(*ico);
    temp.setNum(marker->Atpos) ;
    if( marker->Name.isEmpty() ){
        QString a = marker->FullName ;
        a = a.right(a.length()-a.lastIndexOf('/')-1) ;
        temp.prepend("【 &_"+ a +"】 第") ;
    }
    else temp.prepend("【"+marker->Name+"】 第") ;
    temp.append("行："+marker->Information) ;
    le->setText(temp);
    w->addItem(le);
}

void OtherWindow::RefreshBookMark(BkeMarkList *b)
{
    membookmark.clear();
    membookmark = *b ;
    bookmarklist->clear();
    for( int i = 0 ; i < membookmark.size() ; i++){
        AddItem(membookmark.at(i),icobookmark,bookmarklist);
    }
}


