#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"
#include "OptionSet.h"
#include "SparseState.h"

#include "../loli/loli_island.h"

#include "../BKS_info.h"

#include "../ParseData.h"

#define BASE_MASK 63
#define BEGAL_MASK (1<<6)
//in @ or [ command
#define CMD_MASK (1<<7)

class BKE_Lexer :public ILexer
{
private:
	//what do we need for a perfect highlight?

	LexAccessor *myAccessor;

	StyleContext/*BKE_Accessor*/ *styler;

	BKE_Info *info;

	ParseData *pdata;
	BaseNode *curnode;

	list<BaseNode*>::iterator cur_it;

	unsigned char cur_mask;

	bool firstLex;

	int start, end;
	unsigned char startStyle;

	IDocument *myDoc;

	//something for fold
	int lineCurrent;
	int firstLine;
	int levelPrev;	//实际是当前行的level
	int levelCurrent;	//实际是下一行的level

	//下一行增加缩进；当前行减少缩进
	//else和elseif指令为同时具有这两种效果
	QSet<QString> nextIndent, currentDeIndent, stopAndStart;

	class BracketsStack
	{
	public:
		enum BracketType
		{
			Parenthesis, //小括号
			Bracket, //中括号
			Brace, //大括号
		};
		typedef vector<pair<int, BracketType>>::iterator iterator;
	private:
		vector<pair<int, BracketType>> _stack;
	public:
		void pushParenthesis(int pos){ _stack.push_back(make_pair(pos, Parenthesis)); } //加入小括号
		void pushBracket(int pos){ _stack.push_back(make_pair(pos, Bracket)); }; //加入中括号
		void pushBrace(int pos){ _stack.push_back(make_pair(pos, Brace)); }; ////加入大括号

		bool has(BracketType type)
		{
			for (auto it = begin(); it != end(); ++it)
			{
				if (it->second == type)
					return true;
			}
			return false;
		}
		iterator find(BracketType type)
		{
			for (auto it = begin(); it != end(); ++it)
			{
				if (it->second == type)
					return it;
			}
			return end();
		}
		bool hasParenthesis(){ return has(Parenthesis); }
		bool hasBracket(){ return has(Bracket); }
		bool hasBrace(){ return has(Brace); }

		iterator findParenthesis(){ return find(Parenthesis); }
		iterator findBracket(){ return find(Bracket); }
		iterator findBrace(){ return find(Brace); }

		pair<int, BracketType> back(){ return _stack.back(); }
		bool empty(){ return _stack.empty(); }
		int backPos(){ return _stack.back().first; }
		BracketType backType(){ return _stack.back().second; }

		void pop(){ _stack.pop_back(); }

		iterator begin(){ return _stack.begin(); }
		iterator end(){ return _stack.end(); }
		iterator erase(iterator it){ return _stack.erase(it); }
	};

	void handleBracketsError(BracketsStack &stack);

	void handleFold()
	{
		int lev = levelPrev;
		if (levelCurrent > levelPrev) {
			lev |= SC_FOLDLEVELHEADERFLAG;
		};
		if (lev != myAccessor->LevelAt(lineCurrent)) {
			myAccessor->SetLevel(lineCurrent, lev);
		};
		lineCurrent++;
		levelPrev = levelCurrent;
	}

	void stopAndStartNewBlock()
	{
		if (levelPrev == SC_FOLDLEVELBASE || lineCurrent == firstLine)
		{
			levelCurrent++;
		}
		else
		{
			levelPrev--;
		}
	}

	void stopBlock()
	{
		if (levelPrev == SC_FOLDLEVELBASE || lineCurrent == firstLine)
		{
			//donothing
		}
		else
		{
			levelPrev--;
			levelCurrent = levelPrev;
		}
	}

public:
	BKE_Lexer()
	{
		myAccessor = NULL;
		styler = NULL;
		info = NULL;
		firstLex = false;	//反正第一次是一下高亮完的
		cur_mask = 0;
		pdata = NULL;
		curnode = NULL;

		nextIndent.insert("if");
		nextIndent.insert("for");

		currentDeIndent.insert("endif");
		currentDeIndent.insert("next");

		stopAndStart.insert("else");
		stopAndStart.insert("elseif");
	}

	virtual int SCI_METHOD Version() const
	{
		return lvOriginal; 
	};
	virtual void SCI_METHOD Release()
	{
		//deconstruct
		delete this;
	}
	virtual const char * SCI_METHOD PropertyNames()
	{
		return "bke";
	}
	virtual int SCI_METHOD PropertyType(const char *name)
	{
		//hahahaha, we use this to return prop value of Lexer
		if (!strcmp(name, "start"))
			return start;
		if (!strcmp(name, "end"))
			return end;
		if (!strcmp(name, "startstyle"))
			return startStyle;
		return 0;
	}
	virtual const char * SCI_METHOD DescribeProperty(const char *name)
	{
		return "bke lexer";
	}
	virtual int SCI_METHOD PropertySet(const char *key, const char *val)
	{
		//do nothing
		return -1;
	}
	virtual const char * SCI_METHOD DescribeWordListSets()
	{
		return "";
	}
	virtual int SCI_METHOD WordListSet(int n, const char *wl)
	{
		return -1;
	}
	virtual void SCI_METHOD Lex(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess);
	virtual void SCI_METHOD Fold(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess);
	virtual void * SCI_METHOD PrivateCall(int operation, void *pointer)
	{
		//do nothing
		switch (operation)
		{
		case 0:
			info = (BKE_Info*)pointer;
			break;
		case 1:
			pdata = (ParseData*)pointer;
			//cur_it = pdata->fileNodes.begin();
			break;
		}
		return NULL;
	}

