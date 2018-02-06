#include "Bagel_GC.h"
#include "Bagel_Var.h"

#undef new

void *Bagel_Object::operator new(size_t size)
{
	void* p = BagelMalloc(size);
	_GC.addNewObject(p);
	return p;
};

void GC_Manager::root2Changed(const Bagel_Var * p)
{
	if (gc_progress != GC_MARKING || !p->isObject())
		return;
	GC_Markself(p->forceAsObject());
}

void GC_Manager::GC_little(int objcount /* = 10ms*/)
{
	if (disabled)
		return;
#if PARSER_MULTITHREAD
	std::lock_guard<std::mutex> guard(gc_running);
#endif
	//int64_t begintime = clock();
	//int64_t endtime = CLOCKS_PER_SEC / 1000 * timeout + begintime;
	const int markstep = objcount;	//every time mark 1000 objects
	const int sweepstep = objcount;	//every time sweep 1000 objects
	int markcount = 0;
	int sweepcount = 0;
	switch (gc_progress)
	{
	case GC_IDLE:
		if (count < count_threshold && (stringcount < stringcount_threshold || (gc_smallcount % 2) == 1))
			return;
		//printf("enter GC when count=%d\n", count);
		//start mark
		if (stringcount >= stringcount_threshold && (gc_smallcount % 2) == 0)
		{
			//只有gc_smallcount为偶数时才启用FullGC，此时字符串表和普通对象表的marked_flag都是相同的。
			gc_type = GC_FULL;
		}
		else
		{
			gc_type = GC_SMALL;
		}
		GCrounds++;
		//step 1, check and mark roots
	#if PARSER_MULTITHREAD
		rootlock.lock();
	#endif
		{
			markcount = 0;
			auto root_it = roots.begin();
			while (root_it != roots.end())
			{
			#if PARSER_DEBUG >= 1
				if (root_it->second < 0)
				{
					fprintf(stderr, "reference < 0 at pointer %X\n", (intptr_t)root_it->first);
					++root_it;
				}
				else
			#endif
				if (root_it->second <= 0)
					root_it = roots.erase(root_it);
				else
				{
					Bagel_Object *o = (Bagel_Object*)root_it->first;
					GC_Markself(o);
					//o->marknext = tomark.exchange(o, memory_order_relaxed);
					//o->mark = GC_MARKSELF;
					//tomarkcount++;
					++root_it;
					markcount++;
				}
			}
			auto root_it2 = roots2.begin();
			while (root_it2 != roots2.end())
			{
			#if PARSER_DEBUG >= 1
				if (root_it2->second < 0)
				{
					fprintf(stderr, "reference < 0 at pointer %X\n", (intptr_t)root_it->first);
				}
				else
			#endif
				if (root_it2->second <= 0)
					root_it2 = roots2.erase(root_it2);
				else
				{
					Bagel_Var *o = (Bagel_Var*)root_it2->first;
					GC_Markself(*o);
					++root_it2;
					markcount++;
				}
			}
		}
	#if PARSER_MULTITHREAD
		rootlock.unlock();
	#endif
		gc_progress = GC_MARKING;
		tomarkhead = tomark.exchange(nullptr, memory_order_relaxed);
		//add a virtual object to marked
		markedtail = new Bagel_DummyObject();
		++count;
		markedtail->mark = marked_flag;
		markedtail->next = nullptr;
		markedtail->last = nullptr;
		marked = markedtail;
		if (gc_type == GC_FULL)
		{
			markedtail2 = new Bagel_DummyObject();
			markedtail2->vt = VAR_STR;
			++stringcount;
			markedtail2->mark = marked_flag;
			markedtail2->next = nullptr;
			markedtail2->last = nullptr;
			markedstring = markedtail2;
		}
	#if PARSER_DEBUG
		profile_markcount = profile_sweepcount = 0;
	#endif
		if (markcount > markstep)
			return;
	case GC_MARKING:
		while (++markcount <= markstep)
		{
			if (!tomarkhead)
			{
				tomarkhead = tomark.exchange(nullptr, memory_order_relaxed);
			#if PARSER_DEBUG
				tomarkcount = 0;
			#endif
				if (!tomarkhead)
					goto markover;
			}
		#if PARSER_DEBUG
			profile_markcount++;
		#endif
			tomarkhead->markChildren();
			//把tomarkhead从tosweep链表中卸下
			if(tomarkhead->next)
			{
				tomarkhead->next->last = tomarkhead->last;
			}
			if(tomarkhead->last)
			{
				tomarkhead->last->next = tomarkhead->next;
			}
			else
			{
				if(tomarkhead->vt == VAR_STR)
					stringhead.store(tomarkhead->next, memory_order_relaxed);
				else
					tosweep.store(tomarkhead->next, memory_order_relaxed);
			}
			//把tomarkhead加入marked链表首
			tomarkhead->last = nullptr;
			if (tomarkhead->vt == VAR_STR)
			{
				tomarkhead->next = markedstring;
				markedstring->last = tomarkhead;
				markedstring = tomarkhead;
			}
			else
			{
				tomarkhead->next = marked;
				marked->last = tomarkhead;
				marked = tomarkhead;
			}
			tomarkhead->mark = marked_flag;
			tomarkhead = tomarkhead->marknext;
		}
		return;
	markover:
		gc_progress = GC_SWEEPING;
		//不删最开始的，这样下面移除链表节点的时候不需要判断last是否为空
		sweephead = tosweep.load(memory_order_relaxed);
		sweephead = sweephead->next;
		if(gc_type == GC_FULL)
		{
			sweephead2 = stringhead.load(memory_order_relaxed);
			sweephead2 = sweephead2->next;
		}
		else
		{
			sweephead2 = nullptr;
		}
		return;
	case GC_SWEEPING:
		while (sweephead && ++sweepcount <= sweepstep)
		{
		#if PARSER_DEBUG
			profile_sweepcount++;
		#endif
			auto next = sweephead->next;
			if (sweephead->mark == sweep_flag)
			{
				if (!finalize_lock && sweephead->vt == VAR_CLASS)
				{
					Bagel_Class *cla = (Bagel_Class*)sweephead;
					if (!cla->finalize())
					{
						//means this object has finalized
						//so we clear it
						sweephead->last->next = next;
						if (next)
							next->last = sweephead->last;
						sweephead->~Bagel_Object();
						sweephead->operator delete(sweephead, true);
						--count;
					#if PARSER_DEBUG
						profile_deletecount++;
					#endif
					}
					else
					{
						//实际上，finalize可能会使自己重新变为活对象，从而使children也变为活对象，而这些可能已经之前就被sweep了。
						//所以只能做规定，限制finalize不能复活自己，或者保存所有class做保守处理
						sweephead->mark = marked_flag;	//means sweep_flag after sweep
					}
				}
				else
				{
					sweephead->last->next = next;
					if (next)
						next->last = sweephead->last;
					sweephead->~Bagel_Object();
					sweephead->operator delete(sweephead, true);
					--count;
				#if PARSER_DEBUG
					profile_deletecount++;
				#endif
				}
			}
			else
			{
				sweephead->mark = marked_flag;	//means sweep_flag after sweep
			}
			sweephead = next;
		}
		if(sweephead)
			return;
		while (sweephead2 && ++sweepcount <= sweepstep)
		{
		#if PARSER_DEBUG
			profile_sweepcount++;
		#endif
			auto next = sweephead2->next;
			if (sweephead2->mark == sweep_flag)
			{
				sweephead2->last->next = next;
				if (next)
					next->last = sweephead2->last;
				sweephead2->~Bagel_Object();
				sweephead2->operator delete(sweephead2, true);
				--stringcount;
			#if PARSER_DEBUG
				profile_deletecount++;
			#endif
			}
			else
			{
				sweephead2->mark = marked_flag;	//means sweep_flag after sweep
			}
			sweephead2 = next;
		}
		if (sweephead2)
			return;
		//finish gc
		tomark.store(nullptr, memory_order_relaxed);
	#if PARSER_DEBUG
		tomarkcount = 0;
	#endif
		{
			markedtail->next = tosweep.load(memory_order_relaxed);
			while (!tosweep.compare_exchange_weak(markedtail->next, marked, memory_order_relaxed));
			markedtail->next->last = markedtail;
			marked = nullptr;
			count_threshold = count < GC_INCREMENTAL ? count + GC_INCREMENTAL : count + (count >> 1);
			gc_smallcount++;
		}
		if(gc_type == GC_FULL)
		{
			markedtail2->next = tosweep.load(memory_order_relaxed);
			while (!stringhead.compare_exchange_weak(markedtail2->next, markedstring, memory_order_relaxed));
			markedtail2->next->last = markedtail2;
			markedstring = nullptr;
			stringcount_threshold = stringcount < GC_INCREMENTAL ? stringcount + GC_INCREMENTAL : stringcount + (stringcount >> 1);
			gc_smallcount = 0;
		}
		{
			auto s = sweep_flag;
			sweep_flag = marked_flag;
			marked_flag = s;
		}
		gc_progress = GC_IDLE;
		return;
	}
}

