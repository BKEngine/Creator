#include <weh.h>
#include "bkescintilla.h"
#include "../BKS_info.h"
#include "../codewindow.h"

QImage BKE_AUTOIMAGE_KEY(":/auto/source/auto_key.png");
QImage BKE_AUTOIMAGE_FUNCTION(":/auto/source/auto_funcotin.png");
QImage BKE_AUTOIMAGE_DICTIONARIES(":/auto/source/auto_diction.png");
QImage BKE_AUTOIMAGE_NORMAL(":/auto/source/auto_nomal.png");
QImage BKE_AUTOIMAGE_MATH(":/auto/source/auto_math.png");

BkeScintilla::BkeScintilla(QWidget *parent)
	:QsciScintilla(parent)
	, refind(true)
{
	//setAttribute(Qt::WA_DeleteOnClose, true); //关闭后自动析构
	//setStyleSheet("QsciScintilla{border:0px solid red ;}");
	this->setContextMenuPolicy(Qt::CustomContextMenu); //使用用户右键菜单
	this->setAttribute(Qt::WA_InputMethodEnabled, true);
	findstr_length = 0;

	setUtf8(true);
	setMarginWidth(0, "012345678");
	setMarginWidth(1, 20);
	markerDefine(QImage(":/info/source/errorsmall.png"), 1);
	markerDefine(QImage(":/info/source/wainningsmall.png"), 2);
	markerDefine(QImage(":/info/source/Bookmarksmall.png"), 3);
	markerDefine(QImage(":/info/source/pinsmall.png"), 4);
	markerDefine(QsciScintilla::LeftRectangle, 5);
	SendScintilla(SCI_MARKERSETBACK, 5, QColor(0, 159, 60));

	DefineIndicators(BKE_INDICATOR_FIND, INDIC_STRAIGHTBOX);  //标记搜索的风格指示器
	SendScintilla(SCI_INDICSETFORE, BKE_INDICATOR_FIND, QColor(0, 0xff, 0));
	SendScintilla(SCI_INDICSETALPHA, BKE_INDICATOR_FIND, 255);
	SendScintilla(SCI_INDICSETUNDER, BKE_INDICATOR_FIND, true);

	registerImage(CompleteBase::BKE_TYPE_NORMAL, BKE_AUTOIMAGE_NORMAL);
	registerImage(CompleteBase::BKE_TYPE_FUNCTION, BKE_AUTOIMAGE_FUNCTION);
	registerImage(CompleteBase::BKE_TYPE_DICTIONARIES, BKE_AUTOIMAGE_DICTIONARIES);
	registerImage(9, BKE_AUTOIMAGE_KEY);
	registerImage(8, BKE_AUTOIMAGE_MATH);

	setMarginsForegroundColor(QColor(100, 100, 100));
	setMarginsBackgroundColor(QColor(240, 240, 240));
	//setAutoIndent(true);

	//setIndentationGuides(true) ;
	Separate = QString(" ~!@#$%^&*()-+*/|{}[]:;/=.,?><\\\n\r");
	AutoState = AUTO_NULL;
	ChangeIgnore = false;
	UseCallApi = false;
	LastKeywordEnd = -1;
	LastLine = -1;
	ChangeStateFlag = 0;
	IsNewLine = false;
	IsWorkingUndo = false;

	deflex = new QsciLexerBkeScript;
	setLexer(deflex);
	Selfparser = defparser = new BkeParser;     //新的词法分析器

	connect(this, SIGNAL(SCN_MODIFIED(int, int, const char *, int, int, int, int, int, int, int)),
		SLOT(EditModified(int, int, const char *, int, int, int, int, int, int, int)));
	connect(this, SIGNAL(SCN_UPDATEUI(int)), this, SLOT(UiChange(int)));
	//用户列表被选择
	connect(this, SIGNAL(SCN_USERLISTSELECTION(const char*, int)), this, SLOT(UseListChoose(const char*, int)));
	//自动完成列表被选择
	connect(this, SIGNAL(SCN_AUTOCSELECTION(const char*, int)), this, SLOT(AutoListChoose(const char*, int)));
	//光标位置被改变
	connect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(CurChange(int, int)));
	//输入单个字符
	connect(this, SIGNAL(SCN_CHARADDED(int)), this, SLOT(CharHandle(int)));

}

