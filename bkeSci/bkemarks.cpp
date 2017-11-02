#include <weh.h>
#include <QRegularExpression>
#include "bkemarks.h"


void BkeMarkSupport::SetFile(const QString &file)
{
	Name = file;
	QString cfile = LOLI_OS_QSTRING(file);
	currentbookmarklist = MarkListOf(cfile, BKE_MARK_BOOKMARK);
	currentmarklist = MarkListOf(cfile, BKE_MARK_MARKER);
	currentproblemlist = MarkListOf(cfile, BKE_MARK_PROBLEM);
	currentruntimeproblemlist = MarkListOf(cfile, BKE_MARK_RUNTIME_PROBLEM);
}

//从编译输出文本中提取错误信息
void BkeMarkSupport::ProblemsFromText(const QString &dir, const QString &text)
{
	ClearMarks(BKE_MARK_PROBLEM);
	QStringList tk = text.split("\n");
	QString temp;

	errorcount = 0;
	for (int i = 0; i < tk.size(); i++) {
		temp = tk.at(i);
		if (temp.endsWith(".bkscr") || temp.endsWith(".bkpsr")) {
			SetFile(dir + temp);
		}
		else if (temp.startsWith("error")) {
			AddProblem(temp);
			errorcount++;
		}
		else if (temp.startsWith("warning")) {
			AddProblem(temp);
		}
	}
}

void BkeMarkSupport::AddProblem(QString text)
{
	BkeMarkerBase *c = new BkeMarkerBase;
	if (text.startsWith("error")) c->Type = 1;
	else c->Type = 2;

	c->FullName = Name;
	int k = Name.lastIndexOf("/");
	if (k < 0) k = Name.lastIndexOf("\\");
	c->Name = Name.right(Name.length() - k - 1);
	c->Information = ReadProblem(c->Atpos, text);
	currentproblemlist->append(c);
}

QString BkeMarkSupport::ReadProblem(int &pos, QString &problem)
{
	pos = 1;
	int k = problem.indexOf("：");
	if (k < 0) return QString();

	QString temp = problem.left(k);
	int a = temp.indexOf("第");
	if (a < 0) return problem.right(problem.length() - k - 1);
	int b = temp.indexOf("行", a);
	if (b < 0) return problem.right(problem.length() - k - 1);

	pos = temp.mid(a + 1, b - a - 1).toInt();
	return problem.right(problem.length() - k - 1);
}

void BkeMarkSupport::AddRuntimeProblem(const QString &dir, int32_t level, const QString & text)
{
	BkeMarkerBase *c = new BkeMarkerBase;
	if (level == 1) 
		c->Type = 1;
	else if (level == 2) 
		c->Type = 2;
	else
		return;
	SetRuntimeProblemFile(dir, text);
	c->FullName = Name;
	int k = Name.lastIndexOf("/");
	if(k < 0) 
		k = Name.lastIndexOf("\\");
	if (k < 0)
		c->Name = Name;
	else
		c->Name = Name.right(Name.length() - k - 1);
	c->Information = ReadRuntimeProblem(c->Atpos, text);
	if(c->Atpos >= 0)
		currentruntimeproblemlist->append(c);
}

bool BkeMarkSupport::SetRuntimeProblemFile(const QString & dir, const QString & problem)
{
	static QRegularExpression fileexp("文件（(.+?)）");
	QRegularExpressionMatch match;
	int k = problem.indexOf("\n");
	if (k < 0)
	{
		SetFile("");
		return false;
	}
	k = problem.indexOf(fileexp, k, &match);
	if (k < 0 || !match.hasMatch())
	{
		SetFile("");
		return false;
	}
	else
	{
		SetFile(dir + match.captured(1));
		return true;
	}
}

QString BkeMarkSupport::ReadRuntimeProblem(int & pos, const QString & problem)
{
	pos = -1;
	
	int a = problem.indexOf("第", 0);
	if (a < 0) return problem;
	int b = problem.indexOf("行", a);
	if (b < 0) return problem;

	pos = problem.mid(a + 1, b - a - 1).toInt();
	QString temp;
	a = problem.lastIndexOf("\n", a);
	if (a >= 0)
		temp = problem.left(a + 1);
	b = problem.indexOf("\n", b);
	if (b >= 0)
		temp += problem.right(problem.length() - b);
	return temp;
}

