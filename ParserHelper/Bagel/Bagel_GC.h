#pragma once

#include "Bagel_Utils.h"

#include <list>
#include <stack>
#include <atomic>
#include <mutex>

#include "BKE_hash.hpp"

#pragma push_macro("new")
#undef new

enum GC_State
{
	GC_TOFREE,
	GC_NEWOBJECT,
	GC_MARKSELF,
	GC_MARKALL,
};

/*
new object is set to GC_NEWOBJECT,
every loop, pick up a traceble object , set itself to GC_MARKALL and mark all its children to GC_MARKSELF
mark step ends when no more GC_MARKSELF objects.
sweep step will sweep all GC_TOFREE objects and turn GC_MARKALL objects to GC_TOFREE.

when an object A is set to another object B's child (wrtie barrier):
(during mark step)
if B is an new object, do nothing
if B is GC_MARKSELF, do nothing
if B is GC_MARKALL, mark A to GC_MARKSELF
if B is GC_TOFREE, do nothing or mark A to GC_NEWOBJECT(because there must be some reference to B at current, so A will become GC_MARKSELF if B is still reachable,
and GC_TOFREE if B finally be unreachable)
(during sweep step), we simply do nothing because a GC_TOFREE object cannot be pointered from others.

so, there are three lists in GC Manager, the tomark list, the tosweep list, and the marked list.
new object is in tosweep list, but with flag GC_NEWOBJECT, so they will not be freed during next sweep step.(next sweep will set these objects to GC_TOFREE)
from root, mark every object to GC_MARKSELF and push it to tomark list.
objects which is marked GC_MARKALL is moved to marked lists.
after sweep, all marked objects will be pushed to tosweep list and be set to GC_TOFREE.

so, discuss:
if we use four-color algorithm in LuaJIT, then a write barrier will also be triggered when B is GC_TOFREE.
Total complexity:
	mark step traverse all alive objects, and sweep step release other objects, then insert marked list into tofree list.
	the write barrier makes container objects back to gray.
else, we can give a flip flag to GC Manager, means current GC_TOFREE is real GC_TOFREE or GC_MARKALL, so after sweep step , we only need to flip the flag.
	whether barrier back or barrier forward: the judge stmt is same, barrier back may be friendly to cach, but need to redo some judge.
	and because GC_MARKSELF is just a internal state, so it is no need to set a real GC_MARKSELF flag except the GC_MARKALL objects. (umm, maybe to judge is more costly)

currently, we don't make a generational GC.
*/

class Bagel_Var;
class Bagel_Stack;

#define Bagel_ObjectID int32_t
/// <summary>
/// Bagel所有类的父类，表来标志这个类是由GC管理的，<b>不要</b>手动去delete。
/// </summary>
class Bagel_Object
{
	friend class GC_Manager;
	friend void initFieldData();
protected:
	//使用index作为唯一标识而不是地址。因为了内存池的存在，两个不同Object地址相同的概率更高。
	mutable Bagel_ObjectID index;
	mutable GC_State mark;
	mutable Bagel_Object *last;
	mutable Bagel_Object *next;
	mutable Bagel_Object *marknext;

public:
	VarType vt;

	Bagel_Object(VarType t) :vt(t)
	{
	}

	virtual ~Bagel_Object()
	{
	};

	//do nothing, do NOT change last and next and so on
	Bagel_Object(const Bagel_Object&)
	{
	};

	virtual void markChildren() = 0;

	void* operator new (size_t size);

	void* operator new (size_t size, const char *, int){ return operator new(size); };

	void operator delete (void *p)
	{
		//不能手动delete
		assert(false);
	};

	void operator delete (void *p, bool)
	{
		BagelFree(p);
	};

	void operator delete (void *p, const char *, int)
	{
		return operator delete(p);
	};

	Bagel_ObjectID getID() const
	{
		return index;
	}
};

/// <summary>
/// 用来持有一个 Bagel_Object 对象的句柄。用来向GC表明这个对象被用户使用，不应被回收。
/// </summary>
template <typename T>
class Bagel_Handler
{
	static_assert(std::is_convertible<T*, Bagel_Object*>::value, "T must be child class of Bagel_Object");
protected:
	T *inner;

public:
	/// <summary>
	/// 默认的构造函数，为NULL.
	/// </summary>
	Bagel_Handler() noexcept : inner(NULL)
	{
	};

