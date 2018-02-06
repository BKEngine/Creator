#pragma once

#include "Bagel_Utils.h"
#include "Bagel_String.h"
#include <stdarg.h>
#include <initializer_list>
#include <set>
#include "BKE_array.h"
#include "BKE_hash.hpp"
#include "Bagel_GC.h"

#pragma push_macro("new")
#undef new

class Bagel_Closure;
class Bagel_Var;
class Bagel_Array;
class Bagel_ThreadContext;
class Bagel_Vector;
#define _FUNC_PARAM		const Bagel_Var *self, const Bagel_Var *paramarray, int paramcount, Bagel_Closure *_this, Bagel_ThreadContext *ctx
typedef Bagel_Var(*Bagel_NativeFunction)(_FUNC_PARAM);
class Bagel_Serializer;

enum
{
	VARIABLE_VAR,
	TEMP_VAR,
	CONST_VAR,
	LOCK_TYPE,
};

enum PromptType
{
	API_KEYWORDS,
	API_VARIABLE,
	API_FUNCTION,
	API_CLASS,
	API_PROPERTY,
};

class Bagel_Pointer;
class Bagel_Dic;
class Bagel_Function;
class Bagel_Prop;
class Bagel_Class;
class Bagel_ClassDef;
class Bagel_Var
{
	friend class Bagel_VM;
	friend class Bagel_JitCompiler;
	friend void initFieldData();
private:
	inline void clear()
	{
		//vt = VAR_NONE;
	};

	//VarType vt : 16;
	//int is_var : 16;
	VarType vt;

public:
	//VM临时记录用
	union
	{
		Bagel_Var* tag;
		int tick;
	};

private:
	union
	{
		double num;
		Bagel_Object *obj;
		BKE_hashmap<Bagel_StringHolder, Bagel_Var>::iterator it;
	};

public:

	//construct functions
	inline Bagel_Var() :vt(VAR_NONE)//, is_var(VARIABLE_VAR)
	{
	}

	inline Bagel_Var(int16_t i) : vt(VAR_NUM), num(i)//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(uint16_t i) : vt(VAR_NUM), num(i)//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(float i) : vt(VAR_NUM), num(i)//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(double i) : vt(VAR_NUM), num(i)//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(int64_t i) : vt(VAR_NUM), num(i)//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(int32_t i) : vt(VAR_NUM), num(i)//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(uint32_t i) : vt(VAR_NUM), num(i)//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(const wchar_t *s) : vt(VAR_STR), obj(_globalStructures.stringMap->allocateString(s))//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(const char16_t *s) : vt(VAR_STR), obj(_globalStructures.stringMap->allocateString(s))//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(wchar_t s) : vt(VAR_STR), obj(_globalStructures.stringMap->allocateString(wstring(1, s)))//, is_var(VARIABLE_VAR)
	{
	}
#if !defined(_MSC_VER) || _MSC_VER > 1800
	inline Bagel_Var(char16_t s) : vt(VAR_STR), obj(_globalStructures.stringMap->allocateString(u16string(1, s)))//, is_var(VARIABLE_VAR)
	{
	}
#endif
	inline Bagel_Var(const wstring &s) : vt(VAR_STR), obj(_globalStructures.stringMap->allocateString(s))//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(Bagel_StringHolder s) : vt(VAR_STR), obj(s.s)//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(const StringVal &s) : vt(VAR_STR), obj(_globalStructures.stringMap->allocateString(s))//, is_var(VARIABLE_VAR)
	{
	}
	inline Bagel_Var(StringVal &&s) : vt(VAR_STR), obj(_globalStructures.stringMap->allocateString(std::move(s)))//, is_var(VARIABLE_VAR)
	{
	}

	template <class T>
	inline Bagel_Var(const Bagel_Handler<T> &s) : Bagel_Var((T*)s)
	{
	}

	inline Bagel_Var(const Bagel_Object* const o)
	{
		obj = (Bagel_Object*)o;
		if (!o)
			vt = VAR_NONE;
		else 
			vt = o->vt;
		//is_var = VARIABLE_VAR;
	}

	Bagel_Var(const Bagel_Var &v) = default;

	Bagel_Var(Bagel_NativeFunction func);

	template<class T> Bagel_Var(initializer_list<T> list);

	//static constructor for dic and array
	static FORCEINLINE Bagel_Var array(int size = 0);
	static FORCEINLINE Bagel_Var dic();

	template<class Head, class ... Args>
	static Bagel_Var arrayWithObjects(const Head &h, Args... args);

	template<class Head>
	static Bagel_Var arrayWithObjects(const Head &h);

	template<class Head, class ... Args>
	static Bagel_Var arrayWithObjects(Bagel_Array *a, const Head &h, Args... args);

	template<class Head>
	static Bagel_Var arrayWithObjects(Bagel_Array *a, const Head &h);

public:
	static StringVal _getThrowString(const StringVal &str, const Bagel_Var *_this)
	{
		StringVal str2 = W("变量") + _this->saveShortString();
		str2 += W("\t:");
		str2 += str;
		return str2;
	}

public:
	//variable and const
	//inline void makeUnchangable()
	//{
	//	is_var = LOCK_TYPE;
	//};
	//inline void makeConst()
	//{
	//	is_var = CONST_VAR;
	//};
	//inline void makeVar()
	//{
	//	is_var = VARIABLE_VAR;
	//};
	//inline bool isVar() const
	//{
	//	return is_var == VARIABLE_VAR;
	//};
	//inline bool isTemp() const
	//{
	//	return is_var == TEMP_VAR;
	//};
	//inline bool isConst() const
	//{
	//	return is_var == CONST_VAR;
	//};
	//inline bool canNotChange() const
	//{
	//	return is_var >= CONST_VAR;
	//};

	inline bool isObject() const
	{
		return vt >= VAR_STR;
	}

	inline bool isArray() const
	{
		return vt == VAR_ARRAY;
	}

	inline bool isDic() const
	{
		return vt == VAR_DIC;
	}

	//saveStruct -- save to a u16string which can be direct load
	//format means to format result as krkr
	//or all the result will br put in one line
	//result will be append
	//indent is used for recursion
	void save(StringVal &result, bool format = true, int32_t indent = 0, const char16_t *nameinfo = nullptr) const;
	StringVal save(bool format = true, int32_t indent = 0) const
	{
		StringVal res;
		save(res, format, indent);
		return res;
	}
	bool saveToFile(const u16string &filename, bool format = true)
	{
		return _globalStructures.writeFunc(save(format), filename, 0, false);
	}
	bool saveToFile(const string &filename, bool format = true)
	{
		return _globalStructures.writeFunc(save(format), UTF16FromUTF8(filename), 0, false);
	}
	static Bagel_Var readFromFile(const u16string &filename);

	inline StringVal saveString(bool format = true) const
	{
		StringVal s;
		save(s, format, 0);
		return s;
	}

	StringVal saveShortString() const;

	vector<uint32_t>&& toBinaryStream();

	//type-conversion
	inline Bagel_String* asBKEStr() const
	{
		if (vt == VAR_STR)
			return forceAsBKEStr();
		return _globalStructures.stringMap->allocateString(asString());
	}
	bool asBoolean() const;
	int64_t _asInteger() const;
	uint64_t _asUInteger() const;
	double _asNumber() const;
	wstring asWString() const;
	StringVal _asString() const;
	Bagel_Array *asArray() const;
	Bagel_Dic *asDic() const;
	Bagel_Function *asFunc() const;
	Bagel_Class *asClass() const;
	Bagel_ClassDef *asClassDef() const;
	bool _canBeNumber() const;
	inline bool isInteger() const
	{
		return vt == VAR_NUM && isInt(num);
	};
	inline bool isNumber() const
	{
		return vt == VAR_NUM;
	}
	inline bool isString() const
	{
		return vt == VAR_STR;
	}
	int64_t roundAsInteger() const;
	inline int64_t roundToInteger()
	{
		int64_t i = roundAsInteger();
		clear();
		num = i;
		vt = VAR_NUM; 
		return i;
	}
	inline bool toBoolean()
	{
		bool s = asBoolean();
		clear(); 
		num = s ? 1 : 0;
		vt = VAR_NUM; 
		return s;
	};
	inline int64_t toInteger()
	{
		int64_t i = asInteger();
		clear(); 
		num = i;
		vt = VAR_NUM;
		return i;
	};
	inline double toNumber()
	{
		if (vt == VAR_NUM) 
			return num;
		double s = _asNumber();
		clear(); 
		num = s;
		vt = VAR_NUM;
		return s;
	};
	inline const StringVal& toString()
	{
		if (vt != VAR_STR)
		{
			auto s = _asString();
			clear();
			obj = _globalStructures.stringMap->allocateString(s);
			vt = VAR_STR;
		}
		return forceAsBKEStr()->getConstStr();
	};
	inline wstring toWString()
	{
		if (vt != VAR_STR)
		{
			auto s = asString();
			clear();
			obj = _globalStructures.stringMap->allocateString(s);
			vt = VAR_STR;
		}
		return forceAsBKEStr()->getWString();
	};

	Bagel_Object *getObject() const
	{
		return vt <= VAR_NUM ? NULL : obj;
	}

	Bagel_Object *forceAsObject() const
	{
		return obj;
	}

	template <class T>
	T *forceAsObject() const
	{
		return (T*)obj;
	}

	//quick conversion with if
	FORCEINLINE double asNumber() const
	{
		if (getType() == VAR_NUM)
			return num;
		return _asNumber();
	}

	FORCEINLINE int64_t asInteger() const
	{
		if (getType() == VAR_NUM)
			return (int64_t)num;
		return _asInteger();
	}

	FORCEINLINE uint64_t asUInteger() const
	{
		if (getType() == VAR_NUM)
			return (uint64_t)num;
		return _asUInteger();
	}

	FORCEINLINE StringVal asString() const
	{
		if (getType() == VAR_STR)
			return forceAsBKEStr()->getConstStr();
		return _asString();
	}

	FORCEINLINE bool canBeNumber() const
	{
		if (getType() <= VAR_NUM)
			return true;
		return _canBeNumber();
	}

	const double& forceAsNumber() const;
	double& forceAsNumber();
	int64_t forceAsInteger() const;
	bool forceAsBoolean() const;
	u16string forceAsString() const;
	wstring forceAsWString() const;
	Bagel_String* forceAsBKEStr() const;
	Bagel_Array *forceAsArray() const;
	Bagel_Dic *forceAsDic() const;
	Bagel_Function *forceAsFunc() const;
	Bagel_Prop *forceAsProp() const;
	Bagel_Pointer *forceAsPointer() const;
	Bagel_Closure *forceAsClosure() const;
	Bagel_Class *forceAsClass() const;
	Bagel_ClassDef *forceAsClassDef() const;

	//get restrict value whith no throw exception
	Bagel_Number getBKENum(Bagel_Number defaultValue = 0) const;
	Bagel_String* getBKEStr(Bagel_String* defaultValue = NULL) const;
	float getFloat(float defaultValue = 0) const;
	double getDouble(double defaultValue = 0) const;
	int64_t getInteger(int64_t defaultValue = 0) const;
	StringVal getString(const StringVal &defaultValue = StringVal()) const;
	wstring getWString(const wstring &defaultValue = wstring()) const;
	bool getBoolean(bool defaultValue = false) const;
	Bagel_Array *getArray(Bagel_Array *defaultValue = NULL) const;
	Bagel_Dic *getDic(Bagel_Dic *Bagel_Dic = NULL) const;
	Bagel_Function *getFunc(Bagel_Function *defaultValue = NULL) const;
	Bagel_Class *getClass(Bagel_Class *defaultValue = NULL) const;
	Bagel_Closure * getClosure(Bagel_Closure * defaultValue = NULL) const;
	Bagel_Prop *getProp(Bagel_Prop *defaultValue = NULL) const;

