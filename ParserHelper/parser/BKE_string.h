#pragma once

//on different platforms, we use different wchar-type
#include <vector>
#include <string>

#include <mutex>
typedef std::recursive_mutex BKE_WrapperMutex;

#ifndef WCHAR_MAX
#include <wchar.h>
#endif

typedef std::wstring StringVal;
//global string pools
#include "BKE_hash.hpp"

//other functions
#include "BKE_number.h"

struct StringType
{
	StringVal str;
	int ref;
	bkplong hash;
	bool hashed;
	wstring printStr;
	bool printStrAvailable;

	//do nothing
	inline StringType(){};

	//because a Stringtype is unique
	StringType(const StringType &s) = delete;
	StringType& operator =(const StringType &) = delete;

	//although copy is banned, move is enabled
	//used to construct hashed string from unhash one
	StringType(StringType &&s)
	{
		str = std::move(s.str);
		//ref = s.ref;
		//hash = s.hash;
		//hashed = s.hashed;
		ref = 1;
		hashed = true;
		printStr = std::move(s.printStr);
		printStrAvailable = s.printStrAvailable;
	}
	StringType& operator =(StringType &&s)
	{
		str = std::move(s.str);
		//ref = s.ref;
		//hash = s.hash;
		//hashed = s.hashed;
		ref = 1;
		hashed = true;
		printStr = std::move(s.printStr);
		printStrAvailable = s.printStrAvailable;
		return *this;
	};

	inline operator const wchar_t* () const
	{
		return str.c_str();
	}

	inline bool operator == (const StringType &s) const
	{
		return (this->hashed && s.hashed) ? this == &s : str == s.str;
	}

	inline bool operator == (const wchar_t *s) const
	{
		return str == s;
	}

	inline StringType(const wchar_t *s) :str(s)
	{
		ref = 0;
		printStrAvailable = false;
	}

	inline StringType(wstring &&s)
	{
		str = std::move(s);
		ref = 0;
		printStrAvailable = false;
	}

};

class BKE_Number;

class BKE_String
{
	friend int32_t BKE_hash(const BKE_String &str);
	StringType *var;
public:

	inline BKE_String();
	inline BKE_String(const StringType *s){ var = (StringType *)s; }
	inline BKE_String(const wchar_t *str, bool hash = true);
	inline BKE_String(const std::wstring &str, bool hash = true);
	inline BKE_String(const BKE_String &str)
	{
		var = str.var;
		var->ref++;
	}

	inline BKE_String(BKE_String &&str);

	inline BKE_String(std::wstring &&str);

	//construct from number;
	BKE_String(const BKE_Number &num);

	inline ~BKE_String()
	{
		if (!var->hashed && var->ref <= 1)
			delete var;
		else
			var->ref--;
	}

	inline void changeToHashed();

	//to number
	inline BKE_Number asNumber() const
	{
		return str2num(var->str.c_str());
	}

	const wchar_t *c_str() const
	{
		return var->str.c_str();
	}

	inline bool canBeNumber() const
	{
		wchar_t *end;
		str2num(var->str.c_str(),&end);
		return end == var->str.c_str() + var->str.size();
	}

	inline bool empty() const;


	inline bool operator == (const BKE_String &s) const
	{
		return *var == *s.var;
	}

	inline bool operator == (const wchar_t *s) const
	{
		return var->str.c_str() == s;
	}

	inline bool operator != (const BKE_String &s) const
	{
		return !(*var == *s.var);
	}

	inline bool operator >= (const BKE_String &s) const
	{
		if (empty())
			return !s.empty();
		if (s.empty())
			return true;
		return getConstStr() >= s.getConstStr();
	}

	inline bool operator <= (const BKE_String &s) const
	{
		if (empty())
			return true;
		if (s.empty())
			return empty();
		return getConstStr() <= s.getConstStr();
	}

	inline bool operator < (const BKE_String &s) const
	{
		return !(operator >=(s));
	}

	inline bool operator > (const BKE_String &s) const
	{
		return !(operator <=(s));
	}

	inline BKE_String& operator =(const BKE_String &s)
	{
		if (var == s.var)
			return *this;
		if (!var->hashed && var->ref <= 1)
			delete var;
		else
			var->ref--;
		var = s.var;
		var->ref++;
		return *this;
	}

	inline operator const wstring & () const { return var->str; };

	inline void setEmpty();

	inline bkplong size() const{ return var->str.size(); };

	inline bool isVoid() const { return empty(); }

	inline BKE_String& operator =(BKE_String &&s);

	inline BKE_String operator + (const wstring &s) const{ return getConstStr() + s;}

	inline BKE_String operator + (const BKE_String &s) const{ return getConstStr() + s.getConstStr(); }

