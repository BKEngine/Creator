#include "BKE_variable.h"
#include "parser.h"

void GlobalMemoryPool::purge()
{
	clearflag = true;
	for (auto it : *this)
	{
		((BKE_VarObject*)it.first)->forceDelete();
	}
	for (auto it : *this)
	{
		if (it.second <= 4 * SMALL)
			allocator_array()[(it.second + 3) / 4]->dynamic_deallocate(it.first);
		else
			free(it.first);
	}
	clear();
};


void GlobalMemoryPool::finalize()
{
	for (auto &it : *this)
	{
		if (((BKE_VarObject*)it.first)->vt == VAR_CLASS)
		{
			((BKE_VarClass*)it.first)->finalize();
		}
			
	}
}

BKE_bytree::~BKE_bytree()
{
	if (MemoryPool().clearflag)
		return;
	clearChilds();
	if (parent)
	{
		for (int i = 0; i<(bkplong)parent->childs.size(); i++)
		{
			if (parent->childs[i] == this)
			{
				parent->childs[i] = NULL;
				return;
			}
		}
	}
};

//template _BKE_allocator<sizeof(BKE_Variable)>* get_allocator();
//template class BKE_allocator<BKE_Variable>;

BKE_Variable::BKE_Variable(const BKE_Variable &v)
{
	vt = v.vt;
	num = v.num;
	str = v.str;
	obj = v.obj;
	if (obj)
	{
		if (v.isConst())
		{
			switch (v.vt)
			{
			case VAR_ARRAY:
			{
				obj = new BKE_VarArray();
				auto len = static_cast<const BKE_VarArray *>(v.obj)->getCount();
				static_cast<BKE_VarArray *>(obj)->setLength(len);
				for (bkplong i = 0; i < len; i++)
				{
					((BKE_VarArray*)obj)->quickSetMember(i, static_cast<const BKE_VarArray *>(v.obj)->quickGetMember(i).clone());
				}
				break;
			}
			case VAR_DIC:
			{
				obj = new BKE_VarDic();
				auto &&varmap = static_cast<const BKE_VarDic *>(v.obj)->varmap;
				for (auto it = varmap.begin(); it != varmap.end(); it++)
				{
					((BKE_VarDic*)obj)->setMember(it->first, it->second.clone());
				}
			}
				break;
			default:
				obj->addRef();
			}
		}
		else
			obj->addRef();
	}
	is_var = VARIABLE_VAR;
}

BKE_Variable::BKE_Variable(BKE_NativeFunction func)
{
	vt = VAR_FUNC;
	is_var = VARIABLE_VAR;
	obj = new BKE_VarFunction(func);
}

BKE_Variable BKE_Variable::array(int size)
{
	auto arr = new BKE_VarArray();
	arr->setLength(size);
	return arr;
}

BKE_Variable BKE_Variable::dic()
{
	return new BKE_VarDic();
}

