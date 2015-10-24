#include "weh.h"
#include "ParseData.h"
#include "bkeSci\bkescintilla.h"

const char *InidicatorMSG[]=
{
	"标签(%1)后有额外的字符，请核实是不是想写的标签名中带了非法的字符",
	"缺少命令名", 
	"缺少对应的(",
	"缺少对应的[",
	"缺少对应的{",
	"此处需要)",
	"此处需要]",
	"此处需要}",
	"字符串未完结",
	"块注释未完结",
	"缺少标签名",
	"[开始的指令没有正确的以]结尾",
	"##块没有对应的##结尾",
	"该文件被登记为宏文件，应当以*register开头",
	"import命令缺少必需的属性file",
	"import的文件%2不在工程内或文件不存在",
	"macro命令缺少必需的属性name",
	NULL
};

void insert(Pos p, Pos &org, Pos off)
{
	if (p.line < org.line)
	{
		org.line += off.line;
	}
	else if (p.line == org.line && p.pos < org.pos)
	{
		org.line += off.line;
		if (off.line)
			org.pos = org.pos - p.pos + off.pos;
		else
			org.pos += off.pos;
	}
}

void erase(Pos p, Pos &org, Pos off)
{
	if (p.line + off.line < org.line)
	{
		org.line -= off.line;
	}
	else if (p <= org)
	{
		org.line -= off.line;
		org.pos -= off.pos;
	}
}

ParseData::ParseData(QByteArray &file)
{
	//int len = file.length();
	//textbuf = new char[len + 1];
	textbuf = file.data();
	isLineStart = true;
	refresh = true;
}

void ParseData::getLabels(set<QString> &l)
{
	l.clear();
	for (auto &it : labels)
	{
		l.insert((*it)->name);
	}
}

/*
void ParseData::insertChars(Pos p, Pos offset)
{
	auto it = fileNodes.begin();
	if (offset.line)
	{
		while (it != fileNodes.end())
		{
			if ((*it)->endPos >= p)
			{
				insert(p, (*it)->startPos, offset);
				insert(p, (*it)->endPos, offset);
			}
			it++;
		}
	}
	else
	{
		while (it != fileNodes.end() && (*it)->startPos.line <= p.line)
		{
			if ((*it)->endPos >= p)
			{
				insert(p, (*it)->startPos, offset);
				insert(p, (*it)->endPos, offset);
			}
			it++;
		}
	}
}

void ParseData::deleteChars(Pos p, Pos offset)
{
	auto it = fileNodes.begin();
	while (it != fileNodes.end())
	{
		if ((*it)->endPos < p)
		{
		}
		else if ((*it)->startPos < p)
		{
			erase(p, (*it)->endPos, offset);

			if ((*it)->endPos < p)
			{
				(*it)->endPos = p;
			}
		}
		else
		{
			erase(p, (*it)->startPos, offset);
			erase(p, (*it)->endPos, offset);

			if ((*it)->startPos < p)
			{
				if ((*it)->endPos < p)
				{
					//whole cmd is deleted
					it = fileNodes.erase(it);
					continue;
				}
				else
				{
					(*it)->startPos = p;
				}
			}
		}
		it++;
		continue;
	}
}

void ParseData::reMake(int start, int len, unsigned int &outstart, int &outlen, list<BaseNode*>::iterator &iter)
{
	if (fileNodes.empty())
	{
		outstart = 0;
		outlen = scifile->length();
		iter = fileNodes.end();
		return;
	}
	Pos st, ed;
	scifile->lineIndexFromPositionByte(start, &st.line, &st.pos);
	scifile->lineIndexFromPositionByte(start + len, &ed.line, &ed.pos);
	auto it = fileNodes.begin();
	while (it != fileNodes.end())
	{
		if ((*it)->endPos > st)
			break;
		++it;
	}
	if (it == fileNodes.end())
	{
		iter = fileNodes.end();
		outstart = scifile->positionFromLineIndexByte(fileNodes.back()->endPos.line, fileNodes.back()->endPos.pos) + 1;
		outlen = scifile->length() - outstart;
		return;
	}
	auto stit = it == fileNodes.begin() ? it : --it;
	outstart = scifile->positionFromLineIndexByte((*it)->startPos.line, (*it)->startPos.pos);
	while (it != fileNodes.end())
	{
		if ((*it)->startPos >= ed)
			break;
		if ((*it)->isLabel())
			labels.erase((*it)->reflect);
		++it;
	}
	auto stit2 = it;
	iter = it;
	--it;
	assert(it != fileNodes.end());
	outlen = scifile->positionFromLineIndexByte((*it)->endPos.line, (*it)->endPos.pos) - outstart + 1;
	for (auto it2 = stit; it2 != stit2; it2++)
	{
		delete *it2;
	}
	fileNodes.erase(stit, stit2);
}
*/

