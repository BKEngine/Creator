#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QtNetwork>
#include <QTimer>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QScrollBar>
#include <QDesktopServices>
#include <QMutex>
#include "loli/loli_island.h"
#include "weh.h"



namespace Ui {
class MainWindow;
}

class md5type
{
public:
    QString name ;
    QString md5 ;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    enum{
        STATE_NULL ,
        STATE_DOWNINFO ,
        STATE_NEXT ,
        STATE_FINISH ,
        STATE_CHEAKDATE ,
        STATE_STARTUP
    };



public slots:
    void StartX() ;
    void InfodownFinish(QNetworkReply* netfile) ;
    void MakeMD5() ;
    void downFinish(QNetworkReply* netfile) ;
    void startUpdate() ;
    void updateDataReadProgress(qint64 from,qint64 to) ;
    void Nextfile() ;
    void simpleOK() ;
    void DownInfo() ;

private:
    Ui::MainWindow *ui;

    QNetworkAccessManager *netAdmin ;
    QNetworkReply *currentREP ;
    QJsonObject verJSON ;
    QJsonObject newJSON ;
    QJsonObject nfilejs ;
    QJsonObject ofilejs ;
    QJsonObject filesjson ;

    QStringList downlist ;
    QStringList errorlist ;
    QString cmdString ;
    QString currentfile ;

    QMutex mutex ;
    QTimer *autocheck ;

    int ptr ;
    int state ;
    bool isSelfUp ;
    bool isOK ;
    bool isDown ;
    bool isBkeUp;

    void ReadVertion() ;
    void simpleMD5(QString dir) ;
    void addError(const QString &info) ;
    bool isFileDiffrent(const QString &name) ;
};

#endif // MAINWINDOW_H