	static ILexer * LexerFactory()
	{
		return new BKE_Lexer();
		//auto lex = new BKE_Lexer();
		//lex->PrivateCall(0, &global_bke_info);
		//return lex;
	}

private:
	int last_state;

	void setMask(char m)
	{
		cur_mask |= m;
	}
	void removeMask(int m)
	{
		cur_mask &= ~m;
	}

	bool atComment()
	{
		return styler->ch == '/' && (styler->chNext == '/' || styler->chNext == '*');
	}
	bool atLineComment()
	{
		return styler->ch == '/' && styler->chNext == '/';
	}
	bool atBlockComment()
	{
		return styler->ch == '/' && styler->chNext == '*';
	}
	bool isHex(int c)
	{
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
	}

	void JudgeStyle();
	void ContinueText();
	void ContinueBlockComment();
	void ContinueLineComment();
	void ContinueLabel();
	void ContinueLabelInParser();
	void DoAtCommand();
	void DoCommand();
	void ParseBegal(bool ignoreLineEnd, bool ignoreSpace, bool atCommand);	//true, true for ##, false, true for #, false, false for prop-value, the last true/false means @ or [

	bool ParseString();
	bool ParseString2();	//'
	bool ParseNumber();
	bool ParseColor();
	bool ParseVarname(bool forceVariable = false);
};

bool BKE_Lexer::ParseVarname(bool forceVariable)
{
	QByteArray qba;
	qba.append(styler->chPrev);
	while (styler->More())
	{
		if (isalnum(styler->ch) || styler->ch == '_' || styler->ch >= 0x80)
		{
			qba.append(styler->ch);
			styler->Forward();
		}
		else
			break;
	}
	if (!forceVariable && info->BagelWords.contains(QString::fromUtf8(qba)))
	{
		styler->SetState(last_state | cur_mask);
	}
	else
	{
		styler->ChangeState(SCE_BKE_PARSER_VAR | cur_mask);
		styler->SetState(last_state | cur_mask);
	}
	return true;
}

bool BKE_Lexer::ParseColor()
{
	int c = 0;
	while (styler->More())
	{
		if (isHex(styler->ch))
		{
			c++;
			styler->Forward();
		}
		else
			break;
	}
	if (c == 3 || c == 4 || c == 6 || c == 8)
	{
		styler->SetState(last_state | cur_mask);
		return true;
	}
	else
	{
		styler->ChangeState(SCE_BKE_ERROR | cur_mask);
		styler->SetState(last_state | cur_mask);
		return false;
	}
}

bool BKE_Lexer::ParseNumber()
{
	bool hex = false;
	if (styler->chPrev == '0' && tolower(styler->ch) == 'x')
	{
		styler->Forward();
		hex = true;
	}
	if (hex)
	{
		//hex
		while (styler->More())
		{
			if (isHex(styler->ch))
				styler->Forward();
			else
				break;
		}
		//judge .
		if (styler->ch == '.')
		{
			styler->Forward();
			while (styler->More())
			{
				if (isHex(styler->ch))
					styler->Forward();
				else
					break;
			}
		}
		//judge p
		if (styler->ch == 'p' || styler->ch == 'P')
		{
			styler->Forward();
			if (styler->ch == '+' || styler->ch == '-')
				styler->Forward();
			while (styler->More())
			{
				if (styler->ch >= '0' && styler->ch <= '9')
					styler->Forward();
				else
					break;
			}
		}
		//over
		styler->SetState(last_state | cur_mask);
		return true;
	}
	else
	{
		//digit
		while (styler->More())
		{
			if (styler->ch >= '0' && styler->ch <= '9')
				styler->Forward();
			else
				break;
		}
		//judge .
		if (styler->ch == '.')
		{
			styler->Forward();
			while (styler->More())
			{
				if (styler->ch >= '0' && styler->ch <= '9')
					styler->Forward();
				else
					break;
			}
		}
		//judge e
		if (styler->ch == 'e' || styler->ch == 'E')
		{
			styler->Forward();
			if (styler->ch == '+' || styler->ch == '-')
				styler->Forward();
			while (styler->More())
			{
				if (styler->ch >= '0' && styler->ch <= '9')
					styler->Forward();
				else
					break;
			}
		}
		//over
		styler->SetState(last_state | cur_mask);
		return true;
	}
}

