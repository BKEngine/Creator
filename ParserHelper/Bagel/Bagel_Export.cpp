#include "Bagel_Include.h"

#define BAGEL_EXPORT_VERSION 1
#define EXPORT_TAG

BKE_hashmap<string, void*>& getFuncTable()
{
	static BKE_hashmap<string, void*> functable;
	return functable;
}

//空出最后一个位置放0
#define BAGEL_FIELD_MAX_COUNT 12

static int field_data[VAR_TYPE_MAX+1][BAGEL_FIELD_MAX_COUNT];

void initFieldData()
{
	memset(field_data, 0, sizeof(field_data));
	//VAR_NONE
	//actually store VAR_OBJECT
	field_data[VAR_NONE][0] = offsetof(Bagel_Object, index);
	field_data[VAR_NONE][1] = offsetof(Bagel_Object, mark);
	field_data[VAR_NONE][2] = offsetof(Bagel_Object, last);
	field_data[VAR_NONE][3] = offsetof(Bagel_Object, next);
	field_data[VAR_NONE][4] = offsetof(Bagel_Object, marknext);
	field_data[VAR_NONE][5] = offsetof(Bagel_Object, vt);
	//VAR_NUM
	field_data[VAR_NUM][0] = offsetof(Bagel_Number, var);
	//VAR_STR
	field_data[VAR_STR][0] = offsetof(Bagel_String, str);
	field_data[VAR_STR][1] = offsetof(Bagel_String, hash);
	field_data[VAR_STR][2] = offsetof(Bagel_String, pool);
	//VAR_ARRAY
	field_data[VAR_ARRAY][0] = offsetof(Bagel_Array, vararray);
	//VAR_DIC
	field_data[VAR_DIC][0] = offsetof(Bagel_Dic, varmap);
	//VAR_FUNC
	field_data[VAR_FUNC][0] = offsetof(Bagel_Function, func);
	field_data[VAR_FUNC][1] = offsetof(Bagel_Function, self);
	field_data[VAR_FUNC][2] = offsetof(Bagel_Function, closure);
	field_data[VAR_FUNC][3] = offsetof(Bagel_Function, default_stack);
	field_data[VAR_FUNC][4] = offsetof(Bagel_Function, name);
	field_data[VAR_FUNC][5] = offsetof(Bagel_Function, fullname);
	//VAR_PROP
	field_data[VAR_PROP][0] = offsetof(Bagel_Prop, funcget);
	field_data[VAR_PROP][1] = offsetof(Bagel_Prop, funcset);
	field_data[VAR_PROP][2] = offsetof(Bagel_Prop, closure);
	field_data[VAR_PROP][3] = offsetof(Bagel_Prop, self);
	field_data[VAR_PROP][4] = offsetof(Bagel_Prop, setparam);
	field_data[VAR_PROP][5] = offsetof(Bagel_Prop, default_stack);
	field_data[VAR_PROP][6] = offsetof(Bagel_Prop, name);
	field_data[VAR_PROP][7] = offsetof(Bagel_Prop, getname);
	field_data[VAR_PROP][8] = offsetof(Bagel_Prop, setname);
	//VAR_CLO
	field_data[VAR_CLO][0] = offsetof(Bagel_Closure, parent);
	field_data[VAR_CLO][1] = offsetof(Bagel_Closure, varmap);
	//VAR_CLASS
	field_data[VAR_CLASS][0] = offsetof(Bagel_Class, defclass);
	field_data[VAR_CLASS][1] = offsetof(Bagel_Class, native);
	field_data[VAR_CLASS][2] = offsetof(Bagel_Class, classname);
	field_data[VAR_CLASS][3] = offsetof(Bagel_Class, finalized);
	field_data[VAR_CLASS][4] = offsetof(Bagel_Class, tempvar);
	field_data[VAR_CLASS][5] = offsetof(Bagel_Class, cache);
	//VAR_CLASSDEF
	field_data[VAR_CLASSDEF][0] = offsetof(Bagel_ClassDef, innerCreateInstance);
	field_data[VAR_CLASSDEF][1] = offsetof(Bagel_ClassDef, native);
	field_data[VAR_CLASSDEF][2] = offsetof(Bagel_ClassDef, classname);
	field_data[VAR_CLASSDEF][3] = offsetof(Bagel_ClassDef, cannotcreate);
	field_data[VAR_CLASSDEF][4] = offsetof(Bagel_ClassDef, isFinal);
	field_data[VAR_CLASSDEF][5] = offsetof(Bagel_ClassDef, parents);
	field_data[VAR_CLASSDEF][6] = offsetof(Bagel_ClassDef, children);
	field_data[VAR_CLASSDEF][7] = offsetof(Bagel_ClassDef, classvar);
	field_data[VAR_CLASSDEF][8] = offsetof(Bagel_ClassDef, ownvar);
	field_data[VAR_CLASSDEF][9] = offsetof(Bagel_ClassDef, classid);
	//VAR_POINTER
	field_data[VAR_POINTER][0] = offsetof(Bagel_Pointer, var);
	field_data[VAR_POINTER][1] = offsetof(Bagel_Pointer, name);
	field_data[VAR_POINTER][2] = offsetof(Bagel_Pointer, dir);
	//VAR_BYTREE_P
	field_data[VAR_BYTREE_P][0] = offsetof(Bagel_AST, parent);
	field_data[VAR_BYTREE_P][1] = offsetof(Bagel_AST, childs);
	field_data[VAR_BYTREE_P][2] = offsetof(Bagel_AST, Node.opcode);
	field_data[VAR_BYTREE_P][3] = offsetof(Bagel_AST, Node.var);
	field_data[VAR_BYTREE_P][4] = offsetof(Bagel_AST, Node.pos);
	field_data[VAR_BYTREE_P][5] = offsetof(Bagel_AST, Node.pos2);
	//VAR_FUNCCODE_P
	field_data[VAR_FUNCCODE_P][0] = offsetof(Bagel_FunctionCode, code);
	field_data[VAR_FUNCCODE_P][1] = offsetof(Bagel_FunctionCode, bytecode);
	field_data[VAR_FUNCCODE_P][2] = offsetof(Bagel_FunctionCode, native);
	field_data[VAR_FUNCCODE_P][3] = offsetof(Bagel_FunctionCode, paramnames);
	field_data[VAR_FUNCCODE_P][4] = offsetof(Bagel_FunctionCode, initial_stack);
	field_data[VAR_FUNCCODE_P][5] = offsetof(Bagel_FunctionCode, paramarrpos);
	field_data[VAR_FUNCCODE_P][6] = offsetof(Bagel_FunctionCode, info);
	//VAR_BYTECODE_P
	field_data[VAR_BYTECODE_P][0] = offsetof(Bagel_ByteCode, code);
	field_data[VAR_BYTECODE_P][1] = offsetof(Bagel_ByteCode, consts);
	field_data[VAR_BYTECODE_P][2] = offsetof(Bagel_ByteCode, stackDepth);
	field_data[VAR_BYTECODE_P][3] = offsetof(Bagel_ByteCode, localDepth);
	field_data[VAR_BYTECODE_P][4] = offsetof(Bagel_ByteCode, release);
	field_data[VAR_BYTECODE_P][5] = offsetof(Bagel_ByteCode, needSpecialStack);
	field_data[VAR_BYTECODE_P][6] = offsetof(Bagel_ByteCode, runTimes);
	field_data[VAR_BYTECODE_P][7] = offsetof(Bagel_ByteCode, debugInfo);
	field_data[VAR_BYTECODE_P][8] = offsetof(Bagel_ByteCode, varPosInfo);
	//VAR_STACK_P
	field_data[VAR_STACK_P][0] = offsetof(Bagel_Stack, stack);
	field_data[VAR_STACK_P][1] = offsetof(Bagel_Stack, stacksize);
	field_data[VAR_STACK_P][2] = offsetof(Bagel_Stack, relativePos);
	field_data[VAR_STACK_P][3] = offsetof(Bagel_Stack, isSpecialStack);
	//VAR_THREAD_P
	//VAR_VM_P
	//VAR_VECTOR_P
	//VAR_RUNCLO_P
	//VAR_TYPE_MAX
	//actually store Bagel_Var
	field_data[VAR_TYPE_MAX][0] = offsetof(Bagel_Var, vt);
	field_data[VAR_TYPE_MAX][1] = offsetof(Bagel_Var, tag);
	field_data[VAR_TYPE_MAX][2] = offsetof(Bagel_Var, num);
}

