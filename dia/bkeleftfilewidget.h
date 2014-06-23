#ifndef BKELEFTFILEWIDGET_H
#define BKELEFTFILEWIDGET_H

#include <QtCore>
#include <QListWidget>
#include <QAction>
#include <QMenu>

class BkeLeftFileWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit BkeLeftFileWidget(QWidget *parent = 0);

    QAction *btnCloseAll ;
    QAction *btnclose ;
    QAction *btncloseExp ;
signals:

public slots:
    void SetFileList(const QStringList &ls) ;
    void ShowRmenu( const QPoint & pos ) ;
private:
    QMenu *mn ;
};

#endif // BKELEFTFILEWIDGET_H
