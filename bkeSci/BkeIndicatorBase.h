#ifndef BKEINDICATORBASE_H
#define BKEINDICATORBASE_H

#include <QString>

class BkeIndicatorBase
{
public:
    BkeIndicatorBase(){ start = -1 ; end = -1 ; len = -1 ; }
    BkeIndicatorBase(int s,int e){ start = s ; end = e ; len = end-start ; }
    void PosChange(int i){ start += i ; end += i ; }
    void SetStart(int i){ start = i ; len = end - start ; }
    void SetEnd(int i){ end = i ; len = end - start ; }
    int Start(){ return start ; }
    int End(){ return end ; }
    int Len(){ return len ; }
    bool IsNull(){ return (start < 0 || end < 0 || len < 0) ; }
    void Clear(){ start = -1 ; end = -1 ; len = -1 ; }
private:
    int start ;
    int end ;
    int len ;
};
class BkeModifiedBase
{
public:
    int pos ;
    int type ;
    int line ;
    int index ;
    int lineadd ;
    QString text ;

    void clear(){
        pos = 0 ;
        type = 0 ;
        line = 0 ;
        lineadd = 0 ;
        text.clear();
    }
    bool isEmpty(){ return text.isEmpty() || pos < 1 ; }
    int  EndPos(){ return index + text.length() ; }
};

#endif // BKEINDICATORBASE_H
