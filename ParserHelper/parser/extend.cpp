#include "extend.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <random>
#include <functional>
#if BKE_CREATOR
#include "bkutf8.h"
#else
#include <bkutf8.h>
#endif

#include "parser.h"

#ifdef WIN32
#include <Windows.h>
#endif

namespace Parser_Util
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

	NATIVE_FUNC(array)
	{
		BKE_VarArray *arr=new BKE_VarArray();
		if(paramarray)
			for(int i=0;i<paramarray->getCount();i++)
				arr->pushMember(PARAM(i));
		return arr;
	};

	NATIVE_FUNC(dictionary)
	{
		BKE_VarDic *dic=new BKE_VarDic();
		for(int i=0;i<paramarray->getCount();i+=2)
			dic->setMember(PARAM(i), PARAM(i+1));
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
		setlocale(LC_ALL, "");
		if (!PARAMEXIST(0))
			RETURNDEFAULT;
		wstring res;
		PARAM(0).save(res);
		wcout << res;
		for (int i = 1; i < paramarray->getCount(); i++)
		{
			res.clear();
			PARAM(i).save(res);
			wcout << L"," << res.c_str();
		}
		wcout << L"\n";
		RETURNDEFAULT;
	}

	NATIVE_FUNC(print)
	{
		setlocale(LC_ALL, "");
		if (!PARAMEXIST(0))
			RETURNDEFAULT;
		wstring res;
		PARAM(0).save(res);
		wcout << res;
		for (int i = 1; i < paramarray->getCount(); i++)
		{
			res = L"";
			if (PARAM(i).getType() == VAR_STR)
				res = (wstring)PARAM(i);
			else
				PARAM(i).save(res);
			wcout << L"," << res.c_str();
		}
		wcout << L"\n";
		RETURNDEFAULT;
	}

	NATIVE_FUNC(eval)
	{
		if (PARAMEXIST(0))
		{
			bool krmode = Parser::getInstance()->krmode;
			bool rawstr = Parser::getInstance()->rawstr;
			Parser::getInstance()->krmode = PARAM(1);
			Parser::getInstance()->rawstr = PARAM(2);
			BKE_Variable res;
			try
			{
				res = Parser::getInstance()->eval(PARAM(0));
			}
			catch (Var_Except &e)
			{
				Parser::getInstance()->krmode = krmode;
				Parser::getInstance()->rawstr = rawstr;
				throw e;
			}
			Parser::getInstance()->krmode = krmode;
			Parser::getInstance()->rawstr = rawstr;
			return res;
		}
		else
			return BKE_Variable();
	}

	//time(expression)
	NATIVE_FUNC(time)
	{
		MINIMUMPARAMNUM(1);
		double start=getutime();
		Parser::getInstance()->eval(PARAM(0));
		return (getutime() - start) / 1000 ;
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
		BKE_VarArray *arr=new BKE_VarArray();
		if(PARAMEXIST(1))
		{
			bkplong a=PARAM(0);
			bkplong b = PARAM(1);
			if (b > a)
			{
				arr->vararray.reserve(b - a);
				for (int i = a; i < b; i++)
					arr->pushMember(i);
			}
			else if (a>b)
			{
				arr->vararray.reserve(a - b);
				for (int i = a; i > b; i--)
					arr->pushMember(i);
			}
			return arr;
		}
		bkplong a = PARAM(0);
		arr->vararray.reserve(a);
		for(int i=0;i<a;i++)
			arr->pushMember(i);
		return arr;
	}

	//a ---- b-1
	NATIVE_FUNC(random)
	{
		MINIMUMPARAMNUM(1);
		bkplong a, b;
		if(!PARAMEXIST(1))
		{
			a=0;
			b=PARAM(0);
		}
		else
		{
			a=PARAM(0);
			b=PARAM(1);
		}
		float per;
		do 
		{
			per = (float)bkpRandomDouble(0., 1.);
		} while (per == 0.f || per == 1.f);
		return floor(a+(b-a)*per);
	}

	////typeof(xxx)
	//NATIVE_FUNC(typeof)
	//{
	//	return PARAM(0).getTypeBKEString();
	//}

	NATIVE_FUNC(itoa)
	{
		MINIMUMPARAMNUM(1);
		bkplong a = PARAM(0);
		return bkpInt2Str(a);
	}

	NATIVE_FUNC(itoa2)
	{
		MINIMUMPARAMNUM(1);
		bkplong a = PARAM(0);
		wstring str=bkpInt2Str(a);
		for(int i=0;i<(int)str.size();i++)
			str[i]+=L'０'-L'0';
		return str;
	}

	NATIVE_FUNC(char)
	{
		if(!PARAMEXIST(0))
			return 0;
		const wstring &ss = PARAM(0).asBKEStr().getConstStr();
		if(ss.empty())
			return 0;
#if WCHAR_MAX==0xFFFFFFFF
		wchar_t s = ss[0];
		return (bkplong)s;
#else
		bkplong a = ss[0];
		if (a >= 0xD800 && a <= 0xDFFF)
		{
			bkplong b = ss[1];
			return ((a - 0xD800) << 10) + (b - 0xDC00) + 0x10000;
		}
		return a;
#endif
	}

	NATIVE_FUNC(hash)
	{
		MINIMUMPARAMNUM(1);
		const BKE_Variable &var=PARAM(0);
		switch(var.getType())
		{
		case VAR_NONE:
			return 0;
		case VAR_NUM:
			return BKE_hash(var.num);
		case VAR_STR:
			return BKE_hash(var.str);
		default:
			return (bkplonglong)var.obj;
		}
	}

	NATIVE_FUNC(hash16)
	{
		return (bkplong)nativeFunc_hash(self, paramarray, _this) & 0xFFFF;
	}

