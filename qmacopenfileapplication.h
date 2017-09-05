#ifndef QMACOPENFILEAPPLICATION_H
#define QMACOPENFILEAPPLICATION_H

#include <QApplication>

class QMacOpenFileApplication : public QApplication
{
public:
    QMacOpenFileApplication(int &argc, char **argv)
        : QApplication(argc, argv)
    {
    }

    bool event(QEvent *event);
};

#endif // QMACOPENFILEAPPLICATION_H