	/// <summary>
	/// 构造函数，表示持有v这个对象.
	/// </summary>
	/// <param name="v">一个 Bagel_Object 对象</param>
	Bagel_Handler(T *v) noexcept;

	/// <summary>
	/// 拷贝构造函数。
	/// </summary>
	/// <param name="h">另一个 Bagel_Handler 对象。</param>
	Bagel_Handler(const Bagel_Handler& h) noexcept : Bagel_Handler(h.inner)
	{
	};

	/// <summary>
	/// 移动构造函数。
	/// </summary>
	/// <param name="h">另一个 Bagel_Handler 对象。</param>
	Bagel_Handler(Bagel_Handler&& h) noexcept : inner(h.inner)
	{
		h.inner = nullptr;
	};

	/// <summary>
	/// 更换持有的对象。
	/// </summary>
	/// <param name="v">要持有的新 Bagel_Object 对象。</param>
	/// <returns>持有的新 Bagel_Object 对象指针。</returns>
	T* operator = (T *v) noexcept;

	/// <summary>
	/// 更换持有的对象。
	/// </summary>
	/// <param name="h">包含另一个对象的 Bagel_Handler 实例。</param>
	/// <returns>持有的新 Bagel_Object 对象指针。</returns>
	T* operator = (const Bagel_Handler& h) noexcept
	{
		return operator = (h.inner);
	}

	/// <summary>
	/// 更换持有的对象。
	/// </summary>
	/// <param name="h">包含另一个对象的 Bagel_Handler 实例。</param>
	/// <returns>持有的新 Bagel_Object 对象指针。</returns>
	T* operator = (Bagel_Handler&& h) noexcept
	{
		inner = h.inner;
		h.inner = nullptr;
		return inner;
	}

	~Bagel_Handler();

	/// <summary>
	/// 返回持有的对象指针。
	/// </summary>
	/// <returns>持有的对象指针。</returns>
	T* operator -> ()
	{
		return inner;
	}

	T* operator -> () const
	{
		return inner;
	}

	/// <summary>
	/// 返回持有的对象引用。
	/// </summary>
	/// <returns>持有的对象引用。</returns>
	T& operator *() noexcept
	{
		return *inner;
	}

	const T& operator *() const noexcept
	{
		return *inner;
	}

	/// <summary>
	/// 返回持有的对象指针。
	/// </summary>
	/// <returns>持有的对象指针。</returns>
	operator T* () const noexcept
	{
		return inner;
	}

	/// <summary>
	/// 与另一个对象比较。
	/// </summary>
	/// <param name="other">另一个对象。</param>
	/// <returns>相同返回true，不同返回false。</returns>
	bool operator == (T* other) const noexcept
	{
		return inner == other;
	}

	/// <summary>
	/// 与另一个对象比较。
	/// </summary>
	/// <param name="other">另一个对象。</param>
	/// <returns>相同返回false，不同返回true。</returns>
	bool operator != (T* other) const noexcept
	{
		return inner != other;
	}

	/// <summary>
	/// 与另一个 Bagel_Handler 对象比较。
	/// </summary>
	/// <param name="other">另一个对象。</param>
	/// <returns>相同返回true，不同返回false。</returns>
	bool operator == (const Bagel_Handler &other) const noexcept
	{
		return inner == other.inner;
	}

	/// <summary>
	/// 与另一个 Bagel_Handler 对象比较。
	/// </summary>
	/// <param name="other">另一个对象。</param>
	/// <returns>相同返回false，不同返回true。</returns>
	bool operator != (const Bagel_Handler &other) const noexcept
	{
		return inner != other.inner;
	}

	/// <summary>
	/// 布尔强转。
	/// </summary>
	/// <returns>
	/// 返回知否为一个空对象（nullptr），即什么对象也不持有。
	/// </returns>
	operator bool() const noexcept
	{
		return !!inner;
	}
};

template <class T>
inline int32_t BKE_hash(const Bagel_Handler<T> &v)
{
	return v.inner;
}

#define GC_INITSIZE 5000
#define GC_INCREMENTAL 5000
#ifdef _DEBUG
#define GC_SPEED 1000
#else
#define GC_SPEED 10000
#endif