	template<class T, class Cond = void, class... Args>
	struct ConventerDelegate
	{
		static T convertTo(const Bagel_Var &_this, Args&&...args)
		{
			return _this.operator T();
		}

		static Bagel_Var convertFrom(const T& v)
		{
			return Bagel_Var(v);
		}
	};

	template<class T>
	struct ConventerDelegate<Bagel_Handler<T>>
	{
		static Bagel_Handler<T> convertTo(const Bagel_Var &_this)
		{
			return _this.operator T*();
		}

		static Bagel_Var convertFrom(const Bagel_Handler<T>& v)
		{
			return Bagel_Var(v);
		}
	};

	template<class T>
	T convertTo() const
	{
		return ConventerDelegate<T>::convertTo(*this);
	}

	template<class T, class... Args>
	T convertTo(Args&&...args) const
	{
		return ConventerDelegate<T, Args...>::convertTo(*this, args...);
	}

	template<class T>
	static Bagel_Var convertFrom(const T& v)
	{
		return ConventerDelegate<T>::convertFrom(v);
	}

	inline operator float() const
	{
		return (float)asNumber();
	};
	inline operator double() const
	{
		return asNumber();
	};
	inline operator bool() const
	{
		return asBoolean();
	};
	inline operator int16_t() const
	{
		return (int16_t)asInteger();
	};
	inline operator uint16_t() const
	{
		return (uint16_t)asInteger();
	};
	inline operator int32_t() const
	{
		return (int32_t)asInteger();
	};
	inline operator uint32_t() const
	{
		return (uint32_t)asInteger();
	};
	inline operator int64_t() const
	{
		return asInteger();
	};
	inline operator uint64_t() const
	{
		return asUInteger();
	};
	inline operator wstring() const
	{
		return asWString();
	};
	inline operator StringVal() const
	{
		return asString();
	};
	inline operator Bagel_String*() const
	{
		return asBKEStr();
	}
	inline operator Bagel_StringHolder() const
	{
		return asBKEStr();
	}
	//cause operator wstring invalid
	//inline operator const wchar_t*() const
	//{
	//	if (vt == VAR_STR)
	//		return str.getConstStr().c_str();
	//	_throw(W("无法转化为字符串"));
	//}

	FORCEINLINE int getType() const
	{
		return vt;
	};

	FORCEINLINE Bagel_StringHolder getTypeString() const
	{
		return getTypeString(vt);
	}

	static Bagel_StringHolder getTypeString(int type);

	Bagel_StringHolder getChineseTypeString() const;

	Bagel_StringHolder getClassName() const;

	Bagel_StringHolder getTypeBKEString() const
	{
		return getTypeString();
	}

	bool instanceOf(Bagel_StringHolder str) const;

	inline bool isVoid() const
	{
		return !vt;
	}

	Bagel_ObjectID getObjectID() const
	{
		return vt >= VAR_STR ? obj->getID() : -1;
	}

	inline bool equalToVoid() const
	{
		return isVoid() || (vt == VAR_NUM && isZero(num)) || (vt == VAR_STR && forceAsBKEStr()->isVoid());
	}

	Bagel_Var clone() const;

	void copyFrom(const Bagel_Var &v);
	void assignStructure(const Bagel_Var &v, BKE_hashmap<void*, void*> &pMap, bool first = false);

	inline void forceSet(const Bagel_Var &v)
	{
		vt = v.vt;
		num = v.num;
	}

	inline void forceSet(Bagel_Object *o)
	{
		vt = o ? o->vt : VAR_NONE;
		obj = o;
	}

	template <class T>
	inline void forceSetNum(T num)
	{
		vt = VAR_NUM;
		this->num = num;
	}

	void setMember(const Bagel_Var &str, const Bagel_Var &v);

	//= operator
	Bagel_Var& operator = (int16_t v);
	Bagel_Var& operator = (uint16_t v);
	inline Bagel_Var& operator = (bool v)
	{
		return operator = (v ? 1 : 0);
	};
	Bagel_Var& operator = (int32_t v);
	Bagel_Var& operator = (uint32_t v);
	Bagel_Var& operator = (int64_t v);
	Bagel_Var& operator = (double v);
	Bagel_Var& operator = (float v);
	Bagel_Var& operator = (const wstring &str);
	Bagel_Var& operator = (const wchar_t *str);
	Bagel_Var& operator = (const char16_t *str);
	Bagel_Var& operator = (Bagel_StringHolder v);
	Bagel_Var& operator = (const StringVal &v);
	Bagel_Var& operator = (const Bagel_Var &v);
	Bagel_Var& operator = (Bagel_Var &&v);
	Bagel_Var& operator = (Bagel_Object *v);
	Bagel_Var& operator = (const Bagel_Object *v)
	{
		return operator = (const_cast<Bagel_Object*>(v));
	};
	Bagel_Var& operator = (Bagel_NativeFunction func);
	template<class T>
	Bagel_Var& operator = (const Bagel_Handler<T> &v)
	{
		return operator = ((T*)v);
	}

	Bagel_Var operator + (const Bagel_Var &v) const;
	Bagel_Var operator + (const wstring &v) const;
	Bagel_Var operator - (const Bagel_Var &v) const;
	inline Bagel_Var operator * (const Bagel_Var &v) const
	{
		return asNumber() * v.asNumber();
	};
	inline Bagel_Var operator / (const Bagel_Var &v) const
	{
		return asNumber() / v.asNumber();
	};
	inline Bagel_Var operator % (const Bagel_Var &v) const
	{
		return fmod(asNumber(), v.asNumber());
	};
	//^ as power
	inline Bagel_Var operator ^ (const Bagel_Var &v) const
	{
		return pow(asNumber(), v.asNumber());
	};
	inline Bagel_Var& operator | (Bagel_Var &v)
	{
		return isVoid() ? v : *this;
	}
	inline const Bagel_Var& operator | (const Bagel_Var &v) const
	{
		return isVoid() ? v : *this;
	}
	inline Bagel_Var operator ! () const
	{
		return asBoolean() ? 0 : 1;
	};
	//++a
	inline Bagel_Var& operator ++ ()
	{
		auto a = asNumber(); a += 1; return (*this = a);
	};
	//a++
	inline Bagel_Var operator ++ (int)
	{
		auto a = asNumber(); *this = a + 1; return a;
	};
	//--a
	inline Bagel_Var& operator -- ()
	{
		auto a = asNumber(); a -= 1; return (*this = a);
	};
	//a--
	inline Bagel_Var operator -- (int)
	{
		auto a = asNumber(); *this = a - 1; return a;
	};
	Bagel_Var& operator += (const Bagel_Var &v);
	Bagel_Var& operator -= (const Bagel_Var &v);
	//getAddr
	Bagel_Var& operator [] (const Bagel_Var &v) const;
	//getAddr
	Bagel_Var& operator [] (Bagel_StringHolder v) const;
	Bagel_Var& operator [] (const Bagel_String *v) const;
	//getAddr
	Bagel_Var& operator [] (const wstring &v) const;
	Bagel_Var& operator [] (const u16string &v) const;
	//getAddr
	inline Bagel_Var& operator [] (const wchar_t *v) const
	{
		wstring vv(v); return operator [] (vv);
	};
	inline Bagel_Var& operator [] (const char16_t *v) const
	{
		u16string vv(v); return operator [] (vv);
	};
	inline Bagel_Var& operator [] (char16_t *v) const
	{
		u16string vv(v); return operator [] (vv);
	};
	//getAddr
	Bagel_Var& operator [] (int v) const;
	Bagel_Var& operator [] (int64_t v) const;
	bool operator == (const Bagel_Var &v) const;
	bool strictEqual(const Bagel_Var &v) const;
	bool normalEqual(const Bagel_Var &v) const;
	inline bool operator != (const Bagel_Var &v) const
	{
		return !(*this == v);
	};
	bool operator < (const Bagel_Var &v) const;
	bool operator > (const Bagel_Var &v) const;
	inline bool operator <= (const Bagel_Var &v) const
	{
		return !(*this > v);
	}
	inline bool operator >= (const Bagel_Var &v) const
	{
		return !(*this < v);
	}
	Bagel_Var getMid(int32_t *start, int32_t *stop, int32_t step);
	Bagel_Var dotValue(const Bagel_Var &funcname) const;
	Bagel_Var& dotAddr(Bagel_StringHolder funcname);
	bool dotFunc(Bagel_StringHolder funcname, Bagel_Var &out) const;
	Bagel_Var operator + (double v) const;
	Bagel_Var operator - (double v) const;
	inline Bagel_Var operator * (double v) const
	{
		return asNumber() * v;
	};
	inline Bagel_Var operator / (double v) const
	{
		return asNumber() / v;
	};
	inline Bagel_Var operator % (double v) const
	{
		return fmod(asNumber(), v);
	};
	inline Bagel_Var operator ^ (double v) const
	{
		return pow(asNumber(), v);
	};
	inline Bagel_Var& operator *= (double v);
	inline Bagel_Var& operator /= (double v);
	inline Bagel_Var& operator %= (double v);
	inline Bagel_Var& operator ^= (double v);
	inline Bagel_Var& operator *= (const Bagel_Var &v)
	{
		return operator *=(v.asNumber());
	};
	inline Bagel_Var& operator /= (const Bagel_Var &v)
	{
		return operator /=(v.asNumber());
	};
	inline Bagel_Var& operator %= (const Bagel_Var &v)
	{
		return operator %=(v.asNumber());
	};
	inline Bagel_Var& operator ^= (const Bagel_Var &v)
	{
		return operator ^=(v.asNumber());
	};

	Bagel_Var getAllDots() const;		//获得.后面能接的东西

	bool equals(const Bagel_Var &v) const;

	int32_t getCount() const;

	inline void setVoid()
	{
		clear();
		vt = VAR_NONE;;
	}

	void push_back(const Bagel_Var &v);

	Bagel_Var run(Bagel_Object *o)
	{
		Bagel_Var tmp(o);
		return run(tmp);
	}
	Bagel_Var run(Bagel_Var &self);

	template<typename... Args>
	Bagel_Var run(Bagel_Var &self, Args&&...args);

	template<typename... Args>
	Bagel_Var run(Bagel_Object *o, Args&&...args);
private:
	template<typename Param, typename... Args>
	Bagel_Var _run(Bagel_Vector &paramarray, Param &&head, Args&&... rest);
	//end
	template<typename Param>
	Bagel_Var _run(Bagel_Vector &paramarray, Param &&head);
};

inline PromptType getPromptTypeFromVar(const Bagel_Var &v)
{
	switch (v.getType())
	{
	case VAR_FUNC:
		return PromptType::API_FUNCTION;
	case VAR_PROP:
		return PromptType::API_PROPERTY;
	case VAR_CLASS:
	case VAR_CLASSDEF:
		return PromptType::API_CLASS;
	default:
		return PromptType::API_VARIABLE;
	}
}

