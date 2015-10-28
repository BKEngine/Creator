#include <weh.h>
#include "bkescintilla.h"
#include "../BKS_info.h"
#include "../codewindow.h"
#include "../cmdlist_wrapper.h"

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
	analysis = NULL;
	completeType = SHOW_NULL;

	setUtf8(true);
	setMarginWidth(0, "012345678");
	setMarginWidth(1, 20);
	markerDefine(QImage(":/info/source/errorsmall.png"), 1);
	markerDefine(QImage(":/info/source/wainningsmall.png"), 2);
	markerDefine(QImage(":/info/source/Bookmarksmall.png"), 3);
	markerDefine(QImage(":/info/source/pinsmall.png"), 4);
	markerDefine(QsciScintilla::LeftRectangle, 5);
	SendScintilla(SCI_MARKERSETBACK, 5, QColor(0, 159, 60));

	//error
	SendScintilla(SCI_INDICSETSTYLE, BKE_INDICATOR_ERROR, INDIC_SQUIGGLEPIXMAP);
	SendScintilla(SCI_INDICSETFORE, BKE_INDICATOR_ERROR, 0x0000FF);//BGR
	SendScintilla(SCI_INDICSETOUTLINEALPHA, BKE_INDICATOR_ERROR, 255);
	//warning
	SendScintilla(SCI_INDICSETSTYLE, BKE_INDICATOR_WARNING, INDIC_SQUIGGLEPIXMAP);
	SendScintilla(SCI_INDICSETFORE, BKE_INDICATOR_WARNING, 0xFF0000);//BGR
	SendScintilla(SCI_INDICSETOUTLINEALPHA, BKE_INDICATOR_WARNING, 255);


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
	
	SendScintilla(SCI_AUTOCSTOPS, "", ",./!@#$%^&*()_+-=\\;'\"[]{}");

	//setIndentationGuides(true) ;
	Separate = QString(" ~!@#$%^&*()-+*/|{}[]:;/=.,?><\\\n\r");
	AutoState = AUTO_NULL;
	ChangeIgnore = 0;
	UseCallApi = false;
	LastKeywordEnd = -1;
	LastLine = -1;
	ChangeStateFlag = 0;
	IsNewLine = false;
	IsWorkingUndo = false;
	isMacroFile = false;

	deflex = new QsciLexerBkeScript;
	//pdata = new ParseData(this);
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

	tm.setTimerType(Qt::TimerType::CoarseTimer);
	connect(&tm, SIGNAL(timeout()), this, SLOT(onTimer()));
	tm.start(500);
}

BkeScintilla::~BkeScintilla()
{
	tm.stop();
	delete Selfparser;
}

void BkeScintilla::onTimer()
{
	if (!analysis)
		return;
	auto p = analysis->lockFile(FileName);
	if (!p || !p->refresh)
	{
		analysis->unlockFile();
		return;
	}
	p->refresh = false;
	set<QString> l;
	p->getLabels(l);
	emit refreshLabel(l);
	//clear indicator
	int len = length();
	SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 2);
	SendScintilla(BkeScintilla::SCI_INDICATORCLEARRANGE, 0, len);
	SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 3);
	SendScintilla(BkeScintilla::SCI_INDICATORCLEARRANGE, 0, len);
	for (auto &it : p->infos)
	{
		SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, it.type);
		SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, it.value);
		SendScintilla(BkeScintilla::SCI_INDICATORFILLRANGE, it.from, it.len);
	}
	p->infos2_mutex.lock();
	for (auto &it : p->infos2)
	{
		SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, it.type);
		SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, it.value);
		SendScintilla(BkeScintilla::SCI_INDICATORFILLRANGE, it.from, it.len);
	}
	p->infos2_mutex.unlock();
	analysis->unlockFile();
}

