#include "newversioninfowizardpage.h"
#include "newversiondatawizard.h"
#include "ui_newversiondatawizard.h"
#include <QMovie>
#include "weh.h"
#include "codewindow.h"

NewVersionInfoWizardPage::NewVersionInfoWizardPage()
{

}

NewVersionInfoWizardPage::~NewVersionInfoWizardPage()
{

}

void NewVersionInfoWizardPage::initializePage()
{
    _finished = false;
    NewVersionDataWizard *wizard = (NewVersionDataWizard *)this->wizard();
    _scanThread.setProject(wizard->_project);

    QMovie *waitIcon = new QMovie(":/newversioninfo/source/728.GIF", QByteArray(), this);
    wizard->ui->label_4->setMovie(waitIcon);
    waitIcon->start();
    wizard->ui->label_5->setText("正在编译……");

    connect(codeedit, SIGNAL(CompileFinish(int)),this,SLOT(compileFinish(int)));
    codeedit->CompileAll(true);
}

bool NewVersionInfoWizardPage::isComplete() const
{
    if(_finished)
    {
        return true;
    }
    else
    {
        connect(this, SIGNAL(onResult()), this, SIGNAL(completeChanged()));
    }
}

void NewVersionInfoWizardPage::cleanupPage()
{
    disconnect(codeedit, SIGNAL(CompileFinish(int)),this,SLOT(compileFinish(int)));
    if(_scanThread.isRunning())
    {
        _scanThread.shouldTerminate();
        _scanThread.wait();
        _scanThread.exit();
    }
}

void NewVersionInfoWizardPage::compileFinish(int errors)
{
    disconnect(codeedit, SIGNAL(CompileFinish(int)),this,SLOT(compileFinish(int)));
    NewVersionDataWizard *wizard = (NewVersionDataWizard *)this->wizard();
    if(errors>=1)
    {
        QLabel *label = wizard->ui->label_4;
        label->setPixmap(QPixmap(":/newversioninfo/source/error_big.png"));
        wizard->ui->label_5->setText("编译失败");
    }
    else
    {
        wizard->ui->label_5->setText("正在生成……");
        connect(&_scanThread, SIGNAL(result()), this, SLOT(reciveResult()));
        _scanThread.start();
    }
}

void NewVersionInfoWizardPage::reciveResult()
{
    _data = _scanThread.getData();
    _finished = true;
    NewVersionDataWizard *wizard = (NewVersionDataWizard *)this->wizard();
    QLabel *label = wizard->ui->label_4;
    label->setPixmap(QPixmap(":/newversioninfo/source/check_big.png"));
    wizard->ui->label_5->setText("生成完成");
    emit onResult();
}

void WorkThread::run()
{
    _shouldTerminate = false;
    QStringList scripts = _project->AllScriptFiles();
    QStringList sources = _project->AllSourceFiles();
    QString proDir = _project->FileDir();

    for(auto item : scripts)
    {
        if(_shouldTerminate)
            return;
        QFileInfo fi(proDir + "/" + item);
        if(fi.exists())
        {
            QDateTime dt = fi.lastModified();
            _data[item] = dt;
        }
    }

    for(auto item : sources)
    {
        if(_shouldTerminate)
            return;
        QFileInfo fi(proDir + "/" + item);
        if(fi.exists())
        {
            QDateTime dt = fi.lastModified();
            _data[item] = dt;
        }
    }

    emit result();
}

void WorkThread::shouldTerminate()
{
    _shouldTerminate = true;
}