int ParseData::findLabel(const QString &l)
{
	for (auto &it : labels)
	{
		if ((*it)->name == l)
		{
			return (*it)->startPos;
		}
	}
	return -1;
}

//void ParseData::refreshLabel()
//{
//	emit scifile->refreshLabel(scifile);
//}

BaseNode *ParseData::findNode(/*Pos p*/int pos)
{
	auto it = fileNodes.upperBound(pos);
	if (it == fileNodes.begin())
		return NULL;
	--it;
	if ((*it)->endPos < pos)
		return NULL;
	return *it;
	//auto it = fileNodes.begin();
	//while (it != fileNodes.end() && (*it)->endPos < p)
	//{
	//	++it;
	//}
	//if (it != fileNodes.end() && (*it)->startPos <= p)
	//	return *it;
	//return NULL;
}

QString readCmdName(unsigned char *buf, int &start, int end, bool startwithat)
{
	QByteArray ba;
	while (start < end)
	{
		if (ISSPACE(buf[start]))
			break;
		if (!startwithat && buf[start] == ']')
			break;
		ba.push_back(buf[start]);
		start++;
	}
	QString tmp;
	tmp.prepend(ba);
	return tmp;
}

QString readName(unsigned char *buf, int &start, int end)
{
	QByteArray ba;
	if (buf[start] == '_' || isalpha(buf[start]) || buf[start] >= 0x80)
	{
		ba.push_back(buf[start]);
		start++;
		while (start < end && (buf[start] == '_' || isalnum(buf[start]) || buf[start] >= 0x80))
		{
			ba.push_back(buf[start]);
			start++;
		}
		QString tmp;
		tmp.prepend(ba);
		return tmp;
	}
	return QString();
}