bool BKE_Lexer::ParseString()
{
	while (styler->More() && !styler->atLineEnd)
	{
		if (styler->ch == '\"')
		{
			if(styler->chNext != '\"')
			{
				styler->Forward();
				styler->SetState(last_state | cur_mask);
				return true;
			}
			else
			{
				styler->Forward();
				styler->Forward();
			}
		}
		else
		{
			styler->Forward();
		}
	}
	styler->ChangeState(SCE_BKE_ERROR | cur_mask);
	styler->SetState(last_state | cur_mask);
	return false;
}

bool BKE_Lexer::ParseString2()
{
	struct Trans
	{
		int begin;
		int len;
		bool correct;
	};
	std::vector<Trans> trans_info;
	int pos = styler->currentPos;
	int realpos = pos;
	unsigned char ch = styler->ch;
	bool intrans = false;
	realpos++;
	while (ch && ch != '\r' && ch != '\n')
	{
		if (!intrans && ch == '\'')
		{
			//color all
			if (trans_info.empty())
			{
				styler->ForwardBytes(realpos - pos);
				styler->SetState(last_state | cur_mask);
			}
			else
			{
				//int pos2 = styler->currentPos + 1;
				//styler->setPos(pos);
				auto it = trans_info.begin();
				while (it != trans_info.end())
				{
					styler->ForwardBytes(it->begin - styler->currentPos);
					styler->SetState(it->correct ? SCE_BKE_TRANS | cur_mask : SCE_BKE_ERROR | cur_mask);
					styler->ForwardBytes(it->len);
					styler->SetState(SCE_BKE_STRING2 | cur_mask);

					//styler->setPos(it->begin);
					//styler->SetState(it->correct ? SCE_BKE_TRANS | cur_mask : SCE_BKE_ERROR | cur_mask);
					//styler->setPos(it->begin + it->len);
					//styler->SetState(SCE_BKE_STRING2 | cur_mask);
					it++;
				}
				//styler->setPos(pos2);
				styler->ForwardBytes(realpos - styler->currentPos);
				styler->SetState(last_state | cur_mask);
			}
			return true;
		}
		if (!intrans && ch == '\\')
		{
			intrans = true;
			trans_info.push_back(Trans());
			trans_info.back().begin = realpos - 1;
			ch = styler->GetRelative(realpos - pos);
			realpos++;
			continue;
		}
		if (intrans)
		{
			trans_info.back().len = 2;
			trans_info.back().correct = true;
			intrans = false;
			switch (ch)
			{
			case L'n':
			case L'r':
			case L't':
			case L'a':
			case L'b':
			case L'f':
			case L'v':
			case '\"':
			case '\'':
				ch = styler->GetRelative(realpos - pos);
				realpos++;
				break;
			case 'o':
				//need two more num
				ch = styler->GetRelative(realpos - pos);
				realpos++;
				if (ch >= '0' && ch < '8')
				{
					ch = styler->GetRelative(realpos - pos);
					realpos++;
					trans_info.back().len++;
				}
				if (ch >= '0' && ch < '8')
				{
					ch = styler->GetRelative(realpos - pos);
					realpos++;
					trans_info.back().len++;
				}
				break;
			case 'x':
				//need three more num
				ch = styler->GetRelative(realpos - pos);
				realpos++;
				if (isHex(ch))
				{
					ch = styler->GetRelative(realpos - pos);
					realpos++;
					trans_info.back().len++;
				}
				if (isHex(ch))
				{
					ch = styler->GetRelative(realpos - pos);
					realpos++;
					trans_info.back().len++;
				}
				if (isHex(ch))
				{
					ch = styler->GetRelative(realpos - pos);
					realpos++;
					trans_info.back().len++;
				}
				break;
			default:
				trans_info.back().correct = false;
				break;
			}
			continue;
		}
		ch = styler->GetRelative(realpos - pos);
		realpos++;
	}
	styler->ChangeState(SCE_BKE_ERROR | cur_mask);
	styler->SetState(last_state | cur_mask);
	return false;
}

void BKE_Lexer::handleBracketsError(BracketsStack &stack)
{
	for (auto it = stack.begin(); it != stack.end(); ++it)
	{
		myDoc->SetSingleStyles(it->first, 1, SCE_BKE_ERROR | cur_mask);
	}
}

