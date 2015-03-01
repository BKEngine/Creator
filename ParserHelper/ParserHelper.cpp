#include <weh.h>
#include "ParserHelper.h"
#include "parser/parser.h"

QBkeVarExcept::QBkeVarExcept(const Var_Except &e)
{
    msg = QString::fromStdWString(e.getMsg());
}

QString QBkeVarExcept::getMsg()
{ 
    return msg;
}

QBkeVariable::QBkeVariable()
{
    _var = new BKE_Variable;
}

QBkeVariable::~QBkeVariable()
{
    delete _var;
}

QBkeVariable::QBkeVariable(const QVariant &v)
{
    _var = new BKE_Variable;
    switch(v.type())
    {
    case QVariant::Bool:
        *this = QBkeVariable(v.toBool());
        break;
    case QVariant::Double:
        *this = QBkeVariable(v.toReal());
        break;
    case QVariant::Int:
        *this = QBkeVariable(v.toInt());
        break;
    case QVariant::String:
        *this = QBkeVariable(v.toString());
        break;
    case QVariant::StringList:
        *this = QBkeVariable(v.toStringList());
        break;
    case QVariant::Map:
        *this = QBkeVariable(v.toMap());
        break;
    case QVariant::Hash:
        *this = QBkeVariable(v.toHash());
        break;
    case QVariant::List:
        *this = QBkeVariable(v.toList());
        break;
    case QVariant::Invalid:
        break;
    default:
        throw(QBkeVarExcept(QString("无法转换的类型：") + v.typeName())) ;
        break;
    }
}

QBkeVariable::QBkeVariable(const QVariantHash &h)
{
    BKE_Variable r = BKE_Variable::dic();
    for(auto it = h.begin(); it != h.end(); it++)
    {
        r[it.key().toStdWString()] = QBkeVariable(it.value());
    }
    _var = new BKE_Variable(r);
}

QBkeVariable::QBkeVariable(const QVariantList &list)
{
    BKE_Variable r = BKE_Variable::array(list.count());
    for(int i = 0;i < list.count(); i++)
    {
        r[i] = QBkeVariable(list[i]);
    }
    _var = new BKE_Variable(r);
}

QBkeVariable::QBkeVariable(const QBkeArray &list)
{
    BKE_Variable r = BKE_Variable::array(list.count());
    for(int i = 0;i < list.count(); i++)
    {
        r[i] = QBkeVariable(list[i]);
    }
    _var = new BKE_Variable(r);
}

QBkeVariable::QBkeVariable(const QBkeDictionary &h)
{
    BKE_Variable r = BKE_Variable::dic();
    for(auto it = h.begin(); it != h.end(); it++)
    {
        r[it.key().toStdWString()] = QBkeVariable(it.value());
    }
    _var = new BKE_Variable(r);
}

QBkeVariable::QBkeVariable(const QVariantMap &map)
{
    BKE_Variable r = BKE_Variable::dic();
    for(auto it = map.begin(); it != map.end(); it++)
    {
        r[it.key().toStdWString()] = QBkeVariable(it.value());
    }
    _var = new BKE_Variable(r);
}

QBkeVariable::QBkeVariable(const QString &s)
{
    _var = new BKE_Variable(s.toStdWString());
}

QBkeVariable::QBkeVariable(const QStringList &l)
{
    BKE_Variable r = BKE_Variable::array(l.count());
    for(int i = 0; i < l.count(); i++)
        r[i] = l[i].toStdWString();
    _var = new BKE_Variable(r);
}

QBkeVariable::QBkeVariable(bool b)
{
    _var = new BKE_Variable(b);
}

QBkeVariable::QBkeVariable(qreal d)
{
    _var = new BKE_Variable(d);
}

QBkeVariable::QBkeVariable(int i)
{
    _var = new BKE_Variable(i);
}

QStringList QBkeVariable::toStringList() const
{
    QStringList a;
    for(int i = 0 ; i < _var->getCount(); i++)
        a << QString::fromStdWString((*_var)[i].asString());
    return a;
}