BkeScintilla::~BkeScintilla()
{
	delete Selfparser;
}


void BkeScintilla::EditModified(int pos, int mtype, const char *text,
	int len, int added, int line, int foldNow, int foldPrev, int token,
	int annotationLinesAdded)
{
	if (ChangeIgnore) return;

	int xline, xindex;
	lineIndexFromPosition(pos, &xline, &xindex);
	if (mtype & SC_MOD_INSERTTEXT)
	{  //文字被插入

		if ((mtype&SC_PERFORMED_USER) > 0 && !IsWorkingUndo) BkeStartUndoAction();
		ChangeType = mtype;
		modfieddata.pos = pos;
		modfieddata.type = mtype;
		modfieddata.line = xline;
		modfieddata.index = xindex;
		modfieddata.lineadd = added;
		modfieddata.text = QString(text);

	}
	else if (mtype & SC_MOD_DELETETEXT)
	{
		//if( xindex < LastKeywordEnd) CheckLine(xline); //一旦删除超过关键点，则需要重新进行解析

		ChangeType = mtype;
		modfieddata.pos = pos;
		modfieddata.type = mtype;
		modfieddata.line = xline;
		modfieddata.index = xindex;
		modfieddata.lineadd = added;
		//modfieddata.text = QString(text);
	}
	else if (mtype & SC_MOD_BEFOREDELETE)
	{
		ChangeType = SC_MOD_DELETETEXT;
		modfieddata.pos = pos;
		modfieddata.type = mtype;
		modfieddata.line = xline;
		modfieddata.index = xindex;
		modfieddata.lineadd = added;
		int l = 0;
		QString q;
		while (l < len)
		{
			char ch = SendScintilla(SCI_GETCHARAT, pos + l);
			q.push_back(ch);
			l++;
		}
		modfieddata.text = q;

		ChangeType = mtype;
	}
}

void BkeScintilla::CharHandle(int cc)
{
}

//文字或样式被改变后
void BkeScintilla::UiChange(int updated)
{
	if (ChangeIgnore) return;

	if (ChangeType & SC_MOD_INSERTTEXT){
		//自动补全
		defparser->TextBeChange(&modfieddata, this);
		CompliteFromApi();
	}

	if (IsWorkingUndo && !ChangeIgnore) BkeEndUndoAction();

	//缩进
	if (modfieddata.lineadd == 1 && (modfieddata.text == "\n" || modfieddata.text == "\r\n"))
	{
		char chPrev = SendScintilla(SCI_GETCHARAT, modfieddata.pos - 1);
		char chNext = SendScintilla(SCI_GETCHARAT, modfieddata.pos + modfieddata.text.count());

		int count = SendScintilla(SCI_GETLINEINDENTATION, modfieddata.line);
		int ly = defparser->GetIndentLayer(this, modfieddata.line);
		//与上一行对其
		if (ly == 0)
			InsertIndent(count, modfieddata.line + 1);
		else if (ly > 0)
			InsertIndent(count + SendScintilla(SCI_GETTABWIDTH), modfieddata.line + 1);
		else
		{
			SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line, count - SendScintilla(SCI_GETTABWIDTH));
			//本行就应该减了
			InsertIndent(count - SendScintilla(SCI_GETTABWIDTH), modfieddata.line/* + 1*/);
		}
	}

	//括号，引号补全
	if (modfieddata.text.length() == 1 && (modfieddata.text == "(" || modfieddata.text == "[" || modfieddata.text == "{" || modfieddata.text == "\"" || modfieddata.text == "'" || modfieddata.text == ")" || modfieddata.text == "]" || modfieddata.text == "}"))
	{
		QString lefts = "([{\"\'";
		QString rights = ")]}\"\'";
		unsigned char style = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos);
		unsigned char style2 = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos + 1);
		//unsigned char style3 = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos + 1);
		char chPrev = SendScintilla(SCI_GETCHARAT, modfieddata.pos - 1);
		char ch = SendScintilla(SCI_GETCHARAT, modfieddata.pos);
		char chNext = SendScintilla(SCI_GETCHARAT, modfieddata.pos + 1);
		if ((style & 63 == SCE_BKE_STRING) || (style & 63 == SCE_BKE_STRING2))
			return;
		if (ChangeType & SC_MOD_INSERTTEXT)
		{
			char match[2];
			bool caret = false;
			switch (ch)
			{
			case '(':
				match[0] = ')';
				break;
			case '[':
				match[0] = ']';
				break;
			case '{':
				match[0] = '}';
				break;
			case '\"':
			case '\'':
				match[0] = ch;
				//判断是leading还是ending的"'
				if (chPrev == ch && ch == chNext)
					caret = true;
				break;
			default:
				if (style == style2 || style == SCE_BKE_COMMAND)
					caret = true;	//光标前进一格，同时忽略本次输入
				match[0] = 0;
				break;
			}
			match[1] = 0;
			if (caret)
			{
				SendScintilla(SCI_GOTOPOS, modfieddata.pos + 2);
				SendScintilla(SCI_DELETERANGE, modfieddata.pos, 1);
			}
			else if (style != style2 || modfieddata.text[0] != chNext)
			{
				SendScintilla(SCI_INSERTTEXT, modfieddata.pos + 1, match);
			}
		}
		else if (ChangeType & SC_MOD_DELETETEXT)
		{
			char del = modfieddata.text[0].toLatin1();
			if (lefts.indexOf(del) >= 0 && lefts.indexOf(del) == rights.indexOf(ch))
			{
				if ((del != '\"' && del != '\'') || (style2 & 63) == SCE_BKE_ERROR)
					SendScintilla(SCI_DELETERANGE, modfieddata.pos, 1);
			}
		}
	}

	defparser->showtype = BkeParser::SHOW_NULL;
	ChangeType = 0;
	modfieddata.clear();
}




