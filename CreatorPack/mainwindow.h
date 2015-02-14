#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QTimer>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QScrollBar>
#include <QDesktopServices>
#include <QFileDialog>
#include <QTextEdit>
#include "loli/loli_island.h"
#include "weh.h"
#include "QtFtp"



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



public slots:
    QByteArray MakeMD5(QString &dir, QString rdir, QString &info) ;

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void cmdfinish(int, bool error);

    void cmdstart(int);

    void progress(qint64, qint64);

    void speedTimer_timeout();

private:
    Ui::MainWindow *ui;

    QJsonObject filesjson ;
    QJsonObject verJSON;
    QString vertxt ;
    QJsonObject nfilejs ;
    QJsonObject ofilejs ;
    QFtp *ftpClient;
    QByteArray bkecreator;
    QByteArray versionjson;

    long bytesDownload;
    QTimer *speedTimer;

    void stopSpeedTimer();
    void startSpeedTimer();

    QMap<QString, QByteArray> diffs;

    void ReadVersion(QString pdir) ;
    void WriteVersion(const QByteArray &a) ;
    void simpleMD5(QString dir, QString rdir, const QSet<QString> exceptions) ;
    void addError(const QString &info) ;
    bool isFileDiffrent(const QString &name) ;
    void affineJson();
};

#endif // MAINWINDOW_H
