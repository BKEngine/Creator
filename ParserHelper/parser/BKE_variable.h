#pragma once

#include "utils.h"
#include "BKE_number.h"
#include "BKE_string.h"
#include <stdarg.h>
#include <initializer_list>
#include <set>
#include "BKE_array.h"
#include "BKE_hash.hpp"

#pragma push_macro("new")
#undef new

//#if PARSER_DEBUG==2
class GlobalMemoryPool : public BKE_hashmap<void *, bkplong, 12>
//#else
//class GlobalMemoryPool : public BKE_hashmap<void *, _dummy_class, 12>
//#endif
{
private:
	recursive_mutex mu;

public:
#if PARSER_DEBUG==2
	bkplong c;
#endif
	bool clearflag;

	GlobalMemoryPool()
	{
		__init_memorypool();
		clearflag = false;
#if PARSER_DEBUG==2
		c=0;
#endif
	};

	inline void lock(){ mu.lock(); }
	inline void unlock(){ mu.unlock(); }

	void insertKey(void *p, bkplong size)
	{
//#if PARSER_DEBUG==2
//		this->operator[] (p) = c++;
//#else
//		BKE_hashmap<void *, _dummy_class, 12>::insertKey(p);
//#endif
		operator[](p) = size;
	}

	void finalize();

	void purge();

	~GlobalMemoryPool()
	{
		purge();
	};
};

inline GlobalMemoryPool& MemoryPool()
{
	return *_globalStructures.globalMemoryPool;
}

//virtual base class for all Objects
class BKE_VarObject
{
protected:
	virtual ~BKE_VarObject(){};
	BKE_VarObject(VarType t):vt(t), ref(1){}

public:
	VarType vt;
	bkplong ref;

	
	virtual void release(){ if (--ref <= 0 
//#ifndef _DEBUG_MEM
		//&& !MemoryPool().clearflag
//#endif
		) delete this; }
	inline BKE_VarObject *addRef(){ ref++; return this; }

	inline void forceDelete(){ delete this; }

//#ifndef _DEBUG_MEM
	void* operator new (size_t size)
	{
		void *p;
		if (size <= 4 * SMALL)
			p = allocator_array()[(size + 3) / 4]->dynamic_allocate();
		else
			p = malloc(size);
		if (p)
		{
#if PARSER_MULTITHREAD
			MemoryPool().lock();
			MemoryPool().insertKey(p);
			MemoryPool().unlock();
#else
			MemoryPool().insertKey(p, size);
#endif
		}
		return p;
	};

	void* operator new (size_t size, const char *, int){ return operator new(size); };

	void operator delete (void *p)
	{
		//during clear stage, we only do destruct, do not free
		if (MemoryPool().clearflag)
			return;
		if (p)
		{
			//free(p);
#if PARSER_MULTITHREAD
			MemoryPool().lock();
#endif
			auto it = MemoryPool().find(p);
			if (it->second <= 4 * SMALL)
				allocator_array()[(it->second + 3) / 4]->dynamic_deallocate(p);
			else
				free(p);
			MemoryPool().erase(it);
#if PARSER_MULTITHREAD
		MemoryPool().unlock();
#endif
		}
	};

	void operator delete (void *p, const char *, int){ operator delete(p); };
//#endif
};


class BKE_VarObjectAutoReleaser
{
public:
	BKE_VarObject *obj;
	BKE_VarObjectAutoReleaser(BKE_VarObject * o){ obj = o; }
	~BKE_VarObjectAutoReleaser(){ obj->release(); }
	template<class T = BKE_VarObject *>
	operator T() const { static_assert(std::is_convertible<T, BKE_VarObject *>::value, ""); return (T)obj; }
};

template<class T>
T BKE_VarObjectReferencer(T o)
{
	static_assert(std::is_convertible<T, BKE_VarObject *>::value, "");
	return (T)o->addRef();
}

template<class T>
T BKE_VarObjectSafeReferencer(T o)
{
	static_assert(std::is_convertible<T, BKE_VarObject *>::value, "");
	if (o)
		return (T)o->addRef();
	return NULL;
}

class BKE_VarClosure;
class BKE_VarThis;
class BKE_Variable;
template <int T> class BKE_VarArrayTemplate;
typedef BKE_VarArrayTemplate<4> BKE_VarArray;
typedef BKE_Variable(*BKE_NativeFunction)(const BKE_Variable *self, const BKE_VarArray *paramarray, BKE_VarClosure *_this);

enum
{
	VARIABLE_VAR,
	TEMP_VAR,
	CONST_VAR,
	LOCK_TYPE,
};

class BKE_VariablePointer;
class BKE_VarDic;
class BKE_VarFunction;
class BKE_Variable
{
private:
	inline void clear()
	{
		if (!MemoryPool().clearflag && obj)
		{
			obj->release(); obj = NULL; 
		}
	};

public:
	VarType vt;

	BKE_Number num;
	BKE_String str;
	BKE_VarObject *obj;

	//set<BKE_VariablePointer*> reflection;

	bkplong is_var;

	inline ~BKE_Variable();

	//construct functions
	inline BKE_Variable():vt(VAR_NONE), obj(NULL), is_var(VARIABLE_VAR){}

	inline BKE_Variable(bkpshort i) : vt(VAR_NUM), num(i), obj(NULL), is_var(VARIABLE_VAR){}
	inline BKE_Variable(bkpushort i) : vt(VAR_NUM), num(i), obj(NULL), is_var(VARIABLE_VAR){}
	inline BKE_Variable(float i) : vt(VAR_NUM), num(i), obj(NULL), is_var(VARIABLE_VAR){}
	inline BKE_Variable(double i) : vt(VAR_NUM), num(i), obj(NULL), is_var(VARIABLE_VAR){}
	inline BKE_Variable(bkplonglong i) : vt(VAR_NUM), num(i), obj(NULL), is_var(VARIABLE_VAR){}
	inline BKE_Variable(bkplong i) : vt(VAR_NUM), num(i), obj(NULL), is_var(VARIABLE_VAR){}
	inline BKE_Variable(bkpulong i) : vt(VAR_NUM), num(i), obj(NULL), is_var(VARIABLE_VAR){}
	inline BKE_Variable(const BKE_Number &i) : vt(VAR_NUM), num(i), obj(NULL), is_var(VARIABLE_VAR){}

	inline BKE_Variable(const wchar_t *s) : vt(VAR_STR), str(s), obj(NULL), is_var(VARIABLE_VAR){}
	inline BKE_Variable(const wchar_t &s) : vt(VAR_STR), str(wstring(1, s), false), obj(NULL), is_var(VARIABLE_VAR){}
	inline BKE_Variable(const wstring &s) : vt(VAR_STR), str(s), obj(NULL), is_var(VARIABLE_VAR){}
	inline BKE_Variable(const BKE_String &s) : vt(VAR_STR), str(s), obj(NULL), is_var(VARIABLE_VAR){}

	inline BKE_Variable(BKE_VarObject *o){ obj = o;  if (!o)vt = VAR_NONE; else vt = o->vt; is_var = VARIABLE_VAR; }

	BKE_Variable(const BKE_Variable &v);

	inline BKE_Variable(BKE_Variable &&v)
	{
		vt = v.vt;
		num = std::move(v.num);
		str = std::move(v.str);
		obj = v.obj;
		v.obj = NULL;
		is_var = VARIABLE_VAR;
	}

	BKE_Variable(BKE_NativeFunction func);

	template<class T> BKE_Variable(initializer_list<T> list);

	//static constructor for dic and array
	static BKE_Variable array(int size = 0);
	static BKE_Variable dic();

	template<class Head, class ... Args>
	static BKE_Variable arrayWithObjects(const Head &h, Args... args);

