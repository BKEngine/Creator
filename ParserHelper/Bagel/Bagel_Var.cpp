#include "Bagel_VM.h"
#include "Bagel_Parser.h"
#include "Bagel_Serializer.h"
#include "Bagel_VM.h"

#define _throw(str) throw Bagel_Except(Bagel_Var::_getThrowString(str, this) BAGEL_EXCEPT_EXT);
#define _throw2(str, var) throw Bagel_Except(Bagel_Var::_getThrowString(str, &(var)) BAGEL_EXCEPT_EXT);

Bagel_AST::~Bagel_AST()
{
	clearChilds();
	if (parent)
	{
		for (int i = 0; i < (int32_t)parent->childs.size(); i++)
		{
			if (parent->childs[i] == this)
			{
				parent->childs[i] = NULL;
				return;
			}
		}
	}
};

Bagel_StringHolder Bagel_Var::getTypeString(int type)
{
	static const Bagel_String* t[] = {
		REG_STR("void"),
		REG_STR("number"),
		REG_STR("string"),
		REG_STR("array"),
		REG_STR("dictionary"),
		REG_STR("function"),
		REG_STR("property"),
		REG_STR("closure"),
		REG_STR("class"),
		REG_STR("class")/*def*/,
		REG_STR("pointer"),
		REG_STR("AST"),
		REG_STR("function code"),
		REG_STR("bytecode"),
		REG_STR("stack"),
		REG_STR("thread"),
		REG_STR("VM"),
		REG_STR("vector"),
		REG_STR("context"),
		REG_STR("invalid")
	};
	//保证字符串唯一副本
	static_assert(MAX_POOL_STRING_LEN > 13, "MAX_POOL_STRING_LEN too small");
	return t[type];
}

Bagel_StringHolder Bagel_Var::getChineseTypeString() const
{
	static const Bagel_String* t[] = { 
		REG_STR("空类型"),
		REG_STR("数值类型"),
		REG_STR("字符串类型"),
		REG_STR("数组类型"),
		REG_STR("字典类型"),
		REG_STR("函数类型"),
		REG_STR("属性类型"),
		REG_STR("闭包类型"),
		REG_STR("类类型"),
		REG_STR("类类型")/*def*/,
		REG_STR("指针类型"),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
	};
	//保证字符串唯一副本
	static_assert(MAX_POOL_STRING_LEN > 5, "MAX_POOL_STRING_LEN too small");
	return t[vt];
}

void Bagel_Var::save(StringVal &result, bool format, int32_t indent, const char16_t *nameinfo) const
{
	//防止溢出
	if (indent > 30)
		return;
	switch (getType())
	{
	case VAR_NONE:
		result += W("void");
		break;
	case VAR_STR:
		result += forceAsBKEStr()->printStr();
		break;
	case VAR_NUM:
		result += Bagel_Number::toString(num);
		break;
	case VAR_FUNC:
		result += W("function");
		if (nameinfo)
		{
			result += ' ';
			result += nameinfo;
		}
		result += forceAsFunc()->functionInfo();
		if (!static_cast<Bagel_Function*>(obj)->isNativeFunction())
		{
			auto f = static_cast<Bagel_Function*>(obj)->func;
			if (f->info)
			{
				result += f->info->rawexp;
			}
			else if (f->code)
			{
				result += W("{");
				Bagel_Parser::getInstance()->unParse(f->code, result);
				result += '}';
			}
			else
			{
				result += W("{/*compiled code*/}");
			}
		}
		else
		{
			result += W("{/*native function*/}");
		}
		break;
	case VAR_ARRAY:
		{
			int32_t c = static_cast<Bagel_Array*>(obj)->getCount();
			if (c == 0)
			{
				result += W("[]");
				break;
			}
			result += W("[");
			if (format)
			{
				indent += 2;
			}
			int i = 0;
			if (format)
				result += W("\n") + u16string(indent, ' ');
			static_cast<Bagel_Array*>(obj)->getMember(i).save(result, format, indent);
			for (i = 1; i < c; i++)
			{
				result += ',';
				if (format)
					result += W("\n") + u16string(indent, ' ');
				static_cast<Bagel_Array*>(obj)->getMember(i).save(result, format, indent);
			}
			if (format)
				result += W("\n") + u16string(indent - 2, ' ');
			result += ']';
		}
		break;
	case VAR_DIC:
		{
			if (getType() == VAR_DIC && static_cast<Bagel_Dic*>(obj)->getCount() == 0)
			{
				result += W("%[]");
				break;
			}
			result += W("%[");
			if (format)
			{
				indent += 2;
			}
			BKE_hashmap<Bagel_StringHolder, Bagel_Var> const* vmap;
			if (getType() == VAR_DIC)
				vmap = &static_cast<const Bagel_Dic*>(obj)->varmap;
			else
				vmap = &static_cast<const Bagel_Closure*>(obj)->varmap;
			auto it = vmap->begin();
			//while (it != vmap->end() && it->second.isVoid())
			//	it++;
			if (it == vmap->end())
			{
				result += ']';
				break;
			}
			if (format)
				result += W("\n") + u16string(indent, ' ');
			if (it->second.getType() != VAR_CLASSDEF)
			{
				result += it->first.s->printStr();
				result += W("=>");
			}
			it->second.save(result, format, indent);
			it++;
			for (; it != vmap->end(); it++)
			{
				//if (it->second.isVoid())
				//	continue;
				result += ',';
				if (format)
					result += W("\n") + u16string(indent, ' ');
				if (it->second.getType() != VAR_CLASSDEF)
				{
					result += it->first.s->printStr();
					result += W("=>");
				}
				it->second.save(result, format, indent);
			}
			if (format)
				result += W("\n") + u16string(indent - 2, ' ');
			result += ']';
		}
		break;
	case VAR_CLO:
		{
			for (auto &it : forceAsClosure()->varmap)
			{
				if (it.second.getType() == VAR_FUNC)
				{
					it.second.save(result, format, indent, it.first.getConstStr().c_str());
				}
				else if (it.second.getType() == VAR_PROP)
				{
					it.second.save(result, format, indent, it.first.getConstStr().c_str());
				}
				else if (it.second.getType() == VAR_CLASSDEF)
				{
					it.second.save(result, format, indent, it.first.getConstStr().c_str());
				}
				else
				{
					result += W("var ") + it.first;
					if (!it.second.isVoid())
					{
						result += W("=");
						it.second.save(result, format, indent);
						result += W(";");
					}
				}
				if (format)
				{
					result += W("\n") + u16string(indent, ' ');
				}
			}
		}
		break;
	case VAR_PROP:
		{
			auto prop = forceAsProp();
			//if ((prop->funcget && prop->funcget->native) || (prop->funcset && prop->funcset->native))
			//{
			//	break;
			//}
			if (!prop->funcget && !prop->funcset)
			{
				//都是空什么鬼
				break;
			}
			result += W("property");
			if (nameinfo)
			{
				result += ' ';
				result += nameinfo;
			}
			result += '{';
			if (format)
			{
				indent += 2;
				result += W("\n") + u16string(indent, ' ');
			}
			if (prop->funcget)
			{
				result += W("getter");
				if (prop->funcget->native)
				{
					result += W("{/*native code*/}");
				}
				else if (prop->funcget->info)
				{
					result += prop->funcget->info->rawexp;
				}
				else if (prop->funcget->code)
				{
					result += W("{");
					Bagel_Parser::getInstance()->unParse(prop->funcget->code, result);
					result += '}';
				}
				else
				{
					result += W("{/*compiled code*/}");
				}
			}
			if (prop->funcset)
			{
				if (prop->funcset->native)
				{
					result += W("setter(x)");
					result += W("{/*native code*/}");
				}
				else if (prop->funcset->info)
				{
					result += W("setter(") + prop->funcset->paramnames[0].getConstStr() + W(")");
					result += prop->funcset->info->rawexp;
				}
				else if (prop->funcset->code)
				{
					result += W("setter(") + prop->funcset->paramnames[0].getConstStr() + W(")");
					result += W("{");
					Bagel_Parser::getInstance()->unParse(prop->funcset->code, result);
					result += '}';
				}
				else
				{
					result += W("setter(") + prop->funcset->paramnames[0].getConstStr() + W(")");
					result += W("{/*compiled code*/}");
				}
			}
			if (format)
			{
				result += W("\n") + u16string(indent - 2, ' ');
				indent -= 2;
			}
			result += '}';
		}
		break;
		//case VAR_POINTER:
		//	forceAsPointer()->get().save(result, format, indent);
		//	break;
	case VAR_CLASS:
		{
			//don't save functionand property
			auto *cla = static_cast<const Bagel_Class*>(obj);
			result += W("loadClass(\"") + cla->classname.getConstStr() + W("\", %[");
			auto &&vmap = cla->varmap;
			auto it = vmap.begin();
			//while (it!=vmap.end() && (it->second.getType() == VAR_FUNC || it->second.getType() == VAR_PROP))
			//	it++;
			if (it == vmap.end())
			{
				//result += W("])");
				goto _class_save_native;
			}
			if (format)
			{
				indent += 2;
			}
			if (format)
				result += W("\n") + u16string(indent, ' ');
			result += it->first.s->printStr();
			result += W("=>");
			it->second.save(result, format, indent);
			it++;
			for (; it != vmap.end(); it++)
			{
				//if (it->second.getType() == VAR_FUNC || it->second.getType() == VAR_PROP)
				//	continue;
				result += ',';
				if (format)
					result += W("\n") + u16string(indent, ' ');
				result += it->first.s->printStr();
				result += W("=>");
				it->second.save(result, format, indent);
			}
			if (format)
			{
				result += W("\n") + u16string(indent - 2, ' ');
				indent -= 2;
			}
		_class_save_native:
			result += ']';
			if (cla->native)
			{
				auto s = cla->native->nativeSave();
				if (!s.isVoid())
				{
					result += W(", ");
					s.save(result, format, indent);
				}
			}
			result += ')';
		}
		break;
	case VAR_CLASSDEF:
		{
			//don't save functionand property
			auto *cla = static_cast<const Bagel_ClassDef*>(obj);
			result += W("class ") + cla->classname.getConstStr();
			auto &&vmap = cla->varmap;
			if (format)
			{
				result += W("\n") + u16string(indent, ' ');
				indent += 2;
			}
			result += '{';
			if (format)
				result += W("\n") + u16string(indent, ' ');
			//func, property, and static
			for (auto &it : vmap)
			{
				if (it.second.getType() == VAR_FUNC)
				{
					it.second.save(result, format, indent, it.first.getConstStr().c_str());
				}
				else if (it.second.getType() == VAR_PROP)
				{
					it.second.save(result, format, indent, it.first.getConstStr().c_str());
				}
				else
				{
					result += W("static ") + it.first;
					if (!it.second.isVoid())
					{
						result += W("=");
						it.second.save(result, format, indent);
						result += W(";");
					}
				}
				if (format)
				{
					result += W("\n") + u16string(indent, ' ');
				}
			}
			//var
			for (auto &it : cla->classvar)
			{
				result += W("var ") + it.first;
				if (!it.second.isVoid())
				{
					result += W("=");
					it.second.save(result, format, indent);
					result += W(";");
				}
				if (format)
				{
					result += W("\n") + u16string(indent, ' ');
				}
			}
			if (format)
			{
				result += W("\n") + u16string(indent - 2, ' ');
				indent -= 2;
			}
			result += '}';
		}
		break;
	case VAR_BYTECODE_P:
		result += W("/*compiled code*/");
		if (forceAsObject<Bagel_ByteCode>()->debugInfo)
			result += W("/*") + forceAsObject<Bagel_ByteCode>()->debugInfo->rawexp + W("*/");
		break;
	default:
		//used for debugpoint
		result += W("/*invalid*/");
	}
}

