#pragma once

#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <initializer_list>
#include <map>

#include "defines.h"

#include "memorypool.h"

//copied from MS xstddef
template<class T>
bkplong BKE_hash(const T &val)
{
	const bkpulong _FNV_offset_basis = 2166136261U;
	const bkpulong _FNV_prime = 16777619U;
	const unsigned char *p = (const unsigned char*)&val;
	const unsigned char *plim = (const unsigned char*)&val + sizeof(T);
	bkpulong ret = _FNV_offset_basis;
	while (p < plim)
	{
		ret ^= (unsigned char)*p;
		ret *= _FNV_prime;
		p++;
	}
	return (bkplong)ret;
};

template<class T>
bkplong BKE_hash(const T *val)
{
	const bkpulong _FNV_offset_basis = 2166136261U;
	const bkpulong _FNV_prime = 16777619U;
	const unsigned char *p = (const unsigned char*)&val;
	const unsigned char *plim = (const unsigned char*)&val + sizeof(T);
	bkpulong ret = _FNV_offset_basis;
	while (p < plim)
	{
		ret ^= (unsigned char)*p;
		ret *= _FNV_prime;
		p++;
	}
	return (bkplong)ret;
};

template<>
bkplong BKE_hash(const std::wstring &str);
template<>
bkplong BKE_hash(const std::string &str);
template<>
bkplong BKE_hash(const wchar_t* str);
template<>
bkplong BKE_hash(const char * str);

#define HASH_LEVEL 8

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
buf						start
[0]						NULL
[1]						NULL
...						NULL
[start->index]			start
[start->index+1]		NULL
...						NULL
[start->next->index]	start->next    =>    start->next->next
...						NULL
[hashsize]				NULL
*/

template<class _Key_t, class _Val_t>
class BKE_hashmap
{
protected:
	bkplong hashsize;
	bkplong count;

	struct _Content
	{
		const _Key_t first;
		_Val_t second;

		_Content(){};

		_Content(const _Key_t &k) :first(k) {};

		_Content(_Key_t &&k) :first(std::move(k)){};

		_Content(const _Content &c) :first(c.first), second(c.second){};

		_Content(_Content &&c) :first(std::move(c.first)), second(std::move(c.second)){};

		template<typename T, typename TT>
		_Content(const T &k, const TT &v) :first(k), second(v){};
	};

	struct BKE_HashNode
	{
		BKE_HashNode *next;
		BKE_HashNode *last;
		_Content ct;
		bkplong index;
		bkplong hashvalue;

		BKE_HashNode(){};

		BKE_HashNode(const _Key_t &k) :ct(k){};

		BKE_HashNode(_Key_t &&k) :ct(std::move(k)) {};

		BKE_HashNode(BKE_HashNode &&c) :ct(std::move(c.ct)), index(c.index), hashvalue(c.hashvalue){ };

		BKE_HashNode(const BKE_HashNode &c) :ct(c.ct), index(c.index), hashvalue(c.hashvalue){ };

		template<typename T, typename TT>
		BKE_HashNode(const T &k, const TT &v) :ct(k, v){};
	};

	char _head[sizeof(BKE_HashNode)];
	char _end[sizeof(BKE_HashNode)];

	BKE_allocator<BKE_HashNode> al;

	//BKE_HashNode* buf[1<<hashlevel];
	BKE_HashNode** buf;

	//start of ListNode
	//pointer to real header of ListNode
	//list is just for iterator and vs's debug view, not hint to real hash sequence
	BKE_HashNode &start;
	BKE_HashNode &stop;

	struct _Iterator
	{
		friend class BKE_hashmap;
	private:
		mutable BKE_HashNode *node;
	public:
		_Iterator() :node(NULL){};

		_Iterator(BKE_HashNode *n) :node(n){};

