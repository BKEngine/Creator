/*
                   _ooOoo_
                  o8888888o
                  88" . "88
                  (| -_- |)
                  O\  =  /O
               ____/`---'\____
             .'  \\|     |//  `.
            /  \\|||  :  |||//  \
           /  _||||| -:- |||||-  \
           |   | \\\  -  /// |   |
           | \_|  ''\---/''  |   |
           \  .-\__  `-`  ___/-. /
         ___`. .'  /--.--\  `. . __
      ."" '<  `.___\_<|>_/___.'  >'"".
     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
     \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
                   `=---='
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			 no bug forever
*/

#pragma once

//Bakery Memory Pool
//can only used in single thread
#include "defines.h"
#include <vector>

#pragma pack(push)
#pragma pack(4)

#pragma push_macro("new")
#undef new

#define SMALL (128 / 4)

template <int T>
class _BKE_allocator;
_BKE_allocator<1> **allocator_array();
//4*T bytes
template <int T>
class _BKE_allocator
{
private:
	enum
	{
		UNIT = 4*T,
		BLOCK = 14,
		BLOCKSIZE = 1 << BLOCK,
		BLOCKMASK = BLOCKSIZE - 1,
	};

	struct __helper
	{
		char addr[UNIT];
		int32_t next;
		int32_t index;
#ifdef _DEBUG
		int32_t magic;
#endif
	};
	__helper* buf;
	std::vector<__helper *> buflist;
	bkplong capacity;
	int index;
	int unit;
	int sizehelper;

public:
	_BKE_allocator()
	{
		unit = T;
		sizehelper = sizeof(__helper) / 4;
		if (T <= SMALL)
		{
			buf = (__helper*)malloc(BLOCKSIZE * sizeof(__helper));
			buflist.reserve(16);
			buflist.push_back(buf);
			capacity = BLOCKSIZE;
			index = 0;
			for (int i = 0; i < BLOCKSIZE; i++)
				buf[i].next = i + 1;
			assert(!allocator_array()[T]);
			allocator_array()[T] = (_BKE_allocator<1> *)this;
		}
	}

	void* allocate()
	{
		if (T <= SMALL)
		{
			if (index >= capacity)
			{
				buf = (__helper*)malloc(BLOCKSIZE * sizeof(__helper));
				if (!buf)
					return NULL;
				for (int i = 0; i < BLOCKSIZE; i++)
					buf[i].next = i + capacity + 1;
				buflist.push_back(buf);
				buf[0].index = capacity;
				capacity += BLOCKSIZE;
				buf[0].next = -1;
				index++;
#ifdef _DEBUG
				buf[0].magic = UNIT;
#endif
				return &buf[0];
			}
			else
			{
				auto res = &buflist[index >> BLOCK][index & BLOCKMASK];
				res->index = index;
				index = res->next;
				res->next = -1;
#ifdef _DEBUG
				res->magic = UNIT;
#endif
				return res;
			}
		}
		else
			return malloc(UNIT);
	}

	void deallocate(void* p)
	{
		if (T <= SMALL)
		{
#ifdef _DEBUG
			assert(((__helper*)p)->magic == UNIT);
#endif
			((__helper*)p)->next = index;
			index = ((__helper*)p)->index;
		}
		else
			free(p);
	}

	void* dynamic_allocate()
	{
		if (unit <= SMALL)
		{
			if (index >= capacity)
			{
				buf = (__helper*)malloc(BLOCKSIZE * sizehelper);
				if (!buf)
					return NULL;
				for (int i = 0; i < BLOCKSIZE; i++)
					*((int32_t *)buf + sizehelper*i + unit) = i + capacity + 1;
				buflist.push_back(buf);
				*((int32_t *)buf + unit + 1) = capacity;
				capacity += BLOCKSIZE;
				*((int32_t *)buf + unit) = -1;
				index++;
#ifdef _DEBUG
				*((int32_t *)buf + unit + 2) = unit;
#endif
				return &buf[0];
			}
			else
			{
				int32_t* res = ((int32_t *)buflist[index >> BLOCK]) + sizehelper * (index & BLOCKMASK);
				*(res + unit + 1) = index;
				index = *(res + unit);
				*(res + unit) = -1;
#ifdef _DEBUG
				*(res + unit + 2) = unit;
#endif
				return res;
			}
		}
		else
			return malloc(unit);
	}

