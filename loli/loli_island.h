#ifndef LOLI_ISLAND_H
#define LOLI_ISLAND_H

enum
{
	SCE_BKE_DEFAULT,
	SCE_BKE_PARSER_DEFAULT,	//##-##
	SCE_BKE_COMMAND,
	SCE_BKE_COMMAND2,	//[
	SCE_BKE_ATTRIBUTE,
	SCE_BKE_STRING,
	SCE_BKE_STRING2,	//'
	SCE_BKE_NUMBER,
	SCE_BKE_LABEL_DEFINE,
	SCE_BKE_ANNOTATE,	//ÐÐ×¢ÊÍ
	SCE_BKE_COMMENT,	//¿é×¢ÊÍ
	SCE_BKE_OPERATORS,
	SCE_BKE_TEXT,
	SCE_BKE_PARSER,
	SCE_BKE_COLOR,		//#xxxxxx
	SCE_BKE_TRANS,		//µ¥×Ö·û×ªÒå
	SCE_BKE_PARSER_KEYWORD,
	SCE_BKE_PARSER_VAR,
	SCE_BKE_ERROR
};

#include <QtCore>

extern int LOLI_AUTOWRITE_LEN ;

namespace LOLI{
    bool AutoWrite(const QString &file,const QString &text) ;
    bool AutoWrite(const QString &name,QByteArray data) ;
    bool AutoWrite(QFile *file,const QString &text,const char *codecname) ;
    bool AutoRead(QString &text,const QString &name) ;
    bool AutoRead(QString &text,QFile *file) ;
    bool AutoRead(QByteArray &dest,const QString &name) ;
    QString isValidUTF8(QFile *file) ;
    void makeNullFile(const QStringList &list,const QString &dir ) ;
    bool P_OpenFile(QFile *file) ;
}


class QFileName
{
public:
    QFileName() ;
    QFileName(const QString &file) ;
    void setFile(const QString &file) ;
    QString filePath() ;
    void Clear() ;
    QString Path(){ return path ; }
    QString fileName(){ return filename ; }

private:
    QString filename ;
    QString path ;
    QString suffix ;
};






#endif // LOLI_ISLAND_H
