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

#include "../paper/creator_parser.h"

#define BASE_MASK 0
#define BEGAL_MASK (1<<5)
//in @ or [ command
#define CMD_MASK (1<<6)
//block comment
#define COMMENT_MASK (1<<7)

class BKE_Lexer :public ILexer
{
private:
	//what do we need for a perfect highlight?

	LexAccessor *accessor;

	StyleContext/*BKE_Accessor*/ *styler;

	BKE_Info *info;
public:
	BKE_Lexer()
	{
		accessor = NULL;
		styler = NULL;
		info = NULL;
	}

	virtual int SCI_METHOD Version() const
	{
		return 1;
	};
	virtual void SCI_METHOD Release()
	{
		//deconstruct
	}
	virtual const char * SCI_METHOD PropertyNames()
	{
		return "bke";
	}
	virtual int SCI_METHOD PropertyType(const char *name)
	{
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
		}
		return NULL;
	}

	static ILexer * LexerFactory()
	{
		auto lex = new BKE_Lexer();
		lex->PrivateCall(0, &global_bke_info);
		return lex;
	}

private:
	int last_state;

	bool atComment()
	{
		return styler->ch == '/' && (styler->chNext == '/' || styler->chNext == '*');
	}
	bool isHex(char c)
	{
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
	}

	void JudgeStyle();
	void ContinueText();
	void ContinueBlockComment();
	void ContinueLineComment();
	void ContinueLabel();
	void DoAtCommand();
	void DoCommand();
	void ParseBegal(bool ignoreLineEnd, bool ignoreSpace, bool atCommand);	//true, true for ##, false, true for #, false, false for prop-value, the last true/false means @ or [

	bool ParseString();
	bool ParseString2();	//'
	bool ParseNumber();
	bool ParseColor();
	bool ParseVarname();
};

bool BKE_Lexer::ParseVarname()
{
	QString s(styler->chPrev);
	while (styler->More())
	{
		if (isalnum(styler->ch) || styler->ch == '_' || styler->ch >= 0x80)
		{
			s.append(styler->ch);
			styler->Forward();
		}
		else
			break;
	}
	if (info->BagelWords.contains(s))
	{
		styler->SetState(last_state);
	}
	else
	{
		styler->ChangeState(SCE_BKE_PARSER_VAR);
		styler->SetState(last_state);
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
		styler->SetState(last_state);
		return true;
	}
	else
	{
		styler->ChangeState(SCE_BKE_ERROR);
		styler->SetState(last_state);
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
		styler->SetState(last_state);
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
		styler->SetState(last_state);
		return true;
	}
}

bool BKE_Lexer::ParseString()
{
	while (styler->More() && !styler->atLineEnd)
	{
		if (styler->ch == '\"' && styler->chNext != '\"')
		{
			styler->Forward();
			styler->SetState(last_state);
			return true;
		}
		else
		{
			styler->Forward();
		}
	}
	styler->ChangeState(SCE_BKE_ERROR);
	styler->SetState(last_state);
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
	bool intrans = false;
	while (styler->More() && !styler->atLineEnd)
	{
		if (!intrans && styler->ch == '\'')
		{
			//color all
			if (trans_info.empty())
			{
				styler->Forward();
				styler->SetState(last_state);
			}
			else
			{
				int pos2 = styler->currentPos + 1;
				styler->setPos(pos);
				auto it = trans_info.begin();
				while (it != trans_info.end())
				{
					styler->setPos(it->begin);
					styler->SetState(it->correct ? SCE_BKE_TRANS : SCE_BKE_ERROR);
					styler->setPos(it->begin + it->len);
					styler->SetState(SCE_BKE_STRING2);
					it++;
				}
				styler->setPos(pos2);
				styler->SetState(last_state);
			}
			return true;
		}
		if (!intrans && styler->ch == '\\')
		{
			intrans = true;
			trans_info.push_back(Trans());
			trans_info.back().begin = styler->currentPos;
			styler->Forward();
			continue;
		}
		if (intrans)
		{
			trans_info.back().len = 2;
			trans_info.back().correct = true;
			intrans = false;
			switch (styler->ch)
			{
			case L'n':
			case L'r':
			case L't':
			case L'a':
			case L'b':
			case L'f':
			case L'v':
				styler->Forward();
				break;
			case 'o':
				//need two more num
				styler->Forward();
				if (styler->ch >= '0' && styler->ch < '8')
				{
					styler->Forward();
					trans_info.back().len++;
				}
				if (styler->ch >= '0' && styler->ch < '8')
				{
					styler->Forward();
					trans_info.back().len++;
				}
				break;
			case 'x':
				//need three more num
				styler->Forward();
				if (isHex(styler->ch))
				{
					styler->Forward();
					trans_info.back().len++;
				}
				if (isHex(styler->ch))
				{
					styler->Forward();
					trans_info.back().len++;
				}
				if (isHex(styler->ch))
				{
					styler->Forward();
					trans_info.back().len++;
				}
				break;
			default:
				trans_info.back().correct = false;
				break;
			}
			continue;
		}
		styler->Forward();
	}
	styler->ChangeState(SCE_BKE_ERROR);
	styler->SetState(last_state);
	return false;
}

