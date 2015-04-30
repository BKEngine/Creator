#ifndef NEWVERSIONINFOWIZARDPAGE2_H
#define NEWVERSIONINFOWIZARDPAGE2_H

#include <QWizardPage>

class NewVersionInfoWizardPage2 : public QWizardPage
{
public:
    NewVersionInfoWizardPage2();
    ~NewVersionInfoWizardPage2();

private:
    bool validatePage();
};

#endif // NEWVERSIONINFOWIZARDPAGE2_H