void BKE_Variable::save(wstring &result, bool format, bkplong indent) const
{
	//防止溢出
	if (indent > 30)
		return;
	switch (getType())
	{
	case VAR_NONE:
		result += L"void";
		break;
	case VAR_STR:
		result += str.printStr();
		break;
	case VAR_NUM:
		result += num.tostring();
		break;
	case VAR_FUNC:
		if (!static_cast<BKE_VarFunction*>(obj)->isNativeFunction())
		{
			result += L"function(";
			auto f = static_cast<BKE_VarFunction*>(obj);
			auto &&p = f->paramnames;
			auto &&m = f->initials;
			bkplong s = f->paramnames.size();
			if (s > 0)
			{
				result += p[0].getConstStr();
				if (m.find(p[0]) != m.end())
				{
					result += L'=';
					m[p[0]].save(result, false);
				}
			}
			for (int i = 1; i < s; i++)
			{
				result += L"," + p[i].getConstStr();
				if (m.find(p[i]) != m.end())
				{
					result += L'=';
					m[p[i]].save(result, false);
				}
			}
			result += L"){";
			Parser::getInstance()->unParse(f->func->code, result);
			result += L'}';
		}
		break;
	case VAR_ARRAY:
		{
			bkplong c = static_cast<BKE_VarArray*>(obj)->getCount();
			if (c == 0)
			{
				result += L"[]";
				break;
			}
			result += L"[";
			if (format)
			{
				indent += 2;
			}
			int i = 0;
			if (format)
				result += L"\n" + space(indent);
			static_cast<BKE_VarArray*>(obj)->getMember(i).save(result, format, indent);
			for (i = 1; i<c; i++)
			{
				result += L",";
				if (format)
					result += L"\n" + space(indent);
				static_cast<BKE_VarArray*>(obj)->getMember(i).save(result, format, indent);
			}
			if (format)
				result += L"\n" + space(indent - 2);
			result += L"]";
		}
		break;
	case VAR_DIC:
	case VAR_CLO:
		{
			if ((getType()==VAR_DIC && static_cast<BKE_VarDic*>(obj)->getCount() == 0 )||
				(getType() == VAR_CLO && static_cast<BKE_VarClosure*>(obj)->getNonVoidCount() == 0))
			{
				result += L"%[]";
				break;
			}
			result += L"%[";
			if (format)
			{
				indent += 2;
			}
			BKE_hashmap<BKE_String, BKE_Variable> const* vmap;
			if (getType()==VAR_DIC)
				vmap = &static_cast<const BKE_VarDic*>(obj)->varmap;
			else
				vmap = &static_cast<const BKE_VarClosure*>(obj)->varmap;
			auto it = vmap->begin();
			while (it != vmap->end() && it->second.isVoid())
				it++;
			if (it == vmap->end())
			{
				result += L"]";
				break;
			}
			if (format)
				result += L"\n" + space(indent);
			BKE_Variable tmp = it->first;
			tmp.save(result, format, indent);
			result += L"=>";
			it->second.save(result, format, indent);
			it++;
			for (; it != vmap->end(); it++)
			{
				if (it->second.isVoid())
					continue;
				result += L",";
				if (format)
					result += L"\n" + space(indent);
				tmp = it->first;
				tmp.save(result, format, indent);
				result += L"=>";
				it->second.save(result, format, indent);
			}
			if (format)
				result += L"\n" + space(indent - 2);
			result += L"]";
		}
		break;
	case VAR_PROP:
		static_cast<BKE_VarProp*>(obj)->get().save(result, format, indent);
		break;
	case VAR_CLASS:
		{
			//don't save functionand property
			auto *cla = static_cast<const BKE_VarClass*>(obj);
			if (!cla->isInstance())
				break;
			result += L"loadClass(\"" + cla->classname.getConstStr() + L"\", %[";
			auto &&vmap = cla->varmap;
			auto it = vmap.begin();
			while (it!=vmap.end() && (it->second.getType() == VAR_FUNC || it->second.getType() == VAR_PROP))
				it++;
			if (it == vmap.end())
			{
				//result += L"])";
				goto _class_save_native;
			}
			if (format)
			{
				indent += 2;
			}
			if (format)
				result += L"\n" + space(indent);
			{
				BKE_Variable tmp = it->first;
				tmp.save(result, format, indent);
			}
			result += L"=>";
			it->second.save(result, format, indent);
			it++;
			for (; it != vmap.end(); it++)
			{
				if (it->second.isVoid() || it->second.getType() == VAR_FUNC || it->second.getType() == VAR_PROP)
					continue;
				result += L",";
				if (format)
					result += L"\n" + space(indent);
				BKE_Variable tmp = it->first;
				tmp.save(result, format, indent);
				result += L"=>";
				it->second.save(result, format, indent);
			}
			if (format)
				result += L"\n" + space(indent - 2);
_class_save_native:
			result += L"]";
			if (cla->native)
			{
				auto s = cla->native->nativeSave();
				if (!s.isVoid())
				{
					result += L", ";
					s.save(result, format, indent);
				}
			}
			result += L')';
		}
		break;
	default:
		//used for debugpoint
		result += L"void";
	}
}

double BKE_Variable::asNumber() const
{
	switch (vt)
	{
	case VAR_NONE:
		return 0;
	case VAR_NUM:
		return num.asNumber();
	case VAR_STR:
		return str.asNumber().asNumber();
	case VAR_PROP:
		//if (static_cast<const BKE_VarProp *>(obj)->hasGet())
			return static_cast<const BKE_VarProp *>(obj)->get().asNumber();
		//else
		//	_throw(L"没有访问值的权限");
	default:
		_throw(L"无法转化为数值");
	}
}

bkplonglong BKE_Variable::asInteger() const
{
	switch (vt)
	{
	case VAR_NONE:
		return 0;
	case VAR_NUM:
		return num.asInteger();
	case VAR_STR:
		return str.asNumber().asInteger();
	case VAR_PROP:
		//if (static_cast<const BKE_VarProp *>(obj)->hasGet())
			return (bkplonglong)(static_cast<const BKE_VarProp *>(obj)->get().asNumber());
		//else
		//	_throw(L"没有访问值的权限");
	default:
		_throw(L"无法转化为数值");
	}
}

bool BKE_Variable::canBeNumber() const
{
	switch (vt)
	{
	case VAR_NONE:
		return true;
	case VAR_NUM:
		return true;
	case VAR_STR:
		return str.canBeNumber();
	case VAR_PROP:
		//if (static_cast<const BKE_VarProp *>(obj)->hasGet())
		return static_cast<const BKE_VarProp *>(obj)->get().canBeNumber();
		//else
		//	_throw(L"没有访问值的权限");
	default:
		return false;
	}
}

BKE_Number BKE_Variable::asBKENum() const
{
	switch (vt)
	{
	case VAR_NONE:
		return 0;
	case VAR_NUM:
		return num;
	case VAR_STR:
		return str2num(str.getConstStr().c_str());
	case VAR_PROP:
		return static_cast<const BKE_VarProp *>(obj)->get().asBKENum();
	default:
		_throw(L"无法转化为数值。");
	}
}


bkplonglong BKE_Variable::roundAsInteger() const
{
	switch (vt)
	{
	case VAR_NONE:
		return 0;
	case VAR_NUM:
		return num.Round();
	case VAR_STR:
		return str.asNumber().Round();
	case VAR_PROP:
		//if (static_cast<const BKE_VarProp *>(obj)->hasGet())
			return (bkplonglong)round(static_cast<const BKE_VarProp *>(obj)->get().asNumber());
		//else
		//	_throw(L"没有访问值的权限");
	default:
		_throw(L"无法转化为数值");
	}
}