void GC_Manager::GC_Markself(const Bagel_Var & v)
{
	if (v.isObject())
		GC_Markself(v.forceAsObject());
}

void GC_Manager::GC_All()
{
	disabled = 0;
	count_threshold = 0;
	stringcount_threshold = 0;
	do
	{
		GC_little(INT_MAX);
	}
	while (gc_progress != GC_IDLE);
	//second time to flush all new objects
	count_threshold = 0;
	stringcount_threshold = 0;
	do
	{
		GC_little(INT_MAX);
	}
	while (gc_progress != GC_IDLE);
}

void GC_Manager::GC_FinalizeAll()
{
	auto node = tosweep.load(memory_order_relaxed);
	while (node)
	{
		auto node2 = node->next;
		if (node->vt == VAR_CLASS)
		{
			((Bagel_Class*)node)->finalize();
		}
		node = node2;
	}
	finalize_lock = true;
}

void GC_Manager::writeBarrier(const Bagel_Var & parent, const Bagel_Var & child)
{
	if (parent.isObject() && child.isObject())
	{
		writeBarrier(parent.forceAsObject(), child.forceAsObject());
	}
}

void GC_Manager::writeBarrierStack(Bagel_Stack * parent)
{
	if (gc_progress != GC_MARKING || !parent->isSpecialStack || parent->mark != marked_flag)
		return;
	for (auto &v : *parent)
	{
		GC_Markself(v);
	}
}

