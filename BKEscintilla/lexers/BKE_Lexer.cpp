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

#define BASE_MASK 63
#define BEGAL_MASK (1<<6)
//in @ or [ command
#define CMD_MASK (1<<7)

class BKE_Lexer :public ILexer
{
private:
	//what do we need for a perfect highlight?

	LexAccessor *accessor;

	StyleContext/*BKE_Accessor*/ *styler;

	BKE_Info *info;

	unsigned char cur_mask;
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
		if (styler->ch == '\"' && styler->chNext != '\"')
		{
			styler->Forward();
			styler->SetState(last_state | cur_mask);
			return true;
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
	bool intrans = false;
	while (styler->More() && !styler->atLineEnd)
	{
		if (!intrans && styler->ch == '\'')
		{
			//color all
			if (trans_info.empty())
			{
				styler->Forward();
				styler->SetState(last_state | cur_mask);
			}
			else
			{
				int pos2 = styler->currentPos + 1;
				styler->setPos(pos);
				auto it = trans_info.begin();
				while (it != trans_info.end())
				{
					styler->setPos(it->begin);
					styler->SetState(it->correct ? SCE_BKE_TRANS | cur_mask : SCE_BKE_ERROR | cur_mask);
					styler->setPos(it->begin + it->len);
					styler->SetState(SCE_BKE_STRING2 | cur_mask);
					it++;
				}
				styler->setPos(pos2);
				styler->SetState(last_state | cur_mask);
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
			case '\"':
			case '\'':
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
	styler->ChangeState(SCE_BKE_ERROR | cur_mask);
	styler->SetState(last_state | cur_mask);
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
			removeMask(BEGAL_MASK);
			styler->SetState(SCE_BKE_DEFAULT | cur_mask);
			break;
		}
		if (isspace(styler->ch) && !ignoreSpace)
		{
			removeMask(BEGAL_MASK);
			styler->SetState(SCE_BKE_DEFAULT | cur_mask);
			break;
		}
		if (styler->ch == ']' && !atCommand)
		{
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
		last_state = styler->state & BASE_MASK;
		switch (styler->ch)
		{
		case '\"':
			styler->SetState(SCE_BKE_STRING | cur_mask);
			styler->Forward();
			ParseString();
			break;
		case '\'':
			styler->SetState(SCE_BKE_STRING2 | cur_mask);
			styler->Forward();
			ParseString2();
			break;
		case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:
			styler->SetState(SCE_BKE_NUMBER | cur_mask);
			styler->Forward();
			ParseNumber();
			break;
		case '#':
			styler->SetState(SCE_BKE_COLOR | cur_mask);
			styler->Forward();
			ParseColor();
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
		default:
			if (info->OperatorAncestor.indexOf(styler->ch) >= 0)
			{
				styler->SetState(SCE_BKE_OPERATORS | cur_mask);
				styler->Forward();
			}
			else if (isalpha(styler->ch) || styler->ch == '_' || styler->ch >= 0x80)
			{
				styler->SetState(SCE_BKE_PARSER_KEYWORD | cur_mask);
				styler->Forward();
				ParseVarname();
			}
			else if (isspace(styler->ch))
			{
				styler->SetState(defaultState | cur_mask);
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
				styler->SetState(SCE_BKE_ERROR | cur_mask);
				styler->Forward();
			}
		}
	}
}

void BKE_Lexer::DoCommand()
{
	//check cmd name
	setMask(CMD_MASK);
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
		styler->ChangeState(SCE_BKE_ERROR | cur_mask);
	}
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
				styler->SetState(SCE_BKE_DEFAULT | cur_mask);
				styler->Forward();
			}
			else
			{
				//see as no attr
				styler->ChangeState(SCE_BKE_DEFAULT | cur_mask);
				styler->setPos(startPos);
			}
		}
		else
		{
			//no attr
			styler->ChangeState(SCE_BKE_DEFAULT | cur_mask);
			styler->setPos(startPos);
		}
		ParseBegal(false, false, false);
	}
	removeMask(CMD_MASK);
	if (styler->ch == ']')
	{
		styler->SetState(SCE_BKE_COMMAND | cur_mask);
		styler->Forward();
		styler->SetState(SCE_BKE_DEFAULT | cur_mask);
	}
}