//自动补全
void BkeScintilla::CompliteFromApi(int type)
{
	int word_end;
	QString context = apiContext2(SendScintilla(SCI_GETCURRENTPOS), StartWordPos, word_end);
	if (defparser->showtype == BkeParser::SHOW_AUTOVALLIST && context.length() < 3) return;
	else if (defparser->showtype == BkeParser::SHOW_LABEL && context.length() < 2) return;

	SendScintilla(SCI_AUTOCSETIGNORECASE, true);

	comss = defparser->GetList(context);

	if (comss == 0) return;

	if (context.isEmpty()){
		SendScintilla(SCI_USERLISTSHOW, 1, comss);
	}
	else{
		SendScintilla(SCI_AUTOCSHOW, context.length(), comss);
	}
}

void BkeScintilla::CompliteFromApi2(int lest)
{
	int word_end, nowpos = SendScintilla(SCI_GETCURRENTPOS);
	QString context = apiContext2(nowpos, StartWordPos, word_end);
	if (context.length() < lest) return;

	const char *ks = defparser->GetValList(context);
	SendScintilla(SCI_AUTOCSETIGNORECASE, true);

	if (ks == 0) return;
	else if (context.isEmpty()){
		SendScintilla(SCI_USERLISTSHOW, 1, ks);
	}
	else{
		SendScintilla(SCI_AUTOCSHOW, context.length(), ks);
	}
}


//获取上下文
QString BkeScintilla::apiContext2(int pos, int &word_start, int &word_end)
{
	int ch, beginpos = pos;
	QString word;

	while (pos > 0)
	{
		ch = GetByte(--pos);
		if (ch == 0 || IsSeparate(ch))  break; //不应该出现\0
		word.prepend(ch);

	}
	word_start = pos;
	word_end = beginpos;
	return word;
}

//是否分割符
bool BkeScintilla::IsSeparate(int ch)
{
	if (ch <= 0 || ch > 127) return false;
	char a = ch & 255;
	if (Separate.indexOf(a) >= 0) return true;
	return false;
}

int BkeScintilla::GetByte(int pos)
{
	unsigned char s;
	s = SendScintilla(SCI_GETCHARAT, pos);
	return s;
}

//用户列表被选择
void BkeScintilla::UseListChoose(const char* text, int id)
{
	ChangeIgnore = true;
	BkeStartUndoAction();
	//我们需要检测重复字符
	int line, index;
	getCursorPosition(&line, &index);
	int length = lineLength(line);
	char *buf = new char[length + 1];
	SendScintilla(SCI_GETLINE, line, buf);
	buf[index] = 0;	//截断在index
	QString t(text);
	int maxre = min(t.length(), index);
	int i = 1;
	while (i <= maxre)
	{
		QString tmp(buf + index - i);
		if (t.startsWith(tmp))
			break;
		i++;
	}
	if (i<=maxre)
		ChooseComplete(text, positionFromLineIndex(line, index) - i);
	else
		ChooseComplete(text, positionFromLineIndex(line, index));
	ChangeIgnore = false;
}

