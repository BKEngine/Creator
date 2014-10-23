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
    switch (style) {
    case Default: return QString("Default") ;
    case Command: return QString("Command") ;
    case Attribute: return QString("Attribute") ;
    case String:  return QString("String") ;
    case Number:  return QString("Number") ;
    case Lable:   return QString("Lable") ;
    case Annotate: return QString("Annotate") ;
    case Operators: return QString("Operators") ;
    case Text :    return QString("Text") ;
    case Error:    return QString("Error") ;
    case UntypeA:  return QString("Untypea") ;
    }

    return QString() ;
}

QColor QsciLexerBkeScript::defaultColor (int style) const
{
    switch (style) {
    case Default: return QColor( 0x00,0x00,0x00) ;
    case Command: return QColor( 0x00,0x00,0xff) ;
    case Attribute: return QColor( 0x9f,0x3c,0x00) ;
    case String:  return QColor( 0x00,0x9f,0x3c) ;
    case Number:  return QColor( 0xff,0x00,0x00) ;
    case Lable:   return QColor( 0xff,0x00,0xff) ;
    case Annotate: return QColor( 0x80,0x80,0x80) ;
    case Operators: return QColor( 0x00,0x00,0x00) ;
    case Text :    return QColor( 0x00,0x00,0x00) ;
    case Error:    return QColor( 0xff,0x00,0x00) ;
    case UntypeA:  return QColor( 0x80,0x80,0x00 ) ; //保留字，
    }
    return QColor( 0x00,0x00,0x00) ;
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