const QBkeVariable QBkeVariable::operator [](int i) const
{
    return (*_var)[i];
}

const QBkeVariable QBkeVariable::operator [](const QString &k) const
{
    return (*_var)[k.toStdWString()];
}

QBkeVariableRef QBkeVariable::operator [](int i)
{
    if(_var->isVoid())
        *_var = BKE_Variable::array();
    return &(*_var)[i];
}

QBkeVariableRef QBkeVariable::operator [](const QString &k)
{
    if(_var->isVoid())
        *_var = BKE_Variable::dic();
    return &(*_var)[k.toStdWString()];
}

void QBkeVariable::loadFromBinary(const QByteArray &b)
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString a = codec->toUnicode(b);
    loadFromString(a);
}

void QBkeVariable::loadFromString(const QString &s)
{
    BKE_VarClosure *clo = new BKE_VarClosure;
    try
    {
        *_var = Parser::getInstance()->evalMultiLineStr(s.toStdWString(),clo);
    }
    catch(Var_Except e)
    {
        clo->release();
        throw(QBkeVarExcept(e));
    }
    clo->release();      
}

void QBkeVariable::loadClosureDicFromString(const QString &s)
{
    BKE_VarClosure *clo = new BKE_VarClosure;
    try
    {
        Parser::getInstance()->evalMultiLineStr(s.toStdWString(),clo);
    }
    catch(Var_Except e)
    {
        clo->release();
        throw(QBkeVarExcept(e));
    }
    *_var = BKE_Variable::dic();
    _var->asDic()->varmap.Union(clo->varmap, true);
    clo->release();
}

void QBkeVariable::remove(const QString &key)
{
    if(_var->getType()==VAR_DIC)
    {
        BKE_VarDic *v = (BKE_VarDic *)_var->obj;
        v->deleteMemberIndex(key.toStdWString());
    }
}

void QBkeVariable::remove(int i)
{
    if(_var->getType()==VAR_ARRAY)
    {
        BKE_VarArray *v = (BKE_VarArray *)_var->obj;
        v->deleteMemberIndex(i);
    }
}

void QBkeVariable::insert(int i, const QBkeVariable &_v)
{
    if(_var->isVoid())
        *_var = BKE_Variable::array();
    if(_var->getType()==VAR_ARRAY)
    {
        BKE_VarArray *v = (BKE_VarArray *)_var->obj;
        v->insertMember(i, *_v._var);
    }
}


QBkeDictionary QBkeVariable::toBkeDic() const
{
    QMap<QString, QBkeVariable> a;
    if(_var->isVoid())
        return a;
    if(_var->getType()!=VAR_DIC)
        throw(QBkeVarExcept("非字典类型不能转换为QtMap。"));
    auto b = ((BKE_VarDic *)_var->obj)->varmap;
    for(auto it = b.begin(); it != b.end(); it++)
    {
        a[QString::fromStdWString(it->first.getConstStr())] = QBkeVariable(it->second);
    }
    return a;
}

QBkeArray QBkeVariable::toBkeArray() const
{
    QBkeArray a;
    if(_var->isVoid())
        return a;
    if(_var->getType()!=VAR_ARRAY)
        throw(QBkeVarExcept("非数组类型不能转换为QtList。"));
    auto b = (BKE_VarArray *)_var->obj;
    for(auto i = 0; i < _var->getCount(); i++)
    {
        a.append(QBkeVariable(b->getMember(i)));
    }
    return a;
}

QBkeVector QBkeVariable::toBkeVector() const
{
    QBkeVector a;
    if(_var->isVoid())
        return a;
    if(_var->getType()!=VAR_ARRAY)
        throw(QBkeVarExcept("非数组类型不能转换为QVector。"));
    auto b = (BKE_VarArray *)_var->obj;
    for(auto i = 0; i < _var->getCount(); i++)
    {
        a.append(QBkeVariable(b->getMember(i)));
    }
    return a;
}