/// <summary>
/// Bagel 的异常类。负责所有VM内的语法异常。（不包括std异常比如bad_alloc内存分配失败等）
/// 注意，从VM出来的异常描述信息内已经包含了位置信息和trace信息，不需要再设位置信息。
/// </summary>
class Bagel_Except
{
public:
	StringVal msg;
	int32_t pos;
	int32_t line;
#if PARSER_DEBUG
	StringVal lineinfo;
	StringVal functionname;
	std::u16string _file;
	std::u16string _func;
	std::u16string _line;
	Bagel_Except(const StringVal &str, const std::u16string &_f, const std::u16string &_f2, const std::u16string &l) :msg(str), pos(-1), line(-1), _file(_f), _func(_f2), _line(l)
	{
	};
	Bagel_Except(const StringVal &str, int32_t p, const std::u16string & _f, const std::u16string & _f2, const std::u16string &l) :msg(str), pos(p), line(-1), _file(_f), _func(_f2), _line(l)
	{
	};
	Bagel_Except(const std::wstring &str, const std::u16string &_f, const std::u16string &_f2, const std::u16string &l) :msg(UniToUTF16(str)), pos(-1), line(-1), _file(_f), _func(_f2), _line(l)
	{
	};
	Bagel_Except(const std::wstring &str, int32_t p, const std::u16string & _f, const std::u16string & _f2, const std::u16string &l) :msg(UniToUTF16(str)), pos(p), line(-1), _file(_f), _func(_f2), _line(l)
	{
	};
#endif
	/// <summary>
	/// 构造一个异常类。
	/// </summary>
	/// <param name="str">异常的描述字符串。</param>
	/// <param name="p">位置，默认为-1。</param>
	Bagel_Except(const std::wstring &str, int32_t p = -1) :msg(UniToUTF16(str)), pos(p), line(-1)
	{
	};
	/// <summary>
	/// 构造一个异常类。
	/// </summary>
	/// <param name="str">异常的描述字符串。</param>
	/// <param name="p">位置，默认为-1。</param>
	Bagel_Except(const StringVal &str, int32_t p = -1) :msg(str), pos(p), line(-1)
	{
	};
	/// <summary>
	/// 构造一个异常类。
	/// </summary>
	/// <param name="str">异常的描述字符串。</param>
	/// <param name="p">位置，默认为-1。</param>
	Bagel_Except(StringVal &&str, int32_t p = -1) :msg(std::move(str)), pos(p), line(-1)
	{
	};
	/// <summary>
	/// 获得包含行和位置信息（如果有的话）的宽字符版本的异常描述。
	/// </summary>
	/// <returns>异常描述字符串。</returns>
	inline std::wstring getWMsg() const
	{
		if (line == -1 && pos == -1)
			return UniFromUTF16(msg);
		std::wstring s = L"在";
		if (line != -1)
			s += bkpInt2WStr((int)line) + L"行";
		if (pos != -1)
			s += bkpInt2WStr((int)pos) + L"处：";
		s += UniFromUTF16(msg);
		return s;
	};
	/// <summary>
	/// 获得包含行和位置信息（如果有的话）的 std::u16string 版本的异常描述。
	/// </summary>
	/// <returns>异常描述字符串。</returns>
	inline std::u16string getMsg() const
	{
		if (line == -1 && pos == -1)
			return msg;
		std::u16string s = W("在");
		if (line != -1)
			s += bkpInt2Str((int)line) + W("行");
		if (pos != -1)
			s += bkpInt2Str((int)pos) + W("处：");
		s += msg;
		return s;
	};
	/// <summary>
	/// 获得异常描述的原始字符串（没有位置信息，类型为std::u16string）。
	/// </summary>
	/// <returns>异常描述字符串。</returns>
	inline StringVal getMsgWithoutPos() const
	{
		return msg;
	};

	/// <summary>
	/// 设置异常信息。
	/// </summary>
	/// <param name="str">异常描述字符串。</param>
	inline void setMsg(const Bagel_String &str)
	{
		msg = str;
	};
	/// <summary>
	/// 给异常添加位置信息。
	/// </summary>
	/// <param name="pos">位置。</param>
	inline void addPos(int32_t pos)
	{
		this->pos = pos;
	}
	/// <summary>
	/// 移除异常的位置信息。
	/// </summary>
	inline void removePos()
	{
		this->pos = -1;
	}
	/// <summary>
	/// 给异常添加行号信息。
	/// </summary>
	/// <param name="pos">行号。</param>
	inline void addLine(int32_t line)
	{
		this->line = line;
	}
	/// <summary>
	/// 移除异常的行号信息。
	/// </summary>
	inline void removeLine()
	{
		this->line = -1;
	}
	/// <summary>
	/// 获得异常的位置。
	/// </summary>
	/// <returns>异常的位置。</returns>
	inline int32_t getPos()
	{
		return pos;
	}
	/// <summary>
	/// 获得异常的行号。
	/// </summary>
	/// <returns>异常的行号。</returns>
	inline int32_t getLine()
	{
		return line;
	}
	/// <summary>
	/// 判断异常是否有位置信息。
	/// </summary>
	/// <returns>位置信息是否存在。</returns>
	inline bool hasPos()
	{
		return pos > -1;
	}
};

//表示VM里手动throw的变量异常，只是个记号，异常的变量放在btc->expectInfo里
class Bagel_ManualExcept
{
public:
	Bagel_ManualExcept()
	{
	}
};

class Bagel_Vector : public Bagel_Object, public vector<Bagel_Var>
{
public:
	~Bagel_Vector()
	{
	};

	Bagel_Vector() :Bagel_Object(VAR_VECTOR_P)
	{
	};

	Bagel_Vector(std::initializer_list<Bagel_Var> l):Bagel_Object(VAR_VECTOR_P), vector<Bagel_Var>(l)
	{
	};

	virtual void markChildren() override
	{
		for (auto &it : *this)
		{
			_GC.GC_Markself(it);
		}
	}
};

class Bagel_Stack : public Bagel_Object
{
	friend class Bagel_ThreadContext;
	friend class GC_Manager;
	friend class Bagel_Serializer;
	friend void initFieldData();
protected:
	//vector<Bagel_Var> stack;
	Bagel_Var *stack;
public:
	int stacksize;
	Bagel_Var *relativePos;
	bool isSpecialStack;	//是否是被闭包函数引用的栈

	~Bagel_Stack();

	Bagel_Stack() : Bagel_Object(VAR_STACK_P), isSpecialStack(false)
	{
	};

	virtual void markChildren() override
	{
		for (int i = 0; i < stacksize; i++)
		{
			_GC.GC_Markself(stack[i]);
		}
	}

	Bagel_Var *begin() const
	{
		return stack;
	}

	Bagel_Var *end() const
	{
		return stack + stacksize;
	}
};

//include array, dic, closure, func, prop, class

#define ARRAY_MAXSTEP 32768

inline int calc_capacity(int idx)
{
	assert(idx > 0);
	if (idx > ARRAY_MAXSTEP)
	{
		return (idx & ~(ARRAY_MAXSTEP - 1)) + ARRAY_MAXSTEP;
	}
	else
	{
		int c = ARRAY_MAXSTEP;
		while (!(c & idx))
			c >>= 1;
		return c << 1;
	}
}

class Bagel_Array :public Bagel_Object
{
public:
	BKE_array<Bagel_Var> vararray;
	//vector<Bagel_Var, BKE_Allocator<Bagel_Var>> vararray;

	inline Bagel_Array() :Bagel_Object(VAR_ARRAY)
	{
		vararray.reserve(8);
	};
	Bagel_Array(std::initializer_list<Bagel_Var> l) :Bagel_Object(VAR_ARRAY), vararray(l)
	{
	}
	~Bagel_Array()
	{
	}
	virtual void markChildren() override
	{
		for (auto &it : vararray)
		{
			_GC.GC_Markself(it);
		}
	}
	FORCEINLINE int32_t getCount() const
	{
		return vararray.size();
	};
	FORCEINLINE Bagel_Var getMember(int index) const
	{
		if (index < 0)
			index += vararray.size();
		if (index >=0 && index < (int32_t)vararray.size())
			return vararray[index];
		if (index < 0)
			throw Bagel_Except(W("下标越界（过小）"));
		return Bagel_Var();
	};
	FORCEINLINE Bagel_Var& getMemberAddr(int index)
	{
		if (index < 0)
			index += getCount();
		if (index < 0)
			throw Bagel_Except(W("下标越界（过小）"));
		if (index >= (int32_t)vararray.size())
		{
			if (index >= vararray.capacity())
			{
				vararray.reserve(calc_capacity(index));
			}
			vararray.resize(index + 1);
		}
		return vararray[index];
	};
	inline Bagel_Var& quickGetMember(int index)
	{
		//if (index<0 || index >= getCount())
		//	throw Bagel_Except(W("下标越界"));
		return vararray[index];
	};
	void insertMember(int index, const Bagel_Var &obj)
	{
		int32_t cnt = getCount();
		if (index < 0)
			index += cnt;
		if (index < 0)
			throw Bagel_Except(W("下标越界（过小）"));
		if (index >= cnt)
		{
			setMember(index, obj);
			return;
		}
		vararray.insert(vararray.begin() + index, obj);
	}
	void setMember(int index, const Bagel_Var &obj)
	{
		if (index < 0)
			index += getCount();
		if (index < 0)
			throw Bagel_Except(W("下标越界（过小）"));
		if (index >= vararray.size())
			vararray.resize(index + 1);
		vararray[index] = obj;
		if (obj.isObject())
			_GC.writeBarrier(this, obj.forceAsObject());
	}
	inline void quickSetMember(int index, const Bagel_Var &obj)
	{
		vararray[index] = obj;
	}
	void deleteMemberIndex(int index)
	{
		if (index < 0)
			index += getCount();
		if (index < 0 || index >= vararray.size())
			return;
		vararray.erase(vararray.begin() + index);
	};
	inline void deleteMember(const Bagel_Var &obj)
	{
		for (auto it = vararray.begin(); it != vararray.end(); )
		{
			if (it->normalEqual(obj))
				it = vararray.erase(it);
			else
				++it;
		}
	};
	inline void clear()
	{
		vararray.clear();
	}
	void cloneFrom(Bagel_Array *v)
	{
		if (v)
		{
			clear();
			vararray.resize(v->getCount());
			for (int i = 0; i < v->getCount(); i++)
			{
				vararray[i].copyFrom(v->quickGetMember(i));
			}
		}
	};
	void concat(Bagel_Array *v)
	{
		if (v)
		{
			int32_t c = vararray.size();
			int32_t count = v->getCount();
			vararray.resize(c + count);
			for (int i = 0; i < count; i++)
			{
				vararray[c + i] = v->quickGetMember(i);
			}
		}
	};
	inline void pushMember(const Bagel_Var &v)
	{
		vararray.push_back(v);
	}
	inline void setLength(int l)
	{
		//if (l<0)
		//	return;
		vararray.resize(l);
	}
	bool equals(Bagel_Array *v) const
	{
		int32_t s1 = getCount();
		int32_t s2 = v ? v->getCount() : 0;
		int32_t s = max(s1, s2);
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
	int indexOf(const Bagel_Var &v) const
	{
		for (int i = 0; i < vararray.size(); i++)
		{
			if (vararray[i] == v)
				return i;
		}
		return -1;
	}
	StringVal join(const StringVal &c)
	{
		StringVal res;
		if (vararray.empty())
			return res;
		int i;
		for (i = 0; i < vararray.size() - 1; i++)
		{
			res += vararray[i].asString() + c;
		}
		res += vararray[i].asString();
		return res;
	}
};

template<class T> Bagel_Var::Bagel_Var(initializer_list<T> list)
{
	auto arr = new Bagel_Array();
	arr->vararray.reserve(list.size());
	for (auto &&i : list)
	{
		arr->vararray.push_back(i);
	}
	obj = arr;
	vt = VAR_ARRAY;
}

template<class Head, class ... Args>
Bagel_Var Bagel_Var::arrayWithObjects(const Head &h, Args... args)
{
	Bagel_Array *a = new Bagel_Array;
	a->pushMember(h);
	return arrayWithObjects(a, args...);
}

template<class Head>
Bagel_Var Bagel_Var::arrayWithObjects(const Head &h)
{
	Bagel_Array *a = new Bagel_Array;
	a->pushMember(h);
	return a;
}

template<class Head, class ... Args>
Bagel_Var Bagel_Var::arrayWithObjects(Bagel_Array *a, const Head &h, Args... args)
{
	a->pushMember(h);
	return arrayWithObjects(a, args...);
}

template<class Head>
Bagel_Var Bagel_Var::arrayWithObjects(Bagel_Array *a, const Head &h)
{
	a->pushMember(h);
	return a;
}

class Bagel_Dic :public Bagel_Object
{
public:
	BKE_hashmap<Bagel_StringHolder, Bagel_Var> varmap;

