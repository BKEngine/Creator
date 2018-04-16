#pragma once

#include <vector>
#include <string>

#include <mutex>
typedef std::recursive_mutex BKE_WrapperMutex;

#include "Bagel_Utils.h"

#ifndef WCHAR_MAX
#include <wchar.h>
#endif

//global string pools
#include "BKE_hash.hpp"

//other functions
#include "Bagel_Number.h"
//Bagel_Object
#include "Bagel_GC.h"

#pragma push_macro("new")
#undef new

/// <summary>
/// 字符串对象类。
/// 用户不能主动创建这个类的实例，应通过 Bagel_VM::getStringRef(const StringVal &str) 来获得一个字符串的引用。
///  Bagel_StringHolder 辅助类用于存放该类对象的指针。
/// </summary>
/// <seealso cref="Bagel_Object" />
class Bagel_String : public Bagel_Object
{
	friend int32_t _BKE_hash(const Bagel_String &str);
	friend struct GlobalStructures;
	friend class GlobalStringMap;
	friend void initFieldData();

protected:
	StringVal str;
	mutable int32_t hash;
	bool pool;

	inline Bagel_String() : Bagel_Object(VAR_STR), hash(0), pool(true)
	{
	};
	inline Bagel_String(const std::u16string &str, bool p = true) : Bagel_Object(VAR_STR), hash(0), pool(p)
	{
		this->str = str;
	}
	inline Bagel_String(std::u16string &&str, bool p = true) : Bagel_Object(VAR_STR), hash(0), pool(p)
	{
		this->str = std::move(str);
	}
	//construct from number;
	Bagel_String(double num) : Bagel_Object(VAR_STR), hash(0)
	{
		str = Bagel_Number::toString(num);
	};

public:
	Bagel_String(const Bagel_String &s) = delete;
	Bagel_String(Bagel_String &&s) = delete;

	virtual ~Bagel_String();

	virtual void markChildren()
	{
	};

	void* operator new (size_t size)
	{
		void* p = BagelMalloc(size);
		_GC.addNewString(p);
		return p;
	}

	void* operator new (size_t size, const char *, int){ return operator new(size); };

	void operator delete (void *p)
	{
		//不能手动delete
		assert(false);
	};

	void operator delete (void *p, bool)
	{
		BagelFree(p);
	};

	void operator delete (void *p, const char *, int)
	{
		return operator delete(p);
	};

	//to number
/// <summary>
/// 将字符串转为数值，支持十进制浮点和十六进制浮点。转换会一直到非法的字符或结束。
/// </summary>
/// <returns>转换的结果</returns>
	inline double asNumber() const noexcept
	{
		return Bagel_Number::str2num(str.c_str());
	}

	/// <summary>
	/// 返回字符串的char16_t*的地址。
	/// </summary>
	/// <returns>字符串地址。</returns>
	const BKE_Char *c_str() const noexcept
	{
		return str.c_str();
	}

	/// <summary>
	/// 判断这个字符串是否可以全部被转化为数值。
	/// </summary>
	/// <returns>如果转化完后面还有非法字符，则返回false，否则返回true。</returns>
	inline bool canBeNumber() const noexcept
	{
		BKE_Char *end;
		Bagel_Number::str2num(str.c_str(), &end);
		return !*end;
	}

	static bool canBeNumber(const u16string &u) noexcept
	{
		BKE_Char *end;
		Bagel_Number::str2num(u.c_str(), &end);
		return !*end;
	}

	static bool canBeNumber(const char16_t *str, int size) noexcept
	{
		return canBeNumber(u16string(str, size));
	}

	/// <summary>
	/// 判断字符串是否为空。
	/// </summary>
	/// <returns>为空则返回true，否则返回false。</returns>
	inline bool empty() const noexcept
	{
		return str.empty();
	};

	/// <summary>
	/// 和另一个字符串比较。
	/// </summary>
	/// <param name="s">另一个字符串。</param>
	/// <returns>是否相同</returns>
	inline bool operator == (const Bagel_String &s) const noexcept
	{
		//字符串池唯一副本
		if (pool && s.pool)
			return this == &s;
		//字符串池和非字符串池是按长度区分的，因此必定不同
		else if (pool || pool)
			return false;
		//都是非字符串池，硬比较
		else
			return str.length() == s.str.length() && str == s.str;
	}