//自动完成列表被选择
void BkeScintilla::AutoListChoose(const char* text, int pos)
{
	ChangeIgnore = true;
	BkeStartUndoAction();
	SendScintilla(SCI_AUTOCCANCEL);  //取消自动完成，手动填充
	ChooseComplete(text, pos);
	ChangeIgnore = false;
}


void BkeScintilla::ChooseComplete(const char *text, int pos)
{
	int i = SendScintilla(SCI_AUTOCGETCURRENT);
	QString temp(text);
	if (pos < 0)
		pos = SendScintilla(SCI_GETCURRENTPOS);
	bool iscommand = (GetByte(pos - 1) == ';');

	if (!temp.endsWith("\"") && iscommand && !defparser->HasTheChileOf(temp))
		temp.append("=");

	if (iscommand)
	{
		SendScintilla(SCI_SETSELECTIONSTART, pos - 1); //移除逗号
		if (GetByte(pos - 2) != ' ') temp.prepend(" "); //逗号之前不是空格，增加空格
	}
	else 
		SendScintilla(SCI_SETSELECTIONSTART, pos);

	defparser->showtype = BkeParser::SHOW_NULL;
	modfieddata.clear();
	SendScintilla(SCI_SETSELECTIONEND, SendScintilla(SCI_GETCURRENTPOS));
	removeSelectedText();
	InsertAndMove(temp);
}


//插入并移动光标位置
void BkeScintilla::InsertAndMove(const QString &text)
{
	insert(text);
	int line, index;
	getCursorPosition(&line, &index);
	setCursorPosition(line, index + text.length());
}


//光标定位到新行
void BkeScintilla::CurChange(int line, int index)
{
	//改变忽略，行没有改变，文本被选择不会触发语法检查
	if (ChangeIgnore || line == LastLine || hasSelectedText()){
		IsNewLine = false;
		return;
	}

	LastKeywordEnd = 2; //关键位置重置为2
	if (!hasSelectedText()){
		QString temp = this->text(LastLine);
		defparser->ParserText(temp);  //非选择状态，词法分析上一行
		//        //CheckLine( line ); //检查行状态
		//        AutoState = AUTO_NULL ;
		//        LastKeywordEnd = 99 ;
		//        ComPleteLeast = 3 ;
		//         //获得缩进
		//        IndentCount = defparser->GetIndentLayer()*8 ;

	}
	markerDelete(LastLine, 5);
	markerDelete(LastLine + 1, 5);
	LastLine = line;
	markerAdd(line, 5);
	//光标移动是不会触发填充模式的
	vautostate = P_AUTO_NULL;
}


//移除前面的;号
void BkeScintilla::RemoveDou()
{
	unsigned char s = 'a';
	bool m = false;
	int pos = SendScintilla(SCI_GETCURRENTPOS);
	while (!IsSeparate(s) && pos > 0){
		s = GetByte(--pos);
		if (s == ';'){
			m = true;
			break;
		}
	}

	if (!m) return;
	SendScintilla(SCI_SETSELECTIONSTART, pos);
	SendScintilla(SCI_SETSELECTIONEND, pos + 1);
	removeSelectedText();
}

//定义指示器
void BkeScintilla::DefineIndicators(int id, int intype)
{
	SendScintilla(SCI_INDICSETSTYLE, id, intype);
}

int BkeScintilla::findFirst1(const QString fstr, bool cs, bool exp, bool word, bool mark)
{
	findcount = 0;
	fstrdata = fstr.toUtf8();
	const char *ss = fstrdata.constData();
	testlist.clear();
	findstr_length = fstrdata.length();
	findflag = (cs ? SCFIND_MATCHCASE : 0) |
		(word ? SCFIND_WHOLEWORD : 0) |
		(exp ? SCFIND_REGEXP : 0);

	SendScintilla(SCI_SETINDICATORCURRENT, BKE_INDICATOR_FIND);
	ClearIndicators(BKE_INDICATOR_FIND);

	BkeIndicatorBase abc;
	int a, len, b;
	len = this->length();
	refind = false;
	findlast.Clear();
	for (a = 0; a < len;){
		abc = simpleFind(ss, findflag, a, len);
		if (abc.IsNull())
			return findcount;
		//对结果进行标记
		testlist.append(abc);
		if (mark)
			SetIndicator(BKE_INDICATOR_FIND, abc);
		findcount++;
		a = abc.End();
	}
	return findcount;
}


