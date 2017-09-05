#include <QDebug>
#include "qbkewizard.h"
#include "ui_qbkewizard.h"

QBkeWizard::QBkeWizard(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QBkeWizard)
{
    ui->setupUi(this);
}

QBkeWizard::~QBkeWizard()
{
    delete ui;
}

void QBkeWizard::addPage(QBkeWizardPage *page)
{
    if(!_pages.empty())
    {
        QBkeWizardPage *prevPage = _pages.last();
        auto pageId = _pages.size();
        if(prevPage->nextPageId()==-1)
        {
            prevPage->setNextPageId(pageId);
        }
        page->setPageId(pageId);
        page->setLastPageByWizard(true);
        prevPage->setLastPageByWizard(false);
    }
    else
    {
        page->setFirstPageByWizard(true);
        page->setPageId(0);
    }
    page->setWizard(this);
    _pages.append(page);
    ui->stackedWidget->addWidget(page);
    if(page->isFirstPage())
    {
        _currentPageId = page->pageId();
        ui->stackedWidget->setCurrentWidget(page);
        _updateButtons(page);
    }
}

void QBkeWizard::setSubtitleSequence(const QList<int> &sequenceOfPages)
{
    _subtitleSequence = sequenceOfPages;
    _setupSubtitles();
}

int QBkeWizard::subtitleOrderById(int id)
{
    return _subtitleSequence.indexOf(id);
}

void QBkeWizard::nextToPage(int id)
{
    int currentPageId = _currentPageId;
    QBkeWizardPage *currentPage = pageById(currentPageId);
    int nextPageId = id;
    if(id < 0 || id >= _pages.size())
    {
        qWarning()<<"Error Page Id :" + QString::number(id);
        return;
    }
    currentPage->onNextToOrFinished(nextPageId);
    QBkeWizardPage *nextPage = pageById(nextPageId);
    ui->stackedWidget->setCurrentIndex(nextPageId);
    nextPage->setPrevPageId(currentPageId);
    _currentPageId = nextPageId;
    nextPage->onNextFrom(currentPageId);
    nextPage->onInitialize(!nextPage->isAccessed());
    nextPage->setAccessed();
    _updateButtons(nextPage);
}

void QBkeWizard::backToPage(int id)
{
    int currentPageId = _currentPageId;
    QBkeWizardPage *currentPage = pageById(currentPageId);
    int prevPageId = id;
    if(id < 0 || id >= _pages.size())
    {
        qWarning()<<"Error Page Id :" + QString::number(id);
        return;
    }
    currentPage->onBackTo(prevPageId);
    QBkeWizardPage *prevPage = pageById(prevPageId);
    ui->stackedWidget->setCurrentIndex(prevPageId);
    _currentPageId = prevPageId;
    prevPage->onBackFrom(currentPageId);
    prevPage->onInitialize(!prevPage->isAccessed());
    prevPage->setAccessed();
    _updateButtons(prevPage);
}

void QBkeWizard::on_cancelButton_clicked()
{
    emit rejected();
}

void QBkeWizard::on_nextButton_clicked()
{
    if(_currentPageId == -1)
        return;
    //临终检查
    QBkeWizardPage *currentPage = pageById(_currentPageId);
    if(currentPage->onValidate())
    {
        if(currentPage->isLastPage())
        {
            currentPage->onNextToOrFinished(-1);
            emit accept();
        }
        else
        {
            int nextPageId = currentPage->nextPageId();
            nextToPage(nextPageId);
        }
    }
}

void QBkeWizard::on_backButton_clicked()
{
    if(_currentPageId == -1)
        return;
    QBkeWizardPage *currentPage = pageById(_currentPageId);
    int prevPageId = currentPage->prevPageId();
    backToPage(prevPageId);
}

void QBkeWizard::on_QBkeWizard_rejected()
{
    if(_currentPageId == -1)
        return;
    pageById(_currentPageId)->onCancel();
}

void QBkeWizard::_updateButtons(QBkeWizardPage *currentPage)
{
    if(currentPage->isFirstPage())
    {
        ui->backButton->setVisible(false);
    }
    else
    {
        ui->backButton->setVisible(true);
    }
    if(currentPage->isLastPage())
    {
        ui->nextButton->setText("完成");
    }
    else
    {
        ui->nextButton->setText("下一步");
    }
}
