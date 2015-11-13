#include "wordsupport.h"
#include <QDebug>

QString BKE_WORD_KEY = QString(" for foreach in extends do while function propset propget int string number typeof var delete class if else continue break return this true false void global ");
QString BKE_WORD_OPERATOR = QString("+,-,*,/,^,%,!,~,++,--,&,|,&&,||");

WordSupport::WordSupport()
{
	Separate = QString(" ~!@#$%^&*()-+*/|{}[]:;/=.,?><\\");
	atLineEnd = false;
	atParserEnd = false;
}

void WordSupport::setText(const QString &t, int pos)
{
	text = t;
	IsParser = false;   //默认为@模式
	Gopos(pos);
	IgnoreEmpty();
}

void WordSupport::Gopos(int pos)
{
	cch = SafeChar(pos);
	nch = SafeChar(pos + 1);
	currentpos = pos;
	more = (currentpos < text.length());
	atLineEnd = IsLineEnd(currentpos);
	atParserEnd = IsPLineEnd(currentpos);
	IsSeparation = IsSeparate(cch);
	if (!more){
		atLineEnd = true;
		atParserEnd = true;
	}
}


void WordSupport::GoPos2(int pos)
{
	Gopos(pos);
	IgnoreEmpty();
	cch = SafeChar(pos);
	nch = SafeChar(pos + 1);
	currentpos = pos;
	IsSeparation = IsSeparate(cch);
}

QChar WordSupport::SafeChar(int pos)
{
	if (pos < text.length()) return text.at(pos);
	else return QChar();
}


void WordSupport::IgnoreEmpty()
{
	if (cch == QChar('/') && nch == QChar('/')) ToLineEnd();
	while (cch.unicode() < 33 && more && !atLineEnd){  //处于命令模式下，到达换行，跳出
		Gopos(currentpos + 1);
		if (cch == QChar('/') && nch == QChar('/')) ToLineEnd();
	}

	//parser模式下，换行是被忽略的
	if (IsParser && more && atLineEnd){
		NextLine(true);
		IgnoreEmpty();
	}
}

//到底自然化的结束"\r\n"
void WordSupport::ToLineEnd()
{
	while (more && cch != QChar('\r') && cch != QChar('\n')) Gopos(currentpos + 1);
}

//到行结束
void WordSupport::ToTlineEnd()
{
	while (more && !IsLineEnd(currentpos))  Gopos(currentpos + 1);
}

//参数决定是否强制使用换行符作为下一行标记(parser使用；作为标记)
void WordSupport::NextLine(bool MustUseLineEnd)
{
	if (MustUseLineEnd){
		while (!atLineEnd && more) Gopos(currentpos + 1);
		while (atLineEnd && more) Gopos(currentpos + 1);
	}
	else if (IsParser){
		while (!atParserEnd && more) Gopos(currentpos + 1);  //parser的下一行是特殊的
		while (atParserEnd && more) Gopos(currentpos + 1);
	}
	else{
		while (!atLineEnd && more) Gopos(currentpos + 1);
		while (atLineEnd && more) Gopos(currentpos + 1);
	}

	//    int k = NowAt() ;
	//    NextWord();
	//    Gopos(k);
}

bool WordSupport::IsSeparate(QChar c)
{
	return (Separate.indexOf(c) >= 0);
}

void WordSupport::NextWord()
{
	int end;
	pWord = cWord;
	ptype = ctype;
	cWord = GetWord(currentpos, end);
	ctype = WordType(cWord);
	Gopos(end);
	int k = end;
	nWord = GetWord(currentpos, end);
	ntype = WordType(nWord);
	GoPos2(k);
}


