#include "bkeconfigdialog.h"
#include "ui_bkeconfigdialog.h"
#include "function.h"
#include "bkeprojectconfig.h"
#include "paper/wordsupport.h"
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolTip>

#ifdef Q_OS_LINUX


char* itoa(int num,char* str,int radix)
{/*索引表*/
char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
unsigned unum;/*中间变量*/
int i=0,j,k;
/*确定unum的值*/
if(radix==10&&num<0)/*十进制负数*/
{
unum=(unsigned)-num;
str[i++]='-';
}
else unum=(unsigned)num;/*其他情况*/
/*转换*/
do{
str[i++]=index[unum%(unsigned)radix];
unum/=radix;
}while(unum);
str[i]='\0';
/*逆序*/
if(str[0]=='-')k=1;/*十进制负数*/
else k=0;
char temp;
for(j=k;j<=(i-1)/2;j++)
{
temp=str[j];
str[j]=str[i-1+k-j];
str[i-1+k-j]=temp;
}
return str;
}

#endif

QBkeConfigDialog::QBkeConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::bkeconfigdialog)
{
    ui->setupUi(this);
    ui->gameName->setValidator(new QRegExpValidator(QRegExp("[^/\\\\\":<>?*|]+")));
}

QBkeConfigDialog::~QBkeConfigDialog()
{
    delete ui;
}

void QBkeConfigDialog::on_saveDirBtn_clicked()
{
    OpenDir(ui->saveDir, false);
}

static QStringList getDirText( QLineEdit *e)
{
    if( e->text().isEmpty() ) return QStringList();
    QStringList ls = e->text().split("|") ;
    for(auto &i : ls)
    {
        i = i.trimmed();
        if(i.endsWith('/') || i.endsWith('\\'))
            i = i.left(i.size()-1);
    }
    return ls;
}

void QBkeConfigDialog::OpenDir(QLineEdit *dst, bool multiselectMode)
{
    QString name;
    do
    {
        name = QFileDialog::getExistingDirectory(this,"选择文件夹",config->projDir) ;
        if( name.isEmpty() )
            return ;

        name = BkeFullnameToName(name,config->projDir) ;
        if(name.isEmpty())
            QMessageBox::warning(this, "警告", "选择了一个工程外的目录！请重新选择！");
        else
            break;
    }while(1);

    if(name.endsWith('/') || name.endsWith('\\'))
        name = name.left(name.size()-1);
    if(!multiselectMode)
    {
        dst->setText(name);
        return;
    }
    QString t = dst->text() ;
    auto lst = getDirText(dst);
    if (lst.contains(name))
        return;
    if( !t.isEmpty() && !t.trimmed().endsWith("|") )
        t.append(" | "+name) ;
    else
        t.append(name) ;
    dst->setText( t );
}

void QBkeConfigDialog::on_picASDBtn_clicked()
{
    OpenDir(ui->picASD, true);
}

void QBkeConfigDialog::on_audioASDBtn_clicked()
{
    OpenDir(ui->audioASD, true);
}

void QBkeConfigDialog::on_scriptASDBtn_clicked()
{
    OpenDir(ui->scriptASD, true);
}


void QBkeConfigDialog::StartConfig(BkeProjectConfig *config)
{
    this->config = config;
    connect(config, SIGNAL(onFileChanged()), this, SLOT(refreshUI()));
    refreshUI();
    this->exec() ;
}

static QString SearchPath2Str(const QStringList &v)
{
    return v.join(" | ");
}

static QString Size2Str(int s[])
{
    return QString("%1, %2").arg(s[0]).arg(s[1]);
}

static QString toType(const QString &t)
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

#include <stdlib.h>
#include <stdio.h>

static QString Int2Hex(int a)
{
    char tmp[12];
    sprintf(tmp, "0x%06X", a);
    return QString(tmp);
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

void QBkeConfigDialog::callText(QWidget *w, QPoint size,const QString &info)
{
    size.setX(w->x()+size.x());
    size.setY(size.y()+w->y());
    QToolTip::showText( size ,info,this) ;
    return ;
}

void QBkeConfigDialog::on_savebtn_clicked()
{
    QPoint pt = this->pos();
    BkeProjectConfig tmp;

    //是否有名称
    if( ui->gameName->text().isEmpty() ) {
        callText(ui->gameName,pt,"工程名称必须存在");
        return ;
    }

    tmp.projectName = ui->gameName->text();

    //是否有分辨率
    if(!Str2Size(tmp.resolutionSize, ui->gameResolution->text())){
        callText(ui->gameResolution,pt,"无效的分辨率设置");
        return ;
    }

    tmp.gameTitle = ui->gameTitle->text();
    tmp.saveDir = ui->saveDir->text();
    tmp.imageAutoSearchPath = getDirText(ui->picASD);
    tmp.audioAutoSearchPath = getDirText(ui->audioASD);
    tmp.scriptAutoSearchPath = getDirText(ui->scriptASD);

    if( WordSupport::IsNumber(ui->fontSize->text()))
        tmp.defaultFontSize = ui->fontSize->text().toInt();
    else{
        callText(ui->fontSize,pt,"无效的数字");
        return ;
    }
    if( WordSupport::IsNumber(ui->maxSaveNum->text()))
        tmp.maxSaveDataNum = ui->maxSaveNum->text().toInt();
    else{
        callText(ui->maxSaveNum,pt,"无效的数字");
        return ;
    }
    if( WordSupport::IsFontColor(ui->fontColor->text()))
        tmp.defaultFontColor = Color2Str(ui->fontColor->text());
    else{
        callText(ui->fontColor,pt,"无效的字体颜色");
        return ;
    }
    tmp.defaultFontName = ui->fontName->text();
    tmp.debugLevel = ui->logLevel->value();

    tmp.live2DKey = ui->live2DKey->text();

    QFile ks;
    ks.setFileName(config->projDir+"/"+tmp.saveDir+"/settings.bkpsr");
    ks.remove() ;
    *config = tmp;
    config->writeFile();
    close() ;
}

void QBkeConfigDialog::refreshUI()
{
    ui->gameName->setText(config->projectName);
    ui->gameResolution->setText(Size2Str(config->resolutionSize));
    ui->gameTitle->setText(config->gameTitle);

    ui->saveDir->setText(config->saveDir);
    ui->picASD->setText(SearchPath2Str(config->imageAutoSearchPath));
    ui->audioASD->setText(SearchPath2Str(config->audioAutoSearchPath));
    ui->scriptASD->setText(SearchPath2Str(config->scriptAutoSearchPath));
    ui->maxSaveNum->setText(QString::number(config->maxSaveDataNum));
    ui->fontSize->setText(QString::number(config->defaultFontSize));
    ui->fontColor->setText(Color2Str(config->defaultFontColor));
    ui->fontName->setText(config->defaultFontName);
    ui->logLevel->setValue(config->debugLevel);
    ui->live2DKey->setText(config->live2DKey);
}

