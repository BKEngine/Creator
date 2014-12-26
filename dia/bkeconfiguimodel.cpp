#include "bkeconfiguimodel.h"

BkeConfigUiModel::BkeConfigUiModel(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle("编辑配置文件");

    btnclose = new QPushButton("取消",this) ;
    btnsave = new QPushButton("保存",this) ;

    QVBoxLayout *mainLayout = new QVBoxLayout();
    form = new QFormLayout() ;

    projectname = new QLineEdit("Bke Game",this) ;
    form->addRow("游戏名称*:",projectname);

    projectsize = new QLineEdit("800, 600",this) ;
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

    live2dkey = new QLineEdit(this);
    form->addRow("Live2D授权码：", live2dkey);

    form->setLabelAlignment(Qt::AlignRight);

    mainLayout->addLayout(form);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnsave);
    btnLayout->addStretch();
    btnLayout->addWidget(btnclose);
    btnLayout->addStretch();
    btnsave->setMinimumSize(100,45);
    btnclose->setMinimumSize(100,45);

    mainLayout->addLayout(btnLayout);

    setLayout(mainLayout);

    connect(btnclose,SIGNAL(clicked()),this,SLOT(close())) ;
    connect(btnsave,SIGNAL(clicked()),this,SLOT(Sure())) ;
    resize(500,430);
    setMaximumSize(500,430);
    setMinimumSize(500,430);
    hide();
}

void BkeConfigUiModel::StartConfig(BkeProjectConfig *config)
{
    this->config = config;
    connect(config, SIGNAL(onFileChanged()), this, SLOT(refreshUI()));
    refreshUI();
    this->exec() ;
}

void BkeConfigUiModel::ToScriptModel()
{
    hide();
    //emit FileOpen(Name,Dir);
    close() ;
}

static QString Size2Str(int s[])
{
    return QString("%1, %2").arg(s[0]).arg(s[1]);
}

QString toType(const QString &t)
{
    QString abc = t.trimmed() ;
    if( abc.startsWith('\"')) abc = abc.right(abc.length()-1) ;
    if( abc.endsWith('\"')) abc = abc.left(abc.length()-1) ;
    if( abc.startsWith('[') && abc.endsWith(']')){
        abc = abc.mid(1,abc.length()-2) ;
        QStringList ls = abc.split(",") ;
        QStringList lz ;
        for( int i = 0 ; i < ls.size() ; i++){
            abc = toType(ls.at(i)) ;
            if( !abc.isEmpty() ) lz.append( abc );
        }
        abc = lz.join(",") ;
    }
    return abc ;
}

static bool Str2Size(int s[], const QString &str)
{
    QStringList l = str.split(',');
    if(l.size()!=2)
        return false;
    bool ok = false;
    s[0] = l[0].trimmed().toInt(&ok);
    if(!ok)
        return false;
    s[1] = l[1].trimmed().toInt(&ok);
    if(!ok)
        return false;
    return true;
}

static QString SearchPath2Str(const QStringList &v)
{
    return v.join(", ");
}

static QString Int2Hex(int a)
{
    char tmp[8];
    itoa(a, tmp, 16);
    QString num = QString(tmp).toUpper();
    if(num.size() < 6)
    {
        num.prepend(QString(6-num.size(),'0'));
    }
    return QString("0x") + num;
}

static QString Color2Str(const QBkeVariable &v)
{
    if(v.isNull())
        return QString();
    if(v.isArray())
        return Int2Hex(v[0].toInt()) + ", " + Int2Hex(v[1].toInt());
    return Int2Hex(v.toInt());
}

static int Color2Int(const QString &t)
{
    QString s = t.toLower() ;
    if( t.startsWith("0x") && s.length() == 8)
    {
        s = s.right(6);
        return s.toInt(0,16);
    }
    else if(t.startsWith("#"))
    {
        s = s.right(s.size()-1);
        switch(s.length())
        {
        case 3:
            s = QString(s[0]) + s[0] + s[1] + s[1] +s[2] + s[2];
        case 6:
            return s.toInt(0,16);
        }
    }
    return -1;
}

