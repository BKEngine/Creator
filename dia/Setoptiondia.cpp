#include "Setoptiondia.h"
#include "ui_Setoptiondia.h"

SetOptionDia::SetOptionDia(QWidget *parent) :
QDialog(parent),
ui(new Ui::SetOptionDia)
{
	ui->setupUi(this);
	this->setLayout(ui->gridLayout);
	resize(600, 400);

	set.push_back(CEditOption = new CTextEdit(this));
	set.push_back(dirOption = new CDirOption(this));
	set.push_back(skinOption = new CSkinOption(this));

	ui->stackedWidget->addWidget(CEditOption);
	ui->stackedWidget->addWidget(dirOption);
	ui->stackedWidget->addWidget(skinOption);

	connect(ui->listWidget, SIGNAL(currentRowChanged(int)), ui->stackedWidget, SLOT(setCurrentIndex(int)));
	connect(ui->btnok, SIGNAL(clicked()), this, SLOT(downOK()));
	connect(ui->btnreset, SIGNAL(clicked()), this, SLOT(resetCurrent()));
	connect(ui->btnresetall, SIGNAL(clicked()), this, SLOT(resetAll()));
	ui->listWidget->setCurrentRow(0);
}

SetOptionDia::~SetOptionDia()
{
	delete ui;
}

void SetOptionDia::downOK()
{
	for (auto &i : set)
	{
		i->save();
	}

	CEditOption->SetCurrentStyle("");

	BKE_SKIN_SETTING->setValue("StyleName", BKE_SKIN_CURRENT);

	this->close();
}

void SetOptionDia::resetCurrent()
{
	set[ui->stackedWidget->currentIndex()]->reset();
}

void SetOptionDia::resetAll()
{
	for (auto &i : set)
	{
		i->reset();
	}
}
