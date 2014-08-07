#ifndef BKEPARSER_H
#define BKEPARSER_H

#include <QString>
#include <QByteArray>

class BkeParser
{
public:
    BkeParser();

    void ParserText(const QString &text) ;

private:
    unsigned char pch ;
    unsigned char cch ;
    unsigned char nch ;
    int currentpos ;
    QString Text ;
    void Forword() ;

};

#endif // BKEPARSER_H
