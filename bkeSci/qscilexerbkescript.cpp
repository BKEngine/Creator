#include "qscilexerbkescript.h"

QsciLexerBkeScript::QsciLexerBkeScript(QObject *parent)
    :QsciLexer(parent)
{
    Lfont.setFamily("微软雅黑");
    Lfont.setPointSize(12);
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
	case SCE_BKE_STRING:  return QString("String");
	case SCE_BKE_STRING2:  return QString("String");
	case SCE_BKE_NUMBER:  return QString("Number");
	case SCE_BKE_LABEL:   return QString("Lable");
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
	case SCE_BKE_DEFAULT: return QColor(0x00, 0x00, 0x00);
	case SCE_BKE_PARSER_DEFAULT: return QColor(0x00, 0x00, 0x00);
	case SCE_BKE_COMMAND: return QColor(0x00, 0x00, 0xff);
	case SCE_BKE_COMMAND2: return QColor(0x00, 0x00, 0xff);
	case SCE_BKE_ATTRIBUTE: return QColor(0x9f, 0x3c, 0x00);
	case SCE_BKE_STRING:  return QColor(0x00, 0x9f, 0x3c);
	case SCE_BKE_STRING2:  return QColor(0x00, 0x9f, 0x3c);
	case SCE_BKE_NUMBER:  return QColor(0xff, 0x00, 0x00);
	case SCE_BKE_LABEL:   return QColor(0xff, 0x00, 0xff);
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

QColor QsciLexerBkeScript::defaultPaper(int style) const
{
	if ((style & (1 << 6)) && (!(style & (1 << 7))))
		return QColor(0xE0, 0xE0, 0xE0);
	if (style == 65)
		return QColor(0xE0, 0xE0, 0xE0);
	return QsciLexer::defaultPaper();
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
    QString hname = BKE_USER_SETTING->value("sys/Highlight",QString("默认")).toString() ;
    return hname ;
}

//返回配置列表
QStringList QsciLexerBkeScript::ConfigList()
{
    QString temp = BKE_USER_SETTING->value("sys/HighlightList").toString() ;
    if( temp.isEmpty() ) return QStringList() ;
    else return temp.split("&&") ;
}


//从文件中读取配置
void QsciLexerBkeScript::ReadConfig(QString hname)
{
    if( hname == "默认" ){
        for( int i = 0 ; i < 32 ; i++ ){
            hlb[i].font = defaultFont(0) ;
            hlb[i].fc = defaultColor(i).rgb() ;
            hlb[i].bc = qRgb(255,255,255) ;
        }
        return ;
    }

    QString akb ;
    QFont ft ;
    for( int i = 0 ; i < 32 ; i++ ){
        akb.setNum(i) ;
        ft.fromString(BKE_USER_SETTING->value(hname+"/"+akb+"_font").toString()) ;
        hlb[i].font = ft ;
        hlb[i].fc   = BKE_USER_SETTING->value(hname+"/"+akb+"_fc").toUInt();
        hlb[i].bc   = BKE_USER_SETTING->value(hname+"/"+akb+"_bc").toUInt();
    }
}
