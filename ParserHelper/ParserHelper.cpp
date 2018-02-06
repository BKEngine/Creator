#include "ParserHelper.h"

GLOBALSTRUCTURES_INIT();

QBkeVarExcept::QBkeVarExcept(const Bagel_Except &e)
{
    msg = QString::fromStdU16String(e.getMsg());
}

QString QBkeVarExcept::getMsg()
{ 
    return msg;
}

QBkeVariable::QBkeVariable()
{
}

QBkeVariable::~QBkeVariable()
{
}

QBkeVariable::QBkeVariable(const QVariant &v)
{
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
	_var = Bagel_Var::dic();
    for(auto it = h.begin(); it != h.end(); it++)
    {
        //r[it.key().toStdU16String()] = QBkeVariable(it.value());
		(*_var)[it.key().toStdU16String()] = QBkeVariable(it.value());
    }
    //_var = new Bagel_Var(r);
}

QBkeVariable::QBkeVariable(const QVariantList &list)
{
	_var = Bagel_Var::array(list.count());
    for(int i = 0;i < list.count(); i++)
    {
        //r[i] = QBkeVariable(list[i]);
		(*_var)[i] = QBkeVariable(list[i]);
    }
    //_var = new Bagel_Var(r);
}

QBkeVariable::QBkeVariable(const QBkeArray &list)
{
	_var = Bagel_Var::array(list.count());
    for(int i = 0;i < list.count(); i++)
    {
        //r[i] = QBkeVariable(list[i]);
		(*_var)[i] = QBkeVariable(list[i]);
	}
    //_var = new Bagel_Var(r);
}

QBkeVariable::QBkeVariable(const QBkeDictionary &h)
{
	_var = Bagel_Var::dic();
	for(auto it = h.begin(); it != h.end(); it++)
    {
        //r[it.key().toStdU16String()] = QBkeVariable(it.value());
		(*_var)[it.key().toStdU16String()] = QBkeVariable(it.value());
    }
    //_var = new Bagel_Var(r);
}

QBkeVariable::QBkeVariable(const QVariantMap &map)
{
	_var = Bagel_Var::dic();
	for(auto it = map.begin(); it != map.end(); it++)
    {
		//r[it.key().toStdU16String()] = QBkeVariable(it.value());
		(*_var)[it.key().toStdU16String()] = QBkeVariable(it.value());
	}
    //_var = new Bagel_Var(r);
}

QBkeVariable::QBkeVariable(const QString &s)
{
    _var = s.toStdU16String();
}

QBkeVariable::QBkeVariable(const QStringList &l)
{
	_var = Bagel_Var::array(l.count());
	for (int i = 0; i < l.count(); i++)
	{
		//r[i] = l[i].toStdU16String();
		(*_var)[i] = l[i].toStdU16String();
	}
    //_var = new Bagel_Var(r);
}

QBkeVariable::QBkeVariable(bool b)
{
    _var = b;
}

QBkeVariable::QBkeVariable(qreal d)
{
    _var = d;
}

QBkeVariable::QBkeVariable(int i)
{
    _var = i;
}

