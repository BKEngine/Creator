#pragma once

#include "Bagel_Config.h"

#ifdef INNER_MT

#include <stdint.h>
#include <memory>
#include <stdlib.h>
#include <mutex>
#include <atomic>
#include <thread>
#include <assert.h>

using namespace std;

#define POOLSIZE (1<<16)
#define MAXELE 1024
#define POOLUNIT 32
#define POOLFIX(x) ((x+POOLUNIT-1+sizeof(PoolUnitTail)) & ~(POOLUNIT-1))
#define POOLIDX(x) ((x+sizeof(PoolUnitTail)-1) / POOLUNIT)
#define POOLEND(u) ((POOLSIZE-sizeof(PoolTail)) / (u)) * (u)

extern int PoolUnitTailOffset;

struct PoolTail
{
	int32_t count;
	uint8_t *cursor;
	uint8_t *endpos;
	PoolTail *next;
};

struct ThreadPoolManager;

//追加在每块内存开头
struct PoolUnitTail
{
	PoolUnitTail *next;
	PoolTail *pool;
	ThreadPoolManager *mgr;
	int32_t sz;
};

struct ThreadPoolManager
{
#ifdef INNER_MT_ST
	PoolTail* pools[MAXELE / POOLUNIT];
	PoolUnitTail* freelists[MAXELE / POOLUNIT];
#else
	atomic<PoolTail*> pools[MAXELE / POOLUNIT];
	atomic<PoolUnitTail*> freelists[MAXELE / POOLUNIT];
#endif

	thread::id threadid;

	ThreadPoolManager();

	~ThreadPoolManager();
};

struct PoolManager
{
	list<ThreadPoolManager*> thread_pools;

	atomic_int thread_count;

	PoolManager();

	~PoolManager();
};

PoolTail* alloc_pool(int idx);

void* pool_malloc(size_t sz);

#ifdef INNER_MT_ST
void* pool_malloc_idx(int idx, int fixsize, PoolTail* pool, PoolUnitTail** freelist);
#else
void* pool_malloc_idx(int idx, int fixsize, PoolTail* pool, atomic<PoolUnitTail*>* freelist);
#endif

void pool_free(void *p);

void pool_free(void *p, size_t sz);

void pool_free_idx(int idx, int fixsize, PoolUnitTail *tail);

void shrink(ThreadPoolManager *mgr);

void __init_memorypool();

void __init_memorypool_thread();

void __uninit_memorypool();

extern thread_local ThreadPoolManager* thread_pool;

template<class T>
class BKE_PoolAllocator
{
protected:
	int idx;
	int fixsize;

	//PoolTail* pool;
	//atomic<PoolUnitTail*>* freelist;

	struct __helper
	{
		T a;
	};

public:
	typedef T value_type;
	typedef int32_t size_type;

	BKE_PoolAllocator();

	template <class T2>
	BKE_PoolAllocator(const BKE_PoolAllocator<T2> &alloc);

	template<class T2>
	bool operator == (const BKE_PoolAllocator<T2> &alloc) const
	{
		return idx == alloc.idx;
	}

	template<class T2>
	bool operator != (const BKE_PoolAllocator<T2> &alloc) const
	{
		return idx != alloc.idx;
	}

	T* allocate();

	T* allocate(size_t count);

	void deallocate(T* p);

	void deallocate(T* p, size_t count);

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

template<class T>
inline BKE_PoolAllocator<T>::BKE_PoolAllocator()
{
	idx = POOLIDX(sizeof(T));
	if (idx < MAXELE / POOLUNIT)
	{
		fixsize = POOLUNIT * (idx + 1);
		//pool = thread_pool->pools[idx];
		//freelist = &thread_pool->freelists[idx];
	}
	else
	{
		fixsize = 0;
		//pool = nullptr;
		//freelist = nullptr;
	}
}

template<class T>
inline T * BKE_PoolAllocator<T>::allocate()
{
	if (idx < MAXELE / POOLUNIT)
		return (T*)pool_malloc_idx(idx, fixsize, thread_pool->pools[idx], &thread_pool->freelists[idx]);
	else
		return (T*)malloc(sizeof(T));
}

template<class T>
inline T * BKE_PoolAllocator<T>::allocate(size_t count)
{
	if (count == 1)
		return allocate();
	else if (sizeof(T) * count + sizeof(PoolUnitTail) < MAXELE)
		return (T*)pool_malloc(sizeof(T) * count);
	else
		return (T*)malloc(fixsize * count);
}

template<class T>
inline void BKE_PoolAllocator<T>::deallocate(T * p)
{
	if (!p)
		return;
	if (idx < MAXELE / POOLUNIT)
	{
		PoolUnitTail *tail = (PoolUnitTail*)(((uint8_t*)p) - PoolUnitTailOffset);
		pool_free_idx(idx, fixsize, tail);
	}
	else
	{
		free(p);
	}
}

template<class T>
inline void BKE_PoolAllocator<T>::deallocate(T * p, size_t count)
{
	if (!p)
		return;
	if (count == 1)
		return deallocate(p);
	else if (sizeof(T) * count + sizeof(PoolUnitTail) < MAXELE)
		return pool_free(p);
	else
		return free(p);
}

template<class T>
template<class T2>
inline BKE_PoolAllocator<T>::BKE_PoolAllocator(const BKE_PoolAllocator<T2>& alloc)
{
	idx = POOLIDX(sizeof(T2));
	if (idx < MAXELE / POOLUNIT)
	{
		fixsize = POOLUNIT * (idx + 1);
		//pool = thread_pool->pools[idx];
		//freelist = &thread_pool->freelists[idx];
	}
	else
	{
		fixsize = 0;
		//pool = nullptr;
		//freelist = nullptr;
	}
}

#endif //INNER_MT