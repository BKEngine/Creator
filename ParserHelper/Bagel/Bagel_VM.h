#pragma once

#include "Bagel_Vcode.h"
#include "Bagel_GC.h"
#include "extend.h"
#include "stack_allocator.h"

typedef uintptr_t Bagel_ThreadHandle;
#define INVALID_BAGEL_HANDLE (0)
#define MAX_STACK 200

struct Bagel_RunClosure
{
	//this闭包总是作为stack的第一个成员，即stack[0]
	//Bagel_Closure *_this;
	Bagel_ByteCode *bc;
	Bagel_ByteCodeStruct *codepos;
	//int codepos;
	Bagel_StringHolder name;
	Bagel_Var *retpos;

	//vector<Bagel_Var> stack;	//用了vector的话，就必须事先reserve好，运行时不再更改。这样，事先reserve的时候可以放宽一些，之后想内联其它函数的话也不会有问题。
	Bagel_Stack default_stack;
	Bagel_Stack *runstack;
	//闭包栈
	Bagel_Stack *clostack;

	//临时self信息
	bool hasSelf;
	Bagel_Var self;
	Bagel_Closure *clo;
};

class Bagel_ThreadContext : public Bagel_Object
{
	friend class Bagel_VM;
protected:
	StackAllocator<Bagel_Var> alloc;
	StackAllocator<Bagel_RunClosure, MAX_STACK> alloc2;

public:
	vector<Bagel_RunClosure*> callstack;

	Bagel_RunClosure *runningclo;

	Bagel_Var expectInfo;

	//临时用于记录类定义的成员更改，需要保证GET_PTR指令和PTR_SET指令是连续的
	Bagel_Object *parentObj;
	Bagel_Var member;

	bool hasCache;

	enum
	{
		THREAD_FINISH,
		THREAD_SLEEP,
		THREAD_IDLE,
		THREAD_RUNNING
	}status;

	struct TRY
	{
		int codepos;
		int exceptvar;
		int callDepth;

		TRY(int a, int b, int c) :codepos(a), exceptvar(b), callDepth(c)
		{
		};
	};

	vector <TRY> trystack;	//记录遇到except后跳转到的位置，以及放except变量的位置。

	Bagel_ThreadContext() : Bagel_Object(VAR_THREAD_P), hasCache(false), status(THREAD_IDLE)
	{
	}

	virtual ~Bagel_ThreadContext()
	{
	}

	StringVal makeErrorInfo();

	virtual void markChildren() override;

	void addCallstack(Bagel_ByteCode *code, Bagel_Closure *clo, const Bagel_Var &_this = Bagel_Var(), Bagel_Stack* clostack = NULL, Bagel_StringHolder modulename = W("[main]"));

	void popCallStack();

	void clearCallStack()
	{
		alloc2.deallocate(callstack.size());
		while (!callstack.empty())
		{
			runningclo = callstack.back();
			if (!runningclo->runstack->isSpecialStack)
				alloc.deallocate(runningclo->runstack->stacksize);
			callstack.pop_back();
		}
		runningclo = NULL;
		trystack.clear();
	}

	bool handleExcept(const Bagel_Var &info, int returnlevel);

	Bagel_Var Run(Bagel_ThreadHandle *out = nullptr);

	Bagel_ByteCodeStruct *fetchLastCommand()
	{
		if (!runningclo)
			return nullptr;
		return runningclo->codepos;
	}

	Bagel_ByteCodeStruct *fetchLastCommandBasePos()
	{
		if (!runningclo)
			return nullptr;
		return runningclo->bc->code.data();
	}

	//返回从0开始的行数
	int getCurLine() const;

	bool fetchVarAtDebugPos(Bagel_StringHolder varname, Bagel_Var &outvar);
};

/*
设置断点从Bagel_ByteCode类执行，
继续执行或单步要从Bagel_ThreadContext去弄
*/
struct Bagel_BreakPoints
{
	//相应的cmd被替换成BC_DEBUG
	//如果遇到跨几个长度的指令，如PTR_SETxxx或CACHE，一组全部拷过去，最后一个放jump回去的
	unordered_map<Bagel_ByteCode*  /*对应字节码*/, map<int /*原指令index*/, vector<Bagel_ByteCodeStruct>>> rawcmd;

	//line=>bytecodeIndex
	vector<vector<pair<Bagel_ByteCode*, int>>> codemap;

	map<Bagel_ByteCode*, bool> inited;

	void initMap(Bagel_ByteCode *bc);

	void setBreakPointAtLine(int line);

	bool eraseBreakPointAtLine(int line);
};

