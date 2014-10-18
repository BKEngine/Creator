#ifndef WORDSUPPORT_H
#define WORDSUPPORT_H
#include <QString>
#include <QStringList>

extern QString BKE_WORD_KEY ;
extern QString BKE_WORD_OPERATOR ;

class WordSupport
{
public:
    bool atLineEnd ;
    bool atParserEnd ;
    bool IsSeparation ;
    bool more ;
    //int  type ;
    int  ptype ;
    int  ctype ;
    int  ntype ;
    QString pWord ;
    QString cWord ;
    QString nWord ;

    enum{
        BKE_WORD_TYPE_ID ,
        BKE_WORD_TYPE_KEY ,
        BKE_WORD_TYPE_OPEATOR ,
        BKE_WORD_TYPE_OTHER ,
        BKE_WORD_TYPE_NULL
    };

    WordSupport() ;
    void setText(const QString &t,int pos = 0) ;
    void Gopos(int pos) ;
    void SetParserModel(bool m){ IsParser = m ; }
    bool ParserModel(){ return IsParser ;}
    void NextLine(bool MustUseLineEnd = false) ;
    void NextWord() ;
    QString NextWord2() ;
    QString WordUntilLineEnd(int pos = -1) ;
    QString GetLine(int pos = -1 ) ;
    QString GetRightLine(int pos = -1 ) ;
    void NextTwoWord() ;
    int  ErrorPos(){ return currentpos-cWord.length() ; }
    bool IsCurrentSpace(){ return cch == QChar(' ') ; }
    bool IsCurrentType(int t){ return ctype == t ; }
    bool IsAtEnd() ;
    static bool IsNumber(const QString &t) ;
    static bool IsColor(const QString &t) ;
    static bool IsFontColor(const QString &t) ;
    static int  GetEmptyStartCount(const QString &t) ;
    int  NowAt(){ return currentpos ;}
    void RightExp(const QString &t) ;
    QStringList context(const QString &t,int pos) ;
    int WordType(const QString &w) ;
    bool IsSeparate(QChar c) ;
    bool IsLastSeparate(QChar c,int pos) ;
private:
    QString text ;
    QString Separate ;

    int currentpos ;
    bool IsParser ;
    QChar cch ;
    QChar nch ;
    QChar SafeChar(int pos) ;
    bool IsEnd() ;
    bool IsLineEnd(int pos) ;
    bool IsPLineEnd(int pos) ;

    void IgnoreEmpty() ;
    void ToLineEnd() ;
    void ToTlineEnd() ;
    void GoPos2(int pos) ;
    QString MachTo(const QString &s) ;
    QString GetWord(int from,int &end) ;
    QString WordFromSeparate() ;
    void lRightExp(const QChar &t,int &pos) ;


};

#endif // WORDSUPPORT_H
