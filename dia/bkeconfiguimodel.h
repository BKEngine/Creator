#ifndef BKECONFIGUIMODEL_H
#define BKECONFIGUIMODEL_H
#include <QDialog>
#include <QStringList>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QToolTip>
#include <QMessageBox>

#include "function.h"
#include "bkeprojectconfig.h"
#include "paper/wordsupport.h"

class BkeConfigUiModel :public QDialog
{
    Q_OBJECT
public:
    BkeConfigUiModel(QWidget *parent = 0);
    void StartConfig(BkeProjectConfig *config) ;
signals:
    void FileOpen(const QString &file,const QString &dir) ;

private slots:
    void ToScriptModel() ;
    void Sure() ;
    void UseDir() ;
    void refreshUI();
private:
    QPushButton *btnchange ;
    QPushButton *btnclose ;
    QPushButton *btnsave ;
    BkeProjectConfig *config;
    QString basePath;
    QFormLayout *form ;
    QHBoxLayout *btnLineEdit(const QString &basename,QLineEdit **e) ;
    void callText(QWidget *w, QPoint size,const QString &info) ;
    QString GetVal(const QString &key) ;


    QLineEdit* projectname ;
    QLineEdit* projecttitle ;
    QLineEdit* projectsize ;
    QLineEdit* savedir ;
    QLineEdit* imgdir ;
    QLineEdit* audiodir ;
    QLineEdit* scriptdir ;
    QLineEdit* savenum ;
    QLineEdit* fontsize ;
    QLineEdit* fontcolor ;
    QLineEdit* fontname ;
    QLineEdit* debuglevel ;
    QLineEdit* live2dkey ;

    QString Text ;
};

#endif // BKECONFIGUIMODEL_H
