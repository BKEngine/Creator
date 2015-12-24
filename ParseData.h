#pragma once

#include <list>
#include <vector>
#include <set>
#include "ParserHelper\parser\parser.h"

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
		fileclo->release();
		for (auto &it : fileNodes)
			delete it;
	}

	//BkeScintilla *scifile;

	QMap<int, BaseNode*> fileNodes;

	list<QMap<int, BaseNode*>::iterator> labels;

	void getLabels(set<QString> &l);

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

	char fetchNextOne()
	{
		char ch = textbuf[idx];
		if (!ch)
			return 0;
		char ch2 = textbuf[idx + 1];
		if (ch == '/' && ch2 == '/')
		{
			int rawidx = idx + 2;
			skipLineComment();
			lastComment = QString::fromStdString(string(textbuf + rawidx, idx - rawidx));
			return textbuf[idx];
		}
		else if (ch == '/' && ch2 == '*')
		{
			int rawidx = idx + 2;
			skipBlockComment();
			lastComment = QString::fromStdString(string(textbuf + rawidx, idx - 2 - rawidx));
			return textbuf[idx];
		}
		return ch;
	}
	char getNextOne()
	{
		char ch = fetchNextOne();
		idx++;
		return ch;
	}
	void skipLineComment()
	{
		char ch = textbuf[idx];
		while (ch && ch!='\n' && ch!= '\r')
			ch = textbuf[++idx];
	}
	void skipBlockComment()
	{
		char ch = textbuf[++idx];
		bool s = false;
		while (ch)
		{
			if (ch != '*')
			{
				ch = textbuf[++idx];
				continue;
			}
			else
			{
				ch = textbuf[++idx];
				if (ch != '/')
				{
					continue;
				}
				s = true;
				break;
			}
		}
		if (!s)
		{
			infos.emplace_back(3, 9, idx - 1, 1);
		}
		idx++;
	}
	void skipText()
	{
		char ch = fetchNextOne();
		while (ch && ch != '\n' && ch != '\r' && ch != '[')
		{
			idx++;
			ch = fetchNextOne();
		}
	}
	bool skipLineEnd()
	{
		if (!idx)
			return true;
		char ch = textbuf[idx];
		if (ch == '\n')
		{
			idx++;
			return true;
		}
		else if (ch == '\r')
		{
			idx++;
			ch = textbuf[idx];
			if (ch == '\n')
			{
				idx++;
				return true;
			}
			return true;
		}
		return false;
	}
	bool isLineEnd()
	{
		char ch = textbuf[idx];
		return !ch || ch == '\n' || ch == '\r';
	}
	QString readCmdName(bool startwithat)
	{
		QByteArray ba;
		char ch = fetchNextOne();
		while (ch)
		{
			if (ISSPACE(ch) || ch == '=')
				break;
			if (ch == ']' && !startwithat)
				break;
			ba.push_back(ch);
			idx++;
			ch = fetchNextOne();
		}
		QString tmp;
		tmp.prepend(ba);
		return tmp;
	}
	QString readName()
	{
		QByteArray ba;
		unsigned char ch = fetchNextOne();
		if (isalpha(ch) || ch == '_' || ch >= 0x80)
		{
			do
			{
				ba.push_back(ch);
				idx++;
				ch = fetchNextOne();
			} while (isalnum(ch) || ch == '_' || ch >= 0x80);
		}
		QString tmp;
		tmp.prepend(ba);
		return tmp;
	}
	QString readValue(bool startwithat)
	{
		enum BracketType
		{
			Bracket_Small,
			Bracket_Medium,
			Bracket_Large
		};
		vector<BracketType> _stack;
		vector<int> posstack;
		posstack.push_back(idx);

		QByteArray ba;

		int yinhao = 0;

		unsigned char ch = (unsigned char)textbuf[idx];

		while (ch)
		{
			if (ch == ']' && !startwithat && !yinhao && _stack.empty())
				break;
			if (ch == '\r' && ch == '\n' && _stack.empty())
				break;
			if (ISSPACE(ch) && !yinhao && _stack.empty())
				break;
			if (ch == '/' && textbuf[idx + 1] == '/' && !yinhao)
			{
				skipLineComment();
				continue;
			}
			if (ch == '/' && textbuf[idx + 1] == '*' && !yinhao)
			{
				skipBlockComment();
				continue;
			}
			ba.push_back(ch);
			if (ch == '\\')
			{
				if (yinhao == 2)
				{
					idx++;
					ba.push_back(textbuf[idx]);
				}
			}
			else if (ch == '\"')
			{
				if (!yinhao)
				{
					yinhao = 1;
					posstack.push_back(idx);
				}
				else if (yinhao == 1)
				{
					if (textbuf[idx + 1] == '\"')
						ba.push_back(textbuf[++idx]);
					else
					{
						yinhao = 0;
						posstack.pop_back();
					}
				}
			}
			else if (ch == '\'')
			{
				if (!yinhao)
				{
					yinhao = 2;
					posstack.push_back(idx);
				}
				else if (yinhao == 2)
				{
					yinhao = 0;
					posstack.pop_back();
				}
			}
			else if (ch == '(' && !yinhao)
			{
				_stack.push_back(BracketType::Bracket_Small);
				posstack.push_back(idx);
			}
			else if (ch == '[' && !yinhao)
			{
				_stack.push_back(BracketType::Bracket_Medium);
				posstack.push_back(idx);
			}
			else if (ch == '{' && !yinhao)
			{
				_stack.push_back(BracketType::Bracket_Large);
				posstack.push_back(idx);
			}
			else if (ch == ')' && !yinhao)
			{
				if (!_stack.empty() && _stack.back() == BracketType::Bracket_Small)
				{
					_stack.pop_back();
					posstack.pop_back();
				}
				else
				{
					if (_stack.empty())
					{
						//缺少对应的(
						infos.emplace_back(2, 3, idx, 1);
					}
					else
					{
						//此处应为其它括号
						infos.emplace_back(2, 6 + _stack.back(), idx, 1);
					}
				}
			}
			else if (ch == ']' && !yinhao)
			{
				if (!_stack.empty() && _stack.back() == BracketType::Bracket_Medium)
				{
					_stack.pop_back();
					posstack.pop_back();
				}
				else
				{
					if (_stack.empty())
					{
						//缺少对应的[
						infos.emplace_back(2, 4, idx, 1);
					}
					else
					{
						//此处应为其它括号
						infos.emplace_back(2, 6 + _stack.back(), idx, 1);
					}
				}
			}
			else if (ch == '}' && !yinhao)
			{
				if (!_stack.empty() && _stack.back() == BracketType::Bracket_Large)
				{
					_stack.pop_back();
					posstack.pop_back();
				}
				else
				{
					if (_stack.empty())
					{
						//缺少对应的{
						infos.emplace_back(2, 5, idx, 1);
					}
					else
					{
						//此处应为其它括号
						infos.emplace_back(2, 6 + _stack.back(), idx, 1);
					}
				}
			}
			else if (ch == ';' && !yinhao)
			{
				if (!_stack.empty())
				{
					//此处应为其它括号
					if (_stack.empty())
					{
						//缺少对应的(
						infos.emplace_back(2, 2, idx, 1);
					}
					else
					{
						//此处应为其它括号
						infos.emplace_back(2, 6 + _stack.back(), idx, 1);
					}
					_stack.clear();
				}
			}
			ch = textbuf[++idx];

		}
		if (yinhao)
		{
			//字符串未完结
			infos.emplace_back(2, 9, posstack.back(), idx - posstack.back());
		}
		else if (!_stack.empty())
		{
			//此处应为其它括号
			infos.emplace_back(2, 6 + _stack.back(), posstack.back(), idx - posstack.back());
			_stack.clear();
		}
		QString tmp;
		tmp.prepend(ba);
		return tmp;
	}
	bool skipSpace()
	{
		char ch = fetchNextOne();
		bool res = false;
		while (ch && ch != '\r' && ch != '\n' && ISSPACE(ch))
		{
			res = true;
			idx++;
			ch = fetchNextOne();
		}
		return res;
	}

	BKE_VarClosure *fileclo;

	struct Info
	{
		int type;//2(error) or 3(warning)
		int value;
		int from;
		int len;

		Info(int a, int b, int c, int d) :type(a), value(b), from(c), len(d){};
	};
	
	/// <summary>
	/// Warning and Error infos corresponding to syntax.
	/// </summary>
	vector<Info> infos;

	/// <summary>
	/// Warning and Error infos corresponding to macros and commands.
	/// </summary>
	vector<Info> infos2;

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