#ifndef BKESPRITEVIEWER_H
#define BKESPRITEVIEWER_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

namespace Ui {
class BkeSpriteViewer;
}

class DebugServer;
class QTreeWidgetItem;
class BkeSpriteViewer : public QDialog
{
    Q_OBJECT

public:
    explicit BkeSpriteViewer(DebugServer *debugServer, QWidget *parent = 0);
    ~BkeSpriteViewer();

private slots:
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

private:
    Ui::BkeSpriteViewer *ui;
	DebugServer *debugServer;
	QGraphicsScene *scene;
	QGraphicsPixmapItem *pixmapItem;
	void Init();
	void Clear();
};

#endif // BKESPRITEVIEWER_H
