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
#include "paper/wordsupport.h"

class BkeConfigUiModel :public QDialog
{
    Q_OBJECT
public:
    BkeConfigUiModel(QWidget *parent = 0);
    void StartConfig(const QString &file,const QString &dir) ;
signals:
    void FileOpen(const QString &file,const QString &dir) ;

private slots:
    void ToScriptModel() ;
    void Sure() ;
    void UseDir() ;
private:
    QPushButton *btnchange ;
    QPushButton *btnclose ;
    QPushButton *btnsave ;
    QString Name ;
    QString Dir ;
    QFormLayout *form ;
    QHBoxLayout *btnLineEdit(const QString &basename,QLineEdit **e) ;
    void ReadFile(const QString &file) ;
    void callText(QWidget *w, QPoint size,const QString &info) ;
    QString GetDirText( QLineEdit *e,const QString &left) ;
    QString GetVal(const QString &key) ;
    QString ToType(QString t) ;

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

    QString Text ;
};

#endif // BKECONFIGUIMODEL_H