QString WordSupport::GetWord(int from, int &end)
{
	end = currentpos;
	if (from >= text.length()) return QString();
	Gopos(from);
	IgnoreEmpty();

	end = from = currentpos;
	if (IsAtEnd()) return QString();

	if (cch == QChar('\"')){
		QString temp = MatchToQuote(end);
		//end = from + temp.length() ;
		Gopos(end);
		return temp;
	}
	else if (cch == QChar('\'')){
		QString temp = MatchToSingleQuote(end);
		//end = from + temp.length() ;
		Gopos(end);
		return temp;
	}
	else if (IsSeparation){
		QString temp = WordFromSeparate();
		end = from + temp.length();
		Gopos(from);
		return temp;
	}
	else{
		while (more && !atLineEnd && !IsSeparation) Gopos(currentpos + 1);
		end = currentpos;
		Gopos(from);
		return text.mid(from, end - from);
	}
}

QString WordSupport::WordFromSeparate()
{
	int pos = QString("+-*/&|#").indexOf(cch);
	if (pos < 0) return QString(cch);
	else if (cch != nch) return QString(cch);
	else{
		int a = currentpos;
		QChar c = cch;
		while (c == cch) Gopos(currentpos + 1);
		return text.mid(a, currentpos - a);
	}
}

void WordSupport::NextTwoWord()
{
	NextWord();
	NextWord();
}

QString WordSupport::MatchToQuote(int &end)
{
	int be = currentpos;
	int pos = text.indexOf(QChar('"'), currentpos + 1);
	if (pos < 0)
	{
		end = text.length();
		return text.mid(be).replace("\"\"", "\"");
	}

	while (SafeChar(pos + 1) == QChar('"')){
		pos = text.indexOf(QChar('"'), pos + 2);
		if (pos < 0)
		{
			end = text.length();
			return text.mid(be).replace("\"\"", "\"");
		}
	}
	Gopos(be);
	end = pos + 1;
	return text.mid(be, pos - be + 1).replace("\"\"", "\"");
}

QString WordSupport::MatchToSingleQuote(int &end)
{
	int be = currentpos;
	int pos = text.indexOf(QChar('\''), currentpos + 1);
	if (pos < 0)	
	{
		end = text.length();
		return text.mid(be);
	}


	while (SafeChar(pos - 1) == QChar('\\')){
		pos = text.indexOf(QChar('\''), pos + 1);
		if (pos < 0)
		{
			end = text.length();
			return text.mid(be);
		}

	}
	Gopos(be);
	end = pos + 1;
	return text.mid(be, pos - be + 1);
}

int WordSupport::WordType(const QString &w)
{
	if (w.isEmpty()) return BKE_WORD_TYPE_NULL;
	if (BKE_WORD_KEY.indexOf(" " + w + " ") >= 0) return BKE_WORD_TYPE_KEY;
	else if (BKE_WORD_OPERATOR.indexOf(w + ",") >= 0) return BKE_WORD_TYPE_OPEATOR;
	else if (IsSeparate(w.at(0))) return BKE_WORD_TYPE_OTHER;
	else return BKE_WORD_TYPE_ID;
}

QString WordSupport::NextWord2()
{
	NextWord();
	return cWord;
}


bool WordSupport::IsLineEnd(int pos)
{
	QChar a, b, c;

	a = SafeChar(pos);
	b = SafeChar(pos + 1);
	c = SafeChar(pos + 2);
	if (a != QChar('\r') && a != QChar('\n')) return false;
	else if (a == QChar('\r') && b == QChar('\n') && c == QChar('#')) return false;
	else if (a == QChar('\n') && b == QChar('#')) return false;
	else return true;
}

bool WordSupport::IsPLineEnd(int pos)
{
	QChar a;
	a = SafeChar(pos);
	if (a == QChar(';')) return true;
	else return false;
}

bool WordSupport::IsAtEnd()
{
	if (IsParser) return atParserEnd;
	else return atLineEnd;
}

QString WordSupport::WordUntilLineEnd(int pos)
{
	if (pos == -1) pos = currentpos;
	int a = currentpos;
	ToTlineEnd();
	int b = currentpos;
	Gopos(a);
	return text.mid(pos, b - pos);
}

