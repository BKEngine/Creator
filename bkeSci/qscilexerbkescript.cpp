#include <weh.h>
#include "qscilexerbkescript.h"

//int list_colors[] = { SCE_BKE_DEFAULT, SCE_BKE_COMMAND, SCE_BKE_ATTRIBUTE, SCE_BKE_STRING, SCE_BKE_NUMBER, SCE_BKE_COLOR, SCE_BKE_TRANS,
//SCE_BKE_LABEL, SCE_BKE_COMMENT, SCE_BKE_OPERATORS, SCE_BKE_ERROR, SCE_BKE_PARSER_KEYWORD, (1 << 6) };

QsciLexerBkeScript::QsciLexerBkeScript(QObject *parent)
	:QsciLexer(parent)
{
	ReadConfig(ConfigName());
}

const char *QsciLexerBkeScript::language() const { return "Bke" ; }
const char * QsciLexerBkeScript::lexer () const
{
	return "bke" ;
}

QString QsciLexerBkeScript::description(int style) const
{
	switch (style & 63) {
	case SCE_BKE_DEFAULT:
	case SCE_BKE_PARSER:
	case SCE_BKE_PARSER_DEFAULT: return QString("Default");
	case SCE_BKE_COMMAND:
	case SCE_BKE_COMMAND2: return QString("Command");
	case SCE_BKE_ATTRIBUTE: return QString("Attribute");
	case SCE_BKE_STRING:
	case SCE_BKE_STRING2:  return QString("String");
	case SCE_BKE_NUMBER:  return QString("Number");
	case SCE_BKE_LABEL_DEFINE:   return QString("Lable");
	case SCE_BKE_LABEL_IN_PARSER:   return QString("LableInParser");
	case SCE_BKE_ANNOTATE: return QString("LineComment");
	case SCE_BKE_COMMENT: return QString("BlockComment");
	case SCE_BKE_OPERATORS: return QString("Operators");
	case SCE_BKE_TEXT:    return QString("Text");
	case SCE_BKE_PARSER_KEYWORD:    return QString("Keyword");
	case SCE_BKE_PARSER_VAR:    return QString("Variable");
	case SCE_BKE_COLOR:    return QString("Color");
	case SCE_BKE_ERROR:    return QString("Error");
	case SCE_BKE_TRANS:  return QString("TRANS");
	}

	return QString() ;
}

QColor QsciLexerBkeScript::defaultColor (int style) const
{
	switch (style & 63)
	{
	case SCE_BKE_PARSER:
	case SCE_BKE_PARSER_DEFAULT:
	case SCE_BKE_DEFAULT: return QColor(0x00, 0x00, 0x00);
	case SCE_BKE_COMMAND:
	case SCE_BKE_COMMAND2: return QColor(0x00, 0x00, 0xff);
	case SCE_BKE_ATTRIBUTE: return QColor(0x9f, 0x3c, 0x00);
	case SCE_BKE_STRING:
	case SCE_BKE_STRING2:  return QColor(0x00, 0x9f, 0x3c);
	case SCE_BKE_NUMBER:  return QColor(0xff, 0x00, 0x00);
	case SCE_BKE_LABEL_DEFINE:   return QColor(0xff, 0x00, 0xff);
	case SCE_BKE_LABEL_IN_PARSER:   return QColor(0xff, 0x00, 0xff);
	case SCE_BKE_ANNOTATE: return QColor(0x80, 0x80, 0x80);
	case SCE_BKE_COMMENT: return QColor(0x80, 0x80, 0x80);
	case SCE_BKE_OPERATORS: return QColor(0x00, 0x00, 0x00);
	case SCE_BKE_TEXT:    return QColor(0x00, 0x00, 0x00);
	case SCE_BKE_COLOR:    return QColor(43, 145, 175);
	case SCE_BKE_TRANS:  return QColor(0x7a, 0x0f, 0xd1); //转义字符，
	case SCE_BKE_PARSER_KEYWORD:    return QColor(0x00, 0x00, 0xff);
	case SCE_BKE_PARSER_VAR:    return QColor(47, 79, 79);
	case SCE_BKE_ERROR:    return QColor(0xff, 0x00, 0x00);
	}
	return QColor( 0x00,0x00,0x00) ;
}

QColor QsciLexerBkeScript::color(int style) const
{
	int c = style & 63;
	if(c >= SCE_BKE_COLOR_COUNT)
		return QColor(0x00, 0x00, 0x00);
	return hlb[c].fc;
	//switch (style & 63)
	//{
	//case SCE_BKE_PARSER:
	//case SCE_BKE_PARSER_DEFAULT:
	//case SCE_BKE_DEFAULT: return hlb[SCE_BKE_DEFAULT].fc;
	//case SCE_BKE_COMMAND:
	//case SCE_BKE_COMMAND2: return hlb[1].fc;
	//case SCE_BKE_ATTRIBUTE: return hlb[2].fc;
	//case SCE_BKE_STRING:
	//case SCE_BKE_STRING2:  return hlb[3].fc;
	//case SCE_BKE_NUMBER:  return hlb[4].fc;
	//case SCE_BKE_LABEL:   return hlb[7].fc;
	//case SCE_BKE_LABEL_IN_PARSER:   return hlb[7].fc;
	//case SCE_BKE_ANNOTATE:
	//case SCE_BKE_COMMENT: return hlb[8].fc;
	//case SCE_BKE_OPERATORS: return hlb[9].fc;
	//case SCE_BKE_TEXT:    return QColor(0x00, 0x00, 0x00);
	//case SCE_BKE_COLOR:    return hlb[5].fc;
	//case SCE_BKE_TRANS:  return hlb[6].fc; //转义字符，
	//case SCE_BKE_PARSER_KEYWORD:    return hlb[11].fc;
	//case SCE_BKE_PARSER_VAR:    return QColor(47, 79, 79);
	//case SCE_BKE_ERROR:    return hlb[10].fc;
	//}
	//return QColor(0x00, 0x00, 0x00);
}