void BkeMarkSupport::AddBookMark(const QString &info, int pos, const QString &dir)
{
	BkeMarkerBase *abc = new BkeMarkerBase;
	abc->FullName = Name;
	abc->Information = info;
	abc->Atpos = pos + 1;
	abc->Name = dir;

	currentbookmarklist->append(abc);
}

//输出一个使用某outXXXlist的指针
BkeMarkList* BkeMarkSupport::GetFileMarker(const QString &file, int type, bool showall)
{
	BkeMarkHash *hash = HashFromType(type);
	QString cname = LOLI_OS_QSTRING(file);
	BkeMarkList *list = OutListFromType(type);
	list->clear();

	if (!showall) {
		BkeMarkList *ss = hash->value(cname, 0);
		if (ss == 0) return list; //文件没有标记，直接返回清理过后的空白对外输出
								  //拷贝一份内容
		*list = *ss;
		return list;
	}

	//文件排在首位
	if (hash->contains(cname)) list->append(*(hash->value(cname)));

	for (auto ptr = hash->begin(); ptr != hash->end(); ptr++) {
		if (ptr.key() == cname) continue;
		list->append(*(ptr.value()));
	}
	return list;
}


//根据文件及类型，返回list，不存在将创建
BkeMarkList* BkeMarkSupport::MarkListOf(QString &file, int type)
{
	BkeMarkHash *hash = HashFromType(type);
	if (hash->contains(file)) return hash->value(file);
	else {
		BkeMarkList* temp = new BkeMarkList;
		(*hash)[file] = temp;
		return temp;
	}
}

//根据类型获取hash
BkeMarkHash* BkeMarkSupport::HashFromType(int type)
{
	if (type == BKE_MARK_BOOKMARK) return &bookmarkhash;
	else if (type == BKE_MARK_MARKER) return &markhash;
	else if (type == BKE_MARK_RUNTIME_PROBLEM) return &runtimeproblemhash;
	else return &problemhash;
}

//根据类型获取对外输出list
BkeMarkList* BkeMarkSupport::OutListFromType(int type)
{
	if (type == BKE_MARK_BOOKMARK) return &outbookmarkerlist;
	else if (type == BKE_MARK_MARKER) return &outmarklist;
	else if (type == BKE_MARK_RUNTIME_PROBLEM) return &outruntimeproblemlist;
	else return &outproblemlist;
}

BkeMarkList *BkeMarkSupport::GetProblemMark(const QString &file, bool all)
{
	return GetFileMarker(file, BKE_MARK_PROBLEM, all);
}

BkeMarkList *BkeMarkSupport::GetBookMark(const QString &file, bool all)
{
	return GetFileMarker(file, BKE_MARK_BOOKMARK, all);
}

BkeMarkList *BkeMarkSupport::GetMarks(const QString &file, bool all)
{
	return GetFileMarker(file, BKE_MARK_MARKER, all);
}

BkeMarkList * BkeMarkSupport::GetRuntimeProblemMark(const QString & file, bool all)
{
	return GetFileMarker(file, BKE_MARK_RUNTIME_PROBLEM, all);
}

void BkeMarkSupport::ClearMarks(int type)
{
	BkeMarkHash *hash = HashFromType(type);
	BkeMarkList *list;
	for (auto i = hash->begin(); i != hash->end(); i++) {
		list = i.value();
		for (int k = 0; k < list->size(); k++) {
			delete list->at(k);  //删除markbase
		}
		list->clear();  //list不会被删除
	}
}

void BkeMarkSupport::BookMarksFromText(const QString &text, const QString &dir)
{
	QString ea = Name;

	QString ep;
	QStringList lz, ls = text.split("\r\n");
	for (int i = 0; i < ls.size(); i++) {
		if (ls.at(i).startsWith(':')) {
			ep = ls.at(i).right(ls.at(i).length() - 1);
			SetFile(ep);
			ep = ep.right(ep.length() - dir.length() - 1);
		}
		else if (ls.at(i).isEmpty()) continue;
		else {
			lz = ls.at(i).split('/');
			AddBookMark(lz.at(1), lz.at(0).toInt(), ep);
		}
	}
	//重置为原来的文件
	SetFile(ea);
}

void BkeMarkSupport::ClearRuntimeProblems()
{
	ClearMarks(BKE_MARK_RUNTIME_PROBLEM);
}