/// <summary>
/// GC管理类，单例。
/// 使用_GC辅助宏来获取全局的实例。
/// </summary>
class GC_Manager
{
protected:
	GC_State sweep_flag; //GC_TOFREE or GC_MARKALL
	GC_State marked_flag; //GC_TOFREE or GC_MARKALL
#if PARSER_MULTITHREAD
	atomic_int disabled;
#else
	int disabled;
#endif
	enum
	{
		GC_IDLE,
		GC_MARKING,
		GC_SWEEPING
	}gc_progress;

	enum
	{
		GC_SMALL,
		GC_FULL
	}gc_type;

	int gc_smallcount;

	Bagel_Object *sweephead;	//from this object do sweep;
	Bagel_Object *sweephead2;	//from this object do string sweep;
	Bagel_Object *tomarkhead;	//from this object do mark;
	Bagel_Object *markedtail;	//last object of marked
	Bagel_Object *markedtail2;	//last object of markedstring

	Bagel_atomic<Bagel_Object*> tosweep;
	Bagel_Object *marked;

	Bagel_atomic<Bagel_Object*> stringhead;	//string part
	Bagel_Object *markedstring;

	Bagel_atomic<Bagel_Object*> tomark;

	BKE_hashmap<void *, int> roots;
	BKE_hashmap<void *, int> roots2;

#if PARSER_MULTITHREAD
	atomic_int count;
	atomic_int stringcount;
	std::mutex rootlock;
	std::mutex gc_running;
#else
	int count;
	int stringcount;
#endif
	int count_threshold;
	int stringcount_threshold;

	int GCrounds;

	Bagel_ObjectID index;

#if PARSER_DEBUG
	int profile_markcount;
	int profile_sweepcount;
	int profile_deletecount;
	int tomarkcount;
#endif

#if PARSER_DEBUG
	uintptr_t purecall_addr;
#endif

	//dummy class for markedtail
	class Bagel_DummyObject : public Bagel_Object
	{
	public:
		Bagel_DummyObject() :Bagel_Object(VAR_NONE)
		{
		};

		virtual void markChildren()
		{
		};

		void* operator new (size_t size)
		{
			return BagelMalloc(size);
		}

		void* operator new (size_t size, const char *, int){ return operator new(size); };
	
		void operator delete (void *p)
		{
			BagelFree(p);
		};

		void operator delete (void *p, const char *, int)
		{
			return operator delete(p);
		};
	};

public:
	/// <summary>
	/// 锁住GC，使得GC不会工作。
	/// </summary>
	/// @sa unlock isEnabled
	void lock()
	{
#if PARSER_MULTITHREAD
		std::lock_guard<std::mutex> guard(gc_running);
#endif
		disabled++;
	}

	/// <summary>
	/// 解锁，必须和lock成对使用。
	/// </summary>
	/// @sa lock isEnabled
	void unlock()
	{
		disabled--;
	}

	/// <summary>
	/// 查看目前GC是否被允许工作。（即是否被锁住）
	/// </summary>
	/// <returns>是否允许工作。</returns>
	/// @sa lock unlock
	bool isEnabled() const
	{
		return !disabled;
	}

	/// <summary>
	/// 获得虚拟机分配的对象数目。
	/// </summary>
	/// <returns>分配的对象数目。</returns>
	int getObjectCount() const
	{
		return count + stringcount;
	}

	void addRoot(Bagel_Object *p)
	{
	#if PARSER_MULTITHREAD
		rootlock.lock();
		roots[(void*)p]++;
		rootlock.unlock();
	#else
		roots[(void*)p]++;
	#endif
		if (gc_progress != GC_MARKING)
			return;
		GC_Markself(p);
	}

	void addRoot2(Bagel_Var *p)
	{
	#if PARSER_MULTITHREAD
		rootlock.lock();
		roots2[(void*)p]++;
		rootlock.unlock();
	#else
		roots2[(void*)p]++;
	#endif
		root2Changed(p);
	}

	void removeRoot(Bagel_Object *p)
	{
	#if PARSER_MULTITHREAD
		rootlock.lock();
		roots[(void*)p]--;
		rootlock.unlock();
	#else
		roots[(void*)p]--;
	#endif
	}

	void removeAllRoot()
	{
		roots.clear();
	}
	
	void removeRoot2(Bagel_Var *p)
	{
	#if PARSER_MULTITHREAD
		rootlock.lock();
		roots2[(void*)p]--;
		rootlock.unlock();
	#else
		roots2[(void*)p]--;
	#endif
	}

