#ifndef QT_H
#define QT_H

#include <QObject>
#include <QThread>
#include <QtCore>
#include <functional>
#include "../message.h"

#ifdef Q_OS_WIN
#define QT_STATBUF struct _stat64
#include "windows/helper.h"
inline int __stat(const char *filename, QT_STATBUF *stat)
{
    return _wstat64(to_wchar(filename).get_value().c_str(), stat);
}
#define QT_STAT __stat
#define QT_LSTAT QT_STAT
#else
#include <unistd.h>
#include <sys/stat.h>
#define QT_STATBUF struct stat
#define QT_STAT stat
#define QT_LSTAT lstat
#endif
#define QT_STAT_REG		S_IFREG
#define QT_STAT_DIR		S_IFDIR
#ifdef S_IFLNK
#define QT_STAT_LNK		S_IFLNK
#else
#define QT_STAT_LNK		0120000
#endif

class QtMainThreadCallback : public QObject
{
    Q_OBJECT
public:
    QtMainThreadCallback(const std::function<void()> &callback)
    {
        connect(this, &QtMainThreadCallback::_run, this, callback, Qt::QueuedConnection);
    }
    void run()
    {
        emit _run();
    }
signals:
    void _run();
};

inline EntryKind kind_from_stat(const QT_STATBUF &st)
{
  if ((st.st_mode & QT_STAT_LNK) == QT_STAT_LNK) return KIND_SYMLINK;
  if ((st.st_mode & QT_STAT_DIR) == QT_STAT_DIR) return KIND_DIRECTORY;
  if ((st.st_mode & QT_STAT_REG) == QT_STAT_REG) return KIND_FILE;
  return KIND_UNKNOWN;
}

#endif // QT_H
