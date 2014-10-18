#include "bkeconfiguimodel.h"

BkeConfigUiModel::BkeConfigUiModel(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle("编辑配置文件");

    btnclose = new QPushButton("取消",this) ;
    btnsave = new QPushButton("保存",this) ;
    form = new QFormLayout(this) ;

    projectname = new QLineEdit("Bke Game",this) ;
    form->addRow("游戏名称*:",projectname);

    projectsize = new QLineEdit("800*600",this) ;
    form->addRow("游戏分辨率*:",projectsize);

    projecttitle = new QLineEdit(this) ;
    form->addRow("游戏标题栏标题:",projecttitle);

    form->addRow("存档目录:",btnLineEdit("s1",&savedir));
    form->addRow("图片默认搜索目录:",btnLineEdit("s2",&imgdir));
    form->addRow("音频默认搜索目录:",btnLineEdit("s3",&audiodir));
    form->addRow("脚本默认搜索目录:",btnLineEdit("s4",&scriptdir));

    savenum = new QLineEdit(this) ;
    form->addRow("最大存档数目:", savenum );

    fontsize = new QLineEdit(this) ;
    form->addRow("默认字体大小:", fontsize );

    fontcolor = new QLineEdit("0xFFFFFF",this) ;
    form->addRow("默认字体颜色:", fontcolor );

    fontname = new QLineEdit(this) ;
    form->addRow("默认字体文件名:", fontname );

    debuglevel = new QLineEdit(this) ;
    form->addRow("Log等级:", debuglevel );

    form->setLabelAlignment(Qt::AlignRight);

    btnclose->setGeometry(380,370,100,35);
    btnsave->setGeometry(270,370,100,35);

    setLayout(form);

    connect(btnclose,SIGNAL(clicked()),this,SLOT(close())) ;
    connect(btnsave,SIGNAL(clicked()),this,SLOT(Sure())) ;
    resize(500,430);
    hide();
}

void BkeConfigUiModel::StartConfig(const QString &file,const QString &dir)
{
    Name = file ;
    Dir = dir ;
    ReadFile(file);
    this->exec() ;
}

void BkeConfigUiModel::ToScriptModel()
{
    hide();
    //emit FileOpen(Name,Dir);
    close() ;
}

void BkeConfigUiModel::Sure()
{
    QString result ;
    WordSupport w ;
    QPoint pt = this->pos();

    //是否有名称
    if( projectname->text().isEmpty() ) {
        callText(projectname,pt,"项目名称必须存在");
        return ;
    }

    result.append( "GameName =\"" + projectname->text() + "\";\r\n" ) ;

    //是否有分辨率
    w.setText( projectsize->text() );
    w.NextTwoWord();
    if( !w.IsNumber(w.pWord) || !w.IsNumber(w.nWord) ){
        callText(projectsize,pt,"无效的分辨率设置");
        return ;
    }

    if( !projecttitle->text().isEmpty() ) result.append("GameTitle=\""+projecttitle->text()+"\";\r\n") ;
    result.append("ResolutionSize=["+w.pWord+","+w.nWord+"];\r\n" ) ;
    if( !savedir->text().isEmpty() )  result.append("SaveDir=\""+savedir->text()+"\";\r\n") ;
    result.append( GetDirText(imgdir,"ImageAutoSearchPath = ") ) ;
    result.append( GetDirText(audiodir,"AudioAutoSearchPath = ") ) ;
    result.append( GetDirText(scriptdir,"ScriptAutoSearchPath = ") ) ;

    if( WordSupport::IsNumber(fontsize->text())) result.append("DefaultFontSize="+fontsize->text()+";\r\n") ;
    if( WordSupport::IsNumber(savenum->text())) result.append("MaxSaveDataNum="+savenum->text()+";\r\n") ;
    if( WordSupport::IsFontColor(fontcolor->text())) result.append("DefaultFontColor="+fontcolor->text()+";\r\n") ;
    if( !fontname->text().isEmpty() ) result.append("DefaultFontName=\""+fontname->text()+"\";\r\n") ;
    if( !debuglevel->text().isEmpty() ) result.append("DebugLevel="+debuglevel->text()+";\r\n") ;

    QFile ks( Name ) ;
    if( !LOLI::AutoWrite(&ks,result,"UTF-8")){
        QMessageBox::critical(this,"错误","保存"+ Name+ "时遇到了一个错误。",QMessageBox::Ok) ;
        return ;
    }
    ks.close();
    ks.setFileName(Dir+"/settings.bkpsr");
    ks.remove() ;
    close() ;
}