#ifdef ENABLE_FILE

	NATIVE_FUNC(load)
	{
		MINIMUMPARAMNUM(1);
		wstring fname = PARAM(0);
		wstring res;
		bool re = BKE_readFile(res, fname);
		if (!re)
			throw Var_Except(L"打开文件失败");
		auto arr = new BKE_VarArray();
		wchar_t *buf3 = &res[0];
		int i = 0;
		wstring tmp;
		tmp.reserve(256);
		bkplong size = res.size();
		while (i<size)
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
		if (self && self->isVar())
		{
			self->obj->release();
			const_cast<BKE_Variable*>(self)->obj = arr;
		}
		return arr;
	}

	NATIVE_FUNC(save)
	{
		MINIMUMPARAMNUM(1);
		wstring fname = PARAM(0);
		BKE_VarArray *arr = (BKE_VarArray*)(self->obj);
		if (arr->getCount() == 0)
			RETURNDEFAULT;
		wstring str2 = L"";
		wchar_t crlf[] = L"\r\n";
		try
		{
			str2 += arr->getMember(0);
		}
		catch (...){}
		for (int i = 1; i < arr->getCount(); i++)
		{
			str2 += crlf;
			try
			{
				str2 += arr->getMember(i);
			}
			catch (...){}
		}
		bool re = BKE_writeFile(str2, fname, 0);
		if (!re)
			throw Var_Except(L"写入文件失败");
		RETURNDEFAULT;
	}

	NATIVE_FUNC(saveStruct)
	{
		MINIMUMPARAMNUM(1);
		wstring fname = PARAM(0);
		wstring res;
		self->save(res);
		bool re = BKE_writeFile(res, fname, 0);
		if (!re)
			throw Var_Except(L"写入文件失败");
		RETURNDEFAULT;
	}

	NATIVE_FUNC(loadFile)
	{
		MINIMUMPARAMNUM(1);
		wstring fname = PARAM(0);
		wstring res;
		bool re = BKE_readFile(res, fname);
		if (!re)
			throw Var_Except(L"打开文件失败");
		else
			return res;
	}

	//bool saveFile(filename, string, pos)
	//pos==-1, append
	NATIVE_FUNC(saveFile)
	{
		MINIMUMPARAMNUM(2);
		wstring fname = PARAM(0);
		const wstring &res = PARAM(1).asBKEStr().getConstStr();
		bkplong pos = 0;
		if (PARAMEXIST(2))
			pos = PARAM(2);
		bool re = BKE_writeFile(res, fname, pos);
		if (!re)
			throw Var_Except(L"写入文件失败");
		RETURNDEFAULT;
	}

	NATIVE_FUNC(appendFile)
	{
		MINIMUMPARAMNUM(2);
		wstring fname = PARAM(0);
		const wstring &res = PARAM(1).asBKEStr().getConstStr();
		bool re = BKE_writeFile(res, fname, -1);
		if (!re)
			throw Var_Except(L"写入文件失败");
		RETURNDEFAULT;
	}
	
	NATIVE_FUNC(evalFile)
	{
		MINIMUMPARAMNUM(1);
		wstring fname = PARAM(0);
		wstring res;
		bool re = BKE_readFile(res, fname);
		if (!re)
			throw Var_Except(L"打开文件失败");
		return Parser::getInstance()->eval(res);
	}

#endif

	NATIVE_FUNC(toString)
	{
		if (!PARAMEXIST(0))
			RETURNDEFAULT;
		wstring res;
		bool format = false;
		if (PARAMEXIST(1) && (bool)PARAM(1) == true)
			format = true;
		PARAM(0).save(res, format, 0);
		return res;
	}

	//loadClass(classname, dic)
	//loadClass("xd", %[a:1])
	NATIVE_FUNC(loadClass)
	{
		MINIMUMPARAMNUM(2);
		auto name = PARAM(0).asBKEStr();
		auto var = BKE_VarClosure::global()->getMember(name);
		if (var.getType() != VAR_CLASS || static_cast<BKE_VarClass*>(var.obj)->isInstance())
		{
			throw Var_Except(L"传入的第一个参数必须是一个类的名称。");
		}
		if (PARAM(1).getType() != VAR_DIC)
		{
			throw Var_Except(L"传入的第二个参数必须是字典。");
		}
		auto cla = static_cast<BKE_VarClass*>(var.obj)->createInstance(NULL);
		auto obj = static_cast<BKE_VarClass*>(cla.obj);
		obj->varmap.Union(static_cast<BKE_VarDic*>(PARAM(1).obj)->varmap, true);
		if (obj->native)
			obj->native->nativeLoad(PARAM(2));
		return cla;
	}

	NATIVE_GET(length)
	{
		switch (self->getType())
		{
		case VAR_STR:
			return self->str.size();
		case VAR_ARRAY:
			return ((BKE_VarArray*)self->obj)->getCount();
		case VAR_DIC:
			return ((BKE_VarDic*)self->obj)->getCount();
		default:
			RETURNDEFAULT
		}
	}

	NATIVE_SET(length)
	{
		int s=PARAM(0);
		if(s<0)
			throw Var_Except(L"不合法的参数");
		((BKE_VarArray*)self->obj)->setLength(s);
		RETURNDEFAULT
	}

	//string.beginWith(xxx)
	NATIVE_FUNC(beginWith)
	{
		MINIMUMPARAMNUM(1);
#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (((BKE_VarClass *)PARAM(0).obj)->isInstanceof(L"Regex"))
			{
				Regex *r = (Regex *)((BKE_VarClass *)PARAM(0).obj)->native;
				std::wsmatch m;
				std::regex_search(self->str.getConstStr(), m, r->reg);
				if (m.empty())
					return false;
				return m.prefix().length() == 0;
			}
			throw Var_Except(L"参数1必须是Regex的子类或字符串");
		}
#endif
		BKE_String str = PARAM(0).asBKEStr();
		if (str.size() > self->str.size())
			return false;
		const wchar_t *src = self->str.getConstStr().c_str();
		const wchar_t *dst = str.getConstStr().c_str();
		while (*dst)
		{
			if (*src++ != *dst++)
				return false;
		}
		return true;
	}

	//string.endWith(xxx)
	NATIVE_FUNC(endWith)
	{
		MINIMUMPARAMNUM(1);
#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (((BKE_VarClass *)PARAM(0).obj)->isInstanceof(L"Regex"))
			{
				Regex *r = (Regex *)((BKE_VarClass *)PARAM(0).obj)->native;
				const wstring &s = self->str.getConstStr();
				for (int i = (int)s.size() - 1; i >= 0; i--)
				{
					if (std::regex_match(s.substr(i), r->reg))
						return true;
				}
				return false;
			}
			throw Var_Except(L"参数1必须是Regex的子类或字符串");
		}
#endif
		BKE_String str = PARAM(0).asBKEStr();
		const wchar_t *src = self->str.getConstStr().c_str();
		const wchar_t *dst = str.getConstStr().c_str();
		bkplong off = (bkplong)self->str.getConstStr().size() - (bkplong)str.getConstStr().size();
		if (off < 0)
			return false;
		return wcscmp(src + off, dst)==0;
	}

	//replace(a, b)		a=>b
	NATIVE_FUNC(replace)
	{
		MINIMUMPARAMNUM(2);
#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			auto param = new BKE_VarArray;
			param->pushMember(*self);
			param->pushMember(PARAM(1));
			BKE_Variable tmp(param);
			return Regex::nativeFunc_replaceAll(&PARAM(0), param, _this);
		}