QString readValue(unsigned char *buf, int &start, int end, bool startwithat, BkeScintilla *scifile, int startpos)
{
	enum BracketType
	{
		Bracket_Small,
		Bracket_Medium,
		Bracket_Large
	};
	vector<BracketType> _stack;
	vector<int> posstack;
	posstack.push_back(start);

	QByteArray ba;

	int yinhao = 0;

	while (start < end)
	{
		if (buf[start] == ']' && !startwithat && !yinhao && _stack.empty())
			break;
		if (buf[start] == '\r' && buf[start] == '\n' && _stack.empty())
			break;
		if (ISSPACE(buf[start]) && !yinhao && _stack.empty())
			break;
		ba.push_back(buf[start]);
		char ch = buf[start];
		if (ch == '\\' && start + 1 < end)
		{
			if (yinhao != 2)
			{
				start++;
				ba.push_back(buf[start]);
			}
		}
		else if (ch == '\"')
		{
			if (!yinhao)
			{
				yinhao = 1;
				posstack.push_back(start);
			}
			else if (yinhao == 1)
			{
				if (start +1 <end && buf[start+1]=='\"')
					ba.push_back(buf[++start]);
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
				posstack.push_back(start);
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
			posstack.push_back(start);
		}
		else if (ch == '[' && !yinhao)
		{
			_stack.push_back(BracketType::Bracket_Medium);
			posstack.push_back(start);
		}
		else if (ch == '{' && !yinhao)
		{
			_stack.push_back(BracketType::Bracket_Large);
			posstack.push_back(start);
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
				scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 2);
				if (_stack.empty())
				{
					//缺少对应的(
					scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 3);
				}
				else
				{
					//此处应为其它括号
					scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 6 + _stack.back());
				}
				scifile->SendScintilla(BkeScintilla::SCI_INDICATORFILLRANGE, startpos + start, 1);
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
				scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 2);
				if (_stack.empty())
				{
					//缺少对应的[
					scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 4);
				}
				else
				{
					//此处应为其它括号
					scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 6 + _stack.back());
				}
				scifile->SendScintilla(BkeScintilla::SCI_INDICATORFILLRANGE, startpos + start, 1);
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
				scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 2);
				if (_stack.empty())
				{
					//缺少对应的{
					scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 5);
				}
				else
				{
					//此处应为其它括号
					scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 6 + _stack.back());
				}
				scifile->SendScintilla(BkeScintilla::SCI_INDICATORFILLRANGE, startpos + start, 1);
			}
		}
		else if (ch == ';' && !yinhao)
		{
			if (!_stack.empty())
			{
				//此处应为其它括号
				scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 2);
				scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 6 + _stack.back());
				scifile->SendScintilla(BkeScintilla::SCI_INDICATORFILLRANGE, startpos + start, 1);
				_stack.clear();
			}
		}
		++start;
	}
	if (yinhao)
	{
		//字符串未完结
		scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 2);
		scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 9);
		scifile->SendScintilla(BkeScintilla::SCI_INDICATORFILLRANGE, startpos + posstack.back(), start + 1 - posstack.back());
	}
	else if (!_stack.empty())
	{
		//此处应为其它括号
		scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 2);
		scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 6 + _stack.back());
		scifile->SendScintilla(BkeScintilla::SCI_INDICATORFILLRANGE, startpos + posstack.back(), start + 1 - posstack.back());
		_stack.clear();
	}
	QString tmp;
	tmp.prepend(ba);
	return tmp;
}

bool skipSpace(char *buf, int &start, int end)
{
	if (!ISSPACE(buf[start]))
		return false;
	while (start < end && ISSPACE(buf[start]))
		start++;
	return true;
}

