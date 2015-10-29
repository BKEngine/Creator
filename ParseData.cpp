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
	"macro不能与系统命令重名",
	"macro名称必须为常量字符串表达式",
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

ParseData::ParseData(QByteArray &file, BKE_VarClosure *clo)
{
	//int len = file.length();
	//textbuf = new char[len + 1];
	textbuf = file.data();
	isLineStart = true;
	refresh = true;
	fileclo = new BKE_VarClosure();
	fileclo->cloneFrom(clo);
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
			{
				int rawidx = idx;
				skipLineComment();
				lastComment = QString::fromStdString(string(textbuf + rawidx, idx - rawidx));
				delete node;
				continue;
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
					node2 = new BaseNode(node);
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
				idx++;
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
				else
					idx += 2;
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
		node->comment = lastComment;
		lastComment.clear();
	}
	return true;
}

PAModule::PAModule(const QString &str) : Parser(false), expstr(str.toStdWString())
{
	p = Parser::getInstance();
	curpos = exp = expstr.c_str();
	expsize = expstr.size();
	NextIsBareWord = false;
	forcequit = false;

	//copy lbp, pre, runner
	memcpy(lbp, p->lbp, sizeof(lbp));
	pre = lbp + OP_COUNT;
	memcpy(runner, p->runner, sizeof(runner));

	restree = new BKE_bytree();
	restree->Node.opcode = OP_RESERVE + OP_COUNT;
	next.opcode = OP_STOP;
	do
	{
		try
		{
			while (next.opcode == OP_STOP)
				readToken();
			restree->addChild();
			expression(&restree->childs.back());
		}
		catch (Var_Except &)
		{
			skipToNextSentence();
		}
		if (!restree->childs.empty() && restree->childs.back() == NULL)
			restree->childs.pop_back();
	} while (next.opcode != OP_END);

	if (restree->childs.empty())
	{
		constvar = false;
		restree->release();
		restree = NULL;
	}
	else
	{
		BKE_bytree *tr = restree->childs.back();
		assert(tr);
		constvar = tr->Node.opcode == OP_CONSTVAR + OP_COUNT;
		if (constvar)
			res = tr->Node.var;
	}
}

void PAModule::skipToNextSentence()
{
	while (1)
	{
		try
		{
			readToken();
			if (next.opcode == OP_STOP || next.opcode == OP_END)
				break;
		}
		catch (Var_Except &)
		{
			curpos++;
		}
	}
}

void PAModule::analysisToClosure(BKE_VarClosure *clo)
{
	if (!restree || !clo)
		return;
	top = clo;
	for (int i = 0; i < restree->childs.size(); i++)
	{
		_analysisToClosure(restree->childs[i], clo, NULL);
	}
}