GC_Manager::GC_Manager()
{
	sweep_flag = GC_TOFREE;
	marked_flag = GC_MARKALL;
	disabled = 0;
	gc_progress = GC_IDLE;
	gc_type = GC_SMALL;
	{
		auto _o = new Bagel_DummyObject();
		_o->mark = sweep_flag;
		_o->next = _o->last = nullptr;
		tosweep.store(_o, memory_order_relaxed);
	}
	tomarkhead = nullptr;
	markedtail = nullptr;
	sweephead = nullptr;
	sweephead2 = nullptr;
	marked = nullptr;
	tomark.store(nullptr, memory_order_relaxed);
	count = 1;
	stringcount = 1;
	{
		auto _o = new Bagel_DummyObject();
		_o->vt = VAR_STR;
		_o->mark = sweep_flag;
		_o->next = _o->last = nullptr;
		stringhead.store(_o, memory_order_relaxed);
	}
	count_threshold = GC_INITSIZE;
	stringcount_threshold = GC_INITSIZE;
	gc_smallcount = 0;
	GCrounds = 0;
	markedtail2 = nullptr;
	markedstring = nullptr;
	finalize_lock = false;
	index = 0;
#if PARSER_DEBUG
	profile_deletecount = 0;
	profile_markcount = 0;
	profile_sweepcount = 0;
	tomarkcount = 0;

	purecall_addr = 0;
#endif
}