//从后往前历遍
QStringList WordSupport::context(const QString &t, int pos)
{
	QStringList list;
	if (pos < 0) return list;
	if (pos >= t.length()) return list;
	setText(t);

	for (int i = pos; i >= 0; i--){
		Gopos(i);
		if (!IsSeparate(text.at(i))) continue;
		else if (cch == QChar('.')){
			Gopos(i + 1);
			list.prepend(NextWord2());
			list.prepend(".");
		}
		else if (cch == QChar(']')){
			lRightExp(QChar('['), --i);
			i--;
		}
		else if (i + 1 >= text.length()) return list;
		else{
			Gopos(i + 1);
			list.prepend(NextWord2());
			list.prepend(text.at(i));   //取得最后一个符号
			return list;
		}
	}

	Gopos(0);
	list.prepend(NextWord2());
	list.prepend(" "); //取得最后一个符号
	return list;
}

void WordSupport::lRightExp(const QChar &t, int &pos)
{
	QChar a;
	while (pos >= 0){
		a = text.at(pos);
		if (a == t) return;
		else if (a == QChar(']')) lRightExp(QChar('['), --pos);
		else if (a == QChar(')')) lRightExp(QChar('('), --pos);
		else if (a == QChar('{') || a == QChar('}') || a == QChar(';')) return;
		pos--;
	}
}

void WordSupport::RightExp(const QString &t)
{
	NextWord();
	while (more && !IsAtEnd()){
		if (cWord == t) return;
		else if (cWord == "(") RightExp(")");
		else if (cWord == "[") RightExp("]");
		else if (cWord == "{" || cWord == "}" || cWord == ";") return;
		NextWord();
	}
}

bool WordSupport::IsNumber(const QString &t)
{
	bool ok;
	t.toInt(&ok, 0);
	if (!ok)
	{
		t.toDouble(&ok);
	}
	return ok;
}

bool WordSupport::IsColor(const QString &t)
{
	int start = 0;
	if (t.startsWith("#"))
	{
		int len = t.length();
		if (len == 3 || len == 5 || len == 7 || len == 9)
		{
			QString s = t.mid(1);
			bool ok;
			t.toUInt(&ok, 16);
			return ok;
		}
		return false;
	}
	else
	{
		bool ok;
		t.toUInt(&ok, 0);
		return ok;
	}
}

bool WordSupport::IsFontColor(const QString &t)
{
	QString s = t.toLower();
	if(s.size()>=2 && s.left(1)==QChar('"') && s.right(1)==QChar('"'))
		s = s.mid(1, s.size()-2);
	if(s.isEmpty())
		return true;
	if (IsColor(s))
		return true;

	int pos = s.indexOf(',');
	if (pos != -1)
	{
		if (!IsColor(s.mid(0, pos).trimmed()))
			return false;
		if (!IsColor(s.mid(pos + 1).trimmed()))
			return false;
		return true;
	}
	return false;
}

int  WordSupport::GetEmptyStartCount(const QString &t)
{
	int i;
	for (i = 0; i < t.size() && t.at(i).unicode() < 33; i++);
	if (i == t.size()) return 0;
	else return i;
}

bool WordSupport::IsLastSeparate(QChar c, int pos)
{
	if (pos >= text.length()) pos = text.length() - 1;

	for (int i = pos; i >= 0; i--){
		if (IsSeparate(text.at(i))){
			if (text.at(i) == c) return true;
			else return false;
		}
	}
	return false;
}

//获取指定位置的所在行，小于0为当前行，其他不存在时返回空
QString WordSupport::GetLine(int pos)
{
	if (pos < 0) pos = NowAt();
	if (pos < 0 || pos >= text.length()) return QString();

	int ef, ee;
	ef = text.lastIndexOf("\n", pos);
	if (ef < 0) ef = 0;
	ee = text.indexOf("\n");
	if (ee < 0) ee = text.length();
	return text.mid(ef, ee - ef).trimmed();
}


QString WordSupport::GetRightLine(int pos)
{
	if (pos < 0) pos = NowAt();
	if (pos < 0 || pos >= text.length()) return QString();

	int ee = text.indexOf("\n");
	if (ee < 0) ee = text.length();
	return text.mid(pos, ee - pos).trimmed();
}
