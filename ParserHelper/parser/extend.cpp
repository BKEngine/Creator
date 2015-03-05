#include "extend.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <random>
#include <functional>

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
		if(!PARAMEXIST(0))
			RETURNDEFAULT;
		wstring res;
		PARAM(0).save(res);
		wcout<<res;
		for(int i=1;i<paramarray->getCount();i++)
		{
			res=L"";
			PARAM(i).save(res);
			wcout<<L","<<res.c_str();
		}
		wcout<<L"\n";
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
		for (int i = 1; i<paramarray->getCount(); i++)
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
		} while (per == 1.f);
		return (bkplong)(a+(b-a)*per);
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
		MINIMUMPARAMNUM(1);
#if WCHAR_MAX==0xFFFFFFFF
		wchar_t s = PARAM(0).asString()[0];
		return (bkplong)s;
#else
		const wstring &ss = PARAM(0).asBKEStr().getConstStr();
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
		MINIMUMPARAMNUM(1);
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
				for (int i = s.size() - 2; i >= 0; i--)
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
		bkplong off = (long)self->str.getConstStr().size() - (bkplong)str.getConstStr().size();
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
		bkplong oldsize=old.size();
		wstring newvalue=PARAM(1);
		bkplong newsize=newvalue.size();
		const wstring &s=self->str.getConstStr();
		bkplong ssize=s.size();
		wstring res;
		bkplong i=0;
		bkplong start;
		do
		{
			start=i;
			i=s.find(old, start);
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
#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (((BKE_VarClass *)PARAM(0).obj)->isInstanceof(L"Regex"))
			{
				Regex *r = (Regex *)((BKE_VarClass *)PARAM(0).obj)->native;
				std::wsmatch m;
				if(!std::regex_search(self->str.getConstStr(), m, r->reg))
					return -1;
				else
					return (bkplong)m.position();
			}
			throw Var_Except(L"参数1必须是Regex的子类或字符串");
		}
