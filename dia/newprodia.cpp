#include "newprodia.h"

NewProDia::NewProDia(QWidget *parent) :
    QWidget(parent)
{
    prodialog = new QDialog(parent) ;
    dir = new QPushButton("...",prodialog) ;
    yes = new QPushButton("确定",prodialog);
    no = new QPushButton("取消",prodialog);
    s1 = new QLineEdit(prodialog);
    s2 = new QLineEdit(prodialog) ;
    k1 = new QLabel("创建一个Bke项目，Bke项目能有效的帮你维护文件关系，方便编译脚本，测试与发布游戏。\r\n"
                    "存储位置不存在时，将自动创建",prodialog) ;
    k2 = new QLabel("    名称",prodialog) ;
    k3 = new QLabel("存储位置",prodialog);

    k1->setWordWrap(true);
    k1->setGeometry(42,34,326,146);
    k2->setGeometry(12,191,60,20);
    k3->setGeometry(12,221,60,20);
    s1->setGeometry(84,191,290,20);
    s2->setGeometry(84,221,290,20);
    dir->setGeometry(385,221,50,20);
    yes->setGeometry(300,261,54,30);
    no->setGeometry(362,261,54,30);

    resize(400,300);
    prodialog->setWindowTitle("新建项目");

    type = -1 ;

    connect(dir,SIGNAL(clicked()),this,SLOT(LookDir())) ;
    connect(yes,SIGNAL(clicked()),this,SLOT(SureOK())) ;
    connect(no,SIGNAL(clicked()),prodialog,SLOT(close())) ;
}

bool NewProDia::WaitUser()
{
    prodialog->exec() ;
    return true ;
}

void NewProDia::LookDir()
{
    QString dirs = QFileDialog::getExistingDirectory(prodialog,"选择保存文件夹","") ;
    if( dirs.isNull() ) return ;
    dirs.replace(QRegExp("\\\\"),"/") ;
    s2->setText(dirs);
}

void NewProDia::SureOK()
{
    //QString dirs,name ;
    if( s1->text().isEmpty()){
        QMessageBox::information(prodialog,"信息","请输入项目名称",QMessageBox::Ok) ;
        return ;
    }
    else if( s2->text().isEmpty()){
        QMessageBox::information(prodialog,"信息","请选择保存路径",QMessageBox::Ok) ;
        return ;
    }

    okdir = s2->text() ;
    okname = s1->text() ;

    okdir.replace(QRegExp("\\\\"),"/") ;
    if( okdir.endsWith("/") ) okdir = okdir.left(okdir.length()-1) ; //"不以/结尾"

    QFile temp(okdir + "/" + BKE_PROJECT_NAME) ;
    if( temp.exists()){
        QMessageBox tk(prodialog) ;
        tk.setText("项目已存,在是否覆盖？");
        tk.setStandardButtons(QMessageBox::Yes|QMessageBox::Cancel|QMessageBox::Open);
        tk.setDefaultButton(QMessageBox::Open);
        int i = tk.exec() ;
        if( i == QMessageBox::Cancel) return ;
        else if( i == QMessageBox::Open){
            type = 1 ;
            okname = temp.fileName() ;
            prodialog->close() ;
            return ;
        }
    }

    type = 0 ;
    QDir ak(okdir) ;
    ak.mkpath(okdir) ;
    prodialog->close() ;
    return ;

}
