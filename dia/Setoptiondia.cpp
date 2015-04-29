#include "Setoptiondia.h"
#include "ui_Setoptiondia.h"

SetOptionDia::SetOptionDia(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetOptionDia)
{
    ui->setupUi(this);
    this->setLayout(ui->gridLayout);
    resize(600,400);

    CEditOption = new CTextEdit(this) ;
    dirOption = new CDirOption(this) ;
    skinOption = new CSkinOption(this) ;

    ui->stackedWidget->addWidget(CEditOption) ;
    ui->stackedWidget->addWidget(dirOption) ;
    ui->stackedWidget->addWidget(skinOption) ;

    connect(ui->listWidget,SIGNAL(currentRowChanged(int)),ui->stackedWidget,SLOT(setCurrentIndex(int))) ;
    connect(ui->btnok,SIGNAL(clicked()),this,SLOT(downOK())) ;
    ui->listWidget->setCurrentRow(0);
}

SetOptionDia::~SetOptionDia()
{
    delete ui;
}

void SetOptionDia::downOK()
{
    CEditOption->onSave();
    CEditOption->SetCurrentStyle("");

    BKE_SKIN_SETTING->setValue("StyleName",BKE_SKIN_CURRENT);

    this->close() ;
}