bool BKE_Variable::asBoolean() const
{
	switch (vt)
	{
	case VAR_NONE:
		return false;
	case VAR_NUM:
		return !!asInteger();
	case VAR_STR:
		return str.getConstStr() != L"";
	case VAR_PROP:
		return static_cast<BKE_VarProp*>(obj)->get().asBoolean();
	default:
		return true;
	}
}

wstring BKE_Variable::asString() const
{
	switch (vt)
	{
	case VAR_NONE:
		return L"";
	case VAR_NUM:
		return num.tostring();
	case VAR_STR:
		return str.getStrCopy();
	case VAR_PROP:
		return static_cast<BKE_VarProp*>(obj)->get().asString();
	default:
		_throw(L"无法转化为字符串");
	}
}

BKE_VarClosure *BKE_Variable::asClosure() const
{
	switch (vt)
	{
	case VAR_THIS:
	case VAR_CLO:
	case VAR_CLASS:
		return static_cast<BKE_VarClosure*>(obj);
	case VAR_PROP:
		if (static_cast<BKE_VarProp*>(obj)->hasGet())
			return static_cast<BKE_VarProp*>(obj)->get().asClosure();
		else
			return NULL;
	default:
		return NULL;
	}
}

BKE_VarArray *BKE_Variable::asArray() const
{
	switch (vt)
	{
	case VAR_ARRAY:
		return (BKE_VarArray*)obj;
		break;
	case VAR_PROP:
		if (static_cast<BKE_VarProp*>(obj)->hasGet())
			return static_cast<BKE_VarProp*>(obj)->get().asArray();
		break;
	default:
		break;
	}
	return NULL;
}

BKE_VarDic *BKE_Variable::asDic() const
{
	switch (vt)
	{
	case VAR_DIC:
		return (BKE_VarDic*)(obj);
		break;
	case VAR_PROP:
		if (static_cast<BKE_VarProp*>(obj)->hasGet())
			return static_cast<BKE_VarProp*>(obj)->get().asDic();
		break;
	default:
		break;
	}
	return NULL;
}

BKE_VarFunction *BKE_Variable::asFunc() const
{
	switch (vt)
	{
	case VAR_FUNC:
		return (BKE_VarFunction*)(obj);
		break;
	case VAR_PROP:
		if (static_cast<BKE_VarProp*>(obj)->hasGet())
			return static_cast<BKE_VarProp*>(obj)->get().asFunc();
		break;
	default:
		break;
	}
	return NULL;
}

BKE_VarThis *BKE_Variable::asThisClosure() const
{
	switch (vt)
	{
	case VAR_CLO:
		{
			auto *s = dynamic_cast<BKE_VarThis*>(obj);
			if (s)
				return static_cast<BKE_VarThis*>(s->addRef());
		}
	case VAR_THIS:
		return static_cast<BKE_VarThis*>(obj->addRef());
	case VAR_CLASS:
		return new BKE_VarThis(static_cast<BKE_VarClass*>(obj));
	case VAR_PROP:
		if (static_cast<BKE_VarProp*>(obj)->hasGet())
			return static_cast<BKE_VarProp*>(obj)->get().asThisClosure();
		else
			return NULL;
	default:
		return NULL;
	}
}

BKE_Variable BKE_Variable::clone() const
{
	switch (vt)
	{
	case VAR_ARRAY:
		{
			auto arr = new BKE_VarArray();
			arr->cloneFrom(static_cast<const BKE_VarArray *>(obj));
			return arr;
		}
	case VAR_DIC:
		{
			auto dic = new BKE_VarDic();
			dic->cloneFrom(static_cast<const BKE_VarDic *>(obj));
			return dic;
		}
	default:
		return *this;
	}
}

