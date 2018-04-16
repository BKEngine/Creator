#pragma once

#include <stdlib.h>
#include "Bagel_Config.h"
#include "memorypool.h"
#include "poolmalloc.h"
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

/*
template<class T>
class BKE_array
{
protected:
	BKE_Allocator<T> al;

	vector<T*, BKE_Allocator<T*>> addrs;

	typedef typename vector<T*, BKE_Allocator<T*>>::iterator addr_iterator;

public:
	class iterator
	{
		friend class BKE_array;
	protected:
		addr_iterator it;

	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef T value_type;
		typedef intptr_t difference_type;
		typedef T* pointer;
		typedef T& reference;

		iterator()
		{
		};

		iterator(const addr_iterator &i) : it(i)
		{
		};

		iterator(const iterator &i) : it(i.it)
		{
		};

		bool operator == (const iterator &i) const
		{
			return it == i.it;
		}

		bool operator != (const iterator &i) const
		{
			return it != i.it;
		}

		bool operator > (const iterator &i) const
		{
			return it > i.it;
		}

		bool operator < (const iterator &i) const
		{
			return it < i.it;
		}

		bool operator >= (const iterator &i) const
		{
			return it >= i.it;
		}

		bool operator <= (const iterator &i) const
		{
			return it <= i.it;
		}

		iterator operator + (int a) const
		{
			return it + a;
		}

		iterator& operator += (int a)
		{
			it += a;
			return *this;
		}

		iterator operator - (int a) const
		{
			return it - a;
		}

		iterator& operator -= (int a)
		{
			it -= a;
			return *this;
		}

		difference_type operator - (const iterator &i) const
		{
			return it - i.it;
		}

		//++it
		iterator& operator ++ ()
		{
			++it;
			return *this;
		}

		//it++
		iterator operator ++ (int)
		{
			auto i = *this;
			++it;
			return i;
		}
		//--it
		iterator& operator -- ()
		{
			--it;
			return *this;
		}

		//it--
		iterator operator -- (int)
		{
			auto i = *this;
			--it;
			return i;
		}

		reference operator *() const
		{
			return **it;
		}

		pointer operator ->() const
		{
			return *it;
		}
	};
	class const_iterator
	{
		friend class BKE_array;
	protected:
		typedef typename vector<T*, BKE_Allocator<T*>>::const_iterator const_addr_iterator;
		const_addr_iterator it;

	public:
		typedef std::bidirectional_iterator_tag iterator_category;
		typedef T value_type;
		typedef intptr_t difference_type;
		typedef const T* pointer;
		typedef const T& reference;

		const_iterator()
		{
		};

		const_iterator(const const_addr_iterator &i) : it(i)
		{
		};

		const_iterator(const const_iterator &i) : it(i.it)
		{
		};

		bool operator == (const const_iterator &i) const
		{
			return it == i.it;
		}

		bool operator != (const const_iterator &i) const
		{
			return it != i.it;
		}

		bool operator > (const const_iterator &i) const
		{
			return it > i.it;
		}

		bool operator < (const const_iterator &i) const
		{
			return it < i.it;
		}

		bool operator >= (const const_iterator &i) const
		{
			return it >= i.it;
		}

		bool operator <= (const const_iterator &i) const
		{
			return it <= i.it;
		}

		const_iterator operator + (int a) const
		{
			return it + a;
		}

		const_iterator& operator += (int a)
		{
			it += a;
			return *this;
		}

		const_iterator operator - (int a) const
		{
			return it - a;
		}

		const_iterator& operator -= (int a)
		{
			it -= a;
			return *this;
		}

		difference_type operator - (const const_iterator &i) const
		{
			return it - i.it;
		}

		//++it
		const_iterator& operator ++ ()
		{
			++it;
			return *this;
		}

		//it++
		const_iterator operator ++ (int)
		{
			auto i = *this;
			++it;
			return i;
		}
		//--it
		const_iterator& operator -- ()
		{
			--it;
			return *this;
		}

		//it--
		const_iterator operator -- (int)
		{
			auto i = *this;
			--it;
			return i;
		}

		reference operator *() const
		{
			return **it;
		}

		pointer operator ->() const
		{
			return *it;
		}
	};
	//typedef addr_iterator iterator;
	//typedef typename vector<T*, BKE_Allocator<T*>>::const_iterator const_iterator;


	inline iterator begin()
	{
		return addrs.begin();
	}

	inline const_iterator begin() const
	{
		return addrs.begin();
	}

	inline const_iterator cbegin() const
	{
		return addrs.cbegin();
	}

	inline iterator end()
	{
		return addrs.end();
	}

	inline const_iterator end() const
	{
		return addrs.end();
	}

	inline const_iterator cend() const
	{
		return addrs.cend();
	}

	BKE_array()
	{
	}

	BKE_array & operator = (const BKE_array &arr)
	{
		clear();
		for (auto &&i : arr.addrs)
		{
			addrs.push_back(al.op_new(i));
		}
		return *this;
	}

	BKE_array(initializer_list<T> val)
	{
		addrs.resize(val.size());
		auto it = addrs.begin();
		for (auto &&i : val)
		{
			*it++ = al.op_new(i);
		}
	}

	inline void clear()
	{
		for (auto &i : addrs)
		{
			al.op_delete(i);
		}
		addrs.clear();
	}

	~BKE_array()
	{
	}

	inline T& operator [] (const int32_t index) const
	{
		return *addrs[index];
	}

	inline T& back() const
	{
		return *addrs.back();
	}

	void resize(int32_t size)
	{
		_ASSERT_EXPR(size>=0, W("大小不能设为负数"));
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
				addrs.push_back(al.op_new());
			}
		}
		else
		{
			while (addrs.size() > size)
			{
				al.op_delete(addrs.back());
				addrs.pop_back();
			}
		}
	}

	void reserve(int32_t size)
	{
		addrs.reserve(size);
	}

	T& push_back(const T &t)
	{
		addrs.push_back(al.op_new(t));
		return back();
	}

	T& push_back(T &&t)
	{
		addrs.push_back(al.op_new(std::move(t)));
		return back();
	}

	T& push_back_new()
	{
		addrs.push_back(al.op_new());
		return back();
	}

	void insert(int32_t index, const T& t)
	{
		addrs.insert(addrs.begin() + index, al.op_new(t));
	}

	void insert(const iterator &it, const T& t)
	{
		addrs.insert(it.it, al.op_new(t));
	}

	inline int32_t size() const
	{
		return addrs.size();
	}

	inline bool empty() const
	{
		return addrs.empty();
	}

	inline void pop_back()
	{
		al.op_delete(addrs.back());
		addrs.pop_back();
	}

	inline void pop_back(T &res)
	{
		res = back();
		pop_back();
	}

	void erase(int32_t index)
	{
		al.op_delete(addrs[index]);
		addrs.erase(addrs.begin() + index);
	}

	iterator erase(const iterator &it)
	{
		al.op_delete(it.operator->());
		return addrs.erase(it.it);
	}

	inline void eraseValue(const T &t)
	{
		for (auto it = addrs.begin(); it != addrs.end();)
		{
			if ((**it) == t)
			{
				al.op_delete(*it);
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
*/