	template<class Head>
	static BKE_Variable arrayWithObjects(const Head &h);

	template<class Head, class ... Args>
	static BKE_Variable arrayWithObjects(BKE_VarArray *a, const Head &h, Args... args);

	template<class Head>
	static BKE_Variable arrayWithObjects(BKE_VarArray *a, const Head &h);

	//throw
#define _throw(str)	{						\
		wstring str2 = L"变量";				\
		save(str2, false);					\
		str2 += L"\t:";						\
		str2+= str;							\
		throw Var_Except(str2 VAR_EXCEPT_EXT);				\
		}

	//variable and const
	inline void makeUnchangable(){ is_var = LOCK_TYPE; };
	inline void makeConst(){ is_var = CONST_VAR; };
	inline void makeVar(){ is_var = VARIABLE_VAR; };
	inline bool isVar() const { return is_var == VARIABLE_VAR; };
	inline bool isTemp() const { return is_var == TEMP_VAR; };
	inline bool isConst() const { return is_var == CONST_VAR; };
	inline bool canNotChange() const { return is_var >= CONST_VAR; };

	//saveStruct -- save to a wstring which can be direct load
	//format means to format result as krkr
	//or all the result will br put in one line
	//result will be append
	//indent is used for recursion
	void save(wstring &result, bool format = true, bkplong indent = 0) const;
	wstring save(bool format = true, bkplong indent = 0) const
	{
		wstring res;
		save(res, format, indent);
		return std::move(res);
	}
	bool saveToFile(const wstring &filename, bool format = true)
	{
		return BKE_writeFile(save(format), filename);
	}
	bool saveToFile(const string &filename, bool format = true)
	{
		return BKE_writeFile(save(format), filename);
	}
	static BKE_Variable readFromFile(const wstring &filename);

	inline wstring saveString(bool format = true) const
	{
		wstring s;
		save(s, format, 0);
		return std::move(s);
	}

	//type-conversion
	double asNumber() const;
	BKE_Number asBKENum() const;
	inline BKE_String asBKEStr() const
	{
		if (vt == VAR_STR)
			return str;
		return asString();
	}
	inline double toNumber() { if (vt == VAR_NUM) return num; double s = asNumber(); clear(); num = s; vt = VAR_NUM; return s; };
	inline bool isInteger() const { return vt == VAR_NUM && num.isInteger(); };
	bkplonglong asInteger() const;
	bkplonglong roundAsInteger() const;
	bool canBeNumber() const;
	inline bkplonglong toInteger(){ bkplonglong i = asInteger(); clear(); num = i; vt = VAR_NUM; return i; };
	inline bkplonglong roundToInteger(){ bkplonglong i = roundAsInteger(); clear(); num = i; vt = VAR_NUM; return i; }
	bool asBoolean() const;
	inline bool toBoolean(){ bool s = asBoolean(); clear(); num = s ? 1 : 0; vt = VAR_NUM; return s; };
	wstring asString() const;
	inline const wstring& toString(){ if (vt == VAR_STR) return str.getConstStr(); wstring s = asString(); clear(); str = s; vt = VAR_STR; return str.getConstStr(); };
	
	//已addRef
	BKE_VarThis *asThisClosure() const;
	//未addRef
	BKE_VarClosure *asClosure() const;
	BKE_VarArray *asArray() const;
	BKE_VarDic *asDic() const;
	BKE_VarFunction *asFunc() const;
	inline operator float() const{ return (float)asNumber(); };
	inline operator double() const{ return asNumber(); };
	inline operator bool() const{ return asBoolean(); };
	inline operator bkpshort() const{ return (bkpshort)asInteger(); };
	inline operator bkpushort() const{ return (bkpushort)asInteger(); };
	inline operator bkplong() const{ return (bkplong)asInteger(); };
	inline operator bkpulong() const{ return (bkpulong)asInteger(); };
	inline operator bkplonglong() const{ return asInteger(); };
	inline operator wstring() const{ return asString(); };
	inline operator BKE_Number() const{ return asNumber(); };
	inline operator BKE_String() const
	{
		if (vt == VAR_STR)
			return str;
		return asString();
	}

	inline int getType() const { return vt; };

	wstring getTypeString() const
	{
		static const wchar_t *t[] = { L"void", L"number", L"string", L"array", L"dictionary", L"function", L"property", L"closure", L"class", L"thisclosure" };
		return t[vt];
	}

	wstring getChineseTypeString() const
	{
		static const wchar_t *t[] = { L"空类型", L"数值类型", L"字符串类型", L"数组类型", L"字典类型", L"函数类型", L"属性类型", L"闭包类型", L"类类型", L"this闭包类型" };
		return t[vt];
	}

	BKE_String getTypeBKEString() const
	{
		//fixme
		static const wchar_t * t[] = { L"void", L"number", L"string", L"array", L"dictionary", L"function", L"property", L"closure", L"class", L"thisclosure" };
		return t[vt];
	}

	bool instanceOf(const BKE_String &str) const;

	inline bool isVoid() const{ return !vt; }

	inline bool equalToVoid() const
	{
		return isVoid() || (vt == VAR_NUM && num.zero()) || (vt == VAR_STR && str.isVoid());
	}

	BKE_Variable clone() const;

	inline void copyFrom(const BKE_Variable &v){ *this = v.clone(); }

	inline void forceSet(const BKE_Variable &v)
	{
		if (obj)
			obj->release();
		vt = v.vt;
		num = v.num;
		str = v.str;
		obj = v.obj;
		if (obj)
			obj->addRef();
	}

	//= operator
	BKE_Variable& operator = (bkpshort v);
	BKE_Variable& operator = (bkpushort v);
	inline BKE_Variable& operator = (bool v){ return operator = (v ? 1 : 0); };
	BKE_Variable& operator = (bkplong v);
	BKE_Variable& operator = (bkpulong v);
	BKE_Variable& operator = (bkplonglong v);
	BKE_Variable& operator = (double v);
	BKE_Variable& operator = (float v);
	BKE_Variable& operator = (const wstring &str);
	BKE_Variable& operator = (const wchar_t *str);
	BKE_Variable& operator = (const BKE_Number &v);
	BKE_Variable& operator = (const BKE_String &v);
	BKE_Variable& operator = (const BKE_Variable &v);
	BKE_Variable& operator = (BKE_Variable &&v);
	BKE_Variable& operator = (BKE_VarObject *v);
	BKE_Variable& operator = (BKE_NativeFunction func);

	BKE_Variable operator + (const BKE_Variable &v) const;
	BKE_Variable operator + (const wstring &v) const;
	BKE_Variable operator - (const BKE_Variable &v) const;
	inline BKE_Variable operator * (const BKE_Variable &v) const{ return asBKENum() * v.asBKENum(); };
	inline BKE_Variable operator / (const BKE_Variable &v) const{ return asBKENum() / v.asBKENum(); };
	inline BKE_Variable operator % (const BKE_Variable &v) const{ return asBKENum() % v.asBKENum(); };
	//^ as power
	inline BKE_Variable operator ^ (const BKE_Variable &v) const{ return asBKENum() ^ v.asBKENum(); };
	inline BKE_Variable& operator | (BKE_Variable &v)
	{
		return isVoid() ? v : *this;
	}
	inline const BKE_Variable& operator | (const BKE_Variable &v) const
	{
		return isVoid() ? v : *this;
	}
	inline BKE_Variable operator ! () const{ return asBoolean() ? 0 : 1; };
	//++a
	inline BKE_Variable& operator ++ (){ auto a = asNumber(); num = a + 1; return (*this = num); };
	//a++
	inline BKE_Variable operator ++ (int){ auto a = asNumber(); num = a + 1; return a; };
	//--a
	inline BKE_Variable& operator -- (){ auto a = asNumber(); num = a - 1; return (*this = num); };
	//a--
	inline BKE_Variable operator -- (int){ auto a = asNumber(); num = a - 1; return a; };
	BKE_Variable& operator += (const BKE_Variable &v);
	BKE_Variable& operator -= (const BKE_Variable &v);
	BKE_Variable& operator [] (const BKE_Variable &v) const;
	BKE_Variable& operator [] (const BKE_String &v) const;
	BKE_Variable& operator [] (const wstring &v) const;
	inline BKE_Variable& operator [] (const wchar_t *v) const { wstring vv(v); return operator [] (BKE_String(vv)); };
	BKE_Variable& operator [] (int v) const;
	BKE_Variable& operator [] (const BKE_Variable &v);
	BKE_Variable& operator [] (const BKE_String &v);
	BKE_Variable& operator [] (const wstring &v);
	inline BKE_Variable& operator [] (const wchar_t *v){ wstring vv(v); return operator [] (BKE_String(vv)); };
	BKE_Variable& operator [] (int v);
	bool operator == (const BKE_Variable &v) const;
	inline bool operator != (const BKE_Variable &v) const{ return !(*this == v); };
	bool operator < (const BKE_Variable &v) const;
	bool operator > (const BKE_Variable &v) const;
	inline bool operator <= (const BKE_Variable &v) const
	{
		return !(*this > v);
	}
	inline bool operator >= (const BKE_Variable &v) const
	{
		return !(*this < v);
	}
	BKE_Variable getMid(bkplong *start, bkplong *stop, bkplong step);
	BKE_Variable& dot(const BKE_String &funcname);
	BKE_Variable dotFunc(const BKE_String &funcname);
	BKE_Variable operator + (double v) const;
	BKE_Variable operator - (double v) const;
	inline BKE_Variable operator * (double v) const{ return asBKENum() * BKE_Number(v); };
	inline BKE_Variable operator / (double v) const{ return asBKENum() / BKE_Number(v); };
	inline BKE_Variable operator % (double v) const{ return asBKENum() % BKE_Number(v); };
	inline BKE_Variable operator ^ (double v) const{ return asBKENum() ^ BKE_Number(v); };
	inline BKE_Variable& operator *= (double v);
	inline BKE_Variable& operator /= (double v);
	inline BKE_Variable& operator %= (double v);
	inline BKE_Variable& operator ^= (double v);
	inline BKE_Variable& operator *= (const BKE_Variable &v){ return operator *=(v.asNumber()); };
	inline BKE_Variable& operator /= (const BKE_Variable &v){ return operator /=(v.asNumber()); };
	inline BKE_Variable& operator %= (const BKE_Variable &v){ return operator %=(v.asNumber()); };
	inline BKE_Variable& operator ^= (const BKE_Variable &v){ return operator ^=(v.asNumber()); };

	bool equals(const BKE_Variable &v) const;

	bkplong getCount() const;

	inline void setVoid()
	{
		clear();
		vt = VAR_NONE;;
	}

	void push_back(const BKE_Variable &v);

	BKE_Variable run(BKE_VarObject *o){ BKE_Variable tmp; if (o) tmp = o->addRef(); return run(tmp); }
	BKE_Variable run(BKE_Variable &self);

	template<typename... Args>
	BKE_Variable run(BKE_Variable &self, Args...args);

	template<typename... Args>
	BKE_Variable run(BKE_VarObject *o, Args...args);