/*
bool ParseData::checkLabel(BaseNode *node)
{
	int p1, p2;
	p1 = scifile->positionFromLineIndexByte(node->startPos.line, node->startPos.pos);
	p2 = scifile->positionFromLineIndexByte(node->endPos.line, node->endPos.pos);
	char *buf = new char[p2 - p1 + 1];
	scifile->SendScintilla(BkeScintilla::SCI_GETTEXTRANGE, p1, p2, buf);
	int i = 1;
	node->name = readCmdName((u8*)buf, i, p2 - p1, true);
	i++;
	while (i < p2 - p1)
	{
		if (!ISSPACE(buf[i]))
			break;
		i++;
	}
	if (i < p2 - p1 && buf[i] != '\r' && buf[i] != '\n')
	{
		//空格后还有字符，警告
		scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 3);
		scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 1);
		scifile->SendScintilla(BkeScintilla::SCI_INDICATORFILLRANGE, p1 + i, p2 - p1 - i);
	}

	return true;
}

bool ParseData::checkCommand(BaseNode *node, bool startwithat)
{
	int p1, p2;
	p1 = scifile->positionFromLineIndexByte(node->startPos.line, node->startPos.pos);
	p2 = scifile->positionFromLineIndexByte(node->endPos.line, node->endPos.pos);
	char *buf = new char[p2 - p1 + 1];
	scifile->SendScintilla(BkeScintilla::SCI_GETTEXTRANGE, p1, p2 + 1, buf);

	int i = 1;
	node->name = readCmdName((u8*)buf, i, p2 - p1, startwithat);

	if (node->name.isEmpty())
	{
		//缺少命令名，错误
		scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 2);
		scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 2);
		scifile->SendScintilla(BkeScintilla::SCI_INDICATORFILLRANGE, p1, p1 + 1);
	}

	skipSpace(buf, i, p2 - p1);

	while (i < p2 - p1 && buf[i] != '\r' && buf[i] != '\n' && (startwithat || buf[i] != L']'))
	{
		int rawpos = i;
		auto *subnode = new BaseNode(node);
		scifile->lineIndexFromPositionByte(i, &subnode->startPos.line, &subnode->startPos.pos);
		subnode->name = readName((u8*)buf, i, p2 - p1);
		scifile->lineIndexFromPositionByte(i, &subnode->endPos.line, &subnode->endPos.pos);
		if (buf[i] == '=')
		{
			subnode->type = BaseNode::Node_CmdProp;
			subnode->cached = true;
			node->children.push_back(subnode);
			subnode = new BaseNode(node);
			i++;
			scifile->lineIndexFromPositionByte(i, &subnode->startPos.line, &subnode->startPos.pos);
		}
		else
		{
			i = rawpos;
		}
		subnode->name = readValue((u8*)buf, i, p2 - p1, startwithat, scifile, p1);
		scifile->lineIndexFromPositionByte(i, &subnode->endPos.line, &subnode->endPos.pos);
		subnode->type = BaseNode::Node_CmdPropValue;
		subnode->cached = true;
		node->children.push_back(subnode);
		skipSpace(buf, i, p2 - p1);
	}
	if (!startwithat && buf[i] != ']')
	{
		//此处需要]
		scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 2);
		scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORVALUE, 7);
		scifile->SendScintilla(BkeScintilla::SCI_INDICATORFILLRANGE, p1, p1 + 1);
	}

	return true;
}

bool ParseData::checkCmd(BaseNode *node)
{
	node->cached = true;
	//clear indicator
	int p1, p2;
	p1 = scifile->positionFromLineIndexByte(node->startPos.line, node->startPos.pos);
	p2 = scifile->positionFromLineIndexByte(node->endPos.line, node->endPos.pos);
	scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 2);
	scifile->SendScintilla(BkeScintilla::SCI_INDICATORCLEARRANGE, p1, p2);
	scifile->SendScintilla(BkeScintilla::SCI_SETINDICATORCURRENT, 3);
	scifile->SendScintilla(BkeScintilla::SCI_INDICATORCLEARRANGE, p1, p2);

	switch (node->type)
	{
	case BaseNode::Node_Document:
	case BaseNode::Node_CmdProp:
	case BaseNode::Node_CmdPropValue:
	case BaseNode::Node_Comment:
	case BaseNode::Node_LineComment:
	case BaseNode::Node_Text:
		return true;
	case BaseNode::Node_Label:
		return checkLabel(node);
	case BaseNode::Node_AtCommand:
		return checkCommand(node, true);
	case BaseNode::Node_Command:
		return checkCommand(node, false);
	}
}
*/

