#ifndef BKECONFIGDIALOG_H
#define BKECONFIGDIALOG_H

#include <QDialog>
#include "bkeprojectconfig.h"

namespace Ui {
class bkeconfigdialog;
}

class QLineEdit;
class QBkeConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QBkeConfigDialog(QWidget *parent = 0);
    void StartConfig(BkeProjectConfig *config) ;
    ~QBkeConfigDialog();

private slots:

    void refreshUI();

    void on_picASDBtn_clicked();

    void on_audioASDBtn_clicked();

    void on_scriptASDBtn_clicked();

    void on_savebtn_clicked();

    void on_saveDirBtn_clicked();

private:
    Ui::bkeconfigdialog *ui;
    BkeProjectConfig *config;

    void OpenDir(QLineEdit *dst, bool multiselectMode);
    void callText(QWidget *w, QPoint size,const QString &info);
};

#endif // BKECONFIGDIALOG_H
