#pragma once

#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <initializer_list>

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
	while(p<plim)
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
	while(p<plim)
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

template<class _Key_t, class _Val_t, bkplong hashlevel=HASH_LEVEL>
class BKE_hashmap
{
protected:
	bkplong hashsize;
	bkplong count;

	struct BKE_HashNode
	{
		_Key_t key;
		_Val_t val;
		BKE_HashNode *last;
		BKE_HashNode *next;
		bkplong index;
		bkplong hashvalue;
		struct _Content
		{
			const _Key_t &first;
			_Val_t& second;

			_Content(const _Key_t &key, _Val_t &val):first(key), second(val)
			{};
		}_ct;

		BKE_HashNode():_ct(key, val){};

		template<typename T>
		BKE_HashNode(const T &k) :key(k), _ct(key, val){};

		BKE_HashNode(const BKE_HashNode &c) :key(c.key), val(c.val), _ct(key, val){ hashvalue = BKE_hash(key); };

		template<typename T, typename TT>
		BKE_HashNode(const T &k, const TT &v) :key(k), val(v), _ct(key, val){};
	};

	BKE_allocator<BKE_HashNode> al;

	//BKE_HashNode* buf[1<<hashlevel];
	BKE_HashNode** buf;

	struct _Iterator
	{
		friend class BKE_hashmap;
	private:
		BKE_HashNode *node;
		bkplong index;
		const BKE_hashmap *_this;
	public:
		_Iterator() :node(NULL), index(0), _this(NULL){};

		_Iterator& operator =(const _Iterator &it)
		{
			node=it.node;
			index = it.index;
			_this=it._this;
			return *this;
		}
		const _Iterator& operator ++() const
		{
			_ASSERT_EXPR(node, L"end iterator不能再++");
			//if(!node)
			//	return *this;
			auto it = const_cast<_Iterator*>(this);
			if (node->next != NULL)
			{
				it->node = node->next;
				return *this;
			}
			else
			{
				while(index<_this->hashsize - 1)
				{
					it->node = _this->buf[++(it->index)];
					if (node)
					{
						return *this;
					}
				}
				node = NULL;
			}
			return *this;
		}
		_Iterator& operator ++()
		{
			_ASSERT_EXPR(node, L"end iterator不能再++");
			//if(!node)
			//	return *this;
			auto it = this;
			if (node->next != NULL)
			{
				it->node = node->next;
				return *this;
			}
			else
			{
				while (index<_this->hashsize - 1)
				{
					it->node = _this->buf[++(it->index)];
					if (node)
					{
						return *this;
					}
				}
				node = NULL;
			}
			return *this;
		}
		const _Iterator& operator --() const
		{
			auto it = const_cast<_Iterator*>(this);
			if (node->last != NULL)
			{
				it->node = node->last;
				return *this;
			}
			else
			{
				if (!node)
					it->index = _this->hashsize;
				while(index>0)
				{
					BKE_HashNode *node = _this->buf[--(it->index)];
					if(node)
					{
						while(node->next!=NULL)
						{
							node=node->next;
						}
						it->node = node;
						return *this;
					}
				}
			}
			_ASSERT_EXPR(false, L"begin iterator不能再--");
			return *this;
		}
		_Iterator& operator --()
		{
			auto it = this;
			if (node->last != NULL)
			{
				it->node = node->last;
				return *this;
			}
			else
			{
				if (!node)
					it->index = _this->hashsize;
				while (index>0)
				{
					BKE_HashNode *node = _this->buf[--(it->index)];
					if (node)
					{
						while (node->next != NULL)
						{
							node = node->next;
						}
						it->node = node;
						return *this;
					}
				}
			}
			_ASSERT_EXPR(false, L"begin iterator不能再--");
			return *this;
		}
		bool operator ==(const _Iterator &it) const
		{
			if(_this!=it._this)
				return false;
			if(node!=it.node)
				return false;
			return true;
		}
		inline bool operator !=(const _Iterator &it) const
		{
			return !operator ==(it);
		}
		const _Iterator operator ++(int) const
		{
			auto it=*this;
			++(*this);
			return it;
		}
		const _Iterator operator --(int) const
		{
			auto it=*this;
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
		typename BKE_HashNode::_Content *operator ->() const
		{
			_ASSERT_EXPR(node, L"end没有->运算");
			//if(!node)
			//	return NULL;
			return &node->_ct;
		}
		typename BKE_HashNode::_Content &operator *() const
		{
			return *operator ->();
		}
	};