		_Iterator& operator =(const _Iterator &it)
		{
			node = it.node;
			return *this;
		}
		const _Iterator& operator ++() const
		{
			_ASSERT_EXPR(node->next != NULL, L"end iterator不能再++");
			this->node = node->next;
			return *this;
		}
		_Iterator& operator ++()
		{
			_ASSERT_EXPR(node->next != NULL, L"end iterator不能再++");
			this->node = node->next;
			return *this;
		}
		const _Iterator& operator --() const
		{
			_ASSERT_EXPR(node->last != NULL, L"begin iterator不能再--");
			this->node = node->last;
			return *this;
		}
		_Iterator& operator --()
		{
			_ASSERT_EXPR(node->last != NULL, L"begin iterator不能再--");
			this->node = node->last;
			return *this;
		}
		bool operator ==(const _Iterator &it) const
		{
			return node == it.node;
		}
		inline bool operator !=(const _Iterator &it) const
		{
			return node != it.node;
		}
		const _Iterator operator ++(int) const
		{
			auto it = *this;
			++(*this);
			return it;
		}
		const _Iterator operator --(int) const
		{
			auto it = *this;
			--(*this);
			return it;
		}
		_Iterator operator ++(int)
		{
			auto it = *this;
			++(*this);
			return it;
		}
		_Iterator operator --(int)
		{
			auto it = *this;
			--(*this);
			return it;
		}
		_Content *operator ->()
		{
			_ASSERT_EXPR(node->next != NULL, L"end没有->运算");
			return &node->ct;
		}
		const _Content *operator ->() const
		{
			_ASSERT_EXPR(node->next != NULL, L"end没有->运算");
			return &node->ct;
		}
		_Content &operator *()
		{
			return node->ct;
		}
		const _Content &operator *() const
		{
			return node->ct;
		}
	};

	template<typename T, 
		typename = typename std::enable_if<std::is_constructible<_Key_t, T>::value>::type
	>
	BKE_HashNode *_getNode(T &&key)
	{
		bkplong ha = BKE_hash(key);
		bkplong h = ha & (hashsize - 1);
		if (buf[h] == NULL)
		{
			buf[h] = al.op_new(std::forward<T>(key));
			buf[h]->index = h;
			buf[h]->hashvalue = ha;
			buf[h]->next = start.next;
			buf[h]->last = &start;
			start.next->last = buf[h];
			start.next = buf[h];
			count++;
			return buf[h];
		}
		else
		{
			auto node = buf[h];
			while (node && node->index == h)
			{
				if (node->ct.first == key)
					return node;
				node = node->next;
			}
			//insert after buf[h]
			auto newnode = al.op_new(std::forward<T>(key));
			newnode->index = h;
			newnode->hashvalue = ha;
			newnode->last = buf[h];
			newnode->next = buf[h]->next;
			buf[h]->next->last = newnode;
			buf[h]->next = newnode;
			count++;
			return newnode;
		}
	}

public:
	typedef _Iterator iterator;
	typedef const _Iterator const_iterator;

	inline iterator begin() const
	{
		return iterator(start.next);
	}

	inline const_iterator cbegin() const
	{
		return begin();
	}

	inline iterator end() const
	{
		return iterator(&stop);
	}

	inline const_iterator cend() const
	{
		return end();
	}

	void clear()
	{
		if (!buf || !count)
			return;
		auto s = start.next;
		while (s != &stop)
		{
			auto s2 = s->next;
			al.op_delete(s);
			s = s2;
		}
		memset(buf, 0, sizeof(void*) * hashsize);
		start.next = &stop;
		stop.last = &start;
		count = 0;
	}

	template<typename T>
	iterator find(const T &key) const
	{
		iterator it = end();
		if (count == 0)
			return it;
		bkplong h = BKE_hash(key) & (hashsize - 1);
		BKE_HashNode *node = buf[h];
		while (node && node->index == h)
		{
			if (node->ct.first == key)
			{
				it.node = node;
				return it;
			}
			node = node->next;
		}
		return it;
	}