private:
	template<typename Param, typename... Args>
	BKE_Variable _run(BKE_VarArray *paramarray, const Param &head, Args... rest);
	//end
	template<typename Param>
	BKE_Variable _run(BKE_VarArray *paramarray, const Param &head);
};

//extern template _BKE_allocator<sizeof(BKE_Variable)>* get_allocator();
//extern template class BKE_allocator<BKE_Variable>;

//handle continue, break and return
class Special_Except
{
public:
	enum error_type
	{
		CONTINUE,
		BREAK,
		RETURN
	}type;

	BKE_Variable res;

	Special_Except(error_type e, const BKE_Variable &v);
	Special_Except(error_type e, BKE_Variable &&v);
	Special_Except(error_type e){ type = e; };
	Special_Except(const Special_Except &s);
	Special_Except(Special_Except &&s);
	~Special_Except();
};

/*
class BKE_VariablePointer
{
	friend class BKE_Variable;
private:
	BKE_Variable *var;

	void invalid()
	{
		var = NULL;
	}

public:
	BKE_VariablePointer() :var(NULL){};
	BKE_VariablePointer(const BKE_Variable &v)
	{
		var = const_cast<BKE_Variable *>(&v);
		var->reflection.insert(this);
	}

	~BKE_VariablePointer()
	{
		release();
	}

	void release()
	{
		if (var)
			var->reflection.erase(this);
		var = NULL;
	}

	void pointTo(const BKE_Variable &v)
	{
		release();
		var = const_cast<BKE_Variable *>(&v);
		var->reflection.insert(this);
	}

	BKE_Variable * operator -> () const
	{
		return var;
	}

	BKE_Variable operator * () const
	{
		assert(var);
		return *var;
	}

	bool isValid() const
	{
		return var != nullptr;
	}

	bool setValue(const BKE_Variable &v) const
	{
		if (!var)
			return false;
		*var = v;
		return true;
	}
};
*/

inline BKE_Variable::~BKE_Variable()
{
	clear();
	//for (auto it : reflection)
	//	it->invalid();
};

//include array, dic, closure, func, prop, class
template <int T = 4>
class BKE_VarArrayTemplate :public BKE_VarObject
{
private:
	virtual ~BKE_VarArrayTemplate(){};

public:
	BKE_array<BKE_Variable, T> vararray;