BKE_Variable& BKE_Variable::operator = (bkpshort v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (bkpushort v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (bkplong v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (bkpulong v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (bkplonglong v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (double v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (float v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (const wchar_t *str)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(std::move(str));
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	this->str = str;
	vt = VAR_STR;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (const wstring &str)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(str);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	this->str = str;
	vt = VAR_STR;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (const BKE_Number &v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (const BKE_String &v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	str = v;
	vt = VAR_STR;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (const BKE_Variable &v)
{
	//we can only get a copy from const var
	if (v.isConst())
		return operator = (v.clone());
	if (v.getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(v.obj)->hasGet())
			return operator = (static_cast<BKE_VarProp*>(v.obj)->get());
		//else
		//	_throw(L"没有访问值的权限");
	}
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	//if a=a, we do nothing
	if (obj && obj == v.obj)
		return *this;
	clear();
	num = v.num;
	str = v.str;
	obj = v.obj;
	vt = v.vt;
	if (obj)
		obj->addRef();
	return *this;
}

BKE_Variable& BKE_Variable::operator = (BKE_Variable &&v)
{
	if (v.getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(v.obj)->hasGet())
			return operator = (static_cast<BKE_VarProp*>(v.obj)->get());
		//else
		//	_throw(L"没有访问值的权限");
	}
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	//if a=a, we do nothing
	if (obj && obj == v.obj)
		return *this;
	clear();
	num = std::move(v.num);
	str = std::move(v.str);
	obj = v.obj;
	v.obj = NULL;
	vt = v.vt;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (BKE_VarObject *v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(v);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	//if a=a, we do nothing
	if (obj && obj == v)
		return *this;
	clear();
	if (!v)
	{
		vt = VAR_NONE;
		return *this;
	}
	vt = v->vt;
	obj = v;
	return *this;
}

BKE_Variable& BKE_Variable::operator = (BKE_NativeFunction func)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasSet())
		//{
			static_cast<BKE_VarProp*>(obj)->set(func);
			return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	obj = new BKE_VarFunction(func);
	vt = VAR_FUNC;
	return *this;
}

BKE_Variable BKE_Variable::operator + (const BKE_Variable &v) const
{
	switch (vt)
	{
	case VAR_NONE:
		return v;
	case VAR_NUM:
		return num + v.asBKENum();
	case VAR_STR:
		return str + v.asBKEStr();
	case VAR_ARRAY:
		{
			BKE_Variable v2 = clone();
			static_cast<BKE_VarArray *>(v2.obj)->pushMember(v);
			return v2;
		}
	case VAR_DIC:
		{
			if (v.vt==VAR_NONE)
			{
				return *this;
			}
			else if (v.vt==VAR_DIC)
			{
				BKE_Variable v2 = clone();
				static_cast<BKE_VarDic *>(v2.obj)->update((BKE_VarDic *)v.obj);
				return v2;
			}
			_throw(L"字典的加法操作需要右操作数为一个字典。");
		}
	case VAR_PROP:
		//if (!static_cast<BKE_VarProp*>(obj)->hasGet())
		//	_throw(L"没有访问值的权限");
		return static_cast<BKE_VarProp*>(obj)->get() + v;
	default:
		_throw(L"不支持加法操作");
	}
};

BKE_Variable BKE_Variable::operator + (const wstring &v) const
{
	switch (vt)
	{
	case VAR_NONE:
		return v;
	case VAR_NUM:
		return num + str2num(v);
	case VAR_STR:
		return str + v;
	case VAR_ARRAY:
		{
			BKE_Variable v2 = clone();
			static_cast<BKE_VarArray *>(v2.obj)->pushMember(v);
			return v2;
		}
	case VAR_DIC:
		_throw(L"字典的加法操作需要右操作数为一个字典。");
	case VAR_PROP:
		//if (!static_cast<BKE_VarProp*>(obj)->hasGet())
		//	_throw(L"没有访问值的权限");
		return static_cast<BKE_VarProp*>(obj)->get() + v;
	default:
		_throw(L"不支持加法操作");
	}
};

BKE_Variable BKE_Variable::operator + (double v) const
{
	switch (vt)
	{
	case VAR_NONE:
		return v;
	case VAR_NUM:
		return num + v;
	case VAR_STR:
		return str + bkpInt2Str(v);
	case VAR_ARRAY:
		{
			BKE_Variable v2 = clone();
			static_cast<BKE_VarArray *>(v2.obj)->pushMember(v);
			return v2;
		}
	case VAR_DIC:
		_throw(L"字典的加法操作需要右操作数为一个字典。");
	case VAR_PROP:
		//if (!static_cast<BKE_VarProp*>(obj)->hasGet())
		//	_throw(L"没有访问值的权限");
		return static_cast<BKE_VarProp*>(obj)->get() + v;
	default:
		_throw(L"不支持加法操作");
	}
};

BKE_Variable BKE_Variable::operator - (const BKE_Variable &v) const
{
	switch (vt)
	{
	case VAR_NONE:
		return -v.asNumber();
	case VAR_NUM:
		return num - v.asNumber();
	case VAR_STR:
		return asNumber() - v.asNumber();
	case VAR_ARRAY:
		{
			BKE_Variable v2 = clone();
			static_cast<BKE_VarArray *>(v2.obj)->deleteMember(v);
			return v2;
		}
	case VAR_DIC:
		{
			if (v.vt==VAR_NONE)
			{
				return *this;
			}
			BKE_Variable v2 = clone();
			if (v.vt==VAR_STR)
			{
				static_cast<BKE_VarDic *>(v2.obj)->deleteMemberIndex(v.asBKEStr());
			}
			else if (v.vt == VAR_ARRAY)
			{
				BKE_VarArray *arr = (BKE_VarArray *)v.obj;
				for (int i = 0; i < arr->getCount(); i++)
				{
					static_cast<BKE_VarDic *>(v2.obj)->deleteMemberIndex(arr->quickGetMember(i).asBKEStr());
				}
			}
			else if (v.vt==VAR_DIC)
			{
				static_cast<BKE_VarDic *>(v2.obj)->except(static_cast<BKE_VarDic *>(v.obj));
			}
			else
			{
				_throw(L"字典的加法操作需要右操作数为一个字典。");
			}
			return v2;
		}
	case VAR_PROP:
		//if (!static_cast<BKE_VarProp*>(obj)->hasGet())
		//	_throw(L"没有访问值的权限");
		return static_cast<BKE_VarProp*>(obj)->get() - v;
	default:
		_throw(L"不支持加法操作");
	}
}

BKE_Variable BKE_Variable::operator - (double v) const
{
	switch (vt)
	{
	case VAR_NONE:
		return -v;
	case VAR_NUM:
		return num - v;
	case VAR_STR:
		return asNumber() - v;
	case VAR_ARRAY:
		{
			BKE_Variable v2 = clone();
			static_cast<BKE_VarArray *>(v2.obj)->deleteMember(v);
			return v2;
		}
	case VAR_DIC:
		{
			BKE_Variable v2 = clone();
			static_cast<BKE_VarDic *>(v2.obj)->deleteMemberIndex(bkpInt2Str(v));
			return v2;
		}
	case VAR_PROP:
		//if (!static_cast<BKE_VarProp*>(obj)->hasGet())
		//	_throw(L"没有访问值的权限");
		return static_cast<BKE_VarProp*>(obj)->get() - v;
	default:
		_throw(L"不支持加法操作");
	}
}

BKE_Variable& BKE_Variable::operator += (const BKE_Variable &v)
{
	switch (vt)
	{
	case VAR_NONE:
		*this = v;
		return *this;
	case VAR_NUM:
		num += v.asNumber();
		return *this;
	case VAR_STR:
		str += v.asString();
		return *this;
	case VAR_ARRAY:
		static_cast<BKE_VarArray*>(obj)->pushMember(v);
		return *this;
	case VAR_DIC:
		if (v.vt == VAR_NONE)
		{
			return *this;
		}
		else if (v.vt == VAR_DIC)
		{
			static_cast<BKE_VarDic*>(obj)->update((BKE_VarDic *)v.obj);
			return *this;
		}
		_throw(L"字典的加法操作需要右操作数为一个字典。");
	case VAR_PROP:
		{
			auto &&p = static_cast<BKE_VarProp*>(obj);
			//if (!p->hasGet())
			//	_throw(L"没有访问值的权限");
			//if (!p->hasSet())
			//	_throw(L"没有修改值的权限");
			if (p->hasGet() && p->hasSet())
				p->set(p->get() + v);
			return *this;
		}
	default:
		_throw(L"不支持加法操作");
	}
}

BKE_Variable& BKE_Variable::operator -= (const BKE_Variable &v)
{
	switch (vt)
	{
	case VAR_NONE:
		*this = -v.asNumber();
		return *this;
	case VAR_NUM:
		num -= v.asNumber();
		return *this;
	case VAR_STR:
		*this = asNumber() - v.asNumber();
		return *this;
	case VAR_ARRAY:
		static_cast<BKE_VarArray*>(obj)->deleteMember(v);
		return *this;
	case VAR_DIC:
	{
		if (v.vt == VAR_NONE)
		{
			return *this;
		}
		if (v.vt == VAR_STR)
		{
			static_cast<BKE_VarDic *>(obj)->deleteMemberIndex(v.asBKEStr());
		}
		else if (v.vt == VAR_ARRAY)
		{
			BKE_VarArray *arr = (BKE_VarArray *)v.obj;
			for (int i = 0; i < arr->getCount(); i++)
			{
				static_cast<BKE_VarDic *>(obj)->deleteMemberIndex(arr->quickGetMember(i).asBKEStr());
			}
		}
		else if (v.vt == VAR_DIC)
		{
			static_cast<BKE_VarDic *>(obj)->except(static_cast<BKE_VarDic *>(v.obj));
		}
		else
		{
			_throw(L"字典的加法操作需要右操作数为一个字典。");
		}
		return *this;
	}
	case VAR_PROP:
		{
			auto &&p = static_cast<BKE_VarProp*>(obj);
			//if (!p->hasGet())
			//	_throw(L"没有访问值的权限");
			//if (!p->hasSet())
			//	_throw(L"没有修改值的权限");
			if (p->hasGet() && p->hasSet())
				p->set(p->get() - v);
			return *this;
		}
	default:
		_throw(L"不支持加法操作");
	}
}

BKE_Variable& BKE_Variable::operator [] (const BKE_Variable &v) const
{
	if (v.getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(v.obj)->hasGet())
			return operator[](static_cast<BKE_VarProp*>(v.obj)->get());
		//else
		//	_throw(L"没有访问值的权限");
	}
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<BKE_VarProp*>(obj)->hasGet())
		//	return static_cast<BKE_VarProp*>(obj)->Get()[v];
		_throw(L"property类型不能直接用于[]运算");
	case VAR_ARRAY:
		return static_cast<BKE_VarArray*>(obj)->getMember((bkplong)v.asInteger());
	case VAR_DIC:
		return static_cast<BKE_VarDic *>(obj)->getMember(v);
	case VAR_CLO:
		return static_cast<BKE_VarClosure *>(obj)->getMember(v);
	case VAR_THIS:
		return static_cast<BKE_VarThis *>(obj)->getMember(v);
	default:
		_throw(L"不支持的运算");
	}
}

BKE_Variable& BKE_Variable::operator [] (const BKE_Variable &v)
{
	if (v.getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(v.obj)->hasGet())
		return operator[](static_cast<BKE_VarProp*>(v.obj)->get());
		//else
		//	_throw(L"没有访问值的权限");
	}
	if (getType() == VAR_NONE)
	{
		if (v.getType() == VAR_NUM)
		{
			vt = VAR_ARRAY;
			obj = new BKE_VarArray();
		}
		else
		{
			vt = VAR_DIC;
			obj = new BKE_VarDic();
		}
	}
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<BKE_VarProp*>(obj)->hasGet())
		//	return static_cast<BKE_VarProp*>(obj)->Get()[v];
		_throw(L"property类型不能直接用于[]运算");
	case VAR_ARRAY:
		return static_cast<BKE_VarArray*>(obj)->getMember((bkplong)v.asInteger());
	case VAR_DIC:
		return static_cast<BKE_VarDic *>(obj)->getMember(v);
	case VAR_CLO:
		return static_cast<BKE_VarClosure *>(obj)->getMember(v);
	case VAR_THIS:
		return static_cast<BKE_VarThis *>(obj)->getMember(v);
	default:
		_throw(L"不支持的运算");
	}
}