double Bagel_Var::_asNumber() const
{
	switch (vt)
	{
	case VAR_NONE:
		return 0;
	case VAR_NUM:
		return num;
	case VAR_STR:
		return forceAsBKEStr()->asNumber();
	case VAR_PROP:
		//if (static_cast<const Bagel_Prop *>(obj)->hasGet())
		return forceAsProp()->get().asNumber();
		//else
		//	_throw(W("没有访问值的权限"));
	case VAR_POINTER:
		return forceAsPointer()->get().asNumber();
	default:
		_throw(W("无法转化为数值"));
	}
}

const double& Bagel_Var::forceAsNumber() const
{
	return num;
}

double& Bagel_Var::forceAsNumber()
{
	return num;
}

int64_t Bagel_Var::_asInteger() const
{
	switch (vt)
	{
	case VAR_NONE:
		return 0;
	case VAR_NUM:
		return (int64_t)num;
	case VAR_STR:
		return (int64_t)forceAsBKEStr()->asNumber();
	case VAR_PROP:
		//if (static_cast<const Bagel_Prop *>(obj)->hasGet())
		return (int64_t)(forceAsProp()->get().asNumber());
		//else
		//	_throw(W("没有访问值的权限"));
	case VAR_POINTER:
		return (int64_t)forceAsPointer()->get().asNumber();
	default:
		_throw(W("无法转化为数值"));
	}
}

uint64_t Bagel_Var::_asUInteger() const
{
	switch (vt)
	{
	case VAR_NONE:
		return 0;
	case VAR_NUM:
		return (uint64_t)num;
	case VAR_STR:
		return (uint64_t)forceAsBKEStr()->asNumber();
	case VAR_PROP:
		//if (static_cast<const Bagel_Prop *>(obj)->hasGet())
		return (uint64_t)(forceAsProp()->get().asNumber());
		//else
		//	_throw(W("没有访问值的权限"));
	case VAR_POINTER:
		return (uint64_t)forceAsPointer()->get().asNumber();
	default:
		_throw(W("无法转化为数值"));
	}
}

int64_t Bagel_Var::forceAsInteger() const
{
	return (int64_t)num;
}

bool Bagel_Var::_canBeNumber() const
{
	switch (vt)
	{
	case VAR_NONE:
		return true;
	case VAR_NUM:
		return true;
	case VAR_STR:
		return forceAsBKEStr()->canBeNumber();
	case VAR_PROP:
		//if (static_cast<const Bagel_Prop *>(obj)->hasGet())
		return forceAsProp()->get().canBeNumber();
		//else
		//	_throw(W("没有访问值的权限"));
	case VAR_POINTER:
		return forceAsPointer()->get().canBeNumber();
	default:
		return false;
	}
}

StringVal Bagel_Var::saveShortString() const
{
	switch (getType())
	{
	case VAR_NUM:
		return Bagel_Number::toString(num);;
	case VAR_STR:
		{
			if(forceAsBKEStr()->size() <= MAX_POOL_STRING_LEN)
				return forceAsBKEStr()->printStr();
			Bagel_StringHolder s(forceAsBKEStr()->substr(0, MAX_POOL_STRING_LEN + 1) + W("..."));
			return s.s->printStr();
		}
		break;
	case VAR_ARRAY:
		{
			auto arr = forceAsArray();
			if (arr->getCount() == 0)
				return W("[]");
			if (arr->getCount() == 1)
				return W("[") + arr->quickGetMember(0).saveShortString() + W("]");
			return W("[") + arr->quickGetMember(0).saveShortString() + W(", ...]");
		}
		break;
	case VAR_DIC:
		{
			auto dic = forceAsDic();
			if (dic->getCount() == 0)
				return W("%[]");
			if (dic->getCount() == 1)
			{
				auto it = dic->begin();
				return W("[") + Bagel_Var(it->first).saveShortString() + W("=>") + it->second.saveShortString() + W("]");
			}
			{
				auto it = dic->begin();
				return W("[") + Bagel_Var(it->first).saveShortString() + W("=>") + it->second.saveShortString() + W(", ...]");
			}
		}
		break;
	case VAR_FUNC:
		{
			auto func = forceAsFunc();
			return func->fullname;
		}
		break;
	case VAR_PROP:
		{
			auto prop = forceAsProp();
			return W("property ") + prop->name;
		}
		break;
	case VAR_CLASSDEF:
		{
			auto cla = forceAsClassDef();
			return W("classdefine ") + cla->classname;
		}
		break;
	case VAR_CLASS:
		{
			auto cla = forceAsClass();
			return W("classdefine ") + cla->classname;
		}
		break;
	case VAR_BYTECODE_P:
		{
			if (forceAsObject<Bagel_ByteCode>()->debugInfo)
			{
				auto &&exp = forceAsObject<Bagel_ByteCode>()->debugInfo->rawexp;
				int n = exp.find_first_of(W("\r\n"));
				if (n<1 || n > MAX_POOL_STRING_LEN)
					n = MAX_POOL_STRING_LEN;
				if(n >= exp.size())
					return W("/*compiled code*/ /*") + exp + W("*/");
				else
					return W("/*compiled code*/ /*") + exp.substr(0, n) + W("... */");
			}
			else
				return W("/*compiled code*/");
		}
	default:
		return getTypeString();
	}
}

vector<uint32_t>&& Bagel_Var::toBinaryStream()
{
	static Bagel_Serializer l;
	return l.serialize(*this);
}

int64_t Bagel_Var::roundAsInteger() const
{
	switch (vt)
	{
	case VAR_NONE:
		return 0;
	case VAR_NUM:
		return round(num);
	case VAR_STR:
		return round(forceAsBKEStr()->asNumber());
	case VAR_PROP:
		//if (static_cast<const Bagel_Prop *>(obj)->hasGet())
		return (int64_t)round(forceAsProp()->get().asNumber());
		//else
		//	_throw(W("没有访问值的权限"));
	case VAR_POINTER:
		return (int64_t)round(forceAsPointer()->get().asNumber());
	default:
		_throw(W("无法转化为数值"));
	}
}

bool Bagel_Var::asBoolean() const
{
	switch (vt)
	{
	case VAR_NONE:
		return false;
	case VAR_NUM:
		return !!asInteger();
	case VAR_STR:
		return !forceAsBKEStr()->empty() && forceAsBKEStr()->getConstStr() != W("false");
	case VAR_PROP:
		return forceAsProp()->get().asBoolean();
	case VAR_POINTER:
		return forceAsPointer()->get().asBoolean();
	default:
		return true;
	}
}

bool Bagel_Var::forceAsBoolean() const
{
	return !!forceAsInteger();
}

