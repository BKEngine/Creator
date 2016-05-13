#pragma once

#include <stdlib.h>
#include "utils.h"
#include "memorypool.h"
#include <vector>
#include <list>
#include <initializer_list>

using namespace std;

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

template<class T>
class BKE_array
{
private:
	//typedef list<T, gc_allocator<T>> ele_t;
	typedef list<T> ele_t;
	//typedef list<T, BKE_allocator<T>> ele_t;

	ele_t eles;

	//vector<ele_t::iterator, gc_allocator<ele_t::iterator>> addrs;

	//may not need gc tracable

public:
	typedef typename ele_t::iterator raw_iterator;

	vector<raw_iterator> addrs;

	BKE_array()
	{
	}

	BKE_array & operator = (const BKE_array &arr)
	{
		clear();
		for (auto &&i : arr.addrs)
		{
			eles.emplace_front(*i);
			addrs.push_back(eles.begin());
		}
		return *this;
	}

	BKE_array(initializer_list<T> val)
	{
		addrs.resize(val.size());
		auto it = addrs.begin();
		for (auto &&i : val)
		{
			eles.emplace_front(i);
			*it++ = eles.begin();
		}
	}

	inline void clear()
	{
		addrs.clear();
		eles.clear();
	}

	~BKE_array()
	{
	}

	inline T& operator [] (const bkplong index) const
	{
		return *addrs[index];
	}

	inline T& back() const
	{
		return *addrs.back();
	}

	void resize(bkplong size)
	{
		_ASSERT_EXPR(size>=0, L"大小不能设为负数");
		size_t count = addrs.size();
		if(!size)
		{
			clear();
			return;
		}
		if (size == count)
			return;
		if(size > count)
		{
			for (size_t i = count; i < (size_t)size; i++)
			{
				eles.emplace_front();
				addrs.push_back(eles.begin());
			}
		}
		else
		{
			while (addrs.size() > size)
			{
				eles.erase(addrs.back());
				addrs.pop_back();
			}
		}
	}

	void reserve(bkplong size)
	{
		addrs.reserve(size);
	}

	T& push_back(const T &t)
	{
		eles.emplace_front(t);
		addrs.push_back(eles.begin());
		return back();
	}

	T& push_back(T &&t)
	{
		eles.emplace_front(t);
		addrs.push_back(eles.begin());
		return back();
	}

	T& push_back_new()
	{
		eles.emplace_front();
		addrs.push_back(eles.begin());
		return back();
	}

	void insert(bkplong index, const T& t)
	{
		eles.emplace_front(t);
		addrs.insert(addrs.begin() + index, eles.begin());
	}

	inline bkplong size() const
	{
		return addrs.size();
	}

	inline bool empty() const
	{
		return addrs.empty();
	}

	inline void pop_back()
	{
		eles.erase(addrs.back());
		addrs.pop_back();
	}

	inline void pop_back(T &res)
	{
		res = back();
		pop_back();
	}

	T take(bkplong index)
	{
		T value = this->operator[](index);
		this->erase(index);
		return value;
	}

	void erase(bkplong index)
	{
		eles.erase(addrs[index]);
		addrs.erase(addrs.begin() + index);
	}

	inline void eraseValue(const T &t)
	{
		for (auto it = addrs.begin(); it != addrs.end();)
		{
			if ((**it) == t)
			{
				eles.erase(*it);
				it = addrs.erase(it);
			}
			else
				++it;
		}
	}

	void swap(int idx1, int idx2)
	{
		assert(idx1 >= 0 && idx1 < addrs.size());
		assert(idx2 >= 0 && idx2 < addrs.size());
		if (idx1 == idx2)
			return;
		auto tmp = addrs[idx1];
		addrs[idx1] = addrs[idx2];
		addrs[idx2] = tmp;
	}
};

#pragma pop_macro("new")