	template<typename T, typename TT,
		typename = typename std::enable_if<std::is_constructible<_Key_t, T>::value>::type,
		typename = typename std::enable_if<std::is_constructible<_Val_t, TT>::value>::type
		>
	typename std::remove_cv<typename std::remove_reference<TT>::type>::type& insert(T &&key, TT &&val)
	{
		auto node = _getNode(std::forward<T>(key));
		node->ct.second = std::forward<TT>(val);
		return node->ct.second;
	}

	template<typename T, typename TT,
		typename = typename std::enable_if<std::is_constructible<_Key_t, T>::value>::type,
		typename = typename std::enable_if<std::is_constructible<_Val_t, TT>::value>::type
	>
	iterator insert(const std::pair<T, TT> &p)
	{
		auto node = _getNode(p.first);
		node->ct.second = p.second;
		return iterator(node);
	}

	bool find(const _Key_t &key, _Val_t &val) const
	{
		if (count == 0)
			return false;
		bkplong h = BKE_hash(key) & (hashsize - 1);
		BKE_HashNode *node = buf[h];
		while (node && node->index == h)
		{
			if (node->ct.first == key)
			{
				val = node->ct.second;
				return true;
			}
			node = node->next;
		}
		return false;
	}

	bool find(const _Key_t &key, _Val_t *&val) const
	{
		if (count == 0)
			return false;
		bkplong h = BKE_hash(key) & (hashsize - 1);
		BKE_HashNode *node = buf[h];
		while (node && node->index == h)
		{
			if (node->ct.first == key)
			{
				val = &node->ct.second;
				return true;
			}
			node = node->next;
		}
		return false;
	}

	inline bool contains(const _Key_t &key) const
	{
		if (count == 0)
			return false;
		bkplong h = BKE_hash(key) & (hashsize - 1);
		BKE_HashNode *node = buf[h];
		while (node && node->index == h)
		{
			if (node->ct.first == key)
			{
				return true;
			}
			node = node->next;
		}
		return false;
	}

	iterator erase(const iterator &it)
	{
		if (it != end())
		{
			iterator it2 = it;
			++it2;
			it.node->next->last = it.node->last;
			it.node->last->next = it.node->next;
			if (buf[it.node->index] == it.node)
			{
				buf[it.node->index] = it.node->next;
				if (it.node->next->index != it.node->index)
					buf[it.node->index] = NULL;
			}
			al.op_delete(it.node);
			count--;
			return it2;
		}
		return it;
	}

	inline void erase(const _Key_t &key)
	{
		erase(find(key));
	}

	inline bkplong getCount() const
	{
		return count;
	};

	//hashlevel根据hash表中元素的大约个数来选取，hash表的大小为2的hashlevel次方
	BKE_hashmap(int initsize = HASH_LEVEL) :start(*reinterpret_cast<BKE_HashNode*>(_head)), stop(*reinterpret_cast<BKE_HashNode*>(_end))
	{
		//static_assert(hashlevel >= 4, "hashlevel必须不小于4");
		this->hashsize = 1 << initsize;
		count = 0;
		buf = (BKE_HashNode **)malloc(sizeof(void*) * hashsize);
		memset(buf, 0, sizeof(void*) * hashsize);
		start.index = stop.index = -1;
		start.last = NULL;
		start.next = &stop;
		stop.last = &start;
		stop.next = NULL;
	}

	struct KeyValuePair
	{
		const _Key_t key;
		const _Val_t value;
		KeyValuePair(const _Key_t &k, const _Val_t &v) :key(k), value(v){}
	};

	BKE_hashmap(std::initializer_list<KeyValuePair> l) :BKE_hashmap(l.size() < 16 ? 4 : HASH_LEVEL)
	{
		for (auto it = l.begin(); it != l.end(); it++)
			insert(it->key, it->value);
	}

	~BKE_hashmap()
	{
		if (!buf)
			return;
		BKE_HashNode *s = start.next;
		while (s != &stop)
		{
			auto s2 = s->next;
			al.op_delete(s);
			s = s2;
		}
		free(buf);
	}

