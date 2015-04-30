#include "newversiondatawizard.h"
#include "ui_newversiondatawizard.h"
#include "newversioninfowizardpage.h"

#include <QMovie>

NewVersionDataWizard::NewVersionDataWizard(BkeProject *proj, QWidget *parent) :
    QWizard(parent),
    _project(proj),
    ui(new Ui::NewVersionDataWizard)
{
    ui->setupUi(this);
    _accepted=false;
}

NewVersionDataWizard::~NewVersionDataWizard()
{
    delete ui;
}


void NewVersionDataWizard::on_NewVersionDataWizard_accepted()
{
    name = ui->lineEdit->text();
    info = ui->lineEdit_2->text();
    time = QDateTime::currentDateTimeUtc();
    data = ((NewVersionInfoWizardPage *)ui->wizardPage3)->getData();
    _accepted=true;
}
