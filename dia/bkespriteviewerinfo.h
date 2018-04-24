#ifndef BKESPRITEVIEWERINFO_H
#define BKESPRITEVIEWERINFO_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class BkeSpriteViewerInfo;
}

class BkeSpriteViewerInfo : public QWidget
{
    Q_OBJECT

public:
	enum State {
		Left,
		Right,
	};
public:
    explicit BkeSpriteViewerInfo(QWidget *parent = 0);
	void setState(State state);
	void setInfoIndex(int32_t index);
	void setInfoFilename(const QString &filename);
	void setInfoType(const QString &type);
	void setInfoPos(double x, double y, double glox, double gloy);
	void setInfoRect(const QRectF &rect);
	void setInfoOpacity(uint8_t opacity);
	void setInfoVisible(bool visible);
	void setInfoIsActing(bool acting);
    ~BkeSpriteViewerInfo();

private:
    Ui::BkeSpriteViewerInfo *ui;
};

#endif // BKESPRITEVIEWERINFO_H