bool ParseData::Parse()
{
	int idx2 = idx + 10 * 1024;
	while (1)
	{
		if (idx >= idx2)
			return true;
		isLineStart = skipLineEnd();
		char ch = getNextOne();
		if (!ch)
			return false;
		
		if (isLineStart)
		{
			while (ch == '\t')
			{
				ch = getNextOne();
			}
			if (!ch)
				return false;
		}

		int rawidx = idx;

		while (ch && ISSPACE(ch) && ch != '\r' && ch != '\n')
			ch = getNextOne();

		if (ch == '\r' || ch == '\n')
			continue;

		if (!(ch == '@' || ch == '[' || ch == '#' || ch == '*'))
		{
			idx = rawidx;
			ch = getNextOne();
		}

		auto node = new BaseNode(NULL);
		bool startwithat = false;

		switch (ch)
		{
		case ';':
			if (isLineStart)
				skipLineComment();
			else
			{
				node->type = BaseNode::Node_Text;
				node->startPos = idx - 1;
				skipText();
				node->endPos = idx - 1;
				fileNodes.insert(node->startPos, node);
			}
			break;
		case '*':
			if (isLineStart)
			{
				node->type = BaseNode::Node_Label;
				node->startPos = idx - 1;
				node->name = readCmdName(true);
				if (node->name.isEmpty())
				{
					infos.emplace_back(3, 11, node->startPos, 1);
				}
				skipSpace();
				if (!isLineEnd())
				{
					int start = idx;
					skipLineComment();
					int len = idx - start;
					infos.emplace_back(3, 1, start, len);
				}
				node->endPos = idx - 1;
				auto it = fileNodes.insert(node->startPos, node);
				labels.push_back(it);
			}
			else
			{
				node->type = BaseNode::Node_Text;
				node->startPos = idx - 1;
				skipText();
				node->endPos = idx - 1;
				fileNodes.insert(node->startPos, node);
			}
			break;
		case '@':
			if (!isLineStart)
			{
				node->type = BaseNode::Node_Text;
				node->startPos = idx - 1;
				skipText();
				node->endPos = idx - 1;
				fileNodes.insert(node->startPos, node);
				break;
			}
			else
				startwithat = true;
		case '[':
			node->type = startwithat ? BaseNode::Node_AtCommand : BaseNode::Node_Command;
			node->startPos = idx - 1;
			node->name = readCmdName(startwithat);
			if (node->name.isEmpty())
			{
				infos.emplace_back(3, 2, node->startPos, 1);
			}
			skipSpace();
			ch = fetchNextOne();
			while (!isLineEnd() && (startwithat || ch != L']'))
			{
				auto node2 = new BaseNode(node);
				node2->type = BaseNode::Node_CmdProp;
				node2->startPos = idx - node->startPos;
				rawidx = idx;
				node2->name = readCmdName(startwithat);
				ch = fetchNextOne();
				if (node2->name.isEmpty() || ch != '=')
				{
					idx = rawidx;
					node->cmdParam.push_back(NULL);
				}
				else
				{
					node2->endPos = idx;
					node->cmdParam.push_back(node2);
					auto node2 = new BaseNode(node);
					idx++;
				}
				node2->type = BaseNode::Node_CmdPropValue;
				node2->startPos = idx - node->startPos;
				node2->name = readValue(startwithat);
				node2->endPos = idx - 1;
				node->cmdValue.push_back(node2);
				skipSpace();
				ch = fetchNextOne();
			}
			if (!startwithat)
			{
				if (ch == ']')
					idx++;
				if (ch != ']')
				{
					int start = idx;
					skipLineComment();
					infos.emplace_back(2, 12, start, idx - start);
				}
			}
			node->endPos = idx;
			fileNodes.insert(node->startPos, node);
			break;
		case '#':
			node->type = BaseNode::Node_LineParser;
			node->startPos = idx - 1;
			ch = textbuf[idx];
			if (ch == '#')
			{
				rawidx = idx;
				skipSpace();
				if (isLineEnd())
				{
					node->type = BaseNode::Node_Parser;
					skipLineEnd();
				}
				else
					idx = rawidx;
			}
			if (node->type == BaseNode::Node_LineParser)
			{
				rawidx = idx;
				skipLineComment();
				node->name = QString::fromStdString(string(textbuf + rawidx, idx - rawidx));
				node->endPos = idx - 1;
				fileNodes.insert(node->startPos, node);
			}
			else
			{
				rawidx = idx;
				ch = textbuf[idx];
				while (ch)
				{
					if (ch == '#' && textbuf[idx + 1] == '#')
					{
						break;
					}
					skipLineComment();
					skipLineEnd();
					ch = textbuf[idx];
				}
				node->name = QString::fromStdString(string(textbuf + rawidx, idx - rawidx));
				node->endPos = ch ? idx + 1 : idx - 1;
				if (!ch)
				{
					infos.emplace_back(2, 13, idx - 1, 1);
				}
				fileNodes.insert(node->startPos, node);
			}
			break;
		default:
			node->type = BaseNode::Node_Text;
			node->startPos = idx - 1;
			skipText();
			node->endPos = idx - 1;
			fileNodes.insert(node->startPos, node);
			break;
		}
	}
	return true;
}