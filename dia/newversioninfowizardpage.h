#ifndef NEWVERSIONINFOWIZARDPAGE_H
#define NEWVERSIONINFOWIZARDPAGE_H

#include <QWizardPage>
#include <QThread>
#include <QMap>
#include "bkeproject.h"

class WorkThread : public QThread
{
    Q_OBJECT
    virtual void run() override;

    BkeProject *_project;
    volatile bool _shouldTerminate = false;
    QMap<QString, QDateTime> _data;
public:
    void setProject(BkeProject *project){_project=project;}
    QMap<QString, QDateTime> &getData(){return _data;}
public slots:
    void shouldTerminate();
signals:
    void result();
};

class NewVersionInfoWizardPage : public QWizardPage
{
    Q_OBJECT
    QMap<QString, QDateTime> _data;
public:
    NewVersionInfoWizardPage();
    ~NewVersionInfoWizardPage();

    QMap<QString, QDateTime> &getData(){return _data;}

private:
    bool _finished;
    WorkThread _scanThread;
    virtual void initializePage() override;
    virtual bool isComplete() const override;
    virtual void cleanupPage() override;

private slots:
    void reciveResult();
    void compileFinish(int errors);

signals:
    void onResult();
};

#endif // NEWVERSIONINFOWIZARDPAGE_H