void BkeScintilla::ClearIndicators(int id)
{
	int xl, xi;
	lineIndexFromPosition(this->length(), &xl, &xi);
	clearIndicatorRange(0, 0, xl, xi, id);
	if (id == BKE_INDICATOR_FIND){
		findcount = 0;
		findlast.Clear();
	}
}

//
bool BkeScintilla::FindForward(int pos)
{
	//SetIndicator(BKE_INDICATOR_FIND, findlast);
	//clearSelection();
	if (findcount < 1){
		QMessageBox::information(this, "查找", "没有找到任何匹配的文本！", QMessageBox::Ok);
		return false;
	}

	BkeIndicatorBase abc;
	//if( !findlast.IsNull() ){ //上一个节点有效，则再寻找一次
	//    abc = simpleFind(fstrdata.constData(),findflag,findlast.Start(),findlast.End()) ;
	//    SetIndicator(BKE_INDICATOR_FIND,abc);
	//}
	abc = findIndicator(BKE_INDICATOR_FIND, pos);
	abc = simpleFind(fstrdata.constData(), findflag, abc.Start(), abc.End());
	if (abc.IsNull()) return false;
	setSelection(abc);
	findlast = abc;
	//abc.SetEnd(abc.Start() + findstr_length);
	ClearIndicator(abc);
	return true;
}

bool BkeScintilla::FindBack(int pos)
{
	if (pos <= 0) pos = this->length() - 1;
	if (hasSelectedText()){ //光标总是在被选择文字的后面，搜索从选择文字之前
		pos = SendScintilla(SCI_GETSELECTIONSTART) - 1;
	}
	//SetIndicator(BKE_INDICATOR_FIND, findlast);
	//clearSelection();   //清理光标必须放在这里
	if (findcount < 1){
		QMessageBox::information(this, "查找", "没有找到任何匹配的文本！", QMessageBox::Ok);
		return false;
	}

	if (!findlast.IsNull()){
		SetIndicator(BKE_INDICATOR_FIND, findlast);
		clearSelection();
	}
	BkeIndicatorBase abc2 = findIndicatorLast(BKE_INDICATOR_FIND, pos);
	if (abc2.IsNull() || abc2.End() == 0)
	{
		QMessageBox::information(this, "查找", "再往前没有了！", QMessageBox::Ok);
		return false;
	}
	BkeIndicatorBase abc;
	abc.SetEnd(abc2.Start());
	do
	{
		abc = simpleFind(fstrdata.constData(), findflag, abc.End(), abc2.End());
	} while (abc.End() < abc2.End());
	ClearIndicator(abc);
	setSelection(abc);
	findlast = abc;
	return true;
}


void BkeScintilla::ReplaceAllFind(const QString &rstr)
{
	BkeIndicatorBase abc(0, 0);
	while (FindForward(abc.End())){
		abc = ReplaceFind(rstr);
		//if( abc.IsNull() ) return ;
	}
}

