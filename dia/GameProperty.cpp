#include "GameProperty.h"

GameProperty::GameProperty(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	this->setLayout(ui.gridLayout);
	resize(600, 400);

	set.push_back(langOption = new CLangEdit(this));
	ui.stackedWidget->addWidget(langOption);

	connect(ui.listWidget, SIGNAL(currentRowChanged(int)), ui.stackedWidget, SLOT(setCurrentIndex(int)));
	connect(ui.btnok, SIGNAL(clicked()), this, SLOT(downOK()));
	connect(ui.btnreset, SIGNAL(clicked()), this, SLOT(resetCurrent()));
	connect(ui.btnresetall, SIGNAL(clicked()), this, SLOT(resetAll()));

	ui.listWidget->setCurrentRow(0);
}

GameProperty::~GameProperty()
{

}

void GameProperty::downOK()
{
	for (auto &i : set)
	{
		i->save();
	}
	this->close();
}

void GameProperty::resetCurrent()
{
	set[ui.stackedWidget->currentIndex()]->reset();
}

void GameProperty::resetAll()
{
	for (auto &i : set)
	{
		i->reset();
	}
}