QHBoxLayout *BkeConfigUiModel::btnLineEdit(const QString &basename,QLineEdit **e)
{
    QHBoxLayout *h1 = new QHBoxLayout;
    QPushButton *p1 = new QPushButton("...",this) ;
    QLineEdit *e1 = new QLineEdit(this) ;
    p1->setFixedWidth(30);
    h1->addWidget(e1);
    h1->addWidget(p1);

    p1->setObjectName(basename+"_p");
    e1->setObjectName(basename+"_e");
    connect(p1,SIGNAL(clicked()),this,SLOT(UseDir())) ;
    *e = e1 ;
    return h1 ;
}

void BkeConfigUiModel::UseDir()
{
    QPushButton *p = dynamic_cast<QPushButton*>(sender()) ;

    QString obname = p->objectName() ;
    obname = obname.left(obname.length()-1)+"e" ;
     QLineEdit* edit= findChild<QLineEdit*>(obname) ;

    if( edit == 0) return ;

    QString name = QFileDialog::getExistingDirectory(this,"选择文件夹",Dir) ;
    if( name.isEmpty() ) return ;

    QString t = edit->text() ;
    name = BkeFullnameToName(name,Dir) ;
    if( !t.isEmpty() && !t.endsWith(",") ) t.append(","+name) ;
    else t.append(name) ;
    edit->setText( t );
}

void BkeConfigUiModel::ReadFile(const QString &file)
{
    LOLI::AutoRead(Text,file) ;
    if( Text.isEmpty() ) return ;

    QString temp ;
    temp = ToType( LOLI_KEY_VAL(Text,"GameName"));
    if( !temp.isEmpty() ) projectname->setText(temp);
    temp = ToType( LOLI_KEY_VAL(Text,"ResolutionSize"));
    if( !temp.isEmpty() ) projectsize->setText(temp);

    projecttitle->setText( ToType( LOLI_KEY_VAL(Text,"GameTitle")) );

    savedir->setText(  ToType( LOLI_KEY_VAL(Text,"SaveDir")) );
    imgdir->setText(ToType( LOLI_KEY_VAL(Text,"ImageAutoSearchPath")));
    audiodir->setText( ToType( LOLI_KEY_VAL(Text,"AudioAutoSearchPath")) )  ;
    scriptdir->setText( ToType( LOLI_KEY_VAL(Text,"ScriptAutoSearchPath")) ) ;
    savenum->setText( ToType( LOLI_KEY_VAL(Text,"MaxSaveDataNum")) ) ;
    fontsize->setText( ToType( LOLI_KEY_VAL(Text,"DefaultFontSize")) )  ;

    temp = ToType( LOLI_KEY_VAL(Text,"DefaultFontColor")) ;
    if( !temp.isEmpty() ) fontcolor->setText( temp ) ;
    fontname->setText( ToType( LOLI_KEY_VAL(Text,"DefaultFontName")) ) ;
    debuglevel->setText( ToType( LOLI_KEY_VAL(Text,"DebugLevel")) ) ;
}


QString BkeConfigUiModel::ToType(QString t)
{
    QString abc = t.trimmed() ;
    if( abc.startsWith('\"')) abc = abc.right(abc.length()-1) ;
    if( abc.endsWith('\"')) abc = abc.left(abc.length()-1) ;
    if( abc.startsWith('[') && abc.endsWith(']')){
        abc = abc.mid(1,abc.length()-2) ;
        QStringList ls = abc.split(",") ;
        QStringList lz ;
        for( int i = 0 ; i < ls.size() ; i++){
            abc = ToType(ls.at(i)) ;
            if( !abc.isEmpty() ) lz.append( abc );
        }
        abc = lz.join(",") ;
    }
    return abc ;
}

void BkeConfigUiModel::callText(QWidget *w, QPoint size,const QString &info)
{
    size.setX(w->x()+size.x());
    size.setY(size.y()+w->y());
    QToolTip::showText( size ,info,this) ;
    return ;
}

QString BkeConfigUiModel::GetDirText( QLineEdit *e,const QString &left)
{
    if( e->text().isEmpty() ) return QString();
    QString temp ;
    QStringList ls = e->text().split(",") ;
    if( ls.size() < 1) return temp;

    temp = "[" ;
    QString k ;
    for( int i = 0 ; i < ls.size() ; i++){
        k = ls.at(i) ;
        if( !k.startsWith('\"') ) k.prepend('\"') ;
        if( !k.endsWith('\"') ) k.append('\"') ;
        if( i < ls.size() -1 ) temp.append(k+",") ;
        else temp.append(k+"]") ;
    }

    temp.prepend(left) ;
    temp.append(";\r\n" ) ;
    return temp ;
}
