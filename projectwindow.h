#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include "weh.h"
#include <QDialogButtonBox>
#include "topbarwindow.h"
#include "bkeproject.h"
#include "dia/bkeconfigdialog.h"

//extern QList<BkeProject*> projectlist ;


// 工程树窗口
class ProjectWindow : public QTreeWidget
{
	Q_OBJECT
public:
	enum{
		//btn_active,
		btn_compile,
		btn_release,
		btn_insertdir,
		btn_preview,
		btn_newscript,
		btn_newimport,
		btn_addfile,
		btn_adddir,
		btn_exportscenario,
		btn_showindir,
		btn_search,
		btn_remove,
		btn_close,
		btn_rename,
		btn_runfromit,
		BTN_COUNT
	};

	~ProjectWindow();
	ProjectWindow(QWidget *parent = 0);
	void OpenProject(const QString &file);
	bool hasProject(){ return !!workpro; }

	BkeProject *workpro;

signals:
	void OpenBkscrFile(const QString &file);
	void DirWillBeInsert(const QString &text);
	void CheckFileOpen(const QString &file, bool &IsOpen);
	void ImportFileChange(const QString &text, int type);
	void FileNameChange(const QString &oldname, const QString &newname);
	void CurrentProChange(BkeProject *pro);
	void Compile();
	void TextToMarks(const QString &text, const QString &dir, int type);

	void onProjectOpen(BkeProject *p);
	void onProjectClose();

public slots:
	void NewProject();
	void OpenProject();
	void ItemDoubleClick(QTreeWidgetItem * item, int column);
	void OpenProjectFile(QString);
	void ShowRmenu(const QPoint & pos);
	void SetCurrentItem(const QString &file);
	//void ReName() ;
	void OpenFile();
	void ActionAdmin();
	void CloseProject();

private:
	//QList<BKEproject*> projectlist ;
	//BkeProject *temppro ;
	ItemInfo info;

	QAction *btns[BTN_COUNT];
	QAction *action_del;

	bool ReadItemInfo(QTreeWidgetItem *dest, ItemInfo &f);
	void BkeChangeCurrentProject(/*BkeProject *p*/);
	void ConfigProject(BkeProjectConfig *config);
	void NewFile(const ItemInfo &f, int type);
	void DeleteFile(const ItemInfo &f);
	void AddFiles(const ItemInfo &f);
	void AddDir(const ItemInfo &f);
	void PreviewFile(const ItemInfo &f);
	void CloseProject(const ItemInfo &f);
	void RenameFile(const ItemInfo &f);
	//void Active(const ItemInfo &f);
	void ShowInDir(const ItemInfo &f);
	void ReleaseGame(const ItemInfo &f);
	void ExportScenario(const ItemInfo &f);
	QTreeWidgetItem *findFileInProject(const QString &name);
	void RunBKEFromFile(const ItemInfo &f);

private slots:
	void DeleteCurrentFile();

public:
	void ReleaseCurrentGame();
};



#endif // PROJECTWINDOW_H