	/// <summary>
	/// 和另一个字符串比较。
	/// </summary>
	/// <param name="s">另一个字符串。</param>
	/// <returns>是否不同</returns>
	inline bool operator != (const Bagel_String &s) const noexcept
	{
		return !operator == (s);
	}

	/// <summary>
	/// 和另一个字符串比较大小。
	/// </summary>
	/// <param name="s">另一个字符串。</param>
	/// <returns>若大于等于另一个字符串则返回true，否则返回false。</returns>
	inline bool operator >= (const Bagel_String &s) const noexcept
	{
		return str >= s.str;
	}

	/// <summary>
	/// 和另一个字符串比较大小。
	/// </summary>
	/// <param name="s">另一个字符串。</param>
	/// <returns>若小于等于另一个字符串则返回true，否则返回false。</returns>
	inline bool operator <= (const Bagel_String &s) const noexcept
	{
		return str <= s.str;
	}

	/// <summary>
	/// 和另一个字符串比较大小。
	/// </summary>
	/// <param name="s">另一个字符串。</param>
	/// <returns>若小于另一个字符串则返回true，否则返回false。</returns>
	inline bool operator < (const Bagel_String &s) const noexcept
	{
		return !(operator >=(s));
	}

	/// <summary>
	/// 和另一个字符串比较大小。
	/// </summary>
	/// <param name="s">另一个字符串。</param>
	/// <returns>若大于另一个字符串则返回true，否则返回false。</returns>
	inline bool operator > (const Bagel_String &s) const noexcept
	{
		return !(operator <=(s));
	}

	/// <summary>
	/// 转化为 std::u16string 。
	/// </summary>
	/// <returns>
	/// 转化的结果。
	/// </returns>
	inline operator const StringVal & () const noexcept
	{
		return str;
	};

	/// <summary>
	/// 获取字符串长度。
	/// </summary>
	/// <returns>字符串的长度。</returns>
	inline int32_t size() const noexcept
	{
		return (int32_t)str.size();
	};

	/// <summary>
	/// 判断字符串是否为空，同empty。
	/// </summary>
	/// <returns>为空则返回true，否则返回false。</returns>
	inline bool isVoid() const noexcept
	{
		return empty();
	};

	/// <summary>
	/// 取第n个字符。
	/// </summary>
	/// <param name="n">要取的字符的位置。</param>
	/// <returns>字符串中第n的字符。</returns>
	inline BKE_Char operator[](int n) noexcept
	{
		return getConstStr()[n];
	};

	/// <summary>
	/// 取第n个字符，返回的是由这一个字符组成的字符串。
	/// </summary>
	/// <param name="n">要取的字符的位置。</param>
	/// <returns>由字符串中第n的字符这一个字符组成的字符串。</returns>
	inline StringVal getElement(int n) noexcept
	{
		auto &&ss = getConstStr();
		if (n < 0)
			n += ss.length();
		if (n < 0 || n >= ss.length())
			return W("");
		return StringVal(1, ss[n]);
	}

	/// <summary>
	/// 获取对应的宽字符串。
	/// </summary>
	/// <returns>字符串的 std::wstring 版本。</returns>
	inline wstring getWString() const noexcept
	{
		return UniFromUTF16(str.c_str(), str.size());
	};

	/// <summary>
	/// 获取对应的 std::u16string 字符串。
	/// </summary>
	/// <returns> std::u16string 字符串。</returns>
	inline const StringVal& getConstStr() const noexcept
	{
		return str;
	};

	/// <summary>
	/// 返回字符串的子串。
	/// </summary>
	/// <param name="start">开始位置。</param>
	/// <param name="count">子串的长度，默认为-1，-1表示取到结尾。</param>
	/// <returns>子串。</returns>
	inline StringVal substr(int32_t start = 0, uint32_t count = (uint32_t)-1) const noexcept
	{
		return str.substr(start, count);
	}

