#include "qfswatcher.h"
#include "src/hub.h"
#include <QPointer>
#include <QDebug>

class QFsWatcherPrivate
{
    Q_DECLARE_PUBLIC(QFsWatcher)
public:
    QFsWatcherPrivate(QFsWatcher *q)
        :q_ptr(q)
    {
    }
    ~QFsWatcherPrivate()
    {
        for(auto &&path : watchMap)
        {
            Hub::get().unwatch(path, nullptr);
        }
    }
    void watch(const QString &path, bool recursive)
    {
        Q_Q(QFsWatcher);
        QPointer<QFsWatcher> qp(q);
        Hub::get().watch(path.toStdString(), false, recursive, [this, qp, path](auto err, auto channelId){
            if(!qp)
                return;
            if(channelId){
                watchMap.insert(path, *channelId.get());
            }
        }, [this, qp](std::unique_ptr<std::string> a, std::unique_ptr<std::vector<FileSystemEvent>> b){
            if(!qp)
                return;
            onEvent(std::move(a), std::move(b));
        });
    }
    bool unwatch(const QString &path)
    {
        auto it = watchMap.find(path);
        if(it != watchMap.end()){
            Hub::get().unwatch(it.value(), nullptr);
            watchMap.erase(it);
            return true;
        } else {
            return false;
        }
    }
private:
    QFsWatcherEntryType convert(EntryKind kind)
    {
        switch(kind)
        {
        case KIND_DIRECTORY:
            return QFSW_TYPE_DIRECTORY;
        case KIND_FILE:
            return QFSW_TYPE_FILE;
        }
        return QFSW_TYPE_UNKNOWN;
    }
    void onEvent(std::unique_ptr<std::string> perror, std::unique_ptr<std::vector<FileSystemEvent>> pevents)
    {
        if(perror)
        {
            qDebug() << QString::fromStdString(*perror.get());
        }
        else if(pevents)
        {
            Q_Q(QFsWatcher);
            std::vector<FileSystemEvent> &events = *pevents.get();
            for(auto &&event : events)
            {
                switch(event.get_filesystem_action())
                {
                case ACTION_CREATED:
                    emit q->onCreate(QString::fromStdString(event.get_path()), convert(event.get_entry_kind()));
                    break;
                case ACTION_DELETED:
                    emit q->onDelete(QString::fromStdString(event.get_path()), convert(event.get_entry_kind()));
                    break;
                case ACTION_RENAMED:
                    emit q->onRename(QString::fromStdString(event.get_old_path()), QString::fromStdString(event.get_path()), convert(event.get_entry_kind()));
                    break;
                }
            }
        }
    }
    QFsWatcher *const q_ptr;
    QHash<QString, ChannelID> watchMap;
};

QFsWatcher::QFsWatcher(QObject *parent)
    : QObject(parent)
    , d_ptr(new QFsWatcherPrivate(this))
{
}

QFsWatcher::~QFsWatcher()
{
    Q_D(QFsWatcher);
    delete d;
}

void QFsWatcher::watch(const QString &root, bool recursive)
{
    Q_D(QFsWatcher);
    d->watch(root, recursive);
}

bool QFsWatcher::unwatch(const QString &root)
{
    Q_D(QFsWatcher);
    return d->unwatch(root);
}


