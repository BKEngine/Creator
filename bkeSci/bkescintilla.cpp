#include <weh.h>
#include "bkescintilla.h"
#include "../BKS_info.h"
#include "../codewindow.h"
#include "CmdListLoader.h"
#include "loli/loli_island.h"
#include <Qsci/qscicommandset.h>

BkeScintilla::BkeScintilla(QWidget *parent)
	:QsciScintilla(parent)
	, refind(true)
{
	//setAttribute(Qt::WA_DeleteOnClose, true); //关闭后自动析构
	//setStyleSheet("QsciScintilla{border:0px solid red ;}");
	this->setContextMenuPolicy(Qt::CustomContextMenu); //使用用户右键菜单
	this->setAttribute(Qt::WA_InputMethodEnabled, true);
	analysis = NULL;

	setUtf8(true);

	deflex = new QsciLexerBkeScript(this);
	//pdata = new ParseData(this);
	setLexer(deflex);

	setMarginLineNumbers(0, false);
	setMarginWidth(0, 36);
	setMarginLineNumbers(1, true);
	setMarginWidth(1, 36);
	markerDefine(QImage(":/info/errorsmall.png"), 1);
	markerDefine(QImage(":/info/warningsmall.png"), 2);
	markerDefine(QImage(":/info/bookmarksmall.png"), 3);
	markerDefine(QImage(":/info/pinsmall.png"), 4);
	markerDefine(QImage(":/info/errorsmall.png"), 5);
	markerDefine(QImage(":/info/warningsmall.png"), 6);
	setMarginMarkerMask(0, 0b111111111);
	setMarginMarkerMask(1, 0);
	setMarginSensitivity(0, true);
	//Debug Breakpoint
	markerDefine(QsciScintilla::Circle, 7);
	setMarkerBackgroundColor(QColor(Qt::GlobalColor::red), 7);
	//Debug CurrentLine
	markerDefine(QsciScintilla::RightArrow, 8);
	setMarkerBackgroundColor(QColor(Qt::GlobalColor::darkYellow), 8);

	SendScintilla(SCI_SETSCROLLWIDTHTRACKING, true);
	SendScintilla(SCI_SETMOUSEDWELLTIME, 500);

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

	DefineIndicators(BKE_INDICATOR_CLICK_COMMAND, INDIC_HIDDEN);
	SendScintilla(SCI_INDICSETHOVERSTYLE, BKE_INDICATOR_CLICK_COMMAND, INDIC_PLAIN);
	SendScintilla(SCI_INDICSETFORE, BKE_INDICATOR_CLICK_COMMAND, deflex->defaultColor(SCE_BKE_COMMAND));
	SendScintilla(SCI_INDICSETALPHA, BKE_INDICATOR_CLICK_COMMAND, 255);

	DefineIndicators(BKE_INDICATOR_CLICK_LABEL, INDIC_HIDDEN);
	SendScintilla(SCI_INDICSETHOVERSTYLE, BKE_INDICATOR_CLICK_LABEL, INDIC_PLAIN);
	SendScintilla(SCI_INDICSETFORE, BKE_INDICATOR_CLICK_LABEL, deflex->defaultColor(SCE_BKE_LABEL_IN_PARSER));
	SendScintilla(SCI_INDICSETALPHA, BKE_INDICATOR_CLICK_LABEL, 255);

	DefineIndicators(BKE_INDICATOR_HIGHLIGHT, INDIC_STRAIGHTBOX);
	SendScintilla(SCI_INDICSETFORE, BKE_INDICATOR_HIGHLIGHT, QColor(0x32, 0x99, 0xcc));
	SendScintilla(SCI_INDICSETALPHA, BKE_INDICATOR_HIGHLIGHT, 60);
	SendScintilla(SCI_INDICSETOUTLINEALPHA, BKE_INDICATOR_HIGHLIGHT, 128);
	SendScintilla(SCI_INDICSETUNDER, BKE_INDICATOR_HIGHLIGHT, true);

	//Multi Selection
	SendScintilla(SCI_SETMULTIPLESELECTION, true);
	SendScintilla(SCI_SETADDITIONALSELECTIONTYPING, true);
	SendScintilla(SCI_SETMULTIPASTE, SC_MULTIPASTE_EACH);
	SendScintilla(SCI_SETADDITIONALCARETSBLINK, true);
	SendScintilla(SCI_SETADDITIONALCARETSVISIBLE, true);

	setMarginsForegroundColor(QColor(100, 100, 100));
	setMarginsBackgroundColor(QColor(240, 240, 240));
	//setAutoIndent(true);
	
	SendScintilla(SCI_SETUSETABS, true);
	SendScintilla(SCI_SETINDENT, 0);

	//setIndentationGuides(true) ;
	Separate = QString(" ~!@#$%^&*()-+/|{}[]:;/=.,?><\\\n\r");
	ChangeIgnore = 0;
	LastLine = -1;
	ChangeStateFlag = 0;
	WorkingUndoDepth = 0;

	standardCommands()->find(QsciCommand::LineCut)->setKey(0);

	connect(this, SIGNAL(SCN_MODIFIED(int, int, const char *, int, int, int, int, int, int, int)),
		SLOT(EditModified(int, int, const char *, int, int, int, int, int, int, int)));
	connect(this, SIGNAL(SCN_UPDATEUI(int)), this, SLOT(UiChange(int)));
	connect(this, SIGNAL(SCN_DWELLSTART(int, int, int)), this, SLOT(OnDwellStart(int, int, int)));
	connect(this, SIGNAL(SCN_DWELLEND(int, int, int)), this, SLOT(OnDwellEnd(int, int, int)));
	//光标位置被改变
	connect(this, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(CurrentPosChanged(int, int)));
	connect(this, &QsciScintilla::marginClicked, this, &BkeScintilla::OnMarginClicked);
	//输入单个字符
	//connect(this, SIGNAL(SCN_CHARADDED(int)), this, SLOT(CharHandle(int)));

	tm.setTimerType(Qt::TimerType::CoarseTimer);
	connect(&tm, SIGNAL(timeout()), this, SLOT(onTimer()));
	tm.start(100);

	highlightTimer.setTimerType(Qt::CoarseTimer);
	connect(&highlightTimer, &QTimer::timeout, this, &BkeScintilla::ShowHighlight);
}

BkeScintilla::~BkeScintilla()
{
	tm.stop();
	highlightTimer.stop();
	//delete Selfparser;
}

void BkeScintilla::OnMarginClicked(int margin, int line, Qt::KeyboardModifiers state)
{
	if (margin == 0)
	{
		if (state == 0)
		{
			ToggleBreakpoint(line);
		}
	}
}

void BkeScintilla::Detach()
{
	CancelHighlight();
}

void BkeScintilla::Attach()
{
	ScanMacroDefine();
}

void BkeScintilla::onTimer()
{
	if (!analysis)
		return;
	auto p = analysis->lockFile(FileName);
	if (!p || !p->refresh)
	{
		return;
	}
	p->refresh = false;
	QSortedSet<QString> l;
	p->getLabels(l);
	emit refreshLabel(l);
	//clear indicator
	int len = length();
	SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, BKE_INDICATOR_ERROR);
	SendScintilla(BkeScintilla::SCI_INDICATORCLEARRANGE, 0, len);
	SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, BKE_INDICATOR_WARNING);
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

	ScanMacroDefine(p, l);
}

