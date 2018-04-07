#pragma once

#include "bkeproject.h"
#include "SocketDataType.h"
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>
#include <QMap>
#include <functional>

#define DEBUGPORT 54321

class DebugServer : public QObject
{
	Q_OBJECT

public:
	typedef std::function<void(SocketDataType, const unsigned char *data, int32_t len)> Callback;
private:
	QWebSocketServer *server;
	QList<QWebSocket *> connections;
	QWebSocket *debugClient = nullptr;
	QMap<QWebSocket *, QMap<int32_t, Callback>> callbacks;
	void Send(QByteArray &data);
	void Send(QByteArray &data, Callback &&callback);

public:
	DebugServer(QObject *parent = nullptr);
	~DebugServer();

	bool IsValid();
	void Send(SocketDataType type);
	void Send(SocketDataType type, Callback &&callback);
	void Send(SocketDataType type, const QString &msg);
	void Send(SocketDataType type, const QString &msg, Callback &&callback);
	void Send(SocketDataType type, const QByteArray &msg);
	void Send(SocketDataType type, const QByteArray &msg, Callback &&callback);
	
signals:
	void logReceived(int32_t level, QString log);
	void onDebugClientConnected();
	void onDebugClientDisconnected();

public slots:
	void WorkproChanged(BkeProject *);

private slots:
	void onNewConnection();
	void closed();
	void processTextMessage(const QString &message);
	void processBinaryMessage(const QByteArray &message);
	void socketDisconnected();

private:
	int32_t taskcode = 0x100;
	BkeProject *workpro = nullptr;
	void reply(QWebSocket *client, SocketDataType type, int32_t taskmask);
};