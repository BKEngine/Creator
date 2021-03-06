#ifndef BKESPRITEVIEWER_H
#define BKESPRITEVIEWER_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

namespace Ui {
class BkeSpriteViewer;
}

class DebugServer;
class QTreeWidgetItem;
class BkeSpriteViewerInfo;
class BkeSpriteViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit BkeSpriteViewer(DebugServer *debugServer, QWidget *parent = 0);
    ~BkeSpriteViewer();

private slots:
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_checkBox_toggled(bool checked);

    void on_checkBox_2_toggled(bool checked);

    void on_comboBox_currentIndexChanged(int index);

    void on_action_toggled(bool arg1);

signals:
	void onNewImage();

private:
    Ui::BkeSpriteViewer *ui;
	DebugServer *debugServer;
	QGraphicsScene *scene;
	QGraphicsPixmapItem *pixmapItem;
	QGraphicsRectItem *rectItem;
	QTreeWidgetItem *currentItem = nullptr;
	BkeSpriteViewerInfo *info;
	void Init();
	void Clear();
	void Refresh();
};

#endif // BKESPRITEVIEWER_H