void* Bagel_query(const char* name)
{
	return getFuncTable()[name];
}

static inline void addFunc(const char *name, void *func)
{
	assert(!getFuncTable().contains(name));
	getFuncTable()[name] = func;
}

//verify
EXPORT_TAG static bool verifyBagelPluginVersion(int MainVersion, int field_count, int type_count, int data[VAR_TYPE_MAX + 1][BAGEL_FIELD_MAX_COUNT])
{
	if (MainVersion != BAGEL_EXPORT_VERSION || field_count != BAGEL_FIELD_MAX_COUNT || type_count != VAR_TYPE_MAX)
		return false;
	for (int i = 0; i < VAR_TYPE_MAX + 1; i++)
	{
		int j = 0;
		while (field_data[i][j] != 0)
		{
			if (field_data[i][j] != data[i][j])
				return false;
			++j;
		}
		if (data[i][j] != 0)
			return false;
	}
	return true;
}

//allocate
EXPORT_TAG static void* Bagel_Malloc(size_t sz)
{
	return BagelMalloc(sz);
}

EXPORT_TAG static void Bagel_Free(void* p)
{
	BagelFree(p);
}

EXPORT_TAG static Bagel_String *allocString(const char *s)
{
	return _globalStructures.stringMap->allocateString(s);
}

