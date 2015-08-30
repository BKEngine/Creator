#ifndef QSEARCHLINEEDIT_H
#define QSEARCHLINEEDIT_H

#include <QLineEdit>
#include <QToolButton>
#include <QAction>
#include <QtCore>
#include <QLayout>
#include <QListWidget>
#include <QTreeWidgetItem>
#include "function.h"

class QSearchLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit QSearchLineEdit(const QString &backtext , QWidget *parent = 0);

signals:

public slots:
    void CheckText() ;
    void ItemDClick(QListWidgetItem* le) ;

private:
    QString BackText ;
    QToolButton *btns ;
    QListWidget *listwidget ;
    QList<QTreeWidgetItem*> ils ;

};

#endif // QSEARCHLINEEDIT_H
