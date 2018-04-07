#ifndef BKESPRITEVIEWER_H
#define BKESPRITEVIEWER_H

#include <QWidget>

namespace Ui {
class BkeSpriteViewer;
}

class DebugServer;
class BkeSpriteViewer : public QWidget
{
    Q_OBJECT

public:
    explicit BkeSpriteViewer(DebugServer *debugServer, QWidget *parent = 0);
    ~BkeSpriteViewer();

private:
    Ui::BkeSpriteViewer *ui;
	DebugServer *debugServer;
	void Init();
};

#endif // BKESPRITEVIEWER_H