void BkeScintilla::EditModified(int pos, int mtype, const char *text,
	int len, int added, int line, int foldNow, int foldPrev, int token,
	int annotationLinesAdded)
{
	if (ChangeIgnore) return;

	int xline, xindex;
	lineIndexFromPosition(pos, &xline, &xindex);
	Pos st, off;
	st.line = xline;
	st.pos = xindex;
	lineIndexFromPosition(pos + len, &off.line, &off.pos);
	off -= st;
	if (ChangeType & SC_PERFORMED_USER)
	{
		BkeStartUndoAction();
	}
	if (mtype & SC_MOD_INSERTTEXT)
	{  //文字被插入
		ChangeType = mtype;
		modfieddata.pos = pos;
		modfieddata.type = mtype;
		modfieddata.line = xline;
		modfieddata.index = xindex;
		modfieddata.lineadd = added;
		modfieddata.text = QString(text);


		if (!FileName.isEmpty())
		{
			int buflen = length() + 1;
			char *buf = new char[buflen];
			SendScintilla(SCI_GETTEXT, buflen, buf);
			analysis->pushFile(FileName, buf);
			delete[] buf;
		}
		//pdata->insertChars(st, off);
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

		if (!FileName.isEmpty())
		{
			int buflen = length() + 1;
			char *buf = new char[buflen];
			SendScintilla(SCI_GETTEXT, buflen, buf);
			analysis->pushFile(FileName, buf);
			delete[] buf;
		}
		//pdata->deleteChars(st, off);
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
		char *buf = new char[len + 1];
		SendScintilla(SCI_GETTEXTRANGE, pos, pos + len, buf);
		modfieddata.text = buf;

		ChangeType = mtype;
	}
}

void BkeScintilla::CharHandle(int cc)
{
}

QString BkeScintilla::getAttrs(const QString &name, const QString &alltext)
{
	std::set<QString> params;
	QString res;
	{
		auto it = CmdList.find(name);
		if (it != CmdList.end())
		{
			if (it->argNames.empty())
				return res;
			for (auto &it2 : it->argNames)
				params.insert(it2);
			auto it2 = params.begin();
			res += *it2;
			for (it2++; it2 != params.end(); it2++)
			{
				res += ' ';
				res += *it2;
			}
			return res;
		}
	}
	//test specialcmd
	{
		auto it = SpecialCmdList.find(name);
		if (it != SpecialCmdList.end())
		{
			QRegExp reg("mode=\"(\\w+)\"");
			QRegExp reg2("mode=(\\d+)");
			auto modeidx = alltext.indexOf(reg);
			auto modeidx2 = alltext.indexOf(reg2);
			if (modeidx >= 0 || modeidx2 >= 0)
			{
				bool f = modeidx < 0;
				BKECmdInfo *info = NULL;
				QString modename = reg.cap(1);
				int idx = -1;
				if (f)
					idx = reg2.cap(1).toInt(&f, 0);
				if (f)
				{
					for (auto &it3 : it->modes)
					{
						if (it3.second.first == idx)
						{
							info = &it3.second.second;
							modename = it3.first;
						}
					}
				}
				else
				{
					auto it3 = it->modes.find(modename);
					if (it3 != it->modes.end())
						info = &it3->second.second;
				}

				if (info)
				{
					for (auto &it2 : info->argNames)
						params.insert(it2);
					auto it2 = params.begin();
					res += *it2;
					for (it2++; it2 != params.end(); it2++)
					{
						res += ' ';
						res += *it2;
					}
					return res;
				}
			}
			else
				return "mode";
			return res;
		}
	}
	//test macro
	{
		BKEMacros macro;
		bool r = analysis->findMacro(name, &macro);
		if (r)
		{
			for (auto &it2 : macro.paramqueue)
				params.insert(it2.first);
			auto it2 = params.begin();
			res += *it2;
			for (it2++; it2 != params.end(); it2++)
			{
				res += ' ';
				res += *it2;
			}
			return res;
		}
	}
	return res;
}

