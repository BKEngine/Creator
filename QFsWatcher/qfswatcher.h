#ifndef QFSWATCHER_H
#define QFSWATCHER_H

#include <QObject>

enum QFsWatcherEntryType
{
    QFSW_TYPE_UNKNOWN,
    QFSW_TYPE_FILE,
    QFSW_TYPE_DIRECTORY,
};

class QFsWatcherPrivate;
class QFsWatcher : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QFsWatcher)
public:
    QFsWatcher(QObject *parent = nullptr);
    ~QFsWatcher();
    void watch(const QString &root, bool recursive);
    bool unwatch(const QString &root);

signals:
    void onCreate(QString path, QFsWatcherEntryType type);
    void onDelete(QString path, QFsWatcherEntryType type);
    void onRename(QString oldpath, QString path, QFsWatcherEntryType type);

private:
    QFsWatcherPrivate *const d_ptr;
};

#endif // QFSWATCHER_H