	inline BKE_VarArrayTemplate() :BKE_VarObject(VAR_ARRAY){};
	BKE_VarArrayTemplate(std::initializer_list<BKE_Variable> l) :BKE_VarObject(VAR_ARRAY){ vararray.resize(l.size()); for (auto it = l.begin(); it != l.end(); it++) vararray[(bkplong)(it - l.begin())] = *it; }
	inline bkplong getCount() const
	{
		return vararray.size();
	};
	BKE_Variable& getMember(short index)
	{
		if (index<0)
			index += getCount();
		if (index<0)
			throw Var_Except(L"下标越界（过小）");
		if (index >= (bkplong)vararray.size())
		{
			vararray.resize(index + 1);
		}
		return vararray[index];
	};
	const BKE_Variable& getMember(short index) const
	{
		if (index<0)
			index += getCount();
		if (index<0)
			throw Var_Except(L"下标越界（过小）");
		if (index >= (bkplong)vararray.size())
		{
			const_cast<BKE_array<BKE_Variable, T> *>(&this->vararray)->resize(index + 1);
		}
		return vararray[index];
	};
	inline BKE_Variable& quickGetMember(short index) const
	{
		//if (index<0 || index >= getCount())
		//	throw Var_Except(L"下标越界");
		return vararray[index];
	};
	void insertMember(short index, const BKE_Variable &obj)
	{
		bkplong cnt = getCount();
		if (index<0)
			index += cnt;
		if (index<0)
			throw Var_Except(L"下标越界（过小）");
		if (index >= cnt)
		{
			setMember(index, obj);
			return;
		}
		vararray.insert(index, obj);
	}
	void setMember(short index, const BKE_Variable &obj)
	{
		if (index<0)
			index += getCount();
		if (index<0)
			throw Var_Except(L"下标越界（过小）");
		if (index >= vararray.size())
			vararray.resize(index + 1);
		vararray[index] = obj;
	}
	void setMember(short index, BKE_Variable &&obj)
	{
		if (index<0)
			index += getCount();
		if (index<0)
			throw Var_Except(L"下标越界（过小）");
		if (index >= vararray.size())
			vararray.resize(index + 1);
		vararray[index] = obj;
	}
	inline void quickSetMember(short index, const BKE_Variable &obj)
	{
		vararray[index] = obj;
	}
	void deleteMemberIndex(short index)
	{
		if (index<0)
			index += getCount();
		if (index < 0 || index >= vararray.size())
			return;
		vararray.erase(index);
	};
	inline void deleteMember(const BKE_Variable obj)
	{
		vararray.eraseValue(obj);
	};
	inline void clear()
	{
		vararray.clear();
	}
	template<int TT>
	void cloneFrom(const BKE_VarArrayTemplate<TT> *v)
	{
		if (v)
		{
			clear();
			vararray.resize(v->getCount());
			for (int i = 0; i<v->getCount(); i++)
			{
				vararray[i] = v->quickGetMember(i).clone();
			}
		}
	};
	template<int TT>
	void concat(const BKE_VarArrayTemplate<TT> *v)
	{
		if (v)
		{
			bkplong c = vararray.size();
			bkplong count = v->getCount();
			vararray.resize(c + count);
			for (int i = 0; i < count; i++)
			{
				vararray[c + i] = v->quickGetMember(i);
			}
		}
	};
	inline void pushMember(const BKE_Variable &v)
	{
		vararray.push_back(v);
	}
	inline void setLength(unsigned short l)
	{
		//if (l<0)
		//	return;
		vararray.resize(l);
	}
	template<int TT>
	bool equals(const BKE_VarArrayTemplate<TT> *v) const
	{
		bkplong s1 = getCount();
		bkplong s2 = v ? v->getCount() : 0;
		bkplong s = max(s1, s2);
		for (int i = 0; i < s; i++)
		{
			if (i < s1)
			{
				if (i < s2)
				{
					if (!vararray[i].equals(v->quickGetMember(i)))
						return false;
				}
				else
					if (!vararray[i].equalToVoid())
						return false;
			}
			else
				if (!v->quickGetMember(i).equalToVoid())
					return false;
		}
		return true;
	}
	int indexOf(const BKE_Variable &v) const
	{
		for (int i = 0; i < vararray.size();i++)
		{
			if (vararray[i] == v)
				return i;
		}
		return -1;
	}
};

typedef BKE_VarArrayTemplate<4> BKE_VarArray;

template<class T> BKE_Variable::BKE_Variable(initializer_list<T> list)
{
	auto arr = new BKE_VarArray();
	arr->vararray.reserve(list.size());
	for (auto &&i : list)
	{
		arr->vararray.push_back(i);
	}
	obj = arr;
	vt = VAR_ARRAY; 
	is_var = VARIABLE_VAR;
}

template<class Head, class ... Args>
BKE_Variable BKE_Variable::arrayWithObjects(const Head &h, Args... args)
{
	BKE_VarArray *a = new BKE_VarArray;
	a->pushMember(h);
	return arrayWithObjects(a, args...);
}

template<class Head>
BKE_Variable BKE_Variable::arrayWithObjects(const Head &h)
{
	BKE_VarArray *a = new BKE_VarArray;
	a->pushMember(h);
	return a;
}

template<class Head, class ... Args>
BKE_Variable BKE_Variable::arrayWithObjects(BKE_VarArray *a, const Head &h, Args... args)
{
	a->pushMember(h);
	return arrayWithObjects(a, args...);
}

template<class Head>
BKE_Variable BKE_Variable::arrayWithObjects(BKE_VarArray *a, const Head &h)
{
	a->pushMember(h);
	return a;
}

class BKE_VarDic :public BKE_VarObject
{
private:
	virtual ~BKE_VarDic(){};

public:
	BKE_hashmap<BKE_String, BKE_Variable> varmap;
	BKE_VarDic() :BKE_VarObject(VAR_DIC){}

	struct KeyValuePair
	{
		const BKE_String key;
		const BKE_Variable value;
		KeyValuePair(const BKE_String &k, const BKE_Variable &v) :key(k), value(v){}
	};

	BKE_VarDic(std::initializer_list<KeyValuePair> l) :BKE_VarObject(VAR_DIC)
	{
		for (auto it = l.begin(); it != l.end(); it++) 
			setMember(it->key, it->value);
	}	

	inline bkplong getCount() const
	{
		bkplong num = 0;
		auto it = varmap.begin();
		for (; it != varmap.end(); it++)
			if (!it->second.isVoid())
				num++;
		return num;
	};
	inline BKE_Variable& getMember(const BKE_String &key){ return varmap[key]; };
	inline const BKE_Variable& getMember(const BKE_String &key) const{ return const_cast<BKE_hashmap<BKE_String, BKE_Variable> *>(&this->varmap)->operator[](key); };
	inline void setMember(const BKE_String &key, const BKE_Variable &obj){ varmap.insert(key, obj); };
	inline void deleteMemberIndex(const BKE_String &key){ varmap.erase(key); };
	void deleteMember(const BKE_Variable &obj)
	{
		auto it = varmap.begin();
		while (it != varmap.end())
			if (it->second == obj)
				varmap.erase(it++);
			else
				it++;
	};
	inline void clear(){ varmap.clear(); };
	inline BKE_VarDic *cloneFrom(const BKE_VarDic *v)
	{
		if (v)
		{
			clear();
			for (auto it = v->varmap.begin(); it != v->varmap.end(); it++)
			{
				varmap[it->first] = it->second.clone();
			}
		}
		return this;
	}
	inline void update(const BKE_VarDic *v)
	{
		for (auto it = v->varmap.begin(); it != v->varmap.end(); it++)
			varmap[it->first] = it->second;
	}
	inline void except(const BKE_VarDic *v)
	{
		for (auto it = v->varmap.begin(); it != v->varmap.end(); it++)
			varmap.erase(it->first);
	}
	BKE_Variable toArray()
	{
		BKE_VarArray *arr = new BKE_VarArray();
		for (auto it = varmap.begin(); it != varmap.end(); it++)
		{
			arr->pushMember(it->first);
			arr->pushMember(it->second);
		}
		return arr;
	};

	inline bool isVoidDic() const
	{
		for (auto it = varmap.begin(); it != varmap.end(); it++)
		{
			if (!it->second.isVoid())
				return false;
		}
		return true;
	};

	bool equals(const BKE_VarDic *v) const
	{
		if (!v)
			return isVoidDic();
		BKE_hashmap<BKE_String, BKE_Variable> varbackup(varmap);
		for (auto it = v->varmap.begin(); it != v->varmap.end(); it++)
		{
			if (it->second.equalToVoid())
				continue;
			auto ii = varbackup.find(it->first);
			if (ii == varbackup.end() || !ii->second.equals(it->second))
				return false;
			varbackup.erase(it->first);
		}
		for (auto it = varbackup.begin(); it != varbackup.end(); it++)
		{
			if (!it->second.equalToVoid())
				return false;
		}
		return true;
	}
};

class BKE_VarClass;

class BKE_VarClosure :public BKE_VarObject
{
	friend struct GlobalStructures;
protected:
	inline BKE_Variable& quickGetMember(const BKE_String &key)
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
			return it->second;
		return parent->getMember(key);
	}
	virtual ~BKE_VarClosure()
	{
		if (!MemoryPool().clearflag && withvar)
			withvar->release();
	};
