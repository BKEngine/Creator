#ifndef QSEARCHLINEEDIT_H
#define QSEARCHLINEEDIT_H

#include <QLineEdit>
#include <QToolButton>
#include <QAction>
#include <QAction>
#include <QtCore>
#include <QLayout>
#include <QListWidget>
#include <QTreeWidgetItem>
#include <QToolBar>
#include <QComboBox>
#include "function.h"

class QSearchLineEdit : public QToolBar
{
    Q_OBJECT
public:
    explicit QSearchLineEdit(QWidget *parent = 0);
    void addHistory(const QString &text) ;

public slots:
    void CheckText(const QString &text) ;
    void SearchText() ;
    void showHistory() ;

private:
    QToolButton *btnrecord ;
    QAction *btnclose ;
    QComboBox *edittext ;
    QStringList history ;
} ;

#endif // QSEARCHLINEEDIT_H
