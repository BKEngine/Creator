#ifndef QBKEWIZARDPAGE_H
#define QBKEWIZARDPAGE_H

#include <QWidget>

class QBkeWizard;

class QBkeWizardPageDelegate
{
public:
    //点击下一步/结束之前的检查，true才能进行下一步
    virtual bool onValidate(){return true;}
    //下一步/结束按钮是否允许
    virtual bool isCompleted(){return true;}
    //切换到当前页时执行的函数,isFirst表示是否是初次进入
    virtual void onInitialize(bool isFirst){}
    //从当前页取消时执行的函数
    virtual void onCancel(){}
    //从当前页返回上一页时执行的函数
    virtual void onBackTo(int id){}
    //从下一页返回本页面时执行的函数
    virtual void onBackFrom(int id){}
    //从当前页进入下一页/结束时执行的函数
    virtual void onNextToOrFinished(int id){}
    //从上一页进入本页时执行的函数
    virtual void onNextFrom(int id){}
};

class QBkeWizardPage : public QWidget
{
    Q_OBJECT
    friend class QBkeWizard;
public:
    explicit QBkeWizardPage(QWidget *parent = 0);
    ~QBkeWizardPage();

    bool isLastPage(){return _lastPage;}
    void setLastPage(bool b){_lastPageControledByWizard=false;_lastPage=b;}
    bool isFirstPage(){return _lastPage;}
    void setFirstPage(bool b){_firstPageControledByWizard=false;_firstPage=b;}
    int pageId(){return _pageId;}
    int nextPageId(){return _nextPageId;}
    void setNextPageId(int id){_nextPageId = id;}

    QString subTitle(){return _subTitle.isEmpty()?"page"+QString::number(pageId()):_subTitle;}
    void setSubTitle(const QString &subTitle){_subTitle = subTitle;}

    int prevPageId(){return _prevPageId;}

    //点击下一步/结束之前的检查，true才能进行下一步
    virtual bool onValidate();
    //下一步/结束按钮是否允许
    virtual bool isCompleted();
    //切换到当前页时执行的函数,isFirst表示是否是初次进入
    virtual void onInitialize(bool isFirst);
    //从当前页取消时执行的函数
    virtual void onCancel();
    //从当前页返回上一页时执行的函数
    virtual void onBackTo(int id){}
    //从下一页返回本页面时执行的函数
    virtual void onBackFrom(int id){}
    //从当前页进入下一页/结束时执行的函数
    virtual void onNextToOrFinished(int id){}
    //从上一页进入本页时执行的函数
    virtual void onNextFrom(int id){}

    void setDelegate(QBkeWizardPageDelegate *delegate){_delegate = delegate;}

private:
    int _pageId = -1;
    int _nextPageId = -1;
    int _prevPageId = -1;
    bool _firstPageControledByWizard = true;
    bool _firstPage = false;
    bool _lastPageControledByWizard = true;
    bool _lastPage = false;
    bool _isFirstAccessed = true;
    QString _subTitle;
    QBkeWizard *_wizard = NULL;
    QBkeWizardPageDelegate *_delegate;
    void setPageId(int id){_pageId=id;} //只有wizard能设置pageId
    void setPrevPageId(int id){_prevPageId=id;} //只有wizard能设置prevPageId
    void setWizard(QBkeWizard *wizard){_wizard=wizard;}
    void setFirstPageByWizard(bool b){if(_firstPageControledByWizard) _firstPage=b;}
    void setLastPageByWizard(bool b){if(_lastPageControledByWizard) _lastPage=b;}
    void setAccessed(){_isFirstAccessed=true;}
    bool isAccessed(){return _isFirstAccessed;}

signals:
    void completeChange();
public slots:
};

#endif // QBKEWIZARDPAGE_H
