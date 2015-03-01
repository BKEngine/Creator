#include <weh.h>
#include "bkestyle.h"

BKEstyle::BKEstyle(QObject *parent)
    :QsciLexer(parent)
{
}

const char* BKEstyle::language() const
{
    return "Bke" ;
}

const char *BKEstyle::lexer() const
{
    return "bke" ;
}

QString BKEstyle::description(int style ) const
{
    switch (style) {
    case S_DEFAULT: return QString("Default") ;
    case S_COMMAND: return QString("Command") ;
    case S_ATTRIBUTE: return QString("Attribute") ;
    case S_STRING:  return QString("String") ;
    case S_NUMBER:  return QString("Number") ;
    case S_LABEL:   return QString("Lable") ;
    case S_ANNOTATE: return QString("Annotate") ;
    case S_OPERATORS: return QString("Operators") ;
    case S_TEXT :    return QString("Text") ;
    case S_ERROR:    return QString("Error") ;
    case S_UNTYPEA:  return QString("Untypea") ;
    }

    return QString() ;
}

QColor BKEstyle::defaultColor (int style) const
{
    switch (style) {
    case S_DEFAULT: return QColor(0,0,0) ;
    case S_COMMAND: return QColor( 0x00,0x00,0xff) ;
    case S_ATTRIBUTE: return QColor( 0x9f,0x3c,0x00) ;
    case S_STRING:  return QColor( 0x00,0x9f,0x3c) ;
    case S_NUMBER:  return QColor( 0xff,0x00,0x00) ;
    case S_LABEL:   return QColor( 0xff,0x00,0xff) ;
    case S_ANNOTATE: return QColor( 0x80,0x80,0x80) ;
    case S_OPERATORS: return QColor( 0x00,0x00,0x00) ;
    case S_TEXT :    return QColor( 0x00,0x00,0x00) ;
    case S_ERROR:    return QColor( 0xff,0x00,0x00) ;
    case S_UNTYPEA:  return QColor( 0x80,0x80,0x00 ) ;
    default: return QColor(0,0,0) ;
    }
}
