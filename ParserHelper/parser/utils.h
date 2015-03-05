#ifndef UTILS
#define UTILS

//comment this line if you have your own file system handle class
//#define ENABLE_FILE
#define HAS_REGEX

#define PARSER_MULTITHREAD 0

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cmath>
#include <time.h>
#include <algorithm>
#include <iomanip>
#include <cstdio>
#include <cstdlib>
#include <clocale>
#include <cstring>
#include <cassert>

#include "defines.h"

#ifndef _WIN32
#include <sys/time.h>
#endif

#define EPS 0.00000001

inline bool bkpIsWSpace(wchar_t ch)
{
	for (const wchar_t *a = L"\x0009\x000A\x000B\x000C\x000D\x0020\x0085\x00A0\x1680\x180E\x2002\x2003\x2004\x2005\x2006\x2008\x2009\x200A\x200B\x200C\x200D\x205F\x3000"; *a; a++)
	{
		if (ch == *a)
			return 1;
	}
	return 0;
}

using namespace std;

bkplong bkpColor2Int(const wstring &src);
bkplong bkpStr2Int(const wstring &src);
wstring bkpInt2Str(bkplong v);
wstring space(bkplong count, wchar_t ch=L' ');
inline bool isZero(const bkplong &c){return !c;};
inline bool isInt(double k)
{
	if(k>0)
	{
		return fabs(k - (bkplong)(k + 0.5))<EPS;
	}
	else
	{
		return fabs(k - (bkplong)(k - 0.5))<EPS;
	}
};
inline bkplonglong toInt(double k)
{
	if(k>0)
	{
		return (bkplonglong)(k + 0.5);
	}
	else
	{
		return (bkplonglong)(k - 0.5);
	}
};

bkplong bkpwcstoxl(const wchar_t *nptr, const wchar_t **endptr, int ibase, int flags = 0);

wstring bkpInt2Str(double v);
double str2num(const wstring &src);
double bkpwcstonum(const wchar_t *nptr, const wchar_t **endptr);
inline bool isZero(const double &c)
{
	return fabs(c)<EPS;
}
inline bool isZero(const float &c)
{
	return fabs(c)<EPS;
}

bool BKE_readFile(wstring &res, const wstring &filename);

bool BKE_writeFile(const wstring &res, const wstring &filename, bkplong pos=0);

bool BKE_writeFile(const wstring &res, const string &filename, bkplong pos = 0);

enum VarType
{
	VAR_NONE,
	VAR_NUM,
	VAR_STR,
	VAR_ARRAY,
	VAR_DIC,
	VAR_FUNC,
	VAR_PROP,
	VAR_CLO,
	VAR_CLASS,
	VAR_THIS,
	VAR_BYTREE_P,
	VAR_FUNCCODE_P,
};

//to avoid ambiguity of NULL
#ifdef NULL
#undef NULL
#define NULL nullptr
#endif

#define WSTRNUM(x) L###x
#define WSTR(x) L##x
#define WSTR2(x) WSTR(x)
#define WSTR2NUM(x) WSTRNUM(x)
#define W_FILE WSTR2(__FILE__)
#define W_FUNCTION WSTR2(__FUNCTION__)
#define W_LINE WSTR2NUM(__LINE__)

#if PARSER_DEBUG
#define VAR_EXCEPT_EXT , W_FILE, W_FUNCTION, W_LINE
#else
#define VAR_EXCEPT_EXT
#endif

class Var_Except
{
private:
	std::wstring msg;
	bkplong pos;
	bkplong line;
public:
#ifdef PARSER_DEBUG
	std::wstring lineinfo;
	std::wstring functionname;
	std::wstring _file;
	std::wstring _func;
	std::wstring _line;
	Var_Except(const std::wstring &str, const std::wstring &_f, const std::wstring &_f2, const std::wstring &l) :msg(str), pos(-1), line(-1), _file(_f), _func(_f2), _line(l){};
	Var_Except(const std::wstring &str, bkplong p, const std::wstring & _f, const std::wstring & _f2, const std::wstring &l) :msg(str), pos(p), line(-1), _file(_f), _func(_f2), _line(l){};
#endif
	Var_Except(const std::wstring &str) :msg(str), pos(-1), line(-1){};
	Var_Except(const std::wstring &str, bkplong p) :msg(str), pos(p), line(-1){};
    inline std::wstring getMsg()const{ return line == -1 ? (pos == -1 ? msg : L"在" + bkpInt2Str((int)pos) + L"处：" + msg) : L"在" + bkpInt2Str((int)line) + L"行" + bkpInt2Str((int)pos) + L"处：" + msg; };
	inline std::wstring getMsgWithoutPos(){ return msg; };
	inline void setMsg(const std::wstring &str){ msg = str; };
	inline void addPos(bkplong pos){ this->pos = pos; }
	inline void removePos(){ this->pos = -1; }
	inline void addLine(bkplong line){ this->line = line; }
	inline void removeLine(){ this->line = -1; }
	inline bkplong getPos(){ return pos; }
	inline bkplong getLine(){ return line; }
	inline bool hasPos(){ return pos > -1; }
};

//some suppliments

void utf16toucs4(uint32_t *ucs4, const uint16_t* utf16, int size = -1);

void ucs4toutf16(uint16_t* utf16, const uint32_t *ucs4, int size = -1);

void bkpwcstombs(char *dst, const wchar_t *src, int srclen = -1);

void bkpmbstowcs(wchar_t *dst, const char *src, int srclen = -1);

double getutime();

void bkpRandomSeed(unsigned int seed);

int bkpRandomInt();

double bkpRandomDouble(double min, double max);

#define BKE_UNUSED_PARAM(_a) (void)_a

class GlobalStringMap;
class GlobalMemoryPool;
class BKE_VarClosure;
struct GlobalStructures
{
	GlobalStringMap *globalStringMap;
	GlobalMemoryPool *globalMemoryPool;
	BKE_VarClosure *globalClosure;
	GlobalStructures();
    void init();
};

extern GlobalStructures _globalStructures;

#define GLOBALSTRUCTURES_INIT() static struct __GlobalLoader { __GlobalLoader(){_globalStructures.init();}} __globalLoader

#endif