	template<typename T,
		typename = typename std::enable_if<std::is_constructible<_Key_t, T>::value>::type
		>
	inline _Val_t& operator [](T &&key)
	{
		return _getNode(std::forward<T>(key))->ct.second;
	}

	template<typename T,
		typename = typename std::enable_if<std::is_constructible<_Key_t, T>::value>::type
	>
	inline _Key_t& insertKey(T &&key)
	{
		return _getNode(std::forward<T>(key))->ct.first;
	}

	//copy constructor
	//notice: the sequence of Nodes in same leaf may not same
	BKE_hashmap(const BKE_hashmap<_Key_t, _Val_t> &h) :start(*reinterpret_cast<BKE_HashNode*>(_head)), stop(*reinterpret_cast<BKE_HashNode*>(_end))
	{
		hashsize = h.hashsize;
		buf = (BKE_HashNode **)malloc(sizeof(void*)* hashsize);
		memset(buf, 0, sizeof(void*)* hashsize);
		count = h.count;
		start.index = stop.index = -1;
		start.last = NULL;
		start.next = &stop;
		stop.last = &start;
		stop.next = NULL;
		auto node = h.start.next;
		while (node != &h.stop)
		{
			auto s = al.op_new(*node);
			s->last = stop.last;
			s->next = &stop;
			stop.last->next = s;
			stop.last = s;
			if (buf[s->index] == NULL)
				buf[s->index] = s;
			node = node->next;
		}
	}

	template<class T>
	BKE_hashmap(const BKE_hashmap<_Key_t, _Val_t> &h, const T &func) :start(*reinterpret_cast<BKE_HashNode*>(_head)), stop(*reinterpret_cast<BKE_HashNode*>(_end))
	{
		hashsize = h.hashsize;
		buf = (BKE_HashNode **)malloc(sizeof(void*)* hashsize);
		memset(buf, 0, sizeof(void*)* hashsize);
		count = h.count;
		start.index = stop.index = -1;
		start.last = NULL;
		start.next = &stop;
		stop.last = &start;
		stop.next = NULL;
		auto node = h.start.next;
		while (node != &h.stop)
		{
			auto s = al.op_new(node->ct.first);
			s->val = func(node->ct.second);
			s->hashvalue = node->hashvalue;
			s->index = node->index;
			s->last = stop.last;
			s->next = &stop;
			stop.last->next = s;
			stop.last = s;
			if (buf[s->index] == NULL)
				buf[s->index] = s;
			node = node->next;
		}
	}

	BKE_hashmap(BKE_hashmap<_Key_t, _Val_t> &&h) :start(*reinterpret_cast<BKE_HashNode*>(_head)), stop(*reinterpret_cast<BKE_HashNode*>(_end))
	{
		hashsize = h.hashsize;
		buf = h.buf;
		h.buf = NULL;
		count = h.count;
		h.count = 0;
		start.index = stop.index = -1;
		start.last = NULL;
		start.next = h.start.next;
		start.next->last = &start;
		stop.last = h.stop.last;
		stop.last->next = &stop;
		stop.next = NULL;
		h.stop.last = &h.start;
		h.start.next = &h.stop;
	}

	BKE_hashmap& operator = (BKE_hashmap<_Key_t, _Val_t> &&h)
	{
		if (buf)
		{
			BKE_HashNode *s = start.next;
			while (s != &stop)
			{
				auto s2 = s->next;
				al.op_delete(s);
				s = s2;
			}
		}
		free(buf);
		hashsize = h.hashsize;
		buf = h.buf;
		h.buf = NULL;
		count = h.count;
		h.count = 0;
		start.next = h.start.next;
		start.next->last = &start;
		stop.last = h.stop.last;
		stop.last->next = &stop;
		h.stop.last = &h.start;
		h.start.next = &h.stop;
		return *this;
	}

