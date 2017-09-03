#ifndef QBKEWIZARD_H
#define QBKEWIZARD_H

#include <QDialog>
#include <QMap>
#include "qbkewizardpage.h"

namespace Ui {
class QBkeWizard;
}

class QBkeWizard : public QDialog
{
    Q_OBJECT

public:
    explicit QBkeWizard(QWidget *parent = 0);
    ~QBkeWizard();

    void addPage(QBkeWizardPage *page);
    QBkeWizardPage *pageById(int id){return _pages[id];}
    void setSubtitleSequence(const QList<int> &sequenceOfPages);
    int subtitleOrderById(int id);

public slots:
    void nextToPage(int id);
    void backToPage(int id);

private slots:
    void on_cancelButton_clicked();
    void on_nextButton_clicked();
    void on_backButton_clicked();
    void on_QBkeWizard_rejected();

private:
    void _updateButtons(QBkeWizardPage *currentPage);
    void _setupSubtitles();
    void _updateSubtitles(QBkeWizardPage *currentPage);

private:
    Ui::QBkeWizard *ui;
    QList<QBkeWizardPage *> _pages;
    QList<int> _subtitleSequence;
    int _currentPageId = -1;
};

#endif // QBKEWIZARD_H
