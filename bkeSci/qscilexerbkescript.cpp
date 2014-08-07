#include "qscilexerbkescript.h"

QsciLexerBkeScript::QsciLexerBkeScript(QObject *parent)
    :QsciLexer(parent)
{
    Lfont.setFamily("微软雅黑");
    Lfont.setPointSize(12);
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
    case UntypeA:  return QColor( 0x80,0x80,0x00 ) ;
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