void BkeScintilla::ScanMacroDefine()
{
	if (analysis->isMacroFile(FileName))
	{
		auto p = analysis->lockFile(FileName);
		if (!p)
			return;
		QSortedSet<QString> l;
		p->getLabels(l);
		ScanMacroDefine(p, l);
	}
}

void BkeScintilla::ScanMacroDefine(ScopePointer<ParseData> &p, const QSortedSet<QString>& l)
{
	// macro file的annotation
	if (analysis->isMacroFile(FileName))
	{
		saveTopLine();
		clearAnnotations(AnnotationType::MACRO_DEFINE);
		BKEMacros macro;
		for (auto && label : l)
		{
			if (analysis->findMacro(label, &macro) && macro.definefile == FileName)
			{
				QString info = "//";
				QString params;
				for (int i = 0; i < macro.paramqueue.size(); i++)
				{
					params += " " + macro.paramqueue[i].first;
					if (!macro.paramqueue[i].second.isEmpty())
						params += "=" + macro.paramqueue[i].second;
				}
				if (!params.isEmpty())
					params.remove(0, 1);
				if (!params.isEmpty())
					info += "参数:" + params;
				else
					info += "参数：无";
				if (!macro.comment.isEmpty())
					info += "\n" + macro.comment;
				static QsciStyle ks3(-1, "Macro Define", QColor(0, 0x80, 0), QColor(0xe0, 0xff, 0xe0), GetAnnotationFont());
				annotate(lineFromPosition(p->findLabel(label)) - 1, info, ks3, MACRO_DEFINE);
			}
		}
		restoreTopLine();
	}
}

void BkeScintilla::saveTopLine()
{
	topLine = SendScintilla(SCI_GETFIRSTVISIBLELINE);
}

void BkeScintilla::restoreTopLine()
{
	SendScintilla(SCI_SETFIRSTVISIBLELINE, topLine);
}

void BkeScintilla::CurrentPosChanged(int line, int index)
{
	int lineChanged = abs(line - LastLine);
	LastLine = line;
	if (lineChanged >= 3)
		emit ShouldAddToNavigation();
	//index 代表着 column，所以不能使用它提供的positionFromLineIndex接口，有病
	if (!IsIndicator(BKE_INDICATOR_HIGHLIGHT, GetCurrentPosition()) && !IsIndicator(BKE_INDICATOR_HIGHLIGHT, GetCurrentPosition() - 1))
	{
		CancelHighlight();
		if (!IgnorePosChanged && IsSelectionsEmpty())
			highlightTimer.start(400);
	}
	if (IgnorePosChanged)
		return;
	if(autoCompleteType != SHOW_NULL)
		emit AutoCompleteCancel();
}

void BkeScintilla::OnDwellStart(int position, int x, int y)
{
	ShowToolTip(position, QPoint(x, y));
}

void BkeScintilla::OnDwellEnd(int position, int x, int y)
{
	HideToolTip();
}

void BkeScintilla::EditModified(int pos, int mtype, const char *text,
	int len, int added, int line, int foldNow, int foldPrev, int token,
	int annotationLinesAdded)
{
	if (ChangeIgnore) return;

	int xline, xindex;
	lineIndexFromPosition(pos, &xline, &xindex);

	if (mtype & (SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE))
	{
		IgnorePosChanged = true;
		if (mtype & SC_PERFORMED_USER)
		{
			BkeStartUndoAction();
		}
	}
	else if (mtype & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT))
	{
		ChangeType = mtype;
		modfieddata.pos = pos;
		modfieddata.type = mtype;
		modfieddata.line = xline;
		modfieddata.index = xindex;
		modfieddata.lineadd = added;
		modfieddata.bytes = text;
		modfieddata.text = QString::fromUtf8(text);
	}
}

QFont BkeScintilla::GetAnnotationFont()
{
	QFont font = deflex->font(0);
	font.setPointSize(12);
	return font;
}

static bool isVarName(const QString &s)
{
	if (s.isEmpty())
		return false;
	bool r = true;
	QByteArray ba = s.toUtf8();
	for (int i = 0; i < ba.length(); i++)
	{
		unsigned char c = ba[i];
		if (i == 0 && isalpha(c))
			continue;
		if (i > 0 && isalnum(c))
			continue;
		if (c == '_' || c >= 0x80)
			continue;
		r = false;
		break;
	}
	return r;
}

static void getAllMembers(const BKE_hashmap<BKE_String, BKE_Variable> &map, QList<QPair<QString, int>> &result)
{
	for (auto &it : map)
	{
		//TODO func def
		if (it.second.getType() == VAR_FUNC)
			result << qMakePair(QString::fromStdWString(it.first.getConstStr()) + "()", 3);
		else
			result << qMakePair(QString::fromStdWString(it.first.getConstStr()), 1);
	}
}

static void getAllMembers(BKE_VarClass *cla, QList<QPair<QString, int>> &result)
{
	for (int i = 0; i < cla->parents.size(); i++)
		getAllMembers(cla->parents[i], result);
	getAllMembers(cla->varmap, result);
}

static void getAllMembers(BKE_VarClosure *clo, QList<QPair<QString, int>> &result)
{
	if (clo->parent)
		getAllMembers(clo->parent, result);
	getAllMembers(clo->varmap, result);
}

static void getAllMembers(BKE_VarDic *dic, QList<QPair<QString, int>> &result)
{
	getAllMembers(dic->varmap, result);
}

QList<QPair<QString, int>> BkeScintilla::GetAttrs(const QString &name, const QStringList &attrs, const QString &alltext)
{
	QList<QPair<QString, int>> result;
	//test macro first
	{
		BKEMacros macro;
		bool r = analysis->findMacro(name, &macro);
		if (r && !macro.paramqueue.empty())
		{
			for (int i = 0; i < macro.paramqueue.size(); i++)
			{
				QString &attr = macro.paramqueue[i].first;
				if (!attrs.contains(attr) && !attrs.contains(QString::number(i)))
				{
					result.push_back(qMakePair(attr, 0));
				}
			}
			return result;
		}
	}
	{
		auto it = CmdList.find(name);
		if (it != CmdList.end())
		{
			if (it->argNames.empty())
				return result;
			for (int i = 0; i < it->argNames.size(); i++)
			{
				QString &attr = it->argNames[i];
				if (!attrs.contains(attr) && !attrs.contains(QString::number(i)))
				{
					result.push_back(qMakePair(attr, 0));
				}
			}
			return result;
		}
	}
	//test specialcmd
	{
		auto it = SpecialCmdList.find(name);
		if (it != SpecialCmdList.end())
		{
			QRegExp reg("\\smode=([^ ]*)");
			auto modeidx = alltext.indexOf(reg);
			QBkeCmdInfo *info = nullptr;
			if (modeidx >= 0)
			{
				QString modename = reg.cap(1);
				PAModule pa(modename);
				bool f;
				int idx = pa.getIntValue(&f);
				if (f)
				{
					/*auto &&m = it.value();
					for (auto it3 = m.begin(); it3!=m.end(); it3++)
					{
						if (it3.key() == idx)
						{
							info = &it3.second.second;
							modename = it3.first;
						}
					}*/
				}
				else
				{
					modename = pa.getStringValue(&f);
					if (f)
					{
						auto it3 = it->find(modename);
						if (it3 != it->end())
							info = &it3.value();
					}
				}

				if (info)
				{
					if (info->argNames.isEmpty())
						return result;
					for (int i = 0; i < info->argNames.size(); i++)
					{
						QString &attr = info->argNames[i];
						if (!attrs.contains(attr) && !attrs.contains(QString::number(i)))
						{
							result.push_back(qMakePair(attr, 0));
						}
					}
					return result;
				}
			}
			else
			{
				result.push_back(qMakePair(QString("mode"), 0));
			}
			return result;
		}
	}
	return result;
}