	/// <summary>
	/// 返回字符串的显示的版本（可解析为原字符串的版本）。
	/// 对于普通字符串为两边加上双引号（内容中的双引号换成两个双引号，参见Bagel的双引号字符串的定义）。
	/// 如果有其他字符，则返回单引号版本的字符串，内容中的特殊字符都用转义来表示。
	/// </summary>
	/// <returns>返回的可显示的版本，直接解析这个返回值就可以得到原字符串内容。</returns>
	StringVal printStr() const noexcept;

	bool beginWith(const StringVal &dst) const noexcept
	{
		if (dst.length() == 0)
			return true;
		if (dst.length() > str.length())
			return false;
		return memcmp(str.c_str(), dst.c_str(), dst.length() * sizeof(BKE_Char)) == 0;
	}

	bool endWith(const StringVal &dst) const noexcept
	{
		if (dst.length() == 0)
			return true;
		int offset = str.length() - dst.length();
		if (offset < 0)
			return false;
		return memcmp(str.c_str() + offset, dst.c_str(), dst.length() * sizeof(BKE_Char)) == 0;
	}
};

inline int32_t _BKE_hash(const Bagel_String &str)
{
	if (!str.hash)
	{
		str.hash = BKE_hash(str.str);
	}
	return str.hash;
}

template<>
inline int32_t bke_hash::BKE_hash(const Bagel_String &str)
{
	return _BKE_hash(str);
}

class Bagel_Var;

/// <summary>
/// 辅助类，存放 Bagel_String 的指针。
/// </summary>
class Bagel_StringHolder
{
public:
	/// <summary>
	/// 存放的 Bagel_String 的指针。
	/// </summary>
	Bagel_String *s;

	/// <summary>
	/// 默认构造函数，存放一个空字符串。
	/// </summary>
	Bagel_StringHolder() noexcept;

	/// <summary>
	/// null构造函数，存放一个空字符串。
	/// </summary>
	Bagel_StringHolder(nullptr_t) noexcept;

	/// <summary>
	/// 存放一个 Bagel_String 指针。
	/// </summary>
	/// <param name="ss"> Bagel_String 字符串指针。</param>
	Bagel_StringHolder(const Bagel_String *const ss) noexcept :s((Bagel_String*)ss)
	{
	};

	/// <summary>
	/// 拷贝构造函数。
	/// </summary>
	/// <param name="ss">另一个 Bagel_StringHolder 对象。</param>
	Bagel_StringHolder(const Bagel_StringHolder& ss) noexcept :s(ss.s)
	{
	};

	/// <summary>
	/// 从一个Bagel_Var对象中获得字符串。如果对象不能转化为字符串，会抛出异常。
	/// </summary>
	/// <param name="v"> Bagel_Var 对象。</param>
	explicit Bagel_StringHolder(const Bagel_Var& v) noexcept;

	/// <summary>
	/// 从wchar_t*的字符串构建 Bagel_StringHolder 对象。
	/// </summary>
	/// <param name="str">字符串。</param>
	Bagel_StringHolder(const wchar_t *str) noexcept;

	/// <summary>
	/// 从 std::wstring 的字符串构建 Bagel_StringHolder 对象。
	/// </summary>
	/// <param name="str">字符串。</param>
	Bagel_StringHolder(const std::wstring &str) noexcept;

	/// <summary>
	/// 从char16_t*的字符串构建 Bagel_StringHolder 对象。
	/// </summary>
	/// <param name="str">字符串。</param>
	Bagel_StringHolder(const char16_t *str) noexcept;

	/// <summary>
	/// 从 std::u16string 的字符串构建 Bagel_StringHolder 对象。
	/// </summary>
	/// <param name="str">字符串。</param>
	Bagel_StringHolder(const std::u16string &str) noexcept;

	/// <summary>
	/// 从 std::u16string&& 的字符串构建 Bagel_StringHolder 对象。
	/// </summary>
	/// <param name="str">字符串。</param>
	Bagel_StringHolder(std::u16string &&str) noexcept;

	//construct from number;
/// <summary>
/// 从数字构建 Bagel_StringHolder 对象。
/// </summary>
/// <param name="num">数字。</param>
	Bagel_StringHolder(double num) noexcept;

	//construct from integer;
	/// <summary>
	/// 从整数构建 Bagel_StringHolder 对象。
	/// </summary>
	/// <param name="num">数字。</param>
	Bagel_StringHolder(int num) noexcept;