	Bagel_Dic() :Bagel_Object(VAR_DIC)
	{
	}

	virtual ~Bagel_Dic()
	{
	};
	struct KeyValuePair
	{
		const Bagel_StringHolder key;
		const Bagel_Var value;
		KeyValuePair(Bagel_StringHolder k, const Bagel_Var &v) :key(k), value(v)
		{
		}
	};

	Bagel_Dic(std::initializer_list<KeyValuePair> l) :Bagel_Object(VAR_DIC)
	{
		for (auto it = l.begin(); it != l.end(); it++)
			setMember(it->key, it->value);
	}

	virtual void markChildren() override
	{
		for (auto &it : varmap)
		{
			_GC.GC_Markself(it.first.s);
			_GC.GC_Markself(it.second);
		}
	}
	inline int32_t getCount() const
	{
		return varmap.getCount();
	};
	inline Bagel_Var& getMember(Bagel_StringHolder key)
	{
		return varmap[key];
	};
	inline bool getMemberValue(Bagel_StringHolder key, Bagel_Var **dv)
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
		{
			*dv = &it->second;
			return true;
		}
		return false;
	}
	inline Bagel_Var getMemberValue(Bagel_StringHolder key) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
			return it->second;
		return Bagel_Var();
	};
	inline Bagel_Var& setMember(Bagel_StringHolder key, const Bagel_Var &obj)
	{
		auto &&res = varmap.insert(key, obj);
		if(obj.isObject())
			_GC.writeBarrier(this, obj.forceAsObject());
		return res;
	};
	inline void deleteMemberIndex(Bagel_StringHolder key)
	{
		varmap.erase(key);
	};
	void deleteMember(const Bagel_Var &obj)
	{
		auto it = varmap.begin();
		while (it != varmap.end())
			if (it->second == obj)
				varmap.erase(it++);
			else
				it++;
	};
	inline void clear()
	{
		varmap.clear();
	};
	inline Bagel_Dic *cloneFrom(const Bagel_Dic *v)
	{
		if (v)
		{
			clear();
			for (auto it = v->varmap.begin(); it != v->varmap.end(); it++)
			{
				varmap[it->first].copyFrom(it->second);
			}
		}
		return this;
	}
	inline void update(const Bagel_Dic *v)
	{
		for (auto it = v->varmap.begin(); it != v->varmap.end(); it++)
		{
			varmap[it->first] = it->second;
		}
	}
	inline void except(const Bagel_Dic *v)
	{
		for (auto it = v->varmap.begin(); it != v->varmap.end(); it++)
		{
			varmap.erase(it->first);
		}
	}
	inline bool contains(Bagel_StringHolder s)
	{
		return varmap.contains(s);
	}
	Bagel_Var toArray()
	{
		Bagel_Array *arr = new Bagel_Array();
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

	bool equals(const Bagel_Dic *v) const
	{
		if (!v)
			return isVoidDic();
		BKE_hashmap<Bagel_StringHolder, Bagel_Var> varbackup(varmap);
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

	//iterator

	typedef BKE_hashmap<Bagel_StringHolder, Bagel_Var>::iterator iterator;
	typedef BKE_hashmap<Bagel_StringHolder, Bagel_Var>::const_iterator const_iterator;
	inline iterator begin()
	{
		return varmap.begin();
	}
	inline const_iterator begin() const
	{
		return varmap.begin();
	}
	inline iterator end()
	{
		return varmap.end();
	}
	inline const_iterator end() const
	{
		return varmap.end();
	}

	iterator find(Bagel_StringHolder key) const
	{
		return varmap.find(key);
	}

	//map for sort
	void getAllVariables(std::map<StringVal, PromptType> &result);
	void getAllVariablesWithPrefix(std::map<StringVal, PromptType>& result, const StringVal &prefix);
	void getAllVariablesWithPrefixAndType(std::map<StringVal, PromptType>& result, const StringVal &prefix, VarType vt);
};

class Bagel_Closure :public Bagel_Object
{
	friend struct GlobalStructures;
public:
	Bagel_Closure *parent;
	mutable BKE_hashmap<Bagel_StringHolder, Bagel_Var> varmap;

	inline Bagel_Closure(Bagel_Closure *p = NULL) :Bagel_Object(VAR_CLO)
	{
		parent = p;
	};
	virtual ~Bagel_Closure()
	{
	};
	virtual void markChildren() override
	{
		_GC.GC_Markself(parent);
		for (auto &it : varmap)
		{
			_GC.GC_Markself(it.first.s);
			_GC.GC_Markself(it.second);
		}
	}
	inline void clear()
	{
		varmap.clear();
	}
	virtual bool hasMember(Bagel_StringHolder key) const
	{
		if (varmap.find(key) != varmap.end())
			return true;
		if (parent)
			return parent->hasMember(key);
		varmap.insertKey(key);
		return true;
	}
	virtual bool hasMember(Bagel_StringHolder key, Bagel_Var **var) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
		{
			*var = &it->second;
			return true;
		}
		if (parent)
			return parent->hasMember(key, var);
		*var = &varmap[key];
		return true;
	}
	Bagel_Var& getMember(Bagel_StringHolder key)
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
		{
			return it->second;
		}
		if (parent)
			return parent->getMember(key);
		return varmap[key];
	}
	Bagel_Var getMember(Bagel_StringHolder key) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
		{
			return it->second;
		}
		if (parent)
			return parent->getMember(key);
		return Bagel_Var();
	}
	inline Bagel_Var& forceSetMember(Bagel_StringHolder key, const Bagel_Var &obj)
	{
		auto &v = varmap[key];
		v.forceSet(obj);
		if (obj.isObject())
			_GC.writeBarrier(this, obj.forceAsObject());
		return v;
	};
	inline Bagel_Var& setMember(Bagel_StringHolder key, const Bagel_Var &obj)
	{
		Bagel_Var &var = varmap[key];
		//if (!var.isVar() && !var.isTemp())
		//{
		//	throw Bagel_Except(W("操作数必须是变量") BAGEL_EXCEPT_EXT);
		//}
		var.forceSet(obj);
		if (obj.isObject())
			_GC.writeBarrier(this, obj.forceAsObject());
		return var;
	};
	inline void setConstMember(Bagel_StringHolder key, const Bagel_Var &obj)
	{
		varmap[key].forceSet(obj);
		//Bagel_Var &var = varmap[key];
		//if (!var.isVar() && !var.isTemp())
		//{
		//	throw Bagel_Except(W("操作数必须是变量") BAGEL_EXCEPT_EXT);
		//}
		//var = obj;
		//var.makeConst();
	};
	inline void setConstVar(Bagel_StringHolder key, const Bagel_Var &obj)
	{
		varmap[key].forceSet(obj);
		//Bagel_Var &var = varmap[key];
		//if (!var.isVar() && !var.isTemp())
		//{
		//	throw Bagel_Except(W("操作数必须是变量") BAGEL_EXCEPT_EXT);
		//}
		//var = obj;
		//var.makeUnchangable();
	};
	inline void deleteMemberIndex(Bagel_StringHolder key)
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
		{
			//if (it->second.isConst())
			//{
			//	throw Bagel_Except(W("不能删除常量"));
			//}
			varmap.erase(it);
		}
	};
	inline void Union(const Bagel_Handler<Bagel_Dic> &dic, bool override) 
	{
		if (!dic)
			return;
		varmap.Union(dic->varmap, override);
	}
	void deleteMember(const Bagel_Var obj)
	{
		auto it = varmap.begin();
		while (it != varmap.end())
			if (it->second == obj)
				varmap.erase(it++);
			else
				it++;
	};
	int32_t getNonVoidCount() const
	{
		int32_t num = 0;
		auto it = varmap.begin();
		for (; it != varmap.end(); it++)
			if (!it->second.isVoid())
				num++;
		return num;
	};
	int32_t geCount() const
	{
		return varmap.getCount();
	};
	inline Bagel_Closure *cloneFrom(const Bagel_Closure *v)
	{
		if (v)
		{
			clear();
			for (auto it = v->varmap.begin(); it != v->varmap.end(); it++)
			{
				varmap[it->first].copyFrom(it->second);
			}
		}
		return this;
	}
	inline static Bagel_Closure* global()
	{
		return _globalStructures.globalClosure;
	};

	virtual inline void addNativeFunction(Bagel_StringHolder key, Bagel_NativeFunction func);
	virtual void addNativePropGet(Bagel_StringHolder key, Bagel_NativeFunction func);
	virtual void addNativePropSet(Bagel_StringHolder key, Bagel_NativeFunction func);
	Bagel_Closure *getThisClosure();
	void assignStructure(Bagel_Closure *v, BKE_hashmap<void*, void*> &pMap, bool first = false);

	//map for sort
	void getAllVariables(std::map<StringVal, PromptType> &result);
	void getAllVariablesWithPrefix(std::map<StringVal, PromptType>& result, const StringVal &prefix);
	void getAllVariablesWithPrefixAndType(std::map<StringVal, PromptType>& result, const StringVal &prefix, VarType vt);
};

class Bagel_AST;

struct Bagel_ByteCode;
class Bagel_ThreadContext;
struct Bagel_DebugInformation;

class Bagel_FunctionCode :public Bagel_Object
{
protected:
	Bagel_Var VMDebugRun(const Bagel_Var &self, const Bagel_Var *paramarray, int paramcount, Bagel_Closure *_this, Bagel_Stack* clostack, Bagel_StringHolder name, Bagel_ThreadContext *ctx) const;
	Bagel_Var VMDebugRunAndBlock(const Bagel_Var &self, const Bagel_Var *paramarray, int paramcount, Bagel_Closure *_this, Bagel_Stack* clostack, Bagel_StringHolder name, Bagel_ThreadContext *ctx) const;

	Bagel_Var VMReleaseRun(const Bagel_Var &self, const Bagel_Var *paramarray, int paramcount, Bagel_Closure *_this, Bagel_Stack* clostack, Bagel_StringHolder name, Bagel_ThreadContext *ctx) const;
	Bagel_Var VMReleaseRunAndBlock(const Bagel_Var &self, const Bagel_Var *paramarray, int paramcount, Bagel_Closure *_this, Bagel_Stack* clostack, Bagel_StringHolder name, Bagel_ThreadContext *ctx) const;

public:
	Bagel_AST *code;
	Bagel_ByteCode *bytecode;
	Bagel_NativeFunction native;
	vector<Bagel_StringHolder> paramnames;
	vector<Bagel_Var> initial_stack;
	int paramarrpos;	//*xxx这样的参数的位置
	shared_ptr<Bagel_DebugInformation> info;
#if PARSER_DEBUG
	vector<Bagel_Var> returnvar;
#endif
	virtual void markChildren() override;
	inline Bagel_FunctionCode() :Bagel_Object(VAR_FUNCCODE_P), code(NULL), bytecode(NULL), native(NULL), paramarrpos(0)
	{
	};
	inline Bagel_FunctionCode(Bagel_NativeFunction func) :Bagel_Object(VAR_FUNCCODE_P), code(NULL), bytecode(NULL), native(func), paramarrpos(0)
	{
	};
	virtual ~Bagel_FunctionCode()
	{
	};
	Bagel_FunctionCode(Bagel_AST *code);
	Bagel_FunctionCode(Bagel_ByteCode *code) :Bagel_Object(VAR_FUNCCODE_P), code(NULL), bytecode(code), native(NULL), paramarrpos(0)
	{
	}
	//_this is the real run-time closure

