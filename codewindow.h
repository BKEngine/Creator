#ifndef CODEWINDOW_H
#define CODEWINDOW_H
#include "weh.h"
#include <QDockWidget>
#include <QProcess>
#include "topbarwindow.h"
#include "bkeSci/bkescintilla.h"
#include "projectwindow.h"
#include "bkeSci/bkecompile.h"
#include "bkeSci/bkemarks.h"
#include "otherwindow.h"
#include <Qsci/qscistyle.h>
#include <QProgressBar>
#include <QFileSystemWatcher>
#include "dia/searchbox.h"
#include "dia/bkeleftfilewidget.h"
#include "DebugServer.h"

class BkeDocBase
{
public:
	BkeScintilla *edit;

	BkeDocBase(){ edit = 0; }
	~BkeDocBase(){
		//delete edit;
		if (fileIO.isOpen()) fileIO.close();
	}

	QString FullName(){ return fullname; }
	QString Name(){ return name; }
	QString ProjectDir(){ return prodir; }
	QFile* File(){ return &fileIO; }

	bool isNull(){  //不是绝对的
		return edit == 0;
	}

	bool isProjectFile(){
		return !prodir.isEmpty();
	}

	void SetProjectDir(const QString &n){
		prodir = n;
	}

	void SetFileName(const QString &e){
		if (fileIO.isOpen()) fileIO.close();
		fullname = e;
		if (e.lastIndexOf('/') > 0){
			name = e.right(e.length() - e.lastIndexOf('/') - 1);
		}
		else name = e;
		info.setFile(fullname);
		fileIO.setFileName(fullname);
		upFileTime();
	}

	void upFileTime(){
		info.refresh();
	}

	bool isFileChange(){  //文件是否从外部被改变
		QDateTime t1 = info.lastModified();
		info.refresh();
		return t1 != info.lastModified();
	}

	QString dir(){
		QString temp;
		temp = fullname.left(fullname.length() - name.length());
		if (temp.endsWith("/")) return temp.left(temp.length() - 1);
		return temp;
	}

	QWidget *widget(){ return edit; }

private:
	QFile fileIO;
	QString fullname;
	QString name;
	QString prodir;
	QFileInfo info;
};


class CodeWindow : public QMainWindow
{
	Q_OBJECT

public:

	~CodeWindow();
	CodeWindow(QWidget *parent = 0);

	QSize sizeHint() const;
	void BindOtherWindow(OtherWindow *win);
	void BindProjectWindow(ProjectWindow *p);
	void BindFileListWidget(BkeLeftFileWidget *flist);
	enum{
		ERROR_NO,
		ERROR_SAVE
	};

	BkeLeftFileWidget *filewidget;

	QAction *jumpToDef;
	QAction *jumpToCode;
	QAction *gotoLabel;

	QAction *btnnextact;
	QAction *btnlastact;
	QAction *btnsaveact;
	QAction *btnsaveasact;
	QAction *btncopyact;
	QAction *btncutact;
	QAction *btnpasteact;
	QAction *btncompileact;
	QAction *btncompilelang;
	QAction *btncompilerunact;
	QAction *btnrunact;
	QAction *btndebugact;
	QAction *btncloseact;
	QAction *btnfindact;
	QAction *btnfindactall;
	QAction *btnreplaceact;
	QAction *btnreplaceallact;
	QAction *btnbookmarkact;
	QAction *btnmarkact;
	QAction *btnrunfromlabel;
	QAction *btnredoact;
	QAction *btnundoact;
	QAction *btnclearact;
	QAction *btncodeact;
	QAction *btnselectall;
	QAction *btnfly;
	QAction *btngotolabellist;
	QAction *btngotofile;
	QAction *btnswitchfold;	//	全部折叠/全部展开
	QToolBar *toolbar;
	QComboBox *slablelist;
	QStringList slablels;
	bool ignoreActive;

signals:
    void CurrentFileChange(QString file);
	void CurrentFileChange(const QString &name, const QString &prodir);
	void FileAddProject(const QString &file);