	/// <summary>
	/// 判断字符串是否为空。
	/// </summary>
	/// <returns>为空则返回true，否则返回false。</returns>
	inline bool empty() const noexcept
	{
		return s->empty();
	};

	/// <summary>
	/// 获取字符串长度。
	/// </summary>
	/// <returns>字符串的长度。</returns>
	inline size_t size() const noexcept
	{
		return s->size();
	}

	/// <summary>
	/// 获取 std::u16string 常量字符串。
	/// </summary>
	/// <returns>字符串。</returns>
	inline const StringVal& getConstStr() const noexcept
	{
		return s->getConstStr();
	};

	/// <summary>
	/// 获取 std::wstring 字符串。
	/// </summary>
	/// <returns>字符串。</returns>
	inline wstring getStrCopy() const noexcept
	{
		return s->getWString();
	};

	/// <summary>
	/// 获取字符串转化为的数值。
	/// </summary>
	/// <returns>数值</returns>
	inline double asNumber() const noexcept
	{
		return s->asNumber();
	}

	/// <summary>
	/// 判断字符串是否可以转化为数值。
	/// </summary>
	/// <returns>布尔</returns>
	inline bool canBeNumber() const noexcept
	{
		return s->canBeNumber();
	}

	/// <summary>
	/// 获取字符串首个字符的地址。
	/// </summary>
	/// <returns>首地址，字符串以\0结尾。</returns>
	const BKE_Char *c_str() const noexcept
	{
		return s->c_str();
	}

	operator const u16string& () const noexcept
	{
		return s->getConstStr();
	}

	operator wstring () const noexcept
	{
		return s->getWString();
	}
	
	/// <summary>
	/// 获取小写版本的字符串。
	/// </summary>
	/// <returns>小写版本的字符串。</returns>
	inline Bagel_StringHolder toLowerCase() noexcept
	{
		auto s = getConstStr();
		for (auto &i : s)
		{
			if (i <= 'Z' && i >= 'A')
				i |= 0x20;
		}
		return s;
	};

	/// <summary>
	/// 获取大写版本的字符串。
	/// </summary>
	/// <returns>大写版本的字符串。</returns>
	inline Bagel_StringHolder toUpperCase() noexcept
	{
		auto s = getConstStr();
		for (auto &i : s)
		{
			if (i <= 'z' && i >= 'a')
				i &= 0xDF;
		}
		return s;
	};

	bool beginWith(const StringVal &dst) const noexcept
	{
		return s->beginWith(dst);
	}

	bool endWith(const StringVal &dst) const noexcept
	{
		return s->endWith(dst);
	}

	/// <summary>
	/// 获取字符串中v对应位置的字符。
	/// </summary>
	/// <param name="v">位置。</param>
	/// <returns>对应位置的字符。</returns>
	char16_t operator [] (int v) const noexcept
	{
		return getConstStr()[v];
	}
};

/// <summary>
/// Bagel_StringHolder 的加法。
/// </summary>
/// <param name="str1">字符串1。</param>
/// <param name="str2">字符串2。</param>
/// <returns>相加后的字符串</returns>
inline Bagel_StringHolder operator + (const Bagel_StringHolder &str1, const Bagel_StringHolder &str2) noexcept
{
	return str1.getConstStr() + str2.getConstStr();
}

/// <summary>
/// Bagel_StringHolder 的比较。
/// </summary>
/// <param name="str1">字符串1。</param>
/// <param name="str2">字符串2。</param>
/// <returns>等于则返回true，否则返回false。</returns>
inline bool operator == (const Bagel_StringHolder &s1, const Bagel_StringHolder &s2) noexcept
{
	return *s1.s == *s2.s;
}

/// <summary>
/// Bagel_StringHolder 的比较。
/// </summary>
/// <param name="str1">字符串1。</param>
/// <param name="str2">字符串2。</param>
/// <returns>不等于则返回true，否则返回false。</returns>
inline bool operator != (const Bagel_StringHolder &s1, const Bagel_StringHolder &s2) noexcept
{
	return *s1.s != *s2.s;
}

