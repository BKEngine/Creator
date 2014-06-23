#ifndef LABLESUREDIALOG_H
#define LABLESUREDIALOG_H

//选择框的确认对话框
#include "weh.h"

class LableSureDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LableSureDialog(QWidget *parent = 0);
    void SetLable(const QString &text) ;
    void SetCheckbox(const QStringList &list) ;
    void SetBtn(const QStringList &list) ;
    void SetCheckboxAble(int index,bool is) ;
    void SetDefaultBtn(int i){ defbtn = i ;  }
    void SetCenterWidget(QWidget *w){ cwidget = w ; }
    bool IsCheckboxChoise(int index) ;
    int  WaitUser(int w = 320 , int h = 240) ;
signals:

public slots:
    void BtnIsDown() ;
    void BtnIsClick() ;

private:
    QWidget *cwidget ;
    QString LableText ;
    QStringList checklist ;
    QStringList btnlist ;
    QVBoxLayout *h1 ;
    QHBoxLayout *h2 ;
    QVBoxLayout *v1 ;
    QVBoxLayout *v2 ;
    int result ;
    int defbtn ;
    QList<QPushButton*> qbtnlist ;
    QList<QCheckBox*> qchklist ;
};

#endif // LABLESUREDIALOG_H
