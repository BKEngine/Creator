#ifndef FUNCTION_H
#define FUNCTION_H

//#include <Qsci/qsciscintilla.h>
#include <QtCore>
#include "loli/loli_island.h"

//公用函数，公用的变量
extern QString BKE_CURRENT_DIR;
extern QString BKE_PROJECT_NAME;
extern QString BKE_USE_NAME;
//extern QString BKE_API_FILE ;
extern QString BKE_CREATOR_VERTION;
extern QStringList BKE_Recently_Project;
//extern QStringList BKE_Recently_Files ;
extern QJsonObject BKE_MARKS_OBJECT;
extern QSettings *BKE_CLOSE_SETTING;
extern QSettings *BKE_USER_SETTING;
extern QSettings *BKE_SKIN_SETTING;
extern QString BKE_SKIN_CURRENT;
//extern QJsonObject BKE_QCSS_OBJECT ;
extern bool isSYSTEMP_LOWDER;

#define BKE_CREATOR_TITLE "BKE Creator"
#define BKE_CREATOR_TITLE_PROJECT_PREFIX BKE_CREATOR_TITLE " - "
#define BKE_CREATOR_TITLE_PROJECT(p) ((BKE_CREATOR_TITLE_PROJECT_PREFIX) + p) 

QString LOLI_MID(const QString &text, const QString from, const QString to, int pos = 0);
QString LOLI_OS_QSTRING(const QString &text);
bool    LOLI_MAKE_NULL_FILE(const QString &filename, const QString &stencilname = QString());
void    LOLI_CLEAR_TEMP(const QString &dir, const QString &sfix = QString());
int     LOLI_SORT_INSERT(QStringList &list, const QString &s);
QString LOLI_AUTONEXT_QSTRING(QString text, int len = 20);
bool copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist, const QString &sfix);
QString BkeFullnameToName(const QString &fullname, const QString &dir);
QStringList ListDirsCopy(QStringList &list, const QString &dir, const QString &newdir);

namespace BkeCreator {
	void LoadRecentProject();
	void AddRecentProject(const QString &file);
	void RemoveRecentProject(const QString & file);
	//void AddRecentFile(const QString &file) ;
	//void ReNameRecentFile(const QString &old, const QString &now);
	//void ReadApiList(QStringList *ls, const QString &name, int type);
	QStringList CopyStencil(const QString &dir, const QStringList &ls);
	QString IntToRgbString(unsigned int rgb);
}

inline QString chopFileExt(const QString &filename)
{
	QString ext = QFileInfo(filename).suffix();
	return ext.isEmpty() ? QString() : '.' + ext;
}

inline QString chopFileName(const QString &fullname)
{
	return QFileInfo(fullname).fileName();
}

inline QString chopFileNameWithoutExt(const QString &fullname)
{
	auto f = chopFileName(fullname);
	return f.left(f.size() - chopFileExt(f).size());
}

bool BkeCopyDirRecursively(QString fromDir, QString toDir, bool replaceOnConflict);

#endif // FUNCTION_H
