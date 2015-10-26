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
#include <assert.h>

#pragma pack(push)
#pragma pack(4)

#pragma push_macro("new")
#undef new

#define SMALL 32
#define MEMORY_UNIT 8

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
		UNIT = MEMORY_UNIT * T,
		BLOCK = 14,
		BLOCKSIZE = ((1 << BLOCK) - 40) / UNIT,
		BLOCKMEMSIZE = 1 << BLOCK,
		BLOCKMASK = BLOCKSIZE - 1,
	};

	struct __helper_array;

	struct __helper
	{
		bkpchar addr[UNIT];
		__helper *next;
		__helper_array *group;
#ifdef _DEBUG
		//是否可用/是否可以分配
		bkplong valid;
		bkplong magic;
#endif
	};

	struct __helper_array_header
	{
		bkpulong count;
		__helper *endpos;
		__helper *curpos;
		__helper *freeList;
		__helper_array *last;
		__helper_array *next;
	};

	struct __helper_array
	{
		bkpulong count;
		__helper *endpos;
		__helper *curpos;
		__helper *freeList;
		__helper_array *last;
		__helper_array *next;
		//这个放在最后保证对任何UNIT上面的几个成员的偏移都是固定的
		__helper ptr[BLOCKSIZE];
	};

	__helper_array *cur;
	__helper_array *head;
	__helper_array *tail;

	bkpulong capacity;
	bkpulong index;
	bkpulong unit;
	bkpulong sizehelper;
	bkpulong sizehelperarray;
	bkpulong blocksize;

private:
	inline bool alloc_array()
	{
		//assert(tail->next==NULL);
		tail->next = (__helper_array*)malloc(BLOCKMEMSIZE);
		if (!tail->next)
			return false;
		capacity += blocksize;
		tail->next->last = tail;
		tail->next->next = NULL;
		tail = tail->next;
		tail->count = 0;
		tail->curpos = tail->ptr;
		tail->endpos = tail->ptr + blocksize;
		tail->freeList = NULL;
#ifdef _DEBUG
		for (bkpulong i = 0; i < blocksize; i++)
			tail->ptr[i].valid = 1;
#endif
		cur = tail;
		return true;
	}

	inline bool dynamic_alloc_array()
	{
		//assert(tail->next == NULL);
		tail->next = (__helper_array*)malloc(BLOCKMEMSIZE);
		if (!tail->next)
			return false;
		capacity += blocksize;
		tail->next->last = tail;
		tail->next->next = NULL;
		tail = tail->next;
		tail->count = 0;
		bkpchar *ptrpos = (bkpchar*)tail + sizeof(bkpulong) + sizeof(__helper*) * 3 + sizeof(__helper_array*) * 2;
		tail->curpos = (__helper*)ptrpos;
		tail->endpos = (__helper*)(ptrpos + blocksize * sizehelper);
		tail->freeList = NULL;
#ifdef _DEBUG
		for (bkpulong i = 0; i < blocksize; i++)
			*(bkplong*)(ptrpos + i * sizehelper + unit + sizeof(__helper*) + sizeof(__helper_array*)) = 1;
#endif
		cur = tail;
		return true;
	}
