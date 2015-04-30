#ifndef VERSIONINFO_H
#define VERSIONINFO_H

#include <QDialog>
#include "bkeproject.h"

namespace Ui {
class VersionInfo;
}

class VersionInfo : public QDialog
{
    Q_OBJECT
    typedef BkeProject::VersionData VersionData;
public:
    explicit VersionInfo(BkeProject *proj, QWidget *parent = 0);
    ~VersionInfo();

private slots:
    void on_listWidget_activated(const QModelIndex &index);

    void on_deleteButton_clicked();

    void on_addButton_clicked();

    void on_listWidget_clicked(const QModelIndex &index);

private:
    BkeProject *_project;
    QList<VersionData> &_versionDataList;
    Ui::VersionInfo *ui;
    void _buildDetailTable(const decltype(VersionData::data) &data);
    void _updateInfoWithIndex(int index);//更新右侧的信息和表格，index为-1表示清空。
    void _buildVersionList(); //建立左侧version data列表
};

#endif // VERSIONINFO_H