void PAModule::_analysisToClosure(BKE_bytree *tr, BKE_VarClosure *clo, BKE_Variable *var)
{
	if (!tr)
		return;
	switch (tr->Node.opcode)
	{
	case OP_IF + OP_COUNT:
	case OP_FOR + OP_COUNT:
	case OP_WHILE + OP_COUNT:
	case OP_DO + OP_COUNT:
	case OP_QUICKFOR + OP_COUNT:
	case OP_FOREACH + OP_COUNT:
	case OP_BLOCK + OP_COUNT:
		{
			auto c = new BKE_VarClosure(clo);
			for (int i = 0; i < tr->childs.size(); i++)
				_analysisToClosure(tr->childs[i], c, var);
			c->release();
		}
		break;
	case OP_VAR + OP_COUNT:
		for (int i = 0; i < tr->childs.size(); i+=2)
		{
			auto str = tr->childs[i]->Node.var.asBKEStr();
			BKE_Variable *v = &clo->getMember(str);
			_analysisToClosure(tr->childs[i + 1], clo, v);
		}
		break;
	case OP_CONSTVAR + OP_COUNT:
		if (var)
			*var = tr->Node.var;
		break;
	case OP_LITERAL + OP_COUNT:
		tmpvar = &clo->getMember(tr->Node.var.asBKEStr());
		break;
	case OP_DOT + OP_COUNT:
		{
			auto str = tr->childs[0]->Node.var.asBKEStr();
			if (!clo->withvar)
			{
				tmpvar = &top->getMember(str);
			}
			else
			{
				if (clo->withvar->vt == VAR_CLASS)
					tmpvar = &((BKE_VarClass*)clo->withvar)->getMember(str);
				else if (clo->withvar->vt == VAR_DIC)
					tmpvar = &((BKE_VarDic*)clo->withvar)->getMember(str);
			}
		}
		break;
	case OP_DOT:
		tmpvar = NULL;
		_analysisToClosure(tr->childs[0], clo, NULL);
		if (tmpvar && tmpvar->getType() == VAR_DIC)
		{
			tmpvar = &((BKE_VarDic*)tmpvar->obj)->getMember(tr->childs[1]->Node.var.asBKEStr());
		}
		else
			tmpvar = NULL;	//识别终止
		break;
	case OP_SET:
	case OP_SETADD:
	case OP_SETDIV:
	case OP_SETSUB:
	case OP_SETMUL:
	case OP_SETMOD:
	case OP_SETPOW:
	case OP_SETSET:
		tmpvar = NULL;
		_analysisToClosure(tr->childs[0], clo, NULL);
		_analysisToClosure(tr->childs[1], clo, tmpvar);
		break;
	default:
		for (int i = 0; i < tr->childs.size(); i++)
			_analysisToClosure(tr->childs[i], clo, var);
		break;
	}
}

#define MATCH(a, b) if(MatchFunc(L##a, &curpos)){ node.opcode = b; return;}
#define QRET(c)	curpos++;node.opcode = c;return;

bool PAModule::MatchFunc(const wchar_t *a, const wchar_t **c)
{
	const wchar_t *b;
	b = *c;
	while ((*a) && (*b))
	{
		if ((*a) != (*b))
			return false;
		b++;
		a++;
	}
	*c = b;
	return true;
}