void BKE_Lexer::ParseBegal(bool ignoreLineEnd, bool ignoreSpace, bool atCommand)
{
	BracketsStack stack; //栈
	bool lastOpIsDot = false;
	bool nextIsLed = false;	//下一个是不是双目运算符
	char lastOp = 0;
	int defaultState;
	if (ignoreLineEnd)
		defaultState = SCE_BKE_PARSER_DEFAULT;
	else if (ignoreSpace)
		defaultState = SCE_BKE_PARSER;
	else
		defaultState = SCE_BKE_DEFAULT;
	while (styler->More())
	{
		//judge end
		if (styler->atLineEnd && !ignoreLineEnd)
		{
			styler->SetState(SCE_BKE_DEFAULT | cur_mask); //加上这句的原因是，handleBracketsError中最后一个（当前）的括号会被之后的setstate覆盖掉……
			if (!stack.empty())
			{
				handleBracketsError(stack);
			}
			removeMask(BEGAL_MASK);
			styler->SetState(SCE_BKE_DEFAULT | cur_mask);
			break;
		}
		if (isspace(styler->ch) && !ignoreSpace && stack.empty())
		{
			removeMask(BEGAL_MASK);
			styler->SetState(SCE_BKE_DEFAULT | cur_mask);
			break;
		}
		if (styler->atLineEnd)
		{
			handleFold();
			styler->Forward();
			continue;
		}
		if (styler->ch == ']' && !atCommand && !stack.hasBracket())
		{
			styler->SetState(SCE_BKE_DEFAULT | cur_mask);
			if (!stack.empty())
			{
				handleBracketsError(stack);
			}
			removeMask(BEGAL_MASK);
			styler->SetState(SCE_BKE_DEFAULT | cur_mask);
			break;
		}
		if (styler->atLineStart && styler->ch == '#' && styler->chNext == '#')
		{
			int p = styler->currentPos;
			styler->Forward();
			styler->Forward();
			removeMask(BEGAL_MASK);
			styler->SetState(SCE_BKE_DEFAULT | cur_mask);
			bool error = false;
			while (!styler->atLineEnd && styler->More())
			{
				if (isspace(styler->ch))
					styler->Forward();
				else
				{
					styler->SetState(SCE_BKE_ERROR | cur_mask);
					styler->Forward();
					error = true;
				}
			}
			if (error)
			{
				styler->SetState(SCE_BKE_DEFAULT | cur_mask);
			}
			break;
		}
		last_state = styler->state & BASE_MASK;
		switch (styler->ch)
		{
		case '\"':
			styler->SetState(SCE_BKE_STRING | cur_mask);
			styler->Forward();
			ParseString();
			nextIsLed = true;
			break;
		case '\'':
			styler->SetState(SCE_BKE_STRING2 | cur_mask);
			styler->Forward();
			ParseString2();
			nextIsLed = true;
			break;
		case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
			styler->SetState(SCE_BKE_NUMBER | cur_mask);
			styler->Forward();
			ParseNumber();
			nextIsLed = true;
			break;
		case '#':
			styler->SetState(SCE_BKE_COLOR | cur_mask);
			styler->Forward();
			ParseColor();
			nextIsLed = true;
			break;
		//operator and comment
		case '/':
			if (styler->chNext == '/')
			{
				styler->SetState(SCE_BKE_ANNOTATE | cur_mask);
				styler->Forward();
				styler->Forward();
				ContinueLineComment();
				styler->SetState(defaultState | cur_mask);
				break;
			}
			else if (styler->chNext == '*')
			{
				styler->SetState(SCE_BKE_COMMENT | cur_mask);
				styler->Forward();
				styler->Forward();
				ContinueBlockComment();
				break;
			}
			//no break, '/' is same as default
		default:
			if (info->OperatorAncestor.indexOf(styler->ch) >= 0)
			{
				if (styler->ch == ']' && (stack.empty() || stack.backType() != BracketsStack::Bracket))
				{
					styler->SetState(SCE_BKE_ERROR | cur_mask);
				}
				else if (styler->ch == '}' && (stack.empty() || stack.backType() != BracketsStack::Brace))
				{
					styler->SetState(SCE_BKE_ERROR | cur_mask);
				}
				else if (styler->ch == ')' && (stack.empty() || stack.backType() != BracketsStack::Parenthesis))
				{
					styler->SetState(SCE_BKE_ERROR | cur_mask);
				}
				else
				{
					styler->SetState(SCE_BKE_OPERATORS | cur_mask);
					switch (styler->ch)
					{
					case '[':
						stack.pushBracket(styler->currentPos);
						nextIsLed = false;
						break;
					case '{':
						stack.pushBrace(styler->currentPos);
						if (ignoreLineEnd)
							levelCurrent++;
						nextIsLed = false;
						break;
					case '(':
						stack.pushParenthesis(styler->currentPos);
						nextIsLed = false;
						break;
					case ']':
					case '}':
					case ')':
						//正确性已在上面检验过
						stack.pop();
						if (styler->ch == '}' && ignoreLineEnd)
							levelCurrent--;
						nextIsLed = true;
						break;
					case ';':
					case ',':
						nextIsLed = false;
						break;
					case '*':
						if (!nextIsLed)
						{
							styler->SetState(SCE_BKE_LABEL_IN_PARSER | cur_mask);
							ContinueLabelInParser();
							continue;
						}
						break;
					default:
						nextIsLed = false;
					}
					/*
					if (styler->ch == '[')
					{
						stack.pushBracket(styler->currentPos);
					}
					else if (styler->ch == '{')
					{
						stack.pushBrace(styler->currentPos);
						if (ignoreLineEnd)
							levelCurrent++;
					}
					else if (styler->ch == '(')
					{
						stack.pushParenthesis(styler->currentPos);
					}
					else if (styler->ch == ']' || styler->ch == '}' || styler->ch == ')')	//正确性已在上面检验过
					{
						stack.pop();
						if (styler->ch == '}' && ignoreLineEnd)
							levelCurrent--;
						nextIsLed = false;
					}
					else if (styler->ch == '*' && !nextIsLed)
					{

					}
					*/
					lastOp = styler->ch;
				}
				styler->Forward();
			}
			else if (isalpha(styler->ch) || styler->ch == '_' || styler->ch >= 0x80)
			{
				styler->SetState(SCE_BKE_PARSER_KEYWORD | cur_mask);
				styler->Forward();
				ParseVarname(lastOpIsDot);
				nextIsLed = true;
			}
			else if (isspace(styler->ch))
			{
				styler->SetState(defaultState | cur_mask);
				lastOp = 0;
				while (styler->More() && !styler->atLineEnd)
				{
					if (styler->ch == '\t')
					{
						if (styler->atLineStart)
						{
							styler->Forward();
							styler->atLineStart = true;
						}
						else
						{
							styler->Forward();
						}
					}
					else if (isspace(styler->ch))
						styler->Forward();
					else
						break;
				}
				continue;
			}
			else
			{
				//invalid symbol
				styler->SetState(SCE_BKE_ERROR | cur_mask);
				styler->Forward();
			}
		}
		//这里已经不会出现空字符，所以不是.就是其他有效字符
		if (styler->ch == '.')
			lastOpIsDot = true;
		else
			lastOpIsDot = false;
	}
	styler->SetState(SCE_BKE_DEFAULT | cur_mask);
}

