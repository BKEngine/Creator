#ifndef BKESCINTILLA_H
#define BKESCINTILLA_H

#include <QWidget>
#include <Qsci/qsciscintilla.h>
#include <QMessageBox>
#include <QToolTip>
#include <QSortedSet>
#include "bkeSci/qscilexerbkescript.h"
#include "bkeSci/BkeIndicatorBase.h"
#include <weh.h>
#include "../BG_Analysis.h"

class BkeDocBase;
class BkeProject;

class BkeScintilla : public QsciScintilla
{
	Q_OBJECT
public:
	BkeScintilla(QWidget *parent=0 ) ;
	~BkeScintilla() ;

	//填充状态
	enum{
		AUTO_NULL ,
		AUTO_COMMAND ,
		AUTO_ATRRIBUTE ,
		AUTO_FINISH ,
		AUTO_LINEERROR ,
		AUTO_WAIT ,
		P_AUTO_NORMAL ,
		P_AUTO_NULL ,
		P_AUTO_START
	} ;
	enum{
		BKE_INDICATOR_FIND = 8,
		BKE_INDICATOR_ERROR,
		BKE_INDICATOR_WARNING,
		BKE_INDICATOR_CLICK_COMMAND,
		BKE_INDICATOR_CLICK_LABEL,
	};
	enum{
		BKE_CHANGE_REPLACE = 0x1
	};
	//所有搜索结果的indicator
	QList<BkeIndicatorBase> testlist ;
	//为了显示选择效果而隐藏的indicator
	BkeIndicatorBase findlast;
	QString FileName;

	QString GetLastError(){ return errorinfo; }
	void    DefineIndicators(int id,int intype) ;
	void    ClearIndicators(int id) ;
	void ClearIndicator(int id, const BkeIndicatorBase &p) ;
	int findFirst1(const QString fstr,bool cs,bool exp,bool word,bool mark = true) ;
	void ReplaceAllFind(const QString &rstr) ;
	bool ReplaceText(const QString &rstr, const QString &dstr, bool cs, bool exp, bool word);
	void ReplaceAllText(const QString &rstr, const QString &dstr, bool cs, bool exp, bool word);
	BkeIndicatorBase ReplaceFind(const QString &rstr);
	int findIndicatorStart(int id,int from) ;
	int findIndicatorEnd(int id,int from) ; //不是以ID开头直接返回-1
	BkeIndicatorBase findIndicatorLast(int id,int from) ;
	bool FindForward(int pos = 0) ;
	bool FindBack(int pos = -1) ;
	bool HasFind(){ return findcount > 0 ; }
	void clearSelection(int pos = -1) ;
	void SetIndicator(int id, const BkeIndicatorBase &p) ;
	void BkeAnnotateSelect() ;
	BkeIndicatorBase findIndicator(int id,int postion) ;
	int GetCurrentLine();
	int GetCurrentPosition();
	void SetCurrentPosition(int pos);
	void setLexer(QsciLexer *lex = 0);
	void setSelection(BkeIndicatorBase &p);
	int GetTextLength();
	int ClosedPositionAt(const QPoint &point);
	int PositionAt(const QPoint & point);
	QPoint PointByPosition(int position);
	bool IsIndicator(int id, int pos);
	int GetByte(int pos) const;
	QString TextForRange(const BkeIndicatorBase &range);

	int findcount ;

	int topLine;

	BkeDocBase *basedoc;
	BkeProject *workpro;
	QsciLexerBkeScript *deflex;
	//ParseData *pdata;
	BG_Analysis *analysis;

	int positionFromLineIndexByte(int line, int index) const
	{
		return SendScintilla(SCI_POSITIONFROMLINE, line) + index;
	}

	void lineIndexFromPositionByte(int position, int *line, int *index) const
	{
		*line = SendScintilla(SCI_LINEFROMPOSITION, position);
		*index = position - SendScintilla(SCI_POSITIONFROMLINE, *line);
	}

	int lineFromPosition(int position) const
	{
		return SendScintilla(SCI_LINEFROMPOSITION, position);
	}

signals:
	void refreshLabel(BkeScintilla *sci);
	void refreshLabel(QSortedSet<QString> &l);
	void ShouldAddToNavigation();

public slots:
//    void undo () ;
//    void redo() ;
private slots:
	void EditModified(int pos, int mtype, const char *text,
									int len, int added, int line, int foldNow, int foldPrev, int token,
									int annotationLinesAdded) ;
	void UiChange(int updated) ;
	void UseListChoose(const char* text ,int id ) ;
	void AutoListChoose(const char* text ,int pos ) ;
	void ChooseComplete(const char *text,int pos) ;
	void InsertAndMove(const QString &text);
	void CurrentPosChanged(int line , int index );
	void CharHandle(int cc) ;
	QFont GetAnnotationFont();
	void onTimer();

public:
	int ChangeStateFlag;
	int ChangeIgnore;
	bool refind;

	void saveTopLine();
	void restoreTopLine();

private:
	QTimer tm;
	int AutoState;
	int vautostate ;
	int StartWordPos ;
	int ChangeType;
	int LastKeywordEnd ;
	int ComPleteLeast ;
	int IndentCount ;
	bool UseCallApi ;
	bool IsNewLine ;
	bool IsWorkingUndo ;
	int LastLine ;
	int SectionPos ;
	int findstr_length;
	BkeModifiedBase modfieddata ;
	BkeModifiedBase indentdate ;
	int findflag ;
	const char *comss ;
	QByteArray fstrdata ;
	QString Separate ;
	QString LaterInsertWord ;   //Ui更新后插入

	QFile fileIO ;
	QString errorinfo ;

	bool IsSeparate(int ch) ;
	void RemoveDou() ;
	void BkeStartUndoAction(bool newUndo = true) ;//记录Undo，如果当前正在记录，那么newUndo决定是记录一个新的还是继续当前记录
	void BkeEndUndoAction() ;
	int GetActualIndentCharLength(int lineID) ;

	BkeIndicatorBase simpleFind(const char *ss , int flag,int from,int to) ;

protected:

	//补全相关
	enum{
		SHOW_NULL,
		SHOW_USECOMMANDLIST,
		SHOW_AUTOCOMMANDLIST,
		SHOW_AUTOVALLIST,
		SHOW_ENUMLIST,
		SHOW_USEVALLIST,
		SHOW_LABEL,
		SHOW_ATTR,
		SHOW_SYS
	}completeType;

	QString completeList;

	QStringList getScriptList();
	void showComplete();
	QString getAttrs(const QString &name, const QStringList &attrs, const QString &alltext);
	QString getEnums(const QString &name, const QString &attr, const QString &alltext);
	QString getValList(const QStringList &ls, const QString &alltext);
	QString getGlobalList(const QString &ls, const QString &alltext);

	//Annotations管理
public:

	enum AnnotationType
	{
		PROBLEM = 1,
		RUNTIME_PROBLEM = 2,
		MACRO_DEFINE = 3,
	};

private:

	QMultiHash<int, AnnotationType> annotations;

public:

	void clearAnnotations(AnnotationType type);
	void clearAnnotationsAll();
	void annotate(int line, const QString &text, int style, AnnotationType type);
	void annotate(int line, const QString &text, const QsciStyle &style, AnnotationType type);
	void annotate(int line, const QsciStyledText &text, AnnotationType type);
	void annotate(int line, const QList<QsciStyledText> &text, AnnotationType type);

	// 悬浮信息显示
public:
	void ShowToolTip(QPoint pos);
	BkeIndicatorBase GetRangeForStyle(int position, unsigned char style);
};

#endif // BKESCINTILLA_H