wstring Bagel_Var::asWString() const
{
	switch (vt)
	{
	case VAR_NONE:
		return L"";
	case VAR_NUM:
		return Bagel_Number::toWString(num);
	case VAR_STR:
		return forceAsBKEStr()->getWString();
	case VAR_PROP:
		return forceAsProp()->get().asWString();
	case VAR_POINTER:
		return forceAsPointer()->get().asWString();
	default:
		_throw(W("无法转化为字符串"));
	}
}

StringVal Bagel_Var::_asString() const
{
	switch (vt)
	{
	case VAR_NONE:
		return W("");
	case VAR_NUM:
		return Bagel_Number::toString(num);
	case VAR_STR:
		return forceAsBKEStr()->getConstStr();
	case VAR_PROP:
		return forceAsProp()->get().asString();
	case VAR_POINTER:
		return forceAsPointer()->get().asString();
	default:
		_throw(W("无法转化为字符串"));
	}
}

Bagel_Array *Bagel_Var::asArray() const
{
	//临时变量会被回收
	//if (getType() == VAR_PROP)
	//	return static_cast<Bagel_Prop*>(obj)->get().asArray();
	switch (vt)
	{
	case VAR_ARRAY:
		return forceAsArray();
	case VAR_POINTER:
		return forceAsPointer()->get().asArray();
	default:
		_throw(W("无法转化为数组"));
	}
}

Bagel_Dic *Bagel_Var::asDic() const
{
	//if (getType() == VAR_PROP)
	//	return static_cast<Bagel_Prop*>(obj)->get().asDic();
	switch (vt)
	{
	case VAR_DIC:
		return forceAsDic();
	case VAR_POINTER:
		return forceAsPointer()->get().asDic();
	default:
		_throw(W("无法转化为字典"));
	}
}

Bagel_Function *Bagel_Var::asFunc() const
{
	//if (getType() == VAR_PROP)
	//	return static_cast<Bagel_Prop*>(obj)->get().asFunc();
	switch (vt)
	{
	case VAR_FUNC:
		return forceAsFunc();
	case VAR_POINTER:
		return forceAsPointer()->get().asFunc();
	default:
		_throw(W("无法转化为方法"));
	}
}

Bagel_Class *Bagel_Var::asClass() const
{
	//if (getType() == VAR_PROP)
	//	return static_cast<Bagel_Prop*>(obj)->get().asClass();
	switch (vt)
	{
	case VAR_CLASS:
		return forceAsClass();
	case VAR_POINTER:
		return forceAsPointer()->get().asClass();
	default:
		_throw(W("无法转化为类"));
	}
}

Bagel_ClassDef * Bagel_Var::asClassDef() const
{
	switch (vt)
	{
	case VAR_CLASSDEF:
		return forceAsClassDef();
	case VAR_POINTER:
		return forceAsPointer()->get().asClassDef();
	default:
		_throw(W("无法转化为类"));
	}
}

u16string Bagel_Var::forceAsString() const
{
	return forceAsBKEStr()->getConstStr();
}

wstring Bagel_Var::forceAsWString() const
{
	return forceAsBKEStr()->getWString();
}

Bagel_String* Bagel_Var::forceAsBKEStr() const
{
	return (Bagel_String*)obj;
}

Bagel_Closure *Bagel_Var::forceAsClosure() const
{
	switch (vt)
	{
	case VAR_CLO:
	case VAR_CLASS:
	case VAR_CLASSDEF:
		return static_cast<Bagel_Closure*>(obj);
	default:
		return NULL;
	}
}

Bagel_Class * Bagel_Var::forceAsClass() const
{
	return static_cast<Bagel_Class*>(obj);
}

Bagel_ClassDef * Bagel_Var::forceAsClassDef() const
{
	return static_cast<Bagel_ClassDef*>(obj);
}

Bagel_Array *Bagel_Var::forceAsArray() const
{
	return static_cast<Bagel_Array *>(obj);
}

Bagel_Dic *Bagel_Var::forceAsDic() const
{
	return static_cast<Bagel_Dic *>(obj);
}

Bagel_Function *Bagel_Var::forceAsFunc() const
{
	return static_cast<Bagel_Function *>(obj);
}

Bagel_Prop *Bagel_Var::forceAsProp() const
{
	return static_cast<Bagel_Prop *>(obj);
}

Bagel_Pointer * Bagel_Var::forceAsPointer() const
{
	return static_cast<Bagel_Pointer *>(obj);
}

Bagel_Number Bagel_Var::getBKENum(Bagel_Number defaultValue) const
{
	switch (vt)
	{
	case VAR_NUM:
		return num;
	case VAR_PROP:
		return forceAsProp()->get().getBKENum(defaultValue);
	case VAR_POINTER:
		return forceAsPointer()->get().getBKENum(defaultValue);
	default:
		return defaultValue;
	}
}

Bagel_String *Bagel_Var::getBKEStr(Bagel_String *defaultValue) const
{
	switch (vt)
	{
	case VAR_STR:
		return forceAsBKEStr();
	case VAR_PROP:
		return forceAsProp()->get().getBKEStr(defaultValue);
	case VAR_POINTER:
		return forceAsPointer()->get().getBKEStr(defaultValue);
	default:
		return defaultValue ? defaultValue : _globalStructures.stringMap->nullString;
	}
}

float Bagel_Var::getFloat(float defaultValue /*= 0*/) const
{
	switch (vt)
	{
	case VAR_NUM:
		return num;
	case VAR_PROP:
		return forceAsProp()->get().getFloat(defaultValue);
	case VAR_POINTER:
		return forceAsPointer()->get().getFloat(defaultValue);
	default:
		return defaultValue;
	}
}

double Bagel_Var::getDouble(double defaultValue) const
{
	switch (vt)
	{
	case VAR_NUM:
		return num;
	case VAR_PROP:
		return forceAsProp()->get().getDouble(defaultValue);
	case VAR_POINTER:
		return forceAsPointer()->get().getDouble(defaultValue);
	default:
		return defaultValue;
	}
}

int64_t Bagel_Var::getInteger(int64_t defaultValue) const
{
	switch (vt)
	{
	case VAR_NUM:
		return (int64_t)num;
	case VAR_PROP:
		return forceAsProp()->get().getInteger(defaultValue);
	case VAR_POINTER:
		return forceAsPointer()->get().getInteger(defaultValue);
	default:
		return defaultValue;
	}
}

StringVal Bagel_Var::getString(const StringVal & defaultValue) const
{
	switch (vt)
	{
	case VAR_STR:
		return forceAsBKEStr()->getConstStr();
	case VAR_PROP:
		return forceAsProp()->get().getString(defaultValue);
	case VAR_POINTER:
		return forceAsPointer()->get().getString(defaultValue);
	default:
		return defaultValue;
	}
}

wstring Bagel_Var::getWString(const wstring &defaultValue) const
{
	switch (vt)
	{
	case VAR_STR:
		return forceAsBKEStr()->getWString();
	case VAR_PROP:
		return forceAsProp()->get().getWString(defaultValue);
	case VAR_POINTER:
		return forceAsPointer()->get().getWString(defaultValue);
	default:
		return defaultValue;
	}
}

bool Bagel_Var::getBoolean(bool defaultValue) const
{
	if (getType() == VAR_PROP)
		return static_cast<Bagel_Prop*>(obj)->get().getBoolean(defaultValue);
	switch (vt)
	{
	case VAR_NUM:
		return !!(int64_t)num;
	case VAR_STR:
		return !forceAsBKEStr()->getConstStr().empty() && forceAsBKEStr()->getConstStr() != W("false");
	case VAR_PROP:
		return forceAsProp()->get().getBoolean(defaultValue);
	case VAR_POINTER:
		return forceAsPointer()->get().getBoolean(defaultValue);
	default:
		return defaultValue;
	}
}

Bagel_Array *Bagel_Var::getArray(Bagel_Array *defaultValue) const
{
	switch (vt)
	{
	case VAR_ARRAY:
		return static_cast<Bagel_Array*>(obj);
		//Bagel_Prop get出来的是临时变量
		//case VAR_PROP:
		//	return forceAsProp()->get().getArray(defaultValue);
	case VAR_POINTER:
		return forceAsPointer()->get().getArray(defaultValue);
	default:
		return defaultValue;
	}
}

Bagel_Dic *Bagel_Var::getDic(Bagel_Dic *defaultValue) const
{
	switch (vt)
	{
	case VAR_DIC:
		return static_cast<Bagel_Dic*>(obj);
	case VAR_POINTER:
		return forceAsPointer()->get().getDic(defaultValue);
	default:
		return defaultValue;
	}
}

Bagel_Function *Bagel_Var::getFunc(Bagel_Function *defaultValue) const
{
	switch (vt)
	{
	case VAR_FUNC:
		return static_cast<Bagel_Function*>(obj);
	case VAR_POINTER:
		return forceAsPointer()->get().getFunc(defaultValue);
	default:
		return defaultValue;
	}
}

Bagel_Class *Bagel_Var::getClass(Bagel_Class *defaultValue) const
{
	switch (vt)
	{
	case VAR_CLASS:
		return static_cast<Bagel_Class*>(obj);
	case VAR_POINTER:
		return forceAsPointer()->get().getClass(defaultValue);
	default:
		return defaultValue;
	}
}

Bagel_Closure *Bagel_Var::getClosure(Bagel_Closure *defaultValue) const
{
	switch (vt)
	{
	case VAR_CLO:
	case VAR_CLASS:
	case VAR_CLASSDEF:
		return static_cast<Bagel_Closure*>(obj);
	case VAR_POINTER:
		return forceAsPointer()->get().getClosure(defaultValue);
	default:
		return defaultValue;
	}
}

