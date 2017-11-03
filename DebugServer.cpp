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

bool DebugServer::IsValid()
{
	return debugClient != nullptr;
}

void DebugServer::Send(QByteArray &data)
{
	*(int32_t *)&data.data()[0] |= taskcode;
	taskcode += 0x100;
	debugClient->sendBinaryMessage(data);
}

void DebugServer::Send(QByteArray &data, Callback && callback)
{
	callbacks[debugClient].insert(taskcode, std::move(callback));
	Send(data);
}

void DebugServer::Send(SocketDataType type)
{
	QByteArray data(8, Qt::Initialization());
	*(int32_t *)&data.data()[0] = type;
	*(int32_t *)&data.data()[4] = 0;
	Send(data);
}

void DebugServer::Send(SocketDataType type, Callback && callback)
{
	if (debugClient == nullptr)
	{
		callback(SocketDataType::ERROR, 0, 0);
		return;
	}
	callbacks[debugClient].insert(taskcode, std::move(callback));
	Send(type);
}

void DebugServer::Send(SocketDataType type, const QString &msg)
{
	QByteArray data((char *)msg.data(), msg.length() * 2);
	Send(type, data);
}

void DebugServer::Send(SocketDataType type, const QString &msg, Callback && callback)
{
	if (debugClient == nullptr)
	{
		callback(SocketDataType::ERROR, 0, 0);
		return;
	}
	callbacks[debugClient].insert(taskcode, std::move(callback));
	Send(type, msg);
}

void DebugServer::Send(SocketDataType type, const QByteArray & msg)
{
	QByteArray data(8 + msg.length(), Qt::Initialization());
	*(int32_t *)&data.data()[0] = type;
	*(int32_t *)&data.data()[4] = msg.length();
	memcpy(&data.data()[8], msg.constData(), msg.length());
	Send(data);
}

void DebugServer::Send(SocketDataType type, const QByteArray & msg, Callback && callback)
{
	if (debugClient == nullptr)
	{
		callback(SocketDataType::ERROR, 0, 0);
		return;
	}
	callbacks[debugClient].insert(taskcode, std::move(callback));
	Send(type, msg);
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
		// 在debugClient未指定或者Client不是debugClient的时候，除了CONNECT_CONFIRM以外的所有消息都不处理
		if ((debugClient == nullptr || debugClient != pClient) && type != SocketDataType::CONNECT_CONFIRM)
			return;
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
				if (flag)
				{
					flag = debugClient == nullptr;
				}
				if (flag)
				{
					debugClient = pClient;
					emit onDebugClientConnected();
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
		callbacks.remove(pClient);
		pClient->deleteLater();
		if (pClient == debugClient)
		{
			debugClient = nullptr;
			emit onDebugClientDisconnected();
		}
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
	callbacks.insert(pSocket, {});
}