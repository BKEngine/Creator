#include "weh.h"
#include "DebugServer.h"
#include <QWebSocket>
#include <QString>
#include <QDir>

DebugServer::DebugServer(QObject *parent/* = nullptr*/)
	: QObject(parent)
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
	if (message.length() >= 8)
	{
		auto protocol = *(int32_t *)(&message.constData()[0]);
		auto type = (SocketDataType)protocol & 0xFF;
		auto taskmask = protocol & 0xFFFFFF00;
		auto datalen = *(int32_t *)(&message.constData()[4]);
		switch (type)
		{
			case SocketDataType::LOG:
			{
				auto level = *(int32_t *)(&message.constData()[8]);
				QString log = QString::fromUtf16((const char16_t *)&message.constData()[12], (datalen - 4) / 2);
				emit logReceived(level, log);
				break;
			}
			case SocketDataType::CONNECT_CONFIRM:
			{
				bool flag = workpro != nullptr;
				if (flag)
				{
					QString path = QString::fromUtf16((const char16_t *)&message.constData()[8], datalen / 2);
					QDir dir1(path);
					QDir dir2(workpro->ProjectDir());
					flag = dir1 == dir2;
				}
				reply(pClient, flag ? SocketDataType::RETURN_SUCCESS : SocketDataType::RETURN_FAIL, taskmask);
			}
			default:
				break;
		}
	}
}

void DebugServer::socketDisconnected()
{
	QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
	if (pClient) {
		connections.removeAll(pClient);
		pClient->deleteLater();
	}
}

void DebugServer::reply(QWebSocket *client, SocketDataType type, int32_t taskmask)
{
	char buffer[8];
	*(int32_t *)&buffer[0] = type | taskmask;
	*(int32_t *)&buffer[4] = 0;
	client->sendBinaryMessage(QByteArray(buffer, 8));
}

void DebugServer::WorkproChanged(BkeProject *pro)
{
	workpro = pro;
}

void DebugServer::onNewConnection()
{
	QWebSocket *pSocket = server->nextPendingConnection();

	connect(pSocket, &QWebSocket::textMessageReceived, this, &DebugServer::processTextMessage);
	connect(pSocket, &QWebSocket::binaryMessageReceived, this, &DebugServer::processBinaryMessage);
	connect(pSocket, &QWebSocket::disconnected, this, &DebugServer::socketDisconnected);

	connections << pSocket;
}