QStringList QBkeVariable::toStringList() const
{
    try
    {
        QStringList a;
        for(int i = 0 ; i < _var->getCount(); i++)
            a << QString::fromStdU16String((*_var)[i].asString());
        return a;
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

const QBkeVariable QBkeVariable::operator [](int i) const
{
    try
    {
        return (*_var)[i];
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

const QBkeVariable QBkeVariable::operator [](const QString &k) const
{
    try
    {
        return (*_var)[k.toStdU16String()];
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

QBkeVariableRef QBkeVariable::operator [](int i)
{
    try
    {
        if(_var->isVoid())
            *_var = Bagel_Var::array();
        return QBkeVariableRef(*_var, i);
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

QBkeVariableRef QBkeVariable::operator [](const QString &k)
{
    try
    {
        if(_var->isVoid())
            *_var = Bagel_Var::dic();
		return QBkeVariableRef(*_var, k.toStdU16String());
	}
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

void QBkeVariable::loadFromBinary(const QByteArray &b)
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString a = codec->toUnicode(b);
    loadFromString(a);
}

//Bagel_Run只能放在一个线程，这里我们放主线程，因为子线程基本只做一些静态分析的事情
//会污染global
void QBkeVariable::loadFromString(const QString &s)
{
	Bagel_Handler<Bagel_Closure> clo = new Bagel_Closure();
    try
    {
		_var = Bagel_VM::getInstance()->Run(s.toStdU16String(), clo);
        //*_var = Parser::getInstance()->evalMultiLineStr(s.toStdU16String(),clo);
    }
    catch(Bagel_Except e)
    {
        throw(QBkeVarExcept(e));
    }
}

//只分析字符串内容是不是纯粹的一个变量
void QBkeVariable::parseFromString(const QString & s)
{
	try
	{
		Bagel_Handler<Bagel_AST> tree = getMainParser()->parse(s.toStdU16String());
		if (tree != nullptr)
		{
			if (tree->Node.opcode == OP_CONSTVAR + OP_COUNT)
			{
				_var = tree->Node.var;
			}
		}
	}
	catch (...) {}
}

void QBkeVariable::loadClosureDicFromString(const QString &s)
{
	Bagel_Handler<Bagel_Closure> clo = new Bagel_Closure();
	try
	{
		Bagel_VM::getInstance()->Run(s.toStdU16String(), clo);
		//*_var = Parser::getInstance()->evalMultiLineStr(s.toStdU16String(),clo);
	}
	catch (Bagel_Except e)
	{
		throw(QBkeVarExcept(e));
	}
	_var = Bagel_Var::dic();
	_GC.writeBarrier(_var->forceAsDic(), clo);
	_var->forceAsDic()->varmap.Union(clo->varmap, true);
}

void QBkeVariable::setVoid()
{
	_var->setVoid();
}

void QBkeVariable::remove(const QString &key)
{
    if(_var->getType()==VAR_DIC)
    {
		_var->forceAsDic()->deleteMemberIndex(key.toStdU16String());
    }
}

void QBkeVariable::remove(int i)
{
    if(_var->getType()==VAR_ARRAY)
    {
		_var->forceAsArray()->deleteMemberIndex(i);
    }
}

void QBkeVariable::append(const QBkeVariable &v)
{
	_GC.writeBarrier(_var, *v._var);
	_var->push_back(*v._var);
}

void QBkeVariable::insert(const QString & key, const QBkeVariable & value)
{
	if (_var->isVoid())
		_var = Bagel_Var::dic();
	if (_var->getType() == VAR_DIC)
	{
		_GC.writeBarrier(_var, *value._var);
		_var->forceAsDic()->setMember(key.toStdU16String(), *value._var);
	}
}

void QBkeVariable::insert(int i, const QBkeVariable &_v)
{
    if(_var->isVoid())
        _var = Bagel_Var::array();
    if(_var->getType()==VAR_ARRAY)
    {
		_GC.writeBarrier(_var, *_v._var);
		_var->forceAsArray()->insertMember(i, *_v._var);
	}
}


QBkeDictionary QBkeVariable::toBkeDic() const
{
    QMap<QString, QBkeVariable> a;
    if(_var->isVoid())
        return a;
    if(_var->getType()!=VAR_DIC)
        throw(QBkeVarExcept("非字典类型不能转换为QtMap。"));
    auto b = _var->forceAsDic()->varmap;
    for(auto it = b.begin(); it != b.end(); it++)
    {
        a[QString::fromStdU16String(it->first.getConstStr())] = QBkeVariable(it->second);
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
    auto b = _var->forceAsArray();
    for(auto i = 0; i < _var->getCount(); i++)
    {
        a.append(QBkeVariable(b->quickGetMember(i)));
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
	auto b = _var->forceAsArray();
	for(auto i = 0; i < _var->getCount(); i++)
    {
        a.append(QBkeVariable(b->quickGetMember(i)));
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
	auto b = _var->forceAsDic()->varmap;
	for(auto it = b.begin(); it != b.end(); it++)
    {
        a.append(QString::fromStdU16String(it->first.getConstStr()));
    }
    a.sort();
    return a;
}

const QBkeVariable QBkeVariableRef::operator [](int i) const
{
	if (!_var)
		return QBkeVariable();
	try
    {
		Bagel_Var v = _var->get();
		if (v.isVoid())
			return QBkeVariable();
        return v[i];
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

const QBkeVariable QBkeVariableRef::operator [](const QString &k) const
{
	if (!_var)
		return QBkeVariable();
	try
    {
		Bagel_Var v = _var->get();
		if (v.isVoid())
			return QBkeVariable();
		return v[k.toStdU16String()];
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

QBkeVariableRef QBkeVariableRef::operator [](int i)
{
	if (!_var)
		return QBkeVariableRef();
	try
    {
		Bagel_Var v = _var->get();
		if (v.isVoid())
		{
			v = Bagel_Var::array();
			_var->set(v);
		}
		//v is temp
		return &v[i];
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

QBkeVariableRef QBkeVariableRef::operator [](const QString &k)
{
	if (!_var)
		return QBkeVariableRef();
	try
    {
		Bagel_Var v = _var->get();
		if (v.isVoid())
		{
			v = Bagel_Var::dic();
			_var->set(v);
		}
		//v is temp
		return &v[k.toStdU16String()];
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

void QBkeVariableRef::remove(const QString &key)
{
	if (!_var)
		return;
	Bagel_Var v = _var->get();
    if(v.getType()==VAR_DIC)
    {
        v.forceAsDic()->deleteMemberIndex(key.toStdU16String());
    }
}

void QBkeVariableRef::remove(int i)
{
	if (!_var)
		return;
	Bagel_Var v = _var->get();
    if(v.getType()==VAR_ARRAY)
    {
        v.forceAsArray()->deleteMemberIndex(i);
    }
}

void QBkeVariableRef::insert(int i, const QBkeVariable &_v)
{
    if(!_var)
        return;
	Bagel_Var v = _var->get();
	if (v.isVoid())
	{
		v = Bagel_Var::array();
	}
	if (v.getType() == VAR_ARRAY)
	{
		v.forceAsArray()->insertMember(i, *_v._var);
	}
	_var->set(v);
}

QBkeVariable QBkeVariableRef::value() const
{
    if(!_var)
        return QBkeVariable();
    return _var->get();
}

QBkeVariableRef &QBkeVariableRef::operator =(const QBkeVariable &r)
{
    if(_var)
        _var->set(*r._var);
    return *this;
}

QBkeVariableRef &QBkeVariableRef::operator -(const QBkeVariable &r)
{
    if(_var)
	{
		_var->set(_var->get() - r);
    }
    return *this;
}

QBkeVariableRef &QBkeVariableRef::operator -=(const QBkeVariable &r)
{
	return operator - (r);
}

void QBkeVariableRef::append(const QBkeVariable &v)
{
	if (_var)
	{
		Bagel_Var vv = _var->get();
		vv.push_back(*v._var);
		_var->set(vv);
	}
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
    _var->changeAddr(&*v._var);
}

QBkeVariableRef &QBkeVariableRef::operator =(const QBkeVariableRef &r)
{
    if(_var)
        *_var = *r._var;
    return *this;
}

QBkeVariableRef &QBkeVariableRef::operator -(const QBkeVariableRef &r)
{
	return operator - (r.value());
}

QBkeVariableRef &QBkeVariableRef::operator -=(const QBkeVariableRef &r)
{
	return operator - (r.value());
}

int QBkeVariableRef::getCount() const
{
    try
    {
        if(!_var)
            return 0;
        return _var->get().getCount();
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

bool QBkeVariableRef::isNull() const
{
    if(!_var)
        return true;
    return _var->get().isVoid();
}

bool QBkeVariableRef::operator ==(const QBkeVariableRef &r) const
{
    if(!_var || !r._var)
        return false;
    try
    {
        return value() == r.value();
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

QBkeVariable::QBkeVariable(const QBkeVariableRef &v)
{
    _var = v.value();
}

QBkeVariable::QBkeVariable(const QBkeVariable &v)
{
    _var = *v._var;
}

QBkeVariable::QBkeVariable(const Bagel_Var &v)
{
    _var = v;
}

QBkeVariable &QBkeVariable::operator =(const QBkeVariable &v)
{
    _var = *v._var;
    return *this;
}

QBkeVariable &QBkeVariable::operator =(const QBkeVariableRef &v)
{
	_var = v.value();
    return *this;
}

QBkeVariable &QBkeVariable::operator -(const QBkeVariable &v)
{
    QBkeVariable t = *this;
    return t -= *v._var;
}

QBkeVariable &QBkeVariable::operator -(const QBkeVariableRef &v)
{
    QBkeVariable t = *this;
    return t -= v.value();
}

QBkeVariable &QBkeVariable::operator -=(const QBkeVariable &v)
{
    *_var -= *v._var;
    return *this;
}

QBkeVariable &QBkeVariable::operator -=(const QBkeVariableRef &v)
{
    *_var -= v.value();
    return *this;
}

bool QBkeVariable::operator ==(const QBkeVariable &r) const
{
    try
    {
        return *_var==*r._var;
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
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
    return QString::fromStdU16String(_var->save(false));
}

QBkeVariable QBkeVariable::array(int count)
{
    return Bagel_Var::array(count);
}

QBkeVariable QBkeVariable::dic()
{
    return Bagel_Var::dic();
}

int QBkeVariable::toInt() const
{
    return _var->asInteger();
}

QString QBkeVariable::toString() const
{
    return QString::fromStdU16String(_var->asString());
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
    return _var->get().getType()==VAR_DIC;
}

bool QBkeVariableRef::isArray() const
{
    return _var->get().getType()==VAR_ARRAY;
}

bool QBkeVariableRef::isNum() const
{
    return _var->get().getType()==VAR_NUM;
}

bool QBkeVariableRef::isString() const
{
    return _var->get().getType()==VAR_STR;
}

int QBkeVariableRef::toInt() const
{
    return _var->get().asInteger();
}

QString QBkeVariableRef::toString() const
{
    return QString::fromStdU16String(_var->get().asString());
}

qreal QBkeVariableRef::toReal() const
{
    return _var->get().asNumber();
}

bool QBkeVariableRef::toBool() const
{
    return _var->get().asBoolean();
}

QStringList QBkeVariableRef::toStringList() const
{
    try
    {
        QStringList a;
		int c = getCount();
        for(int i = 0 ; i < c; i++)
            a << QString::fromStdU16String(_var->get()[i].asString());
        return a;
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

QBkeDictionary QBkeVariableRef::toBkeDic() const
{
    QMap<QString, QBkeVariable> a;
	Bagel_Var v = _var->get();
    if(v.isVoid())
        return a;
    if(v.getType()!=VAR_DIC)
        throw(QBkeVarExcept("非字典类型不能转换为QtMap。"));
    try
    {
        auto b = v.forceAsDic()->varmap;
        for(auto it = b.begin(); it != b.end(); it++)
        {
            a[QString::fromStdU16String(it->first.getConstStr())] = QBkeVariable(it->second);
        }
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
	return a;
}

QBkeArray QBkeVariableRef::toBkeArray() const
{
    QBkeArray a;
	Bagel_Var v = _var->get();
	if (v.isVoid())
		return a;
	if (v.getType() != VAR_ARRAY)
		throw(QBkeVarExcept("非数组类型不能转换为QtList。"));
    try
    {
        auto b = v.forceAsArray();
        for(auto i = 0; i < b->getCount(); i++)
        {
            a.append(QBkeVariable(b->getMember(i)));
        }
        return a;
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}

QBkeVector QBkeVariableRef::toBkeVector() const
{
    QBkeVector a;
	Bagel_Var v = _var->get();
	if (v.isVoid())
		return a;
	if (v.getType() != VAR_ARRAY)
		throw(QBkeVarExcept("非数组类型不能转换为QVector。"));
    try
    {
		auto b = v.forceAsArray();
		for (auto i = 0; i < b->getCount(); i++)
		{
            a.append(QBkeVariable(b->getMember(i)));
        }
        return a;
    }
    catch(Bagel_Except &e)
    {
        throw(QBkeVarExcept(e));
    }
}
