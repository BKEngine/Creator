#pragma once

#include "Bagel_Config.h"
#include "BKE_hash.hpp"
#include "Bagel_Var.h"

/// <summary>
/// 用于序列化保存读取的类。
/// </summary>
class Bagel_Serializer
{
protected:
	int counter;
	BKE_hashmap<Bagel_Object*, int> cache;
	vector<Bagel_Object*> recache;
	vector<uint32_t> data;

	enum
	{
		SEL_NOERROR,
		SEL_SIGERROR,
		SEL_LENGTHERROR,
		SEL_CRCERROR,
		SEL_NODEFINECLASS,
	}lastError;

	enum
	{
		SEL_VOID,
		SEL_NUM,
		SEL_REFSTR,
		SEL_NULLSTR,
		SEL_STR,
		SEL_OBJ,
		//SEL_NEWOBJ,
		SEL_ARR,
		SEL_DIC,
		SEL_FUNC,
		SEL_PROP,
		SEL_CLO,
		SEL_DEFCLASS,
		SEL_CLASS,
		SEL_POINTER,
		SEL_FUNCCODE,
		SEL_BYTECODE,
		SEL_BYTREE,
		SEL_GLOBALCLO,
		SEL_STACK,
		SEL_VECTOR,
	};

	struct _cache
	{
		int pos;
		bool isnew;
	};

	inline _cache getObject(Bagel_Object *o)
	{
		auto it = cache.find(o);
		if (it == cache.end())
		{
			return{ cache[o] = counter++, true };
		}
		else
		{
			return{ it->second, false };
		}
	}

	void operator << (const Bagel_Var &v);

	virtual Bagel_Var _parse(const uint32_t* &save);

public:
	Bagel_Serializer() : counter(0)
	{
	}

	/// <summary>
	/// 序列化变量v。
	/// </summary>
	/// <param name="v">要被序列化的变量。</param>
	/// <returns>序列化的结果。</returns>
	vector<uint32_t>&& serialize(const Bagel_Var &v)
	{
		data.clear();
		data.reserve(256);
		data.push_back(BAGEL_SERIALIZE_SIG);
		data.push_back(0);
		data.push_back(0);
		*this << v;
		data[1] = data.size();
		uint32_t h = 0;
		for (unsigned int i = 3; i < data.size(); i++)
			h ^= data[i];
		data[2] = h;
		return std::move(data);
	};

	/// <summary>
	/// 反序列化。
	/// </summary>
	/// <param name="save">序列化的结果。</param>
	/// <returns>反序列化得到的变量</returns>
	Bagel_Var parse(const vector<uint32_t> &save);

	/// <summary>
	/// 反序列化。
	/// </summary>
	/// <param name="save">指向序列化的结果的内存区域的指针。</param>
	/// <param name="size">内存区域的大小，以uint32_t（4字节）的数目计。</param>
	/// <returns></returns>
	Bagel_Var parse(const uint32_t *save, int size);

	/// <summary>
	/// 获取最近一次parse操作的错误。
	/// </summary>
	/// <returns>返回的错误值，分别有SEL_NOERROR(0)，SSEL_SIGERROR(1)，SEL_LENGTHERROR(2)，SEL_CRCERROR(3)</returns>
	int getLastError() const
	{
		return lastError;
	}
};