//static struct  
//{
//	QString cmd;
//	QString param;
//	QHash<QString, uint32_t> *map;
//} argEnumAutoLists[] = {
//	{"animate", "loop", &CmdAnimateLoopModeEnumList},
//};

QList<QPair<QString, int>> BkeScintilla::GetScriptList()
{
	auto ls = workpro->AllScriptFiles();
	QList<QPair<QString, int>> result;
	for (QString &s : ls)
	{
		result << qMakePair('\"' + QFileInfo(s).baseName() + '\"' , 0);
	}
	return result;
}

QList<QPair<QString, int>> BkeScintilla::GetLabelList()
{
	QList<QPair<QString, int>> result;
	QSortedSet<QString> ls;
	analysis->getLabels(FileName, ls);
	for (auto &s : ls)
	{
		result.push_back(qMakePair("*" + s, 0));
	}
	return result;
}

QList<QPair<QString, int>> BkeScintilla::GetEnums(const QString &name, const QString &attr, const QString &alltext)
{
	QList<QPair<QString, int>> result;
	{
		auto it = CmdList.find(name);
		if (it != CmdList.end())
		{
			auto it2 = it->argNames.indexOf(attr);
			if (it2 >= 0)
			{
				//for (auto &&s : argEnumAutoLists)
				//{
				//	if (name == s.cmd && attr == s.param)
				//	{
				//		QHash<QString, uint32_t> *map = s.map;
				//		auto ls = map->keys();
				//		for (auto &it3 : ls)
				//		{
				//			it3 = '\"' + it3 + '\"';
				//		}
				//		return ls.join(' ');
				//	}
				//}
				QBkeCmdInfo *info = &it.value();
				auto argEnums = info->argEnums[it2];
				if (!argEnums.isEmpty())
				{
					QStringList qs = argEnums.split(" ");
					for(auto &&s : qs)
					{
						result << qMakePair(s, 0);
					}
				}
				else if (info->argFlags[it2] & PT_BOOL)
				{
					return result << qMakePair(QString("true"), 0) << qMakePair(QString("false") ,0);
				}
				else if (info->argFlags[it2] & PT_SCRIPT)
				{
					return GetScriptList();
				}
				else if (info->argFlags[it2] & PT_LABEL)
				{
					return GetLabelList();
				}
			}
			return result;
		}
	}
	{
		auto it = SpecialCmdList.find(name);
		if (it != SpecialCmdList.end())
		{
			if (attr == "mode")
			{
				QStringList ls = it->keys();
				std::sort(ls.begin(), ls.end(), [&it](const QString &l, const QString &r) {
					QBkeCmdInfo &linfo = (*it)[l];
					QBkeCmdInfo &rinfo = (*it)[r];
					if (linfo.priority > rinfo.priority)
						return true;
					else if (linfo.priority == rinfo.priority)
						return l > r;
					return false;
				});
				for (auto &&it3 : ls)
				{
					result << qMakePair('\"' + it3 + '\"', 0);
				}
				return result;
			}
			QRegExp reg("\\smode=([^ ]*)");
			auto modeidx = alltext.indexOf(reg);
			if (modeidx >= 0)
			{
				QString modename = reg.cap(1);
				PAModule pa(modename);
				bool f;
				QBkeCmdInfo *info = NULL;
				int idx = pa.getIntValue(&f);
				if (f)
				{
					/*for (auto &it3 : it->modes)
					{
						if (it3.second.first == idx)
						{
							info = &it3.second.second;
							modename = it3.first;
						}
					}*/
				}
				else
				{
					modename = pa.getStringValue(&f);
					if (f)
					{
						auto it3 = it->find(modename);
						if (it3 != it->end())
							info = &it3.value();
					}
				}

				if (info)
				{
					auto it2 = info->argNames.indexOf(attr);
					if (it2 >= 0)
					{
						//for (auto &&s : argEnumAutoLists)
						//{
						//	if (name == s.cmd && attr == s.param)
						//	{
						//		QHash<QString, uint32_t> *map = s.map;
						//		auto ls = map->keys();
						//		for (auto &it3 : ls)
						//		{
						//			it3 = '\"' + it3 + '\"';
						//		}
						//		return ls.join(' ');
						//	}
						//}
						auto argEnums = info->argEnums[it2];
						if (!argEnums.isEmpty())
						{
							QStringList qs = argEnums.split(" ");
							for (auto &&s : qs)
							{
								result << qMakePair(s, 0);
							}
						}
						else if (info->argFlags[it2] & PT_BOOL)
						{
							return result << qMakePair(QString("true"), 0) << qMakePair(QString("false"), 0);
						}
						else if (info->argFlags[it2] & PT_SCRIPT)
						{
							return GetScriptList();
						}
						else if (info->argFlags[it2] & PT_LABEL)
						{
							return GetLabelList();
						}
					}
					return result;
				}
			}
			return result;
		}
	}
	return result;
}

QList<QPair<QString, int>> BkeScintilla::GetValList(const QStringList &ls, const QString &alltext)
{
	QList<QPair<QString, int>> result;
	auto p = analysis->lockFile(FileName);
	if (!p)
	{
		return result;
	}
	BKE_Variable v;
	auto idx = 0;
	while (idx != ls.size() - 1)
	{
		if (!idx)
		{
			if (ls[idx] == "global")
				v = p->fileclo->addRef();
			else
				v = p->fileclo->getMember(ls[idx].toStdWString());
		}
		else if (v.getType() == VAR_DIC || v.getType() == VAR_CLASS || v.getType() == VAR_CLO)
		{
			v = v.dot(ls[idx].toStdWString());
		}
		else
		{
			return result;
		}
		idx++;
	}
	BKE_Variable vv;
	switch (v.getType())
	{
	case VAR_DIC:
		vv = BKE_VarClosure::global()->getMember(v.getTypeBKEString());
		if (vv.getType() != VAR_CLASS)
		{
			return result;
		}
		getAllMembers((BKE_VarClass*)vv.obj, result);
		getAllMembers((BKE_VarDic*)v.obj, result);
		return result;
	case VAR_CLO:
		getAllMembers((BKE_VarClosure*)v.obj, result);
		return result;
	case VAR_CLASS:
		getAllMembers((BKE_VarClass*)v.obj, result);
		return result;
	default:
		vv = BKE_VarClosure::global()->getMember(v.getTypeBKEString());
		if (vv.getType() != VAR_CLASS)
		{
			return result;
		}
		getAllMembers((BKE_VarClass*)vv.obj, result);
		return result;
	}
}

QList<QPair<QString, int>> BkeScintilla::GetGlobalList(const QString &ls, const QString &alltext)
{
	QList<QPair<QString, int>> result;
	auto p = analysis->lockFile(FileName);
	if (!p)
	{
		return result;
	}
	getAllMembers(p->fileclo, result);
	for (auto &it : global_bke_info.BagelWords)
	{
		result << qMakePair(it, 9);
	}
	return result;
}

void BkeScintilla::clearAnnotations(AnnotationType type)
{
	for (auto it = annotations.begin(); it != annotations.end();)
	{
		if (it.value() == type)
		{
			QsciScintilla::clearAnnotations(it.key());
			it = annotations.erase(it);
		}
		else
			it++;
	}
}

