#pragma once

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>

#define DEBUGPORT 54321

class DebugServer : public QObject
{
	Q_OBJECT

	QWebSocketServer *server;
	QList<QWebSocket *> connections;

public:
	DebugServer(QObject *parent = nullptr);
	~DebugServer();

signals:
	void logReceived(int32_t level, QString log);

private slots:
	void onNewConnection();
	void closed();
	void processTextMessage(const QString &message);
	void processBinaryMessage(const QByteArray &message);
	void socketDisconnected();
};