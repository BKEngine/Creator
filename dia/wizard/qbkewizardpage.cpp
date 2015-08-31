#include "qbkewizardpage.h"

QBkeWizardPage::QBkeWizardPage(QWidget *parent) : QWidget(parent, Qt::Dialog & (~Qt::WindowMinMaxButtonsHint))
{

}

QBkeWizardPage::~QBkeWizardPage()
{

}

bool QBkeWizardPage::onValidate()
{
    if(_delegate)
    {
        return _delegate->onValidate();
    }
    return true;
}

bool QBkeWizardPage::isCompleted()
{
    if(_delegate)
    {
        return _delegate->isCompleted();
    }
    return true;
}

void QBkeWizardPage::onInitialize(bool isFirst)
{
    if(_delegate)
    {
        _delegate->onInitialize(isFirst);
    }
}

void QBkeWizardPage::onCancel(int id)
{
    if(_delegate)
    {
        _delegate->onCancel(id);
    }
}

void QBkeWizardPage::onBackTo(int id)
{
    if(_delegate)
    {
        _delegate->onBackTo(id);
    }
}

void QBkeWizardPage::onNextToOrFinished(int id)
{
    if(_delegate)
    {
        _delegate->onNextToOrFinished(id);
    }
}

void QBkeWizardPage::onBackFrom(int id)
{
    if(_delegate)
    {
        _delegate->onBackFrom(id);
    }
}

void QBkeWizardPage::onNextFrom(int id)
{
    if(_delegate)
    {
        _delegate->onNextFrom(id);
    }
}