public:
	_BKE_allocator()
	{
		unit = UNIT;
		index = 0;
		capacity = 0;
		sizehelper = sizeof(__helper);
		blocksize = (BLOCKMEMSIZE - sizeof(__helper_array_header)) / sizehelper;
		sizehelperarray = sizeof(__helper_array);
		if (T <= SMALL)
		{
			//assert(!allocator_array()[T]);
			allocator_array()[T] = (_BKE_allocator<1> *)this;
			cur = head = (__helper_array*)malloc(BLOCKMEMSIZE);
			cur->count = 1;
#ifdef _DEBUG
			for (bkpulong i = 0; i < blocksize; i++)
				cur->ptr[i].valid = 1;
#endif
			//预留一个已分配且禁止释放的位置，防止第一个block被free
			cur->curpos = cur->ptr + 1;
			cur->endpos = cur->ptr + blocksize;
			cur->freeList = NULL;
			cur->last = NULL;
			cur->next = NULL;
			tail = head;
		}
	}

	bkpulong getallocsize()
	{
		return capacity / blocksize * sizehelperarray;
	}

	bkpulong getusedsize()
	{
		__helper_array *iter = head;
		//去除第一个保留位
		bkpulong start = (bkpulong)-1;
		while (iter)
		{
			start += iter->count;
			iter = iter->next;
		}
		return start;
	}

	void* allocate()
	{
		if (T <= SMALL)
		{
			__helper *res;
			if (cur->freeList)
			{
				res = cur->freeList;
				cur->freeList = res->next;
			}
			else if (cur->curpos < cur->endpos)
			{
				res = cur->curpos++;
			}
			else
			{
				__helper_array *raw = cur;
				do
				{
					if (!cur->next)
					{
						cur = head;
						break;
					}
					else
					{
						cur = cur->next;
					}
				} while (cur != raw && cur->curpos >= cur->endpos && !cur->freeList);
				if (cur->freeList)
				{
					res = cur->freeList;
					cur->freeList = res->next;
				}
				else if (cur->curpos < cur->endpos)
				{
					res = cur->curpos++;
				}
				else
				{
					if(!alloc_array())
						return NULL;
					res = cur->curpos++;
				}
			}
			cur->count++;
			res->group = cur;
#ifdef _DEBUG
			res->valid = 0;
			res->magic = UNIT;
#endif
			return res;
		}
		else
			return malloc(UNIT);
	}

	void deallocate(void* p)
	{
		if (!p)
			return;
		if (T <= SMALL)
		{
#ifdef _DEBUG
			assert(((__helper*)p)->magic == UNIT && ((__helper*)p)->valid == 0);
#endif
			((__helper*)p)->next = ((__helper*)p)->group->freeList;
			((__helper*)p)->group->freeList = (__helper*)p;
			((__helper*)p)->group->count--;
		}
		else
			free(p);
	}

	void* dynamic_allocate()
	{
		if (unit <= SMALL)
		{
			void *res;
			if (cur->freeList)
			{
				res = cur->freeList;
				/*cur->freeList = res->next;*/cur->freeList = *(__helper**)((bkpchar*)res + unit);
			}
			else if (cur->curpos < cur->endpos)
			{
				/*res = cur->curpos++;*/res = cur->curpos; cur->curpos = (__helper*)((bkpchar*)(cur->curpos) + sizehelper);
			}
			else
			{
				__helper_array *raw = cur;
				do
				{
					if (!cur->next)
					{
						cur = head;
						break;
					}
					else
					{
						cur = cur->next;
					}
				} while (cur != raw && cur->curpos >= cur->endpos && !cur->freeList);
				if (cur->freeList)
				{
					res = cur->freeList;
					/*cur->freeList = res->next;*/cur->freeList = *(__helper**)((bkpchar*)res + unit);
				}
				else if (cur->curpos < cur->endpos)
				{
					/*res = cur->curpos++;*/res = cur->curpos; cur->curpos = (__helper*)((bkpchar*)(cur->curpos) + sizehelper);
				}
				else
				{
					if (!dynamic_alloc_array())
						return NULL;
					/*res = cur->curpos++;*/res = cur->curpos; cur->curpos = (__helper*)((bkpchar*)(cur->curpos) + sizehelper);
				}
			}
			cur->count++;
			/*res->group = cur;*/*(__helper_array**)((bkpchar*)res + unit + sizeof(__helper*)) = cur;
#ifdef _DEBUG
			/*res->valid = 0;*/*(bkplong*)((bkpchar*)res + unit + sizeof(__helper*) + sizeof(__helper_array*)) = 0;
			/*res->magic = UNIT;*/*(bkplong*)((bkpchar*)res + unit + sizeof(__helper*) + sizeof(__helper_array*) + sizeof(bkplong)) = unit;
#endif
			return res;
	}
		else
			return malloc(unit);
	}

	void dynamic_deallocate(void* p)
	{
		if (!p)
			return;
		if (unit <= SMALL)
		{
#ifdef _DEBUG
			//assert(((__helper*)p)->magic == UNIT && ((__helper*)p)->valid == 0);
			assert(*(bkplong*)((bkpchar*)p + unit + sizeof(__helper*) + sizeof(__helper_array*)) == 0);
			assert(*(bkplong*)((bkpchar*)p + unit + sizeof(__helper*) + sizeof(__helper_array*) + sizeof(bkplong)) == unit);
#endif
			/*((__helper*)p)->next = ((__helper*)p)->group->freeList;*/*(__helper**)((bkpchar*)p + unit) = (*(__helper_array**)((bkpchar*)p + unit + sizeof(__helper*)))->freeList;
			/*((__helper*)p)->group->freeList = (__helper*)p;*/(*(__helper_array**)((bkpchar*)p + unit + sizeof(__helper*)))->freeList = (__helper*)p;
			/*((__helper*)p)->group->count--;*/(*(__helper_array**)((bkpchar*)p + unit + sizeof(__helper*)))->count--;
		}
		else
			free(p);
	}

	void shrink()
	{
		__helper_array *iter = head;
		while (iter)
		{
			tail = iter;
			if (!iter->count)
			{
				iter->last->next = iter->next;
				if (iter->next)
					iter->next->last = iter->last;
				tail = iter->last;
				__helper_array *i = iter;
				iter = i->next;
				free(i);
				capacity -= BLOCKSIZE;
			}
			else
				iter = iter->next;
		}
		cur = head;
	}

	~_BKE_allocator()
	{
		if (T <= SMALL)
		{
			while (head)
			{
				cur = head->next;
				free(head);
				head = cur;
			}
			allocator_array()[T] = NULL;
		}
	}
};