	//Bagel_Var run(Bagel_String name, Bagel_Var *self, Bagel_Var *paramarray, int paramcount, Bagel_Closure *_this) const;
	//Bagel_Var run(Bagel_String name, Bagel_Var *self, const Bagel_AST *tr, Bagel_Closure *runclo, Bagel_Closure *paramclo) const;

	Bagel_Var VMRun(const Bagel_Var &self, const Bagel_Var *paramarray, int paramcount, Bagel_Closure *_this, Bagel_Stack* clostack, Bagel_StringHolder name, Bagel_ThreadContext *ctx);

	Bagel_Var VMRunAndBlock(const Bagel_Var &self, const Bagel_Var *paramarray, int paramcount, Bagel_Closure *_this, Bagel_Stack* clostack, Bagel_StringHolder name, Bagel_ThreadContext *ctx);
};

class Bagel_Function :public Bagel_Object
{
	friend class Bagel_Parser;
	friend class Bagel_Var;
	friend class Bagel_Class;

protected:
	inline Bagel_Closure* getClo()
	{
		return closure;
	}

public:
	virtual ~Bagel_Function()
	{
	}

	Bagel_FunctionCode *func;
	//从*self改为self，这意味着int和string类的function（如replace）再也不可以更改原字符串的内容了【
	Bagel_Var self;
	Bagel_Closure *closure;
	Bagel_Stack *default_stack;

	Bagel_StringHolder name;
	Bagel_StringHolder fullname;

	Bagel_Function() = delete;
	//used for deserialize
	inline Bagel_Function(Bagel_FunctionCode *funccode) :Bagel_Object(VAR_FUNC), default_stack(NULL)
	{
		func = funccode;
		closure = NULL;
	};
	inline Bagel_Function(Bagel_NativeFunction fun, Bagel_Var _self = Bagel_Var()) :Bagel_Object(VAR_FUNC), default_stack(NULL)
	{
		func = new Bagel_FunctionCode(fun);
		self = _self;
		closure = NULL;
	};
	inline Bagel_Function(Bagel_AST *tree, Bagel_Var _self = Bagel_Var()) :Bagel_Object(VAR_FUNC), default_stack(NULL)
	{
		func = new Bagel_FunctionCode(tree);
		self = _self;
		closure = NULL;
	};
	Bagel_Function(const Bagel_Function &f, Bagel_Closure *c = Bagel_Closure::global(), const Bagel_Var &_self = Bagel_Var()) :Bagel_Object(VAR_FUNC)
	{
		func = f.func;
		name = f.name;
		fullname = W("function ") + _self.getClassName() + W(".") + f.name;
		if (!_self.isVoid())
			self = _self;
		else
			self = f.self;
		closure = c;
		default_stack = f.default_stack;
	};
	virtual void markChildren() override
	{
		_GC.GC_Markself(func);
		_GC.GC_Markself(self);
		_GC.GC_Markself(closure);
		_GC.GC_Markself(name.s);
		_GC.GC_Markself(fullname.s);
		//cannot mark static stack
		if (default_stack && !default_stack->isSpecialStack)
			return;
		_GC.GC_Markself(default_stack);
	}
	FORCEINLINE Bagel_Var VMRun(Bagel_Var *params, int paramcount, Bagel_ThreadContext *ctx = NULL)
	{
		//if (func->native)
		//{
		//	return (func->native)(&self, params, paramcount, getClo(), ctx);
		//}
		return func->VMRun(self, params, paramcount, closure, default_stack, fullname, ctx);
	}
	FORCEINLINE Bagel_Var VMRunWithSelf(Bagel_Var *params, int paramcount, const Bagel_Var &_self, Bagel_Closure* clo, Bagel_ThreadContext *ctx = NULL)
	{
		//if (func->native)
		//{
		//	return (func->native)(&self, params, paramcount, getClo(), ctx);
		//}
		return func->VMRun(_self, params, paramcount, clo, default_stack, fullname, ctx);
	}
	FORCEINLINE Bagel_Var run(Bagel_Var *params, int paramcount)
	{
		return VMRun(params, paramcount, NULL);
	}
	inline void setSelf(const Bagel_Var &v)
	{
		self = v;
	};
	inline void setClosure(Bagel_Closure *c)
	{
		closure = c;
	}
	inline bool isNativeFunction()
	{
		return func->native != NULL;
	};
	//为否表示是闭包函数
	bool isNormalFunction() const
	{
		return closure == _globalStructures.globalClosure || self.getType() == VAR_CLASS;
	}
	u16string functionSimpleInfo() const
	{
		u16string s = W("(");
		for (int i = 0; i < func->paramnames.size(); i++)
		{
			if (i > 0)
				s += W(", ");
			s += func->paramnames[i].getConstStr();
		}
		s += W(")");
		return s;
	}
	u16string functionInfo()
	{
		u16string s = W("(");
		for (int i = 0; i < func->paramnames.size(); i++)
		{
			if (i > 0)
				s += W(", ");
			s += func->paramnames[i].getConstStr();
			if (!func->initial_stack[i].isVoid())
				s += W(" = ") + func->initial_stack[i].save(false);
		}
		s += W(")");
		return s;
	}
	void assignStructure(Bagel_Function *f, BKE_hashmap<void*, void*> &pMap, bool first = false);
};

inline void Bagel_Closure::addNativeFunction(Bagel_StringHolder key, Bagel_NativeFunction func)
{
	Bagel_Function *f = new Bagel_Function(func);
	f->name = key.getConstStr();
	varmap[key] = f;
}

class Bagel_Prop :public Bagel_Object
{
#if PARSER_DEBUG
	friend class PAModule;
#endif
private:
	inline Bagel_Closure *getClo() const
	{
		//有self时，说明是类的prop，使用类的闭包
		auto clo = self.forceAsClosure();
		return clo ? clo : closure;
	}
public:
	Bagel_FunctionCode *funcget;
	Bagel_FunctionCode *funcset;
	Bagel_Closure *closure;
	mutable Bagel_Var self;
	Bagel_StringHolder setparam;
	Bagel_Stack *default_stack;
	Bagel_StringHolder name;
	Bagel_StringHolder getname;
	Bagel_StringHolder setname;

	Bagel_Prop(Bagel_Closure *clo = NULL, Bagel_NativeFunction get = NULL, Bagel_NativeFunction set = NULL) : Bagel_Object(VAR_PROP), default_stack(NULL)
	{
		funcget = funcset = NULL;
		if (get)
			funcget = new Bagel_FunctionCode(get);
		if (set)
			funcset = new Bagel_FunctionCode(set);
		closure = clo;
	}
	Bagel_Prop(const Bagel_Prop &p, const Bagel_Closure *c = NULL, const Bagel_Var &_self = Bagel_Var()) : Bagel_Object(VAR_PROP), default_stack(NULL)
	{
		funcget = p.funcget;
		funcset = p.funcset;
		//p应该是一个ClassDef的property，name已经有类名前缀
		name = p.name;
		getname = p.getname;
		setname = p.setname;
		if (!_self.isVoid())
			self = _self;
		else
			self = p.self;
		closure = const_cast<Bagel_Closure*>(c);
		setparam = p.setparam;
	}
	virtual ~Bagel_Prop()
	{
	}
	virtual void markChildren() override
	{
		_GC.GC_Markself(funcget);
		_GC.GC_Markself(funcset);
		_GC.GC_Markself(self);
		_GC.GC_Markself(closure);
		_GC.GC_Markself(setparam.s);
		_GC.GC_Markself(name.s);
		_GC.GC_Markself(getname.s);
		_GC.GC_Markself(setname.s);
		//cannot mark static stack
		if (default_stack && !default_stack->isSpecialStack)
			return;
		_GC.GC_Markself(default_stack);
	}
	inline void addPropGet(Bagel_NativeFunction get)
	{
		funcget = new Bagel_FunctionCode(get);
	}
	inline void addPropGet(Bagel_AST *get)
	{
		funcget = new Bagel_FunctionCode(get);
	}
	inline void addPropSet(Bagel_NativeFunction set)
	{
		funcset = new Bagel_FunctionCode(set);
	}
	inline void addPropSet(Bagel_StringHolder setparam, Bagel_AST *set)
	{
		funcset = new Bagel_FunctionCode(set);
		this->setparam = setparam;
	}
	inline bool hasGet() const
	{
		return !!funcget;
	}
	inline bool hasSet() const
	{
		return !!funcset;
	}
	inline void setSelf(const Bagel_Var &v) const
	{
		self = v;
	};
	inline void setClosure(const Bagel_Closure *c)
	{
		closure = const_cast<Bagel_Closure*>(c);
	}

	inline Bagel_Var get() const
	{
		return VMGet(NULL);
	}
	FORCEINLINE Bagel_Var VMGet(Bagel_ThreadContext *ctx = NULL, bool blocked = false) const
	{
		if (funcget)
		{
			if(blocked)
				return funcget->VMRunAndBlock(self, NULL, 0, closure, default_stack, getname, ctx);
			return funcget->VMRun(self, NULL, 0, closure, default_stack, getname, ctx);
		}
		return Bagel_Var();
	};
	FORCEINLINE Bagel_Var VMGetWithSelf(const Bagel_Var &_self, Bagel_Closure* clo, Bagel_ThreadContext *ctx = NULL, bool blocked = false) const
	{
		if (funcget)
		{
			if (blocked)
				return funcget->VMRunAndBlock(_self, NULL, 0, clo, default_stack, getname, ctx);
			return funcget->VMRun(_self, NULL, 0, clo, default_stack, getname, ctx);
		}
		return Bagel_Var();
	};
	inline void set(const Bagel_Var &v) const
	{
		VMSet(v, NULL);
	}
	FORCEINLINE void VMSet(const Bagel_Var &v, Bagel_ThreadContext *ctx = NULL) const
	{
		if (funcset)
		{
			funcset->VMRun(self, &v, 1, closure, default_stack, setname, ctx);
		}
	}
	FORCEINLINE void VMSetWithSelf(const Bagel_Var &v, const Bagel_Var &_self, Bagel_Closure* clo, Bagel_ThreadContext *ctx = NULL) const
	{
		if (funcset)
		{
			funcset->VMRun(_self, &v, 1, clo, default_stack, setname, ctx);
		}
	}
	void assignStructure(Bagel_Prop *p, BKE_hashmap<void*, void*> &pMap, bool first = false);
};

class Bagel_Pointer : public Bagel_Object
{
#if PARSER_DEBUG
	friend class PAModule;
#endif
public:
	mutable Bagel_Var var;
	Bagel_StringHolder name;
	Bagel_Var *dir;

