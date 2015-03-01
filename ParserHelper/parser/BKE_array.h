#pragma once

#include <stdlib.h>
#include "defines.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#pragma push_macro("new")
#ifdef new
#undef new
#endif

#ifdef _DEBUG
#ifndef _ASSERT_EXPR
#define _ASSERT_EXPR(exp, msg) assert(exp)
#endif
#else
#ifdef _ASSERT_EXPR
#undef _ASSERT_EXPR
#endif
#define _ASSERT_EXPR(exp, msg)
#endif

//to decrease times of new and delete and realloc
//and the address of element will never changed
//when you get the address of array(4), and insert another element into 4, then your address will point to the new element.
template<class T, bkplong unit=4>
class BKE_array
{
private:
	struct __array_helper
	{
		T a;
	};
	bkplong ARR_OFFSET;
	bkplong ARR_UNITSIZE;
	bkplong ARR_MASK;
	T **addrlist;
	//max index that addrlist[index] in available
	bkplong addrcount;
	bkplong count;
	//length of addrlist
	bkplong addrsize;
	T *backaddr;
	char buf[(1<<unit) * sizeof(T)];

	inline void __init(T* addr, bkplong count)
	{
		while(count-->0)
		{
			new((void *)addr) __array_helper;
			addr++;
		}
	}

public:
	BKE_array()
	{
		addrlist=(T **)malloc(sizeof(void*) * 16);
		addrlist[0]=(T*)buf;
		count=0;
		addrcount=0;
		addrsize=16;
		backaddr=NULL;
		ARR_OFFSET = unit;
		ARR_UNITSIZE = 1 << unit;
		ARR_MASK = (1 << unit) - 1;
//		BKE_inlineInit((T*)buf, ARR_UNITSIZE);
	}

	BKE_array & operator = (const BKE_array &arr)
	{
		clear();
		free(addrlist);
		addrsize = arr.addrsize;
		addrcount = arr.addrcount;
		addrlist = (T **)malloc(sizeof(void*) * addrsize);
		addrlist[0] = (T*)buf;
		memcpy(addrlist[0], arr.addrlist[0], ARR_UNITSIZE * sizeof(T));
		for (int i = 1; i < addrcount + 1; i++)
		{
			addrlist[i] = (T*)malloc(ARR_UNITSIZE * sizeof(T));
			memcpy(addrlist[i], arr.addrlist[i], ARR_UNITSIZE * sizeof(T));
		}
		backaddr = arr.backaddr - arr.addrlist[addrcount] + addrlist[addrcount];
		return *this;
	}

	inline void clear()
	{
		if(!count)
			return;
		T* base=addrlist[addrcount];
		for(bkplong i=((count-1) & ARR_MASK);i>=0;i--)
			((__array_helper*)(base+i))->~__array_helper();
		if(addrcount>0)
		{
			free(base);
			for(bkplong i=1;i<addrcount;i++)
			{
				base=addrlist[i];
				for (bkplong j = ARR_MASK; j >= 0; j--)
					((__array_helper*)(base+j))->~__array_helper();
				free(base);
			}
			for (bkplong j = ARR_MASK; j >= 0; j--)
				((__array_helper*)(buf + j * sizeof(T)))->~__array_helper();
		}
		backaddr = NULL;
		count=0;
		addrcount=0;
	}

	~BKE_array()
	{
		clear();
		free(addrlist);
	}

	inline T& operator [] (const bkplong index) const
	{
		_ASSERT_EXPR(index>=0 && index<count, L"下标越界");
		return addrlist[index>>ARR_OFFSET][index & ARR_MASK];
	}

	inline T& back() const
	{
		_ASSERT_EXPR(backaddr!=NULL, L"空数组没有back");
		return *backaddr;
	}

