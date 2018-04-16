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
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_checkBox_toggled(bool checked);

    void on_checkBox_2_toggled(bool checked);

    void on_checkBox_3_toggled(bool checked);

    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::BkeSpriteViewer *ui;
	DebugServer *debugServer;
	QGraphicsScene *scene;
	QGraphicsPixmapItem *pixmapItem;
	QGraphicsRectItem *rectItem;
	QTreeWidgetItem *currentItem = nullptr;
	void Init();
	void Clear();
	void Refresh();
};

#endif // BKESPRITEVIEWER_H