QStringList QBkeVariable::getKeys() const
{
    QStringList a;
    if(_var->isVoid())
        return a;
    if(_var->getType()!=VAR_DIC)
        throw(QBkeVarExcept("非字典类型不能获取key list。"));
    auto b = ((BKE_VarDic *)_var->obj)->varmap;
    for(auto it = b.begin(); it != b.end(); it++)
    {
        a.append(QString::fromStdWString(it->first.getConstStr()));
    }
    a.sort();
    return a;
}

const QBkeVariable QBkeVariableRef::operator [](int i) const
{
    if(!_var)
        return QBkeVariable();
    return (*_var)[i];
}

const QBkeVariable QBkeVariableRef::operator [](const QString &k) const
{
    if(!_var)
        return QBkeVariable();
    return (*_var)[k.toStdWString()];
}

QBkeVariableRef QBkeVariableRef::operator [](int i)
{
    if(!_var)
        return QBkeVariableRef();
    if(_var->isVoid())
        *_var = BKE_Variable::array();
    return &(*_var)[i];
}

QBkeVariableRef QBkeVariableRef::operator [](const QString &k)
{
    if(!_var)
        return QBkeVariableRef();
    if(_var->isVoid())
        *_var = BKE_Variable::dic();
    return &(*_var)[k.toStdWString()];
}


void QBkeVariableRef::remove(const QString &key)
{
    if(!_var)
        return;
    if(_var->getType()==VAR_DIC)
    {
        BKE_VarDic *v = (BKE_VarDic *)_var->obj;
        v->deleteMemberIndex(key.toStdWString());
    }
}

void QBkeVariableRef::remove(int i)
{
    if(!_var)
        return;
    if(_var->getType()==VAR_ARRAY)
    {
        BKE_VarArray *v = (BKE_VarArray *)_var->obj;
        v->deleteMemberIndex(i);
    }
}

void QBkeVariableRef::insert(int i, const QBkeVariable &_v)
{
    if(!_var)
        return;
    if(_var->isVoid())
        *_var = BKE_Variable::array();
    if(_var->getType()==VAR_ARRAY)
    {
        BKE_VarArray *v = (BKE_VarArray *)_var->obj;
        v->insertMember(i, *_v._var);
    }
}


QBkeVariable QBkeVariableRef::value() const
{
    if(!_var)
        return QBkeVariable();
    return *_var;
}

QBkeVariableRef &QBkeVariableRef::operator =(const QBkeVariable &r)
{
    if(_var)
        *_var = *r._var;
    return *this;
}

void QBkeVariableRef::append(const QBkeVariable &v)
{
    if(_var)
        _var->push_back(*v._var);
}

const QBkeVariable QBkeVariableRef::value(int i) const
{
    if(!_var)
        return QBkeVariable();
    return (*this)[i];
}

const QBkeVariable QBkeVariableRef::value(const QString &k) const
{
    if(!_var)
        return QBkeVariable();
    return (*this)[k];
}

void QBkeVariableRef::redirect(QBkeVariable &v)
{
    _var = v._var;
}

QBkeVariableRef &QBkeVariableRef::operator =(const QBkeVariableRef &r)
{
    if(_var)
        *_var = *r._var;
    return *this;
}

int QBkeVariableRef::getCount() const
{
    if(!_var)
        return 0;
    return _var->getCount();
}

bool QBkeVariableRef::isNull() const
{
    if(!_var)
        return true;
    return _var->isVoid();
}

bool QBkeVariableRef::operator ==(const QBkeVariableRef &r) const
{
    if(!_var || !r._var)
        return false;
    return *_var == *r._var;
}

QBkeVariable::QBkeVariable(const QBkeVariableRef &v)
{
    _var = new BKE_Variable(*v._var);
}

QBkeVariable::QBkeVariable(const QBkeVariable &v)
{
    _var = new BKE_Variable(*v._var);
}

QBkeVariable::QBkeVariable(const BKE_Variable &v)
{
    _var = new BKE_Variable(v);
}