#endif
		bkplong s = self->str.getConstStr().find(PARAM(0).asString(), (bkplong)PARAM(1));
		if (s > (bkplong)self->str.getConstStr().size())
			return -1;
		return s;
	}

	NATIVE_FUNC(lastIndexOf)
	{
		MINIMUMPARAMNUM(1);
#ifdef HAS_REGEX
		if (PARAM(0).getType() == VAR_CLASS)
		{
			if (((BKE_VarClass *)PARAM(0).obj)->isInstanceof(L"Regex"))
			{
				Regex *r = (Regex *)((BKE_VarClass *)PARAM(0).obj)->native;
				std::wsmatch m;
				const wstring &s = self->str.getConstStr();
				for (int i = s.size() - 2; i >= 0; i--)
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
		bkplong s = self->str.getConstStr().rfind(PARAM(0).asString(), (bkplong)PARAM(1));
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
		if(PARAMEXIST(1))
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
					wchar_t buf2[512];
#ifdef _MSC_VER
					if (towupper(ch) == L'S')
						swprintf(buf2, 511, mode.c_str(), ((wstring)PARAM(paramno++)).c_str());
					else if (towupper(ch) == L'G')
						swprintf(buf2, 511, mode.c_str(), PARAM(paramno++).asNumber());
					else if (towupper(ch) == L'F')
						swprintf(buf2, 511, mode.c_str(), (float)(PARAM(paramno++).asNumber()));
					else
						swprintf(buf2, 511, mode.c_str(), PARAM(paramno++).asInteger());
#else
					char _mode[100];
					char buf3[1024];
					bkpwcstombs(_mode, mode.c_str(), 100);
					if (towupper(ch) == L'S')
					{
						char *__tmp=new char[4 * ((wstring)PARAM(paramno++)).size()];
						bkpwcstombs(__tmp, ((wstring)PARAM(paramno++)).c_str(), 4 * ((wstring)PARAM(paramno++)).size());
						snprintf(buf3, 1023, _mode, __tmp);
						delete[] __tmp;
					}
					else if (towupper(ch) == L'G')
					{
						snprintf(buf3, 1023, _mode, PARAM(paramno++).asNumber());
					}
					else if (towupper(ch) == L'F')
					{
						snprintf(buf3, 1023, _mode, (float)(PARAM(paramno++).asNumber()));
					}
					else
					{
						snprintf(buf3, 1023, _mode, (int)PARAM(paramno++).asInteger());
					}
					bkpmbstowcs(buf2, buf3, 511);
#endif
					res.append(buf2);
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
		wstring split=PARAM(0);
		bkplong splitsize=split.size();
		BKE_VarArray *arr=new BKE_VarArray();
		int start=0;
		bkplong srcsize=(bkplong)src.size();
		while(start<srcsize)
		{
			bkplong pos=src.find(split, start);
			if(pos==src.npos)
				break;
			if(pos>start || !PARAM(1))
				arr->pushMember(src.substr(start, pos-start));
			start = pos + splitsize;
		}
		if(start<srcsize)
			arr->pushMember(src.substr(start));
		else if (!PARAM(1))
			arr->pushMember(BKE_Variable());
		return arr;
	}

	//add(xx,xx,xx,...)
	NATIVE_FUNC(add)
	{
		MINIMUMPARAMNUM(1);
		int i=0;
		long cnt=paramarray->getCount();
		while(i<cnt)
			((BKE_VarArray*)self->obj)->pushMember(PARAM(i++));
		RETURNDEFAULT;
	}

	NATIVE_FUNC(remove)
	{
		MINIMUMPARAMNUM(1);
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
		MINIMUMPARAMNUM(1);
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
				auto it = ((BKE_VarDic *)self->obj)->varmap.find((wstring)var);
				if (it == ((BKE_VarDic *)self->obj)->varmap.end())
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
		long s = arr.size();
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
		for (auto i : varmap)
		{
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

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_FUNC(getYear);
		REG_FUNC(getMonth);
		REG_FUNC(getDay);
		REG_FUNC(getHour);
		REG_FUNC(getMinute);
		REG_FUNC(getSecond);
	}

	NATIVECLASS_FUNC(getYear)
	{
		GETINSTANCE();
		return instance->_tm->tm_year + 1900;
	}

	NATIVECLASS_FUNC(getMonth)
	{
		GETINSTANCE();
		return instance->_tm->tm_mon + 1;
	}

	NATIVECLASS_FUNC(getDay)
	{
		GETINSTANCE();
		return instance->_tm->tm_mday;
	}

	NATIVECLASS_FUNC(getHour)
	{
		GETINSTANCE();
		return instance->_tm->tm_hour;
	}

	NATIVECLASS_FUNC(getMinute)
	{
		GETINSTANCE();
		return instance->_tm->tm_min;
	}

	NATIVECLASS_FUNC(getSecond)
	{
		GETINSTANCE();
		return instance->_tm->tm_sec;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Math
#endif

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
		REG_GET(PI);
		REG_GET(E);
		REG_FUNC(sgn);
		REG_FUNC(abs);
		REG_FUNC(sqrt);
		REG_FUNC(lg);
		REG_FUNC(ln);
		REG_FUNC(log);
		REG_FUNC(floor);
		REG_FUNC(ceil);
		REG_FUNC(round);
		REG_FUNC(sin);
		REG_FUNC(cos);
		REG_FUNC(tan);
		REG_FUNC(asin);
		REG_FUNC(acos);
		REG_FUNC(atan);
		REG_FUNC(random);
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
	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_FUNC(search);
		REG_FUNC(matchAll);
		REG_FUNC(getSubMatch);
		REG_FUNC(replaceFirst);
		REG_FUNC(replaceAll);
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
		if(!std::regex_search(tmp, m, instance->reg))
			RETURNDEFAULT;
		auto arr = new BKE_VarArray();
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
		if(!std::regex_search(src, m, instance->reg))
			RETURNDEFAULT;
		auto arr = new BKE_VarArray();
		for (auto i : m)
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

	void registerExtend(Parser *p)
	{
		//register inner classes and global functions
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(eval));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(time));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(clock));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(log));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(print));
		//BKE_VarClosure::Global()->addNativeFunction(QUICKFUNC(typeof));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(range));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(random));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(itoa));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(itoa2));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(char));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(hash));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(hash16));
