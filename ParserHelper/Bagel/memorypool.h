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

#include "Bagel_Config.h"

#ifdef INNER_ST

//Bakery Memory Pool
//can only used in single thread
#include <assert.h>

#pragma pack(push)
#pragma pack(4)

#pragma push_macro("new")
#undef new

#define SMALL 32
#define MEMORY_UNIT 16

//template <int T>
class _BKE_allocator;
_BKE_allocator **allocator_array();

//4*T bytes
//template <int T>
class _BKE_allocator
{
private:
	enum
	{
		//UNIT = MEMORY_UNIT * T,
		BLOCK = 14,
		BLOCKMEMSIZE = 1 << BLOCK,
	};

	struct __helper_array;

	struct __helper
	{
		__helper *next;
		__helper_array *group;
		//是否可用/是否可以分配
		uintptr_t valid;
		uintptr_t magic;
		//int8_t addr[UNIT];
	};

#define OFFSET(n) ((int)(&reinterpret_cast<__helper*>(0)->n))
#define GETADDR(n) (void*)((__helper*)n+1)
#define GETUNIT(head,i) ((__helper*)((uint8_t*)head->ptr+i*sizehelper))
#define ADVANCE(p) (p)=(__helper*)((uint8_t*)(p)+sizehelper)
	
	struct __helper_array_header
	{
		uint32_t count;
		__helper *endpos;
		__helper *curpos;
		__helper *freeList;
		__helper_array *last;
		__helper_array *next;
	};

	struct __helper_array
	{
		uint32_t count;
		__helper *endpos;
		__helper *curpos;
		__helper *freeList;
		__helper_array *last;
		__helper_array *next;
		//这个放在最后保证对任何UNIT上面的几个成员的偏移都是固定的
		__helper ptr[1];
	};

	__helper_array *cur;
	__helper_array *head;
	__helper_array *tail;

	uint32_t capacity;
	uint32_t index;
	uint32_t unit;
	uint32_t sizehelper;
	uint32_t sizehelperarray;
	uint32_t blocksize;

private:
	inline bool alloc_array()
	{
		//assert(tail->next==nullptr);
		tail->next = (__helper_array*)malloc(BLOCKMEMSIZE);
		if (!tail->next)
			return false;
		capacity += blocksize;
		tail->next->last = tail;
		tail->next->next = nullptr;
		tail = tail->next;
		tail->count = 0;
		tail->curpos = tail->ptr;
		tail->endpos = (__helper*)((uint8_t*)tail->ptr + blocksize * sizehelper);
		tail->freeList = nullptr;
#ifdef _DEBUG
		for (uint32_t i = 0; i < blocksize; i++)
			GETUNIT(tail, i)->valid = 1;
#endif
		cur = tail;
		return true;
	}

public:
	_BKE_allocator(int T)
	{
		assert(T <= SMALL);
		unit = MEMORY_UNIT * T;
		index = T;
		capacity = 0;
		sizehelper = sizeof(__helper) + unit;
		blocksize = (BLOCKMEMSIZE - sizeof(__helper_array_header)) / sizehelper;
		sizehelperarray = sizeof(__helper_array);
		if (T <= SMALL)
		{
			//assert(!allocator_array()[T]);
			//allocator_array()[T] = (_BKE_allocator<1> *)this;
			cur = head = (__helper_array*)malloc(BLOCKMEMSIZE);
			cur->count = 1;
#ifdef _DEBUG
			for (uint32_t i = 0; i < blocksize; i++)
				GETUNIT(cur, i)->valid = 1;
#endif
			//预留一个已分配且禁止释放的位置，防止第一个block被free
			cur->curpos = (__helper*)((uint8_t*)cur->ptr + sizehelper);
			cur->endpos = (__helper*)((uint8_t*)cur->ptr + blocksize * sizehelper);
			cur->freeList = nullptr;
			cur->last = nullptr;
			cur->next = nullptr;
			tail = head;
		}
	}

	uint32_t getallocsize() const
	{
		return capacity / blocksize * BLOCKMEMSIZE;
	}

	uint32_t getusedsize() const
	{
		__helper_array *iter = head;
		//去除第一个保留位
		uint32_t start = (uint32_t)-1;
		while (iter)
		{
			start += iter->count;
			iter = iter->next;
		}
		return start;
	}