Bagel_Prop *Bagel_Var::getProp(Bagel_Prop *defaultValue) const
{
	switch (vt)
	{
	case VAR_PROP:
		return static_cast<Bagel_Prop*>(obj);
	default:
		return defaultValue;
	}
}

Bagel_Var Bagel_Var::clone() const
{
	switch (vt)
	{
	case VAR_ARRAY:
		{
			auto arr = new Bagel_Array();
			arr->cloneFrom(forceAsArray());
			return arr;
		}
	case VAR_DIC:
		{
			auto dic = new Bagel_Dic();
			dic->cloneFrom(forceAsDic());
			return dic;
		}
	default:
		return *this;
	}
}

void Bagel_Var::copyFrom(const Bagel_Var &v)
{
	//clear();
	vt = VAR_NONE;
	switch (v.vt)
	{
	case VAR_ARRAY:
		{
			auto arr = new Bagel_Array();
			arr->cloneFrom(v.forceAsArray());
			*this = arr;
		}
		break;
	case VAR_DIC:
		{
			auto dic = new Bagel_Dic();
			dic->cloneFrom(v.forceAsDic());
			*this = dic;
		}
		break;
	default:
		vt = v.vt;
		num = v.num;
	}
}

void Bagel_Var::assignStructure(const Bagel_Var &v, BKE_hashmap<void*, void*> &pMap, bool first)
{
	clear();
	vt = VAR_NONE;
	if (v.obj && pMap.contains(v.obj))
	{
		*this = (Bagel_Object*)pMap[v.obj];
		return;
	}
	switch (v.vt)
	{
	case VAR_CLO:
		{
			auto clo = new Bagel_Closure();
			clo->assignStructure((Bagel_Closure*)v.obj, pMap);
			*this = clo;
		}
		break;
	case VAR_CLASS:
		{
			auto cla = new Bagel_Class();
			cla->assignStructure((Bagel_Class*)v.obj, pMap);
			*this = cla;
		}
		break;
	case VAR_CLASSDEF:
		{
			auto cla = new Bagel_ClassDef();
			cla->assignStructure((Bagel_ClassDef*)v.obj, pMap);
			*this = cla;
		}
		break;
	case VAR_FUNC:
		{
			Bagel_Function *func = new Bagel_Function((Bagel_NativeFunction)nullptr);
			func->assignStructure((Bagel_Function*)v.obj, pMap);
			*this = func;
		}
		break;
	case VAR_PROP:
		{
			Bagel_Prop *prop = new Bagel_Prop();
			prop->assignStructure((Bagel_Prop*)v.obj, pMap);
			*this = prop;
		}
		break;
	case VAR_POINTER:
		{
			Bagel_Pointer *prop = new Bagel_Pointer();
			prop->assignStructure((Bagel_Pointer*)v.obj, pMap);
			*this = prop;
		}
		break;
	default:
		copyFrom(v);
	}
}

void Bagel_Var::setMember(const Bagel_Var &str, const Bagel_Var & v)
{
	switch (getType())
	{
		//case VAR_NONE:
		//这里不作操作防止实际变量和栈上的值不一致。也不静默了。
		//if (stack[B].getType() == VAR_NUM)
		//{
		//	stack[A] = new Bagel_Array();
		//	stack[A].forceAsArray()->setMember((short)stack[B].asInteger(), stack[C]);
		//}
		//else
		//{
		//	stack[A] = new Bagel_Dic();
		//	stack[A].forceAsDic()->setMember(stack[B], stack[C]);
		//}
		//break;
	case VAR_DIC:
		forceAsDic()->setMember(str, v);
		break;
	case VAR_CLASS:
	case VAR_CLASSDEF:
	case VAR_CLO:
		forceAsClosure()->setMember(str, v);
		break;
	case VAR_ARRAY:
		forceAsArray()->setMember((short)str.asInteger(), v);
		break;
	default:
		throw Bagel_Except(W("该值") + saveString() + W("不能取元素。"));
	}
}

Bagel_Var& Bagel_Var::operator = (int16_t v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		forceAsProp()->set(v);
		return *this;
		//}
		//else
		//	_throw(W("没有赋值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(v);
		return *this;
	}
	//if (!isVar())
	//	_throw(W("常量不能赋值"));
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (uint16_t v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(v);
		return *this;
		//}
		//else
		//	_throw(W("没有赋值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(v);
		return *this;
	}
	//if (!isVar())
	//	_throw(W("常量不能赋值"));
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (int32_t v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(v);
		return *this;
		//}
		//else
		//	_throw(W("没有赋值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(v);
		return *this;
	}
	//if (!isVar())
	//	_throw(W("常量不能赋值"));
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (uint32_t v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(v);
		return *this;
		//}
		//else
		//	_throw(W("没有赋值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(v);
		return *this;
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (int64_t v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(v);
		return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(v);
		return *this;
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (double v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(v);
		return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(v);
		return *this;
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (float v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(v);
		return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(v);
		return *this;
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	num = v;
	vt = VAR_NUM;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (const wchar_t *str)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(str);
		return *this;
		//}
		//else
		//	_throw(L"没有赋值的权限");
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(str);
		return *this;
	}
	//if (!isVar())
	//	_throw(L"常量不能赋值");
	clear();
	obj = _globalStructures.stringMap->allocateString(str);
	vt = VAR_STR;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (const char16_t *str)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(str);
		return *this;
		//}
		//else
		//	_throw(W("没有赋值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(str);
		return *this;
	}
	//if (!isVar())
	//	_throw(W("常量不能赋值"));
	clear();
	obj = _globalStructures.stringMap->allocateString(str);
	vt = VAR_STR;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (const wstring &str)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(str);
		return *this;
		//}
		//else
		//	_throw(W("没有赋值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(str);
		return *this;
	}
	//if (!isVar())
	//	_throw(W("常量不能赋值"));
	clear();
	obj = _globalStructures.stringMap->allocateString(str);
	vt = VAR_STR;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (Bagel_StringHolder v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(v);
		return *this;
		//}
		//else
		//	_throw(W("没有赋值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(v);
		return *this;
	}
	//if (!isVar())
	//	_throw(W("常量不能赋值"));
	clear();
	obj = v.s;
	vt = VAR_STR;
	return *this;
}

Bagel_Var & Bagel_Var::operator=(const StringVal & v)
{
	clear();
	obj = _globalStructures.stringMap->allocateString(v);
	vt = VAR_STR;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (const Bagel_Var &v)
{
	//we can only get a copy from const var
	//if (v.isConst())
	//{
	//	copyFrom(v);
	//	return *this;
	//}
	if (v.getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(v.obj)->hasGet())
		return operator = (v.forceAsProp()->get());
		//else
		//	_throw(W("没有访问值的权限"));
	}
	if (v.getType() == VAR_POINTER)
	{
		return operator = (v.forceAsPointer()->get());
		return *this;
	}
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(v);
		return *this;
		//}
		//else
		//	_throw(W("没有赋值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(v);
		return *this;
	}
	//if (!isVar())
	//	_throw(W("常量不能赋值"));

	num = v.num;
	//obj = v.obj;
	vt = v.vt;
	//clear rawobj
	return *this;
}

Bagel_Var& Bagel_Var::operator = (Bagel_Var &&v)
{
	if (v.getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(v.obj)->hasGet())
		return operator = (v.forceAsProp()->get());
		//else
		//	_throw(W("没有访问值的权限"));
	}
	if (v.getType() == VAR_POINTER)
	{
		return operator = (v.forceAsPointer()->get());
		return *this;
	}
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		forceAsProp()->set(v);
		return *this;
		//}
		//else
		//	_throw(W("没有赋值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(v);
		return *this;
	}
	//if (!isVar())
	//	_throw(W("常量不能赋值"));
	//num = std::move(v.num);
	//obj = v.obj;
	//v.obj = NULL;
	//vt = v.vt;
	//v.vt = VAR_NONE;
	num = v.num;
	vt = v.vt;
	return *this;
}

Bagel_Var& Bagel_Var::operator = (Bagel_Object *v)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(v);
		return *this;
		//}
		//else
		//	_throw(W("没有赋值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(v);
		return *this;
	}
	//if (!isVar())
	//	_throw(W("常量不能赋值"));
	if (!v)
	{
		vt = VAR_NONE;
		clear();
		return *this;
	}
	else
	{
		vt = v->vt;
		obj = v;
	}
	return *this;
}

Bagel_Var& Bagel_Var::operator = (Bagel_NativeFunction func)
{
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasSet())
		//{
		static_cast<Bagel_Prop*>(obj)->set(func);
		return *this;
		//}
		//else
		//	_throw(W("没有赋值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		forceAsPointer()->set(func);
		return *this;
	}
	//if (!isVar())
	//	_throw(W("常量不能赋值"));
	clear();
	obj = new Bagel_Function(func);
	vt = VAR_FUNC;
	return *this;
}

