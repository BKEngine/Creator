#pragma once

#include <QObject>
#include <QGraphicsView>

class QGraphicViewZoomer : public QObject
{
	Q_OBJECT

public:
	enum ZoomSource
	{
		Wheel,
		Gesture,
		ViewResize,
	};
public:
	QGraphicViewZoomer(QGraphicsView* view);

	void gentleZoom(double factor, ZoomSource source = ZoomSource::Wheel);
	void setModifiers(Qt::KeyboardModifiers modifiers);
	void setZoomFactorBase(double value);
	void setMaxFactor(double factor) { _maxFactor = factor; }
	double maxFactor() { return _maxFactor; }
	void setMinFactor(double factor) { _minFactor = factor; }
	double minFactor() { return _minFactor; }

private:
	QGraphicsView * _view;
	Qt::KeyboardModifiers _modifiers;
	double _zoomFactorBase;
	double _absoluteFactor;
	double _maxFactor;
	double _minFactor;
	QPointF targetScenePos, targetViewportPos;
	bool eventFilter(QObject* object, QEvent* event);

public slots:
	void reset();

signals:
	void zoomed();
};