QBkeVariable &QBkeVariable::operator =(const QBkeVariable &v)
{
    *_var = *v._var;
    return *this;
}

QBkeVariable &QBkeVariable::operator =(const QBkeVariableRef &v)
{
    *_var = *v._var;
    return *this;
}

bool QBkeVariable::operator ==(const QBkeVariable &r) const
{
    return *_var==*r._var;
}

int QBkeVariable::getCount() const
{
    return _var->getCount();
}

bool QBkeVariable::isNull() const
{
    return _var->isVoid();
}

QString QBkeVariable::saveToString() const
{
    return QString::fromStdWString(_var->save(false));
}

QBkeVariable QBkeVariable::array(int count)
{
    return BKE_Variable::array(count);
}

QBkeVariable QBkeVariable::dic()
{
    return BKE_Variable::dic();
}

int QBkeVariable::toInt() const
{
    return _var->asInteger();
}

QString QBkeVariable::toString() const
{
    return QString::fromStdWString(_var->asString());
}

qreal QBkeVariable::toReal() const
{
    return _var->asNumber();
}

bool QBkeVariable::toBool() const
{
    return _var->asBoolean();
}

bool QBkeVariable::isDic() const
{
    return _var->getType()==VAR_DIC;
}

bool QBkeVariable::isArray() const
{
    return _var->getType()==VAR_ARRAY;
}

bool QBkeVariable::isNum() const
{
    return _var->getType()==VAR_NUM;
}

bool QBkeVariable::isString() const
{
    return _var->getType()==VAR_STR;
}


bool QBkeVariableRef::isDic() const
{
    return _var->getType()==VAR_DIC;
}

bool QBkeVariableRef::isArray() const
{
    return _var->getType()==VAR_ARRAY;
}

bool QBkeVariableRef::isNum() const
{
    return _var->getType()==VAR_NUM;
}

bool QBkeVariableRef::isString() const
{
    return _var->getType()==VAR_STR;
}

int QBkeVariableRef::toInt() const
{
    return _var->asInteger();
}

QString QBkeVariableRef::toString() const
{
    return QString::fromStdWString(_var->asString());
}

qreal QBkeVariableRef::toReal() const
{
    return _var->asNumber();
}

bool QBkeVariableRef::toBool() const
{
    return _var->asBoolean();
}

QStringList QBkeVariableRef::toStringList() const
{
    QStringList a;
    for(int i = 0 ; i < _var->getCount(); i++)
        a << QString::fromStdWString((*_var)[i].asString());
    return a;
}

QBkeDictionary QBkeVariableRef::toBkeDic() const
{
    QMap<QString, QBkeVariable> a;
    if(_var->isVoid())
        return a;
    if(_var->getType()!=VAR_DIC)
        throw(QBkeVarExcept("非字典类型不能转换为QtMap。"));
    auto b = ((BKE_VarDic *)_var->obj)->varmap;
    for(auto it = b.begin(); it != b.end(); it++)
    {
        a[QString::fromStdWString(it->first.getConstStr())] = QBkeVariable(it->second);
    }
    return a;
}

QBkeArray QBkeVariableRef::toBkeArray() const
{
    QBkeArray a;
    if(_var->isVoid())
        return a;
    if(_var->getType()!=VAR_ARRAY)
        throw(QBkeVarExcept("非数组类型不能转换为QtList。"));
    auto b = (BKE_VarArray *)_var->obj;
    for(auto i = 0; i < _var->getCount(); i++)
    {
        a.append(QBkeVariable(b->getMember(i)));
    }
    return a;
}

QBkeVector QBkeVariableRef::toBkeVector() const
{
    QBkeVector a;
    if(_var->isVoid())
        return a;
    if(_var->getType()!=VAR_ARRAY)
        throw(QBkeVarExcept("非数组类型不能转换为QVector。"));
    auto b = (BKE_VarArray *)_var->obj;
    for(auto i = 0; i < _var->getCount(); i++)
    {
        a.append(QBkeVariable(b->getMember(i)));
    }
    return a;
}
