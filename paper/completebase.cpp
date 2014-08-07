#include "completebase.h"

CompleteBase::CompleteBase(QObject *parent)
    :QObject(parent)
{
    type = BKE_TYPE_NORMAL ;
}

CompleteBase::~CompleteBase()
{
    clearChild();
}

CompleteBase *CompleteBase::AddChild(const QString &n,int type)
{

    if( Contain(n,type) ) return indexOf(n,type);
    CompleteBase *c = indexOf(n,type) ;
    if( c != 0) return c ;

    c = new CompleteBase ;
    c->Name = n ;
    c->type = type ;

    hash[ (n+"?%1").arg(type) ] = c ;
    return c ;
}


bool CompleteBase::Contain(const QString &w,int type)
{
    QString temp =  (w+"?%1").arg(type)  ;
    return ( hash.contains(temp)) ;
}

CompleteBase *CompleteBase::indexOf(const QString &name,int type)
{
    return hash.value((name+"?%1").arg(type),0) ;
}


void CompleteBase::clear()
{
    Name.clear();
    type = BKE_TYPE_NORMAL ;
    resulttype = BKE_TYPE_NORMAL ;
    apiname.clear(); ;
    functioninfo.clear();
}

void CompleteBase::clearChild()
{
    for( auto ptr = hash.begin() ; ptr != hash.end() ; ptr++ ){
        CompleteBase *le = ptr.value() ;
        le->deleteLater();
    }
}

QStringList CompleteBase::TheWords(int type )
{
    QStringList ls(hash.keys()) ;
    return ls ;
//    if( type < 0) return ls ;

//    QRegExp exp(QString("*?%1").arg(type),Qt::CaseInsensitive) ;
//    return ls.indexOf(exp) ;
}