BKE_Variable& BKE_Variable::operator [] (const BKE_String &v) const
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<BKE_VarProp*>(obj)->hasGet())
		//return static_cast<BKE_VarProp*>(obj)->Get()[v];
		_throw(L"property类型不能直接用于[]运算");
	case VAR_ARRAY:
		return static_cast<BKE_VarArray*>(obj)->getMember(bkpStr2Int(v));
	case VAR_DIC:
		return static_cast<BKE_VarDic *>(obj)->getMember(v);
	case VAR_CLO:
		return static_cast<BKE_VarClosure *>(obj)->getMember(v);
	case VAR_THIS:
		return static_cast<BKE_VarThis *>(obj)->getMember(v);
	default:
		_throw(L"不支持的运算");
	}
}

BKE_Variable& BKE_Variable::operator [] (const BKE_String &v)
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<BKE_VarProp*>(obj)->hasGet())
		//return static_cast<BKE_VarProp*>(obj)->Get()[v];
		_throw(L"property类型不能直接用于[]运算");
	case VAR_ARRAY:
		return static_cast<BKE_VarArray*>(obj)->getMember(bkpStr2Int(v));
	case VAR_NONE:
		vt = VAR_DIC;
		obj = new BKE_VarDic();
	case VAR_DIC:
		return static_cast<BKE_VarDic *>(obj)->getMember(v);
	case VAR_CLO:
		return static_cast<BKE_VarClosure *>(obj)->getMember(v);
	case VAR_THIS:
		return static_cast<BKE_VarThis *>(obj)->getMember(v);
	default:
		_throw(L"不支持的运算");
	}
}