	void removeAllRoot2()
	{
		roots2.clear();
	}

	void root2Changed(const Bagel_Var *p);

	void addNewObject(void *p)
	{
		Bagel_Object *o = (Bagel_Object*)p;
		o->mark = GC_NEWOBJECT;
		o->last = nullptr;
		o->next = tosweep.exchange(o, memory_order_relaxed);
		o->next->last = o;
		o->index = index++;
		++count;
	#if PARSER_DEBUG
		purecall_addr = *(uintptr_t*)o;
	#endif
	}

	void addNewString(void *p)
	{
		Bagel_Object *o = (Bagel_Object*)p;
		o->mark = GC_NEWOBJECT;
		o->last = nullptr;
		o->next = stringhead.exchange(o, memory_order_relaxed);
		o->next->last = o;
		o->index = index++;
		++stringcount;
	}

	/// <summary>
	/// 进行增量GC。GC_SPEED宏的值大约为1ms可以进行的步数。
	/// </summary>
	/// <param name="objcount">一次要进行的步数，默认值为大约10ms的工作量。</param>
	void GC_little(int objcount = 10 * GC_SPEED /*ms*/);

	void GC_Markself(const Bagel_Object* const o)
	{
		if (!o || o->mark == marked_flag || o->mark == GC_MARKSELF)
			return;
		if (o->vt == VAR_STR && gc_type == GC_SMALL)
			return;
	#if PARSER_DEBUG
		assert(*(unsigned int*)o != purecall_addr);
	#endif
		o->marknext = tomark.exchange((Bagel_Object*)o, memory_order_relaxed);
		o->mark = GC_MARKSELF;
	#if PARSER_DEBUG
		tomarkcount++;
	#endif
	}

	void GC_Markself(const Bagel_Var &v);

	/// <summary>
	/// 回收所有可回收的对象。
	/// </summary>
	void GC_All();

	bool finalize_lock;

	//执行完此函数后不再finalize
	void GC_FinalizeAll();

	void writeBarrier(Bagel_Object *parent, Bagel_Object *child)
	{
		if (gc_progress != GC_MARKING || parent->mark != marked_flag)
			return;
		GC_Markself(child);
	};

	void writeBarrier(const Bagel_Var &parent, const Bagel_Var &child);

	void forceBarrier(Bagel_Object *child)
	{
		if (gc_progress != GC_MARKING)
			return;
		GC_Markself(child);
	};

	void writeBarrierStack(Bagel_Stack *parent);

	int getGCRounds() const
	{
		return GCrounds;
	}

	GC_Manager();

	~GC_Manager()
	{
		roots.clear();
		roots2.clear();
		clearAll();
	}

	void clearAll()
	{
		auto psweep = tosweep.load(memory_order_relaxed);
		while (psweep)
		{
			auto p = psweep;
			psweep = psweep->next;
			p->~Bagel_Object();
			p->operator delete(p, true);
		}
		tosweep.store(nullptr, memory_order_relaxed);
		psweep = stringhead.load(memory_order_relaxed);
		while (psweep)
		{
			auto p = psweep;
			psweep = psweep->next;
			p->~Bagel_Object();
			p->operator delete(p, true);
		}
		stringhead.store(nullptr, memory_order_relaxed);
	}
};

//extern GC_Manager _GC;
#define _GC (*_globalStructures.GC)

template<typename T>
inline Bagel_Handler<T>::Bagel_Handler(T *v) noexcept
{
	inner = v;
	if(inner)
		_GC.addRoot(inner);
}

template<typename T>
inline T * Bagel_Handler<T>::operator=(T * v) noexcept
{
	if (inner)
		_GC.removeRoot(inner);
	inner = v;
	if (inner)
		_GC.addRoot(inner);
	return inner;
}

template<typename T>
inline Bagel_Handler<T>::~Bagel_Handler()
{
	if(inner)
		_GC.removeRoot(inner);
}

/// <summary>
/// GC的锁，构造函数会自动调用_GC.lock，析构自动调用_GC.unlock
/// </summary>
class GC_Locker
{
public:
	GC_Locker()
	{
		_GC.lock();
	}

	~GC_Locker()
	{
		_GC.unlock();
	}
};

#pragma pop_macro("new")