public:
	int extraref;
	BKE_hashmap<BKE_String, BKE_Variable> varmap;
	BKE_VarClosure *parent;
	BKE_VarObject *withvar;
	virtual void release()
	{
		if (extraref > 0 && ref - 1 == extraref)
			varmap.clear();
		if (--ref <= 0 
//#ifndef _DEBUG_MEM
			//&& !MemoryPool().clearflag
//#endif
			)
			delete this;
	};
	inline BKE_VarClosure(const BKE_VarClosure *p = NULL):BKE_VarObject(VAR_CLO), extraref(0){ parent = const_cast<BKE_VarClosure *>(p); withvar = p ? BKE_VarObjectSafeReferencer(p->withvar) : NULL; };
	inline void clear()
	{
		varmap.clear();
	}
	BKE_VarObject* getWithVar() const
	{
		if (withvar)
			return withvar;
		if (parent)
			return parent->withvar;
		return NULL;
	}
	virtual bool hasMember(const BKE_String &key) const
	{
		if (varmap.find(key) != varmap.end())
			return true;
		if (parent)
			return parent->hasMember(key);
		return false;
	}
	virtual bool hasMember(const BKE_String &key, BKE_Variable **var) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
		{
			*var = &it->second;
			return true;
		}
		if (parent)
			return parent->hasMember(key, var);
		return false;
	}
	BKE_Variable& getMember(const BKE_String &key)
	{
		//if (key[0] == L'$')
		//	return BKE_VarClosure::Global()->getMember(key.substr(1));
		//if (key[0] == L'#')
		//	return varmap[key.substr(1)];
		BKE_Variable *var;
		bool res = hasMember(key, &var);
		if (!res)
			return varmap[key];
		return *var;
	}
	inline BKE_Variable& setMember(const BKE_String &key, const BKE_Variable &obj){ return varmap.insert(key, obj); };
	inline void setConstMember(const BKE_String &key, const BKE_Variable &obj)
	{
		BKE_Variable &var = varmap[key];
		var = obj;
		var.makeConst();
	};
	inline void setConstVar(const BKE_String &key, const BKE_Variable &obj)
	{
		BKE_Variable &var = varmap[key];
		var = obj;
		var.makeUnchangable();
	};
	inline void deleteMemberIndex(const BKE_String &key){ varmap.erase(key); };
	void deleteMember(const BKE_Variable obj)
	{
		auto it = varmap.begin();
		while (it != varmap.end())
			if (it->second == obj)
				varmap.erase(it++);
			else
				it++;
	};
	bkplong getNonVoidCount() const
	{
		bkplong num = 0;
		auto it = varmap.begin();
		for (; it != varmap.end(); it++)
			if (!it->second.isVoid())
				num++;
		return num;
	};
	inline static BKE_VarClosure* global()
	{
		return _globalStructures.globalClosure;
	};
	inline void addNativeFunction(const BKE_String &key, BKE_NativeFunction func);
	void addNativePropGet(const BKE_String &key, BKE_NativeFunction func);
	void addNativePropSet(const BKE_String &key, BKE_NativeFunction func);
	BKE_VarClass *getThisClosure();
};

//copy is banned
class BKE_VarClass;
class BKE_VarThis :public BKE_VarObject
{
private:
	virtual ~BKE_VarThis(){};

public:
	VarType _vt;
	union
	{
		BKE_VarClosure *clo;
		BKE_VarClass *cla;
	};

	BKE_VarThis() = delete;

	inline BKE_VarThis(BKE_VarClosure *c) :BKE_VarObject(VAR_THIS), _vt(VAR_CLO), clo(c){}

	inline BKE_VarThis(BKE_VarClass *c) : BKE_VarObject(VAR_THIS), _vt(VAR_CLASS), cla(c){}

	inline bool hasMember(const BKE_String &key) const;

	inline bool hasMember(const BKE_String &key, BKE_Variable **var) const;

	inline BKE_Variable& getMember(const BKE_String &key);

	inline void setMember(const BKE_String &key, const BKE_Variable &v);

	inline void deleteMemberIndex(const BKE_String &key);

	void deleteMember(const BKE_Variable obj);

	bkplong getNonVoidCount() const;
};

class BKE_bytree;

class BKE_FunctionCode :public BKE_VarObject
{
private:
	virtual ~BKE_FunctionCode();
public:
	BKE_NativeFunction native;
	BKE_bytree *code;

	inline BKE_FunctionCode():BKE_VarObject(VAR_FUNCCODE_P), native(NULL), code(NULL){};
	inline BKE_FunctionCode(BKE_NativeFunction func) :BKE_VarObject(VAR_FUNCCODE_P), native(func), code(NULL){};
	BKE_FunctionCode(BKE_bytree *code);
	BKE_Variable run(BKE_Variable *self, BKE_VarArray *paramarray, BKE_VarClosure *_this) const;
	//inline void release(){ ref--; if (ref <= 0)delete this; };
};

class BKE_VarFunction :public BKE_VarObject
{
	friend class Parser;
	friend class BKE_Variable;
	friend class BKE_VarClass;
private:
	BKE_FunctionCode *func;
	BKE_Variable* self;
	BKE_VarClosure *closure;
	BKE_String name;

	virtual ~BKE_VarFunction()
	{
		if (!MemoryPool().clearflag)
		{
			func->release();
			closure->extraref--;
			closure->release();
		}
	}
	inline BKE_VarClosure* getClo()
	{
		return closure;
	}
public:

#ifdef PARSER_DEBUG
	vector<bkplong> linestartpos;
	wstring rawexp;
#endif
	vector<BKE_String> paramnames;
	BKE_hashmap<BKE_String, BKE_Variable, 4> initials;

	BKE_VarFunction() = delete;
	inline BKE_VarFunction(BKE_NativeFunction fun, BKE_Variable *_self = NULL) :BKE_VarObject(VAR_FUNC){ func = new BKE_FunctionCode(fun); self = _self; closure = (BKE_VarClosure *)BKE_VarClosure::global()->addRef(); closure->extraref++; };
	inline BKE_VarFunction(BKE_bytree *tree, BKE_Variable *_self = NULL) :BKE_VarObject(VAR_FUNC){ func = new BKE_FunctionCode(tree); self = _self; closure = (BKE_VarClosure *)BKE_VarClosure::global()->addRef(); closure->extraref++; };
	inline BKE_VarFunction(const BKE_VarFunction &f, BKE_VarClosure *c = BKE_VarClosure::global(), BKE_Variable *_self = NULL) :BKE_VarObject(VAR_FUNC){ func = (BKE_FunctionCode*)f.func->addRef(); initials = f.initials; paramnames = f.paramnames; if (_self)self = _self; else self = f.self; closure = c; c->addRef(); closure->extraref++; };
	inline BKE_Variable run(BKE_VarArray *params)
	{
		if (func->native)
			return (func->native)(self, params, getClo());
		BKE_VarClosure *clo = new BKE_VarClosure(getClo());
		BKE_VarObjectAutoReleaser cc(clo);
		for (auto i : initials)
		{
			clo->setMember(i.first, i.second);
		}
		int j = 0;
		int pcount = params ? params->getCount() : 0;
		for (int i = 0; i<(bkplong)paramnames.size(); i++)
		{
			if (j >= pcount)
				break;
			if (paramnames[i][0] != L'*')
			{
				clo->setMember(paramnames[i], params->quickGetMember(j));
				j++;
			}
			else
			{
				wstring name = paramnames[i].substr(1);
				BKE_VarArray *arr = new BKE_VarArray();
				if (params)
					for (; j < pcount; j++)
						arr->pushMember(params->quickGetMember(j));
				clo->setMember(name, arr);
			}
		}
		return func->run(self, params, clo);
	}
	//used in parser
	BKE_Variable run(const BKE_bytree *tr, BKE_VarClosure *_tr);
	inline void setSelf(BKE_Variable &v){ self = &v; };
	inline void setClosure(BKE_VarClosure *c){ closure->extraref--;  closure->release(); closure = c; c->addRef(); c->extraref++; }
	inline bool isNativeFunction(){ return func->native != NULL; };
	//inline void release(){ ref--; if (ref <= 0)delete this; };
};