BKE_Variable& BKE_Variable::operator [] (const wstring &v) const
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<BKE_VarProp*>(obj)->hasGet())
		//return static_cast<BKE_VarProp*>(obj)->Get()[v];
		_throw(L"property类型不能直接用于[]运算");
	case VAR_ARRAY:
		return static_cast<BKE_VarArray*>(obj)->getMember(bkpStr2Int(v));
	case VAR_DIC:
		return static_cast<BKE_VarDic *>(obj)->getMember(v);
	case VAR_CLO:
		return static_cast<BKE_VarClosure *>(obj)->getMember(v);
	case VAR_THIS:
		return static_cast<BKE_VarThis *>(obj)->getMember(v);
	default:
		_throw(L"不支持的运算");
	}
}

BKE_Variable& BKE_Variable::operator [] (const wstring &v)
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<BKE_VarProp*>(obj)->hasGet())
		//return static_cast<BKE_VarProp*>(obj)->Get()[v];
		_throw(L"property类型不能直接用于[]运算");
	case VAR_ARRAY:
		return static_cast<BKE_VarArray*>(obj)->getMember(bkpStr2Int(v));
	case VAR_NONE:
		vt = VAR_DIC;
		obj = new BKE_VarDic();
	case VAR_DIC:
		return static_cast<BKE_VarDic *>(obj)->getMember(v);
	case VAR_CLO:
		return static_cast<BKE_VarClosure *>(obj)->getMember(v);
	case VAR_THIS:
		return static_cast<BKE_VarThis *>(obj)->getMember(v);
	default:
		_throw(L"不支持的运算");
	}
}

BKE_Variable BKE_Variable::getMid(bkplong *start, bkplong *stop, bkplong step)
{
	if (step==0)
		_throw(L"间隔不能为0");
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<BKE_VarProp*>(obj)->hasGet())
			return static_cast<BKE_VarProp*>(obj)->get().getMid(start, stop, step);
		//_throw(L"没有访问值的权限");
	case VAR_ARRAY:
		{
			BKE_VarArray *v = static_cast<BKE_VarArray*>(obj);
			if (start)
				v->getMember(*start);
			if (stop)
				v->getMember(*stop);
			bkplong size = v->getCount();
			if (start && *start<0)
				(*start) += size;
			if (stop && *stop < 0)
				(*stop) += size;
			BKE_VarArray *v2 = new BKE_VarArray();
			if (step>0)
			{
				bkplong sstop = stop ? *stop : size;
				bkplong sstart = start ? *start : 0;
				for (int i = sstart; i < sstop; i+=step)
					v2->pushMember(v->quickGetMember(i));
			}
			else
			{
				bkplong sstop = stop ? *stop : -1;
				bkplong sstart = start ? *start : size - 1;
				for (int i = sstart; i > sstop; i += step)
					v2->pushMember(v->quickGetMember(i));
			}
			return v2;
		}
	case VAR_STR:
		{
			const wstring &s = str.getConstStr();
			if (start && *start<0)
				(*start) += s.size();
			if (stop && *stop < 0)
				(*stop) += s.size();
			if (start && *start >= (bkplong)s.size())
				return BKE_Variable(L"");
			wstring ss;
			if (step>0)
			{
				bkplong sstop = stop ? *stop : s.size();
				bkplong sstart = start ? *start : 0;
				for (int i = sstart; i < sstop; i += step)
					ss.push_back(s[i]);
			}
			else
			{
				bkplong sstop = stop ? *stop : -1;
				bkplong sstart = start ? *start : s.size() - 1;
				for (int i = sstart; i > sstop; i += step)
					ss.push_back(s[i]);
			}
			return ss;
		}
	default:
		_throw(L"不支持的运算");
	}
}

