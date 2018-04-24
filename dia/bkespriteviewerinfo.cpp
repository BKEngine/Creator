#include "bkespriteviewerinfo.h"
#include "ui_bkespriteviewerinfo.h"

#include <QStringBuilder>
#include <QMouseEvent>

BkeSpriteViewerInfo::BkeSpriteViewerInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BkeSpriteViewerInfo)
{
    ui->setupUi(this);

	this->setAttribute(Qt::WA_TransparentForMouseEvents, true);

	QPalette pal = palette();
	pal.setColor(QPalette::Background, QColor(0x00, 0x00, 0x00, 0x00));
	setPalette(pal);

	setState(State::Left);
}

void BkeSpriteViewerInfo::setState(State state)
{
	if (state == State::Left)
	{
		ui->horizontalSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
		ui->horizontalSpacer_2->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
	}
	else if(state == State::Right)
	{
		ui->horizontalSpacer_2->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
		ui->horizontalSpacer->changeSize(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
	}
}

void BkeSpriteViewerInfo::setInfoIndex(int32_t index)
{
	ui->index->setText(QString::number(index));
}

void BkeSpriteViewerInfo::setInfoFilename(const QString & filename)
{
	if (filename.isEmpty())
	{
		ui->filename->setVisible(false);
		ui->filename_l->setVisible(false);
	}
	else
	{
		ui->filename->setVisible(true);
		ui->filename->setText(filename);
		ui->filename_l->setVisible(true);
	}
}

void BkeSpriteViewerInfo::setInfoType(const QString & type)
{
	ui->type->setText(type);
}

void BkeSpriteViewerInfo::setInfoPos(double x, double y, double glox, double gloy)
{
	ui->pos->setText(QString("[") % QString::number(x) % ',' % QString::number(y) % ']');
	ui->glopos->setText(QString("[") % QString::number(glox) % ',' % QString::number(gloy) % ']');
}

void BkeSpriteViewerInfo::setInfoRect(const QRectF & rect)
{
	if (rect.isNull())
	{
		ui->rect->setVisible(false);
		ui->rect_l->setVisible(false);
	}
	else
	{
		ui->rect->setVisible(true);
		ui->rect_l->setVisible(true);
		ui->rect->setText(QString("[") % QString::number(rect.x()) % ',' % QString::number(rect.y()) % ',' % QString::number(rect.width()) % ',' % QString::number(rect.height()) % ']');
	}
}

void BkeSpriteViewerInfo::setInfoOpacity(uint8_t opacity)
{
	if (opacity == 255)
	{
		ui->opacity->setVisible(false);
		ui->opacity_l->setVisible(false);
	}
	else
	{
		ui->opacity->setVisible(true);
		ui->opacity_l->setVisible(true);
		ui->opacity->setText(QString::number(opacity));
	}
}

void BkeSpriteViewerInfo::setInfoVisible(bool visible)
{
	if (visible == true)
	{
		ui->visible->setVisible(false);
		ui->visible_l->setVisible(false);
	}
	else
	{
		ui->visible->setVisible(true);
		ui->visible_l->setVisible(true);
		ui->visible->setText("false");
	}
}

void BkeSpriteViewerInfo::setInfoIsActing(bool acting)
{
	if (acting == false)
	{
		ui->visible->setVisible(false);
		ui->visible_l->setVisible(false);
	}
	else
	{
		ui->visible->setVisible(true);
		ui->visible_l->setVisible(true);
		ui->visible->setText("true");
	}
}

BkeSpriteViewerInfo::~BkeSpriteViewerInfo()
{
    delete ui;
}
