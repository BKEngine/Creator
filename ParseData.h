#pragma once

#include <QVector>
#include <QSet>
#include "ParserHelper/parser/parser.h"

using namespace std;

class BkeScintilla;

extern const char *InidicatorMSG[];

#define ISSPACE(x) isspace((unsigned char)(x))

struct Pos
{
	int line;
	int pos;

	//加的时候只考虑行，基础位置在左，偏移量在右
	Pos operator + (const Pos &p) const
	{
		Pos pp;
		if (p.line)
		{
			pp.pos = p.pos;
			pp.line = p.line + this->line;
		}
		else
		{
			pp.pos = p.pos + this->pos;
			pp.line = this->line;
		}
		return pp;
	}

	Pos& operator += (const Pos &p)
	{
		if (p.line)
		{
			this->pos = p.pos;
			this->line += p.line;
		}
		else
		{
			this->pos += p.pos;
		}
		return *this;
	}
	
	Pos operator - (const Pos &p) const
	{
		Pos pp = *this;
		pp.line -= p.line;
		if (!pp.line)
			pp.pos -= p.pos;
		return pp;
	}

	Pos operator -= (const Pos &p)
	{
		this->line -= p.line;
		if (!this->line)
			this->pos -= p.pos;
		return *this;
	}


	bool operator < (const Pos &p) const
	{
		return line < p.line || (line == p.line && pos < p.pos);
	}

	bool operator <= (const Pos &p) const
	{
		return line < p.line || (line == p.line && pos <= p.pos);
	}

	bool operator > (const Pos &p) const
	{
		return line > p.line || (line == p.line && pos > p.pos);
	}

	bool operator >= (const Pos &p) const
	{
		return line > p.line || (line == p.line && pos >= p.pos);
	}

	bool operator == (const Pos &p) const
	{
		return line == p.line && pos == p.pos;
	}

	void setZero()
	{
		line = pos = 0;
	}
};

void insert(Pos p, Pos &org, Pos off);

void erase(Pos p, Pos &org, Pos off);

struct BaseNode
{
	enum
	{
		Node_Document,
		Node_Label,
		Node_LineComment,
		Node_Comment,
		Node_Command,
		Node_AtCommand,
		Node_CmdProp,
		Node_CmdPropValue,
		Node_Parser,
		Node_LineParser,
		Node_Text
	}type;

	BaseNode(BaseNode* p) : parent(p)
	{
		//startPos.line = 0;
		//startPos.pos = 0;
		//endPos.line = 0;
		//endPos.pos = 0;
		startPos = endPos = 0;
		cached = false;
	}

	~BaseNode()
	{
		for (auto &it : cmdParam)
		{
			delete it;
		}
		for (auto &it : cmdValue)
		{
			delete it;
		}
	}

	BaseNode *parent;

	//list<list<BaseNode*>::iterator>::iterator reflect;

	QString name;
	QString comment;

	//Pos startPos, endPos;
	int startPos, endPos;

	bool cached;

	QVector<BaseNode*> cmdParam;
	QVector<BaseNode*> cmdValue;

	bool isCmdLevel() const
	{
		return type == Node_Command || type == Node_AtCommand || type == Node_Text || type == Node_Parser || type == Node_Comment || type == Node_LineComment;
	}

	bool isLabel() const
	{
		return type == Node_Label;
	}

	bool isCommand() const
	{
		return type == Node_Command || type == Node_AtCommand;
	}

	bool isExp() const
	{
		return type == Node_Parser || type == Node_LineParser;
	}

	BaseNode* findIndex(const QString &name, int defaultpos = -1)
	{
		for (int i = 0; i < cmdParam.size(); i++)
		{
			if (cmdParam[i] && cmdParam[i]->name == name)
				return cmdValue[i];
		}
		if (defaultpos >= 0 && defaultpos < cmdParam.size())
			return cmdValue[defaultpos];
		return NULL;
	}

	BaseNode* findChild(/*Pos*/int relativepos)
	{
		for (auto &it : cmdParam)
		{
			if (it && it->startPos <= relativepos && it->endPos >= relativepos)
				return it;
		}
		for (auto &it : cmdValue)
		{
			if (it && it->startPos <= relativepos && it->endPos >= relativepos)
				return it;
		}
		return NULL;
	}
};


class ParseData
{
public:
	ParseData(QByteArray &file, BKE_VarClosure *clo);