void BkeScintilla::clearAnnotationsAll()
{
	QsciScintilla::clearAnnotations();
	annotations.clear();
}

void BkeScintilla::annotate(int line, const QString & text, int style, AnnotationType type)
{
	QsciScintilla::annotate(line, text, style);
	annotations.insert(line, type);
}

void BkeScintilla::annotate(int line, const QString & text, const QsciStyle & style, AnnotationType type)
{
	QsciScintilla::annotate(line, text, style);
	annotations.insert(line, type);
}

void BkeScintilla::annotate(int line, const QsciStyledText & text, AnnotationType type)
{
	QsciScintilla::annotate(line, text);
	annotations.insert(line, type);
}

void BkeScintilla::annotate(int line, const QList<QsciStyledText>& text, AnnotationType type)
{
	QsciScintilla::annotate(line, text);
	annotations.insert(line, type);
}

void BkeScintilla::setFirstVisibleDocumentLine(int linenr)
{
	int docLine = SendScintilla(BkeScintilla::SCI_VISIBLEFROMDOCLINE, linenr);
	setFirstVisibleLine(docLine);
}

void BkeScintilla::UpdateAutoComplete()
{
	//当前不存在AutoComplete的Context，则尝试找一个新的AutoComplete
	if (autoCompleteType == SHOW_NULL)
	{
		int pos = modfieddata.pos;
		if (ChangeType & SC_MOD_INSERTTEXT)
			pos = pos + modfieddata.bytes.length() - 1;
		else
			pos = pos - 1;
		unsigned char style = SendScintilla(SCI_GETSTYLEAT, pos);
		unsigned char ch = SendScintilla(SCI_GETCHARAT, pos);
		unsigned char curstyle = style;
		int beginPos = pos;
		unsigned int lastStyle = 0;
		unsigned char st = style & 63;
		QString context;
		QList<QPair<QString, int>> completeList;

		if (st == SCE_BKE_STRING || st == SCE_BKE_STRING2 || st == SCE_BKE_TRANS || st == SCE_BKE_ANNOTATE || st == SCE_BKE_COMMENT || st == SCE_BKE_LABEL_DEFINE || st == SCE_BKE_TEXT)
			return;
		//handle label in parser
		if ((style & 63) == SCE_BKE_LABEL_IN_PARSER)
		{
			do 
			{
				beginPos--;
				style = SendScintilla(SCI_GETSTYLEAT, beginPos);
			} while (beginPos > 0 && ((style & 63) == SCE_BKE_LABEL_IN_PARSER));
			beginPos++;
			autoCompleteContext = TextForRange({ beginPos, pos + 1 });
			autoCompleteType = SHOW_LABEL;
			completeList = GetLabelList();
		}
		else if (style & 64 /*BEGAL_MASK*/)
		{
			do
			{
				beginPos--;
				style = SendScintilla(SCI_GETSTYLEAT, beginPos);
			} while (beginPos > 0 && (style & 64));
			context = TextForRange({ beginPos, pos + 1 });
			QByteArray valexpBytes;
			while (isalnum(ch) || ch == '.' || ch == '_' || ch >= 0x80)
			{
				valexpBytes.push_front(ch);
				ch = SendScintilla(SCI_GETCHARAT, --pos);
			}
			//while (!attrContext.isEmpty() && (attrContext[0] >= '0' && attrContext[0] <= '9'))
			//	attrContext.remove(0, 1);
			if (valexpBytes.isEmpty())
				return;
			if (valexpBytes[0] >= '0' && valexpBytes[0] <= '9')
				return;
			QString tmp = QString::fromUtf8(valexpBytes);
			QStringList ls = tmp.split('.');
			if (ls.size() == 1)
			{
				completeList = GetGlobalList(ls[0], tmp);
			}
			else
			{
				completeList = GetValList(ls, tmp);
			}
            autoCompleteContext = ls.back();
			autoCompleteType = SHOW_AUTOVALLIST;
		}
		else if (style & 128 /*CMD_MASK*/ || (style & 63) == SCE_BKE_COMMAND || (style & 63) == SCE_BKE_COMMAND2)
		{
			if ((style & 63) == SCE_BKE_COMMAND2 && ch == ']')
			{
				autoCompleteType = SHOW_NULL;
			}
			else
			{
				if (!(((style & 63) == SCE_BKE_COMMAND && ch == '@') || ((style & 63) == SCE_BKE_COMMAND2 && ch == '[')))
				{
					unsigned char ch2;
					do
					{
						beginPos--;
						style = SendScintilla(SCI_GETSTYLEAT, beginPos);
						ch2 = SendScintilla(SCI_GETCHARAT, beginPos);
					} while (beginPos > 0 && (style & 128) && !(((style & 63) == SCE_BKE_COMMAND && ch2 == '@') || ((style & 63) == SCE_BKE_COMMAND2 && ch2 == '[')));
				}
				//now we find the beginning the @ or [ command
				context = TextForRange({ beginPos + 1, pos + 1 });
				int p = context.indexOf(' ');
				QString cmdname = context;
				if (p < 0)
				{
					autoCompleteContext = cmdname;
					autoCompleteType = SHOW_AUTOCOMMANDLIST;
					completeList = analysis->getCmdList();
				}
				else
				{
					cmdname.truncate(p);
					int p2 = context.lastIndexOf(' ');
					QString attrContext = context.right(context.length() - p2 - 1);
					int p3 = attrContext.indexOf('=');
					if (p3 >= 0)
						attrContext.truncate(p3);
					if (isspace(ch) || (isVarName(attrContext) && p3 < 0))
					{
						autoCompleteType = SHOW_ATTR;
						bool hasAttr = false;
						unsigned char oldStyle;
						int pp = beginPos + p + 1;
						oldStyle = style = ((unsigned char)SendScintilla(SCI_GETSTYLEAT, pp)) & ~128;
						pp++;
						QStringList attrs;
						QString tmp;
						while (pp <= pos)
						{
							style = ((unsigned char)SendScintilla(SCI_GETSTYLEAT, pp)) & ~128;
							if (style == SCE_BKE_DEFAULT && oldStyle == SCE_BKE_ATTRIBUTE)
							{
								attrs << tmp;
								tmp.clear();
								hasAttr = true;
							}
							else if (style == SCE_BKE_DEFAULT && hasAttr)
							{
								hasAttr = false;
							}
							else if (oldStyle != SCE_BKE_DEFAULT && oldStyle != SCE_BKE_ATTRIBUTE && style == SCE_BKE_DEFAULT && !hasAttr)
							{
								attrs << QString::number(attrs.size());
							}
							else if (style == SCE_BKE_ATTRIBUTE)
							{
								tmp.push_back((char)SendScintilla(SCI_GETCHARAT, pp));
							}
							oldStyle = style;
							pp++;
						}
						completeList = GetAttrs(cmdname, attrs, context);
						autoCompleteContext = attrContext;
					}
					else if (ch == '=' && isVarName(attrContext))
					{
						autoCompleteContext = "";
						autoCompleteType = SHOW_ENUMLIST;
						completeList = GetEnums(cmdname, attrContext, "");
					}
					else
					{
						//detect enums first
						if (isVarName(attrContext))
						{
							unsigned char ch2 = ch;
							auto pos2 = pos;
							QByteArray contextBytes;
							do
							{
								contextBytes.push_front(ch2);
								ch2 = SendScintilla(SCI_GETCHARAT, --pos2);
							} while (ch2 != '=');
							QString tmp = QString::fromUtf8(contextBytes);
							completeList = GetEnums(cmdname, attrContext, tmp);
							if (completeList.size())
							{
								autoCompleteContext = tmp;
								autoCompleteType = SHOW_ENUMLIST;
							}
						}
						if (autoCompleteType == SHOW_NULL)
						{
							QByteArray valexpBytes;
							while (isalnum(ch) || ch == '.' || ch == '_' || ch >= 0x80)
							{
								valexpBytes.push_front(ch);
								ch = SendScintilla(SCI_GETCHARAT, --pos);
							}
							//while (!attrContext.isEmpty() && (attrContext[0] >= '0' && attrContext[0] <= '9'))
							//	attrContext.remove(0, 1);
							if (valexpBytes.isEmpty())
								return;
							if (valexpBytes[0] >= '0' && valexpBytes[0] <= '9')
								return;
							QString tmp = QString::fromUtf8(valexpBytes);
							QStringList ls = tmp.split('.');
							if (ls.size() == 1)
							{
								autoCompleteContext = ls[0];
								completeList = GetGlobalList(context, tmp);
							}
							else
							{
								autoCompleteContext = ls.back();
								completeList = GetValList(ls, tmp);
							}
							autoCompleteType = SHOW_AUTOVALLIST;
						}
					}
				}
			}
		}
		if (autoCompleteType != SHOW_NULL && !completeList.empty())
		{
			emit AutoCompleteStart(completeList, autoCompleteContext);
		}
		else
		{
			autoCompleteType = SHOW_NULL;
			autoCompleteContext.clear();
		}
	}
	else
	{
		if (ChangeType & SC_MOD_INSERTTEXT)
		{
			if (modfieddata.text == "\n" || modfieddata.text == "\r\n")
			{
				emit AutoCompleteCancel();
				return;
			}
			else
			{
				autoCompleteContext.append(modfieddata.text);
			}
		}
		else
		{
			if (modfieddata.text.length() > autoCompleteContext.size())
			{
				emit AutoCompleteCancel();
				return;
			}
			else
			{
				autoCompleteContext.remove(autoCompleteContext.size() - modfieddata.text.length(), modfieddata.text.length());
			}
		}
		//	如果是用户行为或者最后一次UNDO REDO修改的话，更新AutoComplete
		if (ChangeType & (SC_PERFORMED_USER | SC_LASTSTEPINUNDOREDO))
			emit AutoCompleteMatch(autoCompleteContext);
	}
}

