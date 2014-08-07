#include "singleapplication.h"
#include <QtNetwork>
#include <QWidget>
#include <QMessageBox>

SingleApplication::SingleApplication(int argc, char *argv[])
    :QApplication(argc,argv)
{
    _running=0;
    QString serverName = QCoreApplication::applicationName();
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(1000)) { //如果能够连接得上的话，将参数发送到服务器，然后退出
        QTextStream stream(&socket);
        QStringList args = QCoreApplication::arguments();
        if(args.count()>1)
            stream<<args[1];
        stream.flush();
        socket.waitForBytesWritten();
         _running=1;
        return;
    }
    //运行到这里，说明没有实例在运行，那么创建服务器。
    m_localServer = new QLocalServer(this);
    connect(m_localServer, SIGNAL(newConnection()),
            this, SLOT(newLocalSocketConnection())); //监听新到来的连接
    if (!m_localServer->listen(serverName)) {
        if (m_localServer->serverError() == QAbstractSocket::AddressInUseError
            && QFile::exists(m_localServer->serverName())) { //确保能够监听成功
            QFile::remove(m_localServer->serverName());
            m_localServer->listen(serverName);
        }
    }

}

void SingleApplication::newLocalSocketConnection()
{
    QLocalSocket *socket = m_localServer->nextPendingConnection();
    if (!socket || !socket->waitForReadyRead(500))
        return;
    QTextStream stream(socket);
    QString a = stream.readAll();
    emit newApplication(a);

    if(m_activeWidget)
    {
        m_activeWidget->showNormal();
        m_activeWidget->setWindowState(m_activeWidget->windowState() & ~Qt::WindowMinimized);
        m_activeWidget->raise();
        m_activeWidget->activateWindow();
    }
}