inline void BKE_VarClosure::addNativeFunction(const BKE_String &key, BKE_NativeFunction func)
{
	BKE_VarFunction *f = new BKE_VarFunction(func);
	varmap[key] = f;
}


class BKE_VarProp :public BKE_VarObject
{
private:
	BKE_FunctionCode *funcget;
	BKE_FunctionCode *funcset;
	BKE_Variable *self;
	BKE_VarClosure *closure;
	BKE_String setparam;

	virtual ~BKE_VarProp()
	{
		if (funcget)
			funcget->release();
		if (funcset)
			funcset->release();
	}

	inline BKE_VarClosure *getClo() const
	{
		return closure ? closure : (self ? self->asClosure() : NULL);
	}
public:
	//BKE_VarProp(BKE_Variable *_self/* = NULL*/, BKE_NativeFunction get = NULL, BKE_NativeFunction set = NULL)
	//{
	//	funcget = funcset = NULL;
	//	if (get)
	//		funcget = new BKE_FunctionCode(get);
	//	if (set)
	//		funcset = new BKE_FunctionCode(set);
	//	vt = VAR_PROP;
	//	self = _self;
	//}
	BKE_VarProp(BKE_VarClosure *clo = NULL, BKE_NativeFunction get = NULL, BKE_NativeFunction set = NULL)
		:BKE_VarObject(VAR_PROP)
	{
		funcget = funcset = NULL;
		if (get)
			funcget = new BKE_FunctionCode(get);
		if (set)
			funcset = new BKE_FunctionCode(set);
		self = NULL;
		closure = clo;
	}
	BKE_VarProp(const BKE_VarProp &p, BKE_VarClosure *c = NULL, BKE_Variable *_self = NULL)
		:BKE_VarObject(VAR_PROP)
	{
		funcget = p.funcget;
		funcset = p.funcset;
		if (funcget)
			funcget->addRef();
		if (funcset)
			funcset->addRef();
		if (_self)
			self = _self;
		else
			self = p.self;
		closure = c;
	}
	inline void addPropGet(BKE_NativeFunction get){ if (funcget)funcget->release(); funcget = new BKE_FunctionCode(get); }
	inline void addPropGet(BKE_bytree *get){ if (funcget)funcget->release(); funcget = new BKE_FunctionCode(get); }
	inline void addPropSet(BKE_NativeFunction set){ if (funcset)funcset->release(); funcset = new BKE_FunctionCode(set); }
	inline void addPropSet(const BKE_String &setparam, BKE_bytree *set){ if (funcset)funcset->release(); funcset = new BKE_FunctionCode(set); this->setparam = setparam; }
	inline bool hasGet() const { return !!funcget; }
	inline bool hasSet() const { return !!funcset; }
	inline void setSelf(const BKE_Variable &v){ self = (BKE_Variable *)&v; };
	inline void setClosure(const BKE_VarClosure *c){ closure = (BKE_VarClosure*)c; }

	inline BKE_Variable get() const
	{
		if (funcget)
		{
			BKE_Variable v = funcget->run(self, NULL, getClo());
			if (v.getType() == VAR_PROP)
			{
				if (v.obj == this)
					return BKE_Variable();		//阻止死循环
				else
					return ((BKE_VarProp*)v.obj)->get();
			}
			return v;
		}
		return BKE_Variable();
	};
	inline void set(const BKE_Variable &v) const
	{
		if (funcset)
		{
			BKE_Variable tmp = BKE_Variable::arrayWithObjects(v);
			BKE_VarClosure *clo = getClo();
			if (!setparam.empty() && clo)
				clo->setMember(setparam, v);
			funcset->run(self, static_cast<BKE_VarArray*>(tmp.obj), clo);
		}
	}
};

inline void BKE_VarClosure::addNativePropGet(const BKE_String &key, BKE_NativeFunction func)
{
	BKE_Variable &var = varmap[key];
	if (var.getType() != VAR_PROP)
	{
		var = new BKE_VarProp(this);
	}
	static_cast<BKE_VarProp*>(var.obj)->addPropGet(func);
};

inline void BKE_VarClosure::addNativePropSet(const BKE_String &key, BKE_NativeFunction func)
{
	BKE_Variable &var = varmap[key];
	if (var.getType() != VAR_PROP)
	{
		var = new BKE_VarProp(this);
	}
	static_cast<BKE_VarProp*>(var.obj)->addPropSet(func);
};

class BKE_VarClass;

class BKE_NativeClass
{
public:
	BKE_VarClass *_class;
	BKE_NativeClass() = delete;
	virtual ~BKE_NativeClass(){}
	inline BKE_NativeClass(const wchar_t *name) : _class(NULL){ BKE_UNUSED_PARAM(name); }
	virtual void nativeInit(const wstring &name) = 0;
	virtual BKE_Variable nativeSave(){ return BKE_Variable(); }
	virtual void nativeLoad(const BKE_Variable &var){}
	//*_class=self
	virtual BKE_NativeClass* nativeCreateNew(const BKE_VarClass *self, const BKE_VarArray *paramarray) = 0;
};

class Parser;
namespace Parser_Util{
	void registerExtend(Parser *p);
}

class BKE_VarClass :public BKE_VarClosure
{
	friend void Parser_Util::registerExtend(Parser *);
	friend class Parser;
private:
	bool finalized;
	bool delayrelease;

	virtual void release()
	{
		if (delayrelease)
			return;
		if (extraref > 0 && ref - 1 == extraref)
		{
			finalize();
			for (auto &&it : varmap)
			{
				if (it.second.getType() == VAR_FUNC)
					((BKE_VarFunction*)it.second.obj)->setClosure(BKE_VarClosure::global());
				else if (it.second.getType() == VAR_PROP)
					((BKE_VarProp*)it.second.obj)->setClosure(BKE_VarClosure::global());
			}
			varmap.clear();
		}
		if (--ref <= 0)
		{
			delete this;
		}
	}

	void check(BKE_VarClass *cla) const
	{
		if (cla->isInstanceof(classname))
			throw Var_Except(L"类定义存在循环定义，" + cla->classname.getConstStr() + L"已经是" + classname.getConstStr() + L"的子类。");
		if (native && cla->native)
			throw Var_Except(L"目前不支持从多个Native类继承。");
	}

	virtual ~BKE_VarClass()
	{
		if (native)
			delete native;
		if (!MemoryPool().clearflag)
		{
			for (int i = 0; i < parents.size(); i++)
				parents[i]->release();
			if (!isdef)
				parent->release();
		}
	};
	BKE_NativeFunction innerCreateInstance;

public:
	BKE_NativeClass *native;
	BKE_String classname;
	bool isdef;
	BKE_VarClass *defclass;
	bool cannotcreate;
	BKE_array<BKE_VarClass *> parents;
	//staticvar合并进varmap
//	BKE_hashmap<BKE_String, BKE_Variable> staticvar;//only use when isdef is true
	BKE_hashmap<BKE_String, BKE_Variable> classvar;//单独拿出来防止被类名.变量名引用到

	BKE_VarClass() = delete;