EXPORT_TAG static Bagel_String *allocStringW(const char16_t *s)
{
	return _globalStructures.stringMap->allocateString(s);
}

EXPORT_TAG static Bagel_Array *allocArray()
{
	return new Bagel_Array();
}

EXPORT_TAG static Bagel_Dic *allocDic()
{
	return new Bagel_Dic();
}

EXPORT_TAG static Bagel_Function *allocFunction(Bagel_NativeFunction func)
{
	return new Bagel_Function(func);
}

EXPORT_TAG static Bagel_ClassDef *allocClassDef(const char16_t *classname, int32_t parentsnum, const char16_t **parents)
{
	vector<Bagel_StringHolder> p;
	while (parentsnum-- > 0)
	{
		p.push_back(*parents);
		parents++;
	}
	return new Bagel_ClassDef(classname, p);
}

EXPORT_TAG static Bagel_Class *allocClass(Bagel_ClassDef *parent)
{
	return new Bagel_Class(parent);
}

//VM method
EXPORT_TAG static Bagel_VM* getVMInstance()
{
	return Bagel_VM::getInstance();
}

EXPORT_TAG static void VMRun(Bagel_Var *result, Bagel_ByteCode *code, Bagel_Closure *clo = nullptr, const Bagel_Var *_this = NULL, Bagel_Stack* clostack = NULL, const char16_t *modulename = W("[main]"), Bagel_ThreadHandle *out = NULL, Bagel_ThreadHandle in = INVALID_BAGEL_HANDLE)
{
	if (result)
	{
		*result = Bagel_VM::getInstance()->Run(code, clo, _this ? *_this : Bagel_Var(), clostack, modulename, out, in);
	}
}

EXPORT_TAG static void VMRun(Bagel_Var *result, const char16_t *exp, Bagel_Closure *clo = nullptr, const Bagel_Var *_this = NULL, Bagel_Stack* clostack = NULL, const char16_t *modulename = W("[main]"), Bagel_ThreadHandle *out = NULL, Bagel_ThreadHandle in = INVALID_BAGEL_HANDLE)
{
	if (result)
	{
		*result = Bagel_VM::getInstance()->Run(exp, clo, _this ? *_this : Bagel_Var(), clostack, modulename, out, in);
	}
}

EXPORT_TAG static void VMRunFile(Bagel_Var *result, const char16_t *file, Bagel_Closure *clo = nullptr, const Bagel_Var *_this = NULL, Bagel_Stack* clostack = NULL, const char16_t *modulename = W("[main]"), Bagel_ThreadHandle *out = NULL, Bagel_ThreadHandle in = INVALID_BAGEL_HANDLE)
{
	if (result)
	{
		*result = Bagel_VM::getInstance()->RunFile(file, clo, _this ? *_this : Bagel_Var(), clostack, modulename, out, in);
	}
}

EXPORT_TAG static Bagel_Closure* VMgetCurrentGlobal()
{
	return Bagel_VM::getInstance()->getCurrentGlobal();
}