#endif
		wstring old=PARAM(0);
		bkplong oldsize=static_cast<bkplong>(old.size());
		wstring newvalue=PARAM(1);
		//bkplong newsize=static_cast<bkplong>(newvalue.size());
		const wstring &s=self->str.getConstStr();
		//bkplong ssize=static_cast<bkplong>(s.size());
		wstring res;
		bkplong i=0;
		bkplong start;
		do
		{
			start=i;
			i=static_cast<bkplong>(s.find(old, start));
			if(i==s.npos)
			{
				res+=s.substr(start);
				return res;
			}
			res+=s.substr(start, i-start)+newvalue;
			i+=oldsize;
		}while(1);
	}

	NATIVE_FUNC(indexOf)
	{
		MINIMUMPARAMNUM(1);
		bkplong start = (bkplong)PARAM(1);
		if (start < 0 || start > self->str.getConstStr().length())
			return -1;
#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (((BKE_VarClass *)PARAM(0).obj)->isInstanceof(L"Regex"))
			{
				Regex *r = (Regex *)((BKE_VarClass *)PARAM(0).obj)->native;
				std::wsmatch m;
				if(!std::regex_search(self->str.getConstStr().substr(start), m, r->reg))
					return -1;
				else
					return (bkplong)m.position();
			}
			throw Var_Except(L"参数1必须是Regex的子类或字符串");
		}
#endif
		bkplong s = static_cast<bkplong>(self->str.getConstStr().find(PARAM(0).asString(), start));
		if (s > wstring::npos)
			return -1;
		return s;
	}

	NATIVE_FUNC(lastIndexOf)
	{
		MINIMUMPARAMNUM(1);
		bkplong endpos = (bkplong)PARAM(1);
		if (endpos < 0)
			return -1;
#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (((BKE_VarClass *)PARAM(0).obj)->isInstanceof(L"Regex"))
			{
				Regex *r = (Regex *)((BKE_VarClass *)PARAM(0).obj)->native;
				std::wsmatch m;
				const wstring &s = self->str.getConstStr();
				for (int i = endpos - 1; i >= 0; i--)
				{
					std::regex_search((const wstring &)s.substr(i), m, r->reg);
					if (!m.empty())
						return (bkplong)(i + m.position());
				}
				return -1;
			}
			throw Var_Except(L"参数1必须是Regex的子类或字符串");
		}
#endif
		bkplong s = static_cast<bkplong>(self->str.getConstStr().rfind(PARAM(0).asString(), endpos));
		if (s > (bkplong)self->str.getConstStr().size())
			return -1;
		return s;
	}

	NATIVE_FUNC(toLowerCase)
	{
		return self->str.toLowerCase();
	}

	NATIVE_FUNC(toUpperCase)
	{
		return self->str.toUpperCase();
	}

	NATIVE_FUNC(substring)
	{
		MINIMUMPARAMNUM(1);
		bkplong start = (bkplong)PARAM(0);
		if (start < 0 || self->str.getConstStr().length() <= start)
			RETURNDEFAULT;
		if (PARAMEXIST(1))
			return self->str.substr((bkplong)PARAM(0), (bkplong)PARAM(1));
		else
			return self->str.substr((bkplong)PARAM(0));
	}

	NATIVE_FUNC(sprintf)
	{
		const wstring &src=self->str.getConstStr();
		wstring res;
		int i=0;
		int paramno=0;
		bkplong srcsize=(bkplong)src.size();
		while(i<srcsize)
		{
			if(src[i]!=L'%')
				res.push_back(src[i]);
			else
			{
				i++;
				if(i>=srcsize)
				{
					res.push_back(L'%');
					return res;
				}
				else if(src[i]==L'%')
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
					} while (i < srcsize && !iswalpha(ch));
					if (!PARAMEXIST(paramno))
						throw Var_Except(L"参数数目不足");
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
						wstring str = (wstring)PARAM(paramno++).asString();
						string __tmp = UniToUTF8(str);
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
					res.append(UniFromUTF8(buf3, (bkpulong)strlen(buf3)));
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
#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (((BKE_VarClass *)PARAM(0).obj)->isInstanceof(L"Regex"))
			{
				Regex *r = (Regex *)((BKE_VarClass *)PARAM(0).obj)->native;
				std::wsmatch m;
				BKE_VarArray *arr = new BKE_VarArray();
				wstring tmp = self->str.getConstStr();
				std::regex_search(tmp, m, r->reg);
				if (m.empty())
				{
					arr->pushMember(tmp);
					return arr;
				}
				while (m.suffix().length() > 0)
				{
					if (m.prefix().length() > 0 || !PARAM(1))
						arr->pushMember(m.prefix().str());
					tmp = m.suffix();
					if (!std::regex_search(tmp, m, r->reg))
						break;
				}
				if (m.prefix().length() > 0 || !PARAM(1))
					arr->pushMember(m.prefix().str());
				if (m.suffix().length() > 0 || !PARAM(1))
					arr->pushMember(m.suffix().str());
				return arr;
			}
			throw Var_Except(L"参数1必须是Regex的子类或字符串");
		}
#endif
		const wstring &src = self->str.getConstStr();
		wstring split = PARAM(0);
		bkplong splitsize = static_cast<bkplong>(split.size());
		BKE_VarArray *arr = new BKE_VarArray();
		int start = 0;
		bkplong srcsize = (bkplong)src.size();
		while (start < srcsize)
		{
			bkplong pos = static_cast<bkplong>(src.find(split, start));
			if (pos == src.npos)
				break;
			if (pos > start || !PARAM(1))
				arr->pushMember(src.substr(start, pos - start));
			start = pos + splitsize;
		}
		if (start < srcsize)
			arr->pushMember(src.substr(start));
		else if (!PARAM(1))
			arr->pushMember(BKE_Variable());
		return arr;
	}

	//add(xx,xx,xx,...)
	NATIVE_FUNC(add)
	{
		int i=0;
		long cnt=paramarray->getCount();
		while(i<cnt)
			((BKE_VarArray*)self->obj)->pushMember(PARAM(i++));
		RETURNDEFAULT;
	}

	NATIVE_FUNC(remove)
	{
		int i=0;
		while(i<paramarray->getCount())
		{
			if(self->getType()==VAR_ARRAY)
				((BKE_VarArray*)self->obj)->deleteMember(PARAM(i));
			else
				((BKE_VarDic*)self->obj)->deleteMember(PARAM(i));
			i++;
		}
		RETURNDEFAULT;
	}

	NATIVE_FUNC(erase)
	{
		int i=0;
		while (i<paramarray->getCount())
		{
			if (self->getType() == VAR_ARRAY)
				((BKE_VarArray*)self->obj)->deleteMemberIndex(PARAM(i).asInteger());
			else
				((BKE_VarDic*)self->obj)->deleteMemberIndex(PARAM(i).asBKEStr());
			i++;
		}
		RETURNDEFAULT;
	}

	NATIVE_FUNC(clear)
	{
		switch (self->getType())
		{
		case VAR_STR:
			const_cast<BKE_Variable*>(self)->str.setEmpty();
			break;
		case VAR_ARRAY:
			((BKE_VarArray*)self->obj)->clear();
			break;
		case VAR_DIC:
			((BKE_VarDic*)self->obj)->clear();
			break;
		}
		RETURNDEFAULT;
	}

	//insert(index, var)
	NATIVE_FUNC(insert)
	{
		MINIMUMPARAMNUM(2);
		((BKE_VarArray*)self->obj)->insertMember(PARAM(0).asInteger(), PARAM(1));
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
				auto &&v = ((BKE_VarArray *)self->obj)->vararray;
				for (int i = 0; i < v.size(); i++)
				{
					if (var == v[i])
						return i;
				}
				return -1;
			}
		case VAR_DIC:
			{
				auto it = ((BKE_VarDic *)self->obj)->varmap.find(var.asBKEStr());
				if (it == ((BKE_VarDic *)self->obj)->varmap.end() || it->second.isVoid())
					RETURNDEFAULT;
				return 1;
			}
		}
		throw Var_Except(L"只能用于字符串、数组或字典类型");
	}

	NATIVE_FUNC(join)
	{
		wstring s;
		wstring st;
		if (PARAMEXIST(0))
			st = PARAM(0).asString();
		auto &&arr = ((BKE_VarArray *)self->obj)->vararray;
		if (arr.size() > 0)
			s = arr[0].asString();
		for (int i = 1; i < arr.size(); i++)
			s += st + (wstring)arr[i];
		return s;
	}

	NATIVE_FUNC(array_random)
	{
		auto &&v = ((BKE_VarArray *)self->obj)->vararray;
		if (v.size() == 0)
			RETURNDEFAULT;
		return v[((bkpulong)bkpRandomInt()) % v.size()];
	}

