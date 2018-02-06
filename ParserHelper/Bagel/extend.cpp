#include "extend.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <random>
#include <functional>
#include <algorithm>
#include "bkutf8.h"

#include "Bagel_Parser.h"
#include "Bagel_VM.h"
#include "Bagel_Serializer.h"

#ifdef WIN32
#include <Windows.h>
#endif


////to make object hash different with their raw address, so GC wouldn't regard these value as pointers
//#define HASH_MAGIC 2166136261

#ifndef DOXYGEN_PARSE
namespace ParserUtils
#endif
{
	//native constructor of inner types : number(int) , string, array, dictionary
	//NATIVE_FUNC(number)
	//{
	//	if(PARAMEXIST(0))
	//		return PARAM(0).asNumber();
	//	else
	//		return 0;
	//}

	//NATIVE_FUNC(int)
	//{
	//	if(PARAMEXIST(0))
	//		return PARAM(0).asInteger();
	//	else
	//		return 0;
	//};

	//NATIVE_FUNC(string)
	//{
	//	if(PARAMEXIST(0))
	//		return PARAM(0).asString();
	//	else
	//		return L"";
	//};
	
	NATIVE_FUNC(set)
	{
		MINIMUMPARAMNUM(2);
		auto pointer = Bagel_VM::getInstance()->getVar(PARAM(0), _this);
		pointer->set(PARAM(1));
		RETURNDEFAULT;
	}

	NATIVE_FUNC(array)
	{
		auto arr = new Bagel_Array();
		if (paramarray)
		{
			for (int i = 0; i < PARAMCOUNT(); i++)
			{
				arr->pushMember(PARAM(i));
			}
		}
		return arr;
	};

	NATIVE_FUNC(dictionary)
	{
		Bagel_Dic *dic = new Bagel_Dic();
		for (int i = 0; i < PARAMCOUNT(); i += 2)
		{
			dic->setMember(PARAM(i), PARAM(i + 1));
		}
		return dic;
	}

	//xxx.type
	//deleted
	//NATIVE_GET(type)
	//{
	//	return self->getTypeBKEString();
	//}

	NATIVE_FUNC(equals)
	{
		MINIMUMPARAMNUM(1);
		return self->equals(PARAM(0));
	}

	NATIVE_FUNC(log)
	{
		if (!PARAMEXIST(0))
			RETURNDEFAULT;
		StringVal res;
		PARAM(0).save(res);
		for (int i = 1; i < PARAMCOUNT(); i++)
		{
			//res.clear();
			res += ',';
			PARAM(i).save(res);
		}
		res += '\n';
		Bagel_VM::getInstance()->log(res);
		RETURNDEFAULT;
	}

	NATIVE_FUNC(getClassName)
	{
		MINIMUMPARAMNUM(1);
		auto &v = PARAM(0);
		if (v.getType() == VAR_CLASS)
		{
			return v.forceAsClass()->classname;
		}
		else if (v.getType() == VAR_CLASSDEF)
		{
			return v.forceAsClassDef()->classname;
		}
		else
		{
			return v.getTypeBKEString();
		}
	}

	//eval(str, this, krmode, rawstr)
	NATIVE_FUNC(eval)
	{
		if (PARAMEXIST(0))
		{
			bool krmode = Bagel_Parser::getInstance()->krmode;
			bool rawstr = Bagel_Parser::getInstance()->rawstr;
			Bagel_Parser::getInstance()->krmode = PARAMDEFAULT(2, false);
			Bagel_Parser::getInstance()->rawstr = PARAMDEFAULT(3, false);
			Bagel_Var res;
			try
			{
				//res = Bagel_Parser::getInstance()->eval(PARAM(0));
				res = Bagel_VM::getInstance()->Run(PARAM(0).asString(), Bagel_Closure::global(), PARAM(1), NULL, W("[native function eval]"), NULL, (Bagel_ThreadHandle)ctx);
			}
			catch (Bagel_Except &e)
			{
				Bagel_Parser::getInstance()->krmode = krmode;
				Bagel_Parser::getInstance()->rawstr = rawstr;
				throw e;
			}
			Bagel_Parser::getInstance()->krmode = krmode;
			Bagel_Parser::getInstance()->rawstr = rawstr;
			return res;
		}
		else
		{
			return Bagel_Var();
		}
	}

	//time(expression)
	NATIVE_FUNC(time)
	{
		MINIMUMPARAMNUM(1);
		double start = getutime();
		Bagel_VM::getInstance()->Run(PARAM(0).asString(), Bagel_Closure::global(), PARAM(1), NULL, W("[native function time]"), NULL, (Bagel_ThreadHandle)ctx);
		return (getutime() - start) / 1000;
	}

	NATIVE_FUNC(clock)
	{
		return getutime() / 1000;
	}
	//range(a)-->[0,1,2,...,a-1]
	//range(a,b)->[a,a+1,...,b-1]	//a<b
	//range(a,b)->[a,a-1,...,b+1]	//a>b
	//range(a,b)->[]	//a==b
	NATIVE_FUNC(range)
	{
		MINIMUMPARAMNUM(1);
		Bagel_Array *arr = new Bagel_Array();
		if (PARAMEXIST(1))
		{
			int32_t a = PARAM(0);
			int32_t b = PARAM(1);
			if (b > a)
			{
				arr->vararray.reserve(b - a);
				for (int i = a; i < b; i++)
					arr->pushMember(i);
			}
			else if (a > b)
			{
				arr->vararray.reserve(a - b);
				for (int i = a; i > b; i--)
					arr->pushMember(i);
			}
			return arr;
		}
		int32_t a = PARAM(0);
		arr->vararray.reserve(a);
		for (int i = 0; i < a; i++)
			arr->pushMember(i);
		return arr;
	}

	//a ---- b-1
	NATIVE_FUNC(random)
	{
		MINIMUMPARAMNUM(1);
		int32_t a, b;
		if (!PARAMEXIST(1))
		{
			a = 0;
			b = PARAM(0);
		}
		else
		{
			a = PARAM(0);
			b = PARAM(1);
		}
		float per;
		do
		{
			per = (float)bkpRandomDouble(0., 1.);
		}
		while (per == 1.f);
		return floor(a + (b - a)*per);
	}

	////typeof(xxx)
	//NATIVE_FUNC(typeof)
	//{
	//	return PARAM(0).getTypeBKEString();
	//}

	NATIVE_FUNC(itoa)
	{
		MINIMUMPARAMNUM(1);
		int32_t a = PARAM(0);
		return bkpInt2Str(a);
	}

	NATIVE_FUNC(itoa2)
	{
		MINIMUMPARAMNUM(1);
		int32_t a = PARAM(0);
		u16string str = bkpInt2Str(a);
		for (int i = 0; i < (int)str.size(); i++)
			str[i] += L'０' - L'0';
		return str;
	}

	NATIVE_FUNC(char)
	{
		if (!PARAMEXIST(0))
			return 0;
		if (PARAM(0).getType() == VAR_NUM)
		{
			StringVal ss;
			uint32_t num = (uint32_t)PARAM(0).asInteger();
			if (num < 0x10000)
				ss.push_back((char16_t)num);
			else
			{
				num -= 0x10000;
				int b = num & 0x3FF;
				int a = num >> 10;
				ss.push_back((char16_t)(a + 0xD800));
				ss.push_back((char16_t)(b + 0xDC00));
			}
			return ss;
		}
		auto &ss = PARAM(0).asBKEStr()->getConstStr();
		if (ss.empty())
			return 0;
		int32_t a = ss[0];
		if (a >= 0xD800 && a <= 0xDFFF)
		{
			int32_t b = ss[1];
			return ((a - 0xD800) << 10) + (b - 0xDC00) + 0x10000;
		}
		return a;
	}

	NATIVE_FUNC(hash)
	{
		MINIMUMPARAMNUM(1);
		return BKE_hash(PARAM(0));
	}

	NATIVE_FUNC(hash16)
	{
		return BKE_hash(PARAM(0)) & 0xFFFF;
	}

#ifdef ENABLE_FILE

	NATIVE_FUNC(load)
	{
		MINIMUMPARAMNUM(1);
		StringVal fname = PARAM(0);
		StringVal res;
		bool re = _globalStructures.readFunc(res, fname, 0);
		if (!re)
			throw Bagel_Except(W("打开文件失败"));
		auto arr = new Bagel_Array();
		BKE_Char *buf3 = &res[0];
		int i = 0;
		u16string tmp;
		tmp.reserve(256);
		int32_t size = res.size();
		while (i < size)
		{
			if (buf3[i] == L'\r')
			{
				arr->pushMember(tmp);
				tmp.clear();
				if (buf3[i + 1] == L'\n')
					i++;
			}
			else if (buf3[i] == L'\n')
			{
				arr->pushMember(tmp);
				tmp.clear();
			}
			else
				tmp.push_back(buf3[i]);
			i++;
		}
		arr->pushMember(tmp);
		return arr;
	}

	NATIVE_FUNC(save)
	{
		MINIMUMPARAMNUM(1);
		StringVal fname = PARAM(0);
		Bagel_Array *arr = self->forceAsArray();
		if (arr->getCount() == 0)
			RETURNDEFAULT;
		StringVal str2;
		char16_t crlf[] = { '\r', '\n' };
		try
		{
			str2 += (StringVal)arr->getMember(0);
		}
		catch (...)
		{
		}
		for (int i = 1; i < arr->getCount(); i++)
		{
			str2 += crlf;
			try
			{
				str2 += (StringVal)arr->getMember(i);
			}
			catch (...)
			{
			}
		}
		bool re = _globalStructures.writeFunc(str2, fname, 0, false);
		if (!re)
			throw Bagel_Except(L"写入文件失败");
		RETURNDEFAULT;
	}

	//saveStruct(filename, mode = "")
	/*
	mode="oxxx":write from pos xxx
	mode="z":write compress file (if we can)
	*/
	NATIVE_FUNC(saveStruct)
	{
		MINIMUMPARAMNUM(1);
		StringVal fname = PARAM(0);
		StringVal res;
		self->save(res);
		u16string mode = PARAMDEFAULT(1, u16string());
		int pos = 0;
		bool compress = false;
		if (!mode.empty())
		{
			if (mode[0] == 'z')
				compress = true;
			else if (mode[0] == 'o')
				pos = bkpStr2Int(mode.substr(1));
		}
		bool re = _globalStructures.writeFunc(res, fname, pos, compress);
		if (!re)
			throw Bagel_Except(L"写入文件失败");
		RETURNDEFAULT;
	}

	//loadfile(filename, offset=0)
	NATIVE_FUNC(loadFile)
	{
		MINIMUMPARAMNUM(1);
		StringVal fname = PARAM(0);
		StringVal res;
		bool re = _globalStructures.readFunc(res, fname, PARAMDEFAULT(1, 0));
		if (!re)
			throw Bagel_Except(L"打开文件失败");
		else
			return res;
	}

	//bool saveFile(filename, string, pos)
	//pos==-1, append
	NATIVE_FUNC(saveFile)
	{
		MINIMUMPARAMNUM(2);
		StringVal fname = PARAM(0);
		auto &res = PARAM(1).asBKEStr()->getConstStr();
		int32_t pos = PARAMDEFAULT(2, 0);
		bool re = _globalStructures.writeFunc(res, fname, pos, false);
		if (!re)
			throw Bagel_Except(L"写入文件失败");
		RETURNDEFAULT;
	}

	//bool saveFile(filename, var, pos)
	//pos==-1, append
	NATIVE_FUNC(saveVariable)
	{
		static Bagel_Serializer sel;
		MINIMUMPARAMNUM(2);
		StringVal fname = PARAM(0);
		auto data = sel.serialize(PARAM(1));
		int32_t pos = PARAMDEFAULT(2, 0);
		auto re = _globalStructures.writeBinary(&data[0], data.size() * 4, fname, pos, false);
		if (!re)
			throw Bagel_Except(L"写入文件失败");
		RETURNDEFAULT;
	}

	NATIVE_FUNC(appendFile)
	{
		MINIMUMPARAMNUM(2);
		StringVal fname = PARAM(0);
		auto &res = PARAM(1).asBKEStr()->getConstStr();
		bool re = _globalStructures.writeFunc(res, fname, -1, false);
		if (!re)
			throw Bagel_Except(L"写入文件失败");
		RETURNDEFAULT;
	}

	NATIVE_FUNC(evalFile)
	{
		MINIMUMPARAMNUM(1);
		StringVal fname = PARAM(0);
		bool krmode = Bagel_Parser::getInstance()->krmode;
		bool rawstr = Bagel_Parser::getInstance()->rawstr;
		Bagel_Parser::getInstance()->krmode = PARAMDEFAULT(2, false);
		Bagel_Parser::getInstance()->rawstr = PARAMDEFAULT(3, false);
		Bagel_Var val;
		StringVal res;
		int isStream;
		bool re = _globalStructures.readBinary(res, &isStream, fname, 0);
		if (!re)
			throw Bagel_Except(L"打开文件失败");
		_globalStructures.imports->insertKey(fname);
		try
		{
			if (isStream)
			{
				static Bagel_Serializer sel;
				val = sel.parse((const uint32_t *)&res[0], res.size() / 2);
				if (val.getType() == VAR_BYTECODE_P)
				{
					val = Bagel_VM::getInstance()->Run(val.forceAsObject<Bagel_ByteCode>(), Bagel_Closure::global(), PARAM(1), NULL, W("[native function evalFile]"), NULL, (Bagel_ThreadHandle)ctx);
				}
			}
			else
			{
				val = Bagel_VM::getInstance()->Run(res, Bagel_Closure::global(), PARAM(1), NULL, W("[native function evalFile]"), NULL, (Bagel_ThreadHandle)ctx);
			}
		}
		catch (Bagel_Except &e)
		{
			Bagel_Parser::getInstance()->krmode = krmode;
			Bagel_Parser::getInstance()->rawstr = rawstr;
			throw e;
		}
		Bagel_Parser::getInstance()->krmode = krmode;
		Bagel_Parser::getInstance()->rawstr = rawstr;
		return val;
	}

	NATIVE_FUNC(importFile)
	{
		MINIMUMPARAMNUM(1);
		StringVal fname = PARAM(0);
		if (_globalStructures.imports->contains(fname))
			RETURNDEFAULT;
		bool krmode = Bagel_Parser::getInstance()->krmode;
		bool rawstr = Bagel_Parser::getInstance()->rawstr;
		Bagel_Parser::getInstance()->krmode = PARAMDEFAULT(2, false);
		Bagel_Parser::getInstance()->rawstr = PARAMDEFAULT(3, false);
		StringVal res;
		int isStream;
		bool re = _globalStructures.readBinary(res, &isStream, fname, 0);
		if (!re)
			throw Bagel_Except(L"打开文件失败");
		_globalStructures.imports->insertKey(fname);
		try
		{
			if (isStream)
			{
				static Bagel_Serializer sel;
				auto val = sel.parse((const uint32_t *)&res[0], res.size() / 2);
				if (val.getType() == VAR_BYTECODE_P)
				{
					val = Bagel_VM::getInstance()->Run(val.forceAsObject<Bagel_ByteCode>(), Bagel_Closure::global(), PARAM(1), NULL, W("[native function evalFile]"), NULL, (Bagel_ThreadHandle)ctx);
				}
			}
			else
			{
				Bagel_VM::getInstance()->Run(res, Bagel_Closure::global(), PARAM(1), NULL, W("[native function evalFile]"), NULL, (Bagel_ThreadHandle)ctx);
			}
		}
		catch (Bagel_Except &e)
		{
			Bagel_Parser::getInstance()->krmode = krmode;
			Bagel_Parser::getInstance()->rawstr = rawstr;
			throw e;
		}
		Bagel_Parser::getInstance()->krmode = krmode;
		Bagel_Parser::getInstance()->rawstr = rawstr;
		RETURNDEFAULT;
	}

#endif

	NATIVE_FUNC(toString)
	{
		if (!PARAMEXIST(0))
			RETURNDEFAULT;
		StringVal res;
		bool format = PARAMDEFAULT(1, false);
		PARAM(0).save(res, format, 0);
		return res;
	}

	//loadClass(classname, dic)
	//loadClass("xd", %[a:1])
	NATIVE_FUNC(loadClass)
	{
		MINIMUMPARAMNUM(2);
		auto name = PARAM(0).asBKEStr();
		auto var = Bagel_Closure::global()->getMember(name);
		if (var.getType() != VAR_CLASSDEF)
		{
			throw Bagel_Except(W("传入的第一个参数必须是一个类的名称。"));
		}
		if (PARAM(1).getType() != VAR_DIC)
		{
			throw Bagel_Except(W("传入的第二个参数必须是字典。"));
		}
		auto cla = var.forceAsClassDef()->VMcreateInstance(NULL, true);
		auto obj = cla.forceAsClassDef();
		obj->varmap.Union(PARAM(1).forceAsDic()->varmap, true);
		if (obj->native)
			obj->native->nativeLoad(PARAMDEFAULT(2, Bagel_Var()));
		return cla;
	}

	NATIVE_GET(length)
	{
		switch (self->getType())
		{
		case VAR_STR:
			return self->forceAsBKEStr()->size();
		case VAR_ARRAY:
			return (self->forceAsArray())->getCount();
		case VAR_DIC:
			return (self->forceAsDic())->getCount();
		default:
			RETURNDEFAULT
		}
	}

	NATIVE_SET(length)
	{
		int s = PARAM(0);
		if (s < 0)
			throw Bagel_Except(W("不合法的参数"));
		self->forceAsArray()->setLength(s);
		RETURNDEFAULT
	}

	//string.beginWith(xxx)
	NATIVE_FUNC(beginWith)
	{
		MINIMUMPARAMNUM(1);
	#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (PARAM(0).forceAsClass()->isInstanceof(W("Regex")))
			{
				Regex *r = (Regex *)(PARAM(0).forceAsClass()->native);
				std::wsmatch m;
				auto src = self->forceAsBKEStr()->getWString();
				std::regex_search(src, m, r->reg);
				if (m.empty())
					return false;
				return m.prefix().length() == 0;
			}
			throw Bagel_Except(W("参数1必须是Regex的子类或字符串"));
		}
	#endif
		Bagel_String *str = PARAM(0).asBKEStr();
		if (str->size() > self->forceAsBKEStr()->size())
			return false;
		return self->forceAsBKEStr()->beginWith(str->getConstStr().c_str());
	}

	//string.endWith(xxx)
	NATIVE_FUNC(endWith)
	{
		MINIMUMPARAMNUM(1);
	#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (PARAM(0).forceAsClass()->isInstanceof(W("Regex")))
			{
				Regex *r = (Regex *)(PARAM(0).forceAsClass()->native);
				auto s = self->forceAsBKEStr()->getWString();
				for (int i = (int)s.size() - 1; i >= 0; i--)
				{
					if (std::regex_match(s.substr(i), r->reg))
						return true;
				}
				return false;
			}
			throw Bagel_Except(L"参数1必须是Regex的子类或字符串");
		}
	#endif
		Bagel_String *str = PARAM(0).asBKEStr();
		return self->forceAsBKEStr()->endWith(str->getConstStr());
	}

	NATIVE_FUNC(trimLeft)
	{
		int i = 0;
		const StringVal &str = self->asBKEStr()->getConstStr();
		if (str.empty())
		{
			return Bagel_StringHolder();
		}
		while (i < str.size() && bkpIsSpace(str[i]))
		{
			++i;
		}
		return str.substr(i);
	}

	NATIVE_FUNC(trimRight)
	{
		const StringVal &str = self->asBKEStr()->getConstStr();
		if (str.empty())
		{
			return Bagel_StringHolder();
		}
		int i = str.size() - 1;
		while (i >= 0 && bkpIsSpace(str[i]))
		{
			--i;
		}
		return str.substr(0, i + 1);
	}

	NATIVE_FUNC(trim)
	{
		const StringVal &str = self->asBKEStr()->getConstStr();
		if (str.empty())
		{
			return Bagel_StringHolder();
		}
		int i = 0, j = str.size() - 1;
		while (i < str.size() && bkpIsSpace(str[i]))
		{
			++i;
		}
		while (j >= i && bkpIsSpace(str[j]))
		{
			--j;
		}
		return str.substr(i, j - i + 1);
	}

	//replace(a, b)		a=>b
	NATIVE_FUNC(replace)
	{
		MINIMUMPARAMNUM(2);
	#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			Bagel_Var v[2] = { *self, PARAM(1) };
			return Regex::nativeFunc_replaceAll(&paramarray[0], v, 2, _this, ctx);
		}
	#endif
		StringVal old = PARAM(0);
		int32_t oldsize = static_cast<int32_t>(old.size());
		StringVal newvalue = PARAM(1);
		//int32_t newsize=static_cast<int32_t>(newvalue.size());
		auto &s = self->forceAsBKEStr()->getConstStr();
		//int32_t ssize=static_cast<int32_t>(s.size());
		StringVal res;
		int32_t i = 0;
		int32_t start;
		do
		{
			start = i;
			i = static_cast<int32_t>(s.find(old, start));
			if (i == s.npos)
			{
				res += s.substr(start);
				return res;
			}
			res += s.substr(start, i - start) + newvalue;
			i += oldsize;
		}
		while (1);
	}

	NATIVE_FUNC(indexOf)
	{
		MINIMUMPARAMNUM(1);
		int32_t start = PARAMDEFAULT(1, 0);
		if (start < 0 || start > self->forceAsBKEStr()->size())
			return -1;
	#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (PARAM(0).forceAsClass()->isInstanceof(W("Regex")))
			{
				Regex *r = (Regex *)(PARAM(0).forceAsClass()->native);
				auto tmp = self->forceAsBKEStr()->getWString().substr(start);
				std::wsmatch m;
				if (!std::regex_search(tmp, m, r->reg))
					return -1;
				else
					return (int32_t)m.position() + start;
			}
			throw Bagel_Except(W("参数1必须是Regex的子类或字符串"));
		}
	#endif
		int32_t s = static_cast<int32_t>(self->forceAsBKEStr()->getConstStr().find(PARAM(0).asString(), start));
		if (s == wstring::npos)
			return -1;
		return s;
	}

	NATIVE_FUNC(lastIndexOf)
	{
		MINIMUMPARAMNUM(1);
		int32_t endpos = PARAMDEFAULT(1, (int32_t)StringVal::npos);
	#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (PARAM(0).forceAsClass()->isInstanceof(W("Regex")))
			{
				Regex *r = (Regex *)(PARAM(0).forceAsClass()->native);
				std::wsmatch m;
				const wstring &s = self->forceAsBKEStr()->getWString();
				for (int i = endpos - 1; i >= 0; i--)
				{
					std::regex_search((const wstring &)s.substr(i), m, r->reg);
					if (!m.empty())
						return (int32_t)(i + m.position());
				}
				return -1;
			}
			throw Bagel_Except(W("参数1必须是Regex的子类或字符串"));
		}
	#endif
		int32_t s = static_cast<int32_t>(self->forceAsBKEStr()->getConstStr().rfind(PARAM(0).asString(), endpos));
		if (s == StringVal::npos)
			return -1;
		return s;
	}

	NATIVE_FUNC(toLowerCase)
	{
		Bagel_StringHolder s = self->forceAsBKEStr();
		return s.toLowerCase();
	}

	NATIVE_FUNC(toUpperCase)
	{
		Bagel_StringHolder s = self->forceAsBKEStr();
		return s.toUpperCase();
	}

	NATIVE_FUNC(substring)
	{
		MINIMUMPARAMNUM(1);
		int32_t start = (int32_t)PARAM(0);
		if (start < 0 || self->forceAsBKEStr()->size() <= start)
			RETURNDEFAULT;
		if (PARAMEXIST(1))
			return self->forceAsBKEStr()->substr((int32_t)PARAM(0), (int32_t)PARAM(1));
		else
			return self->forceAsBKEStr()->substr((int32_t)PARAM(0));
	}

	NATIVE_FUNC(sprintf)
	{
		const wstring &src = self->forceAsBKEStr()->getWString();
		wstring res;
		int i = 0;
		int paramno = 0;
		int32_t srcsize = (int32_t)src.size();
		while (i < srcsize)
		{
			if (src[i] != L'%')
				res.push_back(src[i]);
			else
			{
				i++;
				if (i >= srcsize)
				{
					res.push_back(L'%');
					return res;
				}
				else if (src[i] == L'%')
				{
					res.push_back(L'%');
				}
				else
				{
					wstring mode = L"%";
					wchar_t ch;
					do
					{
						ch = src[i];
						mode.push_back(ch);
						i++;
					}
					while (i < srcsize && !iswalpha(ch));
					if (!PARAMEXIST(paramno))
						throw Bagel_Except(L"参数数目不足");
				#ifdef _MSC_VER
					wchar_t buf2[512];
					if (towupper(ch) == L'S')
						swprintf(buf2, 512, mode.c_str(), ((wstring)PARAM(paramno++)).c_str());
					else if (towupper(ch) == L'G')
						swprintf(buf2, 512, mode.c_str(), PARAM(paramno++).asNumber());
					else if (towupper(ch) == L'F')
						swprintf(buf2, 512, mode.c_str(), (float)(PARAM(paramno++).asNumber()));
					else
						swprintf(buf2, 512, mode.c_str(), PARAM(paramno++).asInteger());
					res.append(buf2);
				#else
					char buf3[1024];
					string _mode = UniToUTF8(mode);
					if (towupper(ch) == L'S')
					{
						u16string str = PARAM(paramno++).asString();
						string __tmp = UTF16ToUTF8(str);
						snprintf(buf3, 1024, _mode.c_str(), __tmp.c_str());
					}
					else if (towupper(ch) == L'G')
					{
						snprintf(buf3, 1024, _mode.c_str(), PARAM(paramno++).asNumber());
					}
					else if (towupper(ch) == L'F')
					{
						snprintf(buf3, 1024, _mode.c_str(), (float)(PARAM(paramno++).asNumber()));
					}
					else
					{
						snprintf(buf3, 1024, _mode.c_str(), (int)PARAM(paramno++).asInteger());
					}
					res.append(UniFromUTF8(buf3, (uint32_t)strlen(buf3)));
				#endif

					continue;
				}
			}
			i++;
		}
		return res;
	}

	//xxx.split(ch, ignorenull=false)
	//"233233".split("3", false)=["2", void, "2", void]
	//"233233".split("3", true)=["2", "2"]
	NATIVE_FUNC(split)
	{
		MINIMUMPARAMNUM(1);
		bool ignorenull = PARAMDEFAULT(1, false);
	#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (PARAM(0).forceAsClass()->isInstanceof(L"Regex"))
			{
				Regex *r = (Regex *)(PARAM(0).forceAsClass()->native);
				std::wsmatch m;
				Bagel_Array *arr = new Bagel_Array();
				wstring tmp = self->forceAsBKEStr()->getWString();
				std::regex_search(tmp, m, r->reg);
				if (m.empty())
				{
					arr->pushMember(tmp);
					return arr;
				}
				while (m.suffix().length() > 0)
				{
					if (m.prefix().length() > 0 || !ignorenull)
						arr->pushMember(m.prefix().str());
					tmp = m.suffix();
					if (!std::regex_search(tmp, m, r->reg))
						break;
				}
				if (m.prefix().length() > 0 || !ignorenull)
					arr->pushMember(m.prefix().str());
				if (m.suffix().length() > 0 || !ignorenull)
					arr->pushMember(m.suffix().str());
				return arr;
			}
			throw Bagel_Except(L"参数1必须是Regex的子类或字符串");
		}
	#endif
		auto &src = self->forceAsBKEStr()->getConstStr();
		StringVal split = PARAM(0);
		int32_t splitsize = static_cast<int32_t>(split.size());
		Bagel_Array *arr = new Bagel_Array();
		int start = 0;
		int32_t srcsize = (int32_t)src.size();
		while (start < srcsize)
		{
			int32_t pos = static_cast<int32_t>(src.find(split, start));
			if (pos == src.npos)
				break;
			if (pos > start || !ignorenull)
				arr->pushMember(src.substr(start, pos - start));
			start = pos + splitsize;
		}
		if (start < srcsize)
			arr->pushMember(src.substr(start));
		else if (!ignorenull)
			arr->pushMember(Bagel_Var());
		return arr;
	}

	//以str中任意一个字符分割
	//xxx.split(str, ignorenull=false)
	NATIVE_FUNC(splitany)
	{
		MINIMUMPARAMNUM(1);
		bool ignorenull = PARAMDEFAULT(1, false);
		auto &src = self->forceAsBKEStr()->getConstStr();
		StringVal split = PARAM(0);
		int32_t splitsize = 1;
		Bagel_Array *arr = new Bagel_Array();
		int start = 0;
		int32_t srcsize = (int32_t)src.size();
		while (start < srcsize)
		{
			int32_t pos = static_cast<int32_t>(src.find_first_of(split, start));
			if (pos == src.npos)
				break;
			if (pos > start || !ignorenull)
				arr->pushMember(src.substr(start, pos - start));
			start = pos + splitsize;
		}
		if (start < srcsize)
			arr->pushMember(src.substr(start));
		else if (!ignorenull)
			arr->pushMember(Bagel_Var());
		return arr;
	}

	//add(xx,xx,xx,...)
	NATIVE_FUNC(add)
	{
		int i = 0;
		long cnt = PARAMCOUNT();
		while (i < cnt)
			self->forceAsArray()->pushMember(PARAM(i++));
		RETURNDEFAULT;
	}

	NATIVE_FUNC(remove)
	{
		int i = 0;
		while (i < PARAMCOUNT())
		{
			if (self->getType() == VAR_ARRAY)
				self->forceAsArray()->deleteMember(PARAM(i));
			else
				self->forceAsDic()->deleteMember(PARAM(i));
			i++;
		}
		RETURNDEFAULT;
	}

	NATIVE_FUNC(erase)
	{
		int i = 0;
		while (i < PARAMCOUNT())
		{
			if (self->getType() == VAR_ARRAY)
				self->forceAsArray()->deleteMemberIndex(PARAM(i).asInteger());
			else
				self->forceAsDic()->deleteMemberIndex(PARAM(i).asBKEStr());
			i++;
		}
		RETURNDEFAULT;
	}

	NATIVE_FUNC(clear)
	{
		switch (self->getType())
		{
		case VAR_STR:
			const_cast<Bagel_Var*>(self)->forceSet(Bagel_StringHolder());
			break;
		case VAR_ARRAY:
			self->forceAsArray()->clear();
			break;
		case VAR_DIC:
			self->forceAsDic()->clear();
			break;
		}
		RETURNDEFAULT;
	}

	//insert(index, var)
	NATIVE_FUNC(insert)
	{
		MINIMUMPARAMNUM(2);
		self->forceAsArray()->insertMember(PARAM(0).asInteger(), PARAM(1));
		RETURNDEFAULT;
	}

	NATIVE_FUNC(find)
	{
		MINIMUMPARAMNUM(1);
		auto &var = PARAM(0);
		switch (self->getType())
		{
		case VAR_ARRAY:
			{
				auto &&v = self->forceAsArray()->vararray;
				for (int i = 0; i < v.size(); i++)
				{
					if (var == v[i])
						return i;
				}
				return -1;
			}
		case VAR_DIC:
			{
				auto it = self->forceAsDic()->varmap.find(var.asBKEStr());
				if (it == self->forceAsDic()->varmap.end())
					RETURNDEFAULT;
				return 1;
			}
		}
		throw Bagel_Except(W("只能用于数组或字典类型"));
	}

	NATIVE_FUNC(join)
	{
		StringVal s;
		StringVal st;
		if (PARAMEXIST(0))
			st = PARAM(0).asString();
		auto &&arr = self->forceAsArray()->vararray;
		if (arr.size() > 0)
			s = arr[0].asString();
		for (int i = 1; i < arr.size(); i++)
			s += st + (StringVal)arr[i];
		return s;
	}

	NATIVE_FUNC(array_random)
	{
		auto &&v = self->forceAsArray()->vararray;
		if (v.size() == 0)
			RETURNDEFAULT;
		return v[((uint32_t)bkpRandomInt()) % v.size()];
	}

	//randomPick(n=1)
	//return [array]
	NATIVE_FUNC(randomPick)
	{
		int n = PARAMDEFAULT(0, 1);
		auto &&v = self->forceAsArray()->vararray;
		int N = v.size();
		auto res = new Bagel_Array();
		if (N == 0 || n <= 0)
			return res;
		if (6 * n < N && n <= 15)
		{
			int choose[15];
			int choosesize = 0;
			while (choosesize < n)
			{
				int i = bkpRandomInt(0, N);
				bool r = false;
				for (int j = 0; j < choosesize; j++)
				{
					if (choose[j] == i)
					{
						r = true;
						break;
					}
				}
				if (!r)
				{
					choose[choosesize++] = i;
					res->pushMember(v[i]);
				}
			}
		}
		else if (N <= 1024)
		{
			int idx[1024];
			for (int i = 0; i < N; i++)
				idx[i] = i;
			int i = 0;
			while (i < n)
			{
				int r = bkpRandomInt(0, N - i);
				i++;
				res->pushMember(v[idx[r]]);
				idx[r] = idx[N - i];
			}
		}
		else
		{
			int *idx = new int[N];
			for (int i = 0; i < N; i++)
				idx[i] = i;
			int i = 0;
			while (i < n)
			{
				int r = bkpRandomInt(0, N - i);
				i++;
				res->pushMember(v[idx[r]]);
				idx[r] = idx[N - i];
			}
			delete[] idx;
		}
		return res;
	}

	//shuffle(int n=0)
	NATIVE_FUNC(shuffle)
	{
		int n = PARAMDEFAULT(0, 0);
		auto &&v = self->forceAsArray()->vararray;
		int N = v.size();
		if (n <= 0)
			n = N;
		int i = 0;
		while (i < n)
		{
			int r = bkpRandomInt(0, N - i) + i;
			auto tmp = v[r];
			v[r] = v[i];
			v[i] = tmp;
			i++;
		}
		RETURNDEFAULT;
	}

	NATIVE_FUNC(take)
	{
		MINIMUMPARAMNUM(1);
		auto &&v = self->forceAsArray()->vararray;
		if (v.size() == 0)
			RETURNDEFAULT;
		int index = PARAM(0);
		if (index >= v.size())
		{
			RETURNDEFAULT;
		}
		auto t = v[index];
		v.erase(index);
		return t;
	}

	NATIVE_FUNC(takeRandom)
	{
		auto &&v = self->forceAsArray()->vararray;
		if (v.size() == 0)
			RETURNDEFAULT;
		int index = (int)bkpRandomInt(0, v.size());
		auto t = v[index];
		v.erase(index);
		return t;
	}

	//sort([func])
	//sort("mode")
	//mode='0','9','+','-','a','z'
	//default mode is +
	NATIVE_FUNC(sort)
	{
		auto &&arr = self->forceAsArray()->vararray;
		Bagel_Var ss = W("+");
		if (PARAMEXIST(0))
			ss = PARAM(0);
		if (ss.getType() == VAR_FUNC)
		{
			auto func = ss.forceAsFunc();
			try
			{
				sort(arr.begin(), arr.end(), [func](const Bagel_Var& a, const Bagel_Var& b)
				{
					Bagel_Var vv[2] = { a, b };
					return (bool)func->VMRun(vv, 2);
				});
			}
			catch (std::exception &)
			{
			}
		}
		else if (ss.getType() == VAR_STR)
		{
			char16_t mode = ss.forceAsBKEStr()->operator[](0);
			try
			{
				switch (mode)
				{
				case '-':
					sort(arr.begin(), arr.end(), [](const Bagel_Var& a, const Bagel_Var& b)
					{
						return a > b;
					});
					break;
				case '0':
					sort(arr.begin(), arr.end(), [](const Bagel_Var& a, const Bagel_Var& b)
					{
						return (double)a < (double)b;
					});
					break;
				case '9':
					sort(arr.begin(), arr.end(), [](const Bagel_Var& a, const Bagel_Var& b)
					{
						return (double)a > (double)b;
					});
					break;
				case 'a':
					sort(arr.begin(), arr.end(), [](const Bagel_Var& a, const Bagel_Var& b)
					{
						return (u16string)a < (u16string)b;
					});
					break;
				case 'z':
					sort(arr.begin(), arr.end(), [](const Bagel_Var& a, const Bagel_Var& b)
					{
						return (u16string)a >(u16string)b;
					});
					break;
				default:
					sort(arr.begin(), arr.end(), [](const Bagel_Var& a, const Bagel_Var& b)
					{
						return a < b;
					});
					break;
				}
			}
			catch (std::exception &)
			{
			}
		}
		else
		{
			throw Bagel_Except(W("sort的参数必须是个排序函数或表示排序模式的字符串。"));
		}
		RETURNDEFAULT;
	}

	NATIVE_FUNC(concat)
	{
		Bagel_Var arr = self->clone();
		for (int i = 0; i < PARAMCOUNT(); i++)
		{
			auto &v = paramarray[i];
			if (v.getType() == VAR_NONE)
			{
				continue;
			}
			else if (v.getType() == VAR_ARRAY)
			{
				arr.forceAsArray()->concat(v.forceAsArray());
			}
			else
			{
				throw(Bagel_Except(W("concat的参数必须是数组。")));
			}
		}
		return arr;
	}

	//member(key)，用来引用key和方法重名的键值
	NATIVE_FUNC(member)
	{
		MINIMUMPARAMNUM(1);
		if (PARAMCOUNT() == 1)
			return self->forceAsDic()->getMember(PARAM(0));
		else
			return self->forceAsDic()->setMember(PARAM(0), PARAM(1));
	}

	NATIVE_FUNC(clone)
	{
		Bagel_Var res;
		res.copyFrom(*self);
		return res;
	}

	NATIVE_FUNC(update)
	{
		MINIMUMPARAMNUM(1);
		auto &v = PARAM(0);
		if (v.getType() == VAR_NONE)
		{
			RETURNDEFAULT;
		}
		else if (v.getType() == VAR_DIC)
		{
			self->forceAsDic()->update(v.forceAsDic());
			RETURNDEFAULT;
		}
		throw(Bagel_Except(W("字典的update操作需要右操作数为一个字典。")));
	}

	NATIVE_FUNC(except)
	{
		MINIMUMPARAMNUM(1);
		auto &v = PARAM(0);
		if (v.getType() == VAR_NONE)
		{
			RETURNDEFAULT;
		}
		else if (v.getType() == VAR_DIC)
		{
			self->forceAsDic()->except(v.forceAsDic());
			RETURNDEFAULT;
		}
		throw(Bagel_Except(W("字典的except操作需要右操作数为一个字典。")));
	}

	NATIVE_FUNC(toArray)
	{
		return self->forceAsDic()->toArray();
	}

	NATIVE_FUNC(getKeyArray)
	{
		auto &&varmap = self->forceAsDic()->varmap;
		Bagel_Array *arr = new Bagel_Array();
		for (auto it = varmap.begin(); it != varmap.end(); it++)
		{
			arr->pushMember(it->first);
		}
		return arr;
	}

	NATIVE_FUNC(getValueArray)
	{
		auto &&varmap = self->forceAsDic()->varmap;
		Bagel_Array *arr = new Bagel_Array();
		for (auto it = varmap.begin(); it != varmap.end(); it++)
		{
			arr->pushMember(it->second);
		}
		return arr;
	}

	//same as sort
	//sortKeyByValue([func])
	//sortKeyByValue("mode")
	//mode='0','9','+','-','a','z'
	//default mode is +
	NATIVE_FUNC(sortKeyByValue)
	{
		auto &&varmap = self->forceAsDic()->varmap;
		auto v = new Bagel_Array();
		v->vararray.reserve(varmap.getCount());
		for (auto &&i : varmap)
		{
			v->vararray.push_back(i.first);
		}
		Bagel_Var ss = W("+");
		if (PARAMEXIST(0))
			ss = PARAM(0);
		if (ss.getType() == VAR_FUNC)
		{
			auto func = ss.forceAsFunc();
			try
			{
				sort(v->vararray.begin(), v->vararray.end(), [&varmap, func](const Bagel_Var& a, const Bagel_Var& b)
				{
					Bagel_Var vv[2] = { a, b };
					return (bool)func->VMRun(vv, 2);
				}
				);
			}
			catch (std::exception &)
			{
			}
		}
		else if (ss.getType() == VAR_STR)
		{
			char16_t mode = ss.forceAsBKEStr()->operator[](0);
			auto &arr = v->vararray;
			try
			{
				switch (mode)
				{
				case '-':
					sort(arr.begin(), arr.end(), [&varmap](const Bagel_Var& a, const Bagel_Var& b)
					{
						return varmap[(a).forceAsBKEStr()] > varmap[(b).forceAsBKEStr()];
					});
					break;
				case '0':
					sort(arr.begin(), arr.end(), [&varmap](const Bagel_Var& a, const Bagel_Var& b)
					{
						return (double)varmap[(a).forceAsBKEStr()] < (double)varmap[(b).forceAsBKEStr()];
					});
					break;
				case '9':
					sort(arr.begin(), arr.end(), [&varmap](const Bagel_Var& a, const Bagel_Var& b)
					{
						return (double)varmap[(a).forceAsBKEStr()] > (double)varmap[(b).forceAsBKEStr()];
					});
					break;
				case 'a':
					sort(arr.begin(), arr.end(), [&varmap](const Bagel_Var& a, const Bagel_Var& b)
					{
						return (u16string)varmap[(a).forceAsBKEStr()] < (u16string)varmap[(b).forceAsBKEStr()];
					});
					break;
				case 'z':
					sort(arr.begin(), arr.end(), [&varmap](const Bagel_Var& a, const Bagel_Var& b)
					{
						return (u16string)varmap[(a).forceAsBKEStr()] > (u16string)varmap[(b).forceAsBKEStr()];
					});
					break;
				default:
					sort(arr.begin(), arr.end(), [&varmap](const Bagel_Var& a, const Bagel_Var& b)
					{
						return varmap[(a).forceAsBKEStr()] < varmap[(b).forceAsBKEStr()];
					});
					break;
				}
			}
			catch (std::exception&)
			{
			}
		}
		else
		{
			throw Bagel_Except(W("sort的参数必须是个排序函数或表示排序模式的字符串。"));
		}
		return v;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Date
#endif
	NATIVECLASS_CREATENEW()
	{
		return new Date();
	}

	REG_FUNC_BEGIN(Date)
	{
		FUNC_INFO(getYear),
			FUNC_INFO(getMonth),
			FUNC_INFO(getDay),
			FUNC_INFO(getHour),
			FUNC_INFO(getMinute),
			FUNC_INFO(getSecond),

			FUNC_INFO(setYear),
			FUNC_INFO(setMonth),
			FUNC_INFO(setDay),
			FUNC_INFO(setHour),
			FUNC_INFO(setMinute),
			FUNC_INFO(setSecond),

			FUNC_INFO(format),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_FUNC(Date);
	}

	NATIVECLASS_FUNC(getYear)
	{
		GETINSTANCE();
		return instance->_tm->tm_year + 1900;
	}

	NATIVECLASS_FUNC(setYear)
	{
		MINIMUMPARAMNUM(1);
		GETINSTANCE();
		instance->_tm->tm_year = PARAM(0).asInteger() - 1970;
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getMonth)
	{
		GETINSTANCE();
		return instance->_tm->tm_mon + 1;
	}

	NATIVECLASS_FUNC(setMonth)
	{
		MINIMUMPARAMNUM(1);
		GETINSTANCE();
		instance->_tm->tm_mon = PARAM(0).asInteger() - 1;
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getDay)
	{
		GETINSTANCE();
		return instance->_tm->tm_mday;
	}

	NATIVECLASS_FUNC(setDay)
	{
		MINIMUMPARAMNUM(1);
		GETINSTANCE();
		instance->_tm->tm_mday = PARAM(0).asInteger();
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getHour)
	{
		GETINSTANCE();
		return instance->_tm->tm_hour;
	}

	NATIVECLASS_FUNC(setHour)
	{
		MINIMUMPARAMNUM(1);
		GETINSTANCE();
		instance->_tm->tm_hour = PARAM(0).asInteger();
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getMinute)
	{
		GETINSTANCE();
		return instance->_tm->tm_min;
	}

	NATIVECLASS_FUNC(setMinute)
	{
		MINIMUMPARAMNUM(1);
		GETINSTANCE();
		instance->_tm->tm_min = PARAM(0).asInteger();
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getSecond)
	{
		GETINSTANCE();
		return instance->_tm->tm_sec;
	}

	NATIVECLASS_FUNC(setSecond)
	{
		MINIMUMPARAMNUM(1);
		GETINSTANCE();
		instance->_tm->tm_sec = PARAM(0).asInteger();
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(format)
	{
		MINIMUMPARAMNUM(1);
		GETINSTANCE();
		StringVal format = PARAM(0).asString();
		StringVal res;
		uint32_t i = 0;
		uint32_t s = (uint32_t)format.size();
		bool shrink = false;	//正处于#的作用下
		while (i < s)
		{
			auto ch = format[i++];
			if (ch != '%' && !shrink)
				res.push_back(ch);
			else
			{
				if (i >= s)
					return res;
				if (!shrink)
					ch = format[i++];
				u16string tmpstr(1, ch);
				switch (ch)
				{
				case '%':
					break;
				case '#':
					if (!shrink)
					{
						shrink = true;
						continue;
					}
					break;
				case 'Y':
					tmpstr = bkpInt2Str(instance->_tm->tm_year + 1900);
					if (tmpstr.size() < 4)
						tmpstr = u16string(4 - tmpstr.size(), L'0');
					if (shrink)
						tmpstr = tmpstr.substr(tmpstr.size() - 2);
					break;
				case 'M':
					tmpstr = bkpInt2Str(instance->_tm->tm_mon + 1);
					if (!shrink && instance->_tm->tm_mon < 9)
						tmpstr = (char16_t)'0' + tmpstr;
					break;
				case 'D':
					tmpstr = bkpInt2Str(instance->_tm->tm_mday);
					if (!shrink && instance->_tm->tm_mday < 10)
						tmpstr = (char16_t)'0' + tmpstr;
					break;
				case 'h':
					tmpstr = bkpInt2Str(instance->_tm->tm_hour);
					if (!shrink && instance->_tm->tm_hour < 10)
						tmpstr = (char16_t)'0' + tmpstr;
					break;
				case 'm':
					tmpstr = bkpInt2Str(instance->_tm->tm_min);
					if (!shrink && instance->_tm->tm_min < 10)
						tmpstr = (char16_t)'0' + tmpstr;
					break;
				case 's':
					tmpstr = bkpInt2Str(instance->_tm->tm_sec);
					if (!shrink && instance->_tm->tm_sec < 10)
						tmpstr = (char16_t)'0' + tmpstr;
					break;
				}
				shrink = false;
				res += tmpstr;
			}
		}
		return res;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Math
#endif

	REG_GET_BEGIN(Math)
	{
		GET_INFO(PI),
			GET_INFO(E),
	}
	REG_GET_END;

	REG_FUNC_BEGIN(Math)
	{
		FUNC_INFO(sgn),
		FUNC_INFO(pow),
		FUNC_INFO(abs),
		FUNC_INFO(sqrt),
		FUNC_INFO(lg),
		FUNC_INFO(ln),
		FUNC_INFO(log),
		FUNC_INFO(floor),
		FUNC_INFO(ceil),
		FUNC_INFO(round),
		FUNC_INFO(clamp),
		FUNC_INFO(sin),
		FUNC_INFO(cos),
		FUNC_INFO(tan),
		FUNC_INFO(asin),
		FUNC_INFO(acos),
		FUNC_INFO(atan),
		FUNC_INFO(random),
		FUNC_INFO(min),
		FUNC_INFO(max),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
		REG_GET(Math);
		REG_FUNC(Math);
	}

	NATIVECLASS_GET(PI)
	{
		return M_PI;
	}

	NATIVECLASS_GET(E)
	{
		return M_E;
	}

	NATIVECLASS_FUNC(sgn)
	{
		MINIMUMPARAMNUM(1);
		double a = PARAM(0);
		if (isZero(a))
			return 0;
		if (a > 0)
			return 1;
		return -1;
	}

	NATIVECLASS_FUNC(abs)
	{
		MINIMUMPARAMNUM(1);
		return fabs(PARAM(0).asNumber());
	}

	NATIVECLASS_FUNC(pow)
	{
		MINIMUMPARAMNUM(2);
		return powf(PARAM(0).asNumber(), PARAM(1).asNumber());
	}

	NATIVECLASS_FUNC(sqrt)
	{
		MINIMUMPARAMNUM(1);
		return sqrt(PARAM(0).asNumber());
	}

	NATIVECLASS_FUNC(lg)
	{
		MINIMUMPARAMNUM(1);
		return log10(PARAM(0).asNumber());
	}

	NATIVECLASS_FUNC(ln)
	{
		MINIMUMPARAMNUM(1);
		return log(PARAM(0).asNumber());
	}

	//log(base, num)
	NATIVECLASS_FUNC(log)
	{
		MINIMUMPARAMNUM(2);
		return log(PARAM(1).asNumber()) / log(PARAM(0).asNumber());
	}

	NATIVECLASS_FUNC(floor)
	{
		MINIMUMPARAMNUM(1);
		return floor(PARAM(0).asNumber());
	}

	NATIVECLASS_FUNC(ceil)
	{
		MINIMUMPARAMNUM(1);
		return ceil(PARAM(0).asNumber());
	}

	NATIVECLASS_FUNC(round)
	{
		MINIMUMPARAMNUM(1);
		return PARAM(0).roundAsInteger();
	}

	NATIVECLASS_FUNC(clamp)
	{
		MINIMUMPARAMNUM(3);
		auto x = PARAM(0).asNumber();
		auto min = PARAM(1).asNumber();
		auto max = PARAM(2).asNumber();
		return x < min ? min : (x > max ? max : x);
	}

	NATIVECLASS_FUNC(sin)
	{
		MINIMUMPARAMNUM(1);
		return sin(PARAM(0).asNumber());
	}

	NATIVECLASS_FUNC(cos)
	{
		MINIMUMPARAMNUM(1);
		return cos(PARAM(0).asNumber());
	}

	NATIVECLASS_FUNC(tan)
	{
		MINIMUMPARAMNUM(1);
		return tan(PARAM(0).asNumber());
	}

	NATIVECLASS_FUNC(asin)
	{
		MINIMUMPARAMNUM(1);
		return asin(PARAM(0).asNumber());
	}

	NATIVECLASS_FUNC(acos)
	{
		MINIMUMPARAMNUM(1);
		return acos(PARAM(0).asNumber());
	}

	NATIVECLASS_FUNC(atan)
	{
		MINIMUMPARAMNUM(1);
		return atan(PARAM(0).asNumber());
	}

	//return random number of 0-1
	NATIVECLASS_FUNC(random)
	{
		static std::uniform_real_distribution<double> distribution(0.0, 1.0);
		static std::mt19937 engine;
		return distribution(engine);
	}

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

	NATIVECLASS_FUNC(min)
	{
		MINIMUMPARAMNUM(2);
		return std::min(PARAM(0).asNumber(), PARAM(1).asNumber());
	}

	NATIVECLASS_FUNC(max)
	{
		MINIMUMPARAMNUM(2);
		return std::max(PARAM(0).asNumber(), PARAM(1).asNumber());
	}

#ifdef HAS_REGEX
#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Regex
#endif

	REG_FUNC_BEGIN(Regex)
	{
		FUNC_INFO(search),
		FUNC_INFO(matchAll),
		FUNC_INFO(getSubMatch),
		FUNC_INFO(replaceFirst),
		FUNC_INFO(replaceAll),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_FUNC(Regex);
	}

	//Regex(reg, ignorecase=false)
	NATIVECLASS_CREATENEW()
	{
		MINIMUMPARAMNUM(1);
		return new Regex((wstring)PARAM(0), ((bool)PARAM(1)) ? std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::icase : std::regex_constants::ECMAScript | std::regex_constants::optimize);
	}

	NATIVECLASS_FUNC(search)
	{
		GETINSTANCE();
		MINIMUMPARAMNUM(1);
		std::wsmatch m;
		wstring tmp = (wstring)PARAM(0);
		auto arr = new Bagel_Array();
		if (!std::regex_search(tmp, m, instance->reg))
			return arr;
		while (!m.empty())
		{
			arr->pushMember(m[0].str());
			tmp = m.suffix();
			if (tmp.empty())
				return arr;
			if (!std::regex_search(tmp, m, instance->reg))
				return arr;
		}
		return arr;
	}

	NATIVECLASS_FUNC(matchAll)
	{
		GETINSTANCE();
		MINIMUMPARAMNUM(1);
		return std::regex_match((wstring)PARAM(0), instance->reg);
	}

	NATIVECLASS_FUNC(getSubMatch)
	{
		GETINSTANCE();
		MINIMUMPARAMNUM(1);
		std::wsmatch m;
		wstring src = PARAM(0);
		auto arr = new Bagel_Array();
		if (!std::regex_search(src, m, instance->reg))
			return arr;
		for (auto &&i : m)
		{
			arr->pushMember(i.str());
		}
		return arr;
	}

	static void replace(wstring &raw, std::wsmatch &m)
	{
		for (int i = 0; i < (int)raw.size(); i++)
		{
			if (raw[i] == '\\')
			{
				if (i + 1 < (int)raw.size())
				{
					if (raw[i + 1] == '\\')
						raw.erase(i);
					else
					{
						int rawi = i;
						int num = 0;
						while (i + 1 < (int)raw.size() && iswdigit(raw[i + 1]))
						{
							num *= 10;
							num += raw[i + 1] - L'0';
							i++;
						}
						raw.erase(rawi, i - rawi + 1);
						if (num < (int)m.size())
						{
							raw.insert(rawi, m[num].str());
							i = rawi + m[num].length() - 1;
						}
					}
				}
			}
		}
	}

	NATIVECLASS_FUNC(replaceFirst)
	{
		GETINSTANCE();
		MINIMUMPARAMNUM(2);
		std::wsmatch m;
		wstring src = PARAM(0);
		std::regex_search(src, m, instance->reg);
		if (m.size() == 0)
			return PARAM(0);
		wstring rpl = PARAM(1);
		replace(rpl, m);
		return m.prefix().str() + rpl + m.suffix().str();
	}

	NATIVECLASS_FUNC(replaceAll)
	{
		GETINSTANCE();
		MINIMUMPARAMNUM(2);
		std::wsmatch m;
		wstring tmp = (wstring)PARAM(0);
		std::regex_search(tmp, m, instance->reg);
		if (m.size() == 0)
			return PARAM(0);
		wstring res;
		while (m.suffix().length() > 0)
		{
			res += m.prefix().str();
			wstring backup = PARAM(1);
			replace(backup, m);
			res += backup;
			tmp = m.suffix();
			if (!std::regex_search(tmp, m, instance->reg))
				break;
		}
		res += m.prefix().str();
		wstring backup = PARAM(1);
		replace(backup, m);
		res += backup;
		return res;
	}
#endif

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Flags
#endif

	REG_FUNC_BEGIN(Flags)
	{
		FUNC_INFO(contains),
			FUNC_INFO(split),
			FUNC_INFO(mask),
			FUNC_INFO(merge),
	};
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
		REG_FUNC(Flags);
	}

	NATIVECLASS_CREATENEW()
	{
		return NULL;
	}

	NATIVECLASS_FUNC(contains)
	{
		MINIMUMPARAMNUM(2);
		uint32_t container = PARAM(0);
		uint32_t flag = PARAM(1);
		return (bool)((container & flag) == flag);
	}

	NATIVECLASS_FUNC(split)
	{
		MINIMUMPARAMNUM(1);
		uint32_t container = PARAM(0);
		Bagel_Array *arr = new Bagel_Array();
		uint32_t mask = 1;
		int idx = 0;
		while (idx < 32)
		{
			if (container & mask)
			{
				arr->pushMember(mask);
			}
			mask <<= 1;
			idx++;
		}
		return arr;
	}

	NATIVECLASS_FUNC(mask)
	{
		MINIMUMPARAMNUM(2);
		uint32_t container = PARAM(0);
		uint32_t mask = PARAM(1);
		return container & mask;
	}

	NATIVECLASS_FUNC(merge)
	{
		MINIMUMPARAMNUM(1);
		uint32_t container = PARAM(0);
		int count = PARAMCOUNT();
		for (int i = 1; i < count; i++)
		{
			container |= (uint32_t)PARAM(i).asInteger();
		}
		return container;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Coroutine
#endif

	REG_FUNC_BEGIN(Coroutine)
	{
		FUNC_INFO(resume),
		FUNC_INFO(restart),
		FUNC_INFO(stop),
		FUNC_INFO(finalize),
	};
	REG_FUNC_END;

	REG_GET_BEGIN(Coroutine)
	{
		GET_INFO(status),
	};
	REG_GET_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		REG_FUNC(Coroutine);
		REG_GET(Coroutine);
	}

	NATIVECLASS_CREATENEW()
	{
		MINIMUMPARAMNUM(1);
		if (PARAM(0).getType() != VAR_FUNC)
		{
			throw Bagel_Except(W("参数需要为一个函数"));
		}
		auto func = PARAM(0).forceAsFunc();
		if (func->isNativeFunction())
		{
			throw Bagel_Except(W("参数不能为native function"));
		}
		auto c = Bagel_VM::getInstance()->createNULLThreadHandle();
		func->VMRun((Bagel_Var*)paramarray + 1, paramcount - 1, c);
		c->status = Bagel_ThreadContext::THREAD_SLEEP;
		return new Coroutine(c, func);
	}

	void Coroutine::markChildren()
	{
		_GC.GC_Markself(func);
	}

	NATIVECLASS_FUNC(resume)
	{
		GETINSTANCE();
		if (!instance->ctx || instance->ctx->status == Bagel_ThreadContext::THREAD_FINISH)
		{
			throw Bagel_Except(W("协程已经运行结束"));
		}
		ctx->status = Bagel_ThreadContext::THREAD_IDLE;
		Bagel_Var res;
		Bagel_Var p;
		if (PARAMEXIST(0))
			p = PARAM(0);
		if (instance->ctx->runningclo->retpos)
			instance->ctx->runningclo->retpos->forceSet(p);
		try
		{
			res = instance->ctx->Run();
		}
		catch (Bagel_Except &e)
		{
			//在此抛出异常意味着该协程已结束
			instance->ctx = NULL;
			ctx->status = Bagel_ThreadContext::THREAD_RUNNING;
			throw e;
		}
		if (instance->ctx->status == Bagel_ThreadContext::THREAD_FINISH)
		{
			instance->ctx = NULL;
		}
		ctx->status = Bagel_ThreadContext::THREAD_RUNNING;
		return res;
	}

	NATIVECLASS_FUNC(restart)
	{
		GETINSTANCE();
		Bagel_VM::getInstance()->stop((Bagel_ThreadHandle)instance->ctx);
		auto c = Bagel_VM::getInstance()->createNULLThreadHandle();
		instance->func->VMRun((Bagel_Var*)paramarray, paramcount, c);
		c->status = Bagel_ThreadContext::THREAD_SLEEP;
		instance->ctx = c;
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(stop)
	{
		GETINSTANCE();
		Bagel_VM::getInstance()->stop((Bagel_ThreadHandle)instance->ctx);
		instance->ctx = NULL;
		RETURNDEFAULT;
	}

	//same as stop
	NATIVECLASS_FUNC(finalize)
	{
		GETINSTANCE();
		Bagel_VM::getInstance()->stop((Bagel_ThreadHandle)instance->ctx);
		instance->ctx = NULL;
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(status)
	{
		GETINSTANCE();
		if (!instance->ctx)
			return 0;
		return instance->ctx->status;
	}


	REG_FUNC_BEGIN(Global)
	{

		/**
		\addtogroup global
		这里列出了全局函数
		@{
		*/

		/**
		*  \fn void set(exp, value)
		*  \param exp 传入的变量字符串
		*  \param value 要设置的值
		*  \brief  将exp所代表的变量的值设为value。
		*  \body 比如exp="tf.a", value=1。那么执行后tf.a的值为1。
		*/
		{
			QUICKFUNC(set)
		},
		/**
		*  \fn void eval()
		*  \param str 传入的字符串
		*  \return 执行的结果
		*  \brief  返回str的执行结果。
		*/
		{
			QUICKFUNC(eval)
		},
			/**
			*  @param string str
			*  @return number
			*  @brief  执行str，然后返回执行用的时间（单位ms）。
			*/
		{ QUICKFUNC(time) },
				/**
				*  @return number
				*  @brief  返回当前的时间戳（单位ms）。
				*/
		{ QUICKFUNC(clock) },
				/**
				*  @prototype (anytype value1, anytype value2, ...)
				*  @brief  逐个打印参数，逗号分隔。
				*/
		{ QUICKFUNC(log) },
				/**
				*  @prototype anytype var
				*  @brief  获得一个变量的类型名
				*  @return string name 类型名
				*/
		{ QUICKFUNC(getClassName) },
				/**
				*  @prototype number num1
				*  @prototype number num1, number num2
				*  @brief  生成一个数组。
				*  @body 如果是第一种形式，那么生成的是[0, 1, ..., num1-1]
				*  如果是第二种形式，那么：
				*  {@t}如果num2>num1，那么生成的是[num1, num1+1, ..., num2-1]
				*  {@t}如果num2<num1，那么生成的是[num1, num1-1, ..., num2+1]
				*  {@t}如果num2=num1，那么生成的是空数组
				*/
		{ QUICKFUNC(range) },
				/**
				*  @prototype number num1
				*  @prototype number num1, number num2
				*  @brief  生成一个随机整数。
				*  @body 如果是第一种形式，那么相当于random[0, num1)。
				*  如果是第二种形式，那么：
				*  {@t}如果num2>num1，那么生成的是num1到num2-1之间的一个整数
				*  {@t}如果num2<num1，那么生成的是num2到num1-1之间的一个整数
				*  {@t}如果num2=num1，那么返回num1
				*/
		{ QUICKFUNC(random) },
				/**
				*  @param number num
				*  @return string
				*  @brief  相当于(string)(int)num，先对数字取整，然后生成相应的字符串。
				*/
		{ QUICKFUNC(itoa) },
				/**
				*  @param number num
				*  @return string
				*  @brief  同itoa，只是生成的字符串是全角的'０', '１'等字符。
				*/
		{ QUICKFUNC(itoa2) },
				/**
				*  @param string str
				*  @return integer
				*  @brief  返回str[0]对应的unicode编码。没有则返回0。
				*/
		{ QUICKFUNC(char) },
				/**
				*  @param anytype value
				*  @return integer
				*  @brief  返回value对应的32位hash值。
				*/
		{ QUICKFUNC(hash) },
				/**
				*  @param anytype value
				*  @return integer
				*  @brief  返回value对应的hash值（short版本）。
				*/
		{ QUICKFUNC(hash16) },
			#ifdef ENABLE_FILE
				/**
				*	@param string filename 文件名
				*	@return string
				*	@brief 读取一个文件，返回里面的原始内容（字符串）
				*	@example 以下将演示一个从内容为
				*		{@box abc}
				*		的1.txt文件载入为数组的示例：
				*	@example_code [].loadFile("1.txt")
				*	@example_result "abc"
				*/
			{ QUICKFUNC(loadFile) },
				/**
				*	@param string filename 文件名
				*	@param string str 将要保存的字符串
				*	@param bool append=false 是否追加在文件末尾。若为否，则覆盖整个文件。
				*	@return bool 是否成功
				*	@brief 将一个字符串写入文件。
				*/
			{ QUICKFUNC(saveFile) },
				/**
				*	@param string filename 文件名
				*	@param anytype var 将要保存的变量
				*	@param bool append=false 是否追加在文件末尾。若为否，则覆盖整个文件。
				*	@return bool 是否成功
				*	@brief 将一个变量以序列化形式写入文件，与{@link saveFile}不同的是，这种方式可以保留变量里成员间的相互引用关系。
				*/
			{ QUICKFUNC(saveVariable) },
				/**
				*	@param string filename 文件名
				*	@param string str 将要保存的字符串
				*	@return bool 是否成功
				*	@brief 将一个字符串添加到文件末尾。等同于{@link saveFile}(filename, str, -1);
				*/
			{ QUICKFUNC(appendFile) },
				/**
				*	@param string filename 文件名
				*	@param bool krmode=false 若为真，表示双引号字符串可以像单引号字符串一样使用转义（此时不使用原有的""表示"的功能）
				*	@param bool rawstr=false 若为真，表示字符串可以跨行
				*	@return var 反序列化的结果
				*	@brief 对文件读取并且解析（反序列化），将结果返回。（使用evalFile或importFile执行过的文件依然会被重新执行）
				*	@body 等同于{@link eval}({@link loadFile}(filename, krmode, rawstr));
				*/
			{ QUICKFUNC(evalFile) },
				/**
				*	@param string filename 文件名
				*	@param bool krmode=false 若为真，表示双引号字符串可以像单引号字符串一样使用转义（此时不使用原有的""表示"的功能）
				*	@param bool rawstr=false 若为真，表示字符串可以跨行
				*	@brief 读取并执行一个文件，使用evalFile或importFile执行过的文件将不会再被执行。
				*/
			{ QUICKFUNC(importFile) },
			#endif
				/**
				*  @param anytype value
				*  @param bool format 是否格式化
				*  @return string
				*  @brief  将value转变成字符串，format表示是否加上缩进格式。
				*  @example 以下将演示一个数组[1,2,3]的toString结果
				*  @example_code toString([1,2,3])
				*  @example_result [1,2,3]
				*  @example_code toString([1,2,3], true)
				*  @example_result [
				*		{@t}1,
				*		{@t}2,
				*		{@t}3
				*	]
				*/
			{ QUICKFUNC(toString) },
				/**
				*  @prototype string classname, sictionary dic(, anytype nativedata)
				*  @return class instance
				*  @brief  内部用，用于从保存的类对象中还原。
				*/
			{ QUICKFUNC(loadClass) },

				/*! @} */
	}
	REG_FUNC_END;

	REG_FUNC_BEGIN(Undefined)
	{
		/**
		*	@class undefined（内部保留类）
		*   @param anytype value
		*   @return bool
		*   @brief  比较本对象与value值是否相等。
		*/
		{
			QUICKFUNC(equals)
		},
	}
	REG_FUNC_END;

	REG_GET_BEGIN(String)
	{
		/**
		*	@class string（内部保留类）
		*   @return integer
		*   @brief  返回字符串的长度。
		*/
		{
			QUICKGETTER(length)
		},
			/**
			*	@class string（内部保留类）
			*   @return integer
			*   @brief  同{@link length}，返回字符串的长度。
			*/
		{ W("size"), &ParserUtils::nativeGet_length }
	}
	REG_GET_END;

	REG_FUNC_BEGIN(String)
	{
		/**
		*	@class string（内部保留类）
		*	@prototype integer start
		*	@prototype integer start, integer len
		*   @return string
		*   @brief 取子串
		*   @body  取从start开始长度为len的子串，如果省略len，则取到原字符串末尾。如果start位置不合法，则返回空串。
		*/
		{
			QUICKFUNC(substring)
		},
			/**
			*	@class string（内部保留类）
			*	@prototype integer start
			*	@prototype integer start, integer len
			*   @return string
			*   @brief  同{@link substring}，取从start开始长度为len的子串，如果省略len，则取到原字符串末尾。如果start位置不合法，则返回空串。
			*/
		{ W("substr"), &ParserUtils::nativeFunc_substring },
				/**
				*	@class string（内部保留类）
				*	@prototype (anytype arg1, anytype arg2, ...)
				*   @return string
				*   @brief  格式化字符串，以本字符串为格式，根据传入的参数生成一个新的字符串。
				*   @example 以下将演示用"%04d"来格式化233的结果
				*   @example_code "%04d".sprintf(233)
				*   @example_result "0233"
				*/
		{ QUICKFUNC(sprintf) },
				/**
				*	@class string（内部保留类）
				*	@prototype string str(, ignorenull = false)
				*	@prototype Regex regexp(, ignorenull = false)
				*   @return array
				*   @brief  第一个参数为字符串或正则表达式类。用第一个参数分割原字符串得到一串数组，ignorenull表示是否忽略结果数组中的空串，如果为真，结果数组中的空串将被抹去
				*   @example 以下将演示用","来分割"1,2,,3"的结果
				*   @example_code "1,2,,3".split(",")
				*   @example_result ["1", "2", "", "3"]
				*   @example_code "1,2,,3".split(",", true)
				*   @example_result ["1", "2", "3"]
				*/
		{ QUICKFUNC(split) },
				/**
				*	@class string（内部保留类）
				*	@prototype string str(, ignorenull = false)
				*   @return array
				*   @brief  第一个参数为字符串。用字符串中任意字符分割原字符串得到一串数组，ignorenull表示是否忽略结果数组中的空串，如果为真，结果数组中的空串将被抹去
				*   @example 以下将演示用"()"来分割"(1,2,3)(4,5,6)"的结果
				*   @example_code "(1,2,3)(4,5,6)".split("()", true)
				*   @example_result ["1,2,3", "4,5,6"]
				*   @example_code "(1,2,3)(4,5,6)".split("(),", true)
				*   @example_result ["1", "2", "3", "4", "5", "6"]
				*/
		{ QUICKFUNC(splitany) },
				/**
				*	@class string（内部保留类）
				*	@prototype string str(, startpos = 0)
				*	@prototype Regex regexp(, startpos = 0)
				*   @return integer
				*   @brief  第一个参数为字符串或正则表达式类。从startpos位置开始，寻找第一次出现参数1的位置。
				*   @example 以下将演示在"1,2,,3"的查找","的结果
				*   @example_code "1,2,,3".indexOf(",")
				*   @example_result 1
				*   @example_code "1,2,,3".indexOf(",", 2)
				*   @example_result 3
				*/
		{ QUICKFUNC(indexOf) },
				/**
				*	@class string（内部保留类）
				*	@prototype string str(, endpos = 0)
				*	@prototype Regex regexp(, endpos = 0)
				*   @return integer
				*   @brief  第一个参数为字符串或正则表达式类。从endpos位置开始，向前寻找第一次出现参数1的位置。
				*   @example 以下将演示在"1,2,,3"的查找","的结果
				*   @example_code "1,2,,3".lastIndexOf(",")
				*   @example_result 4
				*   @example_code "1,2,,3".lastIndexOf(",", 2)
				*   @example_result 1
				*/
		{ QUICKFUNC(lastIndexOf) },
		{ QUICKFUNC(trimLeft) },
		{ QUICKFUNC(trimRight) },
		{ QUICKFUNC(trim) },
				/**
				*	@class string（内部保留类）
				*	@prototype string str, string str2
				*	@prototype Regex regexp, string str2
				*   @return string
				*   @brief  参数为字符串或正则表达式类。将原字符串中str1全部替换为str2。
				*   @example 以下将演示将"aaa"中"a"替换为"ab"的结果
				*   @example_code "aaa".replace("a", "ab")
				*   @example_result "ababab"
				*/
		{ QUICKFUNC(replace) },
				/**
				*  @param anytype value
				*  @return bool
				*  @brief  比较本对象与value值是否相等。
				*/
		{ QUICKFUNC(equals) },
				/**
				*	@class string（内部保留类）
				*	@prototype string str
				*	@prototype Regex regexp
				*   @return bool
				*   @brief  参数为字符串或正则表达式类。返回原字符串是否已参数给定的字符串开头。
				*/
		{ QUICKFUNC(beginWith) },
				/**
				*	@class string（内部保留类）
				*	@prototype string str
				*	@prototype Regex regexp
				*   @return bool
				*   @brief  参数为字符串或正则表达式类。返回原字符串是否已参数给定的字符串结尾。
				*/
		{ QUICKFUNC(endWith) },
				//被删除，老老实实=""就好了，"233".clear()会带来隐患。
				//{ QUICKFUNC(clear) },

				/**
				*	@class string（内部保留类）
				*   @return string
				*   @brief  返回新字符串，原字符串中所有大写字母被转化为小写。
				*/
		{ QUICKFUNC(toLowerCase) },
				/**
				*	@class string（内部保留类）
				*   @return string
				*   @brief  返回新字符串，原字符串中所有小写字母被转化为大写。
				*/
		{ QUICKFUNC(toUpperCase) },
	}
	REG_FUNC_END;

	REG_SET_BEGIN(Array)
	{
		/**
		*	@class array（内部保留类）
		*   @return integer
		*   @brief  返回数组的长度。
		*/
		{
			QUICKSETTER(length)
		},
			/**
			*	@class array（内部保留类）
			*   @return integer
			*   @brief  同length，返回数组的长度。
			*/
		{ W("size"), &ParserUtils::nativeSet_length }
	}
	REG_SET_END;

	REG_FUNC_BEGIN(Array)
	{
		/**
		*	@class array（内部保留类）
		*	@prototype (anytype arg1, anytype arg2, ...)
		*   @brief  向数组尾部添加参数。
		*/
		{
			QUICKFUNC(add)
		},
			/**
			*	@class array（内部保留类）
			*	@prototype (anytype arg1, anytype arg2, ...)
			*   @brief  从数组中删除与参数相同的元素，使用==比较。
			*/
		{ QUICKFUNC(remove) },
				/**
				*	@class array（内部保留类）
				*	@prototype (integer index1, integer index2, ...)
				*   @brief  从数组中删除参数对应下标的元素，负数表示从后往前数，注意是从前往后逐个删除的，所以erase(1,1)会删除原数组下标为1,2位置的元素。越界会抛出异常。
				*/
		{ QUICKFUNC(erase) },
				/**
				*	@class array（内部保留类）
				*	@prototype integer index, anytype value
				*   @brief  在数组index位置插入一个新的value。
				*   @example 以下将演示在[1,2,3]的2位置插入4的结果
				*   @example_code var a=[1,2,3];
				*	a.insert(2,4);
				*	log(a);
				*   @example_result [1,2,4,3]
				*/
		{ QUICKFUNC(insert) },
				/**
				*	@class array（内部保留类）
				*	@param anytype value
				*	@return integer
				*   @brief  返回数组中第一个等于value的元素的位置，使用==比较，找不到返回-1。
				*/
		{ QUICKFUNC(find) },
				/**
				*	@class array（内部保留类）
				*	@param string str="" 分隔符
				*	@return string
				*   @brief  将数组中所有元素转成字符串，然后用str连接。
				*   @example 以下将演示用"-"连接[1,2,3]的结果
				*   @example_code [1,2,3].join("-")
				*   @example_result "1-2-3"
				*/
		{ QUICKFUNC(join) },
				/**
				*	@class array（内部保留类）
				*	@prototype (array arr1, array arr2, ...)
				*	@return string
				*   @brief  会将此数组与参数中所有数组的元素连接起来形成的大数组，原数组不受影响。
				*   @example 以下将演示[1,2,3]和[3,2,1]连接的结果
				*   @example_code [1,2,3].concat([3,2,1])
				*   @example_result [1,2,3,3,2,1]
				*/
		{ QUICKFUNC(concat) },
				/**
				*	@class array（内部保留类）
				*	@prototype function func
				*   @prototype string modestr="+"
				*	@param function func 排序函数
				*   @param string modestr="+" 排序模式字符串
				*   @brief  根据指定的函数或模式排序。会改动原数组，没有返回值。
				*	@body 如果指定的是函数，将把此函数（func(a,b)）当做<的比较函数，从小到大排序。
				*	如果指定的是模式字符串，则：
				*		{@t}"+"：以默认的比较运算符从小到大排序
				*		{@t}"-"：以默认的比较运算符从大到小排序
				*		{@t}"0"：按数值从小到大排序
				*		{@t}"9"：按数值从大到小排序
				*		{@t}"a"：按字符串从小到大排序
				*		{@t}"z"：按字符串从大到小排序
				*   @example 以下将演示[1,2,3]按"-"排序的结果
				*   @example_code var a=[1,2,3];
				*	a.sort("9");
				*	log(a);
				*   @example_result [3,2,1]
				*/
		{ QUICKFUNC(sort) },
				/**
				*	@class array（内部保留类）
				*	@return array
				*   @brief  复制出一份一样的新数组，更改新数组不会影响原数组。
				*/
		{ QUICKFUNC(clone) },
				/**
				*	@class array（内部保留类）
				*   @param anytype value
				*   @return bool
				*   @brief  比较本对象与value值是否相等。如果value是数组，会比较每个元素是否相等，长度较短的数组会补上void去做比较
				*   @example 以下将演示[1,2,3]和[1,2,3,0]的比较
				*   @example_code [1,2,3].equals([1,2,3,0])
				*   @example_result true
				*/
		{ QUICKFUNC(equals) },
				/**
				*	@class array（内部保留类）
				*   @brief  清空本数组。
				*/
		{ QUICKFUNC(clear) },
				/**
				*	@class array（内部保留类）
				*	@return anytype
				*   @brief  随机返回数组中一个元素。
				*/
		{ W("random"), &ParserUtils::nativeFunc_array_random },
			/**
			*	@class array（内部保留类）
			*	@param integer n=1
			*	@return array
			*   @brief  取出数组中n个不重复的元素，返回数组，原数组顺序不变。
			*/
		{ QUICKFUNC(randomPick) },
			/**
			*	@class array（内部保留类）
			*	@param integer n=0
			*   @brief  数组洗牌，打乱数组中前n个元素，n<=0时打乱全数组。
			*/
		{ QUICKFUNC(shuffle) },
			/**
			*	@class array（内部保留类）
			*	@param integer index
			*	@return anytype
			*   @brief  取出数组中一个元素，即取值并且清除（erase）。
			*/
		{ QUICKFUNC(take) },
			/**
			*	@class array（内部保留类）
			*	@return anytype
			*   @brief  取出数组中一个随机元素，即取值并且清除（erase）。
			*/
		{ QUICKFUNC(takeRandom) },
		#ifdef ENABLE_FILE
				/**
				*	@class array（内部保留类）
				*	@param string filename 文件名
				*	@param bool append=false 是否追加在文件末尾。若为否，则覆盖整个文件。
				*	@brief 将字符串数组按照换行合并，并保存到文件
				*	@example 以下将演示一个从内容为["a","b","c"]的数组写入文件的示例：
				*	@example_code [].save("1.txt")
				*/
			{ QUICKFUNC(save) },
				/**
				*	@class array（内部保留类）
				*	@static
				*	@param string filename 文件名
				*	@return array
				*	@brief 读取文件并按照换行切分为数组。返回一个新数组，不改变当前数组内容。
				*	@example 以下将演示一个从内容为
				*		{@box}a
				*		{@box}b
				*		{@box}c
				*		的1.txt文件载入为数组的示例：
				*	@example_code [].load("1.txt")
				*	@example_result ["a","b","c"]
				*/
			{ QUICKFUNC(load) },
				/**
				*	@class array（内部保留类）
				*	@param string filename 文件名
				*	@param string mode="" 模式。若首字母为'o'则字符串的其余部分代表一个偏移，文件将从该偏移处进行覆盖保存，负值将从文件末尾进行计算。若首字母为'z'则与'o'类似，只不过将启用压缩。
				*	@brief 将数组进行序列化，并保存到文件。
				*	@body 文件的内容可以直接使用{@link evalFile}进行反序列化，读取至变量。
				*	@example 以下将演示一个从内容为[1,2,3]的数组写入文件的示例：
				*	@example_code [1,2,3].saveStruct("1.txt")
				*/
			{ QUICKFUNC(saveStruct) },
			#endif
	}
	REG_FUNC_END;

	REG_FUNC_BEGIN(Dictionary)
	{
		/**
		*	@class dictionary（内部保留类）
		*	@prototype (anytype arg1, anytype arg2, ...)
		*   @brief  从字典中删除值与参数相同的元素，使用==比较。
		*/
		{
			QUICKFUNC(remove)
		},
			/**
			*	@class dictionary（内部保留类）
			*	@prototype (anytype arg1, anytype arg2, ...)
			*   @brief  从字典中删除键与参数相同的元素，使用==比较。
			*/
		{ QUICKFUNC(erase) },
		{ QUICKFUNC(member) },
				/**
				*	@class dictionary（内部保留类）
				*	@return dictionary
				*   @brief  复制出一份一样的新字典，更改新字典不会影响原字典。
				*/
		{ QUICKFUNC(clone) },
				/**
				*	@class dictionary（内部保留类）
				*	@param string key
				*	@return bool
				*   @brief  返回字典中是否存在键名为key的键。
				*/
		{ QUICKFUNC(find) },
				/**
				*	@class dictionary（内部保留类）
				*	@return array
				*   @brief  返回[键1, 值1, 键2, 值2, ...]的数组。
				*/
		{ QUICKFUNC(toArray) },
				/**
				*	@class dictionary（内部保留类）
				*   @param anytype value
				*   @return bool
				*   @brief  比较本对象与value值是否相等。如果value是字典，会比较每对键-值是否相等，不存在的键会认为值是void去做比较。
				*   @example 以下将演示%[key1:1,key2:2]和%[key1:1,key2:2,key3:3]的比较
				*   @example_code %[key1:1,key2:2].equals(%[key1:1,key2:2,key3:3])
				*   @example_result false
				*/
		{ QUICKFUNC(equals) },
				/**
				*	@class dictionary（内部保留类）
				*   @param dictionary dic
				*   @brief  与新字典合并，重复的键将被覆盖。
				*/
		{ QUICKFUNC(update) },
				/**
				*	@class dictionary（内部保留类）
				*   @param dictionary dic
				*   @brief  从本字典中去除参数字典中包含的键。
				*/
		{ QUICKFUNC(except) },
				/**
				*	@class dictionary（内部保留类）
				*   @return array
				*   @brief  返回所有键组成的数组。
				*/
		{ QUICKFUNC(getKeyArray) },
				/**
				*	@class dictionary（内部保留类）
				*   @return array
				*   @brief  返回所有值组成的数组。
				*/
		{ QUICKFUNC(getValueArray) },
				/**
				*	@class dictionary（内部保留类）
				*	@prototype function func
				*   @prototype string modestr="+"
				*	@param function func 排序函数
				*   @param string modestr="+" 排序模式字符串
				*	@return array
				*   @brief  按值排序所有的键，比较的方式根据指定的函数或模式决定。
				*	@body 如果指定的是函数，将把此函数（func(a,b)）当做<的比较函数，从小到大排序。
				*	如果指定的是模式字符串，则：
				*		{@t}"+"：以默认的比较运算符从小到大排序
				*		{@t}"-"：以默认的比较运算符从大到小排序
				*		{@t}"0"：按数值从小到大排序
				*		{@t}"9"：按数值从大到小排序
				*		{@t}"a"：按字符串从小到大排序
				*		{@t}"z"：按字符串从大到小排序
				*   @example 以下将演示%[key1:1,key2:2,key3:3]按"-"排序的结果
				*   @example_code %[key1:1,key2:2,key3:3].sort("9");
				*   @example_result ["key3", "key2", "key1"]
				*/
		{ QUICKFUNC(sortKeyByValue) },
				/**
				*	@class array（内部保留类）
				*   @brief  清空本字典。
				*/
		{ QUICKFUNC(clear) },
			#ifdef ENABLE_FILE
				/**
				*	@class dictionary（内部保留类）
				*	@param string filename 文件名
				*	@param string mode="" 模式。若首字母为'o'则字符串的其余部分代表一个偏移，文件将从该偏移处进行覆盖保存，负值将从文件末尾进行计算。若首字母为'z'则与'o'类似，只不过将启用压缩。
				*	@brief 将数组进行序列化，并保存到文件。
				*	@body 文件的内容可以直接使用{@link evalFile}进行反序列化，读取至变量。
				*	@example 以下将演示一个从内容为%[key:1]的字典写入文件的示例：
				*	@example_code %[key:1].saveStruct("1.txt")
				*/
			{ QUICKFUNC(saveStruct) },
			#endif
	}
	REG_FUNC_END;

	void registerExtend(Bagel_VM *p)
	{
		//register inner classes and global functions
		Bagel_Closure *global = Bagel_Closure::global();
		{
			Bagel_Closure *_class = global;
			REG_FUNC(Global);
		}

		//undefined
		Bagel_StringHolder s_void(W("void"));
		Bagel_ClassDef *_class = new Bagel_ClassDef(s_void);
		_class->isFinal = true;
		_globalStructures.typeclass[VAR_NONE] = _class;
		REG_FUNC(Undefined);
		global->forceSetMember(s_void, _class);

		//number
		Bagel_StringHolder s_number(W("number"));
		_class = new Bagel_ClassDef(s_number);
		_class->isFinal = true;
		_globalStructures.typeclass[VAR_NUM] = _class;
		REG_FUNC(Undefined);
		global->forceSetMember(s_number, _class);

		//string
		Bagel_StringHolder s_string(W("string"));
		_class = new Bagel_ClassDef(s_string);
		_class->isFinal = true;
		_globalStructures.typeclass[VAR_STR] = _class;
		REG_GET(String);
		REG_FUNC(String);
		global->forceSetMember(s_string, _class);

		//array
		Bagel_StringHolder s_array(W("array"));
		_class = new Bagel_ClassDef(s_array);
		_class->isFinal = true;
		_globalStructures.typeclass[VAR_ARRAY] = _class;
		_class->innerCreateInstance = &ParserUtils::nativeFunc_array;
		REG_GET(String);  // Yes, the same getter list as String
		REG_SET(Array);
		REG_FUNC(Array);
		global->forceSetMember(s_array, _class);

		Bagel_StringHolder s_dictionary(W("dictionary"));
		_class = new Bagel_ClassDef(s_dictionary);
		_class->isFinal = true;
		_globalStructures.typeclass[VAR_DIC] = _class;
		_class->innerCreateInstance = &ParserUtils::nativeFunc_dictionary;
		REG_GET(String);  // Yes, the same getter list as String
		REG_FUNC(Dictionary);
		global->forceSetMember(s_dictionary, _class);

		Bagel_StringHolder s_function(W("function"));
		_class = new Bagel_ClassDef(s_function);
		_class->isFinal = true;
		_globalStructures.typeclass[VAR_FUNC] = _class;
		REG_FUNC(Undefined);
		global->forceSetMember(s_function, _class);

		//clo取消，clo.xxxx永远从闭包中找，不考虑equals

		//Bagel_StringHolder s_closure(W("closure"));
		//_class = new Bagel_Class(s_closure);
		//_globalStructures.typeclass[VAR_CLO] = _class;
		//REG_FUNC(Undefined);
		//global->forceSetMember(s_closure, _class);

		//othe native class
		p->registerClass(QUICKCLASS(Date));
		p->registerClass(QUICKCLASS(Math));
	#ifdef HAS_REGEX
		p->registerClass(QUICKCLASS(Regex));
	#endif
		p->registerClass(QUICKCLASS(Flags));
		p->registerClass(QUICKCLASS(Coroutine));
	}
}

