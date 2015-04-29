#include "cdiroption.h"
#include "ui_cdiroption.h"

CDirOption::CDirOption(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CDirOption)
{
    ui->setupUi(this);
}

CDirOption::~CDirOption()
{
    delete ui;
}