QString BkeScintilla::getEnums(const QString &name, const QString &attr, const QString &alltext)
{
	std::set<QString> params;
	QString res;
	{
		auto it = CmdList.find(name);
		if (it != CmdList.end())
		{
			auto it2 = it->argNames.indexOf(attr);
			if (it2 >= 0)
			{
				if (!it->argAutoList[it2].isEmpty())
					return it->argAutoList[it2];
				else if (it->argFlags[it2] & ptBool)
					return "false true";
				else if (it->argFlags[it2] & ptScript)
				{
					auto ls = workpro->AllScriptFiles();
					ls.pop_front();
					for (auto &ff : ls)
						ff = '\"' + ff + '\"';
					return ls.join(' ');
				}
			}
			return res;
		}
	}
	{
		auto it = SpecialCmdList.find(name);
		if (it != SpecialCmdList.end())
		{
			if (attr == "mode")
			{
				for (auto &it3 : it->modes)
				{
					params.insert(it3.first);
				}
				for (auto &it3 : params)
				{
					res += " \"" + it3 + "\"";
				}
				res.remove(0, 1);
				return res;
			}
			QRegExp reg("\smode=([^ ]*)");
			auto modeidx = alltext.indexOf(reg);
			if (modeidx >= 0)
			{
				QString modename = reg.cap(1);
				PAModule pa(modename);
				bool f;
				BKECmdInfo *info = NULL;
				int idx = pa.getIntValue(&f);
				if (f)
				{
					for (auto &it3 : it->modes)
					{
						if (it3.second.first == idx)
						{
							info = &it3.second.second;
							modename = it3.first;
						}
					}
				}
				else
				{
					modename = pa.getStringValue(&f);
					if (f)
					{
						auto it3 = it->modes.find(modename);
						if (it3 != it->modes.end())
							info = &it3->second.second;
					}
				}

				if (info)
				{
					auto it2 = info->argNames.indexOf(attr);
					if (it2 >= 0)
					{
						if (!info->argAutoList[it2].isEmpty())
							return info->argAutoList[it2];
						else if (info->argFlags[it2] & ptBool)
							return "false true";
						else if (info->argFlags[it2] & ptScript)
						{
							auto ls = workpro->AllScriptFiles();
							ls.pop_front();
							for (auto &ff : ls)
								ff = '\"' + ff + '\"';
							return ls.join(' ');
						}
					}
					return res;
				}
			}
			return res;
		}
	}
}

QString BkeScintilla::getValList(const QStringList &ls, const QString &alltext)
{
	QString res;
	auto p = analysis->lockFile(FileName);
	BKE_Variable v;
	auto idx = 0;
	while (idx != ls.size() - 1)
	{
		if (!idx)
		{
			v = p->fileclo->getMember(ls[idx].toStdWString());
		}
		else if (v.getType() == VAR_DIC || v.getType() == VAR_CLASS)
		{
			v = v.dot(ls[idx].toStdWString());
		}
		else
		{
			analysis->unlockFile();
			return res;
		}
		idx++;
	}
	std::set<QString> params;
	switch (v.getType())
	{
	case VAR_DIC:
		for (auto &it : ((BKE_VarDic*)v.obj)->varmap)
		{
			params.insert(QString::fromStdWString(it.first));
		}
		analysis->unlockFile();
		for (auto &it2 : params)
		{
			res += ' ';
			res += it2;
		}
		res.remove(0, 1);
		return res;
	case VAR_CLASS:
		getAllMembers((BKE_VarClass*)v.obj, params);
		analysis->unlockFile();
		for (auto &it2 : params)
		{
			res += ' ';
			res += it2;
		}
		res.remove(0, 1);
		return res;
	default:
		{
			BKE_Variable &vv = BKE_VarClosure::global()->getMember(v.getTypeBKEString());
			if (vv.getType() != VAR_CLASS)
			{
				analysis->unlockFile();
				return res;
			}
			getAllMembers((BKE_VarClass*)vv.obj, params);
			analysis->unlockFile();
			for (auto &it2 : params)
			{
				res += ' ';
				res += it2;
			}
			res.remove(0, 1);
			return res;
		}
	}
}

