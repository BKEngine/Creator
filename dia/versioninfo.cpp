#include "versioninfo.h"
#include "ui_versioninfo.h"

VersionInfo::VersionInfo(BkeProject *proj, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VersionInfo),
    _project(proj),
    _versionDataList(proj->getVersionDataList())
{
    ui->setupUi(this);
    _buildVersionList();
    ui->tableWidget->setHorizontalHeaderLabels(QStringList()<<"文件"<<"修改日期");
}

VersionInfo::~VersionInfo()
{
    delete ui;
}

void VersionInfo::_updateInfoWithIndex(int index)
{
    if(index<0)
    {
        ui->nameLabel->clear();
        ui->timeLabel->clear();
        ui->infoLabel->clear();
        ui->tableWidget->clear();
        ui->deleteButton->setEnabled(false);
    }
    else
    {
        auto && versionData = _versionDataList[index];
        ui->nameLabel->setText(versionData.name);
        ui->timeLabel->setText(versionData.date.toLocalTime().toString("yyyy-MM-dd hh:mm:ss"));
        ui->infoLabel->setText(versionData.info);
        _buildDetailTable(versionData.data);
        ui->deleteButton->setEnabled(true);
    }
}

void VersionInfo::_buildVersionList()
{
    ui->listWidget->clear();
    for(int i = 0;i < _versionDataList.size(); i++)
    {
        ui->listWidget->addItem(_versionDataList[i].name);
    }
}

void VersionInfo::on_listWidget_activated(const QModelIndex &index)
{
    if(index.isValid())
    {
        _updateInfoWithIndex(index.row());
    }
    else
    {
        _updateInfoWithIndex(-1);
    }
}

void VersionInfo::_buildDetailTable(const decltype(VersionData::data) &data)
{
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(data.count());
    ui->tableWidget->setColumnCount(2);
    int currentRow = 0;
    for(auto it = data.begin(); it!= data.end(); it++, currentRow++)
    {
        ui->tableWidget->setItem(currentRow, 0, new QTableWidgetItem(it.key()));
        ui->tableWidget->setItem(currentRow, 1, new QTableWidgetItem(it.value().toLocalTime().toString("yyyy-MM-dd hh:mm:ss")));
    }
}

void VersionInfo::on_deleteButton_clicked()
{
    int index = ui->listWidget->currentRow();
    if(index<0)
        return;
    auto result = QMessageBox::question(this, "删除","是否删除该版本信息？");
    if(result != QMessageBox::Yes)
        return;
    ui->listWidget->removeItemWidget(ui->listWidget->currentItem());
    _versionDataList.removeAt(index);
    _updateInfoWithIndex(-1);
}

void VersionInfo::on_addButton_clicked()
{
    int index = _project->addVersionData(this);
    if(index >= 0)
    {
        _buildVersionList();
        ui->listWidget->setCurrentRow(index);
    }
}

void VersionInfo::on_listWidget_clicked(const QModelIndex &index)
{
    if(index.isValid())
    {
        _updateInfoWithIndex(index.row());
    }
    else
    {
        _updateInfoWithIndex(-1);
    }
}