void BkeScintilla::OnAutoCompleteCanceled()
{
	autoCompleteContext.clear();
	autoCompleteType = SHOW_NULL;
}

//自动完成列表被选择
void BkeScintilla::OnAutoCompleteSelected(QString text)
{
	this->setFocus();
	ChangeIgnore++;
	BkeStartUndoAction();
	ChooseComplete(text);
	BkeEndUndoAction();
	ChangeIgnore--;
	OnAutoCompleteCanceled();
}


void BkeScintilla::ChooseComplete(const QString &text)
{
	//int i = SendScintilla(SCI_AUTOCGETCURRENT);
	int pos = SendScintilla(SCI_GETCURRENTPOS);
	int autoCompleteStartPos = pos - autoCompleteContext.toUtf8().length();
	
	RemoveRange({ autoCompleteStartPos, pos });
	InsertAndMove(text);

	switch (autoCompleteType)
	{
		case BkeScintilla::SHOW_AUTOCOMMANDLIST:
			QTimer::singleShot(0 ,[this]() {
				InsertAndMove(" ");
			});
			break;
		case BkeScintilla::SHOW_AUTOVALLIST:
			//如果最后是(),光标回移一格
			if (text.endsWith("()"))
			{
				SendScintilla(SCI_GOTOPOS, SendScintilla(SCI_GETCURRENTPOS) - 1);
			}
			break;
		case BkeScintilla::SHOW_ENUMLIST:
		{
			int pos = SendScintilla(SCI_GETCURRENTPOS);
			unsigned char a = GetByte(pos - 1);
			unsigned char b = GetByte(pos);
			if (a == b && a == '"')
			{
				SendScintilla(SCI_DELETERANGE, pos, 1);
			}
			QTimer::singleShot(0, [this]() {
				InsertAndMove(" ");
			});
		}
		break;
		case BkeScintilla::SHOW_LABEL:
			break;
		case BkeScintilla::SHOW_ATTR:
			QTimer::singleShot(0, [this]() {
				InsertAndMove("=");
			});
			break;
		case BkeScintilla::SHOW_SYS:
			break;
		default:
			break;
	}

}

// 鼠标点击，取消自动补全
void BkeScintilla::mousePressEvent(QMouseEvent * e)
{
	if(autoCompleteType != SHOW_NULL)
		emit AutoCompleteCancel();
	QsciScintilla::mousePressEvent(e);
}

// focusOut，取消自动补全
void BkeScintilla::focusOutEvent(QFocusEvent * e)
{
	if (autoCompleteType != SHOW_NULL)
		emit AutoCompleteCancel();
	CancelHighlight();
	QsciScintilla::focusOutEvent(e);
}