void BKE_Lexer::DoCommand()
{
	//check cmd name
	setMask(CMD_MASK);
	styler->SetState(styler->state | cur_mask);
	//bool error = false;
	QString cmdName;
	if (!styler->atLineEnd && !isspace(styler->ch) && styler->ch != ']')
	{
		while (styler->More())
		{
			if (isspace(styler->ch) || styler->ch == ']')
			{
				break;
			}
			else if (atComment())
			{
				int rawstate = styler->state;
				JudgeStyle();
				if ((styler->state & BASE_MASK) == SCE_BKE_ANNOTATE)
					ContinueLineComment();
				else
					ContinueBlockComment();
				styler->SetState(rawstate);
				//removeMask(CMD_MASK);
				//styler->SetState(SCE_BKE_DEFAULT | cur_mask);
			}
			else
			{
				cmdName.push_back(styler->ch);
				styler->Forward();
			}
		}
	}
	//else
	//{
	//	error = true;
	//	有了indicator后不用在Lexer里画Error了
	//	styler->ChangeState(SCE_BKE_ERROR | cur_mask);
	//}
	styler->SetState(SCE_BKE_DEFAULT | cur_mask);
	//prop-value
	while (styler->More() && !styler->atLineEnd && styler->ch != ']')
	{
		if (isspace(styler->ch))
		{
			styler->Forward();
			continue;
		}
		styler->SetState(SCE_BKE_ATTRIBUTE | cur_mask);
		int startPos = 0;
		unsigned char ch = styler->ch;
		if (ch >= 0x80 || ch == '_' || isalpha(ch))
		{
			do
			{
				startPos++;
				ch = styler->GetRelative(startPos);
				if (!(ch >= 80 || ch == '_' || isalnum(ch)))
					break;
			} while (ch);
			if (ch == '=')
			{
				styler->ForwardBytes(startPos);
				styler->SetState(SCE_BKE_DEFAULT | cur_mask);
				styler->Forward();
			}
			else
			{
				//see as no attr
				styler->ChangeState(SCE_BKE_DEFAULT | cur_mask);
			}
		}
		else
		{
			//no attr
			styler->ChangeState(SCE_BKE_DEFAULT | cur_mask);
		}
		ParseBegal(false, false, false);
	}
	if (styler->ch == ']')
	{
		//if (error)
		//	styler->SetState(SCE_BKE_ERROR | cur_mask);
		//else
			styler->SetState(SCE_BKE_COMMAND2 | cur_mask);
		styler->Forward();
		removeMask(CMD_MASK);
		styler->SetState(SCE_BKE_DEFAULT | cur_mask);
	}
	else
	{
		removeMask(CMD_MASK);
	}
	//fold
	if (nextIndent.contains(cmdName))
	{
		levelCurrent++;
	}
	if (currentDeIndent.contains(cmdName))
	{
		stopBlock();
	}
	if (stopAndStart.contains(cmdName))
	{
		stopAndStartNewBlock();
	}
}