//only for xlass T() which has T() and default T(const T&) and default ~T()
template<class T>
class BKE_array
{
	friend class Bagel_JitCompiler;
	static BKE_Allocator<T> memal;
	static_assert(std::is_trivially_copy_constructible<T>::value, "T should be trivially copy constructible");
	static_assert(std::is_trivially_destructible<T>::value, "T should be trivially destructible");
protected:
	T* _Myfirst;
	T* _Myend;
	
	int count;
	int _capacity;

	void _reserve(int n)
	{
		assert(n > _capacity);
		int raw = _capacity;
		do
		{
			_capacity <<= 1;
		}
		while (_capacity < n);
		T *ptr = memal.allocate(_capacity);
		if(count)
			memcpy(ptr, _Myfirst, count * sizeof(T));
		memal.deallocate(_Myfirst, raw);
		_Myfirst = ptr;
		_Myend = _Myfirst + count;
	}

	void _resize(int n)
	{
		assert(n > count);
		reserve(n);
		for (auto i = count; i < n; i++)
		{
			new (_Myfirst + i) T();
		}
		count = n;
		_Myend = _Myfirst + count;
	}

public:
	typedef T* iterator;
	typedef const T* const_iterator;


	BKE_array() : count(0), _capacity(8)
	{
		_Myfirst = memal.allocate(8);
		_Myend = _Myfirst;
	}

	BKE_array(int sz) : count(sz), _capacity(8)
	{
		_Myfirst = nullptr;
		while (_capacity < sz)
		{
			_capacity <<= 1;
		}
		_Myfirst = memal.allocate(_capacity);
		for (auto i = 0; i < sz; i++)
		{
			new (_Myfirst + i) T();
		}
		_Myend = _Myfirst + count;
	}

	BKE_array(std::initializer_list<T> val) : _capacity(8)
	{
		int sz = val.size();
		_Myfirst = nullptr;
		while (_capacity < sz)
		{
			_capacity <<= 1;
		}
		_Myfirst = memal.allocate(_capacity);
		int idx = 0;
		for (auto &it : val)
		{
			new (_Myfirst + idx) T(it);
			++idx;
		}
		count = sz;
		_Myend = _Myfirst + sz;
	}

	BKE_array(const BKE_array &arr)
	{
		_capacity = arr._capacity;
		count = arr.count;
		_Myfirst = memal.allocate(_capacity);
		_Myend = _Myfirst + count;
		memcpy(_Myfirst, arr._Myfirst, count * sizeof(T));
	}

	BKE_array(BKE_array &&arr)
	{
		_capacity = arr._capacity;
		count = arr.count;
		_Myfirst = arr._Myfirst;
		_Myend = arr._Myend;
		arr._Myfirst = nullptr;
		arr._Myend = nullptr;
		arr.count = 0;
	}