void BKE_Lexer::ParseBegal(bool ignoreLineEnd, bool ignoreSpace, bool atCommand)
{
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
			styler->SetState(SCE_BKE_DEFAULT);
			break;
		}
		if (isspace(styler->ch) && !ignoreSpace)
		{
			styler->SetState(SCE_BKE_DEFAULT);
			break;
		}
		if (styler->ch == ']' && !atCommand)
		{
			styler->SetState(SCE_BKE_DEFAULT);
			break;
		}
		if (styler->atLineStart && styler->ch == '#' && styler->chNext == '#')
		{
			int p = styler->currentPos;
			styler->SetState(SCE_BKE_DEFAULT);
			styler->Forward();
			styler->Forward();
			bool end = true;
			while (!styler->atLineEnd)
			{
				if (isspace(styler->ch))
					styler->Forward();
				else
				{
					end = false;
					styler->setPos(p);
					break;
				}
			}
			if (end)
				break;
		}
		last_state = styler->state;
		switch (styler->ch)
		{
		case '\"':
			styler->SetState(SCE_BKE_STRING);
			styler->Forward();
			ParseString();
			break;
		case '\'':
			styler->SetState(SCE_BKE_STRING2);
			styler->Forward();
			ParseString2();
			break;
		case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:
			styler->SetState(SCE_BKE_NUMBER);
			styler->Forward();
			ParseNumber();
			break;
		case '#':
			styler->SetState(SCE_BKE_COLOR);
			styler->Forward();
			ParseColor();
			break;
		//operator and comment
		case '/':
			if (styler->chNext == '/')
			{
				styler->SetState(SCE_BKE_ANNOTATE);
				styler->Forward();
				styler->Forward();
				ContinueLineComment();
				styler->SetState(defaultState);
				break;
			}
			else if (styler->chNext == '*')
			{
				styler->SetState(SCE_BKE_COMMENT);
				styler->Forward();
				styler->Forward();
				ContinueBlockComment();
				break;
			}
		default:
			if (info->OperatorAncestor.indexOf(styler->ch) >= 0)
			{
				styler->SetState(SCE_BKE_OPERATORS);
				styler->Forward();
			}
			else if (isalpha(styler->ch) || styler->ch == '_' || styler->ch >= 0x80)
			{
				styler->SetState(SCE_BKE_PARSER_KEYWORD);
				styler->Forward();
				ParseVarname();
			}
			else if (isspace(styler->ch))
			{
				styler->SetState(defaultState);
				styler->Forward();
				while (styler->More())
				{
					if (isspace(styler->ch))
						styler->Forward();
					else
						break;
				}
			}
			else
			{
				styler->SetState(SCE_BKE_ERROR);
				styler->Forward();
			}
		}
	}
}

void BKE_Lexer::DoCommand()
{
	//check cmd name
	if (!styler->atLineEnd && !isspace(styler->ch) && styler->ch != ']')
	{
		while (styler->More())
		{
			if (isspace(styler->ch) || styler->ch == ']')
				break;
			else
				styler->Forward();
		}
	}
	else
	{
		styler->ChangeState(SCE_BKE_ERROR);
	}
	styler->SetState(SCE_BKE_DEFAULT);
	//prop-value
	while (styler->More() && !styler->atLineEnd && styler->ch != ']')
	{
		if (isspace(styler->ch))
		{
			styler->Forward();
			continue;
		}
		styler->SetState(SCE_BKE_ATTRIBUTE);
		int startPos = styler->currentPos;
		if (styler->ch >= 80 || styler->ch == '_' || isalpha(styler->ch))
		{
			while (styler->More())
			{
				if (styler->ch >= 80 || styler->ch == '_' || isalnum(styler->ch))
				{
					styler->Forward();
				}
				else
					break;
			}
			if (styler->ch == '=')
			{
				styler->ChangeState(SCE_BKE_DEFAULT);
				styler->Forward();
			}
			else
			{
				//see as no attr
				styler->ChangeState(SCE_BKE_DEFAULT);
				styler->setPos(startPos);
			}
		}
		else
		{
			//no attr
			styler->ChangeState(SCE_BKE_DEFAULT);
			styler->setPos(startPos);
		}
		ParseBegal(false, false, false);
	}
	if (styler->ch == ']')
	{
		styler->SetState(SCE_BKE_COMMAND);
		styler->Forward();
		styler->SetState(SCE_BKE_DEFAULT);
	}
}