	inline BKE_String& operator += (const wstring &s){ wstring ss = getConstStr() + s; *this = ss; return *this; }

	inline BKE_String& operator += (const BKE_String &s){ wstring ss = getConstStr() + s.getConstStr(); *this = ss; return *this; }

	inline wchar_t& operator[](int n) const{ return (wchar_t &)getConstStr()[n]; }

	inline wstring getStrCopy() const{ return getConstStr(); }

	inline wstring toLowerCase() const { wstring a = getStrCopy(); for (auto &i : a){ if (i <= L'Z' && i >= L'A') i |= 0x20; } return a; }

	inline wstring toUpperCase() const { wstring a = getStrCopy(); for (auto &i : a){ if (i <= L'z' && i >= L'a') i &= 0xDF; } return a; }

	inline const wstring& getConstStr() const { return var->str; }

	inline wstring substr(bkplong start=0, bkpulong count = wstring::npos) const
	{
		return getConstStr().substr(start, count);
	}

	BKE_String& operator =(const std::wstring &s);
	BKE_String& operator =(const wchar_t *s);
	inline BKE_String& operator =(const StringType *s)
	{
		if (!var->hashed && var->ref <= 1)
			delete var;
		else
			var->ref--;
		var = (StringType *)s;
		return *this;
	}
	const wstring& printStr() const;
};

//special hash function
template<>
inline int32_t BKE_hash(const StringType &str)
{
	return str.hashed ? str.hash : BKE_hash(str.str);
}

class GlobalStringMap :protected BKE_hashset<StringType, 12>
{
private:
	BKE_WrapperMutex mu;
	StringType nullString;

public:
	inline GlobalStringMap() :nullString(L""){ 
		nullString.hashed = true; nullString.hash = 0;
	};

	inline StringType *allocNullString()
	{
		//nullString.ref++;
		return &nullString;
	}

	StringType *allocString(const wchar_t *str);

	StringType *allocHashString(const wchar_t *str);

	StringType *hashString(StringType &&s);

	StringType *allocHashString(wstring &&str);

	StringType *allocString(wstring &&str);

	void forceGC();

	inline void GC()
	{
		if (getCount() > 4096)
			forceGC();
	}
};

inline GlobalStringMap& StringMap()
{
	return *_globalStructures.globalStringMap;
}

inline int32_t BKE_hash(const BKE_String &str)
{
	if (!str.var->hashed)
	{
		BKE_String *_s = (BKE_String *)&str;
		auto back = str.var;
		if (back->ref <= 1)
		{
			_s->var = StringMap().hashString(std::move(*str.var));
			delete back;
		}
		else
		{
			_s->var = StringMap().allocHashString(*str.var);
			back->ref--;
		}
	}
	return str.var->hash;
}

inline BKE_String::BKE_String(const wchar_t *str, bool hash)
{
	if(hash)
		var = StringMap().allocHashString(str);
	else
		var = StringMap().allocString(str);
}

inline BKE_String::BKE_String(const std::wstring &str, bool hash)
{
	if (hash)
		var = StringMap().allocHashString(str.c_str());
	else
		var = StringMap().allocString(str.c_str());
}

inline BKE_String& BKE_String::operator = (const std::wstring &s)
{
	auto v = StringMap().allocString(s.c_str());
	if (!var->hashed && var->ref <= 1)
		delete var;
	else
		var->ref--;
	var = v;
	return *this;
}

inline BKE_String& BKE_String::operator =(const wchar_t *s)
{
	auto v = StringMap().allocString(s);
	if (!var->hashed && var->ref <= 1)
		delete var;
	else
		var->ref--;
	var = v;
	return *this;
}

inline BKE_String::BKE_String()
{
	var = StringMap().allocNullString();
}

inline BKE_String::BKE_String(BKE_String &&str)
{
	var = str.var;
	str.var = StringMap().allocNullString();
}

inline BKE_String& BKE_String::operator = (BKE_String &&s)
{
	if (var == s.var)
		return *this;
	if (!var->hashed && var->ref <= 1)
		delete var;
	else
		var->ref--;
	var = s.var;
	s.var = StringMap().allocNullString();
	return *this;
}

inline bool BKE_String::empty() const
{
	return var == StringMap().allocNullString() || var->str[0] == L'\0';
}

inline void BKE_String::changeToHashed()
{
	if (var->hashed)
		return;
	auto back = var;
	var = StringMap().allocHashString(std::move(*var));
	back->ref--;
	if (back->ref <= 0)
		delete back;
}

inline void BKE_String::setEmpty()
{
	if (!var->hashed && var->ref <= 1)
		delete var;
	else
		var->ref--;
	var = StringMap().allocNullString();
}


inline BKE_String::BKE_String(std::wstring &&str)
{
	var = StringMap().allocString(std::move(str));
}