	~BKE_array()
	{
		memal.deallocate(_Myfirst, _capacity);
	}

	iterator begin()
	{
		return _Myfirst;
	}

	const_iterator begin() const
	{
		return _Myfirst;
	}

	iterator end()
	{
		return _Myend;
	}

	const_iterator end() const
	{
		return _Myend;
	}

	void resize(int n)
	{
		if (n > count)
		{
			reserve(n);
			for (auto i = count; i < n; i++)
			{
				new (_Myfirst + i) T();
			}
		}
		count = n;
		_Myend = _Myfirst + count;
	}

	void push_back(const T& t)
	{
		reserve(++count);
		_Myend = _Myfirst + count;
		new (_Myend - 1) T(t);
	}

	void push_back(T&& t)
	{
		reserve(++count);
		_Myend = _Myfirst + count;
		new (_Myend - 1) T(std::move(t));
	}

	void emplace_back()
	{
		resize(count + 1);
		new (_Myend - 1) T();
	}

	template <class ...Args>
	void emplace_back(const Args&... t)
	{
		resize(count + 1);
		new (_Myend - 1) T(std::forward<Args>(t)...);
	}

	T* pop_back()
	{
		--count;
		--_Myend;
		return _Myend;
	}

	void reserve(int n)
	{
		if (n <= _capacity)
			return;
		_reserve(n);
	}

	int size() const
	{
		return count;
	}

	int capacity() const
	{
		return _capacity;
	}

	T& operator [] (int n)
	{
		assert(n < count && n >= 0);
		return _Myfirst[n];
	}

	T operator [] (int n) const
	{
		assert(n < count && n >= 0);
		return _Myfirst[n];
	}

	void insert(int idx, const T& t)
	{
		assert(idx <= count && idx >= 0);
		insert(_Myfirst + idx, t);
	}

	void insert(const iterator &it, const T& t)
	{
		assert(it <= _Myend && it >= _Myfirst);
		if (count < _capacity)
		{
			if (it == _Myend)
			{
				new (_Myend) T(t);
			}
			else
			{
				memmove(it + 1, it, (uint8_t*)_Myend - (uint8_t*)it);
				new (it) T(t);
			}
			++count;
			++_Myend;
		}
		else
		{
			int raw = _capacity;
			++count;
			_capacity <<= 1;
			T *ptr = memal.allocate(_capacity);
			memcpy(ptr, _Myfirst, (uint8_t*)it - (uint8_t*)_Myfirst);
			new (ptr + (it - _Myfirst)) T(t);
			memcpy(ptr + (it - _Myfirst) + 1, it, (uint8_t*)_Myend - (uint8_t*)it);
			memal.deallocate(_Myfirst, raw);
			_Myfirst = ptr;
			_Myend = _Myfirst + count;
		}
	}

	template <class ...Args>
	void emplace(int idx, const Args&... t)
	{
		assert(idx <= count && idx >= 0);
		emplace(_Myfirst + idx, std::forward<Args>(t)...);
	}

	template <class ...Args>
	void emplace(const iterator &it, const Args&... t)
	{
		assert(it <= _Myend && it >= _Myfirst);
		if (count < _capacity)
		{
			if (it == _Myend)
			{
				new (_Myend) T(std::forward<Args>(t)...);
			}
			else
			{
				memmove(it + 1, it, (uint8_t*)_Myend - (uint8_t*)it);
				new (it) T(std::forward<Args>(t)...);
			}
			++count;
			++_Myend;
		}
		else
		{
			int raw = _capacity;
			++count;
			_capacity <<= 1;
			T *ptr = memal.allocate(_capacity);
			memcpy(ptr, _Myfirst, (uint8_t*)it - (uint8_t*)_Myfirst);
			new (ptr + (it - _Myfirst)) T(std::forward<Args>(t)...);
			memcpy(ptr + (it - _Myfirst) + 1, it, (uint8_t*)_Myend - (uint8_t*)it);
			memal.deallocate(_Myfirst, raw);
			_Myfirst = ptr;
			_Myend = _Myfirst + count;
		}
	}

	int erase(int n)
	{
		assert(n < count && n >= 0);
		erase(_Myfirst + n);
		return n;
	}

	iterator erase(const iterator &it)
	{
		assert(it < _Myend && it >= _Myfirst);
		--count;
		int sz = (uint8_t*)_Myend - (uint8_t*)it;
		memmove(it, it + 1, sz);
		--_Myend;
		return it;
	}

	void clear()
	{
		count = 0;
		_Myend = _Myfirst;
	}

	bool empty() const
	{
		return !count;
	}

	T& back()
	{
		assert(count > 0);
		return *(_Myend - 1);
	}

	T& front()
	{
		assert(count > 0);
		return *_Myfirst;
	}
};

template<class T>
BKE_Allocator<T> BKE_array<T>::memal;

#pragma pop_macro("new")

