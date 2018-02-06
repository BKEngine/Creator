#pragma once

#include "Bagel_Config.h"
//#include "Bagel_Var.h"

using namespace std;

#define DEFAULT_STACKSIZE 1024

template <class T, int reserve = DEFAULT_STACKSIZE>
//#define T Bagel_Var
class StackAllocator
{
protected:
	struct AllocUnit
	{
		int used;
		int capacity;
		T* addr;

		AllocUnit(int n) : used(0), capacity(n)
		{
			addr = new T[n];
		}

		~AllocUnit()
		{
			delete[] addr;
		}
	};

	list<AllocUnit, BKE_Allocator<AllocUnit>> buf;

	typename list<AllocUnit, BKE_Allocator<AllocUnit>>::iterator cur;

public:
	StackAllocator()
	{
		buf.emplace_back(reserve);
		cur = buf.begin();
	}

	T* allocate(int n)
	{
		if (cur->used + n <= cur->capacity)
		{
			T* res = cur->addr + cur->used;
			cur->used += n;
			return res;
		}
		++cur;
		if (cur == buf.end())
		{
			buf.emplace_back(n > reserve ? n : reserve);
			--cur;
		}
		cur->used = n;
		return cur->addr;
	}

	void deallocate(int n)
	{
		if (!cur->used && cur != buf.begin())
			--cur;
		cur->used -= n;
	}
};