Bagel_Var Bagel_Var::operator + (const Bagel_Var &v) const
{
	switch (vt)
	{
	case VAR_NONE:
		return v;
	case VAR_NUM:
		return num + v.asNumber();
	case VAR_STR:
		return forceAsBKEStr()->getConstStr() + v.asBKEStr()->getConstStr();
	case VAR_ARRAY:
		{
			Bagel_Var v2 = clone();
			if (v.getType() == VAR_ARRAY)
			{
				static_cast<Bagel_Array*>(v2.obj)->concat(static_cast<Bagel_Array*>(v.obj));
			}
			else
			{
				static_cast<Bagel_Array*>(v2.obj)->pushMember(v);
			}
			return v2;
		}
	case VAR_DIC:
		{
			if (v.vt == VAR_NONE)
			{
				return *this;
			}
			else if (v.vt == VAR_DIC)
			{
				Bagel_Var v2 = clone();
				static_cast<Bagel_Dic *>(v2.obj)->update((Bagel_Dic *)v.obj);
				return v2;
			}
			_throw(W("字典的加法操作需要右操作数为一个字典。"));
		}
	case VAR_PROP:
		//if (!static_cast<Bagel_Prop*>(obj)->hasGet())
		//	_throw(W("没有访问值的权限"));
		return forceAsProp()->get() + v;
	case VAR_POINTER:
		return forceAsPointer()->get() + v;
	default:
		_throw(W("不支持加法操作"));
	}
};

Bagel_Var Bagel_Var::operator + (const wstring &v) const
{
	switch (vt)
	{
	case VAR_NONE:
		return v;
	case VAR_NUM:
		return num + str2num(v);
	case VAR_STR:
		return forceAsBKEStr()->getWString() + v;
	case VAR_ARRAY:
		{
			Bagel_Var v2 = clone();
			static_cast<Bagel_Array *>(v2.obj)->pushMember(v);
			return v2;
		}
	case VAR_DIC:
		_throw(W("字典的加法操作需要右操作数为一个字典。"));
	case VAR_PROP:
		//if (!static_cast<Bagel_Prop*>(obj)->hasGet())
		//	_throw(L"没有访问值的权限");
		return forceAsProp()->get() + v;
	case VAR_POINTER:
		return forceAsPointer()->get() + v;
	default:
		_throw(W("不支持加法操作"));
	}
};

Bagel_Var Bagel_Var::operator + (double v) const
{
	switch (vt)
	{
	case VAR_NONE:
		return v;
	case VAR_NUM:
		return num + v;
	case VAR_STR:
		return forceAsBKEStr()->getConstStr() + bkpInt2Str(v);
	case VAR_ARRAY:
		{
			Bagel_Var v2 = clone();
			static_cast<Bagel_Array *>(v2.obj)->pushMember(v);
			return v2;
		}
	case VAR_DIC:
		_throw(W("字典的加法操作需要右操作数为一个字典。"));
	case VAR_PROP:
		//if (!static_cast<Bagel_Prop*>(obj)->hasGet())
		//	_throw(W("没有访问值的权限"));
		return forceAsProp()->get() + v;
	case VAR_POINTER:
		return forceAsPointer()->get() + v;
	default:
		_throw(W("不支持加法操作"));
	}
};

Bagel_Var Bagel_Var::operator - (const Bagel_Var &v) const
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
			Bagel_Var v2 = clone();
			static_cast<Bagel_Array *>(v2.obj)->deleteMember(v);
			return v2;
		}
	case VAR_DIC:
		{
			if (v.vt == VAR_NONE)
			{
				return *this;
			}
			Bagel_Var v2 = clone();
			if (v.vt == VAR_STR)
			{
				static_cast<Bagel_Dic *>(v2.obj)->deleteMemberIndex(v.asBKEStr());
			}
			else if (v.vt == VAR_ARRAY)
			{
				Bagel_Array *arr = (Bagel_Array *)v.obj;
				for (int i = 0; i < arr->getCount(); i++)
				{
					static_cast<Bagel_Dic *>(v2.obj)->deleteMemberIndex(arr->quickGetMember(i).asBKEStr());
				}
			}
			else if (v.vt == VAR_DIC)
			{
				static_cast<Bagel_Dic *>(v2.obj)->except(static_cast<Bagel_Dic *>(v.obj));
			}
			else
			{
				_throw(W("字典的减法操作需要右操作数为一个字典。"));
			}
			return v2;
		}
	case VAR_PROP:
		//if (!static_cast<Bagel_Prop*>(obj)->hasGet())
		//	_throw(W("没有访问值的权限"));
		return forceAsProp()->get() - v;
	case VAR_POINTER:
		return forceAsPointer()->get() - v;
	default:
		_throw(W("不支持减法操作"));
	}
}

Bagel_Var Bagel_Var::operator - (double v) const
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
			Bagel_Var v2 = clone();
			static_cast<Bagel_Array *>(v2.obj)->deleteMember(v);
			return v2;
		}
	case VAR_DIC:
		{
			Bagel_Var v2 = clone();
			static_cast<Bagel_Dic *>(v2.obj)->deleteMemberIndex(_globalStructures.stringMap->allocateString(bkpInt2Str(v)));
			return v2;
		}
	case VAR_PROP:
		//if (!static_cast<Bagel_Prop*>(obj)->hasGet())
		//	_throw(W("没有访问值的权限"));
		return forceAsProp()->get() - v;
	case VAR_POINTER:
		return forceAsPointer()->get() - v;
	default:
		_throw(W("不支持减法操作"));
	}
}

Bagel_Var& Bagel_Var::operator += (const Bagel_Var &v)
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
		obj = _globalStructures.stringMap->allocateString(forceAsBKEStr()->getConstStr() + v.asString());
		return *this;
	case VAR_ARRAY:
		{
			if (v.getType() == VAR_ARRAY)
			{
				static_cast<Bagel_Array*>(obj)->concat(static_cast<Bagel_Array*>(v.obj));
			}
			else
			{
				static_cast<Bagel_Array*>(obj)->pushMember(v);
			}
			return *this;
		}
	case VAR_DIC:
		if (v.vt == VAR_NONE)
		{
			return *this;
		}
		else if (v.vt == VAR_DIC)
		{
			static_cast<Bagel_Dic*>(obj)->update((Bagel_Dic *)v.obj);
			return *this;
		}
		_throw(W("字典的加法操作需要右操作数为一个字典。"));
	case VAR_PROP:
		{
			auto &&p = static_cast<Bagel_Prop*>(obj);
			//if (!p->hasGet())
			//	_throw(W("没有访问值的权限"));
			//if (!p->hasSet())
			//	_throw(W("没有修改值的权限"));
			if (p->hasGet() && p->hasSet())
				p->set(p->get() + v);
			return *this;
		}
	case VAR_POINTER:
		forceAsPointer()->get() += v;
		return *this;
	default:
		_throw(W("不支持加法操作"));
	}
}

Bagel_Var& Bagel_Var::operator -= (const Bagel_Var &v)
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
		static_cast<Bagel_Array*>(obj)->deleteMember(v);
		return *this;
	case VAR_DIC:
		{
			if (v.vt == VAR_NONE)
			{
				return *this;
			}
			if (v.vt == VAR_STR)
			{
				static_cast<Bagel_Dic *>(obj)->deleteMemberIndex(v.asBKEStr());
			}
			else if (v.vt == VAR_ARRAY)
			{
				Bagel_Array *arr = (Bagel_Array *)v.obj;
				for (int i = 0; i < arr->getCount(); i++)
				{
					static_cast<Bagel_Dic *>(obj)->deleteMemberIndex(arr->quickGetMember(i).asBKEStr());
				}
			}
			else if (v.vt == VAR_DIC)
			{
				static_cast<Bagel_Dic *>(obj)->except(static_cast<Bagel_Dic *>(v.obj));
			}
			else
			{
				_throw(W("字典的减法操作需要右操作数为一个字典。"));
			}
			return *this;
		}
	case VAR_PROP:
		{
			auto &&p = static_cast<Bagel_Prop*>(obj);
			//if (!p->hasGet())
			//	_throw(W("没有访问值的权限"));
			//if (!p->hasSet())
			//	_throw(W("没有修改值的权限"));
			if (p->hasGet() && p->hasSet())
				p->set(p->get() - v);
			return *this;
		}
	case VAR_POINTER:
		forceAsPointer()->get() -= v;
		return *this;
	default:
		_throw(W("不支持减法操作"));
	}
}

Bagel_Var& Bagel_Var::operator [] (const Bagel_Var &v) const
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<Bagel_Prop*>(obj)->hasGet())
		//	return static_cast<Bagel_Prop*>(obj)->Get()[v];
		_throw(W("property类型不能直接用于[]运算"));
	case VAR_ARRAY:
		if (v.canBeNumber())
			return static_cast<Bagel_Array*>(obj)->getMemberAddr((int32_t)v.asInteger());
		else
			_throw(W("没有成员") + v.saveShortString());
	case VAR_DIC:
		return static_cast<Bagel_Dic *>(obj)->getMember(v);
	case VAR_CLO:
		return static_cast<Bagel_Closure *>(obj)->getMember(v);
	case VAR_CLASS:
		{
			Bagel_Var &vv = static_cast<Bagel_Class*>(obj)->getClassMemberAddr(v);
			//if (vv.getType() == VAR_FUNC)
			//	static_cast<Bagel_Function*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			//if (vv.getType() == VAR_PROP)
			//	static_cast<Bagel_Prop*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			return vv;
		}
	case VAR_CLASSDEF:
		return forceAsClassDef()->getClassMemberAddr(v);
	case VAR_POINTER:
		return forceAsPointer()->get().operator [](v);
	default:
		_throw(W("不支持的运算"));
	}
}