inline _BKE_allocator<1> **allocator_array()
{
	static void* arr[SMALL + 1] = { nullptr };
	return (_BKE_allocator<1> **)arr;
}

template <int T>
inline _BKE_allocator<T> *get_allocator()
{
	static _BKE_allocator<T> alloc;
	return &alloc;
};

inline _BKE_allocator<1> *get_allocator(int size)
{
	switch ((size + MEMORY_UNIT - 1) / MEMORY_UNIT)
	{
	case 32: return (_BKE_allocator<1>*)get_allocator<32>();
	case 31: return (_BKE_allocator<1>*)get_allocator<31>();
	case 30: return (_BKE_allocator<1>*)get_allocator<30>();
	case 29: return (_BKE_allocator<1>*)get_allocator<29>();
	case 28: return (_BKE_allocator<1>*)get_allocator<28>();
	case 27: return (_BKE_allocator<1>*)get_allocator<27>();
	case 26: return (_BKE_allocator<1>*)get_allocator<26>();
	case 25: return (_BKE_allocator<1>*)get_allocator<25>();
	case 24: return (_BKE_allocator<1>*)get_allocator<24>();
	case 23: return (_BKE_allocator<1>*)get_allocator<23>();
	case 22: return (_BKE_allocator<1>*)get_allocator<22>();
	case 21: return (_BKE_allocator<1>*)get_allocator<21>();
	case 20: return (_BKE_allocator<1>*)get_allocator<20>();
	case 19: return (_BKE_allocator<1>*)get_allocator<19>();
	case 18: return (_BKE_allocator<1>*)get_allocator<18>();
	case 17: return (_BKE_allocator<1>*)get_allocator<17>();
	case 16: return (_BKE_allocator<1>*)get_allocator<16>();
	case 15: return (_BKE_allocator<1>*)get_allocator<15>();
	case 14: return (_BKE_allocator<1>*)get_allocator<14>();
	case 13: return (_BKE_allocator<1>*)get_allocator<13>();
	case 12: return (_BKE_allocator<1>*)get_allocator<12>();
	case 11: return (_BKE_allocator<1>*)get_allocator<11>();
	case 10: return (_BKE_allocator<1>*)get_allocator<10>();
	case 9: return (_BKE_allocator<1>*)get_allocator<9>();
	case 8: return (_BKE_allocator<1>*)get_allocator<8>();
	case 7: return (_BKE_allocator<1>*)get_allocator<7>();
	case 6: return (_BKE_allocator<1>*)get_allocator<6>();
	case 5: return (_BKE_allocator<1>*)get_allocator<5>();
	case 4: return (_BKE_allocator<1>*)get_allocator<4>();
	case 3: return (_BKE_allocator<1>*)get_allocator<3>();
	case 2: return (_BKE_allocator<1>*)get_allocator<2>();
	case 1: return (_BKE_allocator<1>*)get_allocator<1>();
	default:
		return nullptr;
	}
}

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

