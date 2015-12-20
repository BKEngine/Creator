#ifndef PARSERHELPER_H
#define PARSERHELPER_H

#include "parserhelper_global.h"
#include <QtCore>
#include <QVariant>
#include <QMap>
#include "config.h"

class Var_Except;
class QBkeVarExcept
{
    QString msg;
public:
    QBkeVarExcept(const Var_Except &e);
    QBkeVarExcept(const QString &e):msg(e){}
    QString getMsg();
};

class QBkeVariable;
typedef QMap<QString, QBkeVariable> QBkeDictionary;
typedef QList<QBkeVariable> QBkeArray;
typedef QVector<QBkeVariable> QBkeVector;

class BKE_Variable;
class QBkeVariableRef
{
protected:
    BKE_Variable *_var;
    friend class QBkeVariable;
public:
    QBkeVariable value() const;
    QBkeVariableRef(BKE_Variable *v = NULL):_var(v){}
    //QBkeVariableRef(BKE_Variable &v):_var(&v){}
    QBkeVariableRef(const QBkeVariableRef &r){_var = r._var;}
    bool operator == (const QBkeVariableRef &r) const;
    QBkeVariableRef &operator = (const QBkeVariableRef &r);
    QBkeVariableRef &operator = (const QBkeVariable &r);
    QBkeVariableRef operator - (const QBkeVariableRef &r);
    QBkeVariableRef operator - (const QBkeVariable &r);
    QBkeVariableRef &operator -= (const QBkeVariableRef &r);
    QBkeVariableRef &operator -= (const QBkeVariable &r);
    void redirect(QBkeVariable &v);
    void redirect(const QBkeVariableRef &v){_var = v._var;}

    const QBkeVariable operator [](int i) const;
    const QBkeVariable operator [](const QString &k) const;
    QBkeVariableRef operator [](int i);
    QBkeVariableRef operator [](const QString &k);

    int getCount()const;
    void append(const QBkeVariable &v);
    void insert(const QString &key, const QBkeVariable &value){if(_var) (*this)[key]=value;}
    void insert(int i, const QBkeVariable &_v);
    void remove(const QString &key);
    void remove(int i);
    const QBkeVariable value(int i) const ;
    const QBkeVariable value(const QString &k) const ;
    QBkeVariableRef value(int i){ if(!_var)return QBkeVariableRef();return (*this)[i];}
    QBkeVariableRef value(const QString &k){ if(!_var)return QBkeVariableRef();return (*this)[k];}

    bool isNull()const;
    bool isVoid()const{return isNull();}
    bool isDic()const;
    bool isArray() const;
    bool isNum() const;
    bool isString() const;
    bool valid()const{return !!_var;}

    int toInt() const;
    QString toString() const;
    QStringList toStringList() const;
    qreal toReal() const;
    bool toBool() const;
    QBkeDictionary toBkeDic() const;
    QBkeArray toBkeArray() const;
    QBkeVector toBkeVector() const;
};

class QBkeVariable
{  
protected:
    BKE_Variable *_var;
    friend class QBkeVariableRef;
public:
    QBkeVariable();
    ~QBkeVariable();
    QBkeVariable(const QVariant &v);
    QBkeVariable(const char *c):QBkeVariable(QString(c)){}
    QBkeVariable(const QString &s);
    QBkeVariable(const QStringList &list);
    QBkeVariable(const QVariantMap &map);
    QBkeVariable(const QVariantHash &h);
    QBkeVariable(const QVariantList &list);
    QBkeVariable(const QBkeDictionary &list);
    QBkeVariable(const QBkeArray &list);
    QBkeVariable(int i);
    QBkeVariable(qreal d);
    QBkeVariable(bool b);
    QBkeVariable(const QBkeVariableRef &v);
    QBkeVariable(const QBkeVariable &v);
    QBkeVariable(const BKE_Variable &v);
    QBkeVariable &operator = (const QBkeVariable &v);
    QBkeVariable &operator = (const QBkeVariableRef &v);
    QBkeVariable &operator - (const QBkeVariable &v);
    QBkeVariable &operator - (const QBkeVariableRef &v);
    QBkeVariable &operator -= (const QBkeVariable &v);
    QBkeVariable &operator -= (const QBkeVariableRef &v);
    operator BKE_Variable &(){return *_var;}
    bool operator == (const QBkeVariable &r) const;
    const QBkeVariable operator [](int i) const;
    const QBkeVariable operator [](const QString &k) const;
    QBkeVariableRef operator [](int i);
    QBkeVariableRef operator [](const QString &k);

    int getCount()const;
    void append(const QBkeVariable &v);
    void insert(const QString &key, const QBkeVariable &value){(*this)[key]=value;}
    void insert(int i, const QBkeVariable &_v);
    const QBkeVariable value(int i) const { return (*this)[i];}
    const QBkeVariable value(const QString &k) const { return (*this)[k];}
    QBkeVariableRef value(int i){ return (*this)[i];}
    QBkeVariableRef value(const QString &k){ return (*this)[k];}
    void remove(const QString &key);
    void remove(int i);
    QStringList getKeys() const;
    QBkeVariableRef ref(){return _var;}

    QByteArray saveToBinary()const {return saveToString().toUtf8();}
    QString saveToString()const ;
    void loadFromBinary(const QByteArray &b);
    void loadFromString(const QString &s);
    void loadClosureDicFromString(const QString &s);

    void setVoid();

    int toInt() const;
    QString toString() const;
    QStringList toStringList() const;
    qreal toReal() const;
    bool toBool() const;
    QBkeDictionary toBkeDic() const;
    QBkeArray toBkeArray() const;
    QBkeVector toBkeVector() const;

    bool isNull()const;
    bool isVoid()const{return isNull();}
    bool isDic()const;
    bool isArray() const;
    bool isNum() const;
    bool isString() const;


    //extern template class QMap<QString, QBkeVariable>;

    static QBkeVariable fromBinary(const QByteArray &b){QBkeVariable a;a.loadFromBinary(b);return a;}
    static QBkeVariable fromString(const QString &b){QBkeVariable a;a.loadFromString(b);return a;}
    static QBkeVariable closureDicFromString(const QString &s){QBkeVariable a;a.loadClosureDicFromString(s);return a;}
    static QBkeVariable array(int count = 0);
    static QBkeVariable dic();

    template<class Head, class... Args>
    static QBkeVariable arrayWithObjects(const Head &h, Args... args)
    {
        QBkeVariable a = QBkeVariable::array();
        a.append(h);
        return _arrayWithObjects(a, args...);
    }

    template<class Head>
    static QBkeVariable arrayWithObjects(const Head &h)
    {
        QBkeVariable a = QBkeVariable::array();
        a.append(h);
        return a;
    }

private:
    template<class Head, class... Args>
    static QBkeVariable _arrayWithObjects(const QBkeVariable &a, const Head &h, Args... args)
    {
        a.append(h);
        return _arrayWithObjects(a, args...);
    }

    template<class Head>
    static QBkeVariable _arrayWithObjects(const QBkeVariable &a, const Head &h)
    {
        a.append(h);
        return a;
    }
};



#endif // PARSERHELPER_H