	template<typename T>
	BKE_HashNode *_getNode(const T &key)
	{
		bkplong ha = BKE_hash(key);
		bkplong h = ha & (hashsize - 1);
		if (buf[h] == NULL)
		{
			buf[h] = al.op_new(key);
			buf[h]->index = h;
			buf[h]->last = NULL;
			buf[h]->next = NULL;
			buf[h]->hashvalue = ha;
			count++;
			return buf[h];
		}
		else
		{
			BKE_HashNode *node = buf[h];
			if (node->key == key)
				return node;
			while (node->next != NULL)
			{
				node = node->next;
				if (node->key == key)
					return node;
			}
			node->next = al.op_new(key);
			node->next->index = h;
			node->next->last = node;
			node->next->next = NULL;
			node->next->hashvalue = ha;
			count++;
			return node->next;
		}
	}

public:
	typedef _Iterator iterator;
	typedef const _Iterator const_iterator;

	inline iterator begin() const
	{
		iterator it;
		bkplong i = 0;
		it.node=buf[i++];
		while(!it.node && i<hashsize)
		{
			it.node=buf[i++];
		}
		it._this=this;
		it.index = i - 1;
		return it;
	}

	inline const_iterator cbegin() const
	{
		return begin();
	}

	inline iterator end() const
	{
		iterator it;
		it.node=NULL;
		it._this=this;
		return it;
	}

	inline const_iterator cend() const
	{
		return end();
	}

	void clear()
	{
		if (!buf || !count)
			return;
		for(int i=0;i<hashsize;i++)
		{
			while(buf[i]!=NULL)
			{
				BKE_HashNode *node=buf[i];
				buf[i]=node->next;
				al.op_delete(node);
			}
		}
		count = 0;
	}

	template<typename T>
	iterator find(const T &key) const
	{
		bkplong h = BKE_hash(key) & (hashsize - 1);
		BKE_HashNode *node = buf[h];
		iterator it = end();
		while (node != NULL)
		{
			if (node->key == key)
			{
				it.node = node;
				return it;
			}
			node = node->next;
		}
		return it;
	}

	template<typename T, typename TT>
	TT& insert(const T &key, const TT &val)
	{
		bkplong ha = BKE_hash(key);
		bkplong h = ha & (hashsize - 1);
		if (buf[h] == NULL)
		{
			buf[h] = al.op_new(key, val);
			buf[h]->index = h;
			buf[h]->last = NULL;
			buf[h]->next = NULL;
			buf[h]->hashvalue = ha;
			count++;
			return buf[h]->val;
		}
		else
		{
			BKE_HashNode *node = buf[h];
			if (node->key == key)
			{
				node->val=val;
				return node->val;
			}
			while (node->next != NULL)
			{
				node = node->next;
				if (node->key == key)
				{
					node->val = val;
					return node->val;
				}
			}
			node->next = al.op_new(key, val);
			node->next->index = h;
			node->next->last = node;
			node->next->next = NULL;
			node->next->hashvalue = ha;
			count++;
			return node->next->val;
		}
	}

	bool find(const _Key_t &key, _Val_t &val) const
	{
		bkplong h = BKE_hash(key) & (hashsize - 1);
		BKE_HashNode *node = buf[h];
		while (node != NULL)
		{
			if (node->key == key)
			{
				val = node->val;
				return true;
			}
			node = node->next;
		}
		return false;
	}

	bool find(const _Key_t &key, _Val_t *&val) const
	{
		bkplong h = BKE_hash(key) & (hashsize - 1);
		BKE_HashNode *node = buf[h];
		while (node != NULL)
		{
			if (node->key == key)
			{
				val = &node->val;
				return true;
			}
			node = node->next;
		}
		return false;
	}

	inline bool contains(const _Key_t &key) const
	{
		bkplong h = BKE_hash(key) & (hashsize - 1);
		BKE_HashNode *node = buf[h];
		while (node != NULL)
		{
			if (node->key == key)
			{
				return true;
			}
			node = node->next;
		}
		return false;
	}