	//def class
	inline BKE_VarClass(const BKE_String &name, BKE_VarClosure *context = BKE_VarClosure::global()) :BKE_VarClosure(context)
	{
		innerCreateInstance = NULL;
		native = NULL;
		defclass = NULL;
		classname = name;
		vt = VAR_CLASS;
		isdef = true;
		//_this = new BKE_VarThis(this);
		cannotcreate = false;
		finalized = false;
		delayrelease = false;
	}
	//single inherit class
	inline BKE_VarClass(const BKE_String &name, BKE_VarClass *parent, BKE_VarClosure *context = BKE_VarClosure::global()) :BKE_VarClosure(context)
	{
		innerCreateInstance = NULL;
		native = NULL;
		defclass = NULL;
		classname = name;
		vt = VAR_CLASS;
		isdef = true;
		check(parent);
		parent->addRef();
		for (auto &&it : parent->classvar)
			classvar[it.first] = it.second.clone();
		//for (auto &&it : parent->staticvar)
		//	staticvar[it.first] = it.second.clone();
		parents.push_back(parent);
		//_this->release();
		//_this = new BKE_VarThis(this);
		cannotcreate = false;
		for (auto it : parent->varmap)
		{
			if (it.second.getType() == VAR_FUNC)
			{
				varmap[it.first].forceSet(new BKE_VarFunction(*(BKE_VarFunction*)(it.second.obj), this));
			}
			else if (it.second.getType() == VAR_PROP)
			{
				varmap[it.first].forceSet(new BKE_VarProp(*(BKE_VarProp*)(it.second.obj), this));
			}
			else
			{
				varmap[it.first].forceSet(it.second.clone());
			}
		}
		if (parent->native)
			native = parent->native->nativeCreateNew(this, NULL);
		finalized = false;
		delayrelease = false;
	}
	//multi inherit class
	inline BKE_VarClass(const BKE_String &name, const BKE_array<BKE_VarClass *> &parent, BKE_VarClosure *context = BKE_VarClosure::global()) :BKE_VarClosure(context)
	{
		innerCreateInstance = NULL;
		native = NULL;
		defclass = NULL;
		classname = name;
		vt = VAR_CLASS;
		isdef = true;
		cannotcreate = false;
		for (int i = 0; i < parent.size(); i++)
		{
			check(parent[i]);
			parents.push_back(parent[i]);
			parent[i]->addRef();
			for (auto &&it : parent[i]->classvar)
				classvar[it.first] = it.second.clone();
			for (auto it : parent[i]->varmap)
			{
				if (it.second.getType() == VAR_FUNC)
				{
					varmap[it.first].forceSet(new BKE_VarFunction(*(BKE_VarFunction*)(it.second.obj), this));
				}
				else if (it.second.getType() == VAR_PROP)
				{
					varmap[it.first].forceSet(new BKE_VarProp(*(BKE_VarProp*)(it.second.obj), this));
				}
				else
				{
					varmap[it.first].forceSet(it.second.clone());
				}
			}
			//for (auto &&it : parent[i]->staticvar)
			//	staticvar[it.first] = it.second.clone();
			if (parents[i]->native)
				native = parent[i]->native->nativeCreateNew(this, NULL);
		}
		finalized = false;
		delayrelease = false;
	}
	//instance
	inline BKE_VarClass(BKE_VarClass *parent, BKE_NativeClass *na = NULL) :BKE_VarClosure(parent)
	{
		innerCreateInstance = NULL;
		classname = parent->classname;
		vt = VAR_CLASS;
		isdef = false;
		native = na;
		defclass = parent;
		parent->addRef();
		for (auto &&it : parent->classvar)
			varmap[it.first] = it.second.clone();
		//_this->release();
		//_this = new BKE_VarThis(this);
		cannotcreate = true;
		for (auto it : parent->varmap)
		{
			if (it.second.getType() == VAR_FUNC)
			{
				varmap[it.first].forceSet(new BKE_VarFunction(*(BKE_VarFunction*)(it.second.obj), this));
			}
			else if (it.second.getType() == VAR_PROP)
			{
				varmap[it.first].forceSet(new BKE_VarProp(*(BKE_VarProp*)(it.second.obj), this));
			}
			//not copy static varS
		}
		finalized = false;
		delayrelease = false;
	};

	void finalize()
	{
		if (!finalized && !isdef && !MemoryPool().clearflag)
		{
			BKE_Variable *var;
			finalized = true;
			static BKE_String s_finalize(L"finalize");
			if (hasClassMember(s_finalize, &var))
			{
				delayrelease = true;
				auto clo = static_cast<BKE_VarFunction*>(var->obj)->closure;
				static_cast<BKE_VarFunction*>(var->obj)->closure = this;
				static_cast<BKE_VarFunction*>(var->obj)->run(NULL);
				static_cast<BKE_VarFunction*>(var->obj)->closure = clo;
				delayrelease = false;
			}
		}
	}

	inline void addNativeFunction(const BKE_String &key, BKE_NativeFunction func)
	{
		auto f = new BKE_VarFunction(func);
		f->setClosure(this);
		varmap[key] = f;
	};

	void addNativePropGet(const BKE_String &key, BKE_NativeFunction func)
	{
		BKE_Variable &var = varmap[key];
		if (var.getType() != VAR_PROP)
		{
			var = new BKE_VarProp(this);
		}
		static_cast<BKE_VarProp*>(var.obj)->addPropGet(func);
	};
	void addNativePropSet(const BKE_String &key, BKE_NativeFunction func)
	{
		BKE_Variable &var = varmap[key];
		if (var.getType() != VAR_PROP)
		{
			var = new BKE_VarProp(this);
		}
		static_cast<BKE_VarProp*>(var.obj)->addPropSet(func);
	};
	inline void addStaticMember(const wstring &key, const BKE_Variable &var){ varmap.insert(key, var); };

	inline bool hasClassMember(const BKE_String &key) const
	{
		if (varmap.find(key) != varmap.end())
			return true;
		if (!isdef)
			return static_cast<BKE_VarClass*>(parent)->hasClassMember(key);
		for (int i = parents.size() - 1; i >= 0; i--)
		{
			if (parents[i]->hasClassMember(key))
				return true;
		}
		return false;
	}