void BKE_Lexer::DoAtCommand()
{
	setMask(CMD_MASK);
	styler->SetState(styler->state | cur_mask);
	//check cmd name
	QString cmdName;
	if (!styler->atLineEnd && !isspace(styler->ch))
	{
		while (styler->More())
		{
			if (isspace(styler->ch))
			{
				break;
			}
			else if (atComment())
			{
				int rawstate = styler->state;
				JudgeStyle();
				if ((styler->state & BASE_MASK) == SCE_BKE_ANNOTATE)
					ContinueLineComment();
				else
					ContinueBlockComment();
				styler->SetState(rawstate);
				//removeMask(CMD_MASK);
				//styler->SetState(SCE_BKE_DEFAULT | cur_mask);
			}
			else
			{
				cmdName.push_back(styler->ch);
				styler->Forward();
			}
		}
	}
	//有了indicator后不用在Lexer里画Error了
	//else
	//{
	//	styler->ChangeState(SCE_BKE_ERROR | cur_mask);
	//}
	styler->SetState(SCE_BKE_DEFAULT | cur_mask);
	//prop-value
	while (styler->More() && !styler->atLineEnd)
	{
		if (isspace(styler->ch))
		{
			styler->Forward();
			continue;
		}
		styler->SetState(SCE_BKE_ATTRIBUTE | cur_mask);
		int startPos = 0;
		unsigned char ch = styler->ch;
		if (ch >= 0x80 || ch == '_' || isalpha(ch))
		{
			do
			{
				startPos++;
				ch = styler->GetRelative(startPos);
				if (!(ch >= 80 || ch == '_' || isalnum(ch)))
					break;
			} while (ch);
			if (ch == '=')
			{
				styler->ForwardBytes(startPos);
				styler->SetState(SCE_BKE_DEFAULT | cur_mask);
				styler->Forward();
			}
			else
			{
				//see as no attr
				styler->ChangeState(SCE_BKE_DEFAULT | cur_mask);
			}
		}
		else
		{
			//no attr
			styler->ChangeState(SCE_BKE_DEFAULT | cur_mask);
		}
		ParseBegal(false, false, true);
	}
	removeMask(CMD_MASK);
	//fold
	if (nextIndent.contains(cmdName))
	{
		levelCurrent++;
	}
	if (currentDeIndent.contains(cmdName))
	{
		stopBlock();
	}
	if (stopAndStart.contains(cmdName))
	{
		stopAndStartNewBlock();
	}
}

void BKE_Lexer::ContinueLabel()
{
	setMask(CMD_MASK);
	styler->SetState(SCE_BKE_LABEL_DEFINE | cur_mask);
	//QByteArray ba;
	bool sp = false;
	while (styler->More() && !styler->atLineEnd)
	{
		if (atLineComment())
			break;
		else if (atBlockComment())
		{
			last_state = styler->state & BASE_MASK;
			styler->SetState(SCE_BKE_COMMENT | cur_mask);
			styler->Forward();
			styler->Forward();
			ContinueBlockComment();
		}
		//if (!sp)
		//	ba.append(styler->ch);
		styler->Forward();
		//sp = isspace(styler->ch);
	}
	removeMask(CMD_MASK);
	styler->SetState(SCE_BKE_DEFAULT | cur_mask);
	//curnode->name.clear();
	//curnode->name.prepend(ba);
	//curnode->cached = true;

	stopAndStartNewBlock();
	return;
}

void BKE_Lexer::ContinueLabelInParser()
{
	//pass '*'
	styler->Forward();
	while (styler->More() && !styler->atLineEnd)
	{
		if (atLineComment())
			break;
		else if (atBlockComment())
		{
			last_state = styler->state & BASE_MASK;
			styler->SetState(SCE_BKE_COMMENT | cur_mask);
			styler->Forward();
			styler->Forward();
			ContinueBlockComment();
		}
		else if (isalnum(styler->ch) || styler->ch == '_' || styler->ch >= 0x80)
		{
			styler->Forward();
		}
		else
		{
			break;
		}
	}
	styler->SetState(SCE_BKE_DEFAULT | cur_mask);
}

void BKE_Lexer::ContinueBlockComment()
{
	while (styler->More())
	{
		if (styler->ch == '*' && styler->chNext == '/')
		{
			styler->Forward();
			styler->Forward();
			//styler->removeMaskState(COMMENT_MASK);
			styler->SetState(last_state | cur_mask);   //恢复原状态
			break;
		}
		if (styler->atLineEnd)
		{
			handleFold();
		}
		styler->Forward();
	}
	return;
}

void BKE_Lexer::ContinueLineComment()
{
	while (styler->More() && !styler->atLineEnd)
	{
		styler->Forward();
	}
	styler->SetState(SCE_BKE_DEFAULT | cur_mask);
}