void BkeScintilla::showComplete()
{
	int pos = modfieddata.pos + modfieddata.text.length() - 1;
	unsigned char style = SendScintilla(SCI_GETSTYLEAT, pos);
	unsigned char ch = SendScintilla(SCI_GETCHARAT, pos);
	unsigned char curstyle = style;
	QString context;
	QString cmdname;
	QString attrContext;
	QString lastContext;
	int beginPos = pos;
	unsigned int lastStyle = 0;
	completeType = SHOW_NULL;

	unsigned char st = style & 63;

	if (st == SCE_BKE_STRING || st == SCE_BKE_STRING2 || st == SCE_BKE_TRANS || st == SCE_BKE_ANNOTATE || st == SCE_BKE_COMMENT || st == SCE_BKE_LABEL || st == SCE_BKE_TEXT)
		return;

	bool l = SendScintilla(SCI_AUTOCACTIVE);
	if (l != 0)
		return;

	if (style & 64 /*BEGAL_MASK*/)
	{
		beginPos--;
		style = SendScintilla(SCI_GETSTYLEAT, beginPos);
		while (beginPos > 0 && (style & 128))
		{
			beginPos--;
			style = SendScintilla(SCI_GETSTYLEAT, beginPos);
		}
		//now we find the beginning the @ or [ command
		char *buf = new char[pos + 2 - beginPos];
		SendScintilla(SCI_GETTEXTRANGE, beginPos, pos + 1, buf);
		context = buf;
		delete[] buf;
		while (isalnum(ch) || ch == '.' || ch == '_')
		{
			attrContext.push_front(ch);
			ch = SendScintilla(SCI_GETCHARAT, --pos);
		}
		while (!attrContext.isEmpty() && (attrContext[0] >= '0' && attrContext[0] <= '9'))
			attrContext.remove(0, 1);
		if (attrContext.isEmpty())
			return;
		QStringList ls = attrContext.split('.');
		if (ls.size() == 1 && ls[0].length() <= 2)
			return;
		lastContext = ls.back();
		completeList = getValList(ls, context);
		completeType = SHOW_AUTOVALLIST;
	}
	else if (style & 128 /*CMD_MASK*/)
	{
		beginPos--;
		style = SendScintilla(SCI_GETSTYLEAT, beginPos);
		while (beginPos > 0 && (style & 128))
		{
			beginPos--;
			style = SendScintilla(SCI_GETSTYLEAT, beginPos);
		}
		//now we find the beginning the @ or [ command
		char *buf = new char[pos + 2 - beginPos];
		SendScintilla(SCI_GETTEXTRANGE, beginPos, pos + 1, buf);
		context = buf;
		delete[] buf;
		int p = context.indexOf(' ');
		cmdname = context;
		if (p >= 0)
			cmdname.truncate(p);
		int p2 = context.lastIndexOf(' ');
		attrContext = context.right(context.length() - p2 - 1);
		int p3 = attrContext.indexOf('=');
		if (p3 >= 0)
			attrContext.truncate(p3);
		if (p < 0 && (cmdname[0] == '@' || cmdname[0] == '['))
		{
			completeType = SHOW_AUTOCOMMANDLIST;
		}
		else if (ch == '=' && isVarName(attrContext))
		{
			completeType = SHOW_ENUMLIST;
		}
		else if (isspace(ch) || isVarName(attrContext))
		{
			completeType = SHOW_ATTR;
		}
		else
		{
			
			completeType = SHOW_AUTOVALLIST;
		}
		cmdname.remove(0, 1);
	}
	else
	{
		if (ch == '@' || ch == '[')
		{
			//begin of cmd
			context = ch;
			completeType = SHOW_AUTOCOMMANDLIST;
		}
		else
			completeType = SHOW_NULL;
	}

	if (completeList.isEmpty())
		return;

	switch (completeType)
	{
	case SHOW_AUTOCOMMANDLIST:
		completeList = analysis->getCmdList();
		SendScintilla(SCI_AUTOCSHOW, cmdname.length(), completeList.toUtf8().data());
		break;
	case SHOW_ATTR:
		completeList = getAttrs(cmdname, context);
		SendScintilla(SCI_AUTOCSHOW, attrContext.length(), completeList.toUtf8().data());
		break;
	case SHOW_ENUMLIST:
		completeList = getEnums(cmdname, attrContext, context);
		SendScintilla(SCI_AUTOCSHOW, 0UL, completeList.toUtf8().data());
		break;
	case SHOW_AUTOVALLIST:
		//completeList is set before
		SendScintilla(SCI_AUTOCSHOW, lastContext.length(), completeList.toUtf8().data());
		break;
	default:
		break;
	}
}