	BKE_hashmap& operator = (const BKE_hashmap<_Key_t, _Val_t> &h)
	{
		if (buf)
		{
			BKE_HashNode *s = start.next;
			while (s != &stop)
			{
				auto s2 = s->next;
				al.op_delete(s);
				s = s2;
			}
		}
		if (h.hashsize != hashsize)
		{
			free(buf);
			buf = (BKE_HashNode **)malloc(sizeof(void*)* h.hashsize);
			hashsize = h.hashsize;
		}
		memset(buf, 0, sizeof(void*)* hashsize);
		count = h.count;
		start.next = &stop;
		stop.last = &start;
		auto node = h.start.next;
		while (node != &h.stop)
		{
			auto s = al.op_new(*node);
			s->last = stop.last;
			s->next = &stop;
			stop.last->next = s;
			stop.last = s;
			if (buf[s->index] == NULL)
				buf[s->index] = s;
			node = node->next;
		}
		return *this;
	}

	inline bool empty() const { return count == 0; }

	void Union(const BKE_hashmap<_Key_t, _Val_t> &h, bool Override = false)
	{
		if (h.empty())
			return;
		if (empty())
		{
			*this = h;
			return;
		}
		if (h.hashsize != hashsize || h.count + count < hashsize)
		{
			for (auto it = h.begin(); it != h.end(); ++it)
			{
				auto it2 = find(it->first);
				if (it2 == end())
					insert(it->first, it->second);
				else if (Override)
					it2->second = it->second;
			}
			return;
		}
		for (int i = 0; i < hashsize; i++)
		{
			BKE_HashNode *hnode = h.buf[i];
			if (hnode == NULL)
				continue;
			BKE_HashNode *nodelist = NULL;
			BKE_HashNode *nodelistend = NULL;
			while (hnode->index == i)
			{
				BKE_HashNode *node = buf[i];
				bool append = false;
				while (node && node->index == i)
				{
					if (node->ct.first == hnode->ct.first)
					{
						if (Override)
						{
							node->last->next = node->next;
							node->next->last = node->last;
							al.op_delete(node);
							append = true;
						}
						break;
					}
					node = node->next;
				}
				if (append)
				{
					if (!nodelist)
						nodelist = nodelistend = al.op_new(*hnode);
					else
					{
						BKE_HashNode *n = al.op_new(*hnode);
						if (nodelistend != nodelist)
						{
							n->next = nodelistend;
							n->last = nodelistend->last;
							nodelistend->last->next = n;
							nodelistend->last = n;
						}
						else
						{
							n->last = nodelist;
							nodelist->next = n;
						}
					}
				}
				hnode = hnode->next;
			}
			if (nodelist)
			{
				if (!buf[i])
				{
					nodelist->last = &start;
					nodelistend->next = start.next;
					start.next->last = nodelistend;
					start.next = nodelist;
					buf[i] = start.next;
				}
				else
				{
					nodelist->last = buf[i]->last;
					nodelistend->next = buf[i];
					buf[i]->last->next = nodelist;
					buf[i]->last = nodelistend;
					buf[i] = nodelist;
				}
			}
		}
	}

	void resizeTableSize(int newsize)
	{
		assert(newsize >= 4);
		if ((1L << newsize) == hashsize)
			return;
		hashsize = 1 << newsize;
		auto buf2 = (BKE_HashNode **)malloc(sizeof(void*)* hashsize);
		memset(buf2, 0, sizeof(void*)* hashsize);
		if (count == 0)
		{
			free(buf);
			buf = buf2;
			return;
		}
		auto raws = start.next;
		start.next = &stop;
		auto b = buf;
		buf = buf2;
		buf2 = b;
		while (raws != &stop)
		{
			raws->index = raws->hashvalue & (hashsize - 1);
			auto nexts = raws->next;
			if (buf[raws->index] == NULL)
			{
				buf[raws->index] = raws;
				raws->last = &start;
				raws->next = start.next;
				start.next->last = raws;
				start.next = raws;
			}
			else
			{
				raws->next = buf[raws->index];
				raws->last = buf[raws->index]->last;
				buf[raws->index]->last->next = raws;
				buf[raws->index]->last = raws;
			}
			raws = nexts;
		}
		free(buf2);
	}
};