void BKE_Lexer::JudgeStyle()
{
	//if (curnode)
	//{
	//	curnode->endPos.line = myDoc->LineFromPosition(styler->currentPos - 1);
	//	curnode->endPos.pos = styler->currentPos - 1 - myDoc->LineStart(curnode->endPos.line);
	//	auto iit = pdata->fileNodes.insert(cur_it, curnode);
	//	if (curnode->isLabel())
	//	{
	//		pdata->labels.push_front(iit);
	//		curnode->reflect = pdata->labels.begin();
	//	}
	//	pdata->checkCmd(curnode);
	//	curnode = NULL;
	//}
	if (styler->ch == '@')
	{
		//curnode = new BaseNode(NULL);
		//curnode->type = BaseNode::Node_AtCommand;
		//curnode->startPos.line = myDoc->LineFromPosition(styler->currentPos);
		//curnode->startPos.pos = styler->currentPos - myDoc->LineStart(curnode->startPos.line);
		styler->SetState(SCE_BKE_COMMAND | cur_mask);
	}
	else if (styler->ch == '[')
	{
		//curnode = new BaseNode(NULL);
		//curnode->type = BaseNode::Node_Command;
		//curnode->startPos.line = myDoc->LineFromPosition(styler->currentPos);
		//curnode->startPos.pos = styler->currentPos - myDoc->LineStart(curnode->startPos.line);
		styler->SetState(SCE_BKE_COMMAND2 | cur_mask);
	}
	else if (styler->ch == '/' && styler->chNext == '/')
	{
		//curnode = new BaseNode(NULL);
		//curnode->type = BaseNode::Node_LineComment;
		//curnode->startPos.line = myDoc->LineFromPosition(styler->currentPos);
		//curnode->startPos.pos = styler->currentPos - myDoc->LineStart(curnode->startPos.line);
		styler->SetState(SCE_BKE_ANNOTATE | cur_mask);
		styler->Forward();
	}
	else if (styler->ch == ';' && styler->atLineStart)
	{
		//curnode = new BaseNode(NULL);
		//curnode->type = BaseNode::Node_LineComment;
		//curnode->startPos.line = myDoc->LineFromPosition(styler->currentPos);
		//curnode->startPos.pos = styler->currentPos - myDoc->LineStart(curnode->startPos.line);
		styler->SetState(SCE_BKE_ANNOTATE | cur_mask);
	}
	else if (styler->ch == '/' && styler->chNext == '*')
	{
		//styler->setMaskState(COMMENT_MASK);
		//curnode = new BaseNode(NULL);
		//curnode->type = BaseNode::Node_Comment;
		//curnode->startPos.line = myDoc->LineFromPosition(styler->currentPos);
		//curnode->startPos.pos = styler->currentPos - myDoc->LineStart(curnode->startPos.line);
		last_state = styler->state;
		styler->SetState(SCE_BKE_COMMENT | cur_mask);
		styler->Forward();
	}
	else if (styler->ch == '*' && styler->atLineStart)
	{
		//curnode = new BaseNode(NULL);
		//curnode->type = BaseNode::Node_Label;
		//curnode->startPos.line = myDoc->LineFromPosition(styler->currentPos);
		//curnode->startPos.pos = styler->currentPos - myDoc->LineStart(curnode->startPos.line);
		styler->SetState(SCE_BKE_LABEL_DEFINE | cur_mask);
	}
	else if (styler->ch == '#')
	{
		if (styler->chNext == '#')
		{
			//curnode = new BaseNode(NULL);
			//curnode->type = BaseNode::Node_Parser;
			//curnode->startPos.line = myDoc->LineFromPosition(styler->currentPos);
			//curnode->startPos.pos = styler->currentPos - myDoc->LineStart(curnode->startPos.line);
			setMask(BEGAL_MASK);
			styler->SetState(SCE_BKE_PARSER_DEFAULT | cur_mask);
			styler->Forward();
		}
		else
		{
			//curnode = new BaseNode(NULL);
			//curnode->type = BaseNode::Node_LineParser;
			//curnode->startPos.line = myDoc->LineFromPosition(styler->currentPos);
			//curnode->startPos.pos = styler->currentPos - myDoc->LineStart(curnode->startPos.line);
			setMask(BEGAL_MASK);
			styler->SetState(SCE_BKE_PARSER | cur_mask);
		}
	}
	//leave string for Begal
	//else if (styler->ch == '\"')
	//{
	//	//styler->setMaskState(BEGAL_MASK);
	//	styler->SetState(SCE_BKE_STRING | cur_mask);
	//}
	//else if (styler->ch == '\'')
	//{
	//	//styler->setMaskState(BEGAL_MASK);
	//	styler->SetState(SCE_BKE_STRING2 | cur_mask);
	//}
	else if (styler->atLineEnd)
	{
		styler->SetState(SCE_BKE_DEFAULT | cur_mask);
		if (styler->ch == '\r' && styler->chNext == '\n')
			styler->Forward();
		handleFold();
	}
	else
	{
		//curnode = new BaseNode(NULL);
		//curnode->type = BaseNode::Node_Text;
		//curnode->startPos.line = myDoc->LineFromPosition(styler->currentPos);
		//curnode->startPos.pos = styler->currentPos - myDoc->LineStart(curnode->startPos.line);
		styler->SetState(SCE_BKE_TEXT | cur_mask);
	}
	bool stillstart = false;
	if (styler->atLineStart && styler->ch == '\t')
		stillstart = true;
	styler->Forward();
	if (!styler->atLineStart && stillstart)
	{
		styler->atLineStart = true;
		styler->ChangeState(SCE_BKE_DEFAULT | cur_mask);
	}
}

void BKE_Lexer::ContinueText()
{
	while (styler->More() && !styler->atLineEnd)
	{
		if (styler->ch == '[' || atComment())
			break;
		styler->Forward();
	}
	styler->SetState(SCE_BKE_DEFAULT);
}