static QBkeVariable Str2Color(const QString &t)
{
    if(t.isEmpty())
    {
        return QBkeVariable();
    }
    QString s = t.toLower();
    if(WordSupport::IsColor(s))
        return QBkeVariable(Color2Int(s));

    int pos = s.indexOf(',');
    if(pos != -1)
    {
        QBkeVariable v;
        v[0] = Color2Int(s.mid(0,pos).trimmed());
        v[1] = Color2Int(s.mid(pos + 1).trimmed());
        return v;
    }
    return QBkeVariable();
}

static QStringList getDirText( QLineEdit *e)
{
    if( e->text().isEmpty() ) return QStringList();
    QStringList ls = e->text().split("|") ;
    for(auto &i : ls)
    {
        i = i.trimmed();
    }
    return ls;
}

void BkeConfigUiModel::Sure()
{
    QPoint pt = this->pos();
    BkeProjectConfig tmp;

    //是否有名称
    if( projectname->text().isEmpty() ) {
        callText(projectname,pt,"项目名称必须存在");
        return ;
    }

    tmp.projectName = projectname->text();

    //是否有分辨率
    if(!Str2Size(tmp.resolutionSize, projectsize->text())){
        callText(projectsize,pt,"无效的分辨率设置");
        return ;
    }

    tmp.gameTitle = projecttitle->text();
    tmp.saveDir = savedir->text();
    tmp.imageAutoSearchPath = getDirText(imgdir);
    tmp.audioAutoSearchPath = getDirText(audiodir);
    tmp.scriptAutoSearchPath = getDirText(scriptdir);

    if( WordSupport::IsNumber(fontsize->text()))
        tmp.defaultFontSize = fontsize->text().toInt();
    else{
        callText(fontsize,pt,"无效的数字");
        return ;
    }
    if( WordSupport::IsNumber(savenum->text()))
        tmp.maxSaveDataNum = savenum->text().toInt();
    else{
        callText(savenum,pt,"无效的数字");
        return ;
    }
    if( WordSupport::IsFontColor(fontcolor->text()))
        tmp.defaultFontColor = Color2Str(fontcolor->text());
    else{
        callText(fontcolor,pt,"无效的字体颜色");
        return ;
    }
    tmp.defaultFontName = fontname->text();

    if( WordSupport::IsNumber(debuglevel->text()))
        tmp.debugLevel = debuglevel->text().toInt();

    tmp.live2DKey = live2dkey->text();

    QFile ks;
    ks.setFileName(config->projDir+"/settings.bkpsr");
    ks.remove() ;
    *config = tmp;
    config->writeFile();
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

    QString name = QFileDialog::getExistingDirectory(this,"选择文件夹",config->projDir) ;
    if( name.isEmpty() ) return ;

    QString t = edit->text() ;
	auto lst = getDirText(edit);
    name = BkeFullnameToName(name,config->projDir) ;
	if (lst.contains(name))
		return;
    if( !t.isEmpty() && !t.endsWith("|") ) t.append("|"+name) ;
    else t.append(name) ;
    edit->setText( t );
}

void BkeConfigUiModel::refreshUI()
{
    projectname->setText(config->projectName);
    projectsize->setText(Size2Str(config->resolutionSize));
    projecttitle->setText(config->gameTitle);

    savedir->setText(config->saveDir);
    imgdir->setText(SearchPath2Str(config->imageAutoSearchPath));
    audiodir->setText(SearchPath2Str(config->audioAutoSearchPath));
    scriptdir->setText(SearchPath2Str(config->scriptAutoSearchPath));
    savenum->setText(QString::number(config->maxSaveDataNum));
    fontsize->setText(QString::number(config->defaultFontSize));
    fontcolor->setText(Color2Str(config->defaultFontColor));
    fontname->setText(config->defaultFontName);
    debuglevel->setText(QString::number(config->debugLevel));
    live2dkey->setText(config->live2DKey);
}

void BkeConfigUiModel::callText(QWidget *w, QPoint size,const QString &info)
{
    size.setX(w->x()+size.x());
    size.setY(size.y()+w->y());
    QToolTip::showText( size ,info,this) ;
    return ;
}