	Bagel_Pointer() : Bagel_Object(VAR_POINTER), dir(nullptr)
	{
	};
	Bagel_Pointer(const Bagel_Var &v, Bagel_StringHolder k, bool forceaddr) : Bagel_Object(VAR_POINTER), var(v), name(k)
	{
		if (forceaddr)
			dir = &var[name];
		else
			dir = nullptr;
	};
	Bagel_Pointer(Bagel_Var *addr) : Bagel_Object(VAR_POINTER)
	{
		dir = addr;
	}
	virtual ~Bagel_Pointer()
	{
	};

	virtual void markChildren() override
	{
		_GC.GC_Markself(var);
		_GC.GC_Markself(name.s);
	}

	Bagel_Var get() const
	{
		if (dir)
			return *dir;
		else
			return var.dotValue(name);
	}
	void set(const Bagel_Var &v)
	{
		(dir ? *dir : var.dotAddr(name)) = v;
		if (v.isObject())
			_GC.forceBarrier(v.forceAsObject());
	};

	void changeAddr(Bagel_Var *addr)
	{
		dir = addr;
		var.setVoid();
		name = Bagel_StringHolder();
	}

	void changeAddr(const Bagel_Var &v, Bagel_StringHolder k, bool forceaddr)
	{
		var = v;
		name = k;
		if (forceaddr)
			dir = &var[name];
		else
			dir = nullptr;
	}

	void assignStructure(Bagel_Pointer *p, BKE_hashmap<void*, void*> &pMap, bool first = false);
};

inline void Bagel_Closure::addNativePropGet(Bagel_StringHolder key, Bagel_NativeFunction func)
{
	Bagel_Var &var = varmap[key];
	if (var.getType() != VAR_PROP)
	{
		var = new Bagel_Prop(this);
	}
	var.forceAsProp()->addPropGet(func);
};

inline void Bagel_Closure::addNativePropSet(Bagel_StringHolder key, Bagel_NativeFunction func)
{
	Bagel_Var &var = varmap[key];
	if (var.getType() != VAR_PROP)
	{
		var = new Bagel_Prop(this);
	}
	var.forceAsProp()->addPropSet(func);
};

class Bagel_Class;

//pointerfree, _class is seen as a weak ptr
class Bagel_NativeClass
{
protected:
	int ref;	//在BKE_Creator等用到分析的时候，会在主闭包和备用闭包间复制class，同时也会复制native，这里加上引用计数一方面避免冗余的构造，一方面防止重复delete
	void _prepareClass(const u16string &name, bool isFinal = true);
public:
	Bagel_Closure *_class;	//classdef or class
	StringVal nativeName;
	Bagel_NativeClass() = delete;
	virtual ~Bagel_NativeClass()
	{
	}
	//inline Bagel_NativeClass(const wchar_t *name) : _class(NULL), nativeName(name)
	//{
	//}
	inline Bagel_NativeClass(const char16_t *name) : _class(NULL), nativeName(name), ref(1)
	{
	}
	virtual void nativeInit(const u16string &name) = 0;
	virtual Bagel_Var nativeSave()
	{
		return Bagel_Var();
	}
	virtual void nativeLoad(const Bagel_Var &var)
	{
	}
	//*_class=self
	virtual Bagel_NativeClass* nativeCreateNew(const Bagel_Class *self, const Bagel_Var *paramarray, int paramcount, Bagel_ThreadContext *ctx) = 0;
	virtual Bagel_NativeClass* nativeCreateNULL()
	{
		return NULL;
	};
	Bagel_NativeClass* retain()
	{
		ref++;
		return this;
	}
	void release()
	{
		if (--ref == 0)
			delete this;
	}

	virtual void markChildren()
	{
	};
};

class Bagel_ClassDef : public Bagel_Closure
{
	void check(Bagel_ClassDef *cla) const
	{
		if (cla->isInstanceof(classname))
			throw Bagel_Except(W("类定义存在循环定义，") + cla->classname.getConstStr() + W("已经是") + classname.getConstStr() + W("的子类。"));
		if (native && cla->native)
			throw Bagel_Except(W("目前不支持从多个Native类继承。"));
	}

	friend class Bagel_Var;
	friend class Bagel_Serializer;

	Bagel_ClassDef()
	{
		vt = VAR_CLASSDEF;
	}

public:
	Bagel_NativeFunction innerCreateInstance;
	Bagel_NativeClass *native;
	Bagel_StringHolder classname;
	bool cannotcreate;
	bool isFinal;
	vector<Bagel_ClassDef *, BKE_Allocator<Bagel_ClassDef*>> parents;
	vector<Bagel_ClassDef *, BKE_Allocator<Bagel_ClassDef*>> children;
	BKE_hashmap<Bagel_StringHolder, Bagel_Var> classvar;//单独拿出来防止被类名.变量名引用到
	BKE_hashset<Bagel_StringHolder> ownvar;	//这些var是自己定义的，父类更新时varmap不受影响

	int32_t classid;

	virtual ~Bagel_ClassDef()
	{
		//finalize();
		if (native)
			native->release();
	};

	virtual void markChildren() override
	{
		Bagel_Closure::markChildren();
		_GC.GC_Markself(classname.s);
		for (auto &it : parents)
		{
			_GC.GC_Markself(it);
		}
		for (auto &it : classvar)
		{
			_GC.GC_Markself(it.first.s);
			_GC.GC_Markself(it.second);
		}
		if (native)
			native->markChildren();
	}

	inline Bagel_ClassDef(Bagel_StringHolder name)//, Bagel_Closure *context = Bagel_Closure::global()) :Bagel_Closure(context)
	{
		cannotcreate = false;
		innerCreateInstance = NULL;
		native = NULL;
		classname = name;
		vt = VAR_CLASSDEF;
		isFinal = false;
		classid = _globalStructures.classid++;
	}

	static void checkParent(Bagel_StringHolder name)
	{
		auto &&p = _globalStructures.globalClosure->varmap[name];
		if (p.getType() != VAR_CLASSDEF)
			throw Bagel_Except(name.getConstStr() + W("不是一个类的定义"));
	}

	static void checkParent(const vector<Bagel_StringHolder> &parent)
	{
		for (auto &name : parent)
		{
			auto &&p = _globalStructures.globalClosure->varmap[name];
			if (p.getType() != VAR_CLASSDEF)
				throw Bagel_Except(name.getConstStr() + W("不是一个类的定义"));
		}
	}
	//#define GETCLASS(p, name) auto &&p = _globalStructures.globalClosure->varmap[name];if(p.getType() != VAR_CLASSDEF)throw Bagel_Except(name.getConstStr() + W("不是一个类的定义"));

	//must check parent class before creating class
#define GETCLASS(p, name) auto &&p = _globalStructures.globalClosure->varmap[name];if(p.getType() != VAR_CLASSDEF)assert(false);
	//single inherit class
	inline Bagel_ClassDef(Bagel_StringHolder name, Bagel_StringHolder parent)//, Bagel_Closure *context = Bagel_Closure::global()) :Bagel_Closure(context)
	{
		cannotcreate = false;
		innerCreateInstance = NULL;
		native = NULL;
		classname = name;
		vt = VAR_CLASSDEF;
		isFinal = false;
		GETCLASS(p, parent);
		auto cc = p.forceAsClassDef();
		check(cc);
		cc->children.push_back(this);
		parents.push_back(cc);
		refreshAll();
	}
	//single inherit class
	inline Bagel_ClassDef(Bagel_StringHolder name, Bagel_ClassDef* p)//, Bagel_Closure *context = Bagel_Closure::global()) :Bagel_Closure(context)
	{
		cannotcreate = false;
		innerCreateInstance = NULL;
		native = NULL;
		classname = name;
		vt = VAR_CLASSDEF;
		isFinal = false;
		check(p);
		p->children.push_back(this);
		parents.push_back(p);
		refreshAll();
	}
	//multi inherit class
	inline Bagel_ClassDef(Bagel_StringHolder name, const vector<Bagel_StringHolder> &parent)//, Bagel_Closure *context = Bagel_Closure::global()) :Bagel_Closure(context)
	{
		cannotcreate = false;
		innerCreateInstance = NULL;
		native = NULL;
		classname = name;
		vt = VAR_CLASSDEF;
		isFinal = false;
		for (int i = 0; i < parent.size(); i++)
		{
			GETCLASS(p, parent[i]);
			auto cc = p.forceAsClassDef();
			check(cc);
			cc->children.push_back(this);
			parents.push_back(cc);
		}
		refreshAll();
	}
	//multi inherit class
	inline Bagel_ClassDef(Bagel_StringHolder name, const vector<Bagel_ClassDef*> &parent)//, Bagel_Closure *context = Bagel_Closure::global()) :Bagel_Closure(context)
	{
		cannotcreate = false;
		innerCreateInstance = NULL;
		native = NULL;
		classname = name;
		vt = VAR_CLASSDEF;
		isFinal = false;
		for (int i = 0; i < parent.size(); i++)
		{
			auto cc = parent[i];
			check(cc);
			cc->children.push_back(this);
			parents.push_back(cc);
		}
		refreshAll();
	}

	void redefineClass(const vector<Bagel_StringHolder> &parent)
	{
		parents.clear();
		for (int i = 0; i < parent.size(); i++)
		{
			GETCLASS(p, parent[i]);
			auto cc = p.forceAsClassDef();
			check(cc);
			cc->children.push_back(this);
			parents.push_back(cc);
		}
		refreshAll();
	}
	void redefineClass(const vector<Bagel_ClassDef*> &parent)
	{
		parents.clear();
		for (int i = 0; i < parent.size(); i++)
		{
			auto cc = parent[i];
			check(cc);
			cc->children.push_back(this);
			parents.push_back(cc);
		}
		refreshAll();
	}

	void refreshAll()
	{
		if (native)
			delete native;
		native = NULL;
		//varmap.clear();
		for (auto it = varmap.begin(); it != varmap.end();)
		{
			if (ownvar.contains(it->first))
				++it;
			else
				it = varmap.erase(it);
		}
		classvar.clear();
		for (int i = 0; i < parents.size(); i++)
		{
			auto cc = parents[i];
			check(cc);
			for (auto &&it : cc->classvar)
				classvar[it.first] = it.second.clone();
			for (auto &it : cc->varmap)
			{
				if (ownvar.contains(it.first))
					continue;
				if (it.second.getType() == VAR_FUNC)
				{
					varmap[it.first].forceSet(new Bagel_Function(*it.second.forceAsFunc(), this, this));
				}
				else if (it.second.getType() == VAR_PROP)
				{
					varmap[it.first].forceSet(new Bagel_Prop(*it.second.forceAsProp(), this, this));
				}
				else
				{
					varmap[it.first].forceSet(it.second.clone());
				}
			}
			if (cc->native)
				native = cc->native->nativeCreateNULL();
		}
		classid = _globalStructures.classid++;
		for (auto &it : children)
			it->refreshAll();
	}

	//由于父类的key成员被更改，从而调用此命令
	//不需要refreshVar，因为不允许在class外部重定义var
	void refresh(Bagel_StringHolder key)
	{
		if (ownvar.contains(key))
			return;
		for (int i = 0; i < parents.size(); i++)
		{
			auto cc = parents[i];
			//check(cc);
			auto it = cc->varmap.find(key);
			if (it != cc->varmap.end())
			{
				if (it->second.getType() == VAR_FUNC)
				{
					varmap[it->first].forceSet(new Bagel_Function(*it->second.forceAsFunc(), this, this));
				}
				else if (it->second.getType() == VAR_PROP)
				{
					varmap[it->first].forceSet(new Bagel_Prop(*it->second.forceAsProp(), this, this));
				}
				else
				{
					varmap[it->first].forceSet(it->second.clone());
				}
			}
		}
		for (auto &it : children)
			it->refresh(key);
	}