bool BkeScintilla::ReplaceText(const QString &rstr, const QString &dstr, bool cs, bool exp, bool word)
{
	int flag = (cs ? SCFIND_MATCHCASE : 0) |
		(word ? SCFIND_WHOLEWORD : 0) |
		(exp ? SCFIND_REGEXP : 0);

	int from = SendScintilla(SCI_GETCURRENTPOS);
	int to = this->length();

	QByteArray rdata = rstr.toUtf8();
	QByteArray ddata = dstr.toUtf8();
	const char *ss = rdata.constData();
	const char *dd = ddata.constData();

	int srclen = strlen(ss);
	int dstlen = strlen(dd);

	ChangeStateFlag |= BKE_CHANGE_REPLACE;

	//if (from < to)
	//do
	{
		SendScintilla(SCI_SETSEARCHFLAGS, flag);
		SendScintilla(SCI_SETTARGETSTART, from);
		SendScintilla(SCI_SETTARGETEND, to);

		if (SendScintilla(SCI_SEARCHINTARGET, strlen(ss), ss) < 0)
		{
			return false;
		}

		if (exp)
		{
			SendScintilla(SCI_REPLACETARGETRE, dstlen, dd);
		}
		else
		{
			SendScintilla(SCI_REPLACETARGET, dstlen, dd);
		}
		ChangeStateFlag &= (~BKE_CHANGE_REPLACE);

		//from = SendScintilla(SCI_GETTARGETEND) + 1;
		SendScintilla(SCI_SETANCHOR, SendScintilla(SCI_GETTARGETEND) + 1);
		SendScintilla(SCI_SETCURRENTPOS, SendScintilla(SCI_GETTARGETEND) + 1);
	}while(0);

	return true;
}

void BkeScintilla::ReplaceAllText(const QString &rstr, const QString &dstr, bool cs, bool exp, bool word)
{
	int flag = (cs ? SCFIND_MATCHCASE : 0) |
		(word ? SCFIND_WHOLEWORD : 0) |
		(exp ? SCFIND_REGEXP : 0);

	int from = 0;
	int to = this->length();

	QByteArray rdata = rstr.toUtf8();
	QByteArray ddata = dstr.toUtf8();
	const char *ss = rdata.constData();
	const char *dd = ddata.constData();

	int srclen = strlen(ss);
	int dstlen = strlen(dd);

	ChangeStateFlag |= BKE_CHANGE_REPLACE;

	while (from < to)
	{
		SendScintilla(SCI_SETSEARCHFLAGS, flag);
		SendScintilla(SCI_SETTARGETSTART, from);
		SendScintilla(SCI_SETTARGETEND, to);

		if (SendScintilla(SCI_SEARCHINTARGET, strlen(ss), ss) < 0)
		{
			break;
		}

		if (exp)
		{
			SendScintilla(SCI_REPLACETARGETRE, dstlen, dd);
		}
		else
		{
			SendScintilla(SCI_REPLACETARGET, dstlen, dd);
		}
		ChangeStateFlag &= (~BKE_CHANGE_REPLACE);

		from = SendScintilla(SCI_GETTARGETEND) + 1;
	}
}

BkeIndicatorBase BkeScintilla::ReplaceFind(const QString &rstr)
{
	BkeIndicatorBase ntr;
	if (!hasSelectedText()) return ntr;

	QByteArray ak = rstr.toUtf8();
	const char *ss = ak.constData();
	//int from = SendScintilla(SCI_GETSELECTIONSTART) ;
	//SendScintilla(SCI_SETTARGETSTART,from) ;
	//SendScintilla(SCI_SETTARGETEND,SendScintilla(SCI_GETSELECTIONEND)) ;

	////标记修改来自替换，将忽略某些改变
	ChangeStateFlag |= BKE_CHANGE_REPLACE;
	SendScintilla(SCI_REPLACESEL, (ulong)0, ss);
	ChangeStateFlag &= (~BKE_CHANGE_REPLACE);

	//ntr.SetStart(from);
	ntr.SetEnd(SendScintilla(SCI_GETSELECTIONEND));
	return ntr;
	//不在已经替换过的内容上再次查找
	//BkeIndicatorBase abc = simpleFind(fstrdata.constData(),SendScintilla(SCI_GETSEARCHFLAGS),from,from+len) ;
	//if( !abc.IsNull() ) SetIndicator(BKE_INDICATOR_FIND,abc);
	//else findcount-- ;
}

int BkeScintilla::findIndicatorStart(int id, int from)
{
	int k = from;
	while (from < this->length()){
		if (SendScintilla(SCI_INDICATORVALUEAT, id, from) > 0) return from;
		from = SendScintilla(SCI_INDICATOREND, id, from);
		//往回走既是到了末尾
		if (from < k) return -1;
	}
	return -1;
}

int BkeScintilla::findIndicatorEnd(int id, int from)
{
	if (SendScintilla(SCI_INDICATORVALUEAT, id, from) <= 0) return -1;
	return SendScintilla(SCI_INDICATOREND, id, from);
}

