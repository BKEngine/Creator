#ifndef BKESCINTILLA_H
#define BKESCINTILLA_H

#include <QWidget>
#include <Qsci/qsciscintilla.h>
#include <QMessageBox>
#include <QToolTip>
#include "bkeSci/qscilexerbkescript.h"
#include "paper/creator_parser.h"
#include "bkeSci/BkeIndicatorBase.h"
#include <weh.h>

extern QImage BKE_AUTOIMAGE_KEY ;
extern QImage BKE_AUTOIMAGE_FUNCTION ;
extern QImage BKE_AUTOIMAGE_DICTIONARIES ;
extern QImage BKE_AUTOIMAGE_NORMAL ;
extern QImage BKE_AUTOIMAGE_MATH ;

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
        BKE_INDICATOR_FIND = 1
    };
    enum{
        BKE_CHANGE_REPLACE = 0x1
    };
    QList<BkeIndicatorBase> testlist ;
	QString FileName;

    QString apiContext2(int pos , int &word_start , int &word_end) ;
    QString GetLatstError(){ return errorinfo; }
    void    DefineIndicators(int id,int intype) ;
    void    ClearIndicators(int id) ;
    void ClearIndicator(BkeIndicatorBase &p) ;
    int findFirst1(const QString fstr,bool cs,bool exp,bool word,bool mark = true) ;
    void ReplaceAllFind(const QString &rstr) ;
    BkeIndicatorBase ReplaceFind(const QString &rstr) ;
    int findIndicatorStart(int id,int from) ;
    int findIndicatorEnd(int id,int from) ; //不是以ID开头直接返回-1
    BkeIndicatorBase findIndicatorLast(int id,int from) ;
    bool FindForward(int pos = 0) ;
    bool FindBack(int pos = -1) ;
    bool HasFind(){ return findcount > 0 ; }
    void clearSelection(int pos = -1) ;
    void SetIndicator(int id, BkeIndicatorBase &p) ;
    void BkeAnnotateSelect() ;
    BkeIndicatorBase findIndicator(int id,int postion) ;
    int GetTrueCurrentLine() ;
    void setLexer(QsciLexer *lex = 0);
    void setParser( BkeParser *p){ defparser = p ; }

    int findcount ;
	BkeIndicatorBase findlast;

signals:
    void Undoready(bool is) ;
    void Redoready(bool is) ;

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
    void CompliteFromApi(int type = 0) ;
    void CompliteFromApi2(int lest = 3) ;
    void InsertAndMove(const QString &text) ;
    void CurChange(int line , int index ) ;
    void CharHandle(int cc) ;

public:
    int ChangeStateFlag ;
	bool refind;

private:
    int AutoState ;
    int vautostate ;
    int StartWordPos ;
    int ChangeType ;
    int LastKeywordEnd ;
    int ComPleteLeast ;
    int IndentCount ;
    bool ChangeIgnore ;
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
    QsciLexerBkeScript *deflex ;
    QString Separate ;
    QString LaterInsertWord ;   //Ui更新后插入



    BkeParser *defparser ;
    BkeParser *Selfparser ;

    QFile fileIO ;
    QString errorinfo ;

    bool IsSeparate(int ch) ;
    bool IsIndicator(int id,int pos) ;
    int  GetByte(int pos) ;
    void RemoveDou() ;
    void setSelection( BkeIndicatorBase &p);
    void BkeStartUndoAction() ;
    void BkeEndUndoAction() ;
    void InsertIndent(int count,int lineID) ;
    void ShowInfomation() ;

    BkeIndicatorBase simpleFind(const char *ss , int flag,int from,int to) ;
protected:
    bool event(QEvent *e) ;

};

#endif // BKESCINTILLA_H
