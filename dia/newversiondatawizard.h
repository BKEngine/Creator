#ifndef NEWVERSIONDATAWIZARD_H
#define NEWVERSIONDATAWIZARD_H

#include <QWizard>
#include "newversioninfowizardpage.h"
#include "newversioninfowizardpage2.h"
#include "bkeproject.h"

namespace Ui {
class NewVersionDataWizard;
}

class NewVersionDataWizard : public QWizard
{
    Q_OBJECT
    friend class NewVersionInfoWizardPage;
    friend class NewVersionInfoWizardPage2;
public:
    explicit NewVersionDataWizard(BkeProject *proj, QWidget *parent = 0);
    QMap<QString, QDateTime> data;
    QString name;
    QString info;
    QDateTime time;
    bool isAccepted(){return _accepted;}
    ~NewVersionDataWizard();

private slots:
    void on_NewVersionDataWizard_accepted();

private:
    bool _accepted;
    BkeProject *_project;
    Ui::NewVersionDataWizard *ui;
};

#endif // NEWVERSIONDATAWIZARD_H
