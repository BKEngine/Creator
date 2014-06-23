#ifndef COMPLETEBASE_H
#define COMPLETEBASE_H
#include <QtCore>
#include <QRegExp>

class CompleteBase ;

typedef QHash<QString,CompleteBase*> CompleteHash ;

class CompleteBase :public QObject
{
public:

        enum{
            BKE_TYPE_NORMAL ,
            BKE_TYPE_CLASS ,
            BKE_TYPE_DICTIONARIES ,
            BKE_TYPE_FUNCTION ,
            BKE_TYPE_COMMAND ,
            BKE_TYPE_SYS ,
            BKE_TYPE_LABEL
        };
    CompleteBase(QObject *parent=0);
    ~CompleteBase() ;
    QString Name ;
    int  type ;
    int  resulttype ;
    int pos ;
    QString apiname ;
    QString functioninfo ;
    CompleteHash hash ;

    //有的话加返回自身，没有的话加入
    CompleteBase *AddChild(const QString &n,int type = BKE_TYPE_NORMAL) ;

    bool Contain(const QString &w,int type) ;
    bool hasChild(){ return (hash.size() > 0) ; }
    CompleteBase *indexOf(const QString &name,int type) ;
    void clear() ;
    void clearChild() ;
    CompleteHash *ThisHash(){ return &hash ; }
    QStringList TheWords(int type = -1) ;
private:

};

#endif // COMPLETEBASE_H