//简单搜索
BkeIndicatorBase BkeScintilla::simpleFind(const char *ss, int flag, int from, int to)
{
	SendScintilla(SCI_SETSEARCHFLAGS, flag);
	SendScintilla(SCI_SETTARGETSTART, from);
	SendScintilla(SCI_SETTARGETEND, to);

	BkeIndicatorBase abc;
	if (from >= to || from < 0)
		return abc;

	if (SendScintilla(SCI_SEARCHINTARGET, strlen(ss), ss) < 0)
		return abc;
	abc.SetStart(SendScintilla(SCI_GETTARGETSTART));
	abc.SetEnd(SendScintilla(SCI_GETTARGETEND));
	return abc;
}

void BkeScintilla::SetIndicator(int id, BkeIndicatorBase &p)
{
	if (p.IsNull()) return;
	SendScintilla(SCI_SETINDICATORCURRENT, id);
	SendScintilla(SCI_INDICATORFILLRANGE, p.Start(), p.Len());
}

BkeIndicatorBase BkeScintilla::findIndicator(int id, int postion)
{
	BkeIndicatorBase abc;
	abc.SetStart(findIndicatorStart(id, postion));
	if (abc.Start() < 0) return abc;
	abc.SetEnd(findIndicatorEnd(id, abc.Start()));
	return abc;
}

void BkeScintilla::setSelection(BkeIndicatorBase &p)
{
	if (p.IsNull()) return;
	int fl, fi, el, ei;
	lineIndexFromPosition(p.Start(), &fl, &fi);
	lineIndexFromPosition(p.End(), &el, &ei);
	SendScintilla(SCI_SETCURRENTPOS, p.End());
	QsciScintilla::setSelection(fl, fi, el, ei);
	//    SendScintilla(SCI_SETSELECTIONSTART,p.Start()) ;
	//    SendScintilla(SCI_SETSELECTIONEND,p.End()) ;
}

BkeIndicatorBase BkeScintilla::findIndicatorLast(int id, int from)
{
	BkeIndicatorBase abc;
	for (; from >= 0 && !IsIndicator(id, from); from--);
	abc.SetEnd(from + 1);
	if (abc.End() < 0) return abc;
	for (; from >= 0 && IsIndicator(id, from); from--);
	abc.SetStart(from + 1);
	return abc;
}

bool BkeScintilla::IsIndicator(int id, int pos)
{
	return  SendScintilla(SCI_INDICATORVALUEAT, id, pos) > 0;
}

void BkeScintilla::ClearIndicator(BkeIndicatorBase &p)
{
	if (p.IsNull()) return;
	SendScintilla(SCI_INDICATORCLEARRANGE, p.Start(), p.Len());
}

void BkeScintilla::clearSelection(int pos)
{
	if (pos < 0 && pos >= this->length()) pos = SendScintilla(SCI_GETCURRENTPOS);
	SendScintilla(SCI_SETEMPTYSELECTION, pos);
}

//从区域中注释，反注释
void BkeScintilla::BkeAnnotateSelect()
{
	int from, to;
	if (!hasSelectedText()){
		int xl, xi;
		lineIndexFromPosition(SendScintilla(SCI_GETCURRENTPOS), &xl, &xi);
		from = to = xl;
	}
	else{
		int xl, xi;
		lineIndexFromPosition(SendScintilla(SCI_GETSELECTIONSTART), &xl, &xi);
		from = xl;
		lineIndexFromPosition(SendScintilla(SCI_GETSELECTIONEND), &xl, &xi);
		to = xl;
	}

	//确定是进行注释还是反注释
	bool isnotex = false;
	for (int i = from; i <= to; i++){
		if (!this->text(i).startsWith("//")){  //出现了一行不以//开头，进行注释
			isnotex = true;
			break;
		}
	}

	for (int i = from; i <= to; i++){
		if (isnotex){
			int k = positionFromLineIndex(i, 0);
			SendScintilla(SCI_SETCURRENTPOS, k);
			insert("//");
		}
		else{
			int k = positionFromLineIndex(i, 0);
			BkeIndicatorBase abc(k, k + 2);
			setSelection(abc);
			replaceSelectedText("");
		}
	}
}


void BkeScintilla::BkeStartUndoAction()
{
	IsWorkingUndo = true;
	QsciScintilla::beginUndoAction();
}