	inline Bagel_ClassDef *cloneFrom(const Bagel_ClassDef *v)
	{
		//copy parent
		parents.resize(v->parents.size());
		for (int i = 0; i < parents.size(); i++)
		{
			parents[i] = v->parents[i];
		}
		if (v)
		{
			clear();
			classvar.clear();
			for (auto it = v->varmap.begin(); it != v->varmap.end(); it++)
			{
				varmap[it->first].copyFrom(it->second);
			}
			for (auto it = v->classvar.begin(); it != v->classvar.end(); it++)
			{
				classvar[it->first].copyFrom(it->second);
			}
		}
		return this;
	}

	void assignStructure(Bagel_ClassDef *cla, BKE_hashmap<void*, void*> &pMap, bool first = false);

	inline void addStaticMember(const u16string &key, const Bagel_Var &var) const
	{
		varmap.insert(key, var);
	};

	inline void addNativeFunction(Bagel_StringHolder key, Bagel_NativeFunction func) override
	{
		auto f = new Bagel_Function(func);
		f->setClosure(this);
		f->setSelf(this);
		varmap[key] = f;
		ownvar.insertKey(key);
		for(auto &it : children)
			it->refresh(key);
	};
	inline void addNativePropGet(Bagel_StringHolder key, Bagel_NativeFunction func) override
	{
		Bagel_Var &var = varmap[key];
		if (var.getType() != VAR_PROP)
		{
			var = new Bagel_Prop(this);
			var.forceAsProp()->setSelf(this);
		}
		var.forceAsProp()->addPropGet(func);
		ownvar.insertKey(key);
		for (auto &it : children)
			it->refresh(key);
	};
	inline void addNativePropSet(Bagel_StringHolder key, Bagel_NativeFunction func) override
	{
		Bagel_Var &var = varmap[key];
		if (var.getType() != VAR_PROP)
		{
			var = new Bagel_Prop(this);
			var.forceAsProp()->setSelf(this);
		}
		var.forceAsProp()->addPropSet(func);
		ownvar.insertKey(key);
		for (auto &it : children)
			it->refresh(key);
	};

	inline bool hasClassMember(Bagel_StringHolder key) const
	{
		if (varmap.find(key) != varmap.end())
			return true;
		return false;
	}

	inline bool hasClassMember(Bagel_StringHolder key, Bagel_Var **var) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
		{
			(*var) = &it->second;
			return true;
		}
		return false;
	}
	
	virtual bool hasMember(Bagel_StringHolder key) const override
	{
		if (!hasClassMember(key))
		{
			//then we find it on global
			return _globalStructures.globalClosure->hasMember(key);
		}
		return true;
	}
	virtual bool hasMember(Bagel_StringHolder key, Bagel_Var **var) const override
	{
		if (!hasClassMember(key, var))
		{
			//then we find it on global
			return _globalStructures.globalClosure->hasMember(key, var);
		}
		return true;
	}

	//获取function时，必须要用value
	Bagel_Var getClassMemberValue(Bagel_StringHolder key) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
			return it->second;
		throw Bagel_Except(W("成员") + key.getConstStr() + W("不存在"));
	}

	//获取function时，必须要用value
	Bagel_Var& VMgetClassMemberValue(Bagel_StringHolder key) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
			return it->second;
		throw Bagel_Except(W("成员") + key.getConstStr() + W("不存在"));
	}

	//获取function时，必须要用value
	Bagel_Var getClassMemberValue2(Bagel_StringHolder key) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
			return it->second;
		return _globalStructures.globalClosure->getMember(key);
	}

	//addr无法获取function
	Bagel_Var& getClassMemberAddr(Bagel_StringHolder key) const
	{
		return varmap[key];
	}

	//最后在全局闭包找的
	Bagel_Var& getClassMemberAddr2(Bagel_StringHolder key) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
			return it->second;
		return _globalStructures.globalClosure->getMember(key);
	}

	//_this执行super时所在的闭包
	inline Bagel_Var getSuperMember(Bagel_StringHolder key, Bagel_Closure *_this = nullptr)
	{
		if (parents.size() == 0)
			throw Bagel_Except(W("类") + classname.getConstStr() + W("不存在父类，super无意义。"));
		if (parents.size() > 1)
			throw Bagel_Except(W("类") + classname.getConstStr() + W("有多个父类，super有歧义。"));
		Bagel_Var *var;
		if (!_this)
			_this = this;
		if (parents[0]->hasClassMember(key, &var))
		{
			if (var->getType() == VAR_FUNC)
			{
				return new Bagel_Function(*var->forceAsFunc(), _this, _this);
			}
			if (var->getType() == VAR_PROP)
			{
				return new Bagel_Prop(*var->forceAsProp(), _this, _this);
			}
			throw Bagel_Except(W("类") + classname.getConstStr() + W("的父类") + parents[0]->classname.getConstStr() + W("的成员") + key.getConstStr() + W("不是函数或属性，super只能取父类的函数或属性"));
		}
		else
			throw Bagel_Except(W("类") + classname.getConstStr() + W("的父类") + parents[0]->classname.getConstStr() + W("不存在成员") + key.getConstStr());
	}

	inline Bagel_Var& getSuperMemberAddr(Bagel_StringHolder key)
	{
		if (parents.size() == 0)
			throw Bagel_Except(W("类") + classname.getConstStr() + W("不存在父类，super无意义。"));
		if (parents.size() > 1)
			throw Bagel_Except(W("类") + classname.getConstStr() + W("有多个父类，super有歧义。"));
		Bagel_Var *var;
		if (parents[0]->hasClassMember(key, &var))
		{
			if (var->getType() == VAR_PROP)
			{
				return *var;
			}
			throw Bagel_Except(W("类") + classname.getConstStr() + W("的父类") + parents[0]->classname.getConstStr() + W("的成员") + key.getConstStr() + W("不是属性，不能被赋值。"));
		}
		else
			throw Bagel_Except(W("类") + classname.getConstStr() + W("的父类") + parents[0]->classname.getConstStr() + W("不存在成员") + key.getConstStr());
	}
	
	void getAllConstructors(BKE_array<Bagel_Function*> *cons)
	{
		for (int i = 0; i < parents.size(); i++)
			parents[i]->getAllConstructors(cons);
		auto it = varmap.find(classname);
		if (it != varmap.end())
		{
			Bagel_Var &var = it->second;
			if (var.getType() == VAR_FUNC)
			{
				cons->push_back(it->second.forceAsFunc());
			}
		}
	}

	void getAllFinalizers(BKE_array<Bagel_Function*> *finals)
	{
		static Bagel_StringHolder s_finalize(W("finalize"));
		for (int i = 0; i < parents.size(); i++)
			parents[i]->getAllFinalizers(finals);
		auto it = varmap.find(s_finalize);
		if (it != varmap.end())
		{
			Bagel_Var &var = it->second;
			if (var.getType() == VAR_FUNC)
			{
				finals->push_back(it->second.forceAsFunc());
			}
		}
	}

	void VMconstruct(Bagel_Var* params, int paramcount, Bagel_Class* _this, Bagel_ThreadContext* ctx = NULL, bool force = false);

	Bagel_Var VMcreateInstance(Bagel_Var * params, int paramcount, Bagel_ThreadContext * ctx = NULL, bool force = false);

	inline bool isInstanceof(Bagel_StringHolder name) const
	{
		if (classname == name)
			return true;
		for (int i = 0; i < parents.size(); i++)
		{
			if (parents[i]->isInstanceof(name))
				return true;
		}
		return false;
	}
};

//_globalStructures.descr[Parser_Descr::VarClass]
//class还是用Bagel_Class*来引用父类，否则global clear后，所有的类引用都无效
class Bagel_Class :public Bagel_Closure
{
	friend class Bagel_Parser;
	friend class Bagel_NativeClass;
	friend class Bagel_VM;
	friend class Bagel_Var;
	friend class Bagel_Serializer;

	Bagel_Class()
	{
		vt = VAR_CLASS;
	};

public:
	Bagel_ClassDef *defclass;
	Bagel_NativeClass *native;
	Bagel_StringHolder classname;
	bool finalized;
	mutable Bagel_Var tempvar;

	vector<Bagel_Var*, BKE_Allocator<Bagel_Var*>> cache;

	virtual ~Bagel_Class()
	{
		//finalize();
		if (native)
			native->release();
	};
	virtual void markChildren() override
	{
		Bagel_Closure::markChildren();
		_GC.GC_Markself(defclass);
		_GC.GC_Markself(classname.s);
		_GC.GC_Markself(tempvar);
		if (native)
			native->markChildren();
	}
	//instance
	inline Bagel_Class(Bagel_ClassDef *parent) : Bagel_Closure(parent)
	{
		finalized = false;
		classname = parent->classname;
		vt = VAR_CLASS;
		native = NULL;
		defclass = parent;
		cache.resize(parent->classvar.getCount());
		int idx = 0;
		for (auto &&it : parent->classvar)
			cache[idx++] = &(varmap[it.first] = it.second.clone());
		varmap.inserttick = 1;
	};

	void assignStructure(Bagel_Class *cla, BKE_hashmap<void*, void*> &pMap, bool first = false);

	bool finalize()
	{
		//暂时没有好方法解决finalize函数被先注销的问题
		return false;
		//if (!finalized)
		//{
		//	//Bagel_Var *var;
		//	finalized = true;
		//	BKE_array<Bagel_Function*> finals;
		//	defclass->getAllFinalizers(&finals);
		//	for (auto &it : finals)
		//	{
		//		try
		//		{
		//			it->VMRunWithSelf(NULL, 0, this, this);
		//		}
		//		catch (...)
		//		{
		//		}
		//	}
		//	return !finals.empty();
		//}
		//return false;
	}

	inline bool hasClassMember(Bagel_StringHolder key) const
	{
		if (varmap.find(key) != varmap.end())
			return true;
		return defclass->hasClassMember(key);
	}