void BKE_Lexer::DoAtCommand()
{
	//check cmd name
	if (!styler->atLineEnd && !isspace(styler->ch))
	{
		while (styler->More())
		{
			if (isspace(styler->ch))
				break;
			else
				styler->Forward();
		}
	}
	else
	{
		styler->ChangeState(SCE_BKE_ERROR);
	}
	styler->SetState(SCE_BKE_DEFAULT);
	//prop-value
	while (styler->More() && !styler->atLineEnd)
	{
		if (isspace(styler->ch))
		{
			styler->Forward();
			continue;
		}
		styler->SetState(SCE_BKE_ATTRIBUTE);
		int startPos = styler->currentPos;
		if (styler->ch >= 80 || styler->ch == '_' || isalpha(styler->ch))
		{
			while (styler->More())
			{
				if (styler->ch >= 80 || styler->ch == '_' || isalnum(styler->ch))
				{
					styler->Forward();
				}
				else
					break;
			}
			if (styler->ch == '=')
			{
				styler->ChangeState(SCE_BKE_DEFAULT);
				styler->Forward();
			}
			else
			{
				//see as no attr
				styler->ChangeState(SCE_BKE_DEFAULT);
				styler->setPos(startPos);
			}
		}
		else
		{
			//no attr
			styler->ChangeState(SCE_BKE_DEFAULT);
			styler->setPos(startPos);
		}
		ParseBegal(false, false, true);
	}
}

void BKE_Lexer::ContinueLabel()
{
	while (styler->More() && !styler->atLineEnd)
	{
		if (atComment())
			break;
		styler->Forward();
	}
	styler->SetState(SCE_BKE_DEFAULT);
	return;
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
			styler->SetState(last_state);   //恢复原状态
			break;
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
	styler->SetState(SCE_BKE_DEFAULT);
}

void BKE_Lexer::JudgeStyle()
{
	//styler->setMaskState(0);
	if (styler->ch == '@')
	{
		//styler->setMaskState(COMMENT_MASK);
		styler->SetState(SCE_BKE_COMMAND);
	}
	else if (styler->ch == '[')
	{
		//styler->setMaskState(COMMENT_MASK);
		styler->SetState(SCE_BKE_COMMAND2);
	}
	else if (styler->ch == '/' && styler->chNext == '/')
	{
		styler->SetState(SCE_BKE_ANNOTATE);
		styler->Forward();
	}
	else if (styler->ch == ';' && styler->atLineStart)
		styler->SetState(SCE_BKE_ANNOTATE);
	else if (styler->ch == '/' && styler->chNext == '*')
	{
		//styler->setMaskState(COMMENT_MASK);
		last_state = styler->state;
		styler->SetState(SCE_BKE_COMMENT);
		styler->Forward();
	}
	else if (styler->ch == '*' && styler->atLineStart)
		styler->SetState(SCE_BKE_LABEL);
	else if (styler->ch == '#')
	{
		if (styler->chNext == '#')
		{
			styler->SetState(SCE_BKE_PARSER_DEFAULT);
			styler->Forward();
		}
		else
		{
			styler->SetState(SCE_BKE_PARSER);
		}
	}
	else if (styler->ch == '\"')
	{
		//styler->setMaskState(BEGAL_MASK);
		styler->SetState(SCE_BKE_STRING);
	}
	else if (styler->ch == '\'')
	{
		//styler->setMaskState(BEGAL_MASK);
		styler->SetState(SCE_BKE_STRING2);
	}
	else if (styler->atLineEnd)
	{
		styler->SetState(SCE_BKE_DEFAULT);
		if (styler->ch == '\r' && styler->chNext == '\n')
			styler->Forward();
	}
	else
	{
		styler->SetState(SCE_BKE_TEXT);
	}
	bool stillstart = false;
	if (styler->atLineStart && styler->ch == '\t')
		stillstart = true;
	styler->Forward();
	if (!styler->atLineStart && stillstart)
	{
		styler->atLineStart = true;
		styler->ChangeState(SCE_BKE_DEFAULT);
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

	StyleContext sc(startPos, lengthDoc, initStyle, accessor);
	styler = &sc;
	//styler->setRange(startPos, startPos + lengthDoc, initStyle);

	while (styler->More())
	{
		switch (styler->state)
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
		case SCE_BKE_LABEL:
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
	//styler->continueStateToPos(startPos + lengthDoc);
	styler->Complete();
}

void SCI_METHOD BKE_Lexer::Fold(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess)
{

}

#define SCLEX_BKE 108

LexerModule lm_BKE(SCLEX_BKE, BKE_Lexer::LexerFactory, "bke");