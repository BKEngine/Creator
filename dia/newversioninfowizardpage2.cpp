#include "newversioninfowizardpage2.h"
#include "newversiondatawizard.h"
#include "ui_newversiondatawizard.h"

NewVersionInfoWizardPage2::NewVersionInfoWizardPage2()
{

}

NewVersionInfoWizardPage2::~NewVersionInfoWizardPage2()
{

}

bool NewVersionInfoWizardPage2::validatePage()
{
    NewVersionDataWizard *wizard = (NewVersionDataWizard *)this->wizard();
    if(!wizard->ui->lineEdit->text().isEmpty())
    {
        return true;
    }
    QMessageBox::warning(this, "错误", "名称不能为空。");
    return false;
}