/// <summary>
/// singleton Virtual Machine class
/// </summary>
/// <seealso cref="Bagel_Object" />
class Bagel_VM : public Bagel_Object
{
	friend class Bagel_ThreadContext;
	friend class Bagel_Serializer;
	friend class Bagel_FunctionCode;
	friend struct Bagel_BreakPoints;
protected:
	/** @cond PRIVATE
	*/
	list<Bagel_ThreadContext*, BKE_Allocator<Bagel_ThreadContext*>> cache;
	//map<Bagel_ThreadHandle, Bagel_ThreadContext*, less<Bagel_ThreadHandle>, BKE_Allocator<pair<Bagel_ThreadHandle, Bagel_ThreadContext*>>> cache;	//handle就是BagelThreadContext的地址

	Bagel_ThreadContext* lastThread;

	BKE_hashset<Bagel_String*> strRefs;

	int cycles;

	Bagel_Closure *_global;

	bool debugMode;		//是否允许命中断点

	unordered_map<u16string /*filename*/, Bagel_BreakPoints> bp;

	unordered_map<Bagel_ByteCode *, Bagel_BreakPoints*> bpcode;	//the bytecode which has debug command in it

/// <summary>
/// _执行字节码的主循环.
/// </summary>
/// <param name="btc">The Bagel_ThreadHandle.</param>
/// <param name="out">The out handle.</param>
/// <param name="returnlevel">执行到第几层就返回.</param>
/// <returns></returns>
	Bagel_Var _debugRun(Bagel_ThreadContext *btc, Bagel_ThreadHandle *out = NULL, int returnlevel = 0);

	void tempClearDebugInfo(Bagel_ByteCode *code)
	{
		auto itc = bpcode.find(code);
		if (itc == bpcode.end())
			return;
		auto &&info = itc->second->rawcmd[code];
		bool res = false;
		for (auto it = info.begin(); it != info.end();)
		{
			auto rawcodepos = &code->code[it->first];
			if (rawcodepos->opcode != Bagel_BC::BC_DEBUG)
			{
				//废弃断点
				it = info.erase(it);
			}
			else
			{
				for (int i = 0; i < it->second.size(); i++)
				{
					swap(rawcodepos[i], it->second[i]);
				}
				++it;
				res = true;
			}
		}
		if (!res)
		{
			//no breakpoint
			itc->second->rawcmd.erase(code);
			bpcode.erase(code);
		}
	}

	void tempRestoreDebugInfo(Bagel_ByteCode *code)
	{
		auto itc = bpcode.find(code);
		if (itc == bpcode.end())
			return;
		auto &&info = itc->second->rawcmd[code];
		for (auto &it : info)
		{
			auto rawcodepos = &code->code[it.first];
			if (it.second[0].opcode != Bagel_BC::BC_DEBUG)
			{
				//just ignore
				continue;
			}
			else
			{
				for (int i = 0; i < it.second.size(); i++)
				{
					swap(rawcodepos[i], it.second[i]);
				}
			}
		}
	}

	/** @endcond
	*/

public:
	Bagel_VM() : Bagel_Object(VAR_VM_P), lastThread(NULL), cycles(0), _global(_globalStructures.globalClosure), debugMode(false)
	{
	};

	virtual ~Bagel_VM()
	{
	}

	static void initVM()
	{
		if (_globalStructures.VM != nullptr)
			return;
		_globalStructures.VM = new Bagel_VM();
		_GC.addRoot(_globalStructures.VM);
		ParserUtils::registerExtend(getInstance());
	}

	static Bagel_VM* getInstance()
	{
		return _globalStructures.VM;
	}

#if DEBUG_CYCLE
	int getVMCycles()
	{
		return cycles;
	}
#endif

	void log(const StringVal &s)
	{
		_globalStructures.logFunc(s);
	}

	Bagel_Closure *getCurrentGlobal() const
	{
		return _global;
	}

	Bagel_Closure *setCurrentGlobal(Bagel_Closure *g)
	{
		assert(g);
		auto r = _global;
		_global = g;
		return r;
	}

	Bagel_ThreadContext* createNULLThreadHandle();

	Bagel_ThreadContext* createThreadHandle(Bagel_ByteCode &code, Bagel_Closure *clo = nullptr, const Bagel_Var &_this = Bagel_Var(), Bagel_Stack* clostack = NULL, Bagel_StringHolder modulename = W("[main]"));

	Bagel_Var Run(Bagel_ByteCode *code, Bagel_Closure *clo = nullptr, const Bagel_Var &_this = Bagel_Var(), Bagel_Stack* clostack = NULL, Bagel_StringHolder modulename = W("[main]"), Bagel_ThreadHandle *out = NULL, Bagel_ThreadHandle in = INVALID_BAGEL_HANDLE);

