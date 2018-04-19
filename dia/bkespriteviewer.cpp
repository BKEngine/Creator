#include "weh.h"
#include "bkespriteviewer.h"
#include "ui_bkespriteviewer.h"
#include "Debugger/DebugServer.h"
#include "ParserHelper/ParserHelper.h"
#include "projectwindow.h"
#include "QGraphicViewZoomer.h"

#include <QPointer>
#include <QImage>
#include <QStackedLayout>
#include "lz4/lz4.h"

BkeSpriteViewer::BkeSpriteViewer(DebugServer *debugServer, QWidget *parent) :
	QMainWindow(parent),
    ui(new Ui::BkeSpriteViewer)
{
	setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
	ui->treeWidget->setHeaderLabel("精灵");
	ui->splitter->setStretchFactor(0, 1);
	ui->splitter->setStretchFactor(1, 1);
	scene = new QGraphicsScene(ui->graphicsView);
	pixmapItem = new QGraphicsPixmapItem();
	rectItem = new QGraphicsRectItem();
	rectItem->setZValue(1);
	QPen pen = rectItem->pen();
	pen.setWidth(2);
	pen.setColor(Qt::green);
	rectItem->setPen(pen);

	ui->graphicsView->setScene(scene);
	scene->addItem(pixmapItem);
	ui->comboBox->addItems(QStringList() << "白色" << "浅灰色" << "深灰色");
	ui->comboBox->setCurrentIndex(0);

	this->debugServer = debugServer;
	Init();
	connect(debugServer, &DebugServer::onDebugClientDisconnected, this, [this]() 
	{
		this->Clear();
		this->setEnabled(false);
	});
	connect(debugServer, &DebugServer::onDebugClientConnected, this, &BkeSpriteViewer::Init);

	QGraphicViewZoomer *zoomer = new QGraphicViewZoomer(ui->graphicsView);
	connect(this, &BkeSpriteViewer::onNewImage, zoomer, &QGraphicViewZoomer::reset);
}

BkeSpriteViewer::~BkeSpriteViewer()
{
    delete ui;
}

static void FeedTreeItem(QTreeWidgetItem *item, const QBkeVariable &dic)
{
	QString index = dic["index"].toString();
	QString filename = dic["filename"].toString();
	QString type = dic["type"].toString();
	QString text = index;
	item->setData(0, Qt::UserRole, index.toInt());
	if (!filename.isEmpty())
	{
		QFileInfo qfi = QFileInfo(filename);
		if (qfi.isAbsolute())
		{
			filename = BkeFullnameToName(filename, projectedit->workpro->ProjectDir());
		}
		text += " " + filename;
	}
	item->setText(0, text);
	QBkeVariable children = dic["children"];
	if (children.isArray())
	{
		auto count = children.getCount();
		for (int i = 0; i < count; i++)
		{
			QBkeVariable dic = children[i];
			QTreeWidgetItem *child = new QTreeWidgetItem(item);
			FeedTreeItem(child, dic);
		}
	}
}

void BkeSpriteViewer::Init()
{
	this->setEnabled(false);
	this->cursor().setShape(Qt::BusyCursor);
	if (!debugServer->IsValid())
	{
		QMessageBox::warning(nullptr, "错误", "请先连接调试器！");
		close();
		return;
	}
	QPointer<BkeSpriteViewer> self(this);
	debugServer->Send(SocketDataType::QUERY_SPRITE_TREE, [self](SocketDataType type, const unsigned char *data, int32_t len) {
		if (type == SocketDataType::ERROR) {
			QMessageBox::warning(nullptr, "错误", "调试器连接失败！");
		}
		else if (type == SocketDataType::RETURN_BAGEL) {
			if (self) {
				self->Clear();
				QBkeVariable tree = QBkeVariable::fromBinary(QByteArray((const char *)data, len));
				QTreeWidgetItem *root = new QTreeWidgetItem(self->ui->treeWidget);
				FeedTreeItem(root, tree);
				self->ui->treeWidget->expandAll();
				self->setEnabled(true);
				self->cursor().setShape(Qt::ArrowCursor);
			}
		}
	});
}

void BkeSpriteViewer::Clear()
{
	ui->treeWidget->clear();
}

void BkeSpriteViewer::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	if (currentItem != current)
	{
		currentItem = current;
	}
	this->Refresh();
}

void BkeSpriteViewer::Refresh()
{
	if (!currentItem)
		return;
	this->setEnabled(false);
	this->cursor().setShape(Qt::BusyCursor);
	char data[6];
	*((int32_t *)&data[0]) = currentItem->data(0, Qt::UserRole).toInt();
	*((bool *)&data[4]) = ui->checkBox->isChecked();
	*((bool *)&data[5]) = ui->checkBox_2->isChecked();
	QPointer<BkeSpriteViewer> self(this);
	debugServer->Send(SocketDataType::QUERY_SPRITE_IMAGE, QByteArray::fromRawData(data, sizeof(data)), [self](SocketDataType type, const unsigned char *data, int32_t len) {
		if (type == SocketDataType::ERROR) {
			QMessageBox::warning(nullptr, "错误", "调试器连接失败！");
		}
		else
		{
			self->setEnabled(true);
			self->cursor().setShape(Qt::ArrowCursor);
			if (type == SocketDataType::RETURN_SPRITE_IMAGE) {
				if (self) {
					uint16_t width = *((uint16_t *)&data[0]);
					uint16_t height = *((uint16_t *)&data[2]);
					int32_t rawsize = *((int32_t *)&data[4]);
					int32_t datasize = len - 8;
					QByteArray decompressed(rawsize, Qt::Uninitialized);
					LZ4_decompress_fast((const char *)data + 8, decompressed.data(), rawsize);
					QImage image((const uint8_t *)decompressed.constData(), width, height, QImage::Format_RGBA8888);
					self->pixmapItem->setPixmap(QPixmap::fromImage(image));
					self->scene->setSceneRect(image.rect());
					self->rectItem->setRect(self->pixmapItem->boundingRect());
					emit self->onNewImage();
				}
			}
		}
	});
}

void BkeSpriteViewer::on_checkBox_toggled(bool checked)
{
	Refresh();
}

void BkeSpriteViewer::on_checkBox_2_toggled(bool checked)
{
	ui->checkBox->setEnabled(checked);
	Refresh();
}

void BkeSpriteViewer::on_comboBox_currentIndexChanged(int index)
{
	QBrush brush(Qt::SolidPattern);
	switch (index)
	{
	case 0:
		brush.setColor(Qt::white);
		break;
	case 1:
		brush.setColor(QColor(0xBB, 0xBB, 0xBB));
		break;
	case 2:
		brush.setColor(QColor(0x28, 0x28, 0x28));
		break;
	default:
		brush.setColor(Qt::transparent);
		break;
	}
	ui->graphicsView->setBackgroundBrush(brush);
}

void BkeSpriteViewer::on_action_toggled(bool checked)
{
	if (checked)
		scene->addItem(rectItem);
	else
		scene->removeItem(rectItem);
}
