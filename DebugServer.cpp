#include "DebugServer.h"
#include <QWebSocket>
#include <QString>

enum SocketDataType : int32_t
{
	UNKNOWN,
	CONNECT_CLOSE,	//unused
	RETURN_SUCCESS,	//datalen=0
	RETURN_FAIL,	//datalen=0
	NEW_BREAKPOINT,	//data=fullfilename:lineNo
	DEL_BREAKPOINT,	//data=fullfilename:lineNo
	NEW_BREAKPOINTONCE,	//data=fullfilename:lineNo
	QUERY_VAR,		//data=variable name expression
	QUERY_SP,		//datalen=2,data=(int32_t)spIndex
	QUERY_SCREEN,	//datalen=0
	QUERY_AUDIO,	//datalen=2, data=(int32_t)audio_index
	STEP_NEXT,		//datalen=0
	STEP_INTO,		//datalen=0
	STEP_OUT,		//datalen=0
	RUN,			//datalen=0
	PAUSE,			//datalen=0
	EXECUTE_BAGEL,	//data=bagel expression
	RET_NOTFOUND,	//datalen=0
	RET_BAGEL,		//data=serialized bagel data
	RET_EXCEPT,		//data=serialized bagel data
	LOG,			//data=[int32]level [string]msg
};

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
		auto type = *(SocketDataType *)(&message.constData()[0]);
		auto datalen = *(int32_t *)(&message.constData()[4]);
		switch (type & 0xFF)
		{
		case SocketDataType::LOG:
		{
			auto level = *(int32_t *)(&message.constData()[8]);
			QString log = QString::fromUtf16((const char16_t *)&message.constData()[12], (datalen - 4) / 2);
			emit logReceived(level, log);
		}
			break;
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

void DebugServer::onNewConnection()
{
	QWebSocket *pSocket = server->nextPendingConnection();

	connect(pSocket, &QWebSocket::textMessageReceived, this, &DebugServer::processTextMessage);
	connect(pSocket, &QWebSocket::binaryMessageReceived, this, &DebugServer::processBinaryMessage);
	connect(pSocket, &QWebSocket::disconnected, this, &DebugServer::socketDisconnected);

	connections << pSocket;
}