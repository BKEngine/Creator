#ifndef NEWPRODIA_H
#define NEWPRODIA_H

//新建项目对话框
#include <QWidget>
#include "weh.h"

class NewProDia : public QWidget
{
    Q_OBJECT
public:
     NewProDia(QWidget *parent = 0);
     int type ;
     QString okdir ;
     QString okname ;

     bool WaitUser() ;

signals:

public slots:
     void LookDir() ;
     void SureOK() ;
private:
     QDialog *prodialog ;
     QPushButton *dir ;
     QPushButton *yes ;
     QPushButton *no ;
     QLineEdit *s1 ;
     QLineEdit *s2 ;
     QLabel *k1 ;
     QLabel *k2 ;
     QLabel *k3 ;
};

#endif // NEWPRODIA_H
