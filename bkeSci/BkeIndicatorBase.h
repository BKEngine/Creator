#ifndef BKEINDICATORBASE_H
#define BKEINDICATORBASE_H

#include <QString>

class BkeIndicatorBase
{
public:
    BkeIndicatorBase(){ start = -1 ; end = -1 ; }
    BkeIndicatorBase(int s,int e){ start = s ; end = e ; }
    void PosChange(int i){ start += i ; end += i ; }
    void SetStart(int i){ start = i ;}
    void SetEnd(int i){ end = i ;  }
    int Start() const { return start ; }
    int End() const { return end ; }
    int Len() const { return start >=0 && end >=0 ? end - start : -1; }
    bool IsNull() const { return (start < 0 || end < 0) ; }
    void Clear(){ start = -1 ; end = -1 ; }
	bool operator == (const BkeIndicatorBase &r) const { return start == r.start && end == r.end; }
	bool operator != (const BkeIndicatorBase &r) const { return start != r.start || end != r.end; }
	operator bool() const { return !IsNull(); }
private:
    int start ;
    int end ;
};
class BkeModifiedBase
{
public:
    int pos ;
	int endpos;
    int type ;
    int line ;
    int index ;
    int lineadd ;
    QString text ;
	QByteArray bytes;
    BkeModifiedBase(){clear();}

    void clear(){
        pos = 0 ;
		endpos = 0;
        type = 0 ;
        line = 0 ;
        lineadd = 0 ;
        text.clear();
    }
    bool isEmpty(){ return text.isEmpty() || pos < 1 ; }
    int  EndPos(){ return endpos ; }
};

#endif // BKEINDICATORBASE_H