Bagel_Var& Bagel_Var::operator [] (Bagel_StringHolder v) const
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<Bagel_Prop*>(obj)->hasGet())
		//return static_cast<Bagel_Prop*>(obj)->Get()[v];
		_throw(W("property类型不能直接用于[]运算"));
	case VAR_ARRAY:
		return static_cast<Bagel_Array*>(obj)->getMemberAddr((int)v.asNumber());
	case VAR_DIC:
		return static_cast<Bagel_Dic *>(obj)->getMember(v);
	case VAR_CLO:
		return static_cast<Bagel_Closure *>(obj)->getMember(v);
	case VAR_CLASS:
		{
			Bagel_Var &vv = static_cast<Bagel_Class*>(obj)->getClassMemberAddr(v);
			//if (vv.getType() == VAR_FUNC)
			//	static_cast<Bagel_Function*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			//if (vv.getType() == VAR_PROP)
			//	static_cast<Bagel_Prop*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			return vv;
		}
	case VAR_CLASSDEF:
		return forceAsClassDef()->getClassMemberAddr(v);
	case VAR_POINTER:
		return forceAsPointer()->get().operator [](v);
	default:
		_throw(W("不支持的运算"));
	}
}

Bagel_Var & Bagel_Var::operator[](const Bagel_String * v) const
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<Bagel_Prop*>(obj)->hasGet())
		//return static_cast<Bagel_Prop*>(obj)->Get()[v];
		_throw(W("property类型不能直接用于[]运算"));
	case VAR_ARRAY:
		return static_cast<Bagel_Array*>(obj)->getMemberAddr((int)v->asNumber());
	case VAR_DIC:
		return static_cast<Bagel_Dic *>(obj)->getMember(v);
	case VAR_CLO:
		return static_cast<Bagel_Closure *>(obj)->getMember(v);
	case VAR_CLASS:
		{
			Bagel_Var &vv = static_cast<Bagel_Class*>(obj)->getClassMemberAddr(v);
			//if (vv.getType() == VAR_FUNC)
			//	static_cast<Bagel_Function*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			//if (vv.getType() == VAR_PROP)
			//	static_cast<Bagel_Prop*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			return vv;
		}
	case VAR_CLASSDEF:
		return forceAsClassDef()->getClassMemberAddr(v);
	case VAR_POINTER:
		return forceAsPointer()->get().operator [](v);
	default:
		_throw(W("不支持的运算"));
	}
}

Bagel_Var& Bagel_Var::operator [] (const wstring &v) const
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<Bagel_Prop*>(obj)->hasGet())
		//return static_cast<Bagel_Prop*>(obj)->Get()[v];
		_throw(W("property类型不能直接用于[]运算"));
	case VAR_ARRAY:
		return static_cast<Bagel_Array*>(obj)->getMemberAddr(bkpStr2Int(v));
	case VAR_DIC:
		return static_cast<Bagel_Dic *>(obj)->getMember(v);
	case VAR_CLO:
		return static_cast<Bagel_Closure *>(obj)->getMember(v);
	case VAR_CLASS:
		{
			Bagel_Var &vv = static_cast<Bagel_Class*>(obj)->getClassMemberAddr(v);
			//if (vv.getType() == VAR_FUNC)
			//	static_cast<Bagel_Function*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			//if (vv.getType() == VAR_PROP)
			//	static_cast<Bagel_Prop*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			return vv;
		}
	case VAR_CLASSDEF:
		return forceAsClassDef()->getClassMemberAddr(v);
	case VAR_POINTER:
		return forceAsPointer()->get().operator [](v);
	default:
		_throw(W("不支持的运算"));
	}
}

Bagel_Var& Bagel_Var::operator [] (const u16string &v) const
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<Bagel_Prop*>(obj)->hasGet())
		//return static_cast<Bagel_Prop*>(obj)->Get()[v];
		_throw(W("property类型不能直接用于[]运算"));
	case VAR_ARRAY:
		return static_cast<Bagel_Array*>(obj)->getMemberAddr(bkpStr2Int(v));
	case VAR_DIC:
		return static_cast<Bagel_Dic *>(obj)->getMember(v);
	case VAR_CLO:
		return static_cast<Bagel_Closure *>(obj)->getMember(v);
	case VAR_CLASS:
		{
			Bagel_Var &vv = static_cast<Bagel_Class*>(obj)->getClassMemberAddr(v);
			//if (vv.getType() == VAR_FUNC)
			//	static_cast<Bagel_Function*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			//if (vv.getType() == VAR_PROP)
			//	static_cast<Bagel_Prop*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			return vv;
		}
	case VAR_CLASSDEF:
		return forceAsClassDef()->getClassMemberAddr(v);
	case VAR_POINTER:
		return forceAsPointer()->get().operator [](v);
	default:
		_throw(W("不支持的运算"));
	}
}

Bagel_Var Bagel_Var::getMid(int32_t *start, int32_t *stop, int32_t step)
{
	if (step == 0)
		_throw(W("间隔不能为0"));
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<Bagel_Prop*>(obj)->hasGet())
		return forceAsProp()->get().getMid(start, stop, step);
		//_throw(W("没有访问值的权限"));
	case VAR_POINTER:
		return forceAsPointer()->get().getMid(start, stop, step);
	case VAR_ARRAY:
		{
			Bagel_Array *v = static_cast<Bagel_Array*>(obj);
			if (start)
				v->getMember(*start);
			if (stop)
				v->getMember(*stop);
			int32_t size = v->getCount();
			if (start && *start < 0)
				(*start) += size;
			if (stop && *stop < 0)
				(*stop) += size;
			Bagel_Array *v2 = new Bagel_Array();
			if (step > 0)
			{
				int32_t sstop = stop ? *stop : size;
				int32_t sstart = start ? *start : 0;
				for (int i = sstart; i < sstop; i += step)
					v2->pushMember(v->quickGetMember(i));
			}
			else
			{
				int32_t sstop = stop ? *stop : -1;
				int32_t sstart = start ? *start : size - 1;
				for (int i = sstart; i > sstop; i += step)
					v2->pushMember(v->quickGetMember(i));
			}
			return v2;
		}
	case VAR_STR:
		{
			auto &s = forceAsBKEStr()->getConstStr();
			if (start && *start < 0)
				(*start) += s.size();
			if (stop && *stop < 0)
				(*stop) += s.size();
			if (start && (*start >= (int32_t)s.size() || *start < 0))
				return Bagel_Var(W(""));
			StringVal ss;
			if (step > 0)
			{
				int32_t sstop = stop ? *stop : s.size();
				int32_t sstart = start ? *start : 0;
				for (int i = sstart; i < sstop; i += step)
					ss.push_back(s[i]);
			}
			else
			{
				int32_t sstop = stop ? *stop : -1;
				int32_t sstart = start ? *start : s.size() - 1;
				for (int i = sstart; i > sstop; i += step)
					ss.push_back(s[i]);
			}
			return ss;
		}
	default:
		_throw(W("不支持的运算"));
	}
}

Bagel_Var& Bagel_Var::operator [] (int v) const
{
	switch (vt)
	{
	case VAR_PROP:
		//if (static_cast<Bagel_Prop*>(obj)->hasGet())
		//	return static_cast<Bagel_Prop*>(obj)->Get()[v];
		_throw(W("property类型不能直接用于[]运算"));
	case VAR_ARRAY:
		return static_cast<Bagel_Array*>(obj)->getMemberAddr(v);
	case VAR_DIC:
		return static_cast<Bagel_Dic *>(obj)->getMember(bkpInt2Str((int32_t)v));
	case VAR_CLO:
		return static_cast<Bagel_Closure *>(obj)->getMember(bkpInt2Str((int32_t)v));
	case VAR_CLASS:
		{
			auto name = bkpInt2Str((int32_t)v);
			Bagel_Var &vv = static_cast<Bagel_Class*>(obj)->getClassMemberAddr(name);
			//if (vv.getType() == VAR_FUNC)
			//	static_cast<Bagel_Function*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			//if (vv.getType() == VAR_PROP)
			//	static_cast<Bagel_Prop*>(vv.obj)->setSelf(*const_cast<Bagel_Var*>(this));
			return vv;
		}
	case VAR_CLASSDEF:
		return forceAsClassDef()->getClassMemberAddr(bkpInt2Str((int32_t)v));
	default:
		_throw(W("不支持的[]运算"));
	}
}

Bagel_Var& Bagel_Var::operator [] (int64_t v) const
{
	return operator [] ((int)v);
}

bool Bagel_Var::operator == (const Bagel_Var &v) const
{
	return strictEqual(v);
}

bool Bagel_Var::strictEqual(const Bagel_Var &v) const
{
	if (getType() == VAR_PROP)
	{
		return forceAsProp()->get().strictEqual(v);
	}
	if (v.getType() == VAR_PROP)
	{
		return v.forceAsProp()->get().strictEqual(*this);
	}
	//if (getType() == VAR_POINTER)
	//{
	//	return forceAsPointer()->get().strictEqual(v);
	//}
	//if (v.getType() == VAR_POINTER)
	//{
	//	return v.forceAsPointer()->get().strictEqual(*this);
	//}
	if (getType() != v.getType())
		return false;
	switch (getType())
	{
	case VAR_NONE:
		return true;
	case VAR_NUM:
		return forceAsNumber() == v.forceAsNumber();
	case VAR_STR:
		return *forceAsBKEStr() == *v.forceAsBKEStr();
	default:
		return obj == v.obj;
	}
}