	~ParseData()
	{
		delete[] textbuf;
		fileclo->clear();
		fileclo->release();
		for (auto &it : fileNodes)
			delete it;
	}

	//BkeScintilla *scifile;

	QMap<int, BaseNode*> fileNodes;

	QList<QMap<int, BaseNode*>::iterator> labels;

	QStringList getLabels();

	//void insertChars(Pos p, Pos offset);

	//void deleteChars(Pos p, Pos offset);

	//void reMake(int start, int len, unsigned int &outstart, int &outlen, list<BaseNode*>::iterator &iter);

	int findLabel(const QString &l);

	//void refreshLabel();
	
	BaseNode *findNode(/*Pos p*/int pos);

	bool checkLabel(BaseNode *node);
	bool checkCommand(BaseNode *node, bool startwithat);
	bool checkCmd(BaseNode *node);

	QString lastComment;

	char fetchNextOne();
	char getNextOne();
	void skipLineComment();
	bool skipBlockComment();
	void skipText();
	bool skipLineEnd();
	bool isLineEnd();
	QString readCmdName(bool startwithat);
	QString readName();
	QString readValue(bool startwithat);
	bool skipSpace();

	BKE_VarClosure *fileclo;

	struct Info
	{
		int type;//2(error) or 3(warning)
		int value;
		int from;
		int len;

		Info(int a, int b, int c, int d) :type(a), value(b), from(c), len(d){};
		Info() {}
	};
	
	/// <summary>
	/// Warning and Error infos corresponding to syntax.
	/// </summary>
	QVector<Info> infos;

	/// <summary>
	/// Warning and Error infos corresponding to macros and commands.
	/// </summary>
	QVector<Info> infos2;

	QMutex infos2_mutex;

	//for parse
	int idx = 0;
	
	bool refresh;

	char *textbuf;

	bool isLineStart;

	bool Parse();
};

/// <summary>
/// Parser Analysis Module
/// </summary>
class PAModule : private Parser
{
private:
	std::wstring expstr;
	Parser *p;
	BKE_bytree *restree;
	BKE_Variable res;
	bool constvar;

	const wchar_t* curpos;

	//for ineer lexer
	bool MatchFunc(const wchar_t *a, const wchar_t **c);
	virtual void readToken();
	virtual void expression(BKE_bytree** tree, int rbp = 0);
	void skipToNextSentence();

	BKE_Variable *tmpvar;
	BKE_VarClosure *top;
	BKE_Variable topvar;
	BKE_Variable posvar;	//used by analysisToPos
	void _analysisToClosure(BKE_bytree *tr, BKE_VarClosure *clo, BKE_Variable *var);
	bool _analysisToPos(BKE_bytree *tr, BKE_VarClosure *clo, int pos, BKE_Variable *var);

public:
	~PAModule()
	{
		if (restree)
			restree->release();
	}

	PAModule(const QString &str);

	BKE_bytree *getTree()
	{
		return restree;
	}

	BKE_Variable getValue(bool *success)
	{
		if (success)
			*success = constvar;
		return res;
	}

	QString getStringValue(bool *success)
	{
		if (success)
			*success = constvar && res.getType() == VAR_STR;
		return QString::fromStdWString(res.forceAsString());
	}

	int getIntValue(bool *success)
	{
		if (success)
			*success = constvar && res.getType() == VAR_NUM;
		return res.forceAsInteger();
	}

	void analysisToClosure(BKE_VarClosure *clo);

	BKE_Variable analysisToPos(BKE_VarClosure *clo, int pos);
};

inline bool isVarName(const QString &s)
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

inline void getAllMembers(BKE_VarClass *cla, set<QString> &params)
{
	for (int i = 0; i < cla->parents.size(); i++)
		getAllMembers(cla->parents[i], params);
	for (auto &it : cla->varmap)
	{
		if (it.second.getType() == VAR_FUNC)
			params.insert(QString::fromStdWString(it.first.getConstStr() + L"()?3"));
		else
			params.insert(QString::fromStdWString(it.first.getConstStr() + L"?0"));
	}
}

inline void getAllMembers(BKE_VarClosure *clo, set<QString> &params)
{
	if (clo->parent)
		getAllMembers(clo->parent, params);
	for (auto &it : clo->varmap)
	{
		if (it.second.getType() == VAR_FUNC)
			params.insert(QString::fromStdWString(it.first.getConstStr() + L"()?3"));
		else
			params.insert(QString::fromStdWString(it.first.getConstStr() + L"?0"));
	}
}