	inline bool hasClassMember(Bagel_StringHolder key, Bagel_Var **var) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
		{
			(*var) = &it->second;
			return true;
		}
		bool res = defclass->hasClassMember(key, var);
		if (res)
		{
			auto &&v = **var;
			if (v.getType() == VAR_FUNC && v.forceAsFunc()->self.isObject() && v.forceAsFunc()->self.forceAsObject() == defclass)
			{
				tempvar.forceSet(new Bagel_Function(*v.forceAsFunc(), const_cast<Bagel_Class*>(this), const_cast<Bagel_Class*>(this)));
				*var = &tempvar;
			}
			else if (v.getType() == VAR_PROP)
			{
				tempvar.forceSet(new Bagel_Prop(*v.forceAsProp(), const_cast<Bagel_Class*>(this), const_cast<Bagel_Class*>(this)));
				*var = &tempvar;
			}
		}
		return res;
	}

	//获取function时，必须要用value
	Bagel_Var getClassMemberValue(Bagel_StringHolder key) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
			return it->second;
		it = defclass->varmap.find(key);
		if(it == defclass->varmap.end())
		{
			throw Bagel_Except(W("成员") + key.getConstStr() +W("不存在"));
		}
		auto &&v = it->second;
		if (v.getType() == VAR_FUNC && v.forceAsFunc()->self.isObject() && v.forceAsFunc()->self.forceAsObject() == defclass)
		{
			tempvar.forceSet(new Bagel_Function(*v.forceAsFunc(), const_cast<Bagel_Class*>(this), const_cast<Bagel_Class*>(this)));
			return tempvar;
		}
		else if (v.getType() == VAR_PROP)
		{
			return v.forceAsProp()->VMGetWithSelf(this, const_cast<Bagel_Class*>(this), nullptr, true);
		}
		else
		{
			return v;
		}
	}

	void VMgetClassMember(Bagel_StringHolder key, Bagel_Var *out, Bagel_ThreadContext *ctx, bool searchGlobal) const;

	//获取function时，必须要用value
	Bagel_Var getClassMemberValue2(Bagel_StringHolder key) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
			return it->second;
		it = defclass->varmap.find(key);
		if (it == defclass->varmap.end())
		{
			return _globalStructures.globalClosure->getMember(key);
		}
		auto &&v = it->second;
		if (v.getType() == VAR_FUNC && v.forceAsFunc()->self.isObject() && v.forceAsFunc()->self.forceAsObject() == defclass)
		{
			tempvar.forceSet(new Bagel_Function(*v.forceAsFunc(), const_cast<Bagel_Class*>(this), const_cast<Bagel_Class*>(this)));
			return tempvar;
		}
		else if (v.getType() == VAR_PROP)
		{
			return v.forceAsProp()->VMGetWithSelf(this, const_cast<Bagel_Class*>(this), nullptr, true);
		}
		else
		{
			return v;
		}
	}

	//addr无法获取function
	Bagel_Var& getClassMemberAddr(Bagel_StringHolder key) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
			return it->second;
		auto it2 = defclass->varmap.find(key);
		if (it2 == defclass->varmap.end() || it2->second.getType() == VAR_FUNC)
		{
			return varmap[key];
		}
		else if (it2->second.getType() == VAR_PROP)
		{
			tempvar.forceSet(new Bagel_Prop(*it2->second.forceAsProp(), this, this));
			return tempvar;
		}
		else
		{
			return it2->second;
		}
	}

	//最后在全局闭包找的
	Bagel_Var& getClassMemberAddr2(Bagel_StringHolder key) const
	{
		auto it = varmap.find(key);
		if (it != varmap.end())
			return it->second;
		auto it2 = defclass->varmap.find(key);
		if (it2 == defclass->varmap.end())
		{
			return _globalStructures.globalClosure->getMember(key);
		}
		else if (it2->second.getType() == VAR_FUNC)
		{
			return varmap[key];
		}
		else if (it2->second.getType() == VAR_PROP)
		{
			tempvar = new Bagel_Prop(*it2->second.forceAsProp(), this, this);
			return tempvar;
		}
		else
		{
			return it2->second;
		}
		return varmap[key];
	}

	virtual bool hasMember(Bagel_StringHolder key) const override
	{
		if (!hasClassMember(key))
		{
			//then we find it on global
			return _globalStructures.globalClosure->hasMember(key);
		}
		return true;
	}
	virtual bool hasMember(Bagel_StringHolder key, Bagel_Var **var) const override
	{
		if (!hasClassMember(key, var))
		{
			//then we find it on global
			return _globalStructures.globalClosure->hasMember(key, var);
		}
		return true;
	}

	inline Bagel_Var getSuperMember(Bagel_StringHolder key)
	{
		return defclass->getSuperMember(key, this);
	}

	inline Bagel_Var& getSuperMemberAddr(Bagel_StringHolder key)
	{
		return defclass->getSuperMemberAddr(key);
	}

	inline bool isInstanceof(Bagel_StringHolder name) const
	{
		return defclass->isInstanceof(name);
	}
};

inline void Bagel_ClassDef::VMconstruct(Bagel_Var *params, int paramcount, Bagel_Class *_this, Bagel_ThreadContext *ctx, bool force)
{
	if (native)
	{
		//override parent class's nativeclass
		if (_this->native)
			delete _this->native;
		if (!force)
		{
			_this->native = this->native->nativeCreateNew(_this, params, paramcount, ctx);
		}
		else
			_this->native = this->native->nativeCreateNULL();
		if (!_this->native)
		{
			throw Bagel_Except(W("不允许创建该类（") + classname.getConstStr() + W("）的实例"));
		}
	}
	//else
	if (!force)	//loadClass的时候不经过构造函数
	{
		BKE_array<Bagel_Function*> cons;
		getAllConstructors(&cons);
		for (int i = cons.size() - 1; i >= 0; i--)
			cons[i]->VMRunWithSelf(params, paramcount, _this, _this, ctx);
	}
}

inline Bagel_Var Bagel_ClassDef::VMcreateInstance(Bagel_Var *params, int paramcount, Bagel_ThreadContext *ctx, bool force)
{
	if (innerCreateInstance != NULL)
	{
		return (*innerCreateInstance)(NULL, params, paramcount, (Bagel_Closure*)static_cast<const Bagel_Closure*>(this), ctx);
	}
	if (cannotcreate)
		throw Bagel_Except(W("不允许创建该类（") + classname.getConstStr() + W("）的实例"));
	Bagel_Class* cla = new Bagel_Class(this);
	VMconstruct(params, paramcount, cla, ctx, force);
	return cla;
}

inline void Bagel_NativeClass::_prepareClass(const u16string &name, bool isFinal)
{
	auto &v = Bagel_Closure::global()->getMember(name);
	if (v.getType() != VAR_CLASSDEF)
	{
		if (_class)
			v.forceSet(new Bagel_ClassDef(name, ((Bagel_ClassDef*)_class)->classname));
		else
			v.forceSet(new Bagel_ClassDef(name));
		if (v.forceAsClassDef()->native)
			delete v.forceAsClassDef()->native;
		v.forceAsClassDef()->native = this;
		v.forceAsClassDef()->isFinal = isFinal;
	}
	_class = v.forceAsClassDef();
}

inline Bagel_Closure *Bagel_Closure::getThisClosure()
{
	auto p = this;
	while (p && p->vt != VAR_CLASS && p->vt != VAR_CLASSDEF)
		p = p->parent;
	return p;
}

inline Bagel_Var& Bagel_Var::operator *= (double v)
{
	if (getType() == VAR_PROP)
	{
		auto &&p = static_cast<Bagel_Prop*>(obj);
		if (p->hasGet() && p->hasSet())
			p->VMSet(p->VMGet() * v);
		return *this;
	}
	toNumber();
	num *= v;
	return *this;
};
inline Bagel_Var& Bagel_Var::operator /= (double v)
{
	if (getType() == VAR_PROP)
	{
		auto &&p = static_cast<Bagel_Prop*>(obj);
		if (p->hasGet() && p->hasSet())
			p->VMSet(p->VMGet() / v);
		return *this;
	}
	toNumber();
	num /= v;
	return *this;
};
inline Bagel_Var& Bagel_Var::operator %= (double v)
{
	if (getType() == VAR_PROP)
	{
		auto &&p = static_cast<Bagel_Prop*>(obj);
		if (p->hasGet() && p->hasSet())
			p->VMSet(p->VMGet() % v);
		return *this;
	}
	toNumber();
	num = fmod(num, v);
	return *this;
};
inline Bagel_Var& Bagel_Var::operator ^= (double v)
{
	if (getType() == VAR_PROP)
	{
		auto &&p = static_cast<Bagel_Prop*>(obj);
		if (p->hasGet() && p->hasSet())
			p->VMSet(p->VMGet() ^ v);
		return *this;
	}
	toNumber();
	num = pow(num, v);
	return *this;
};

template<typename... Args>
Bagel_Var Bagel_Var::run(Bagel_Var &self, Args&&...args)
{
	if (getType() != VAR_FUNC)
		return Bagel_Var();
	auto *v = (Bagel_Function*)obj;
	if (self)
		v->setSelf(self);
	auto arr = new Bagel_Array();
	return _run(arr, std::forward<Args>(args)...);
}

template<typename... Args>
Bagel_Var Bagel_Var::run(Bagel_Object *o, Args&&...args)
{
	if (getType() != VAR_FUNC)
		return Bagel_Var();
	auto *v = (Bagel_Function*)obj;
	if (o)
	{
		Bagel_Var self = o;
		v->setSelf(self);
	}
	Bagel_Vector vec;
	return _run(vec, std::forward<Args>(args)...);
}

template<typename Param, typename... Args>
Bagel_Var Bagel_Var::_run(Bagel_Vector &paramvec, Param &&head, Args&&... rest)
{
	paramvec.emplace_back(std::forward<Param>(head));
	return _run(paramvec, std::forward<Args>(rest)...);
}
//end

template<typename Param>
Bagel_Var Bagel_Var::_run(Bagel_Vector &paramvec, Param &&head)
{
	paramvec.emplace_back(std::forward<Param>(head));
	return ((Bagel_Function*)obj)->run(paramvec.data(), paramvec.size());
}

FORCEINLINE Bagel_Var::Bagel_Var(Bagel_NativeFunction func)
{
	vt = VAR_FUNC;
	//is_var = VARIABLE_VAR;
	obj = new Bagel_Function(func);
}

FORCEINLINE Bagel_Var Bagel_Var::array(int size)
{
	auto arr = new Bagel_Array();
	arr->setLength(size);
	return arr;
}

FORCEINLINE Bagel_Var Bagel_Var::dic()
{
	return new Bagel_Dic();
}


class Bagel_VarHandler
{
	friend class Bagel_Var;
protected:
	Bagel_Var inner;

public:
	Bagel_VarHandler()
	{
		_GC.addRoot2(&inner);
	};

	Bagel_VarHandler(const Bagel_Var& v) : inner(v)
	{
		_GC.addRoot2(&inner);
	}

	Bagel_VarHandler(const Bagel_VarHandler& v) : inner(v.inner)
	{
		_GC.addRoot2(&inner);
	}

	~Bagel_VarHandler()
	{
		_GC.removeRoot2(&inner);
	}

	operator Bagel_Var () const
	{
		return inner;
	}

	void operator = (const Bagel_Var &v)
	{
		inner = v;
		_GC.root2Changed(&inner);
	}

	//dangerous, need to call _GC.writeBarrier()
	Bagel_Var* operator -> ()
	{
		return &inner;
	}

	const Bagel_Var* operator -> () const
	{
		return &inner;
	}

	//dangerous, need to call _GC.writeBarrier()
	Bagel_Var &operator * ()
	{
		return inner;
	}

	const Bagel_Var &operator * () const
	{
		return inner;
	}

	/// <summary>
	/// 与另一个变量比较。执行严格比较。
	/// </summary>
	/// <param name="other">另一个变量。</param>
	/// <returns>严格相等则返回true，否则返回false。</returns>
	bool operator == (const Bagel_Var& other) const
	{
		return inner.strictEqual(other);
	}

	/// <summary>
	/// 与另一个变量比较。执行严格比较。
	/// </summary>
	/// <param name="other">另一个变量。</param>
	/// <returns>不严格相等则返回true，否则返回false。</returns>
	bool operator != (const Bagel_Var& other) const
	{
		return !inner.strictEqual(other);
	}

	/// <summary>
	/// 与另一个变量比较。执行普通比较。
	/// </summary>
	/// <param name="other">另一个变量。</param>
	/// <returns>相等则返回true，否则返回false。</returns>
	bool normalEqual(const Bagel_Var& other) const
	{
		return inner.normalEqual(other);
	}
};

template<>
inline int32_t bke_hash::BKE_hash(const Bagel_Var &var)
{
	switch (var.getType())
	{
	case VAR_NONE:
		return 0;
	case VAR_NUM:
		return BKE_hash(var.forceAsNumber());
	case VAR_STR:
		return BKE_hash(var.forceAsBKEStr());
	default:
		return (int32_t)(intptr_t)var.forceAsObject();
	}
}

#pragma pop_macro("new")