//文字或样式被改变后
void BkeScintilla::UiChange(int updated)
{
	if (ChangeIgnore) return;
	if (ChangeType == 0) return;

	int tabWidth = SendScintilla(SCI_GETTABWIDTH);

	ChangeIgnore++;

	if (ChangeType & SC_PERFORMED_USER)
	{
		if ((ChangeType & SC_MOD_INSERTTEXT) && (modfieddata.text == "]" || modfieddata.text == "}"))
		{
			unsigned char style = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos);
			if (style & 64)
			{
				int count = SendScintilla(SCI_GETLINEINDENTATION, modfieddata.line);
				int len = lineLength(modfieddata.line);
				char *buf = new char[len + 1];
				SendScintilla(SCI_GETLINE, modfieddata.line, buf);
				buf[len] = 0;
				len--;
				while (len >= 0 && buf[len] == '\n' || buf[len] == '\r')
					len--;
				buf[++len] = 0;
				//如果本行只有}或]
				int start = 0;
				while (isspace((unsigned char)buf[start]))
					start++;
				if (start == len - 1)
				{
					//那么往回缩
					if (count < tabWidth)
						count = tabWidth;
					else
						modfieddata.pos--;
					SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line, count - tabWidth);
					SendScintilla(SCI_GOTOPOS, modfieddata.pos + modfieddata.text.count());
				}
			}
		}
		if (modfieddata.lineadd == 1 && (modfieddata.text == "\n" || modfieddata.text == "\r\n"))
		{
			char chPrev = SendScintilla(SCI_GETCHARAT, modfieddata.pos - 1);
			char chNext = SendScintilla(SCI_GETCHARAT, modfieddata.pos + modfieddata.text.count());
			int count = SendScintilla(SCI_GETLINEINDENTATION, modfieddata.line);
			unsigned char style = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos);

			if ((chPrev == '{' && chNext == '}') ||
				(chPrev == '[' && chNext == ']')// ||
												//小括号还是算了
												//(chPrev == '(' && chNext == ')')
				)
			{
				SendScintilla(SCI_INSERTTEXT, modfieddata.pos, modfieddata.text.toLatin1().data());
				SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count + tabWidth);
				SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 2, count);
			}
			else if (style & 64)	//parser
			{
				int len = lineLength(modfieddata.line);
				char *buf = new char[len + 1];
				SendScintilla(SCI_GETLINE, modfieddata.line, buf);
				buf[len] = 0;
				len--;
				while (len >= 0 && buf[len] == '\n' || buf[len] == '\r')
					len--;
				buf[++len] = 0;
				if (len && (buf[len - 1] == '[' || buf[len - 1] == '{'))
				{
					SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count + tabWidth);
				}
				else
				{
					SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count);
				}
				//if (!len || buf[len - 1] == ';' || buf[len - 1] == '}')
				//{
				//沿用上一行的缩进
				//SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count);
				//}
				//else
				//{
				//	SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count + tabWidth);
				//}
				delete[] buf;
				//int ly = defparser->GetIndentLayer(this, modfieddata.line);
				//if (ly < 0)
				//{
				//	count += SendScintilla(SCI_GETTABWIDTH) * ly;
				//	SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line, count);
				//	modfieddata.pos--; //我也不知道为啥会这样，总之偏移了一个字节
				//}
				//else if (ly > 0)
				//{
				//	count += SendScintilla(SCI_GETTABWIDTH);
				//}
				//SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count);
				//SendScintilla(SCI_GOTOPOS, modfieddata.pos + modfieddata.text.count() + GetActualIndentCharLength(modfieddata.line + 1));
			}
			else
			{
				//认为是command域
				int len = lineLength(modfieddata.line);
				char *buf = new char[len + 1];
				SendScintilla(SCI_GETLINE, modfieddata.line, buf);
				buf[len] = 0;
				len--;
				while (len >= 0 && buf[len] == '\n' || buf[len] == '\r')
					len--;
				buf[++len] = 0;
				int Pos = positionFromLineIndexByte(modfieddata.line, len - 1);
				unsigned char style = SendScintilla(SCI_GETSTYLEAT, Pos);

				if (!(style & 128))
				{
					SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count);
				}
				else
				{
					while (style & 128)
					{
						style = SendScintilla(SCI_GETSTYLEAT, --Pos);
					}
					unsigned char ch = SendScintilla(SCI_GETCHARAT, Pos);
					if (ch != '@' && ch != '[')
					{
						SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count);
					}
					else
					{
						//find command name
						QString cmd;
						++Pos;
						ch = SendScintilla(SCI_GETCHARAT, Pos);
						if (ch == '_' || isalpha(ch) || ch >= 0x80)
						{
							do
							{
								cmd.push_back(ch);
								++Pos;
								ch = SendScintilla(SCI_GETCHARAT, Pos);
							} while (ch == '_' || isalnum(ch) || ch >= 0x80);
						}
						if (cmd == "if" || cmd == "for")
						{
							SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count + tabWidth);
						}
						else if (cmd == "else" || cmd == "elseif")
						{
							if (count < tabWidth)
								count = tabWidth;
							else
								modfieddata.pos--;
							SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line, count - tabWidth);
							SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count);
						}
						else if (cmd == "endif" || cmd == "next")
						{
							if (count < tabWidth)
								count = tabWidth;
							else
								modfieddata.pos--;
							SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line, count - tabWidth);
							SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count - tabWidth);
						}
						else
						{
							SendScintilla(SCI_SETLINEINDENTATION, modfieddata.line + 1, count);
						}
					}

				}
			}
			SendScintilla(SCI_GOTOPOS, modfieddata.pos + modfieddata.text.count() + GetActualIndentCharLength(modfieddata.line + 1));
		}


		//括号，引号补全
		if (modfieddata.text.length() == 1 && (modfieddata.text == "(" || modfieddata.text == "[" || modfieddata.text == "{" || modfieddata.text == "\"" || modfieddata.text == "'" || modfieddata.text == ")" || modfieddata.text == "]" || modfieddata.text == "}"))
		{
			unsigned char style3 = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos - 1);
			unsigned char style = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos);
			unsigned char style2 = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos + 1);
			//首先不要在字符串内，否则忽略
			if ((style & 63) == SCE_BKE_STRING || (style & 63) == SCE_BKE_STRING2 || (style & 63) == SCE_BKE_TRANS)
				goto out;

			QString lefts = "([{\"\'";
			QString rights = ")]}\"\'";
			//unsigned char style3 = SendScintilla(SCI_GETSTYLEAT, modfieddata.pos + 1);
			char chPrev = SendScintilla(SCI_GETCHARAT, modfieddata.pos - 1);
			char ch = SendScintilla(SCI_GETCHARAT, modfieddata.pos);
			char chNext = SendScintilla(SCI_GETCHARAT, modfieddata.pos + 1);
			if (ChangeType & SC_MOD_INSERTTEXT)
			{
				char match[2];
				bool caret = false;
				int rawstyle = style & 63;
				switch (ch)
				{
				case '(':
					if ((rawstyle == SCE_BKE_STRING) || (rawstyle == SCE_BKE_STRING2))
						break;
					match[0] = ')';
					break;
				case '[':
					if ((rawstyle == SCE_BKE_STRING) || (rawstyle == SCE_BKE_STRING2))
						break;
					match[0] = ']';
					break;
				case '{':
					if ((rawstyle == SCE_BKE_STRING) || (rawstyle == SCE_BKE_STRING2))
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
					if (ch == chNext)
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

	out:;
		BkeEndUndoAction();
	}
	IgnorePosChanged = false;
	//	如果是用户行为或者最后一次UNDO REDO修改的话
	//if (ChangeType & (SC_PERFORMED_USER | SC_LASTSTEPINUNDOREDO))
	{
		//自动补全
		UpdateAutoComplete();
	}

	{
		int len = length() + 1;
		QByteArray a(len, Qt::Initialization());
		SendScintilla(SCI_GETTEXT, len, a.data());
		analysis->pushFile(FileName, &a);
	}
	
	ChangeIgnore--;

	ChangeType = 0;
	modfieddata.clear();
}

//是否分割符
bool BkeScintilla::IsSeparate(int ch)
{
	if (ch <= 0 || ch > 127) return false;
	char a = ch & 255;
	if (Separate.indexOf(a) >= 0) return true;
	return false;
}

unsigned char BkeScintilla::GetByte(int pos) const
{
	unsigned char s;
	s = SendScintilla(SCI_GETCHARAT, pos);
	return s;
}

QString BkeScintilla::TextForRange(const BkeIndicatorBase &range) const
{
	if (range.IsNull())
		return QString();
	auto len = range.Len();
	auto buf = new char[len + 1];
	SendScintilla(SCI_GETTEXTRANGE, range.Start(), range.End(), buf);
	QString dst = QString::fromUtf8(buf);
	delete[] buf;
	return dst;
}

QByteArray BkeScintilla::TextBytesForRange(const BkeIndicatorBase &range) const
{
	if (range.IsNull())
		return QByteArray();
	auto len = range.Len();
	QByteArray qba(len + 1, Qt::Initialization());
	SendScintilla(SCI_GETTEXTRANGE, range.Start(), range.End(), qba.data());
	return qba;
}

void BkeScintilla::RemoveRange(const BkeIndicatorBase & range)
{
	if (!range) return;
	SendScintilla(SCI_DELETERANGE, range.Start(), range.Len());
}

void BkeScintilla::AppendText(const QString & text)
{
	AppendText(textAsBytes(text));
}

void BkeScintilla::AppendText(const QByteArray &text)
{
	ChangeIgnore++;
	BkeStartUndoAction(false);
	SendScintilla(SCI_APPENDTEXT, text.length(), text.constData());
	BkeEndUndoAction();
	ChangeIgnore--;
}