bool Bagel_Var::normalEqual(const Bagel_Var & v) const
{
	if (getType() == VAR_PROP)
	{
		return forceAsProp()->get().normalEqual(v);
	}
	if (v.getType() == VAR_PROP)
	{
		return v.forceAsProp()->get().normalEqual(*this);
	}
	if (isVoid())
		return v.equalToVoid();
	if (v.isVoid())
		return equalToVoid();
	try
	{
		if (getType() == VAR_STR || v.getType() == VAR_STR)
			return asString() == v.asString();
		else if (getType() == VAR_NUM || v.getType() == VAR_NUM)
			return asNumber() == v.asNumber();
	}
	catch (...)
	{
		return false;
	}
	return obj == v.obj;
}

#define HANDLE_PROP(a) if(getType()==VAR_PROP) return ((Bagel_Prop*)obj)->get() a v;\
			 if(v.getType()==VAR_PROP) return (*this) a ((Bagel_Prop*)v.obj)->get();\
			 //if(getType()==VAR_POINTER) return ((Bagel_Pointer*)obj)->get() a v;\
			 //if(v.getType()==VAR_POINTER) return (*this) a ((Bagel_Pointer*)v.obj)->get();


bool Bagel_Var::operator < (const Bagel_Var &v) const
{
	HANDLE_PROP(< );
	if (getType() == VAR_NUM || v.getType() == VAR_NUM)
		return asNumber() < v.asNumber();
	return asBKEStr() < v.asBKEStr();
}

bool Bagel_Var::operator > (const Bagel_Var &v) const
{
	HANDLE_PROP(> );
	if (getType() == VAR_NUM || v.getType() == VAR_NUM)
		return asNumber() > v.asNumber();
	return asBKEStr() > v.asBKEStr();
}

#undef HANDLE_PROP

Bagel_Var Bagel_Var::getAllDots() const
{
	if (getType() == VAR_PROP)
	{
		return forceAsProp()->get().getAllDots();
	}
	if (getType() == VAR_POINTER)
	{
		return forceAsPointer()->get().getAllDots();
	}
	if (getType() != VAR_CLASS)
	{
		if (getType() == VAR_NONE)
			return new Bagel_Array();
		Bagel_ClassDef *v = _globalStructures.typeclass[getType()];
		if (!v)
			return false;
		auto &&m = v->varmap;
		auto arr = new Bagel_Array();
		for (auto &&it : m)
		{
			if (it.second.getType() != VAR_FUNC)
				arr->pushMember(it.first);
			else
				arr->pushMember(it.first.getStrCopy() + ((Bagel_Function*)it.second.obj)->functionSimpleInfo());
		}
		if (getType() == VAR_DIC)
		{
			auto &&m2 = static_cast<Bagel_Dic*>(obj)->varmap;
			for (auto &&it : m2)
			{
				if (it.second.getType() != VAR_FUNC)
					arr->pushMember(it.first);
				else
					arr->pushMember(it.first.getStrCopy() + ((Bagel_Function*)it.second.obj)->functionSimpleInfo());
			}
		}
		return arr;
	}
	auto arr = new Bagel_Array();
	auto &&m = static_cast<Bagel_Class*>(obj)->varmap;
	for (auto &&it : m)
	{
		if (it.second.getType() != VAR_FUNC)
			arr->pushMember(it.first);
		else
			arr->pushMember(it.first.getStrCopy() + ((Bagel_Function*)it.second.obj)->functionSimpleInfo());
	}
	return arr;
}

bool Bagel_Var::dotFunc(Bagel_StringHolder  funcname, Bagel_Var &out) const
{
	if (getType() != VAR_CLASS && getType() != VAR_CLO)
	{
		//Bagel_Var &v = Bagel_Closure::global()->getMember(getTypeBKEString());
		//if (v.getType() == VAR_NONE)
		//	return false;
		Bagel_ClassDef *v = _globalStructures.typeclass[getType()];
		if (!v)
			return false;
		Bagel_Var *vv;
		auto re = v->hasClassMember(funcname, &vv);
		if (re)
		{
			if (vv->getType() == VAR_FUNC)
			{
				out = new Bagel_Function(*static_cast<Bagel_Function*>(vv->obj), Bagel_Closure::global(), *this);
				return true;
			}
			if (vv->getType() == VAR_PROP)
			{
				out = new Bagel_Prop(*static_cast<Bagel_Prop*>(vv->obj), NULL, *this);
				return true;
			}
			out = *vv;
			return true;
		}
	}
	return false;
}

Bagel_Var Bagel_Var::dotValue(const Bagel_Var &funcname) const
{
	if (getType() == VAR_PROP)
	{
		return static_cast<Bagel_Prop*>(obj)->get().dotValue(funcname);
	}
	if (getType() == VAR_POINTER)
	{
		return forceAsPointer()->get().dotValue(funcname);
	}
	if (getType() == VAR_CLO)
		return static_cast<Bagel_Closure*>(obj)->getMember(funcname);
	//xxx.yyy
	if (getType() != VAR_CLASS)
	{
		Bagel_Var v;
		bool re = dotFunc(funcname, v);
		if (!re)
		{
			if (getType() == VAR_STR)
			{
				return forceAsBKEStr()->getElement(funcname.asInteger());
			}
			if (getType() == VAR_DIC)
			{
				return static_cast<Bagel_Dic*>(obj)->getMember(funcname);
			}
			if (getType() == VAR_CLO)
			{
				return static_cast<Bagel_Closure*>(obj)->getMember(funcname);
			}
			//_throw(W("该变量没有取元素的权限。"));
			_throw(W("该变量不存在方法") + funcname.asString());
		}
		else
		{
			//_throw(W("无法获得成员") + funcname.getConstStr() + W("的地址"));
			return v;
		}
	}
	else
	{
		return static_cast<Bagel_Class*>(obj)->getClassMemberValue(funcname);
	}
}

Bagel_Var& Bagel_Var::dotAddr(Bagel_StringHolder  funcname)
{
	if (getType() == VAR_PROP)
	{
		_throw(W("property不能直接用于dot赋值"));
	}
	if (getType() == VAR_POINTER)
	{
		return forceAsPointer()->get().dotAddr(funcname);
	}
	if (getType() == VAR_CLO)
		return static_cast<Bagel_Closure*>(obj)->getMember(funcname);
	if (getType() != VAR_CLASS)
	{
		//if (getType() == VAR_NONE)
		//{
		//	vt = VAR_DIC;
		//	obj = new Bagel_Dic();
		//}
		Bagel_ClassDef *v = _globalStructures.typeclass[getType()];
		if (!v)
			_throw(W("该变量没有取元素的权限。"));
		//no global class
		Bagel_Var *vv;
		auto re = v->hasClassMember(funcname, &vv);
		if (!re)
		{
			if (getType() == VAR_DIC)
			{
				return static_cast<Bagel_Dic*>(obj)->getMember(funcname);
			}
			if (getType() == VAR_CLO)
			{
				return static_cast<Bagel_Closure*>(obj)->getMember(funcname);
			}
			_throw(W("该变量不存在方法") + funcname.getConstStr());
		}
		else
		{
			_throw(W("无法获得成员") + funcname.getConstStr() + W("的地址"));
		}
	}
	else
	{
		return static_cast<Bagel_Class*>(obj)->getClassMemberAddr(funcname);
	}
}

int32_t Bagel_Var::getCount() const
{
	switch (vt)
	{
	case VAR_ARRAY:
		return static_cast<Bagel_Array *>(obj)->getCount();
	case VAR_DIC:
		return static_cast<Bagel_Dic *>(obj)->getCount();
	case VAR_CLO:
		return static_cast<Bagel_Closure*>(obj)->getNonVoidCount();
	case VAR_PROP:
		return forceAsProp()->get().getCount();
	case VAR_POINTER:
		return forceAsPointer()->get().getCount();
	default:
		_throw(W("该变量无方法getCount"));
	}
}