BKE_Variable& BKE_Variable::operator [] (int v) const
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<BKE_VarProp*>(obj)->hasGet())
		//	return static_cast<BKE_VarProp*>(obj)->Get()[v];
		_throw(L"property类型不能直接用于[]运算");
	case VAR_ARRAY:
		return static_cast<BKE_VarArray*>(obj)->getMember(v);
	case VAR_DIC:
		return static_cast<BKE_VarDic *>(obj)->getMember(bkpInt2Str((bkplong)v));
	case VAR_CLO:
		return static_cast<BKE_VarClosure *>(obj)->getMember(bkpInt2Str((bkplong)v));
	case VAR_THIS:
		return static_cast<BKE_VarThis *>(obj)->getMember(bkpInt2Str((bkplong)v));
	default:
        _throw(L"不支持的[]运算");
	}
}

BKE_Variable& BKE_Variable::operator [] (int v)
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<BKE_VarProp*>(obj)->hasGet())
		//	return static_cast<BKE_VarProp*>(obj)->Get()[v];
		_throw(L"property类型不能直接用于[]运算");
	case VAR_NONE:
		vt = VAR_ARRAY;
		obj = new BKE_VarArray();
	case VAR_ARRAY:
		return static_cast<BKE_VarArray*>(obj)->getMember(v);
	case VAR_DIC:
		return static_cast<BKE_VarDic *>(obj)->getMember(bkpInt2Str((bkplong)v));
	case VAR_CLO:
		return static_cast<BKE_VarClosure *>(obj)->getMember(bkpInt2Str((bkplong)v));
	case VAR_THIS:
		return static_cast<BKE_VarThis *>(obj)->getMember(bkpInt2Str((bkplong)v));
	default:
		_throw(L"不支持的[]运算");
	}
}

bool BKE_Variable::operator == (const BKE_Variable &v) const
{
	if (getType() == VAR_PROP)
	{
		return static_cast<BKE_VarProp*>(obj)->get() == v;
	}
	if (v.getType() == VAR_PROP)
	{
		return static_cast<BKE_VarProp*>(v.obj)->get() == *this;
	}
	if (getType() == VAR_NONE && v.getType() == VAR_NONE)
		return true;
	try
	{
		if (getType() == VAR_STR || v.getType() == VAR_STR)
			return asBKEStr() == v.asBKEStr();
		if (getType() == VAR_NUM || v.getType() == VAR_NUM)
			return asBKENum() == v.asBKENum();
	}
	catch (...)
	{
		return false;
	}
	return obj == v.obj;
}

#define HANDLE_PROP(a) if(getType()==VAR_PROP) return ((BKE_VarProp*)obj)->get() a v; if(v.getType()==VAR_PROP) return (*this) a ((BKE_VarProp*)v.obj)->get();

bool BKE_Variable::operator < (const BKE_Variable &v) const
{
	HANDLE_PROP(<);
	if (getType() == VAR_NUM || v.getType() == VAR_NUM)
		return asBKENum()<v.asBKENum();
	return asBKEStr()<v.asBKEStr();
}

bool BKE_Variable::operator > (const BKE_Variable &v) const
{
	HANDLE_PROP(>);
	if (getType() == VAR_NUM || v.getType() == VAR_NUM)
		return asBKENum()>v.asBKENum();
	return asBKEStr()>v.asBKEStr();
}

#undef HANDLE_PROP

BKE_Variable BKE_Variable::dotFunc(const BKE_String &funcname)
{
	if (getType() != VAR_CLASS)
	{
		if (getType() == VAR_NONE)
		{
			vt = VAR_DIC;
			obj = new BKE_VarDic();
		}
		BKE_Variable &v = BKE_VarClosure::global()->getMember(getTypeBKEString());
		if (v.getType() == VAR_NONE)
			return BKE_Variable();
		BKE_Variable *vv;
		auto re = static_cast<BKE_VarClass*>(v.obj)->hasClassMember(funcname, &vv);
		if (re)
		{
			if (vv->getType() == VAR_FUNC)
				return new BKE_VarFunction(*static_cast<BKE_VarFunction*>(vv->obj), BKE_VarClosure::global(), this);
			if (vv->getType() == VAR_PROP)
				return new BKE_VarProp(*static_cast<BKE_VarProp*>(vv->obj), NULL, this);
		}
	}
	return BKE_Variable();
}