	iterator erase(const iterator &it)
	{
		if(it!=end())
		{
			iterator it2 = it;
			++it2;
			if(it.node->next)
				it.node->next->last=it.node->last;
			if(it.node->last)
				it.node->last->next=it.node->next;
			else
				buf[it.node->index]=it.node->next;
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
	BKE_hashmap()
	{
		static_assert(hashlevel>=4, "hashlevel必须不小于4");
		this->hashsize = 1L << hashlevel;
		count = 0;
		buf=(BKE_HashNode **)malloc(sizeof(void*) * hashsize);
		memset(buf, 0, sizeof(void*) * hashsize);
	}

	struct KeyValuePair
	{
		const _Key_t key;
		const _Val_t value;
		KeyValuePair(const _Key_t &k, const _Val_t &v) :key(k), value(v){}
	};

	BKE_hashmap(std::initializer_list<KeyValuePair> l) :BKE_hashmap()
	{
		for (auto it = l.begin(); it != l.end(); it++)
			insert(it->key, it->value); 
	}

	~BKE_hashmap()
	{
		clear();
		free(buf);
	}

	template<typename T>
	inline _Val_t& operator [](const T &key)
	{
		return _getNode<T>(key)->val;
	}

	template<typename T>
	inline _Key_t& insertKey(const T &key)
	{
		return _getNode<T>(key)->key;
	}

	//copy constructor
	//notice: the sequence of Nodes in same leaf may not same
	BKE_hashmap(const BKE_hashmap<_Key_t, _Val_t, hashlevel> &h)
	{
		hashsize = h.hashsize;
		buf = (BKE_HashNode **)malloc(sizeof(void*)* hashsize);
		memset(buf, 0, sizeof(void*)* hashsize);
		for (int i = 0; i < hashsize; i++)
		{
			if (h.buf[i] == NULL)
				continue;
			BKE_HashNode *node;
			BKE_HashNode *n2 = h.buf[i];
			do
			{
				node = buf[i];
				buf[i] = al.op_new(*n2);
				buf[i]->index = i;
				buf[i]->last = NULL;
				buf[i]->next = node;
				if (node)
					node->last = buf[i];
				n2 = n2->next;
			} while (n2 != NULL);
		}
		count = h.count;
	}

	template<class T>
	BKE_hashmap(const BKE_hashmap<_Key_t, _Val_t, hashlevel> &h, const T &func)
	{
		hashsize = h.hashsize;
		buf = (BKE_HashNode **)malloc(sizeof(void*)* hashsize);
		memset(buf, 0, sizeof(void*)* hashsize);
		for (int i = 0; i < hashsize; i++)
		{
			if (h.buf[i] == NULL)
				continue;
			BKE_HashNode *node;
			BKE_HashNode *n2 = h.buf[i];
			do
			{
				node = buf[i];
				buf[i] = al.op_new(n2->key);
				buf[i]->val = func(n2->val);
				buf[i]->index = i;
				buf[i]->last = NULL;
				buf[i]->next = node;
				if (node)
					node->last = buf[i];
				n2 = n2->next;
			} while (n2 != NULL);
		}
		count = h.count;
	}

	BKE_hashmap(BKE_hashmap<_Key_t, _Val_t, hashlevel> &&h)
	{
		hashsize = h.hashsize;
		buf = h.buf;
		h.buf = NULL;
		count = h.count;
	}

	BKE_hashmap& operator = (BKE_hashmap<_Key_t, _Val_t, hashlevel> &&h)
	{
		hashsize = h.hashsize;
		free(buf);
		buf = h.buf;
		h.buf = NULL;
		count = h.count;
		return *this;
	}

	BKE_hashmap& operator = (const BKE_hashmap<_Key_t, _Val_t, hashlevel> &h)
	{
		clear();
		Union(h);
		return *this;
	}

	inline bool empty() const { return count == 0; }

	void Union(const BKE_hashmap<_Key_t, _Val_t, hashlevel> &h, bool Override = false)
	{
		if (h.empty())
			return;
		for (int i = 0; i < hashsize; i++)
		{
			if (h.buf[i] == NULL)
				continue;
			BKE_HashNode *nodebase = buf[i];
			BKE_HashNode *n2 = h.buf[i];
			do
			{
				BKE_HashNode *node = nodebase;
				bool find = false;
				while (node)
				{
					if (node->key == n2->key)
					{
						find = true;
						if (Override)
							node->val = n2->val;
						break;
					}
					node = node->next;
				}
				if (!find)
				{
					BKE_HashNode *n = buf[i];
					buf[i] = al.op_new(*n2);
					buf[i]->index = i;
					buf[i]->last = NULL;
					buf[i]->next = n;
					if (n)
						n->last = buf[i];
					count++;
				}
				n2 = n2->next;
			} while (n2 != NULL);
		}
	}

	void resizeTableSize(int newsize)
	{
		assert(newsize >= 4);
		hashsize = 1L << newsize;
		auto buf2 = (BKE_HashNode **)malloc(sizeof(void*)* hashsize);
		bkplong idx;
		for (int i = 0; i < hashsize; i++)
		{
			while (buf[i] != NULL)
			{
				auto tmp = buf[i];
				buf[i] = buf[i]->next;
				idx = tmp->hashvalue % hashsize;
				tmp->next = buf2[idx];
				if (buf2[idx])
					buf2[idx]->last = tmp;
				tmp->last = NULL;
				buf2[idx] = tmp;
				tmp->index = idx;
			}
		}
		free(buf);
		buf = buf2;
	}
};

//dummy class used in GlobalStringMap, to make the map actually a set
struct _dummy_class
{
};

template <class Key_t, int hashlevel = HASH_LEVEL>
class BKE_hashset : public BKE_hashmap < Key_t, _dummy_class, hashlevel >
{
};