QColor QsciLexerBkeScript::defaultPaper(int style) const
{
	if ((style & (1 << 6)) && (!(style & (1 << 7))) || style == SCE_BKE_PARSER_BG)
		return QColor(0xE0, 0xE0, 0xE0);
	return QsciLexer::defaultPaper();
}

QColor QsciLexerBkeScript::paper(int style) const
{
	if ((style & (1 << 6)) && (!(style & (1 << 7))))
		return hlb[SCE_BKE_PARSER_BG].bc;
	int c = style & 63;
	if (c >= SCE_BKE_COLOR_COUNT)
		return QsciLexer::defaultPaper();
	return hlb[c].bc;
	//switch (style & 63)
	//{
	//case SCE_BKE_PARSER:
	//case SCE_BKE_PARSER_DEFAULT:
	//case SCE_BKE_TEXT:
	//case SCE_BKE_DEFAULT: return hlb[0].bc;
	//case SCE_BKE_COMMAND:
	//case SCE_BKE_COMMAND2: return hlb[1].bc;
	//case SCE_BKE_ATTRIBUTE: return hlb[2].bc;
	//case SCE_BKE_STRING:
	//case SCE_BKE_STRING2:  return hlb[3].bc;
	//case SCE_BKE_NUMBER:  return hlb[4].bc;
	//case SCE_BKE_LABEL:   return hlb[7].bc;
	//case SCE_BKE_LABEL_IN_PARSER:   return hlb[7].bc;
	//case SCE_BKE_ANNOTATE:
	//case SCE_BKE_COMMENT: return hlb[8].bc;
	//case SCE_BKE_OPERATORS: return hlb[9].bc;
	//case SCE_BKE_COLOR:    return hlb[5].bc;
	//case SCE_BKE_TRANS:  return hlb[6].bc; //转义字符，
	//case SCE_BKE_PARSER_KEYWORD:    return hlb[11].bc;
	//case SCE_BKE_PARSER_VAR:    return QsciLexer::defaultPaper();
	//case SCE_BKE_ERROR:    return hlb[10].bc;
	//}
	//return QsciLexer::defaultPaper();
}

QFont QsciLexerBkeScript::font (int style) const
{
	return Lfont ;
}

QFont QsciLexerBkeScript::defaultFont (int style) const
{
	return Lfont ;
}

QStringList QsciLexerBkeScript::autoCompletionWordSeparators() const
{
	QStringList ls ;
	return ls ;
}

const char * QsciLexerBkeScript::blockStart (int *style ) const
{
	return "{ @if if while @for for foreach" ;
}

const char *QsciLexerBkeScript::keywords(int set) const
{
	if (set == 2)	//parser
	{
		return "if for while do foreach function class propget propset continue break return var delete try throw this super global int string number const typeof instanceof extends in else then catch with static switch case";
	}
	else
		return NULL;
}

//返回配置项的名字
QString QsciLexerBkeScript::ConfigName()
{
	QString temp = BKE_USER_SETTING->value("SyleCurrent",QString("默认")).toString() ;
	if( ConfigVoid(temp) ) return temp ;
	else return "默认" ;
}

//返回配置列表
QStringList QsciLexerBkeScript::ConfigList()
{
	QStringList res,temp = BKE_USER_SETTING->childGroups() ;
	for( int i = 0 ;i < temp.size();i++){
		if( ConfigVoid( temp.at(i)) ){
			res.append( temp.at(i) );
		}
	}
	res.prepend("默认");
	return res ;
}

bool QsciLexerBkeScript::ConfigVoid(QString stylename)
{
	return BKE_USER_SETTING->value(stylename+"/valid",false).toBool() ;
}


//从文件中读取配置
void QsciLexerBkeScript::ReadConfig(QString hname)
{
	QStringList v = BKE_USER_SETTING->value(hname+"/colour", QStringList()).toStringList();
	if (v.empty() || SCE_BKE_COLOR_COUNT * 2 > v.size())
	{
		for (int i = 0; i < SCE_BKE_COLOR_COUNT; i++)
		{
			hlb[i].font = Lfont;
			hlb[i].fc = defaultColor(i).rgb();
			hlb[i].bc = defaultPaper(i).rgb();
		}

		QStringList c;
		for (int i = 0; i < SCE_BKE_COLOR_COUNT; i++)
		{
			c.push_back(QString("%1").arg(hlb[i].fc));
			c.push_back(QString("%1").arg(hlb[i].bc));
		}
		BKE_USER_SETTING->setValue(hname+"/colour", c);
		return;
	}

	for (int i = 0; i < SCE_BKE_COLOR_COUNT; i++)
	{
		hlb[i].font = Lfont;
		hlb[i].fc = v[2 * i].toUInt();
		hlb[i].bc = v[2 * i + 1].toUInt();
	}

	Lfont.setFamily(BKE_USER_SETTING->value(hname+"/font", "Consolas").toString());
	Lfont.insertSubstitution("Consolas","微软雅黑");
	Lfont.setPointSize(BKE_USER_SETTING->value(hname+"/size", "13").toInt());
}
