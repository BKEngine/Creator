#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include "weh.h"
#include "codewindow.h"
#include "projectwindow.h"
#include "otherwindow.h"
#include "otherbasicwin.h"
#include "dia/bkeleftfilewidget.h"
#include "dia/qsearchlineedit.h"
#include "dia/lablesuredialog.h"
//#include "dia/cconfigdia.h"
#include "dia/Setoptiondia.h"
#include "dia/GameProperty.h"

extern QSplitter *ras[3];
extern CodeWindow *codeedit;
extern OtherWindow *otheredit;
extern ProjectWindow *projectedit;
extern BkeLeftFileWidget *fileListWidget;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);
	virtual ~MainWindow();
private:
	static MainWindow *_instance;
public:
	static MainWindow *getInstance(){ return _instance; }

signals:

public slots :
	void HiddenLeft();
	void AboutBkeCreator();
	void ClearMenu();
	void ReflashMenu();
	void Recentfile();
	void NormalAction();
	void Bedestroy();
	void CurrentFileChange(const QString &name, const QString &prodir);
	void HelpCreator();
	void OCupdate();
	void Config();
	void GameConfig();
	void ProjectOpen(BkeProject *p);
	void ProjectClose();


	//发布游戏
	void ReleaseGame();

	void CurrentProChange(BkeProject *pro);

private:
	//    CodeWindow *codeedit ;
	//    FileWindow *fileedit ;
	//    OtherWindow *otheredit ;
	//    ProjectWindow *projectedit ;
	//    BkeLeftFileWidget *fileListWidget ;

	//QSplitter *ras[3] ;
	QToolBar *downbar;

	QMenu *wmenu;
	QMenu *rpmenu;
	QAction *btnhiddenleftact;
	QAction *btnnewprojectact;
	QAction *btnopenprojectact;
	QAction *btnopenfileact;
	QAction *btnnewfileact;
	QAction *btnReleaseGame;
	QAction *btncloseprojectact;
	QToolBar *btnbar;

	QAction *option_prop;

	QSearchLineEdit *editsearch;

	QVector<QFile*> FilePtrList;
	QStringList OpenNameList;
	QStringList upList;

	//各类视图
	QTreeWidget *ProjectBasic;
	QListWidget *OpenFileBasic;
	QTreeWidget *BookMarkBasic;
	QTreeWidget *MarkBasic;

	void CreateMenu();
	void CreateDownBar();
	void BtnChangeStyle(QWidget *btn);
	QList<QAction*> SetMenuList(QMenu *mn, const QStringList &list);

	void test();
protected:
	void closeEvent(QCloseEvent *e);
	bool eventFilter(QObject * watched, QEvent * event);

public:
	void CheckUpdate();

private:
	QProcess *checkUpdate = NULL;
};

#endif // MAINWINDOW_H