/// <summary>
/// Bagel_StringHolder 的比较。
/// </summary>
/// <param name="str1">字符串1。</param>
/// <param name="str2">字符串2。</param>
/// <returns>字符串1大于等于字符串2则返回true，否则返回false。</returns>
inline bool operator >= (const Bagel_StringHolder &s1, const Bagel_StringHolder &s2) noexcept
{
	return *s1.s >= *s2.s;
}

/// <summary>
/// Bagel_StringHolder 的比较。
/// </summary>
/// <param name="str1">字符串1。</param>
/// <param name="str2">字符串2。</param>
/// <returns>字符串1小于等于字符串2则返回true，否则返回false。</returns>
inline bool operator <= (const Bagel_StringHolder &s1, const Bagel_StringHolder &s2) noexcept
{
	return *s1.s <= *s2.s;
}

/// <summary>
/// Bagel_StringHolder 的比较。
/// </summary>
/// <param name="str1">字符串1。</param>
/// <param name="str2">字符串2。</param>
/// <returns>字符串1小于字符串2则返回true，否则返回false。</returns>
inline bool operator < (const Bagel_StringHolder &s1, const Bagel_StringHolder &s2) noexcept
{
	return *s1.s > *s2.s;
}

/// <summary>
/// Bagel_StringHolder 的比较。
/// </summary>
/// <param name="str1">字符串1。</param>
/// <param name="str2">字符串2。</param>
/// <returns>字符串1大于字符串2则返回true，否则返回false。</returns>
inline bool operator > (const Bagel_StringHolder &s1, const Bagel_StringHolder &s2) noexcept
{
	return *s1.s < *s2.s;
}

template<>
inline int32_t bke_hash::BKE_hash(const Bagel_StringHolder &str)
{
	return _BKE_hash(*str.s);
}

class GlobalStringMap : protected BKE_hashset<Bagel_String*>
{
	friend class Bagel_String;
protected:
	void removeString(Bagel_String *s)
	{
		erase(s);
	}

	template<class T>
	Bagel_String *getNode(T &&key) noexcept
	{
		int32_t ha = BKE_hash(key);
		int32_t h = ha & (hashsize - 1);
		if (buf[h] == NULL)
		{
			//buf[h] = al.op_new(std::forward<T>(key));
			buf[h] = al.allocate();
			new (buf[h]) BKE_HashNode(new Bagel_String(std::forward<T>(key), true));
			buf[h]->index = h;
			buf[h]->hashvalue = ha;
			buf[h]->next = start.next;
			buf[h]->last = &start;
			buf[h]->ct.first->hash = ha;
			start.next->last = buf[h];
			start.next = buf[h];
			count++;
			if (count > hashsize)
			{
				auto res = buf[h];
				resizeTableSize(hashsize << 1);
				return res->ct.first;
			}
			return buf[h]->ct.first;
		}
		else
		{
			auto node = buf[h];
			while (node && node->index == h)
			{
				if (node->ct.first->getConstStr() == key)
					return node->ct.first;
				node = node->next;
			}
			//insert before buf[h]
			//auto newnode = al.op_new(std::forward<T>(key));
			auto newnode = al.allocate();
			new (newnode) BKE_HashNode(new Bagel_String(std::forward<T>(key), true));
			newnode->index = h;
			newnode->hashvalue = ha;
			newnode->next = buf[h];
			newnode->last = buf[h]->last;
			newnode->ct.first->hash = ha;
			buf[h]->last->next = newnode;
			buf[h]->last = newnode;
			buf[h] = newnode;
			count++;
			if (count > hashsize)
			{
				resizeTableSize(hashsize << 1);
			}
			return newnode->ct.first;
		}
	}

public:
	Bagel_String *nullString;

	GlobalStringMap() noexcept
	{
		nullString = allocateString(W(""));
	}

	Bagel_String *allocateString(const char *str) noexcept
	{
		return allocateString(UTF16FromUTF8(str));
	}

	Bagel_String *allocateString(const char16_t *str) noexcept
	{
		return allocateString(std::u16string(str));
	}

	Bagel_String *allocateString(const wchar_t *str) noexcept
	{
		return allocateString(UniToUTF16(str));
	}

	Bagel_String *allocateString(const std::string &str) noexcept
	{
		return allocateString(UTF16FromUTF8(str));
	}