	inline bool hasClassMember(const BKE_String &key, BKE_Variable **var) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
		{
			(*var) = &it->second;
			return true;
		}
		if (!isdef)
			return static_cast<BKE_VarClass*>(parent)->hasClassMember(key, var);
		auto p = dynamic_cast<BKE_VarClass*>(parent);
		for (int i = parents.size() - 1; i >= 0; i--)
		{
			if (parents[i]->hasClassMember(key, var))
				return true;
		}
		return false;
	}

	inline BKE_Variable& getClassMember(const BKE_String &key)
	{
		//if (isdef && !hasMember(key))
		//	throw Var_Except(L"类" + classname.getConstStr() + L"不存在成员" + key.getConstStr());
		//return BKE_VarClosure::getMember(key);
		BKE_Variable *var;
		if (hasClassMember(key, &var))
			return *var;
		else
			return varmap[key];
	}

	virtual bool hasMember(const BKE_String &key) const override
	{
		if (!hasClassMember(key))
		{
			//then we find it on current context
			return static_cast<BKE_VarClosure*>(parent)->hasMember(key);
		}
		return true;
	}
	virtual bool hasMember(const BKE_String &key, BKE_Variable **var) const override
	{
		if (!hasClassMember(key, var))
		{
			//then we find it on current context
			return static_cast<BKE_VarClosure*>(parent)->hasMember(key, var);
		}
		return true;
	}

	inline BKE_Variable getSuperMember(const BKE_String &key)
	{
		if (!isdef)
			return defclass->getSuperMember(key);
		if (parents.size() == 0)
			throw Var_Except(L"类" + classname.getConstStr() + L"不存在父类，super无意义。");
		if (parents.size() > 1)
			throw Var_Except(L"类" + classname.getConstStr() + L"有多个父类，super有歧义。");
		BKE_Variable *var;
		if (parents[0]->hasClassMember(key, &var))
		{
			if (var->getType() == VAR_FUNC)
			{
				return new BKE_VarFunction(*(BKE_VarFunction*)(var->obj), this);
			}
			if (var->getType() == VAR_PROP)
			{
				return new BKE_VarProp(*(BKE_VarProp*)(var->obj), this);
			}
			throw Var_Except(L"类" + classname.getConstStr() + L"的父类" + parents[0]->classname.getConstStr() + L"的成员" + key.getConstStr()+L"不是函数或属性，super只能取父类的函数或属性");
		}
		else
			throw Var_Except(L"类" + classname.getConstStr() + L"的父类" + parents[0]->classname.getConstStr() + L"不存在成员" + key.getConstStr());
	}

	void deleteThisMember(const BKE_String &key)
	{
		varmap.erase(key);
	}

	void getAllConstructors(BKE_array<BKE_VarFunction*> *cons, BKE_VarClass *dst)
	{
		for (int i = 0; i < parents.size(); i++)
			parents[i]->getAllConstructors(cons, dst);
		auto it = dst->varmap.find(classname);
		if (it != dst->varmap.end())
		{
			BKE_Variable &var = it->second;
			if (var.getType() == VAR_FUNC)
			{
				cons->push_back(static_cast<BKE_VarFunction*>(var.obj));
			}
		}
	}

	void construct(BKE_VarArray *paramarray, BKE_VarClass *_this)
	{
		if (parent != BKE_VarClosure::global())
			static_cast<BKE_VarClass*>(parent)->construct(paramarray, _this);
		if (native)
		{
			//override parent class's nativeclass
			if (_this->native)
				delete _this->native;
			_this->native = this->native->nativeCreateNew(_this, paramarray);
			if (!_this->native)
			{
				_this->release();
				throw Var_Except(L"不允许创建该类（" + classname.getConstStr() + L"）的实例");
			}
		}
		//else
		{
			BKE_array<BKE_VarFunction*> cons;
			getAllConstructors(&cons, _this);
			for (int i = 0; i < cons.size(); i++)
				cons[i]->run(paramarray);
			//auto it = _this->varmap.find(classname);
			//if (it != _this->varmap.end())
			//{
			//	BKE_Variable &var = it->second;
			//	if (var.getType() == VAR_FUNC)
			//	{
			//		static_cast<BKE_VarFunction*>(var.obj)->run(paramarray);
			//	}
			//}
		}
	}

	BKE_Variable createInstance(BKE_VarArray *paramarray)
	{
		if (!isdef)
		{
			throw Var_Except(L"该对象已经是实例");
		}
		if (innerCreateInstance != NULL)
		{
			return (*innerCreateInstance)(NULL, paramarray, (BKE_VarClosure*)static_cast<const BKE_VarClosure*>(this));
		}
		if (cannotcreate)
			throw Var_Except(L"不允许创建该类（" + classname.getConstStr() + L"）的实例");
		BKE_VarClass* cla = new BKE_VarClass(this);
		construct(paramarray, cla);
		return cla;
	}

	inline bool isInstance() const { return !isdef; }

	inline bool isInstanceof(const BKE_String &name) const
	{
		if (classname == name)
			return true;
		if (!isdef)
			return static_cast<BKE_VarClass*>(defclass)->isInstanceof(name);
		for (int i = 0; i < parents.size();i++)
		{
			if (parents[i]->isInstanceof(name))
				return true;
		}
		return false;
	}
};

inline BKE_VarClass *BKE_VarClosure::getThisClosure()
{
	auto p = this;
	while (p && !dynamic_cast<BKE_VarClass*>(p))
		p = p->parent;
	if (p)
		return dynamic_cast<BKE_VarClass*>(p);
	else
		return NULL;
}

//functions in BKE_VarThis
inline bool BKE_VarThis::hasMember(const BKE_String &key) const
{
	if (_vt == VAR_CLO)
		return clo->varmap.find(key) != clo->varmap.end();
	else
		return cla->hasClassMember(key);
}

inline bool BKE_VarThis::hasMember(const BKE_String &key, BKE_Variable **var) const
{
	if (_vt == VAR_CLO)
	{
		auto it = clo->varmap.find(key);
		if (it == clo->varmap.end())
			return false;
		(*var) = &it->second;
		return true;
	}
	else
	{
		return cla->hasClassMember(key, var);
	}
}

inline BKE_Variable& BKE_VarThis::getMember(const BKE_String &key)
{
	if (_vt == VAR_CLO)
		return clo->varmap[key];
	else
		return cla->getClassMember(key);
}

inline void BKE_VarThis::setMember(const BKE_String &key, const BKE_Variable &v)
{
	if (_vt == VAR_CLO)
		clo->varmap.insert(key, v);
	else
		cla->varmap.insert(key, v);
}

inline void BKE_VarThis::deleteMemberIndex(const BKE_String &key)
{
	if (_vt == VAR_CLO)
		clo->varmap.erase(key);
	else
		cla->varmap.erase(key);
};

inline BKE_Variable& BKE_Variable::operator *= (double v)
{
	if (getType() == VAR_PROP)
	{
		auto &&p = static_cast<BKE_VarProp*>(obj);
		if (p->hasGet() && p->hasSet())
			p->set(p->get() * v);
		return *this;
	}
	toNumber(); num *= v; return *this;
};
inline BKE_Variable& BKE_Variable::operator /= (double v)
{
	if (getType() == VAR_PROP)
	{
		auto &&p = static_cast<BKE_VarProp*>(obj);
		if (p->hasGet() && p->hasSet())
			p->set(p->get() / v);
		return *this;
	}
	toNumber(); num /= v; return *this;
};
inline BKE_Variable& BKE_Variable::operator %= (double v)
{
	if (getType() == VAR_PROP)
	{
		auto &&p = static_cast<BKE_VarProp*>(obj);
		if (p->hasGet() && p->hasSet())
			p->set(p->get() % v);
		return *this;
	}
	toNumber(); num %= v; return *this;
};
inline BKE_Variable& BKE_Variable::operator ^= (double v)
{
	if (getType() == VAR_PROP)
	{
		auto &&p = static_cast<BKE_VarProp*>(obj);
		if (p->hasGet() && p->hasSet())
			p->set(p->get() ^ v);
		return *this;
	}
	toNumber(); num ^= v; return *this;
};

template<typename... Args>
BKE_Variable BKE_Variable::run(BKE_Variable &self, Args...args)
{
	if (getType() != VAR_FUNC)
		return BKE_Variable();
	auto *v = (BKE_VarFunction*)obj;
	if (self)
		v->setSelf(self);
	auto arr = new BKE_VarArray();
	return _run(arr, args...);
}

template<typename... Args>
BKE_Variable BKE_Variable::run(BKE_VarObject *o, Args...args)
{
	if (getType() != VAR_FUNC)
		return BKE_Variable();
	auto *v = (BKE_VarFunction*)obj;
	if (o)
	{
		BKE_Variable self = o->addRef();
		v->setSelf(self);
	}
	auto arr = new BKE_VarArray();
	return _run(arr, args...);
}

template<typename Param, typename... Args>
BKE_Variable BKE_Variable::_run(BKE_VarArray *paramarray, const Param &head, Args... rest)
{
	paramarray->pushMember(head);
	return _run(paramarray, rest...);
}
//end

template<typename Param>
BKE_Variable BKE_Variable::_run(BKE_VarArray *paramarray, const Param &head)
{
	paramarray->pushMember(head);
	return ((BKE_VarFunction*)obj)->run(paramarray);
}

#pragma pop_macro("new")