	void dynamic_deallocate(void* p)
	{
		if (unit <= SMALL)
		{
#ifdef _DEBUG
			assert(*((int32_t*)p + unit + 2) == unit);
#endif
			*((int32_t*)p + unit) = index;
			index = *((int32_t*)p + unit + 1);
		}
		else
			free(p);
	}

	~_BKE_allocator()
	{
		if (T <= SMALL)
		{
			for (auto &it : buflist)
				free(it);
		}
	}
};

inline _BKE_allocator<1> **allocator_array()
{
	static void* arr[SMALL + 1] = { NULL };
	return (_BKE_allocator<1> **)arr;
}

template <int T>
inline _BKE_allocator<T> *get_allocator()
{
	static _BKE_allocator<T> alloc;
	return &alloc;
};

static inline void __init_memorypool()
{
	switch (SMALL)
	{
	case 32:get_allocator<32>();
	case 31:get_allocator<31>();
	case 30:get_allocator<30>();
	case 29:get_allocator<29>();
	case 28:get_allocator<28>();
	case 27:get_allocator<27>();
	case 26:get_allocator<26>();
	case 25:get_allocator<25>();
	case 24:get_allocator<24>();
	case 23:get_allocator<23>();
	case 22:get_allocator<22>();
	case 21:get_allocator<21>();
	case 20:get_allocator<20>();
	case 19:get_allocator<19>();
	case 18:get_allocator<18>();
	case 17:get_allocator<17>();
	case 16:get_allocator<16>();
	case 15:get_allocator<15>();
	case 14:get_allocator<14>();
	case 13:get_allocator<13>();
	case 12:get_allocator<12>();
	case 11:get_allocator<11>();
	case 10:get_allocator<10>();
	case 9:get_allocator<9>();
	case 8:get_allocator<8>();
	case 7:get_allocator<7>();
	case 6:get_allocator<6>();
	case 5:get_allocator<5>();
	case 4:get_allocator<4>();
	case 3:get_allocator<3>();
	case 2:get_allocator<2>();
	case 1:get_allocator<1>();
	}
}

template <class T>
class BKE_allocator
{
private:
	_BKE_allocator<(sizeof(T) + 3) / 4> *al;

	struct __helper
	{
		T a;
	};

public:
	BKE_allocator()
	{
		al = get_allocator<(sizeof(T) + 3) / 4>();
	}

	T* allocate()
	{
		return (T*)(al->allocate());
	}

	T* allocate(size_t count)
	{
		if (count > 1)
			return (T*)malloc(sizeof(T) * count);
		return (T*)(al->allocate());
	}

	void deallocate(T* p)
	{
		al->deallocate(p);
	}

	void deallocate(T* p, size_t count)
	{
		if (count > 1)
			free(p);
		else
			al->deallocate(p);
	}

	void construct(T *p)
	{
		new ((void*)p) __helper();
	}

	template <class ...args>
	T* op_new(args... arg)
	{
		auto p = allocate();
		new ((void*)p) T(arg...);
		return p;
	}

	void construct(T *p, const T &val)
	{
		new ((void*)p) T(val);
	}

	void construct(T *p, T &&val)
	{
		new ((void*)p) T(std::move(val));
	}

	void destroy(T *p)
	{
		((__helper*)p)->~__helper();
	}

	void op_delete(T *p)
	{
		destroy(p);
		deallocate(p);
	}

	size_t max_size() const
	{
		return ((size_t)(-1) / sizeof(T));
	}
};

#pragma pop_macro("new")

#pragma pack(pop)