	void resize(bkplong size)
	{
		_ASSERT_EXPR(size>=0, L"大小不能设为负数");
		if(!size)
		{
			clear();
			return;
		}
		if (size == count)
			return;
		if(size>count)
		{
			size--;
			bkplong dim1 = size >> ARR_OFFSET;
			bkplong dim2 = size & ARR_MASK;
			//init this array
			bkplong start = !count ? -1 : (count - 1) & ARR_MASK;
			bkplong end = dim1 > addrcount ? ARR_MASK : dim2;
			__init(&addrlist[count >> ARR_OFFSET][count & ARR_MASK], end - start);
			if (dim1 == addrcount)
			{
				count = size + 1;
				backaddr = &operator [] (size);
				return;
			}
			//expand addrlist
			bkplong sz = (dim1 & ~15) + 16;
			if (sz > addrsize)
			{
				addrlist = (T**)realloc(addrlist, sz*sizeof(T**));
				addrsize = sz;
			}
			//malloc array
			for (bkplong i = addrcount + 1; i <= dim1; i++)
			{
				addrlist[i] = (T*)malloc(ARR_UNITSIZE * sizeof(T));
			}
			//init last
			__init(&addrlist[dim1][0], dim2 + 1);
			//init rest
			for (bkplong i = addrcount + 1; i < dim1;i++)
				__init(&addrlist[i][0], ARR_UNITSIZE);
			addrcount = dim1;
			count = size + 1;
			backaddr = &operator [] (size);
		}
		else
		{
			while(size<count--)
				((__array_helper*)(&addrlist[count>>ARR_OFFSET][count & ARR_MASK]))->~__array_helper();
			for (int i = (count >> ARR_OFFSET) + 1; i <= addrcount; i++)
				free(addrlist[i]);
			addrcount = count >> ARR_OFFSET;
			count++;
			backaddr=&operator [] (size-1);
		}
	}

	void reserve(bkplong size)
	{
		if(!size)
		{
			clear();
			return;
		}
		_ASSERT_EXPR(size>=0, L"大小不能设为负数");
		if(count>size)
		{
			resize(size);
			bkplong st = ((count - 1) >> ARR_OFFSET) + 1;
			for (bkplong i = st; i <= addrcount; i++)
				free(addrlist[i]);
			addrcount = st - 1;
			return;
		}
		size--;
		size>>=ARR_OFFSET;
		if(size<addrcount)
		{
			for(bkplong i=addrcount;i>size;i--)
				free(addrlist[i]);
		}
		else
		{
			if(size>=addrsize)
			{
				//increase every 16 size
				bkplong dstsize = ((size-1) & ~15) + 16;
				addrlist=(T**)realloc(addrlist, dstsize*sizeof(T**));
				addrsize=dstsize;
			}
			for(bkplong i=addrcount+1;i<=size;i++)
			{
				addrlist[i]=(T*)malloc(ARR_UNITSIZE * sizeof(T));
			}
		}
		addrcount=size;
	}

	T& push_back(const T &t)
	{
		bkplong s=count>>ARR_OFFSET;
		bkplong s2=count & ARR_MASK;
		if((s2 == 0) && (s >addrcount))
		{
			addrcount=s;
			if(addrcount>=addrsize)
			{
				addrlist=(T**)realloc(addrlist, (1+addrcount)*sizeof(T**));
				addrsize++;
			}
			addrlist[addrcount]=(T*)malloc(ARR_UNITSIZE * sizeof(T));
			backaddr=&addrlist[addrcount][0];
			new(backaddr) T(t);
			count++;
			return *backaddr;
		}
		count++;
		backaddr=&addrlist[s][s2];
		new(backaddr) T(t);
		return *backaddr;
	}

	T& push_back(T &&t)
	{
		bkplong s=count>>ARR_OFFSET;
		bkplong s2=count & ARR_MASK;
		if((s2 == 0) && (s >addrcount))
		{
			addrcount=s;
			if(addrcount>=addrsize)
			{
				addrlist=(T**)realloc(addrlist, (1+addrcount)*sizeof(T**));
				addrsize++;
			}
			addrlist[addrcount]=(T*)malloc(ARR_UNITSIZE * sizeof(T));
			backaddr=&addrlist[addrcount][0];
			new(backaddr) T(t);
			count++;
			return *backaddr;
		}
		count++;
		backaddr=&addrlist[s][s2];
		new(backaddr) T(std::move(t));
		return *backaddr;
	}