//文字或样式被改变后
void BkeScintilla::UiChange(int updated)
{
	if (ChangeIgnore) return;

	ChangeIgnore++;
	if (ChangeType & SC_PERFORMED_USER)
	{
		BkeStartUndoAction();
	}

	if (ChangeType & SC_MOD_INSERTTEXT){
		//自动补全
		showComplete();
		//defparser->TextBeChange(&modfieddata, this);
		//CompliteFromApi();
	}

	if (IsWorkingUndo && !ChangeIgnore) BkeEndUndoAction();

	//缩进
	if (modfieddata.lineadd == 1 && (modfieddata.text == "\n" || modfieddata.text == "\r\n"))
	{
		char chPrev = SendScintilla(SCI_GETCHARAT, modfieddata.pos - 1);
		char chNext = SendScintilla(SCI_GETCHARAT, modfieddata.pos + modfieddata.text.count());

		if ((chPrev == '{' && chNext == '}') ||
			(chPrev == '[' && chNext == ']') ||
			(chPrev == '(' && chNext == ')')
			)
		{
			int count = SendScintilla(SCI_GETLINEINDENTATION, modfieddata.line);
			SendScintilla(SCI_INSERTTEXT, modfieddata.pos, modfieddata.text.toLatin1().data());
			SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count + SendScintilla(SCI_GETTABWIDTH));
			SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 2, count);
			SendScintilla(SCI_GOTOPOS, modfieddata.pos + modfieddata.text.count() + GetActualIndentCharLength(modfieddata.line + 1));
		}
		else
		{
			int count = SendScintilla(SCI_GETLINEINDENTATION, modfieddata.line);
			int ly = defparser->GetIndentLayer(this, modfieddata.line);
			if (ly < 0)
			{
				count += SendScintilla(SCI_GETTABWIDTH) * ly;
				SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line, count);
				modfieddata.pos--; //我也不知道为啥会这样，总之偏移了一个字节
			}
			else if (ly > 0)
			{
				count += SendScintilla(SCI_GETTABWIDTH);
			}
			SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count);
			SendScintilla(SCI_GOTOPOS, modfieddata.pos + modfieddata.text.count() + GetActualIndentCharLength(modfieddata.line + 1));
		}
	}

	//括号，引号补全
	if (modfieddata.text.length() == 1 && (modfieddata.text == "(" || modfieddata.text == "[" || modfieddata.text == "{" || modfieddata.text == "\"" || modfieddata.text == "'" || modfieddata.text == ")" || modfieddata.text == "]" || modfieddata.text == "}"))
	{
		QString lefts = "([{\"\'";
		QString rights = ")]}\"\'";
		unsigned char style3 = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos - 1);
		unsigned char style = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos);
		unsigned char style2 = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos + 1);
		//unsigned char style3 = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos + 1);
		char chPrev = SendScintilla(SCI_GETCHARAT, modfieddata.pos - 1);
		char ch = SendScintilla(SCI_GETCHARAT, modfieddata.pos);
		char chNext = SendScintilla(SCI_GETCHARAT, modfieddata.pos + 1);
		if (ChangeType & SC_MOD_INSERTTEXT)
		{
			char match[2];
			bool caret = false;
			switch (ch)
			{
			case '(':
				if ((style & 63 == SCE_BKE_STRING) || (style & 63 == SCE_BKE_STRING2))
					break;
				match[0] = ')';
				break;
			case '[':
				if ((style & 63 == SCE_BKE_STRING) || (style & 63 == SCE_BKE_STRING2))
					break;
				match[0] = ']';
				break;
			case '{':
				if ((style & 63 == SCE_BKE_STRING) || (style & 63 == SCE_BKE_STRING2))
					break;
				match[0] = '}';
				break;
			case '\"':
				match[0] = '\"';
				//判断是leading还是ending的"'
				if (ch == chNext)
					caret = true;
				break;
			case '\'':
				match[0] = '\'';
				//判断是leading还是ending的"'
				if (ch == chNext)
					caret = true;
				break;
			default:
				if (ch==chNext)
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

	if (ChangeType & SC_PERFORMED_USER)
	{
		BkeEndUndoAction();
	}
	ChangeIgnore--;

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

	SendScintilla(SCI_AUTOCSETIGNORECASE, false);

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
	ChangeIgnore++;
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
	BkeEndUndoAction();
	ChangeIgnore--;
	completeList.clear();
}

//自动完成列表被选择
void BkeScintilla::AutoListChoose(const char* text, int pos)
{
	ChangeIgnore++;
	BkeStartUndoAction();
	SendScintilla(SCI_AUTOCCANCEL);  //取消自动完成，手动填充
	ChooseComplete(text, pos);
	BkeEndUndoAction();
	ChangeIgnore--;
	completeList.clear();
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
	abc = findIndicator(BKE_INDICATOR_FIND, pos);
	abc = simpleFind(fstrdata.constData(), findflag, abc.Start(), abc.End());
	if (abc.IsNull()) return false;

	if (!findlast.IsNull())
	{
		SetIndicator(BKE_INDICATOR_FIND, findlast);
		findlast.Clear();
	}
	//if( !findlast.IsNull() ){ //上一个节点有效，则再寻找一次
	//    abc = simpleFind(fstrdata.constData(),findflag,findlast.Start(),findlast.End()) ;
	//    SetIndicator(BKE_INDICATOR_FIND,abc);
	//}
	//abc.SetEnd(abc.Start() + findstr_length);
	ClearIndicator(abc);
	SendScintilla(SCI_GOTOPOS, abc.Start());
	setSelection(abc);
	findlast = abc;
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

	BkeIndicatorBase abc2 = findIndicatorLast(BKE_INDICATOR_FIND, pos);
	if (abc2.IsNull() || abc2.End() == 0)
	{
		QMessageBox::information(this, "查找", "再往前没有了！", QMessageBox::Ok);
		return false;
	}

	if (!findlast.IsNull())
	{
		SetIndicator(BKE_INDICATOR_FIND, findlast);
		findlast.Clear();
	}
	//if (!findlast.IsNull()){
	//	SetIndicator(BKE_INDICATOR_FIND, findlast);
	//	clearSelection();
	//}
	BkeIndicatorBase abc;
	abc.SetEnd(abc2.Start());
	do
	{
		abc = simpleFind(fstrdata.constData(), findflag, abc.End(), abc2.End());
	} while (abc.End() < abc2.End());
	ClearIndicator(abc);
	SendScintilla(SCI_GOTOPOS, abc.Start());
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
	do
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

		//from = SendScintilla(SCI_GETTARGETEND) + 1;
		SendScintilla(SCI_SETANCHOR, SendScintilla(SCI_GETTARGETSTART) + 1);
		SendScintilla(SCI_GOTOPOS, SendScintilla(SCI_GETTARGETSTART));
		SendScintilla(SCI_SETSELECTIONSTART, SendScintilla(SCI_GETTARGETSTART));
		SendScintilla(SCI_SETSELECTIONEND, SendScintilla(SCI_GETTARGETEND));
	} while (0);

	ChangeStateFlag &= (~BKE_CHANGE_REPLACE);
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

		from = SendScintilla(SCI_GETTARGETEND);
	}

	ChangeStateFlag &= (~BKE_CHANGE_REPLACE);
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
	SendScintilla(SCI_SETINDICATORCURRENT, BKE_INDICATOR_FIND);
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


