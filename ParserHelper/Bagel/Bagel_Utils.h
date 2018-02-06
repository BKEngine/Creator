#ifndef UTILS
#define UTILS

#include "Bagel_Config.h"
#include "BKE_hash.hpp"

inline bool bkpIsSpace(wchar_t ch)
{
	for (const wchar_t *a = L"\x0009\x000A\x000B\x000C\x000D\x0020\x0085\x00A0\x1680\x180E\x2002\x2003\x2004\x2005\x2006\x2008\x2009\x200A\x200B\x200C\x200D\x205F\x3000"; *a; a++)
	{
		if (ch == *a)
			return 1;
	}
	return 0;
}

using namespace std;

template <typename T> inline T bkeAdvance(const T &t, int diff)
{
	T tt = t;
	std::advance(tt, diff);
	return tt;
};

int32_t bkpColor2Int(const u16string &src);
int32_t bkpStr2Int(const u16string &src);
int32_t bkpColor2Int(const wstring &src);
int32_t bkpStr2Int(const wstring &src);
wstring bkpInt2WStr(int32_t v);
u16string bkpInt2Str(int32_t v);
wstring bkpUInt2WStr(uint32_t v);
u16string bkpUInt2Str(uint32_t v);
wstring bkpNumber2WStr(double v);
u16string bkpNumber2Str(double v);
wstring space(int32_t count, wchar_t ch = L' ');
inline int u16len(const char16_t *str)
{
	int len = 0;
	while (*str++)
		++len;
	return len;
}
inline bool isZero(const int32_t &c)
{
	return !c;
};
inline bool isInt(double k)
{
	if (k > 0)
	{
		return fabs(k - (int64_t)(k + 0.5)) < EPS;
	}
	else
	{
		return fabs(k - (int64_t)(k - 0.5)) < EPS;
	}
};
inline int64_t toInt(double k)
{
	if (k > 0)
	{
		return (int64_t)(k + 0.5);
	}
	else
	{
		return (int64_t)(k - 0.5);
	}
};

inline StringVal getLengthStr(const StringVal &str, int32_t dstpos)
{
	StringVal s;
	for (int32_t i = 0; i < dstpos - 1; i++)
	{
		if (i >= (int32_t)str.size())
			return s;
		if (str[i] == '\t')
			s += '\t';
		else if (str[i] >= 32 && str[i] < 256)
			s += ' ';
		else if (str[i] >= 256)
			s += W("  ");
	}
	return s;
}

int64_t bkpwcstoxl(const wchar_t *nptr, const wchar_t **endptr, int ibase, int flags = 0);
int64_t bkpwcstoxl(const BKE_Char *nptr, const BKE_Char **endptr, int ibase, int flags = 0);

wstring bkpInt2HexStr(int32_t v);
wstring bkpStrToLower(const wstring &s);
double str2num(const wstring &src);
double bkpwcstonum(const wchar_t *nptr, const wchar_t **endptr);
inline bool isZero(const double &c)
{
	return fabs(c) < EPS;
}
inline bool isZero(const float &c)
{
	return fabs(c) < EPS;
}

#define BAGEL_SERIALIZE_SIG	0xBAEBECDE

void defaultLog(const StringVal &str);

bool Bagel_ReadFile(StringVal &result, const StringVal &filename, int32_t pos = 0);

bool Bagel_ReadBinary(StringVal &result, int *isStream, const StringVal &filename, int32_t pos = 0);

bool Bagel_WriteFile(const StringVal &res, const StringVal &filename, int32_t pos = 0, bool compress = false);

bool Bagel_WriteBinary(void *data, int size, const StringVal &filename, int32_t pos = 0, bool compress = false);

