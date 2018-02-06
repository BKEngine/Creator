#include "poolmalloc.h"

#ifdef INNER_MT

PoolManager *_globalPool;

void __init_memorypool()
{
	_globalPool = new PoolManager();
}

thread_local ThreadPoolManager* thread_pool = nullptr;

void __init_memorypool_thread()
{
	thread_pool = new ThreadPoolManager();
}

void __uninit_memorypool()
{
	delete _globalPool;
}

int PoolUnitTailOffset = 16;

ThreadPoolManager::ThreadPoolManager()
{
	threadid = this_thread::get_id();
	_globalPool->thread_count++;
	_globalPool->thread_pools.push_back(this);
	memset(pools, 0, sizeof(pools));
	memset(freelists, 0, sizeof(freelists));
}

ThreadPoolManager::~ThreadPoolManager()
{
#ifdef INNER_MT_ST
	for (int i = 0; i < MAXELE / POOLUNIT; i++)
	{
		if (!pools[i])
			continue;
		do
		{
			auto p = pools[i];
			pools[i] = pools[i]->next;
			free(((uint8_t*)p) + sizeof(PoolTail) - POOLSIZE);
		}
		while (pools[i]);
	}
#endif
	_globalPool->thread_count--;
	_globalPool->thread_pools.remove(this);
}

PoolManager::PoolManager()
{
	if (sizeof(PoolUnitTail) > 16)
		PoolUnitTailOffset = 32;
	_globalPool = this;
}

PoolManager::~PoolManager()
{
	auto backup = thread_pools;
	for(auto &it : backup)
	{
		delete it;
	}
}

PoolTail* alloc_pool(int idx)
{
	uint8_t *pool = (uint8_t*)malloc(POOLSIZE);
	if (!pool)
		return nullptr;
	PoolTail *tail = (PoolTail*)(pool + POOLSIZE - sizeof(PoolTail));
	tail->count = 0;
	tail->cursor = pool;
	tail->endpos = pool + POOLEND(POOLUNIT * (idx + 1));
	tail->next = thread_pool->pools[idx];
	thread_pool->pools[idx] = tail;
	//tail->next = _threadPool.pools[idx].exchange(tail, memory_order_relaxed);
	return tail;
}

void * pool_malloc(size_t sz)
{
	assert(sz + sizeof(PoolUnitTail) < MAXELE);
	int idx = POOLIDX(sz);
	int sz2 = POOLUNIT * (idx + 1);
#ifdef INNER_MT_ST
	auto p = thread_pool->pools[idx];
#else
	auto p = thread_pool->pools[idx].load(memory_order_relaxed);
#endif // INNER_MT_ST
	return pool_malloc_idx(idx, sz2, p, &thread_pool->freelists[idx]);
}

#ifdef INNER_MT_ST
void * pool_malloc_idx(int idx, int fixsize, PoolTail * pool, PoolUnitTail ** freelist)
#else
void * pool_malloc_idx(int idx, int fixsize, PoolTail * pool, atomic<PoolUnitTail*>* freelist)
#endif
{
	if (!pool || pool->cursor >= pool->endpos)
	{
	#ifdef INNER_MT_ST
		auto pointer = *freelist;
		if (pointer)
		{
			*freelist = pointer->next;
			pointer->pool->count++;
			return ((uint8_t*)pointer) + sizeof(PoolUnitTail) - fixsize;
		}
	#else
		auto pointer = freelist->load(memory_order_relaxed);
		if (pointer)
		{
			PoolUnitTail *next;
			do
			{
				next = pointer->next;
			}
			while (!freelist->compare_exchange_weak(pointer, next, memory_order_relaxed));
			//do
			//{
			//	next = pointer->next;
			//	pointer = freelist->exchange(next, memory_order_relaxed);
			//}
			//while (freelist->load(memory_order_relaxed) != next);
			pointer->pool->count++;
			return ((uint8_t*)pointer) + PoolUnitTailOffset;
		}
	#endif
		pool = alloc_pool(idx);
		if (!pool)
			return nullptr;
	}
	PoolUnitTail *tail = (PoolUnitTail*)pool->cursor;
	pool->cursor += fixsize;
	tail->mgr = thread_pool;
	tail->pool = pool;
	tail->sz = fixsize;
	pool->count++;
	return (uint8_t*)tail + PoolUnitTailOffset;
}

void pool_free(void *p)
{
	assert(p);
	PoolUnitTail *tail = (PoolUnitTail*)(((uint8_t*)p) - PoolUnitTailOffset);
	pool_free_idx(tail->sz / POOLUNIT - 1, tail->sz, tail);
}

void pool_free(void * p, size_t sz)
{
	assert(p);
	int idx = POOLIDX(sz);
	int sz2 = POOLFIX(sz);
	PoolUnitTail *tail = (PoolUnitTail*)(((uint8_t*)p) - PoolUnitTailOffset);
	pool_free_idx(idx, sz2, tail);
}

void pool_free_idx(int idx, int fixsize, PoolUnitTail* tail)
{
	tail->pool->count--;
#ifdef INNER_MT_ST
	assert(tail->mgr = thread_pool);
	tail->next = thread_pool->freelists[idx];
#else
	ThreadPoolManager *mgr = tail->mgr;
	tail->next = mgr->freelists[idx].exchange(tail, memory_order_relaxed);
#endif
}

void shrink(ThreadPoolManager * mgr)
{
	//比较耗时，所以开个子线程慢慢处理吧233
	for (int i = 0; i < MAXELE / POOLUNIT; i++)
	{
	#ifdef INNER_MT_ST
		auto curpool = mgr->pools[i];	//即使count为0也不删除，成为一个dead block，等待下次shrink删除
	#else
		auto curpool = mgr->pools[i].load(memory_order_relaxed);	//即使count为0也不删除，成为一个dead block，等待下次shrink删除
	#endif // INNER_MT_ST

		{
			//先看看要不要删除
			int all = 1;
			int relax = 0;
			if (!curpool)
				continue;
			while (curpool->next)
			{
				curpool = curpool->next;
				all++;
				if (!curpool->count)
					relax++;
			}
			if (relax < 5)
				continue;
		}
	#ifdef INNER_MT_ST
		auto rawfree = mgr->freelists[i];
		mgr->freelists[i] = nullptr;
	#else
		auto rawfree = mgr->freelists[i].exchange(nullptr, memory_order_relaxed);
	#endif
		if (!rawfree)
			continue;
		while (rawfree && !rawfree->pool->count)
		{
			rawfree = rawfree->next;
		}
		if(rawfree)
		{
			while (rawfree->next && !rawfree->next->pool->count)
			{
				rawfree->next = rawfree->next->next;
				rawfree = rawfree->next;
			}
		}
	#ifdef INNER_MT_ST
		curpool = mgr->pools[i];
	#else
		curpool = mgr->pools[i].load(memory_order_relaxed);
	#endif
		while (curpool->next)
		{
			if (!curpool->next->count)
			{
				auto p = curpool->next;
				curpool->next = curpool->next->next;
				free(((uint8_t*)p) + sizeof(PoolTail) - POOLSIZE);
			}
			else
				curpool = curpool->next;
		}
	}
}

#endif //INNER_MT