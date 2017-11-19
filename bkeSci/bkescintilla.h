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

	//本文档被CodeWindow隐藏或释放
	void Detach();
	//本文档被CodeWindow显示
	void Attach();

	enum{
		BKE_INDICATOR_FIND = 8,
		BKE_INDICATOR_ERROR,
		BKE_INDICATOR_WARNING,
		BKE_INDICATOR_CLICK_COMMAND,
		BKE_INDICATOR_CLICK_LABEL,
		BKE_INDICATOR_HIGHLIGHT,
	};
	enum{
		BKE_CHANGE_REPLACE = 0x1
	};
	//所有搜索结果的indicator
	QList<BkeIndicatorBase> testlist ;
	//为了显示选择效果而隐藏的indicator
	BkeIndicatorBase findlast;
	QString FileName;

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
	bool HasFind() const { return findcount > 0 ; }
	void SetIndicator(int id, const BkeIndicatorBase &p);
	void SetIndicator(int id, const BkeIndicatorBase &p, int value);
	void BkeAnnotateSelect() ;
	BkeIndicatorBase findIndicator(int id,int postion) ;
	int GetCurrentLine() const;
	int GetCurrentPosition() const;
	void SetCurrentPosition(int pos);
	void setLexer(QsciLexer *lex = 0);
	int GetTextLength() const;
	int ClosedPositionAt(const QPoint &point) const;
	int PositionAt(const QPoint & point) const;
	QPoint PointByPosition(int position) const;
	bool IsIndicator(int id, int pos) const;
	unsigned char GetByte(int pos) const;
	QString TextForRange(const BkeIndicatorBase &range) const;
	QByteArray TextBytesForRange(const BkeIndicatorBase &range) const;
	void RemoveRange(const BkeIndicatorBase &range);
	void AppendText(const QString &text);
	void AppendText(const QByteArray &text);
	QByteArray TextAsBytes(const QString &text) const;
	int PositionByLine(int line) const;
	BkeIndicatorBase GetRangeForStyle(int position, unsigned char style) const;
	BkeIndicatorBase GetRangeForStyles(int position, const QList<unsigned char> &styles, unsigned char *style = nullptr) const;
	//-----------------------Selection----------------------------
	void ClearSelection(int pos = -1);
	void SetSelection(const BkeIndicatorBase &p);
	bool IsSelectionsEmpty() const;

	int findcount ;

	int topLine = 0;

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
	void UiChange(int updated);
	void InsertAndMove(const QString &text);
	void CurrentPosChanged(int line , int index );
	void OnDwellStart(int, int, int);
	void OnDwellEnd(int, int, int);
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
	int ChangeType = 0;
	int WorkingUndoDepth ;
	int LastLine ;
	BkeModifiedBase modfieddata ;
	int findflag ;
	QByteArray fstrdata ;
	QString Separate ;

	bool IsSeparate(int ch) ;
	void BkeStartUndoAction(bool newUndo = false) ;//记录Undo，如果当前正在记录，那么newUndo决定是记录一个新的还是继续当前记录
	void BkeEndUndoAction() ;
	int GetActualIndentCharLength(int lineID) ;

	BkeIndicatorBase simpleFind(const char *ss , int flag,int from,int to) ;

	//补全相关
public:
	void UpdateAutoComplete();

protected:
	QList<QPair<QString, int>> GetLabelList();
	QList<QPair<QString, int>> GetScriptList();
	QList<QPair<QString, int>> GetAttrs(const QString &name, const QStringList &attrs, const QString &alltext);
	QList<QPair<QString, int>> GetEnums(const QString &name, const QString &attr, const QString &alltext);
	QList<QPair<QString, int>> GetValList(const QStringList &ls, const QString &alltext);
	QList<QPair<QString, int>> GetGlobalList(const QString &ls, const QString &alltext);
	void ChooseComplete(const QString &text);
	bool IgnorePosChanged = false;
	virtual void mousePressEvent(QMouseEvent *e) override;
	virtual void focusOutEvent(QFocusEvent *e) override;

	enum {
		SHOW_NULL,
		SHOW_AUTOCOMMANDLIST,
		SHOW_AUTOVALLIST,
		SHOW_ENUMLIST,
		SHOW_LABEL,
		SHOW_ATTR,
		SHOW_SYS
	}autoCompleteType = SHOW_NULL;
	QString autoCompleteContext;

signals:
	void AutoCompleteStart(QList<QPair<QString, int>>, QString);
	void AutoCompleteMatch(QString);
	void AutoCompleteCancel();

public slots:
	void OnAutoCompleteCanceled();
	void OnAutoCompleteSelected(QString s);

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
private:
	void ShowToolTip(int position, QPoint pos);
	void HideToolTip();

	//高亮当前单词
private:
	bool isHighlightShown = false;
	QTimer highlightTimer;
	void CancelHighlight();
	void ShowHighlight();

	//IME相关，给QT修锅
protected:
	virtual void inputMethodEvent(QInputMethodEvent *event) override;
};

#endif // BKESCINTILLA_H