	Bagel_String *allocateString(const std::u16string &str) noexcept
	{
		if (str.length() <= MAX_POOL_STRING_LEN)
			return getNode(str);
		else
			return new Bagel_String(str, false);
	}

	Bagel_String *allocateString(const std::wstring &str) noexcept
	{
		return allocateString(UniToUTF16(str));
	}

	Bagel_String *allocateString(std::u16string &&str) noexcept
	{
		if (str.length() <= MAX_POOL_STRING_LEN)
			return getNode(std::move(str));
		else
			return new Bagel_String(std::move(str), false);
	}
};

/// <summary>
/// Bagel_Handler<T> 的 Bagel_String 特化版本。
/// </summary>
template <>
class Bagel_Handler<Bagel_String>
{
protected:
	Bagel_StringHolder inner;
	friend int32_t _BKE_hash(const Bagel_Handler<Bagel_String> &str);

public:
	/// <summary>
	/// 默认构造函数，代表一个空字符串。
	/// </summary>
	Bagel_Handler()
	{
		_GC.addRoot(inner.s);
	};

	/// <summary>
	/// null构造一个字符串。
	/// </summary>
	/// <param name="v">参数。</param>
	template<class T>
	Bagel_Handler(nullptr_t) : inner(nullptr)
	{
		_GC.addRoot(inner.s);
	};

	/// <summary>
	/// 从v构造一个字符串。
	/// </summary>
	/// <param name="v">参数。</param>
	template<class T>
	Bagel_Handler(T v) : inner(v)
	{
		_GC.addRoot(inner.s);
	};

	/// <summary>
	/// 拷贝构造函数。
	/// </summary>
	/// <param name="h">拷贝对象。</param>
	Bagel_Handler(const Bagel_Handler& h) : inner(h.inner)
	{
		_GC.addRoot(inner.s);
	};

	/// <summary>
	/// 赋值函数
	/// </summary>
	/// <param name="v">赋值对象。</param>
	/// <returns>返回赋值后的字符串。</returns>
	template<class T>
	Bagel_StringHolder operator = (T v)
	{
		_GC.removeRoot(inner.s);
		inner = v;
		_GC.addRoot(inner.s);
		return inner;
	}

	/// <summary>
	/// 赋值函数
	/// </summary>
	/// <param name="v">赋值对象。</param>
	/// <returns>返回赋值后的字符串。</returns>
	Bagel_StringHolder operator = (const Bagel_Handler& h)
	{
		return operator = (h.inner);
	}

	~Bagel_Handler()
	{
	};

	/// <summary>
	/// 返回 Bagel_StringHolder 的指针。
	/// </summary>
	/// <returns>指针。</returns>
	Bagel_StringHolder* operator -> ()
	{
		return &inner;
	}

	const Bagel_StringHolder* operator -> () const
	{
		return &inner;
	}

	/// <summary>
	/// 返回持有的 Bagel_StringHolder 的引用。
	/// </summary>
	/// <returns>持有的 Bagel_StringHolder 对象引用。</returns>
	Bagel_StringHolder& operator * ()
	{
		return inner;
	}

	/// <summary>
	/// 返回 Bagel_StringHolder 对象。
	/// </summary>
	/// <returns>Bagel_StringHolder 对象。</returns>
	Bagel_StringHolder operator * () const
	{
		return inner;
	}

	/// <summary>
	/// 返回持有的 Bagel_StringHolder 的对象值。
	/// </summary>
	/// <returns>持有的 Bagel_StringHolder 对象值。</returns>
	operator Bagel_StringHolder () const
	{
		return inner;
	}

	/// <summary>
	/// 返回持有的 Bagel_String 的对象指针。
	/// </summary>
	/// <returns>持有的 Bagel_String 的对象指针。</returns>
	operator Bagel_String* () const
	{
		return inner.s;
	}

	/// <summary>
	/// 与另一个字符串比较。
	/// </summary>
	/// <param name="other">另一个字符串。</param>
	/// <returns>相同返回true，不同返回false。</returns>
	bool operator == (Bagel_StringHolder other) const
	{
		return inner == other;
	}

