#include "qmacopenfileapplication.h"
#include <QFileOpenEvent>
#include <weh.h>
#include "projectwindow.h"

bool QMacOpenFileApplication::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        projectedit->OpenProject(openEvent->file());
    }

    return QApplication::event(event);
}