/// <summary>
/// Bagel_Var 类型的枚举。
/// </summary>
enum VarType
{
	VAR_UNKNOWN = -1,
	VAR_NONE,
	VAR_NUM,
	VAR_STR,
	VAR_ARRAY,
	VAR_DIC,
	VAR_FUNC,
	VAR_PROP,
	VAR_CLO,
	VAR_CLASS,
	VAR_CLASSDEF,
	VAR_POINTER,
	VAR_BYTREE_P,
	VAR_FUNCCODE_P,
	VAR_BYTECODE_P,
	VAR_STACK_P,
	VAR_THREAD_P,
	VAR_VM_P,
	VAR_VECTOR_P,
	VAR_RUNCLO_P,
	VAR_TYPE_MAX,
};

//to avoid ambiguity of NULL
#ifdef NULL
#undef NULL
#define NULL nullptr
#endif

#define WSTR2(x) W(x)
#define WSTR2NUM(x) W2(x)
#define W_FILE WSTR2(__FILE__)
#if defined(WIN32) && 0
#define W_FUNCTION WSTR2(__FUNCTION__)
#else
#include "bkutf8.h"
#define W_FUNCTION UTF16FromUTF7(__FUNCTION__)
#endif
#define W_LINE WSTR2NUM(__LINE__)

#if PARSER_DEBUG
#define BAGEL_EXCEPT_EXT , W_FILE, W_FUNCTION, W_LINE
#else
#define BAGEL_EXCEPT_EXT
#endif

//some suppliments

double getutime();

int bkpRandomInt();
int32_t bkpRandomInt(int min, int max);

double bkpRandomDouble(double min, double max);

#define BKE_UNUSED_PARAM(_a) (void)_a

class GlobalStringMap;
class Bagel_Closure;
class GC_Manager;

enum Parser_Descr : int
{
	Variable,
	Closure,
	Function,
	FunctionCode,
	VarProp,
	VarClass,
	Descr_Count
};

class Bagel_VM;
class Bagel_Parser;
class Bagel_String;
class Bagel_Class;
class Bagel_ClassDef;
class GlobalStringMap;
typedef bool(*BKE_writeFile_func)(const StringVal &res, const StringVal &filename, int32_t pos, bool compress);
typedef bool(*BKE_writeBinary_func)(void *data, int size, const StringVal &filename, int32_t pos, bool compress);
typedef bool(*BKE_readFile_func)(StringVal &res, const StringVal &filename, int32_t pos);
typedef bool(*BKE_readBinary_func)(StringVal &res, int *isStream, const StringVal &filename, int32_t pos);
typedef void(*BKE_log_func)(const StringVal &s);

/// <summary>
/// Bagel的全局单例，持有一些全局的状态，如全局闭包，GC管理类，虚拟机，import的文件列表等等。
/// </summary>
struct GlobalStructures
{
	//用来检测GlobalStructures是否已被构造，防止因使用GLOBALSTRUCTURES_INIT()宏位置不当导致init()出错的bug
	int constructed;
	//GC_descr descr[Descr_Count];
/// <summary>
/// Global闭包。
/// </summary>
	Bagel_Closure *globalClosure;
	/// <summary>
	/// GC管理类。即_GC宏。
	/// </summary>
	GC_Manager *GC;
	/// <summary>
	/// 这个在 Bagel_VM::initVM() 之后才有效。
	/// </summary>
	Bagel_VM *VM;

	BKE_hashset<StringVal> *imports;
	GlobalStringMap *stringMap;

	BKE_writeFile_func writeFunc;
	BKE_writeBinary_func writeBinary;
	BKE_readFile_func readFunc;
	BKE_readBinary_func readBinary;
	BKE_log_func logFunc;

	list<Bagel_Parser*> parsers;
	Bagel_Parser *defaultParser;

	Bagel_ClassDef *typeclass[VAR_TYPE_MAX];

	int32_t classid;

	GlobalStructures();
	void init();
	~GlobalStructures();
};

extern GlobalStructures _globalStructures;

#define GLOBALSTRUCTURES_INIT() static struct __GlobalLoader { __GlobalLoader(){_globalStructures.init();}} __globalLoader

#endif
