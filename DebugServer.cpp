#include "DebugServer.h"
#include <QWebSocket>

DebugServer::DebugServer()
{
	server = new QWebSocketServer("BKE_Creator Debug Server", QWebSocketServer::NonSecureMode, this);
	if (server->listen(QHostAddress::LocalHost, DEBUGPORT))
	{
		connect(server, &QWebSocketServer::newConnection,
			this, &DebugServer::onNewConnection);
		connect(server, &QWebSocketServer::closed, this, &DebugServer::closed);
	}
}

DebugServer::~DebugServer()
{
}

void DebugServer::closed()
{
}

void DebugServer::processTextMessage(const QString & message)
{

}

void DebugServer::processBinaryMessage(const QByteArray & message)
{
	QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
	
}

void DebugServer::socketDisconnected()
{
	QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
	if (pClient) {
		connections.removeAll(pClient);
		pClient->deleteLater();
	}
}

void DebugServer::onNewConnection()
{
	QWebSocket *pSocket = server->nextPendingConnection();

	connect(pSocket, &QWebSocket::textMessageReceived, this, &DebugServer::processTextMessage);
	connect(pSocket, &QWebSocket::binaryMessageReceived, this, &DebugServer::processBinaryMessage);
	connect(pSocket, &QWebSocket::disconnected, this, &DebugServer::socketDisconnected);

	connections << pSocket;
}