//dummy class used in GlobalStringMap, to make the map actually a set
struct _dummy_class
{
};

template <class Key_t>
class BKE_hashset : public BKE_hashmap < Key_t, _dummy_class >
{
};

//T.last and T.next must be exist and be the first two element
template <class T>
class BKE_ListTemplate
{
protected:
	T* _start[2];
	T* _stop[2];

	T* start;
	T* stop;

	int num;

	struct _Iterator
	{
		friend class BKE_ListTemplate;
	private:
		mutable T *node;
	public:
		_Iterator() :node(NULL) {};

		_Iterator(T *n) :node(n) {};

		_Iterator& operator =(const _Iterator &it)
		{
			node = it.node;
			return *this;
		}
		const _Iterator& operator ++() const
		{
			_ASSERT_EXPR(node->next != NULL, L"end iterator不能再++");
			this->node = node->next;
			return *this;
		}
		_Iterator& operator ++()
		{
			_ASSERT_EXPR(node->next != NULL, L"end iterator不能再++");
			this->node = node->next;
			return *this;
		}
		const _Iterator& operator --() const
		{
			_ASSERT_EXPR(node->last != NULL, L"begin iterator不能再--");
			this->node = node->last;
			return *this;
		}
		_Iterator& operator --()
		{
			_ASSERT_EXPR(node->last != NULL, L"begin iterator不能再--");
			this->node = node->last;
			return *this;
		}
		bool operator ==(const _Iterator &it) const
		{
			return node == it.node;
		}
		inline bool operator !=(const _Iterator &it) const
		{
			return node != it.node;
		}
		const _Iterator operator ++(int) const
		{
			auto it = *this;
			++(*this);
			return it;
		}
		const _Iterator operator --(int) const
		{
			auto it = *this;
			--(*this);
			return it;
		}
		_Iterator operator ++(int)
		{
			auto it = *this;
			++(*this);
			return it;
		}
		_Iterator operator --(int)
		{
			auto it = *this;
			--(*this);
			return it;
		}
		T *operator ->()
		{
			_ASSERT_EXPR(node->next != NULL, L"end没有->运算");
			return node;
		}
		const T *operator ->() const
		{
			_ASSERT_EXPR(node->next != NULL, L"end没有->运算");
			return node;
		}
		T &operator *()
		{
			return *node;
		}
		const T &operator *() const
		{
			return *node;
		}
		operator T* ()
		{
			return node;
		}
		operator const T* () const
		{
			return node;
		}
	};

public:
	typedef _Iterator iterator;
	typedef const _Iterator const_iterator;

	BKE_ListTemplate()
	{
		start = (T*)_start;
		stop = (T*)_stop;

		_start[0] = NULL;
		_start[1] = stop;
		_stop[0] = start;
		_stop[1] = NULL;

		num = 0;
	}

	inline iterator begin() const
	{
		return iterator(start->next);
	}

	inline const_iterator cbegin() const
	{
		return begin();
	}

	inline iterator end() const
	{
		return iterator(stop);
	}

	inline const_iterator cend() const
	{
		return end();
	}

	iterator push_back(T* p)
	{
		p->next = stop;
		p->last = stop->last;
		p->last->next = p;
		stop->last = p;
		num++;
		return iterator(p);
	}

	iterator erase(T* p)
	{
		p->next->last = p->last;
		p->last->next = p->next;
		num--;
		return iterator(p->next);
	}

	void clear()
	{
		num = 0;
		_start[0] = NULL;
		_start[1] = stop;
		_stop[0] = start;
		_stop[1] = NULL;
	}
};