BKE_Variable& BKE_Variable::dot(const BKE_String &funcname)
{
	if (getType() == VAR_PROP)
	{
		//return static_cast<BKE_VarProp*>(obj)->Get().Dot(funcname);
		_throw(L"property类型不能直接用于.运算");
	}
	if (getType() == VAR_CLO)
		return static_cast<BKE_VarClosure*>(obj)->getMember(funcname);
	if (getType() == VAR_THIS)
		return static_cast<BKE_VarThis*>(obj)->getMember(funcname);
	//xxx.yyy
	if (getType() != VAR_CLASS)
	{
		if (getType() == VAR_NONE)
		{
			vt = VAR_DIC;
			obj = new BKE_VarDic();
		}
		BKE_Variable &v = BKE_VarClosure::global()->getMember(getTypeBKEString());
		if (v.getType() == VAR_NONE)
			_throw(L"该变量没有取元素的权限。");
		//no global class
		BKE_Variable *vv;
		auto re = static_cast<BKE_VarClass*>(v.obj)->hasClassMember(funcname, &vv);
		if (!re)
		{
			if (getType() == VAR_DIC)
			{
				return static_cast<BKE_VarDic*>(obj)->getMember(funcname);
			}
			if (getType() == VAR_CLO)
			{
				return static_cast<BKE_VarClosure*>(obj)->getMember(funcname);
			}
			if (getType() == VAR_THIS)
			{
				return static_cast<BKE_VarThis*>(obj)->getMember(funcname);
			}
            _throw(L"该变量不存在方法" + funcname.getConstStr());
		}
		else
		{
			_throw(L"无法获得成员" + funcname.getConstStr() + L"的地址");
			//if (vv->getType() == VAR_FUNC)
			//	return new BKE_VarFunction(*static_cast<BKE_VarFunction*>(vv->obj), BKE_VarClosure::global(), this);
			//if (vv->getType() == VAR_PROP)
			//	return new BKE_VarProp(*static_cast<BKE_VarProp*>(vv->obj), NULL, this);
			//return BKE_Variable();
		}
	}
	else
	{
		BKE_Variable &vv = static_cast<BKE_VarClass*>(obj)->getClassMember(funcname);
		if (vv.getType() == VAR_FUNC)
			static_cast<BKE_VarFunction*>(vv.obj)->setSelf(*this);
		if (vv.getType() == VAR_PROP)
			static_cast<BKE_VarProp*>(vv.obj)->setSelf(*this);
		return vv;
	}
}

bkplong BKE_Variable::getCount() const
{
	switch (vt)
	{
	case VAR_ARRAY:
		return static_cast<BKE_VarArray *>(obj)->getCount();
	case VAR_DIC:
		return static_cast<BKE_VarDic *>(obj)->getCount();
	case VAR_CLO:
		return static_cast<BKE_VarClosure*>(obj)->getNonVoidCount();
	case VAR_THIS:
		return static_cast<BKE_VarThis*>(obj)->getNonVoidCount();
	default:
        _throw(L"该变量无方法getCount");
	}
}

bool BKE_Variable::equals(const BKE_Variable &v) const
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(obj)->hasGet())
			return static_cast<BKE_VarProp*>(obj)->get().equals(v);
		//else
		//	_throw(L"没有访问值的权限");
	}
	if (v.getType() == VAR_PROP)
	{
		//if (static_cast<BKE_VarProp*>(v.obj)->hasGet())
			return this->equals(static_cast<BKE_VarProp*>(v.obj)->get());
		//else
		//	_throw(L"没有访问值的权限");
	}
	if (getType() == VAR_NONE)
		return v.equalToVoid();
	if (v.getType() == VAR_NONE)
		return equalToVoid();
	if (getType() == VAR_CLASS)
	{
		BKE_VarClass *cla = static_cast<BKE_VarClass *>(obj);
		if (cla->hasMember(L"equals"))
		{
			BKE_Variable &fun = cla->getMember(L"equals");
			if (fun.getType() != VAR_FUNC)
			{
				return false;
			}
			auto pa = new BKE_VarArray();
			pa->pushMember(v);
			BKE_Variable p(pa);
			BKE_Variable c(cla->addRef());
			static_cast<BKE_VarFunction*>(fun.obj)->setSelf(c);
			return static_cast<BKE_VarFunction*>(fun.obj)->run(pa);
		}
		else
		{
			//use default equals
			return obj == v.obj;
		}
	}
	if (v.getType() == VAR_CLASS)
	{
		BKE_VarClass *cla = static_cast<BKE_VarClass *>(v.obj);
		if (cla->hasMember(L"equals"))
		{
			BKE_Variable &fun = cla->getMember(L"equals");
			if (fun.getType() != VAR_FUNC)
			{
				//_throw(L"该类的equals被重定义为非函数类型");
				return false;
			}
			auto pa = new BKE_VarArray();
			pa->pushMember(*this);
			BKE_Variable p(pa);
			BKE_Variable c(cla->addRef());
			static_cast<BKE_VarFunction*>(fun.obj)->setSelf(c);
			return static_cast<BKE_VarFunction*>(fun.obj)->run(pa);
		}
		else
		{
			//use default equals
			return obj == v.obj;
		}
	}
	if (getType() == VAR_NUM || v.getType() == VAR_NUM)
		return asBKENum() == v.asBKENum();
	if (getType() == VAR_STR || v.getType() == VAR_STR)
		return asBKEStr() == v.asBKEStr();
	if (getType() == VAR_ARRAY && v.getType() == VAR_ARRAY)
		return static_cast<BKE_VarArray*>(obj)->equals(static_cast<BKE_VarArray*>(v.obj));
	if (getType() == VAR_DIC && v.getType() == VAR_DIC)
		return static_cast<BKE_VarDic*>(obj)->equals(static_cast<BKE_VarDic*>(v.obj));
	return obj == v.obj;
}

bool BKE_Variable::instanceOf(const BKE_String &str) const
{
	if (str == L"object")
		return (vt != VAR_NUM && vt != VAR_STR);
	if (vt != VAR_CLASS)
		return getTypeBKEString() == str;
	return ((BKE_VarClass *)obj)->isInstanceof(str);
}

void BKE_Variable::push_back(const BKE_Variable &v)
{
	if (getType() == VAR_ARRAY)
	{
		((BKE_VarArray*)obj)->pushMember(v);
	}
}