QByteArray BkeScintilla::TextAsBytes(const QString &text) const
{
	return textAsBytes(text);
}

int BkeScintilla::PositionByLine(int line) const
{
	return SendScintilla(SCI_POSITIONFROMLINE, line);
}

//插入并移动光标位置
void BkeScintilla::InsertAndMove(const QString &text)
{
	insert(text);
	SetCurrentPosition(GetCurrentPosition() + text.toUtf8().length());
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
	if (ss[0] == 0)
		return 0;
	testlist.clear();
	findflag = (cs ? SCFIND_MATCHCASE : 0) |
		(word ? SCFIND_WHOLEWORD : 0) |
		(exp ? SCFIND_CXX11REGEX | SCFIND_REGEXP : 0);

	SendScintilla(SCI_SETINDICATORCURRENT, BKE_INDICATOR_FIND);
	ClearIndicators(BKE_INDICATOR_FIND);

	BkeIndicatorBase abc;
	int a, len;
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
	ClearIndicator(BKE_INDICATOR_FIND,abc);
	SendScintilla(SCI_GOTOPOS, abc.Start());
	SetSelection(abc);
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
	ClearIndicator(BKE_INDICATOR_FIND,abc);
	SendScintilla(SCI_GOTOPOS, abc.Start());
	SetSelection(abc);
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
	ChangeIgnore++;
	BkeStartUndoAction();
	int flag = (cs ? SCFIND_MATCHCASE : 0) |
		(word ? SCFIND_WHOLEWORD : 0) |
		(exp ? SCFIND_CXX11REGEX | SCFIND_REGEXP : 0);

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

	BkeEndUndoAction();
	ChangeIgnore--;
	return true;
}

