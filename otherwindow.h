#ifndef OTHERWINDOW_H
#define OTHERWINDOW_H
#include "weh.h"
#include "topbarwindow.h"
#include "bkeSci/bkemarks.h"

//打开文件视窗

class QToolBar;
class OtherWindow : public TopBarWindow
{
    Q_OBJECT
public:
    enum{
        WIN_PROBLEM ,
        WIN_SEARCH ,
        WIN_COMPILE ,
        WIN_BOOKMARK ,
        WIN_MARK
	};

	OtherWindow(QWidget *parent = 0);
	~OtherWindow();

private:
    QPushButton *btnproblem ;
    QPushButton *btnsearch ;
    QPushButton *btncompiletext ;
    QPushButton *btnbookmark ;
    QPushButton *btnmark ;
	QPushButton *btnruntimeproblem;
	QLineEdit *lewords;

    QStackedWidget *stackw ;
    QListWidget *problemList ;
    QListWidget *searchlist ;
    QsciScintilla *compileedit ;
    QListWidget *bookmarklist ;
	QListWidget *marklist;
	QListWidget *runtimeProblemList;

    int ErrorCount(){ return errorcount ; }
    void checkShow(QPushButton *btn,bool must = false) ;

public:
	void feedDownBar(QToolBar *);
	void setTextCount(int count);
	void ClearSearch();
	void AddSearchItem(const QString &label);
	void SetProblemList(BkeMarkList *list, const QString &dir);
	void SetCompileResult(const QString &label);
	void RefreshBookMark(BkeMarkList *b);
	void SetRuntimeProblemList(BkeMarkList *list, const QString &dir);


signals:
    void Location(BkeMarkerBase *mark,const QString &prodir) ;

private slots:
	void SearchDoubleClicked(QListWidgetItem * item);
	void ProblemDoubleClicked(QListWidgetItem * item);
	void BookMarkDoubleClicked(QListWidgetItem * item);
	void MarkDoubleClicked(QListWidgetItem * item);
	void RuntimeProblemDoubleClicked(QListWidgetItem * item);

public slots:
    void switchToProblemWidget(bool force = false) ;
    void switchToSearchWidget(bool force = false) ;
    void switchToCompileWidget(bool force = false) ;
    void switchToBookmarkWidget(bool force = false) ;
	void switchToMarkWidget(bool force = false);
	void switchToRuntimeProblemWidget(bool force = false);
	void onSearchOne(const QString &file, const QString &fullfile, int line, int start, int end);

private:
    QPushButton *lastbtn ;
    int errorcount ;
	int runtimeerrorcount;

    QIcon *icoerror ;
	QIcon *icowarning;
	QIcon *iconlog;
    QIcon *icobookmark ;

    QColor bc1 ;
    QColor bc2 ;

    BkeMarkList membookmark ;
    BkeMarkList memmark ;
	BkeMarkList memproblem;
	BkeMarkList memruntimeproblem;
	BkeMarkList memsearch;
	QString problemdir;
	QString runtimeproblemdir;

    QHash<QPushButton*,QWidget*> emap ;
	void AddItem(BkeMarkerBase *marker, QIcon *ico, QListWidget* w);

};

#endif // OTHERWINDOW_H