void BkeScintilla::BkeStartUndoAction(bool newUndo/* = true*/)
{
	if (IsWorkingUndo)
	{
		if (newUndo)
		{
			QsciScintilla::endUndoAction();
			emit Undoready(isUndoAvailable());
			QsciScintilla::beginUndoAction();
		}
		return;
	}
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
	//SendScintilla(SCI_PRIVATELEXERCALL, 1, pdata);
	//SendScintilla(SCI_SETMARGINSENSITIVEN, SC_MARKNUM_FOLDER, true);
}

int BkeScintilla::GetActualIndentCharLength(int lineID)
{
	QString text = this->text(lineID);
	int length = 0;
	while (length < text.size() && (text[length] == '\t' || text[length] == ' '))
		length++;
	return length;
}


bool BkeScintilla::event(QEvent *e)
{
	if (e->type() == QEvent::ToolTip){
		ShowInfomation(((QHelpEvent*)e)->pos());
		return true;
	}
	return QsciScintilla::event(e);
}


//显示鼠标悬浮位置的信息
void BkeScintilla::ShowInfomation(QPoint pos)
{
	int n_pos = SendScintilla(BkeScintilla::SCI_POSITIONFROMPOINT, pos.x(), pos.y());

	auto node = analysis->findNode(FileName, n_pos);
	//test indicator
	int v2 = SendScintilla(SCI_INDICATORVALUEAT, 2, n_pos);
	int v3 = SendScintilla(SCI_INDICATORVALUEAT, 3, n_pos);

	int v = v2 ? v2 - 1 : v3 - 1;

	if (v >= 0)
	{
		QString arg1;
		if (node)
			arg1 = node->name;
		QString arg2;
		BaseNode* node2 = NULL;
		if (node)
			node2 = node->findChild(n_pos - node->startPos);
		if (node2)
			arg2 = node2->name;
		QString inform(InidicatorMSG[v]);
		inform = inform.arg(arg1, arg2);
		QToolTip::showText(QCursor::pos(), inform);
		return;
	}

	//获取鼠标所在位置的位置（相当于文档）
	if (node && node->isCommand())
	{
		QString name = node->name;
		//test cmd first
		{
			auto it = CmdList.find(name);
			if (it != CmdList.end())
			{
				QString info = "命令:" + it->name + "\t参数:" + it->argNames.join(' ') + '\n' + it->detail;
				QToolTip::showText(QCursor::pos(), info);
				return;
			}
		}
		//test specialcmd
		{
			auto it = SpecialCmdList.find(name);
			if (it != SpecialCmdList.end())
			{
				auto mode = node->findIndex("mode", 0);
				if (mode && !mode->name.isEmpty())
				{
					BKECmdInfo *info = NULL;
					QString modename = mode->name;
					PAModule pa(modename);
					bool f;
					int idx = pa.getIntValue(&f);
					if (f)
					{
						for (auto &it3 : it->modes)
						{
							if (it3.second.first == idx)
							{
								info = &it3.second.second;
								modename = it3.first;
							}
						}
					}
					else
					{
						modename = pa.getStringValue(&f);
						if (f)
						{
							auto it3 = it->modes.find(modename);
							if (it3 != it->modes.end())
								info = &it3->second.second;
						}
					}

					if (info)
					{
						QString t = "命令:" + node->name + "模式:" + modename + "\t参数:" + info->argNames.join(' ') + '\n' + info->detail;
						QToolTip::showText(QCursor::pos(), t);
						return;
					}
				}
				return;
			}
		}
		//test macro
		{
			BKEMacros m_info;
			bool f = analysis->findMacro(name, &m_info);
			if (f)
			{
				QString info = "宏:" + m_info.name;
				QString p;
				for (int i = 0; i < m_info.paramqueue.size(); i++)
					p += " " + m_info.paramqueue[i].first;
				if (!p.isEmpty())
					p.remove(0, 1);
				if (!p.isEmpty())
					info += "\t参数:" + p;
				if (!m_info.comment.isEmpty())
					info += "\n" + m_info.comment;
				QToolTip::showText(QCursor::pos(), info);
				return;
			}
		}
	}

	//long close_pos = SendScintilla(SCI_POSITIONFROMPOINTCLOSE, pt.x(),pt.y() );
	//int xl,xi ;
	//lineIndexFromPosition(close_pos,&xl,&xi);
	int line = lineAt(pos);
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
	QToolTip::showText(QCursor::pos(), t);
}