void BkeScintilla::ReplaceAllText(const QString &rstr, const QString &dstr, bool cs, bool exp, bool word)
{
	//关闭补全，自动提示等等
	ChangeIgnore++;
	BkeStartUndoAction();

	int flag = (cs ? SCFIND_MATCHCASE : 0) |
		(word ? SCFIND_WHOLEWORD : 0) |
		(exp ? SCFIND_CXX11REGEX | SCFIND_REGEXP : 0);

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

	BkeEndUndoAction();
	ChangeIgnore--;
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

void BkeScintilla::SetIndicator(int id, const BkeIndicatorBase &p)
{
	if (p.IsNull()) return;
	SendScintilla(SCI_SETINDICATORCURRENT, id);
	SendScintilla(SCI_INDICATORFILLRANGE, p.Start(), p.Len());
}

void BkeScintilla::SetIndicator(int id, const BkeIndicatorBase & p, int value)
{
	if (p.IsNull()) return;
	SendScintilla(SCI_SETINDICATORCURRENT, id);
	SendScintilla(SCI_SETINDICATORVALUE, value);
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

void BkeScintilla::SetSelection(const BkeIndicatorBase &p)
{
	if (p.IsNull()) return;
	SendScintilla(SCI_SETSEL, p.Start(), p.End());
	SendScintilla(SCI_SETCURRENTPOS, p.End());
}

bool BkeScintilla::IsSelectionsEmpty() const
{
	return SendScintilla(SCI_GETSELECTIONEMPTY);
}

int BkeScintilla::GetTextLength() const
{
	return SendScintilla(SCI_GETTEXTLENGTH);
}

int BkeScintilla::ClosedPositionAt(const QPoint & point) const
{
	long chpos = SendScintilla(SCI_POSITIONFROMPOINTCLOSE, point.x(), point.y());
	return (int)chpos;
}

int BkeScintilla::PositionAt(const QPoint & point) const
{
	long chpos = SendScintilla(SCI_POSITIONFROMPOINT, point.x(), point.y());
	return (int)chpos;
}

QPoint BkeScintilla::PointByPosition(int position) const
{
	return QPoint(SendScintilla(SCI_POINTXFROMPOSITION, 0, position), SendScintilla(SCI_POINTYFROMPOSITION, 0, position));
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

bool BkeScintilla::IsIndicator(int id, int pos) const
{
	return SendScintilla(SCI_INDICATORVALUEAT, id, pos) > 0;
}

void BkeScintilla::ClearIndicator(int id, const BkeIndicatorBase &p)
{
	if (p.IsNull()) return;
	SendScintilla(SCI_SETINDICATORCURRENT, id);
	SendScintilla(SCI_INDICATORCLEARRANGE, p.Start(), p.Len());
}

void BkeScintilla::ClearSelection(int pos)
{
	if (pos < 0 && pos >= this->length()) pos = SendScintilla(SCI_GETCURRENTPOS);
	SendScintilla(SCI_SETEMPTYSELECTION, pos);
}

//从区域中注释，反注释
void BkeScintilla::BkeAnnotateSelect()
{
	ChangeIgnore++;
	BkeStartUndoAction(false);
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
			SetSelection(abc);
			replaceSelectedText("");
		}
	}
	BkeEndUndoAction();
	ChangeIgnore--;
}


void BkeScintilla::BkeStartUndoAction(bool newUndo/* = true*/)
{
	if (WorkingUndoDepth >= 1)
	{
		if (newUndo)
		{
			QsciScintilla::endUndoAction();
			QsciScintilla::beginUndoAction();
		}
		WorkingUndoDepth++;
		return;
	}
	WorkingUndoDepth++;
	QsciScintilla::beginUndoAction();
}

void BkeScintilla::BkeEndUndoAction()
{
	if (!WorkingUndoDepth)
		return;
	WorkingUndoDepth = 0;
	QsciScintilla::endUndoAction();
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

int BkeScintilla::GetCurrentLine() const
{
	int xl, xi;
	lineIndexFromPosition(SendScintilla(SCI_GETCURRENTPOS), &xl, &xi);
	return xl;
}

int BkeScintilla::GetCurrentPosition() const
{
	return SendScintilla(SCI_GETCURRENTPOS);
}

void BkeScintilla::SetCurrentPosition(int pos)
{
	SendScintilla(SCI_GOTOPOS, pos);
}

void BkeScintilla::setLexer(QsciLexer *lex)
{
	QsciScintilla::setLexer(lex);
	//读取用户设置
/*
	QFont f;
	for (int i = 0; i < 256; i++){
		f = lex->defaultFont(i);
        SendScintilla(SCI_STYLESETFONT, i, f.family().toUtf8().constData());
		SendScintilla(SCI_STYLESETSIZE, i, f.pointSize());
		SendScintilla(SCI_STYLESETITALIC, i, f.italic());
		SendScintilla(SCI_STYLESETBOLD, i, f.bold());
		SendScintilla(SCI_STYLESETUNDERLINE, i, f.underline());

//		SendScintilla(SCI_STYLESETFORE, i, lex->color(i));
//		SendScintilla(SCI_STYLESETBACK, i, lex->paper(i));
	}*/

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

	// 自动补全设置
	SendScintilla(SCI_AUTOCSETORDER, SC_ORDER_CUSTOM);
	SendScintilla(SCI_AUTOCSETIGNORECASE, true);

	setFolding(FoldStyle::BoxedTreeFoldStyle, 2);
	//for debug
	//SendScintilla(SCI_SETFOLDFLAGS, SC_FOLDFLAG_LEVELNUMBERS);

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

//显示鼠标悬浮位置的信息
void BkeScintilla::ShowToolTip(int position, QPoint pos)
{
	if (analysis == nullptr)
		return;
	auto d = analysis->lockFile(FileName);
	if (!d)
		return;
	auto node = d->findNode(position);
	//test indicator
	{
		int v2 = SendScintilla(SCI_INDICATORVALUEAT, BKE_INDICATOR_ERROR, position);
		int v3 = SendScintilla(SCI_INDICATORVALUEAT, BKE_INDICATOR_WARNING, position);

		int v = v2 ? v2 - 1 : v3 - 1;

		if (v >= 0)
		{
			QString arg1;
			if (node)
				arg1 = node->name;
			QString arg2;
			BaseNode* node2 = NULL;
			if (node)
				node2 = node->findChild(position - node->startPos);
			if (node2)
				arg2 = node2->name;
			QString inform(InidicatorMSG[v]);
			inform = inform.arg(arg1, arg2);
			QToolTip::showText(QCursor::pos(), inform);
			return;
		}
	}
	//测试label_in_parser
	{
		BkeIndicatorBase indicator = GetRangeForStyle(position, SCE_BKE_LABEL_IN_PARSER);
		if (!indicator.IsNull())
		{
			QToolTip::showText(QCursor::pos(), "Ctrl+点击可以跳转到本文件内该标签;\n按Alt+Enter可以在本文件内创建该标签。");
			return;
		}
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
				QString info = "命令:" + it->name + "\t参数:" + it->argNames.join(' ') + '\n' + it->description;
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
					QBkeCmdInfo *info = NULL;
					QString modename = mode->name;
					PAModule pa(modename);
					bool f;
					int idx = pa.getIntValue(&f);
					if (f)
					{
						/*for (auto &it3 : it->modes)
						{
							if (it3.second.first == idx)
							{
								info = &it3.second.second;
								modename = it3.first;
							}
						}*/
					}
					else
					{
						modename = pa.getStringValue(&f);
						if (f)
						{
							auto it3 = it->find(modename);
							if (it3 != it->end())
								info = &it3.value();
						}
					}

					if (info)
					{
						QString t = "命令:" + node->name + "模式:" + modename + "\t参数:" + info->argNames.join(' ') + '\n' + info->description;
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
}

void BkeScintilla::HideToolTip()
{
	QToolTip::hideText();
}

void BkeScintilla::CancelHighlight()
{
	highlightTimer.stop();
	if (isHighlightShown) 
	{
		isHighlightShown = false;
		ClearIndicators(BKE_INDICATOR_HIGHLIGHT);
	}
}

void BkeScintilla::ShowHighlight()
{
	CancelHighlight();
	int pos = GetCurrentPosition();
	auto p = analysis->lockFile(FileName);
	if (!p)
		return;
	unsigned char style;
	BkeIndicatorBase range = GetRangeForStyles(pos, {SCE_BKE_PARSER_VAR, SCE_BKE_NUMBER, SCE_BKE_COLOR}, &style);
	if (!range)
		return;
	int startPos = 0;
	int lastPos = length();
	auto node = p->findLastLabelNode(pos);
	if (node)
		startPos = node->endPos;
	node = p->findNextLabelNode(pos);
	if (node)
		lastPos = node->startPos;
	QByteArray textBytes = TextBytesForRange(range);
	BkeIndicatorBase found;
	do 
	{
		found = simpleFind(textBytes.constData(), SCFIND_MATCHCASE, startPos, lastPos);
		if (!found)
			break;
		unsigned char st;
		for (int i = found.Start(); i < found.End(); i++)
		{
			st = SendScintilla(SCI_GETSTYLEAT, i) & 63;
			if (st != style)
				goto end;
		}
		if (found.Start() != 0)
		{
			st = SendScintilla(SCI_GETSTYLEAT, found.Start() - 1) & 63;
			if (st == style)
				goto end;
		}
		if (found.End() != lastPos)
		{
			st = SendScintilla(SCI_GETSTYLEAT, found.End()) & 63;
			if (st == style)
				goto end;
		}
		SetIndicator(BKE_INDICATOR_HIGHLIGHT, found, style);
	end:
		startPos = found.End();
	} 
	while (true);
	isHighlightShown = true;
}

void BkeScintilla::inputMethodEvent(QInputMethodEvent * event)
{
	if (!hasFocus())
		event->setCommitString(QString());
	QsciScintilla::inputMethodEvent(event);
}

BkeIndicatorBase BkeScintilla::GetRangeForStyle(int position, unsigned char style) const
{
	BkeIndicatorBase indicator;
	unsigned char st = SendScintilla(SCI_GETSTYLEAT, position) & 63;
	if (st != style)
	{
		position--;
		st = SendScintilla(SCI_GETSTYLEAT, position) & 63;
		if (st != style)
			return indicator;
	}
	{
		int pos = position;
		do 
		{
			pos--;
			st = SendScintilla(SCI_GETSTYLEAT, pos) & 63;
			if (st != style)
			{
				indicator.SetStart(pos + 1);
				break;
			}
		} while (pos > 0);
	}
	{
		int pos = position;
		int len = this->length();
		do
		{
			pos++;
			st = SendScintilla(SCI_GETSTYLEAT, pos) & 63;
			if (st != style)
			{
				indicator.SetEnd(pos);
				break;
			}
		} while (pos < len);
	}
	return indicator;
}

BkeIndicatorBase BkeScintilla::GetRangeForStyles(int position, const QList<unsigned char> &styles, unsigned char *pstyle /*= nullptr*/) const
{
	BkeIndicatorBase result;
	for (auto style : styles)
	{
		result = GetRangeForStyle(position, style);
		if (!result.IsNull())
		{
			if (style)
			{
				*pstyle = style;
			}
			return result;
		}
	}
	return result;
}

/////////////////// Debug ///////////////////

void BkeScintilla::EmitBreakpointChange()
{
	QMap<int, BreakpointInfo> info;

	for (auto it = breakpointInfos.constBegin(); it != breakpointInfos.constEnd(); ++it)
	{
		int handle = it.key();
		int line = markerLine(handle);
		info.insert(line, it.value());
	}

	emit DebugBreakpointChange(FileName, info);
}

BreakpointInfo * BkeScintilla::GetBreakpointInfoByLine(int line)
{
	for (auto it = breakpointInfos.begin(); it != breakpointInfos.end(); ++it)
	{
		int handle = it.key();
		int l = markerLine(handle);
		if (l == line)
		{
			return &it.value();
		}
	}
	return nullptr;
}

int BkeScintilla::GetBreakpointHandleByLine(int line)
{
	for (auto it = breakpointInfos.begin(); it != breakpointInfos.end(); ++it)
	{
		int handle = it.key();
		int l = markerLine(handle);
		if (l == line)
			return handle;
	}
	return -1;
}

void BkeScintilla::AddBreakpoint(int line)
{
	int handle = markerAdd(line, 7);
	breakpointInfos.insert(handle, BreakpointInfo());
}

void BkeScintilla::DeleteBreakpoint(int line)
{
	int handle = GetBreakpointHandleByLine(line);
	if (handle == -1)
		return;
	breakpointInfos.remove(handle);
	markerDeleteHandle(handle);
}

void BkeScintilla::ToggleBreakpoint(int line)
{
	int handle = GetBreakpointHandleByLine(line);
	if (handle == -1)
		AddBreakpoint(line);
	else
	{
		breakpointInfos.remove(handle);
		markerDeleteHandle(handle);
	}
}