	T& push_back_new()
	{
		bkplong s=count>>ARR_OFFSET;
		bkplong s2=count & ARR_MASK;
		if((s2 == 0) && (s >addrcount))
		{
			addrcount=s;
			if(addrcount>=addrsize)
			{
				addrlist=(T**)realloc(addrlist, (1+addrcount)*sizeof(T**));
				addrsize++;
			}
			addrlist[addrcount]=(T*)malloc(ARR_UNITSIZE * sizeof(T));
			backaddr=&addrlist[addrcount][0];
			new(backaddr) T;
			count++;
			return *backaddr;
		}
		count++;
		backaddr=&addrlist[s][s2];
		new(backaddr) T;
		return *backaddr;
	}

	void insert(bkplong index, const T& t)
	{
		_ASSERT_EXPR(index>=0 && index<count, L"下标越界");
		bkplong s=count>>ARR_OFFSET;
		bkplong s2=count & ARR_MASK;
		if((s2 == 0) && (s >addrcount))
		{
			addrcount++;
			if(addrcount>=addrsize)
			{
				addrlist=(T**)realloc(addrlist, (1+addrcount)*sizeof(T**));
				addrsize++;
			}
			addrlist[addrcount]=(T*)malloc(ARR_UNITSIZE * sizeof(T));
			backaddr=&addrlist[addrcount][0];
			memcpy(backaddr, &addrlist[addrcount - 1][ARR_MASK], sizeof(T));
		}
		bkplong end=index>>ARR_OFFSET;
		for(bkplong i=s;i>end;i--)
		{
			T* base=addrlist[i];
			memcpy(&base[1], &base[0], sizeof(T) * ARR_MASK);
			memcpy(&base[0], &addrlist[i - 1][ARR_MASK], sizeof(T));
			//for(bkplong j=255;j>0;j--)
			//	base[j]=base[j-1];
			//base[0]=addrlist[addrcount-1][255];
		}
		T* base=addrlist[end];
		index &= ARR_MASK;
		if(index != ARR_MASK)
			memcpy(&base[index + 1], &base[index], sizeof(T) * (ARR_MASK-index));
		//bkplong i=255;
		//for(;i>(index & ARR_MASK);i--)
		//	base[i]=base[i-1];
		count++;
		new((void*)&base[index]) T(t);
	}

	inline bkplong size() const
	{
		return count;
	}

	inline bool empty() const
	{
		return !count;
	}

	inline void pop_back()
	{
		_ASSERT_EXPR(count>0, L"没有元素可以再弹");
		count--;
		((__array_helper*)(&addrlist[count>>ARR_OFFSET][count & ARR_MASK]))->~__array_helper();
		backaddr=count>0?&operator [] (count-1):NULL;
	}

	inline void pop_back(T &res)
	{
		count--;
		bkplong s=count>>ARR_OFFSET;
		bkplong s2=count & ARR_MASK;
		res=addrlist[s][s2];
		((__array_helper*)(&addrlist[s][s2]))->~__array_helper();
		backaddr=&operator [] (count-1);
	}

	void erase(bkplong index)
	{
		_ASSERT_EXPR(index>=0 && index<count, L"下标越界");
		if(index==count-1)
		{
			pop_back();
			return;
		}
		bkplong s=index >> ARR_OFFSET;
		bkplong s2=index & ARR_MASK;
		((__array_helper*)(&addrlist[s][s2]))->~__array_helper();
		if (s2 != ARR_MASK)
		{
			memcpy(&addrlist[s][s2], &addrlist[s][s2+1], sizeof(T) * (ARR_MASK-s2));
			s++;
		}
		bkplong end=(--count)>>ARR_OFFSET;
		if (!s)
			s = 1;
		for(bkplong i=s;i<=end;i++)
		{
			memcpy(&addrlist[s - 1][ARR_MASK], &addrlist[s][0], sizeof(T));
			memcpy(&addrlist[s][0], &addrlist[s][1], sizeof(T)* ARR_MASK);
		}
	}

	inline void eraseValue(const T &t)
	{
		for(bkplong i=0;i<count;i++)
		{
			if(operator [] (i) == t)
			{
				erase(i);
				i--;
			}
		}
	}
};

#pragma pop_macro("new")