	void CompileFinish(int errors);
signals:
	void searchOne(const QString &file, const QString &fullfile, int line, int start, int end);

public slots:
	void onTimer();
	bool CloseAll();
	void ChangeCurrentEdit(int pos);
	void SetCurrentEdit(int pos);
	//file:绝对路径
	void AddFile(const QString &file);
	void TextInsert(const QString &text);
	void DocChange(bool m);
	void NextNavigation();
	void LastNavigation();
	void SaveFile();
	void SaveAs();
	void SaveALL();
	void CloseFile();
	void CloseFile(int pos);
	void ImportBeChange(const QString &text, int type);
	void FileWillBeDel(const QString &file);
	void Compile();
	void CompileAll(bool release);
	void CompileAll();
	void CompileLang(bool release);
	void CompileLang();
	void CompileFinish();
	void CompileError(QString s);
	void CompileAndRun(const QStringList &extraArgs = QStringList());
	void FileNameChange(const QString &oldname, const QString &newname, bool &c);
	void ToLocation(BkeMarkerBase *p, const QString &prodir);
	void ShowRmenu(const QPoint & pos);
	void AddBookMark();
	void RunBKE();
	void RunBKEWithArgs();
	void AnnotateSelect();
	void ClearCompile();
	void ChangeCodec();
	void NewEmptyFile();
	void FileReadyToCompile(int i);
	void ChangeProject(BkeProject *p);
	void TextToMarks(const QString &text, const QString &dir, int type);
	void deleteCompileFile();
	void SelectAll();
	void QfileChange(const QString &path);
	void GotoLine();
	void GotoLabel(int i);
	void GotoLabel(QString label);
	void ActUndo();
	void ActRedo();
	void ActCurrentChange();
	void ActCut();
	void ActPaste();
	void ActCopy();
	void GotoLabelList();
	void GotoFile();
	void Rename(const QString &old, const QString &now);
	void searchOneFile(const QString &file, const QString &searchstr, bool iscase, bool isregular, bool isword);
	void searchAllFile(const QString &searchstr, bool iscase, bool isregular, bool isword);
	void replaceOneFile(const QString &file, const QString &searchstr, const QString &replacestr, bool iscase, bool isregular, bool isword, bool stayopen);
	void replaceAllFile(const QString &searchstr, const QString &replacestr, bool iscase, bool isregular, bool isword, bool stayopen);
	void resetLexer();
	void refreshLabel(BkeScintilla *sci);
	void refreshLabel(QSortedSet<QString> &l);
	void SwitchFold();
	void jumpToDefFunc();
	void jumpToCodeFunc();
	void jumpToLabelFunc();
	void AddNavigation(const QString &file, int line);
	void RemoveNavigation(const QString &file);
	void CreateAndGotoLabel(QString label);

private slots:
	void ShouldAddToNavigation();
	void RefreshNavigation();
	void NavigateTo(const QPair<QString, int> &target);

public:
	void backupAll();
	static QString getScenarioTextFromCode(QString text);

private:
	QList<QPair<QString, int>> navigationList;
	int currentNavigation = -1;
	int navigationLocker = 0;
	OtherWindow *othwin;
	ProjectWindow *prowin;
	BkeProject *workpro;
	QStackedWidget *stackwidget;

	QStringList ItemTextList;
	QComboBox *lablelist;

	BkeScintilla *currentedit;
	BkeDocBase* currentbase;

	QHash<QString, BkeDocBase*> docStrHash;
	QHash<QWidget*, BkeDocBase*> docWidgetHash;
	QFileSystemWatcher *filewatcher ;


	QToolBar *toolbar2;
	BkeCompile comtool;
    BkeMarkSupport markadmin;
	QAction *pdeletebookmark;
	QAction *pdeletemark;
	QAction *pannote;
	SearchBox *diasearch;
	QProgressBar *kag;
	BkeScintilla *searchlablelater;

	int currentpos;
	int errorinfo;
	bool ignoreflag;
	bool isRun;
	bool isSearLable;
	int watcherflag ;
	int isCompileNotice;
	bool labelbanned;

	QSize hint;
	QsciStyle ks1;
	QsciStyle ks2;

	QStringList BKE_extraArgs;

	enum{
		_NOTICE_ALWAYS,
		_NOTICE_NOSAVE,
		_NOTICE_ALLSAVE
	};

	QTimer tm;
	void simpleNew(BkeDocBase *loli, const QString &t);
	void simpleSave(BkeDocBase *loli);
	bool simpleBackup(BkeDocBase *loli);
	void simpleClose(BkeDocBase *loli);
	void SetCurrentEdit(QWidget *w);
	void CreateBtn();
	void btnDisable();
	//void btnAble() ;
	void CurrentConnect(bool c);
	//bool ReadyCompile(const QString &file) ;
	void CheckProblemMarks(BkeScintilla *edit, BkeMarkList *list);
	void CheckRuntimeProblemMarks(BkeScintilla *edit, BkeMarkList *list);
	void CheckBookMarks(BkeScintilla *edit, BkeMarkList *list);
	void CheckMarks(BkeScintilla *edit, BkeMarkList *list);
	void AddMarksToEdit();
	void FileIOclose(const QStringList &list);
	void DrawLine(bool isClear);
	bool WriteOpenFile(const QString &dir);
	void StartBKEProcess(const QStringList &args);
	QProcess *bkeprocess = nullptr;

	/////////////Debug Component///////////////
	DebugServer *debugServer;
private slots:
	void DebugLogReceived(int32_t, QString);
};

#endif // CODEWINDOW_H