void BkeScintilla::BkeEndUndoAction()
{
	IsWorkingUndo = false;
	QsciScintilla::endUndoAction();
	emit Undoready(isUndoAvailable());
}

//void BkeScintilla::undo()
//{
//    QsciScintilla::undo() ;
//    if( !isUndoAvailable() ) emit Undoready(false);
//    else emit Redoready(isRedoAvailable());
//}

//void BkeScintilla::redo()
//{
//    QsciScintilla::redo() ;
//    if( !isRedoAvailable() ) emit Redoready(false);
//    //else emit ;
//}

int BkeScintilla::GetTrueCurrentLine()
{
	int xl, xi;
	lineIndexFromPosition(SendScintilla(SCI_GETCURRENTPOS), &xl, &xi);
	return xl;
}

void BkeScintilla::setLexer(QsciLexer *lex)
{
	QsciScintilla::setLexer(lex);
	//读取用户设置
//	QFont f;
//	for (int i = 0; i < 256; i++){
//		f = lex->defaultFont(i);
//        SendScintilla(SCI_STYLESETFONT, i, f.family().toUtf8().constData());
//		SendScintilla(SCI_STYLESETSIZE, i, f.pointSize());
//		SendScintilla(SCI_STYLESETITALIC, i, f.italic());
//		SendScintilla(SCI_STYLESETBOLD, i, f.bold());
//		SendScintilla(SCI_STYLESETUNDERLINE, i, f.underline());
//
////		SendScintilla(SCI_STYLESETFORE, i, lex->color(i));
////		SendScintilla(SCI_STYLESETBACK, i, lex->paper(i));
//	}

	QFont ft("Courier");
	ft.setPointSize(lex->defaultFont(0).pointSize());
	setMarginsFont(ft);

	// 折叠标签样式
	SendScintilla(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_CIRCLEPLUS);
	SendScintilla(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_CIRCLEMINUS);
	SendScintilla(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_CIRCLEPLUSCONNECTED);
	SendScintilla(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_CIRCLEMINUSCONNECTED);
	SendScintilla(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
	SendScintilla(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
	SendScintilla(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);

	// 折叠标签颜色
	SendScintilla(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, 0xa0a0a0);
	SendScintilla(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, 0xa0a0a0);
	SendScintilla(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, 0xa0a0a0);

	SendScintilla(SCI_SETFOLDFLAGS, 16 | 4); //如果折叠就在折叠行的上下各画一条横线

	SendScintilla(SCI_PRIVATELEXERCALL, 0, &global_bke_info);
	//SendScintilla(SCI_SETMARGINSENSITIVEN, SC_MARKNUM_FOLDER, true);
}

void BkeScintilla::InsertIndent(int count, int lineID)
{
	if (count <= 0) return;
	int wi = SendScintilla(SCI_GETTABWIDTH);
	int i = count / wi;
	QString emp;
	if (i > 0) emp.fill(QChar('\t'), i);
	if (count - i*wi > 0){
		emp += QString(count - i*wi, QChar(0x20));
	}

	bool back = ChangeIgnore;
	ChangeIgnore = true;
	SendScintilla(SCI_SETCURRENTPOS, positionFromLineIndex(lineID, 0));
	InsertAndMove(emp);
	ChangeIgnore = back;

}


bool BkeScintilla::event(QEvent *e)
{
	if (e->type() == QEvent::ToolTip){
		ShowInfomation();
		return true;
	}
	return QsciScintilla::event(e);
}


//显示鼠标悬浮位置的信息
void BkeScintilla::ShowInfomation()
{
	QPoint pt1 = QCursor::pos();
	QPoint pt = mapFromGlobal(pt1);

	//获取鼠标所在位置的位置（相当于文档）
	//long close_pos = SendScintilla(SCI_POSITIONFROMPOINTCLOSE, pt.x(),pt.y() );
	//int xl,xi ;
	//lineIndexFromPosition(close_pos,&xl,&xi);
	int line = lineAt(pt);
	if (line == -1)
	{
		QToolTip::hideText();
		return;
	}


	QString t = defparser->GetInfo(this->text(line).trimmed());;
	if (t.isEmpty())
	{
		QToolTip::hideText();
		return;
	}
	QToolTip::showText(pt1, t);
}