void PAModule::readToken()
{
	BKE_Node &node = next;
	while ((*curpos) && bkpIsWSpace(*curpos))curpos++;
	node.pos = static_cast<bkpulong>(curpos - exp);
	if (!(*curpos))
	{
		node.opcode = OP_END;
		return;
	}
	wchar_t ch = *curpos;
	//read operators
	switch (ch)
	{
	case L'+':
		MATCH("+=", OP_SETADD);
		MATCH("++", OP_SELFADD);
		QRET(OP_ADD);
	case L'-':
		MATCH("-=", OP_SETSUB);
		MATCH("--", OP_SELFDEC);
		QRET(OP_SUB);
	case L'*':
		MATCH("*=", OP_SETMUL);
		QRET(OP_MUL);
	case L'/':
		if (MatchFunc(L"//", &curpos))
		{
			//skip a line
			while (*curpos != L'\0' && *curpos != L'\r' && *curpos != L'\n')
				curpos++;
			//bufnodes.pop_back();
			readToken();
			return;
		}
		if (MatchFunc(L"/*", &curpos))
		{
			while (*curpos && !(*curpos == L'*' && *(curpos + 1) == L'/'))
				curpos++;
			//bufnodes.pop_back();
			curpos += 2;
			readToken();
			return;
		}
		MATCH("/=", OP_SETDIV);
		QRET(OP_DIV);
	case L'%':
		MATCH("%[", OP_DIC);
		MATCH("%=", OP_SETMOD);
		QRET(OP_MOD);
	case L'^':
		MATCH("^=", OP_SETPOW);
		QRET(OP_POW);
	case L'!':
		MATCH("!==", OP_NNEQUAL);
		MATCH("!=", OP_NEQUAL);
		QRET(OP_NOT);
	case L'=':
		MATCH("===", OP_EEQUAL);
		MATCH("=>", OP_VALUE);
		MATCH("==", OP_EQUAL);
		QRET(OP_SET);
	case L'>':
		MATCH(">=", OP_LE);
		QRET(OP_LARGER);
	case L'<':
		MATCH("<=", OP_SE);
		QRET(OP_SMALLER);
	case L'|':
		MATCH("|=", OP_SETSET);
		MATCH("||", OP_OR);
		QRET(OP_FASTOR);
	case L'&':
		MATCH("&&", OP_AND);
		QRET(OP_FASTAND);
	case L'(':
		QRET(OP_BRACKET);
	case L'[':
		QRET(OP_ARRAY);
	case L'{':
		QRET(OP_BLOCK);
	case L'.':
		NextIsBareWord = true;
		QRET(OP_DOT);
	case L'?':
		QRET(OP_CHOOSE);
	case L':':
		QRET(OP_MAOHAO);
	case L';':
		QRET(OP_STOP);
	case L')':
		QRET(OP_BRACKET2);
	case L']':
		QRET(OP_ARR2);
	case L'}':
		QRET(OP_BLOCK2);
	case L',':
		QRET(OP_COMMA);
	case L'\"':
		{
			//read const string
			wstring tmp = L"";
			while (*(++curpos))
			{
				ch = *curpos;
				if (ch == '\"')
				{
					curpos++;
					if (*curpos != '\"')
						break;
					else
					{
						tmp.push_back(ch);
					}
				}
				else if (ch == L'\n' || ch == L'\r' || ch == L'\0')
				{
					throw Var_Except(L"读字符串时遇到意料之外的结尾", static_cast<bkpulong>(curpos - exp));
				}
				else
					tmp.push_back(ch);
			}
			//字符串常量不hash了
			BKE_String _hashtmp(tmp, false);
			node.opcode = OP_CONSTVAR;
			//node.varindex = getVarIndex(_hashtmp);
			node.var = _hashtmp;
			return;
		}
		break;
	case L'0':
	case L'1':
	case L'2':
	case L'3':
	case L'4':
	case L'5':
	case L'6':
	case L'7':
	case L'8':
	case L'9':
		//read int
		//node.varindex = getVarIndex(str2num(curpos, &curpos));
		node.var = str2num(curpos, const_cast<wchar_t**>(&curpos));
		node.opcode = OP_CONSTVAR;
		return;
	case L'#':
	{
		//color number
		//RGBA
		unsigned char n[8];
		int nn = 0;
		bkpulong color = 0;
		ch = *(++curpos);
		while (nn<8)
		{
			if (L'0' <= ch && ch <= L'9')
				n[nn++] = ch - L'0';
			else if (L'a' <= ch && ch <= L'f')
				n[nn++] = ch - L'a' + 10;
			else if (L'A' <= ch && ch <= L'F')
				n[nn++] = ch - L'A' + 10;
			else
				break;
			ch = *(++curpos);
		}
		switch (nn)
		{
		case 3:
			n[3] = 15;
		case 4:
			color = 0x11000000 * n[3];
			color |= 0x110000 * n[0];
			color |= 0x1100 * n[1];
			color |= 0x11 * n[2];
			break;
		case 6:
			n[6] = 0x0F;
			n[7] = 0x0F;
		case 8:
			color |= (0x10000000 * n[6]) | (0x01000000 * n[7]);
			color |= (0x100000 * n[0]) | (0x010000 * n[1]);
			color |= (0x1000 * n[2]) | (0x0100 * n[3]);
			color |= (0x10 * n[4]) | (0x01 * n[5]);
			break;
		default:
			throw Var_Except(L"#后面只能接3,4,6,或8个十六进制数字表示颜色值", static_cast<bkpulong>(curpos - exp));
		}
		//node.varindex = getVarIndex(color);
		node.var = color;
		node.opcode = OP_CONSTVAR;
		return;
	}
	}
	if (ch == L'\'')
	{
		wchar_t startch = ch;
		//read const string
		wstring tmp = L"";
		while (*(++curpos))
		{
			ch = *curpos;
			if (ch == '\\')
			{
				bkplong s;
				if (!*(++curpos))
				{
					throw Var_Except(L"读字符串时遇到意料之外的结尾", static_cast<bkpulong>(curpos - exp));
				};
				switch (ch = *curpos)
				{
				case L'n':
					tmp.push_back(L'\n');
					break;
				case L'r':
					tmp.push_back(L'\r');
					break;
				case L't':
					tmp.push_back(L'\t');
					break;
				case L'a':
					tmp.push_back(L'\a');
					break;
				case L'b':
					tmp.push_back(L'\b');
					break;
				case L'f':
					tmp.push_back(L'\f');
					break;
				case L'v':
					tmp.push_back(L'\v');
					break;
				case L'o':
					ch = *(++curpos);
					s = 0;
					if (ch<L'0' || ch>L'9')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s += 64 * (ch - L'0');
					ch = *(++curpos);
					if (ch<L'0' || ch>L'9')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s += 8 * (ch - L'0');
					ch = *(++curpos);
					if (ch<L'0' || ch>L'9')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s += (ch - L'0');
					tmp.push_back((wchar_t)s);
					break;
				case L'x':
					ch = *(++curpos);
					s = 0;
					if (ch<L'0' || (ch>L'9' && towupper(ch)<L'A') || towupper(ch)>L'F')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s += 16 * (ch>L'9' ? towupper(ch) - L'A' + 10 : ch - L'0');
					ch = *(++curpos);
					if (ch<L'0' || (ch>L'9' && towupper(ch)<L'A') || towupper(ch)>L'F')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s += ch>L'9' ? towupper(ch) - L'A' + 10 : ch - L'0';
					tmp.push_back((wchar_t)s);
					break;
				case L'\n':
				case L'\r':
					throw Var_Except(L"读字符串时遇到意料之外的结尾", static_cast<bkpulong>(curpos - exp));
				default:
					tmp.push_back(ch);
				}
			}
			else if (ch == L'\n' || ch == L'\r' || ch == L'\0')
			{
				throw Var_Except(L"读字符串时遇到意料之外的结尾", static_cast<bkpulong>(curpos - exp));
			}
			else if (ch == startch)
			{
				curpos++;
				break;
			}
			else
				tmp.push_back(ch);
		}
		BKE_String _hashtmp(tmp, false);
		node.opcode = OP_CONSTVAR;
		//node.varindex = getVarIndex(_hashtmp);
		node.var = _hashtmp;
		return;
	}
	if (ch >= 0x80 || isalpha(ch) || ch == L'_' /*|| ch==L'$' || ch==L'#'*/)
	{
		//read variable name
		wstring tmp;
		tmp.push_back(ch);
		while ((ch = *(++curpos)))
		{
			if (ch >= 0x80 || iswalnum(ch) || ch == '_')
				tmp.push_back(ch);
			else
				break;
		}
		if (!NextIsBareWord)
		{
			if (p->constmap.find(tmp) != p->constmap.end())
			{
				//node.varindex = getVarIndex(constmap[tmp]);
				node.var = p->constmap[tmp];
				node.opcode = OP_CONSTVAR;
				return;
			}
			auto it = p->opmap.find(tmp);
			if (it != p->opmap.end())
			{
				node.opcode = it->second;
				return;
			}
		}
		NextIsBareWord = false;
		BKE_String _hashtmp(tmp);
		node.opcode = OP_LITERAL;
		//node.varindex = getVarIndex(_hashtmp);
		node.var = _hashtmp;
		return;
	}
	throw Var_Except(wstring(L"读取时遇到非法字符<") + ch + L'>', static_cast<bkpulong>(curpos - exp));
}

void PAModule::expression(BKE_bytree** tree, int rbp)
{
	if (rbp>20 && p->head[next.opcode])
	{
		throw Var_Except(L"该符号必须放在句首。", next.pos);
	}
	token = next;
	readToken();
	token.opcode += OP_COUNT;
	(this->*p->funclist[token.opcode])(tree);
	while (!forcequit && rbp < p->lbp[next.opcode])
	{
		token = next;
		readToken();
		(this->*p->funclist[token.opcode])(tree);
	}
	forcequit = false;
}