void BKE_Lexer::DoAtCommand()
{
	setMask(CMD_MASK);
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
		styler->ChangeState(SCE_BKE_ERROR | cur_mask);
	}
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
				styler->SetState(SCE_BKE_DEFAULT | cur_mask);
				styler->Forward();
			}
			else
			{
				//see as no attr
				styler->ChangeState(SCE_BKE_DEFAULT | cur_mask);
				styler->setPos(startPos);
			}
		}
		else
		{
			//no attr
			styler->ChangeState(SCE_BKE_DEFAULT | cur_mask);
			styler->setPos(startPos);
		}
		ParseBegal(false, false, true);
	}
	removeMask(CMD_MASK);
}

void BKE_Lexer::ContinueLabel()
{
	setMask(CMD_MASK);
	styler->SetState(SCE_BKE_LABEL | cur_mask);
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
		styler->Forward();
	}
	removeMask(CMD_MASK);
	styler->SetState(SCE_BKE_DEFAULT | cur_mask);
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
			styler->SetState(last_state | cur_mask);   //恢复原状态
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
	styler->SetState(SCE_BKE_DEFAULT | cur_mask);
}

void BKE_Lexer::JudgeStyle()
{
	if (styler->ch == '@')
	{
		styler->SetState(SCE_BKE_COMMAND | cur_mask);
	}
	else if (styler->ch == '[')
	{
		styler->SetState(SCE_BKE_COMMAND2 | cur_mask);
	}
	else if (styler->ch == '/' && styler->chNext == '/')
	{
		styler->SetState(SCE_BKE_ANNOTATE | cur_mask);
		styler->Forward();
	}
	else if (styler->ch == ';' && styler->atLineStart)
		styler->SetState(SCE_BKE_ANNOTATE | cur_mask);
	else if (styler->ch == '/' && styler->chNext == '*')
	{
		//styler->setMaskState(COMMENT_MASK);
		last_state = styler->state;
		styler->SetState(SCE_BKE_COMMENT | cur_mask);
		styler->Forward();
	}
	else if (styler->ch == '*' && styler->atLineStart)
	{
		styler->SetState(SCE_BKE_LABEL | cur_mask);
	}
	else if (styler->ch == '#')
	{
		if (styler->chNext == '#')
		{
			setMask(BEGAL_MASK);
			styler->SetState(SCE_BKE_PARSER_DEFAULT | cur_mask);
			styler->Forward();
		}
		else
		{
			setMask(BEGAL_MASK);
			styler->SetState(SCE_BKE_PARSER | cur_mask);
		}
	}
	else if (styler->ch == '\"')
	{
		//styler->setMaskState(BEGAL_MASK);
		styler->SetState(SCE_BKE_STRING | cur_mask);
	}
	else if (styler->ch == '\'')
	{
		//styler->setMaskState(BEGAL_MASK);
		styler->SetState(SCE_BKE_STRING2 | cur_mask);
	}
	else if (styler->atLineEnd)
	{
		styler->SetState(SCE_BKE_DEFAULT | cur_mask);
		if (styler->ch == '\r' && styler->chNext == '\n')
			styler->Forward();
	}
	else
	{
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

	StyleContext sc(startPos, lengthDoc, initStyle, accessor, 0xFF);
	styler = &sc;

	unsigned char laststyle = (unsigned char)pAccess->StyleAt(startPos - 1);
	unsigned char curstyle = (unsigned char)pAccess->StyleAt(startPos);
	cur_mask = initStyle & ~BASE_MASK;

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

	}
	//styler->setRange(startPos, startPos + lengthDoc, initStyle);

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
	LexAccessor styler(pAccess);

	int levelCurrent = SC_FOLDLEVELBASE >> 16;
	styler.SetLevel(0, (levelCurrent + 1) << 16 + 1);
}

#define SCLEX_BKE 108

LexerModule lm_BKE(SCLEX_BKE, BKE_Lexer::LexerFactory, "bke");