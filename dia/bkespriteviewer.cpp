#include "bkespriteviewer.h"
#include "ui_bkespriteviewer.h"
#include "Debugger/DebugServer.h"

BkeSpriteViewer::BkeSpriteViewer(DebugServer *debugServer, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BkeSpriteViewer)
{
    ui->setupUi(this);
	this->debugServer = debugServer;
	Init();
}

BkeSpriteViewer::~BkeSpriteViewer()
{
    delete ui;
}

void BkeSpriteViewer::Init()
{
	if (!debugServer->IsValid())
	{
		QMessageBox::warning(nullptr, "����", "�������ӵ�������");
		close();
	}
	//debugServer->Send(SocketDataType::)
}