	void* allocate()
	{
		__helper *res;
		if (cur->freeList)
		{
			res = cur->freeList;
			cur->freeList = res->next;
		}
		else if (cur->curpos < cur->endpos)
		{
			/*res = cur->curpos++;*/res = cur->curpos; ADVANCE(cur->curpos);
		}
		else
		{
			__helper_array *raw = cur;
			do
			{
				if (!cur->next)
				{
					cur = head;
				}
				else
				{
					cur = cur->next;
				}
			} while (cur != raw && cur->count >= blocksize);
			if (cur->freeList)
			{
				res = cur->freeList;
				cur->freeList = res->next;
			}
			else if (cur->curpos < cur->endpos)
			{
				/*res = cur->curpos++;*/res = cur->curpos;  ADVANCE(cur->curpos);
			}
			else
			{
				if (!alloc_array())
					return nullptr;
				/*res = cur->curpos++;*/res = cur->curpos;  ADVANCE(cur->curpos);
			}
		}
		cur->count++;
		res->group = cur;
#ifdef _DEBUG
		res->valid = 0;
		res->magic = unit;
#endif
		return GETADDR(res);
	}

	void deallocate(void* p) const
	{
		if (!p)
			return;
		__helper *h = (__helper*)((char*)p - sizeof(__helper));
	#ifdef _DEBUG
		assert(h->valid == 0);
		//assert(h->magic == unit);
		h->valid = 1;
	#endif
		h->next = h->group->freeList;
		h->group->freeList = h;
		h->group->count--;
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
				capacity -= BLOCKMEMSIZE;
			}
			else
				iter = iter->next;
		}
		cur = head;
	}

	~_BKE_allocator()
	{
		if (index <= SMALL)
		{
			while (head)
			{
				cur = head->next;
				free(head);
				head = cur;
			}
		}
	}
};

inline _BKE_allocator **allocator_array()
{
	static _BKE_allocator* arr[SMALL + 1] = { nullptr };
	return arr;
}

inline _BKE_allocator *get_allocator(int size)
{
	return allocator_array()[(size + MEMORY_UNIT - 1) / MEMORY_UNIT];
}

static inline void __init_memorypool()
{
#if BKE_CREATOR
#else
	for (int i = 1; i <= SMALL; i++)
	{
		allocator_array()[i] = new _BKE_allocator(i);
	}
#endif
}

static inline void __uninit_memorypool()
{
#if BKE_CREATOR
#else
	for (int i = 1; i <= SMALL; i++)
	{
		delete allocator_array()[i];
	}
#endif
}

inline int __get_memorypool_memory()
{
	int s = 0;
	for (int i = 1; i <= SMALL; i++)
	{
		s += allocator_array()[i]->getallocsize();
	}
	return s;
}

//use standard function for multi-thread
template <class T>
class BKE_allocator
{
	struct __helper
	{
		T a;
	};

public:
	typedef T value_type;
	typedef size_t size_type;

	BKE_allocator()
	{
	}

	template <class T2>
	BKE_allocator(const BKE_allocator<T2> &alloc)
	{
	}

	template<class Ty>
	bool operator == (const BKE_allocator<Ty> &alloc) const
	{
		return true;
	}

	template<class Ty>
	bool operator != (const BKE_allocator<Ty> &alloc) const
	{
		return false;
	}

	T* allocate()
	{
#if BKE_CREATOR
		return (T*)malloc(sizeof(T));
#else
		return (T*)get_allocator(sizeof(T))->allocate();
#endif
	}

	T* allocate(size_t count)
	{
#if BKE_CREATOR
		return (T*)malloc(sizeof(T) * count);
#else
		if (count * sizeof(T) > SMALL * MEMORY_UNIT)
			return (T*)malloc(sizeof(T) * count);
		return (T*)get_allocator(count * sizeof(T))->allocate();
#endif
	}

	void deallocate(T* p)
	{
#if BKE_CREATOR
		free(p);
#else
		get_allocator(sizeof(T))->deallocate(p);
#endif
	}

	void deallocate(T* p, size_t count)
	{
#if BKE_CREATOR
		free(p);
#else
		if (count * sizeof(T) > SMALL * MEMORY_UNIT)
			free(p);
		else
			get_allocator(count * sizeof(T))->deallocate(p);
#endif
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

#endif	/*INNER_ST*/