	/// <summary>
	/// 与另一个字符串比较。
	/// </summary>
	/// <param name="other">另一个字符串。</param>
	/// <returns>相同返回false，不同返回true。</returns>
	bool operator != (Bagel_StringHolder other) const
	{
		return inner != other;
	}

	/// <summary>
	/// 与另一个字符串比较。
	/// </summary>
	/// <param name="other">另一个字符串。</param>
	/// <returns>大于另一个字符串返回true，否则返回false。</returns>
	bool operator > (Bagel_StringHolder other) const
	{
		return inner > other;
	}

	/// <summary>
	/// 与另一个字符串比较。
	/// </summary>
	/// <param name="other">另一个字符串。</param>
	/// <returns>大于等于另一个字符串返回true，否则返回false。</returns>
	bool operator >= (Bagel_StringHolder other) const
	{
		return inner >= other;
	}

	/// <summary>
	/// 与另一个字符串比较。
	/// </summary>
	/// <param name="other">另一个字符串。</param>
	/// <returns>小于另一个字符串返回true，否则返回false。</returns>
	bool operator < (Bagel_StringHolder other) const
	{
		return inner < other;
	}

	/// <summary>
	/// 与另一个字符串比较。
	/// </summary>
	/// <param name="other">另一个字符串。</param>
	/// <returns>小于等于另一个字符串返回true，否则返回false。</returns>
	bool operator <= (Bagel_StringHolder other) const
	{
		return inner <= other;
	}

	/// <summary>
	/// 与另一个 Bagel_Handler<Bagel_String> 执行字符串的比较。
	/// </summary>
	/// <param name="other">另一个对象。</param>
	/// <returns>相同返回true，不同返回false。</returns>
	bool operator == (const Bagel_Handler<Bagel_String> &other) const
	{
		return inner == other.inner;
	}

	/// <summary>
	/// 与另一个 Bagel_Handler<Bagel_String> 执行字符串的比较。
	/// </summary>
	/// <param name="other">另一个对象。</param>
	/// <returns>相同返回false，不同返回true。</returns>
	bool operator != (const Bagel_Handler<Bagel_String> &other) const
	{
		return inner != other.inner;
	}

	/// <summary>
	/// 与另一个 Bagel_Handler<Bagel_String> 执行字符串的比较。
	/// </summary>
	/// <param name="other">另一个对象。</param>
	/// <returns>大于另一个字符串返回true，否则返回false。</returns>
	bool operator > (const Bagel_Handler<Bagel_String> &other) const
	{
		return inner > other;
	}

	/// <summary>
	/// 与另一个 Bagel_Handler<Bagel_String> 执行字符串的比较。
	/// </summary>
	/// <param name="other">另一个对象。</param>
	/// <returns>大于等于另一个字符串返回true，否则返回false。</returns>
	bool operator >= (const Bagel_Handler<Bagel_String> &other) const
	{
		return inner >= other;
	}

	/// <summary>
	/// 与另一个 Bagel_Handler<Bagel_String> 执行字符串的比较。
	/// </summary>
	/// <param name="other">另一个对象。</param>
	/// <returns>小于另一个字符串返回true，否则返回false。</returns>
	bool operator < (const Bagel_Handler<Bagel_String> &other) const
	{
		return inner < other;
	}

	/// <summary>
	/// 与另一个 Bagel_Handler<Bagel_String> 执行字符串的比较。
	/// </summary>
	/// <param name="other">另一个对象。</param>
	/// <returns>小于等于另一个字符串返回true，否则返回false。</returns>
	bool operator <= (const Bagel_Handler<Bagel_String> &other) const
	{
		return inner <= other;
	}

	/// <summary>
	/// 布尔强转。
	/// </summary>
	/// <returns>
	/// 返回知否为一个空对象（nullptr），即什么对象也不持有。
	/// </returns>
	operator bool() const
	{
		return inner.s != _globalStructures.stringMap->nullString;
	}

	operator const u16string& () const
	{
		return inner;
	}

	operator wstring () const
	{
		return inner;
	}
};

inline int32_t _BKE_hash(const Bagel_Handler<Bagel_String> &str)
{
	return BKE_hash(str.inner);
}

template<>
inline int32_t BKE_hash(const Bagel_Handler<Bagel_String> &s)
{
	return _BKE_hash(s);
}

#pragma pop_macro("new")