void SCI_METHOD BKE_Lexer::Lex(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess)
{
	LexAccessor accessor(pAccess);
	myAccessor = &accessor;

	myDoc = pAccess;

	StyleContext sc(startPos, lengthDoc, initStyle, accessor, 0xFF);
	
	styler = &sc;
	cur_mask = initStyle & ~BASE_MASK;
	if (firstLex)
	{
		firstLex = false;
		//pdata->reMake(startPos, lengthDoc, startPos, lengthDoc, cur_it);

		unsigned char laststyle = (unsigned char)pAccess->StyleAt(startPos - 1);
		unsigned char curstyle = (unsigned char)pAccess->StyleAt(startPos);
		//Lex(startPos, lengthDoc, laststyle, pAccess);
		//return;

		//当前处于comment时，必须退回到comment之前进行解析，否则无法知道comment后接的是啥style
		if ((initStyle & 63) == SCE_BKE_COMMENT || (initStyle & 63) == SCE_BKE_ANNOTATE)
		{
			int endPos = startPos + lengthDoc;
			initStyle = curstyle;
			//search begin of cmd
			while (startPos-- > 0)
			{
				if ((initStyle & 63) == SCE_BKE_COMMENT || (initStyle & 63) == SCE_BKE_ANNOTATE)
				{
					initStyle = pAccess->StyleAt(startPos);
				}
				else
				{
					break;
				}
			}
			if (startPos == 0)
			{
				if ((initStyle & 63) == SCE_BKE_COMMENT || (initStyle & 63) == SCE_BKE_ANNOTATE)
				{
					laststyle = SCE_BKE_DEFAULT;
					Lex(0, endPos, SCE_BKE_DEFAULT, pAccess);
					return;
				}
			}
			laststyle = pAccess->StyleAt(startPos - 1);
			firstLex = true;
			Lex(startPos, endPos - startPos, pAccess->StyleAt(startPos - 1), pAccess);
			return;
		}

		if (initStyle & CMD_MASK)
		{
			int endPos = startPos + lengthDoc;
			//search begin of cmd
			while (startPos-- > 0)
			{
				if ((curstyle & CMD_MASK) && !(laststyle & CMD_MASK))
					break;
				curstyle = pAccess->StyleAt(startPos);
				laststyle = pAccess->StyleAt(startPos - 1);
			}
			Lex(startPos, endPos - startPos, pAccess->StyleAt(startPos - 1), pAccess);
			return;
		}

		if (cur_mask & BEGAL_MASK)
		{
			//## or #
			int endPos = startPos + lengthDoc;
			//search begin of cmd
			while (startPos-- > 0)
			{
				if ((curstyle & BEGAL_MASK) && !(laststyle & BEGAL_MASK))
					break;
				curstyle = pAccess->StyleAt(startPos);
				laststyle = pAccess->StyleAt(startPos - 1);
			}
			startPos++;
			Lex(startPos, endPos - startPos, pAccess->StyleAt(startPos - 1), pAccess);
			return;
		}
	}
	//styler->setRange(startPos, startPos + lengthDoc, initStyle);
	//curnode = NULL;

	//fold info
	lineCurrent = accessor.GetLine(startPos);
	firstLine = lineCurrent;
	levelPrev = accessor.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK;
	//qDebug("line:%d, level:%d", lineCurrent, levelCurrent);
	levelCurrent = levelPrev;

	while (styler->More())
	{
		switch (styler->state & BASE_MASK)
		{
		case SCE_BKE_DEFAULT:
			//judge next type
			JudgeStyle();
			break;
		case SCE_BKE_PARSER_DEFAULT:
			ParseBegal(true, true, true);
			break;
		case SCE_BKE_COMMAND:
			DoAtCommand();
			break;
		case SCE_BKE_COMMAND2:
			DoCommand();
			break;
		case SCE_BKE_LABEL_DEFINE:
			ContinueLabel();
			break;
		case SCE_BKE_ANNOTATE:
			ContinueLineComment();
			break;
		case SCE_BKE_COMMENT:
			ContinueBlockComment();
			break;
		case SCE_BKE_TEXT:
			ContinueText();
			break;
		case SCE_BKE_PARSER:
			ParseBegal(false, true, true);
			break;
		default:
			styler->Forward();
		}
	}

	//if (curnode)
	//{
	//	curnode->endPos.line = myDoc->LineFromPosition(styler->currentPos - 1);
	//	curnode->endPos.pos = styler->currentPos - 1 - myDoc->LineStart(curnode->endPos.line);
	//	auto iit = pdata->fileNodes.insert(cur_it, curnode);
	//	if (curnode->isLabel())
	//	{
	//		pdata->labels.push_front(iit);
	//		curnode->reflect = pdata->labels.begin();
	//	}
	//	curnode = NULL;
	//}

	//styler->continueStateToPos(startPos + lengthDoc);
	styler->Complete();
	firstLex = true;
	start = startPos;
	end = startPos + lengthDoc;
	startStyle = pAccess->StyleAt(startPos);

	//pdata->refreshLabel();
}

void SCI_METHOD BKE_Lexer::Fold(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess)
{
	//do nothing because we do all in Lex
	return;
}

#define SCLEX_BKE 108

LexerModule lm_BKE(SCLEX_BKE, BKE_Lexer::LexerFactory, "bke");