//quicksort
	//when a<b, CompareFunc(a,b) return true
	template<class T, class CompareFunc>
	void quickSort(T &arr, bkplong first, bkplong last, const CompareFunc &comp)
	{
		if (last - first < 1)
			return;
		auto obj = arr[first];
		bkplong rawfirst = first;
		bkplong rawlast = last;
		while (first < last)
		{
			for (; last>first; last--)
			{
				if (comp(arr[last], obj))
				{
					arr[first] = arr[last];
					break;
				}
			}
			for (; last > first; first++)
			{
				if (comp(obj, arr[first]))
				{
					arr[last] = arr[first];
					break;
				}
			}
			arr[first] = obj;
			quickSort(arr, rawfirst, first - 1, comp);
			quickSort(arr, first + 1, rawlast, comp);
		}
	}

	template<class T>
	void quickSortByFun(T &arr, bkplong first, bkplong last, BKE_VarFunction *fun)
	{
		if (last - first < 1)
			return;
		auto obj = arr[first];
		bkplong rawfirst = first;
		bkplong rawlast = last;
		auto p = new BKE_VarArray();
		BKE_Variable tmp(p);
		while (first < last)
		{
			for (; last>first; last--)
			{
				p->clear();
				p->pushMember(arr[last]);
				p->pushMember(obj);
				if (fun->run(p))
				{
					arr[first] = arr[last];
					break;
				}
			}
			for (; last > first; first++)
			{
				p->clear();
				p->pushMember(obj);
				p->pushMember(arr[first]);
				if (fun->run(p))
				{
					arr[last] = arr[first];
					break;
				}
			}
			arr[first] = obj;
			quickSortByFun(arr, rawfirst, first - 1, fun);
			quickSortByFun(arr, first + 1, rawlast, fun);
		}
	}

	//sort([func])
	//sort("mode")
	//mode='0','9','+','-','a','z'
	//default mode is +
	NATIVE_FUNC(sort)
	{
		auto &&arr = ((BKE_VarArray *)self->obj)->vararray;
		bkplong s = (bkplong)arr.size();
		BKE_Variable ss = L"+";
		if (PARAMEXIST(0))
			ss = PARAM(0);
		if (ss.getType() == VAR_FUNC)
			quickSortByFun(arr, 0, s - 1, (BKE_VarFunction*)PARAM(0).obj);
		else if (ss.getType() == VAR_STR)
		{
			wchar_t mode = L'+';
			if (paramarray && PARAM(0).asBKEStr().getConstStr().size() > 0)
				mode = PARAM(0).asBKEStr().getConstStr()[0];
			switch (mode)
			{
			case '-':
				quickSort(arr, 0, s - 1, [](const BKE_Variable &a, const BKE_Variable &b){return a > b; });
				break;
			case '0':
				quickSort(arr, 0, s - 1, [](const BKE_Variable &a, const BKE_Variable &b){return (double)a < (double)b; });
				break;
			case '9':
				quickSort(arr, 0, s - 1, [](const BKE_Variable &a, const BKE_Variable &b){return (double)a >(double)b; });
				break;
			case 'a':
				quickSort(arr, 0, s - 1, [](const BKE_Variable &a, const BKE_Variable &b){return (wstring)a < (wstring)b; });
				break;
			case 'z':
				quickSort(arr, 0, s - 1, [](const BKE_Variable &a, const BKE_Variable &b){return (wstring)a >(wstring)b; });
				break;
			default:
				quickSort(arr, 0, s - 1, [](const BKE_Variable &a, const BKE_Variable &b){return a < b; });
				break;
			}
		}
		else
		{
			throw Var_Except(L"sort的参数必须是个排序函数或表示排序模式的字符串。");
		}
		RETURNDEFAULT;
	}

	NATIVE_FUNC(concat)
	{
		BKE_Variable arr = self->clone();
		for (int i = 0; i < paramarray->getCount(); i++)
		{
			BKE_Variable &v = paramarray->quickGetMember(i);
			if (v.vt==VAR_NONE)
			{
				continue;
			}
			else if (v.vt==VAR_ARRAY)
			{
				static_cast<BKE_VarArray *>(arr.obj)->concat(static_cast<BKE_VarArray *>(v.obj));
			}
			else
			{
				throw(Var_Except(L"concat的参数必须是数组。"));
			}
		}
		return arr;
	}

	NATIVE_FUNC(clone)
	{
		BKE_Variable res;
		res.copyFrom(*self);
		return res;
	}

	NATIVE_FUNC(update)
	{
		MINIMUMPARAMNUM(1);
		auto &v = PARAM(0);
		if (v.vt==VAR_NONE)
		{
			RETURNDEFAULT;
		}
		else if (v.vt==VAR_DIC)
		{
			static_cast<BKE_VarDic *>(self->obj)->update(static_cast<BKE_VarDic *>(v.obj));
			RETURNDEFAULT;
		}
		throw(Var_Except(L"字典的update操作需要右操作数为一个字典。"));
	}

	NATIVE_FUNC(except)
	{
		MINIMUMPARAMNUM(1);
		auto &v = PARAM(0);
		if (v.vt == VAR_NONE)
		{
			RETURNDEFAULT;
		}
		else if (v.vt == VAR_DIC)
		{
			static_cast<BKE_VarDic *>(self->obj)->except(static_cast<BKE_VarDic *>(v.obj));
			RETURNDEFAULT;
		}
		throw(Var_Except(L"字典的except操作需要右操作数为一个字典。"));
	}

	NATIVE_FUNC(toArray)
	{
		return ((BKE_VarDic*)self->obj)->toArray();
	}

	NATIVE_FUNC(getKeyArray)
	{
		auto &&varmap = ((BKE_VarDic*)self->obj)->varmap;
		BKE_VarArray *arr = new BKE_VarArray();
		for (auto it = varmap.begin(); it != varmap.end(); it++)
		{
			if (!it->second.isVoid())
				arr->pushMember(it->first);
		}
		return arr;
	}

	NATIVE_FUNC(getValueArray)
	{
		auto &&varmap = ((BKE_VarDic*)self->obj)->varmap;
		BKE_VarArray *arr = new BKE_VarArray();
		for (auto it = varmap.begin(); it != varmap.end(); it++)
		{
			if (!it->second.isVoid())
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
		auto &&varmap = ((BKE_VarDic*)self->obj)->varmap;
		auto v = new BKE_VarArray();
		v->vararray.reserve(varmap.getCount());
		for (auto &&i : varmap)
		{
			if (!i.second.isVoid())
				v->vararray.push_back(i.first);
		}
		BKE_Variable ss = L"+";
		if (PARAMEXIST(0))
			ss = PARAM(0);
		if (ss.getType() == VAR_FUNC)
		{
			BKE_Variable tmp = BKE_Variable::array();
			quickSort(v->vararray, 0, varmap.getCount() - 1, [&](const BKE_Variable &a, const BKE_Variable &b)
				{
					auto arr = (BKE_VarArray*)tmp.obj;
					arr->clear();
					arr->pushMember(varmap[a.str]);
					arr->pushMember(varmap[b.str]);
					return (bool)((BKE_VarFunction*)PARAM(0).obj)->run(arr);
				}
			);
		}
		else if (ss.getType() == VAR_STR)
		{
			wchar_t mode = L'+';
			if (paramarray && PARAM(0).asBKEStr().getConstStr().size() > 0)
				mode = PARAM(0).asBKEStr().getConstStr()[0];
			switch (mode)
			{
			case '-':
				quickSort(v->vararray, 0, varmap.getCount() - 1, [&](const BKE_Variable &a, const BKE_Variable &b){return varmap[a.str] > varmap[b.str]; });
				break;
			case '0':
				quickSort(v->vararray, 0, varmap.getCount() - 1, [&](const BKE_Variable &a, const BKE_Variable &b){return (double)varmap[a.str] < (double)varmap[b.str]; });
				break;
			case '9':
				quickSort(v->vararray, 0, varmap.getCount() - 1, [&](const BKE_Variable &a, const BKE_Variable &b){return (double)varmap[a.str] > (double)varmap[b.str]; });
				break;
			case 'a':
				quickSort(v->vararray, 0, varmap.getCount() - 1, [&](const BKE_Variable &a, const BKE_Variable &b){return (wstring)varmap[a.str] < (wstring)varmap[b.str]; });
				break;
			case 'z':
				quickSort(v->vararray, 0, varmap.getCount() - 1, [&](const BKE_Variable &a, const BKE_Variable &b){return (wstring)varmap[a.str] > (wstring)varmap[b.str]; });
				break;
			default:
				quickSort(v->vararray, 0, varmap.getCount() - 1, [&](const BKE_Variable &a, const BKE_Variable &b){return varmap[a.str] < varmap[b.str]; });
				break;
			}
		}
		else
		{
			throw Var_Except(L"sort的参数必须是个排序函数或表示排序模式的字符串。");
		}
		return v;
	}
#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Date
#endif
	NATIVECLASS_CREATENEW()
	{
		//if(self->classname!=L"Date")
		//	throw Var_Except(L"类型不对应");

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
		instance->_tm->tm_year = PARAM(0).convertTo<int>() - 1970;
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
		instance->_tm->tm_mon = PARAM(0).convertTo<int>() - 1;
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
		instance->_tm->tm_mday = PARAM(0).convertTo<int>();
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
		instance->_tm->tm_hour = PARAM(0).convertTo<int>();
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
		instance->_tm->tm_min = PARAM(0).convertTo<int>();
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
		instance->_tm->tm_sec = PARAM(0).convertTo<int>();
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(format)
	{
		MINIMUMPARAMNUM(1);
		GETINSTANCE();
		wstring format = PARAM(0).asString();
		wstring res;
		bkpulong i = 0;
		bkpulong s = (bkpulong)format.size();
		bool shrink = false;	//正处于#的作用下
		while (i < s)
		{
			wchar_t ch = format[i++];
			if (ch != L'%' && !shrink)
				res.push_back(ch);
			else
			{
				if (i >= s)
					return res;
				if (!shrink)
					ch = format[i++];
				wstring tmpstr(1, ch);
				switch (ch)
				{
				case L'%':
					break;
				case L'#':
					if (!shrink)
					{
						shrink = true;
						continue;
					}
					break;
				case L'Y':
					tmpstr = bkpInt2Str(instance->_tm->tm_year + 1900);
					if (tmpstr.size() < 4)
						tmpstr = wstring(4 - tmpstr.size(), L'0');
					if (shrink)
						tmpstr = tmpstr.substr(tmpstr.size() - 2);
					break;
				case L'M':
					tmpstr = bkpInt2Str(instance->_tm->tm_mon + 1);
					if (!shrink && instance->_tm->tm_mon < 9)
						tmpstr = L'0' + tmpstr;
					break;
				case L'D':
					tmpstr = bkpInt2Str(instance->_tm->tm_mday);
					if (!shrink && instance->_tm->tm_mday < 10)
						tmpstr = L'0' + tmpstr;
					break;
				case L'h':
					tmpstr = bkpInt2Str(instance->_tm->tm_hour);
					if (!shrink && instance->_tm->tm_hour < 10)
						tmpstr = L'0' + tmpstr;
					break;
				case L'm':
					tmpstr = bkpInt2Str(instance->_tm->tm_min);
					if (!shrink && instance->_tm->tm_min < 10)
						tmpstr = L'0' + tmpstr;
					break;
				case L's':
					tmpstr = bkpInt2Str(instance->_tm->tm_sec);
					if (!shrink && instance->_tm->tm_sec < 10)
						tmpstr = L'0' + tmpstr;
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
		FUNC_INFO(sin),
		FUNC_INFO(cos),
		FUNC_INFO(tan),
		FUNC_INFO(asin),
		FUNC_INFO(acos),
		FUNC_INFO(atan),
		FUNC_INFO(random),
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
		return log(PARAM(1).asNumber())/log(PARAM(0).asNumber());
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
		return (bkplong)((double)PARAM(0)+0.5);
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
		auto arr = new BKE_VarArray();
		if(!std::regex_search(tmp, m, instance->reg))
			return arr;
		while (!m.empty())
		{
			arr->pushMember(m[0].str());
			tmp = m.suffix();
			if (tmp.empty())
				return arr;
			if(!std::regex_search(tmp, m, instance->reg))
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
		auto arr = new BKE_VarArray();
		if(!std::regex_search(src, m, instance->reg))
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
			if(!std::regex_search(tmp, m, instance->reg))
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
		bkpulong container = PARAM(0);
		bkpulong flag = PARAM(1);
		return (bool)((container & flag) == flag);
	}

	NATIVECLASS_FUNC(split)
	{
		MINIMUMPARAMNUM(1);
		bkpulong container = PARAM(0);
		BKE_VarArray *arr = new BKE_VarArray();
		bkpulong mask = 1;
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
		bkpulong container = PARAM(0);
		bkpulong mask = PARAM(1);
		return container & mask;
	}

	NATIVECLASS_FUNC(merge)
	{
		MINIMUMPARAMNUM(1);
		bkpulong container = PARAM(0);
		int count = PARAMCOUNT();
		for (int i = 1; i < count; i++)
		{
			container |= (bkpulong)PARAM(i).asInteger();
		}
		return container;
	}

	REG_FUNC_BEGIN(Global)
	{
		/**
		*  @param str
		*  @return value
		*  @brief  返回str的执行结果。
		*/
		{ QUICKFUNC(eval) },
		/**
		*  @param str
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
		*  @param (value1, value2, ...)
		*  @brief  逐个打印参数，逗号分隔。
		*/
		{ QUICKFUNC(log) },
		/**
		*  @param (str1, str2, ...)
		*  @brief  逐个打印字符串内容，逗号分隔。
		*/
		{ QUICKFUNC(print) },
		/**
		*  @param num1(, num2)
		*  @brief  生成一个数组。
		*  如果没有num2，那么生成的是[0, 1, ..., num1-1]
		*  如果num2>num1，那么生成的是[num1, num1+1, ..., num2-1]
		*  如果num2<num1，那么生成的是[num1, num1-1, ..., num2+1]
		*  如果num2=num1，那么生成的是空数组
		*/
		{ QUICKFUNC(range) },
		/**
		*  @param num1(, num2)
		*  @brief  生成一个随机整数。
		*  如果没有num2，那么相当于random(0, num1)，表现见下：
		*  如果num2>num1，那么生成的是num1到num2-1之间的一个整数
		*  如果num2<num1，那么生成的是num2到num1-1之间的一个整数
		*  如果num2=num1，那么返回num1
		*/
		{ QUICKFUNC(random) },
		/**
		*  @param num
		*  @return string
		*  @brief  相当于(string)(int)num，先对数字取整，然后生成相应的字符串。
		*/
		{ QUICKFUNC(itoa) },
		/**
		*  @param num
		*  @return string
		*  @brief  同itoa，只是生成的字符串是全角的'０', '１'等字符。
		*/
		{ QUICKFUNC(itoa2) },
		/**
		*  @param string
		*  @return integer
		*  @brief  返回string[0]对应的unicode编码。没有则返回0。
		*/
		{ QUICKFUNC(char) },
		/**
		*  @param value
		*  @return integer
		*  @brief  返回value对应的32位hash值。
		*/
		{ QUICKFUNC(hash) },
		/**
		*  @param value
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
		*		abc
		*		的1.txt文件载入为数组的示例：
		*	@example_code [].loadFile("1.txt")
		*	@example_result "abc"
		*/
		{ QUICKFUNC(loadFile) },
		/**
		*	@param string filename 文件名
		*	@param string str 将要保存的字符串
		*	@bool append=false 是否追加在文件末尾。若为否，则覆盖整个文件。
		*	@return bool 是否成功
		*	@brief 将一个字符串写入文件。
		*/
		{ QUICKFUNC(saveFile) },
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
		*	@brief 对文件读取并且解析（反序列化），将结果返回。
		*	@body 等同于{@link eval}({@link loadFile}(filename, krmode, rawstr));
		*/
		{ QUICKFUNC(evalFile) },
#endif
		/**
		*  @param value
		*  @param bool format 是否格式化
		*  @return string
		*  @brief  将value转变成字符串，format表示是否加上缩进格式。
		*  @example 以下将演示一个数组[1,2,3]的toString结果
		*  @example_code toString([1,2,3])
		*  @example_result [1,2,3]
		*  @example_code toString([1,2,3], true)
		*  @example_result [
		*		1,
		*		2,
		*		3
		*	]
		*/
		{ QUICKFUNC(toString) },
		/**
		*  @param classname, dic(, nativedata)
		*  @return class instance
		*  @brief  内部用，用于从保存的类对象中还原。
		*/
		{ QUICKFUNC(loadClass) },
	}
	REG_FUNC_END;

	REG_FUNC_BEGIN(Undefined)
	{
		/**
		*	@class undefined（内部保留类）
		*   @param value
		*   @return bool
		*   @brief  比较本对象与value值是否相等。
		*/
		{QUICKFUNC(equals)},
	}
	REG_FUNC_END;

	REG_GET_BEGIN(String)
	{
		/**
		*	@class string（内部保留类）
		*   @return integer
		*   @brief  返回字符串的长度。
		*/
		{ QUICKGETTER(length) },
		/**
		*	@class string（内部保留类）
		*   @return integer
		*   @brief  同length，返回字符串的长度。
		*/
		{ L"size", &Parser_Util::nativeGet_length }
	}
	REG_GET_END;

	REG_FUNC_BEGIN(String)
	{
		/**
		*	@class string（内部保留类）
		*	@param start(, len)
		*   @return string
		*   @brief  取从start开始长度为len的子串，如果省略len，则取到原字符串末尾。如果start位置不合法，则返回空串。
		*/
		{ QUICKFUNC(substring) },
		/**
		*	@class string（内部保留类）
		*	@param start(, len)
		*   @return string
		*   @brief  同substring，取从start开始长度为len的子串，如果省略len，则取到原字符串末尾。如果start位置不合法，则返回空串。
		*/
		{ L"substr", &Parser_Util::nativeFunc_substring },
		/**
		*	@class string（内部保留类）
		*	@param (arg1, arg2, ...)
		*   @return string
		*   @brief  格式化字符串，以本字符串为格式，根据传入的参数生成一个新的字符串。
		*   @example 以下将演示用"%04d"来格式化233的结果
		*   @example_code "%04d".sprintf(233)
		*   @example_result "0233"
		*/
		{ QUICKFUNC(sprintf) },
		/**
		*	@class string（内部保留类）
		*	@param str | regexp(, ignorenull = false)
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
		*	@param str | regexp(, startpos = 0)
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
		*	@param str | regexp(, endpos = 0)
		*   @return integer
		*   @brief  第一个参数为字符串或正则表达式类。从endpos位置开始，向前寻找第一次出现参数1的位置。
		*   @example 以下将演示在"1,2,,3"的查找","的结果
		*   @example_code "1,2,,3".lastIndexOf(",")
		*   @example_result 4
		*   @example_code "1,2,,3".lastIndexOf(",", 2)
		*   @example_result 1
		*/
		{ QUICKFUNC(lastIndexOf) },
		/**
		*	@class string（内部保留类）
		*	@param str | regexp, str2
		*   @return string
		*   @brief  参数为字符串或正则表达式类。将原字符串中str1全部替换为str2。
		*   @example 以下将演示将"aaa"中"a"替换为"ab"的结果
		*   @example_code "aaa".replace("a", "ab")
		*   @example_result "ababab"
		*/
		{ QUICKFUNC(replace) },
		/**
		*  @param value
		*  @return bool
		*  @brief  比较本对象与value值是否相等。
		*/
		{ QUICKFUNC(equals) },
		/**
		*	@class string（内部保留类）
		*	@param str | regexp
		*   @return bool
		*   @brief  参数为字符串或正则表达式类。返回原字符串是否已参数给定的字符串开头。
		*/
		{ QUICKFUNC(beginWith) },
		/**
		*	@class string（内部保留类）
		*	@param str | regexp
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
		{ QUICKGETTER(length) },
		/**
		*	@class array（内部保留类）
		*   @return integer
		*   @brief  同length，返回数组的长度。
		*/
		{ L"size", &Parser_Util::nativeGet_length }
	}
	REG_SET_END;

	REG_FUNC_BEGIN(Array)
	{
		/**
		*	@class array（内部保留类）
		*	@param (arg1, arg2, ...)
		*   @brief  向数组尾部添加参数。
		*/
		{ QUICKFUNC(add) },
		/**
		*	@class array（内部保留类）
		*	@param (arg1, arg2, ...)
		*   @brief  从数组中删除与参数相同的元素，使用==比较。
		*/
		{ QUICKFUNC(remove) },
		/**
		*	@class array（内部保留类）
		*	@param (index1, index2, ...)
		*   @brief  从数组中删除参数对应下标的元素，负数表示从后往前数，注意是从前往后逐个删除的，所以erase(1,1)会删除原数组下标为1,2位置的元素。越界会抛出异常。
		*/
		{ QUICKFUNC(erase) },
		/**
		*	@class array（内部保留类）
		*	@param index, value
		*   @brief  在数组index位置插入一个新的value。
		*   @example 以下将演示在[1,2,3]的2位置插入4的结果
		*   @example_code var a=[1,2,3];
		*	a.insert(2,4);
		*	log(a)
		*   @example_result [1,2,4,3]
		*/
		{ QUICKFUNC(insert) },
		/**
		*	@class array（内部保留类）
		*	@param value
		*	@return integer
		*   @brief  返回数组中第一个等于value的元素的位置，使用==比较，找不到返回-1。
		*/
		{ QUICKFUNC(find) },
		/**
		*	@class array（内部保留类）
		*	@param (str)
		*	@return string
		*   @brief  将数组中所有元素转成字符串，然后用str连接。
		*   @example 以下将演示用"-"连接[1,2,3]的结果
		*   @example_code [1,2,3].join("-")
		*   @example_result "1-2-3"
		*/
		{ QUICKFUNC(join) },
		/**
		*	@class array（内部保留类）
		*	@param (arr1, arr2, ...)
		*	@return string
		*   @brief  但会将此数组与参数中所有数组的元素连接起来形成的大数组，原数组不受影响。
		*   @example 以下将演示[1,2,3]和[3,2,1]连接的结果
		*   @example_code [1,2,3].concat([3,2,1])
		*   @example_result [1,2,3,3,2,1]
		*/
		{ QUICKFUNC(concat) },
		/**
		*	@class array（内部保留类）
		*	@param func | modestr = "+"
		*   @brief  根据指定的函数或模式排序。会改动原字符串，没有返回值。
		*	如果指定的是函数，将把此函数（func(a,b)）当做<的比较函数，从小到大排序。
		*	如果指定的是模式字符串，则：
		*		"+"：以默认的比较运算符从小到大排序
		*		"-"：以默认的比较运算符从大到小排序
		*		"0"：按数值从小到大排序
		*		"9"：按数值从大到小排序
		*		"a"：按字符串从小到大排序
		*		"z"：按字符串从大到小排序
		*   @example 以下将演示[1,2,3]按"-"排序的结果
		*   @example_code var a=[1,2,3];
		*	a.sort("9");
		*	log(a)
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
		*   @param value
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
		*	@return value
		*   @brief  随机返回数组中一个元素。
		*/
		{ L"random", &Parser_Util::nativeFunc_array_random },
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
		*	@brief 读取文件并按照换行切分为数组。
		*	@example 以下将演示一个从内容为
		*		a
		*		b
		*		c
		*		的1.txt文件载入为数组的示例：
		*	@example_code [].load("1.txt")
		*	@example_result ["a","b","c"]
		*/
		{ QUICKFUNC(load) },
		/**
		*	@class array（内部保留类）
		*	@param string filename 文件名
		*	@param string mode=void 模式。若首字母为'o'则字符串的其余部分代表一个偏移，文件将从该偏移处进行覆盖保存，负值将从文件末尾进行计算。若首字母为'z'则与'o'类似，只不过将启用压缩。
		*	@brief 将数组进行序列化，并保存到文件。
		*	@body 文件的内容可以直接使用{@link evalFile}进行反序列化，读取至变量。
		*	@example 以下将演示一个从内容为[1,2,3]的字典写入文件的示例：
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
		*	@param (arg1, arg2, ...)
		*   @brief  从字典中删除值与参数相同的元素，使用==比较。
		*/
		{ QUICKFUNC(remove) },
		/**
		*	@class dictionary（内部保留类）
		*	@param (arg1, arg2, ...)
		*   @brief  从字典中删除键与参数相同的元素，使用==比较。
		*/
		{ QUICKFUNC(erase) },
		/**
		*	@class dictionary（内部保留类）
		*	@return dictionary
		*   @brief  复制出一份一样的新字典，更改新字典不会影响原字典。
		*/
		{ QUICKFUNC(clone) },
		/**
		*	@class dictionary（内部保留类）
		*	@param value
		*	@return bool
		*   @brief  返回字典中是否存在key为value的键，值为空（void）的键将被视为无效键。
		*/
		{ QUICKFUNC(find) },
		/**
		*	@class dictionary（内部保留类）
		*	@return array
		*   @brief  返回[键1, 值1, 键2, 值2, ...]的数组，值为void的键将被无视。
		*/
		{ QUICKFUNC(toArray) },
		/**
		*	@class dictionary（内部保留类）
		*   @param value
		*   @return bool
		*   @brief  比较本对象与value值是否相等。如果value是字典，会比较每对键-值是否相等，不存在的键会认为值是void去做比较。
		*   @example 以下将演示%[key1:1,key2:2]和%[key1:1,key2:2,key3:3]的比较
		*   @example_code %[key1:1,key2:2].equals(%[key1:1,key2:2,key3:3])
		*   @example_result false
		*/
		{ QUICKFUNC(equals) },
		/**
		*	@class dictionary（内部保留类）
		*   @param dic
		*   @brief  与新字典合并，重复的键将被覆盖。
		*/
		{ QUICKFUNC(update) },
		/**
		*	@class dictionary（内部保留类）
		*   @param dic
		*   @brief  从本字典中去除参数字典中包含的有效键（即值不为void）。
		*/
		{ QUICKFUNC(except) },
		/**
		*	@class dictionary（内部保留类）
		*   @return array
		*   @brief  返回所有有效键组成的数组。
		*/
		{ QUICKFUNC(getKeyArray) },
		/**
		*	@class dictionary（内部保留类）
		*   @return array
		*   @brief  返回所有有效键的值组成的数组。
		*/
		{ QUICKFUNC(getValueArray) },
		/**
		*	@class dictionary（内部保留类）
		*	@param func | modestr = "+"
		*	@return array
		*   @brief  按值排序所有的有效键，比较的方式根据指定的函数或模式决定。
		*	如果指定的是函数，将把此函数（func(a,b)）当做<的比较函数，从小到大排序。
		*	如果指定的是模式字符串，则：
		*		"+"：以默认的比较运算符从小到大排序
		*		"-"：以默认的比较运算符从大到小排序
		*		"0"：按数值从小到大排序
		*		"9"：按数值从大到小排序
		*		"a"：按字符串从小到大排序
		*		"z"：按字符串从大到小排序
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
		*	@param string mode=void 模式。若首字母为'o'则字符串的其余部分代表一个偏移，文件将从该偏移处进行覆盖保存，负值将从文件末尾进行计算。若首字母为'z'则与'o'类似，只不过将启用压缩。
		*	@brief 将数组进行序列化，并保存到文件。
		*	@body 文件的内容可以直接使用{@link evalFile}进行反序列化，读取至变量。
		*	@example 以下将演示一个从内容为%[key:1]的字典写入文件的示例：
		*	@example_code %[key:1].saveStruct("1.txt")
		*/
		{ QUICKFUNC(saveStruct) },
#endif
	}
	REG_FUNC_END;

	void registerExtend(Parser *p)
	{
		//register inner classes and global functions
		BKE_VarClosure *global = BKE_VarClosure::global();
		BKE_VarClass *_class = (BKE_VarClass *)global;
		REG_FUNC(Global);

		//undefined
		BKE_String s_undefined(L"undefined");
		_class = new BKE_VarClass(s_undefined);
		REG_FUNC(Undefined);
		global->forceSetMember(s_undefined, _class);

		//number
		BKE_String s_number(L"number");
		_class = new BKE_VarClass(s_number);
		REG_FUNC(Undefined);
		global->forceSetMember(s_number, _class);

		//string
		BKE_String s_string(L"string");
		_class = new BKE_VarClass(s_string);
		REG_GET(String);
		REG_FUNC(String);
		global->forceSetMember(s_string, _class);

		//array
		BKE_String s_array(L"array");
		_class = new BKE_VarClass(s_array);
		REG_GET(String);  // Yes, the same getter list as String
		REG_SET(Array);
		REG_FUNC(Array);
		global->forceSetMember(s_array, _class);

		BKE_String s_dictionary(L"dictionary");
		_class = new BKE_VarClass(s_dictionary);
		_class->innerCreateInstance = &Parser_Util::nativeFunc_dictionary;
		REG_GET(String);  // Yes, the same getter list as String
		REG_FUNC(Dictionary);
		global->forceSetMember(s_dictionary, _class);

		BKE_String s_function(L"function");
		_class = new BKE_VarClass(s_function);
		REG_FUNC(Undefined);
		global->forceSetMember(s_function, _class);
		
		BKE_String s_closure(L"closure");
		_class = new BKE_VarClass(s_closure);
		REG_FUNC(Undefined);
		global->forceSetMember(s_closure, _class);

		//othe native class
		p->registerClass(QUICKCLASS(Date));
		p->registerClass(QUICKCLASS(Math));
#ifdef HAS_REGEX
		p->registerClass(QUICKCLASS(Regex));
#endif
		p->registerClass(QUICKCLASS(Flags));
	}
}