bool Bagel_Var::equals(const Bagel_Var &v) const
{
	static Bagel_StringHolder _equals(W("equals"));
	if (getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(obj)->hasGet())
		return forceAsProp()->get().equals(v);
		//else
		//	_throw(W("没有访问值的权限"));
	}
	if (v.getType() == VAR_PROP)
	{
		//if (static_cast<Bagel_Prop*>(v.obj)->hasGet())
		return this->equals(v.forceAsProp()->get());
		//else
		//	_throw(W("没有访问值的权限"));
	}
	if (getType() == VAR_POINTER)
	{
		return forceAsPointer()->get().equals(v);
	}
	if (v.getType() == VAR_POINTER)
	{
		return this->equals(v.forceAsPointer()->get());
	}
	if (getType() == VAR_NONE)
		return v.equalToVoid();
	if (v.getType() == VAR_NONE)
		return equalToVoid();
	if (getType() == VAR_CLASS)
	{
		Bagel_Class *cla = static_cast<Bagel_Class *>(obj);
		if (cla->hasMember(_equals))
		{
			Bagel_Var fun = cla->getClassMemberValue(_equals);
			if (fun.getType() != VAR_FUNC)
			{
				return false;
			}
			return static_cast<Bagel_Function*>(fun.obj)->run(const_cast<Bagel_Var*>(&v), 1);
		}
		else
		{
			//use default equals
			return obj == v.obj;
		}
	}
	if (v.getType() == VAR_CLASS)
	{
		Bagel_Class *cla = static_cast<Bagel_Class *>(v.obj);
		if (cla->hasMember(_equals))
		{
			Bagel_Var fun = cla->getClassMemberValue(_equals);
			if (fun.getType() != VAR_FUNC)
			{
				//_throw(W("该类的equals被重定义为非函数类型"));
				return false;
			}
			return static_cast<Bagel_Function*>(fun.obj)->run(const_cast<Bagel_Var*>(this), 1);
		}
		else
		{
			//use default equals
			return obj == v.obj;
		}
	}
	if (getType() == VAR_NUM || v.getType() == VAR_NUM)
		return asNumber() == v.asNumber();
	if (getType() == VAR_STR || v.getType() == VAR_STR)
		return *forceAsBKEStr() == *v.forceAsBKEStr();
	if (getType() == VAR_ARRAY && v.getType() == VAR_ARRAY)
		return static_cast<Bagel_Array*>(obj)->equals(static_cast<Bagel_Array*>(v.obj));
	if (getType() == VAR_DIC && v.getType() == VAR_DIC)
		return static_cast<Bagel_Dic*>(obj)->equals(static_cast<Bagel_Dic*>(v.obj));
	return obj == v.obj;
}

Bagel_StringHolder Bagel_Var::getClassName() const
{
	if (vt == VAR_CLASS)
		return forceAsClass()->classname;
	else if (vt == VAR_CLASSDEF)
		return forceAsClassDef()->classname;
	else
		return getTypeString();
}

bool Bagel_Var::instanceOf(Bagel_StringHolder str) const
{
	static Bagel_StringHolder ob(W("object"));
	static Bagel_StringHolder cla(W("class"));
	if (str == ob)
		return isObject();
	if (vt != VAR_CLASS && vt != VAR_CLASSDEF)
		return getTypeBKEString() == str;
	if (str == cla)
		return true;
	if(vt == VAR_CLASS)
		return ((Bagel_Class *)obj)->isInstanceof(str);
	else
		return ((Bagel_ClassDef *)obj)->isInstanceof(str);
}

void Bagel_Var::push_back(const Bagel_Var &v)
{
	if (getType() == VAR_ARRAY)
	{
		((Bagel_Array*)obj)->pushMember(v);
	}
}

void Bagel_Dic::getAllVariables(std::map<StringVal, PromptType>& result)
{
	auto cla = _globalStructures.typeclass[VAR_DIC];
	if (cla)
	{
		cla->getAllVariables(result);
	}
	for (auto &s : varmap)
	{
		result[s.first] = getPromptTypeFromVar(s.second);
	}
}

void Bagel_Dic::getAllVariablesWithPrefix(std::map<StringVal, PromptType>& result, const StringVal & prefix)
{
	auto cla = _globalStructures.typeclass[VAR_DIC];
	if (cla)
	{
		cla->getAllVariablesWithPrefix(result, prefix);
	}
	for (auto &s : varmap)
	{
		if (s.first.beginWith(prefix))
		{
			result[s.first] = getPromptTypeFromVar(s.second);
		}
	}
}

void Bagel_Dic::getAllVariablesWithPrefixAndType(std::map<StringVal, PromptType>& result, const StringVal & prefix, VarType vt)
{
	auto cla = _globalStructures.typeclass[VAR_DIC];
	if (cla)
	{
		cla->getAllVariablesWithPrefixAndType(result, prefix, vt);
	}
	for (auto &s : varmap)
	{
		if (s.second.getType() == vt && s.first.beginWith(prefix))
		{
			result[s.first] = getPromptTypeFromVar(s.second);
		}
	}
}

void Bagel_Closure::assignStructure(Bagel_Closure *v, BKE_hashmap<void*, void*> &pMap, bool first)
{
	clear();
	pMap[v] = this;
	if (!first && v->parent)
	{
		if (pMap.contains(v->parent))
		{
			parent = (Bagel_Closure*)pMap[v->parent];
		}
		else
		{
			parent = new Bagel_Closure();
			parent->assignStructure(v->parent, pMap);
			pMap[v->parent] = parent;
		}
	}
	else
		parent = v->parent;
	for (auto it = v->varmap.begin(); it != v->varmap.end(); it++)
	{
		varmap[it->first].assignStructure(it->second, pMap);
	}
}

void Bagel_Closure::getAllVariables(std::map<StringVal, PromptType>& result)
{
	if (parent)
		parent->getAllVariables(result);
	for (auto &s : varmap)
	{
		result[s.first] = getPromptTypeFromVar(s.second);
	}
}

void Bagel_Closure::getAllVariablesWithPrefix(std::map<StringVal, PromptType>& result, const StringVal & prefix)
{
	if (parent)
		parent->getAllVariablesWithPrefix(result, prefix);
	for (auto &s : varmap)
	{
		if (s.first.beginWith(prefix))
		{
			result[s.first] = getPromptTypeFromVar(s.second);
		}
	}
}

void Bagel_Closure::getAllVariablesWithPrefixAndType(std::map<StringVal, PromptType>& result, const StringVal & prefix, VarType vt)
{
	if (parent)
		parent->getAllVariablesWithPrefixAndType(result, prefix, vt);
	for (auto &s : varmap)
	{
		if (s.second.getType() == vt && s.first.beginWith(prefix))
		{
			result[s.first] = getPromptTypeFromVar(s.second);
		}
	}
}

void Bagel_ClassDef::assignStructure(Bagel_ClassDef *cla, BKE_hashmap<void*, void*> &pMap, bool first)
{
	Bagel_Closure::assignStructure(cla, pMap, first);
	classname = cla->classname;
	cannotcreate = cla->cannotcreate;
	parents = cla->parents;
	children = cla->children;
	native = cla->native->retain();
	isFinal = cla->isFinal;
	for (auto it = cla->classvar.begin(); it != cla->classvar.end(); it++)
	{
		classvar[it->first].assignStructure(it->second, pMap);
	}
}

void Bagel_Class::assignStructure(Bagel_Class *cla, BKE_hashmap<void*, void*> &pMap, bool first)
{
	Bagel_Closure::assignStructure(cla, pMap, first);
	classname = cla->classname;
	finalized = cla->finalized;
	defclass = cla->defclass;
	native = cla->native->retain();
}

void Bagel_Class::VMgetClassMember(Bagel_StringHolder key, Bagel_Var * out, Bagel_ThreadContext * ctx, bool searchGlobal) const
{
	auto it = varmap.find(key);
	if (it != varmap.end())
	{
		out->forceSet(it->second);
		return;
	}
	it = defclass->varmap.find(key);
	if (it == defclass->varmap.end())
	{
		if (searchGlobal)
		{
			//may be property
			out->forceSet(_globalStructures.globalClosure->getMember(key));
			return;
		}
		throw Bagel_Except(W("成员") + key.getConstStr() + W("不存在"));
	}
	auto &&v = it->second;
	if (v.getType() == VAR_FUNC && v.forceAsFunc()->self.isObject() && v.forceAsFunc()->self.forceAsObject() == defclass)
	{
		out->forceSet(new Bagel_Function(*v.forceAsFunc(), const_cast<Bagel_Class*>(this), const_cast<Bagel_Class*>(this)));
		return;
	}
	else if (v.getType() == VAR_PROP)
	{
		out->forceSet(v);
		ctx->runningclo->hasSelf = true;
		ctx->runningclo->self.forceSet(this);
		ctx->runningclo->clo = (Bagel_Closure *)this;
		return;
		//return v.forceAsProp()->VMGetWithSelf(this, const_cast<Bagel_Class*>(this), nullptr, true);
	}
	else
	{
		out->forceSet(v);
	}
}

void Bagel_Function::assignStructure(Bagel_Function *f, BKE_hashmap<void*, void*> &pMap, bool first)
{
	func = (Bagel_FunctionCode*)f->func;
	self.assignStructure(f->self, pMap);
	if (!first && f->closure)
	{
		if (pMap.contains(f->closure))
		{
			closure = (Bagel_Closure*)pMap[f->closure];
		}
		else
		{
			Bagel_Var v;
			v.assignStructure(f->closure, pMap);
			closure = v.forceAsClosure();
			pMap[f->closure] = closure;
		}
	}
	else
	{
		closure = f->closure;
	}
	name = f->name;
}

void Bagel_Prop::assignStructure(Bagel_Prop *p, BKE_hashmap<void*, void*> &pMap, bool first)
{
	funcget = p->funcget;
	funcset = p->funcset;
	self.assignStructure(p->self, pMap);
	if (!first && p->closure)
	{
		if (pMap.contains(p->closure))
		{
			closure = (Bagel_Closure*)pMap[p->closure];
		}
		else
		{
			closure = new Bagel_Closure();
			closure->assignStructure(p->closure, pMap);
			pMap[p->closure] = closure;
		}
	}
	else
		closure = p->closure;
	setparam = p->setparam;
}

void Bagel_Pointer::assignStructure(Bagel_Pointer * p, BKE_hashmap<void*, void*>& pMap, bool first)
{
	name = p->name;
	var.assignStructure(p->var, pMap);
}