#ifdef ENABLE_FILE
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(loadFile));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(saveFile));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(appendFile));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(evalFile));
#endif
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(toString));
		BKE_VarClosure::global()->addNativeFunction(QUICKFUNC(loadClass));
		//BKE_VarClosure::Global()->addNativeFunction(QUICKFUNC(int));

		//undefined
		BKE_VarClass *undefined = new BKE_VarClass(L"undefined");
		BKE_VarClass *num_class = new BKE_VarClass(L"number");
		BKE_VarClass *str_class = new BKE_VarClass(L"string");
		BKE_VarClass *arr_class = new BKE_VarClass(L"array");
		BKE_VarClass *dic_class = new BKE_VarClass(L"dictionary");
		BKE_VarClass *fun_class = new BKE_VarClass(L"function");
		BKE_VarClass *clo_class = new BKE_VarClass(L"closure");

		//undefined->addNativePropGet(QUICKGETTER(type));
		undefined->addNativeFunction(QUICKFUNC(equals));

		//num_class->addNativePropGet(QUICKGETTER(type));
		num_class->addNativeFunction(QUICKFUNC(equals));
		//num_class->innerCreateInstance=&Parser_Util::Native_number;

		//str_class->addNativePropGet(QUICKGETTER(type));
		str_class->addNativePropGet(QUICKGETTER(length));
		str_class->addNativePropGet(L"size", &Parser_Util::nativeGet_length);
		str_class->addNativeFunction(QUICKFUNC(substring));
		str_class->addNativeFunction(L"substr", &Parser_Util::nativeFunc_substring);
		str_class->addNativeFunction(QUICKFUNC(sprintf));
		str_class->addNativeFunction(QUICKFUNC(split));
		str_class->addNativeFunction(QUICKFUNC(indexOf));
		str_class->addNativeFunction(QUICKFUNC(lastIndexOf));
		str_class->addNativeFunction(QUICKFUNC(replace));
		str_class->addNativeFunction(QUICKFUNC(equals));
		str_class->addNativeFunction(QUICKFUNC(beginWith));
		str_class->addNativeFunction(QUICKFUNC(endWith));
		str_class->addNativeFunction(QUICKFUNC(clear));
		str_class->addNativeFunction(QUICKFUNC(toLowerCase));
		str_class->addNativeFunction(QUICKFUNC(toLowerCase));
		//str_class->innerCreateInstance=&Parser_Util::Native_string;

		//arr_class->addNativePropGet(QUICKGETTER(type));
		arr_class->addNativePropGet(QUICKGETTER(length));
		arr_class->addNativePropSet(QUICKSETTER(length));
		arr_class->addNativePropGet(L"size", &Parser_Util::nativeGet_length);
		arr_class->addNativePropSet(L"size", &Parser_Util::nativeSet_length);
		arr_class->addNativeFunction(QUICKFUNC(add));
		arr_class->addNativeFunction(QUICKFUNC(remove));
		arr_class->addNativeFunction(QUICKFUNC(erase));
		arr_class->addNativeFunction(QUICKFUNC(insert));
		arr_class->addNativeFunction(QUICKFUNC(find));
		arr_class->addNativeFunction(QUICKFUNC(join));
		arr_class->addNativeFunction(QUICKFUNC(concat));
		arr_class->addNativeFunction(QUICKFUNC(sort));
		arr_class->addNativeFunction(QUICKFUNC(clone));
		arr_class->addNativeFunction(QUICKFUNC(equals));
		arr_class->addNativeFunction(QUICKFUNC(clear));
		arr_class->innerCreateInstance = &Parser_Util::nativeFunc_array;
#ifdef ENABLE_FILE
		arr_class->addNativeFunction(QUICKFUNC(save));
		arr_class->addNativeFunction(QUICKFUNC(load));
#endif

		//dic_class->addNativePropGet(QUICKGETTER(type));
		dic_class->addNativePropGet(QUICKGETTER(length));
		dic_class->addNativePropGet(L"size", &Parser_Util::nativeGet_length);
		dic_class->addNativeFunction(QUICKFUNC(remove));
		dic_class->addNativeFunction(QUICKFUNC(erase));
		dic_class->addNativeFunction(QUICKFUNC(clone));
		dic_class->addNativeFunction(QUICKFUNC(find));
		dic_class->addNativeFunction(QUICKFUNC(toArray));
		dic_class->addNativeFunction(QUICKFUNC(equals));
		dic_class->addNativeFunction(QUICKFUNC(update));
		dic_class->addNativeFunction(QUICKFUNC(except));
		dic_class->addNativeFunction(QUICKFUNC(getKeyArray));
		dic_class->addNativeFunction(QUICKFUNC(getValueArray));
		dic_class->addNativeFunction(QUICKFUNC(sortKeyByValue));
		dic_class->addNativeFunction(QUICKFUNC(clear));
		dic_class->innerCreateInstance = &Parser_Util::nativeFunc_dictionary;
#ifdef ENABLE_FILE
		dic_class->addNativeFunction(QUICKFUNC(saveStruct));
#endif

		//fun_class->addNativePropGet(QUICKGETTER(type));
		fun_class->addNativeFunction(QUICKFUNC(equals));

		//clo_class->addNativePropGet(QUICKGETTER(type));
		clo_class->addNativeFunction(QUICKFUNC(equals));

		BKE_VarClosure::global()->setMember(L"undefined", undefined);
		BKE_VarClosure::global()->setMember(L"number", num_class);
		BKE_VarClosure::global()->setMember(L"string", str_class);
		BKE_VarClosure::global()->setMember(L"array", arr_class);
		BKE_VarClosure::global()->setMember(L"dictionary", dic_class);
		BKE_VarClosure::global()->setMember(L"function", fun_class);
		BKE_VarClosure::global()->setMember(L"closure", clo_class);

		//undefined->release();
		//num_class->release();
		//str_class->release();
		//arr_class->release();
		//dic_class->release();

		//othe native class
		p->registerClass(QUICKCLASS(Date));
		p->registerClass(QUICKCLASS(Math));
#ifdef HAS_REGEX
		p->registerClass(QUICKCLASS(Regex));
#endif
	}
}

