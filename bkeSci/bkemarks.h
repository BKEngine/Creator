#ifndef COMPILEPROBLEM_H
#define COMPILEPROBLEM_H
#include "weh.h"
#include <Qsci/qsciscintilla.h>

class BkeMarkerBase
{
public:
	QString FullName; //全路径
	QString Name;  //相对路径
	QString Information;   //对于问题，是信息，对于标签是用户输入标记名称
	int Type;
	int Atpos;//line
	int start;
	int end;
};

typedef QList<BkeMarkerBase*> BkeMarkList;
typedef QHash<QString, BkeMarkList*> BkeMarkHash;


//统管bke记号类
class BkeMarkSupport
{
public:

	enum MARKTYPE {
		BKE_MARK_SEARCH,
		BKE_MARK_PROBLEM = 3,
		BKE_MARK_BOOKMARK = 4,
		BKE_MARK_MARKER = 8,
		BKE_MARK_RUNTIME_PROBLEM = 9,
	};
	int errorcount;

	void SetFile(const QString &file);

	void AddBookMark(const QString &info, int pos, const QString &dir);
	void ProblemsFromText(const QString &dir, const QString &text);
	void BookMarksFromText(const QString &text, const QString &dir);
	void ClearRuntimeProblems();
	void AddRuntimeProblem(const QString &dir, int32_t level, const QString &text);

	BkeMarkList *GetProblemMark(const QString &file = QString(), bool all = false);
	BkeMarkList *GetBookMark(const QString &file = QString(), bool all = false);
	BkeMarkList *GetMarks(const QString &file = QString(), bool all = false);
	BkeMarkList *GetRuntimeProblemMark(const QString &file = QString(), bool all = false);
	BkeMarkList* OutListFromType(int type);

	~BkeMarkSupport()
	{
		//ClearMarks(BKE_MARK_SEARCH);
		ClearMarks(BKE_MARK_PROBLEM);
		ClearMarks(BKE_MARK_BOOKMARK);
		ClearMarks(BKE_MARK_MARKER);
		ClearMarks(BKE_MARK_RUNTIME_PROBLEM);
		for (auto &it : bookmarkhash)
			delete it;
		for (auto &it : problemhash)
			delete it;
		for (auto &it : markhash)
			delete it;
		for (auto &it : runtimeproblemhash)
			delete it;
	}
private:
	QString Name;
	BkeMarkHash bookmarkhash;
	BkeMarkHash problemhash;
	BkeMarkHash markhash;
	BkeMarkHash runtimeproblemhash;
	BkeMarkList* currentproblemlist;
	BkeMarkList* currentbookmarklist;
	BkeMarkList* currentmarklist;
	BkeMarkList* currentruntimeproblemlist;
	//对外输出*bkemarklist时，总是改变outlist，相当于一个缓存
	BkeMarkList outproblemlist;
	BkeMarkList outbookmarkerlist;
	BkeMarkList outmarklist;
	BkeMarkList outruntimeproblemlist;

	BkeMarkList* MarkListOf(QString &file, int type);
	BkeMarkHash* HashFromType(int type);

	void ClearMarks(int type);
	QString ReadProblem(int &pos, QString &problem);
	BkeMarkList* GetFileMarker(const QString &file, int type, bool showall = false);
	void AddProblem(QString text);
	bool SetRuntimeProblemFile(const QString &dir, const QString &problem);
	QString ReadRuntimeProblem(int &pos, const QString &problem);

};

#endif // COMPILEPROBLEM_H