	Bagel_Var Run(const StringVal &exp, Bagel_Closure *clo = nullptr, const Bagel_Var &_this = Bagel_Var(), Bagel_Stack* clostack = NULL, Bagel_StringHolder modulename = W("[main]"), Bagel_ThreadHandle *out = NULL, Bagel_ThreadHandle in = INVALID_BAGEL_HANDLE);

	Bagel_Var RunFile(const StringVal &file, Bagel_Closure *clo = nullptr, const Bagel_Var &_this = Bagel_Var(), Bagel_Stack* clostack = NULL, Bagel_StringHolder modulename = W("[main]"), Bagel_ThreadHandle *out = NULL, Bagel_ThreadHandle in = INVALID_BAGEL_HANDLE);

	Bagel_Handler<Bagel_Pointer> getVar(const StringVal &exp, Bagel_Closure *clo = nullptr, const Bagel_Var &_this = Bagel_Var(), Bagel_Stack* clostack = NULL, Bagel_StringHolder modulename = W("[main]"), Bagel_Stack *stack = NULL, Bagel_ThreadHandle *out = NULL, Bagel_ThreadHandle in = INVALID_BAGEL_HANDLE);

	Bagel_Var resume(Bagel_ThreadHandle handle, Bagel_ThreadHandle *out = nullptr);

	Bagel_Var resumeLast(Bagel_ThreadHandle *out);

	void stop(Bagel_ThreadHandle handle);

	virtual void markChildren() override;

	inline void registerClass(const char16_t *name, Bagel_NativeClass *obj)
	{
		obj->nativeInit(name);
	}

	inline void registerClass(const u16string &name, Bagel_NativeClass *obj)
	{
		obj->nativeInit(name);
	}

	inline u16string getVersion()
	{
		return u16string(WSTR2(__DATE__)) + WSTR2(__TIME__);
	}

	Bagel_String *getStringRef(Bagel_StringHolder str)
	{
		strRefs.insertKey(str.s);
		return str.s;
	}

	void enterDebugMode()
	{
		debugMode = true;
	}

	void quitDebugMode()
	{
		debugMode = false;
	}

	bool isDebugMode() const
	{
		return debugMode;
	}

	void addDebugCodeInfo(const u16string &file, Bagel_ByteCode *code)
	{
		if (!code->debugInfo)
			return;
		bp[file].initMap(code);
		for (auto &c : code->debugInfo->innerFunc)
		{
			addDebugCodeInfo(file, c);
		}
	};

	void setBreakPoint(const u16string &file, int line)
	{
		bp[file].setBreakPointAtLine(line);
	}

	bool eraseBreakPoint(const u16string &file, int line)
	{
		return bp[file].eraseBreakPointAtLine(line);
	}

	void continueBreakPoint(Bagel_ThreadHandle handle, bool eraseThisBP);

#define IO_INTERFACE(type, name, member) type name(type newfunc){auto old = _globalStructures.member; _globalStructures.member = newfunc; return old; }

	IO_INTERFACE(BKE_log_func, setLogFunc, logFunc);

	IO_INTERFACE(BKE_readFile_func, setReadFileFunc, readFunc);

	IO_INTERFACE(BKE_readBinary_func, setReadBinaryFunc, readBinary);

	IO_INTERFACE(BKE_writeFile_func, setWriteFileFunc, writeFunc);

	IO_INTERFACE(BKE_writeBinary_func, setWriteBinaryFunc, writeBinary);

protected:
	void cacheCloMemberValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx);
	void cacheCloMemberAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx);
	void noCacheCloMemberAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx);
	void cacheClassMemberValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx);
	void cacheClassMemberAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx);
	void noCacheMemberValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_ThreadContext *ctx);
	void noCacheMemberAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_ThreadContext *ctx);
	void cacheMemberStrValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx);
	void noCacheMemberStrValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx);
	void cacheMemberStrAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx);
	void noCacheMemberStrAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx);
	void noCacheMemberIdxValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_ThreadContext *ctx);
	void noCacheMemberIdxAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_ThreadContext *ctx);
	Bagel_Var& getGlobalMemberValue(Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, bool hasCache, Bagel_ThreadContext *ctx);
	Bagel_Var& getGlobalMemberAddr(Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, bool hasCache, Bagel_ThreadContext *ctx);
};

#define REG_STR(s) Bagel_VM::getInstance()->getStringRef(W(s))
