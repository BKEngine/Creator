#include "QGraphicViewZoomer.h"
#include <QMouseEvent>
#include <QApplication>
#include <QScrollBar>
#include <qmath.h>

QGraphicViewZoomer::QGraphicViewZoomer(QGraphicsView* view)
	: QObject(view), _view(view), _absoluteFactor(1.0)
{
	_view->viewport()->installEventFilter(this);
	_view->setMouseTracking(true);
	_modifiers = Qt::NoModifier;
	_zoomFactorBase = 1.0015;
	_maxFactor = 2.0;
	_minFactor = 0.5;
}

void QGraphicViewZoomer::gentleZoom(double factor, ZoomSource source) {
	if (_absoluteFactor * factor > maxFactor())
		factor = maxFactor() / _absoluteFactor;
	if (_absoluteFactor * factor < minFactor())
		factor = minFactor() / _absoluteFactor;
	_absoluteFactor *= factor;
	_view->scale(factor, factor);
	if (source == ZoomSource::Wheel)
	{
		_view->centerOn(targetScenePos);
		QPointF deltaViewportPos = targetViewportPos - QPointF(_view->viewport()->width() / 2.0,
			_view->viewport()->height() / 2.0);
		QPointF viewportCenter = _view->mapFromScene(targetScenePos) - deltaViewportPos;
		_view->centerOn(_view->mapToScene(viewportCenter.toPoint()));
	}
	if (source != ZoomSource::ViewResize)
		emit zoomed();
}

void QGraphicViewZoomer::setModifiers(Qt::KeyboardModifiers modifiers) {
	_modifiers = modifiers;

}

void QGraphicViewZoomer::setZoomFactorBase(double value) {
	_zoomFactorBase = value;
}

bool QGraphicViewZoomer::eventFilter(QObject *object, QEvent *event) {
	if (event->type() == QEvent::MouseMove) {
		QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
		QPointF delta = targetViewportPos - mouse_event->pos();
		if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5) {
			targetViewportPos = mouse_event->pos();
			targetScenePos = _view->mapToScene(mouse_event->pos());
		}
	}
	else if (event->type() == QEvent::Wheel) {
		QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
		if (QApplication::keyboardModifiers() == _modifiers) {
			if (wheel_event->orientation() == Qt::Vertical) {
				double angle = wheel_event->angleDelta().y();
				double factor = qPow(_zoomFactorBase, angle);
				gentleZoom(factor, ZoomSource::Wheel);
				return true;
			}
		}
	}
	else if (event->type() == QEvent::NativeGesture)
	{
		QNativeGestureEvent * gestureEvent = static_cast<QNativeGestureEvent *>(event);
		if (gestureEvent->gestureType() == Qt::ZoomNativeGesture)
		{
			double factor = 1.0 + gestureEvent->value();
			gentleZoom(factor, ZoomSource::Gesture);
			return true;
		}
	}
	Q_UNUSED(object);
	return false;
}

void QGraphicViewZoomer::reset()
{
	_view->setTransform(QTransform());
	_absoluteFactor = 1.0;
}