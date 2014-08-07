#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QApplication>
#include <QtNetwork>
#include <QtWidgets>

class SingleApplication : public QApplication
{
    Q_OBJECT
public:
    SingleApplication(int argc, char *argv[]);
    bool isRunning(){return _running;}
    void setActiveWidget(QWidget *w){m_activeWidget=w;}
signals:
    void newApplication(QString args);
public slots:
    void newLocalSocketConnection();
private:
    QLocalServer *m_localServer;
    bool _running;
    QWidget *m_activeWidget = NULL;
};

#endif // SINGLEAPPLICATION_H