inline int __get_memorypool_memory()
{
	int s = 0;
	switch (SMALL)
	{
	case 32:s += get_allocator<32>()->getallocsize();
	case 31:s += get_allocator<31>()->getallocsize();
	case 30:s += get_allocator<30>()->getallocsize();
	case 29:s += get_allocator<29>()->getallocsize();
	case 28:s += get_allocator<28>()->getallocsize();
	case 27:s += get_allocator<27>()->getallocsize();
	case 26:s += get_allocator<26>()->getallocsize();
	case 25:s += get_allocator<25>()->getallocsize();
	case 24:s += get_allocator<24>()->getallocsize();
	case 23:s += get_allocator<23>()->getallocsize();
	case 22:s += get_allocator<22>()->getallocsize();
	case 21:s += get_allocator<21>()->getallocsize();
	case 20:s += get_allocator<20>()->getallocsize();
	case 19:s += get_allocator<19>()->getallocsize();
	case 18:s += get_allocator<18>()->getallocsize();
	case 17:s += get_allocator<17>()->getallocsize();
	case 16:s += get_allocator<16>()->getallocsize();
	case 15:s += get_allocator<15>()->getallocsize();
	case 14:s += get_allocator<14>()->getallocsize();
	case 13:s += get_allocator<13>()->getallocsize();
	case 12:s += get_allocator<12>()->getallocsize();
	case 11:s += get_allocator<11>()->getallocsize();
	case 10:s += get_allocator<10>()->getallocsize();
	case 9:s += get_allocator<9>()->getallocsize();
	case 8:s += get_allocator<8>()->getallocsize();
	case 7:s += get_allocator<7>()->getallocsize();
	case 6:s += get_allocator<6>()->getallocsize();
	case 5:s += get_allocator<5>()->getallocsize();
	case 4:s += get_allocator<4>()->getallocsize();
	case 3:s += get_allocator<3>()->getallocsize();
	case 2:s += get_allocator<2>()->getallocsize();
	case 1:s += get_allocator<1>()->getallocsize();
	}
	return s;
}

//use standard function for multi-thread
template <class T>
class BKE_allocator
{
private:
	_BKE_allocator<(sizeof(T) + MEMORY_UNIT - 1) / MEMORY_UNIT> *al;

	struct __helper
	{
		T a;
	};

public:
	BKE_allocator()
	{
		al = get_allocator<(sizeof(T) + MEMORY_UNIT - 1) / MEMORY_UNIT>();
	}

	T* allocate()
	{
		//return (T*)(al->allocate());
		return (T*)malloc(sizeof(T));
	}

	T* allocate(size_t count)
	{
		//if (count > 1)
			return (T*)malloc(sizeof(T) * count);
		//return (T*)(al->allocate());
	}

	void deallocate(T* p)
	{
		free(p);
		//al->deallocate(p);
	}

	void deallocate(T* p, size_t count)
	{
		//if (count > 1)
			free(p);
		//else
		//	al->deallocate(p);
	}

	T* op_new()
	{
		auto p = allocate();
		new ((void*)p) T();
		return p;
	}

	template <class ...args>
	T* op_new(args&&... arg)
	{
		auto p = allocate();
		new ((void*)p) T(std::forward<args>(arg)...);
		return p;
	}

	void op_delete(T *p)
	{
		destroy(p);
		deallocate(p);
	}

	void construct(T *p)
	{
		new ((void*)p) __helper();
	}

	void construct(T *p, T &&val)
	{
		new ((void*)p) T(std::move(val));
	}

	void construct(T *p, const T& val)
	{
		new ((void*)p) T(val);
	}

	void destroy(T *p)
	{
		((__helper*)p)->~__helper();
	}

	size_t max_size() const
	{
		return ((size_t)(-1) / sizeof(T));
	}
};

#pragma pop_macro("new")

#pragma pack(pop)

#ifdef _DEBUG_MEM
#include <debug_new.h>
#endif
