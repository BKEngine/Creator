#include "Bagel_Utils.h"
#include "bkutf8.h"

#include "memorypool.h"
#include "poolmalloc.h"

#if defined(INNER_ST)
void* FASTCALL BagelMalloc(size_t size)
{
	void *p;
	if (size <= MEMORY_UNIT * SMALL)
		p = get_allocator(size)->allocate();
	else
		throw std::runtime_error("too large size for memory pool");
	if (!p)
		throw std::bad_alloc();
	return p;
}
void FASTCALL BagelFree(void *p)
{
	allocator_array()[1]->deallocate(p);
}
#elif defined(INNER_MT)
void *FASTCALL BagelMalloc(size_t size)
{
	void *p;
	if (size + sizeof(PoolUnitTail) <= MAXELE)
		p = pool_malloc(size);
	else
		throw std::exception("too large size for memory pool");
	return p;
}
void FASTCALL BagelFree(void *p)
{
	pool_free(p);
}
#else
void *FASTCALL BagelMalloc(size_t size)
{
	return malloc(size);
}
void FASTCALL BagelFree(void *p)
{
	free(p);
}
#endif


#ifdef _WIN32
#include <Windows.h>
#endif

#include "Bagel_Var.h"
#include "Bagel_Vcode.h"
#include "Bagel_VM.h"

#define FL_UNSIGNED   1       /* wcstoul called */
#define FL_NEG        2       /* negative sign found */
#define FL_OVERFLOW   4       /* overflow occured */
#define FL_READDIGIT  8       /* we've read at least one correct digit */

static int bkeWcharToDigit(wchar_t ch)
{
	return (ch >= '0' && ch <= '9') ? ch - '0' : -1;
}

static inline bool bkeIsUpper(wchar_t character)
{
	return character >= 'A' && character <= 'Z';
}
static inline bool bkeIsLower(wchar_t character)
{
	return character >= 'a' && character <= 'z';
}

static inline bool bkeIsAlpha(wchar_t character)
{
	return bkeIsUpper(character) || bkeIsLower(character);
}

static inline wchar_t bkeToUpper(wchar_t character)
{
	return bkeIsLower(character) ? ((character)& 0xDF) : character;
}

int64_t bkpwcstoxl(const wchar_t *nptr, const wchar_t **endptr, int ibase, int flags)
{
	const wchar_t *p;
	wchar_t c;
	int64_t number;
	int64_t digval;
	int64_t maxval;

	/* validation section */
	if (endptr != NULL)
	{
		/* store beginning of string in endptr */
		*endptr = nptr;
	}

	p = nptr;           /* p is our scanning pointer */
	number = 0;         /* start with zero */

	c = *p++;           /* read char */

	while (iswspace(c))
		c = *p++;       /* skip whitespace */

	if (c == '-')
	{
		flags |= FL_NEG;    /* remember minus sign */
		c = *p++;
	}
	else if (c == '+')
		c = *p++;       /* skip sign */

	if (ibase == 0)
	{
		/* determine base free-lance, based on first two chars of
		   string */
		if (bkeWcharToDigit(c) != 0)
			ibase = 10;
		else if (*p == L'x' || *p == L'X')
			ibase = 16;
		else
			ibase = 8;
	}

	if (ibase == 16)
	{
		/* we might have 0x in front of number; remove if there */
		if (bkeWcharToDigit(c) == 0 && (*p == L'x' || *p == L'X'))
		{
			++p;
			c = *p++;   /* advance past prefix */
		}
	}

	/* if our number exceeds this, we will overflow on multiply */
	maxval = INT64_MAX / ibase;


	for (;;)
	{  /* exit in middle of loop */

/* convert c to value */
		if ((digval = bkeWcharToDigit(c)) != -1)
			;
		else if (bkeIsAlpha(c))
			digval = bkeToUpper(c) - L'A' + 10;
		else
			break;

		if (digval >= ibase)
			break;      /* exit loop if bad digit found */

		/* record the fact we have read one digit */
		flags |= FL_READDIGIT;

		/* we now need to compute number = number * base + digval,
		   but we need to know if overflow occured.  This requires
		   a tricky pre-check. */

		if (number < maxval || (number == maxval &&
								digval <= INT64_MAX % ibase))
		{
			/* we won't overflow, go ahead and multiply */
			number = number * ibase + digval;
		}
		else
		{
			/* we would have overflowed -- set the overflow flag */
			flags |= FL_OVERFLOW;
			if (endptr == NULL)
			{
				/* no need to keep on parsing if we
				   don't have to return the endptr. */
				break;
			}
		}

		c = *p++;       /* read next digit */
	}

	--p;                /* point to place that stopped scan */

	if (!(flags & FL_READDIGIT))
	{
		/* no number there; return 0 and point to beginning of
		   string */
		if (endptr)
			/* store beginning of string in endptr later on */
			p = nptr;
		number = 0L;        /* return 0 */
	}
	else if ((flags & FL_OVERFLOW) ||
			 (!(flags & FL_UNSIGNED)
			  // && ( ( (flags & FL_NEG) && (number > -LONG_MIN) ) ||
			  //  ( !(flags & FL_NEG) && (number > LONG_MAX) ) ) ) )
			 // (number > INT64_MAX)
			  ))
	{
		/* overflow or signed overflow occurred */
		errno = ERANGE;
		if (flags & FL_UNSIGNED)
			number = INT64_MAX;
		else if (flags & FL_NEG)
			number = INT64_MIN;
		else
			number = INT64_MAX;
	}

	if (endptr != NULL)
		/* store pointer to char that stopped the scan */
		*endptr = p;

	if (flags & FL_NEG)
		/* negate result if there was a neg sign */
		number = -number;

	return number;          /* done. */
}

int64_t bkpwcstoxl(const BKE_Char *nptr, const BKE_Char **endptr, int ibase, int flags)
{
	const BKE_Char *p;
	wchar_t c;
	int64_t number;
	int64_t digval;
	int64_t maxval;

	/* validation section */
	if (endptr != NULL)
	{
		/* store beginning of string in endptr */
		*endptr = nptr;
	}

	p = nptr;           /* p is our scanning pointer */
	number = 0;         /* start with zero */

	c = *p++;           /* read char */

	while (iswspace(c))
		c = *p++;       /* skip whitespace */

	if (c == '-')
	{
		flags |= FL_NEG;    /* remember minus sign */
		c = *p++;
	}
	else if (c == '+')
		c = *p++;       /* skip sign */

	if (ibase == 0)
	{
		/* determine base free-lance, based on first two chars of
		string */
		if (bkeWcharToDigit(c) != 0)
			ibase = 10;
		else if (*p == L'x' || *p == L'X')
			ibase = 16;
		else
			ibase = 8;
	}

	if (ibase == 16)
	{
		/* we might have 0x in front of number; remove if there */
		if (bkeWcharToDigit(c) == 0 && (*p == L'x' || *p == L'X'))
		{
			++p;
			c = *p++;   /* advance past prefix */
		}
	}

	/* if our number exceeds this, we will overflow on multiply */
	maxval = INT64_MAX / ibase;


	for (;;)
	{  /* exit in middle of loop */

	   /* convert c to value */
		if ((digval = bkeWcharToDigit(c)) != -1)
			;
		else if (bkeIsAlpha(c))
			digval = bkeToUpper(c) - L'A' + 10;
		else
			break;

		if (digval >= (uint32_t)ibase)
			break;      /* exit loop if bad digit found */

						/* record the fact we have read one digit */
		flags |= FL_READDIGIT;

		/* we now need to compute number = number * base + digval,
		but we need to know if overflow occured.  This requires
		a tricky pre-check. */

		if (number < maxval || (number == maxval &&
								digval <= INT64_MAX % ibase))
		{
			/* we won't overflow, go ahead and multiply */
			number = number * ibase + digval;
		}
		else
		{
			/* we would have overflowed -- set the overflow flag */
			flags |= FL_OVERFLOW;
			if (endptr == NULL)
			{
				/* no need to keep on parsing if we
				don't have to return the endptr. */
				break;
			}
		}

		c = *p++;       /* read next digit */
	}

	--p;                /* point to place that stopped scan */

	if (!(flags & FL_READDIGIT))
	{
		/* no number there; return 0 and point to beginning of
		string */
		if (endptr)
			/* store beginning of string in endptr later on */
			p = nptr;
		number = 0L;        /* return 0 */
	}
	else if ((flags & FL_OVERFLOW)
			 //|| (!(flags & FL_UNSIGNED)
			  // && ( ( (flags & FL_NEG) && (number > -LONG_MIN) ) ||
			  //  ( !(flags & FL_NEG) && (number > LONG_MAX) ) ) ) )
			  // (number > INT64_MAX))
			  )
	{
		/* overflow or signed overflow occurred */
		errno = ERANGE;
		//if (flags & FL_UNSIGNED)
		//	number = INT64_MAX;
		//else 
		if (flags & FL_NEG)
			number = INT64_MIN;
		else
			number = INT64_MAX;
	}

	if (endptr != NULL)
		/* store pointer to char that stopped the scan */
		*endptr = p;

	if (flags & FL_NEG)
		/* negate result if there was a neg sign */
		number = -number;

	return number;          /* done. */
}

int32_t bkpStr2Int(const wstring &src)
{
	if (!src.compare(L"true"))return 1;
	if (!src.compare(L"false"))return 0;
	if (!src.compare(L"void"))return 0;
	if (src[0] == '-')
		return -(int32_t)bkpwcstoxl(&src[1], NULL, 0);
	if (src[0] == L'#')
		return bkpColor2Int(src);
	return bkpwcstoxl(src.c_str(), NULL, 0);
}

int32_t bkpStr2Int(const u16string &src)
{
	if (!src.compare(W("true")))return 1;
	if (!src.compare(W("false")))return 0;
	if (!src.compare(W("void")))return 0;
	if (src[0] == '-')
		return -(int32_t)bkpwcstoxl(&src[1], NULL, 0);
	if (src[0] == L'#')
		return bkpColor2Int(src);
	return bkpwcstoxl(src.c_str(), NULL, 0);
}

//src don't include #
int32_t bkpColor2Int(const wstring &src)
{
	const wchar_t *curpos = src.c_str();
	unsigned char n[8];
	int nn = 0;
	uint32_t color = 0;
	wchar_t ch = *(++curpos);
	while (1)
	{
		if (L'0' <= ch && ch <= L'9')
			n[nn++] = ch - L'0';
		else if (L'a' <= ch && ch <= L'f')
			n[nn++] = ch - L'a' + 10;
		else if (L'A' <= ch && ch <= L'F')
			n[nn++] = ch - L'A' + 10;
		else
			break;
		ch = *(++curpos);
	}
	switch (nn)
	{
	case 3:
		n[3] = 15;
	case 4:
		color = 0x11000000 * n[3];
		color |= 0x110000 * n[0];
		color |= 0x1100 * n[1];
		color |= 0x11 * n[2];
		break;
	case 6:
		(*(unsigned short*)(n + 6)) = 0xFF;
	case 8:
		color |= ((int)(*(unsigned short*)(n + 6))) << 24;
		color |= ((int)(*(unsigned short*)(n))) << 16;
		color |= ((int)(*(unsigned short*)(n + 2))) << 8;
		color |= ((int)(*(unsigned short*)(n + 4)));
		break;
	default:
		break;
	}
	return color;
}

int32_t bkpColor2Int(const u16string &src)
{
	const BKE_Char *curpos = src.c_str();
	unsigned char n[8];
	int nn = 0;
	uint32_t color = 0;
	wchar_t ch = *(++curpos);
	while (1)
	{
		if (L'0' <= ch && ch <= L'9')
			n[nn++] = ch - L'0';
		else if (L'a' <= ch && ch <= L'f')
			n[nn++] = ch - L'a' + 10;
		else if (L'A' <= ch && ch <= L'F')
			n[nn++] = ch - L'A' + 10;
		else
			break;
		ch = *(++curpos);
	}
	switch (nn)
	{
	case 3:
		n[3] = 15;
	case 4:
		color = 0x11000000 * n[3];
		color |= 0x110000 * n[0];
		color |= 0x1100 * n[1];
		color |= 0x11 * n[2];
		break;
	case 6:
		(*(unsigned short*)(n + 6)) = 0xFF;
	case 8:
		color |= ((int)(*(unsigned short*)(n + 6))) << 24;
		color |= ((int)(*(unsigned short*)(n))) << 16;
		color |= ((int)(*(unsigned short*)(n + 2))) << 8;
		color |= ((int)(*(unsigned short*)(n + 4)));
		break;
	default:
		break;
	}
	return color;
}

wstring bkpInt2WStr(int32_t v)
{
	char buf[16];
	sprintf(buf, "%d", v);
	return UniFromUTF7(buf, (uint32_t)strlen(buf));
}

u16string bkpInt2Str(int32_t v)
{
	char buf[16];
	sprintf(buf, "%d", v);
	return UTF16FromUTF7(buf, (uint32_t)strlen(buf));
}

wstring bkpUInt2WStr(uint32_t v)
{
	char buf[16];
	sprintf(buf, "%u", v);
	return UniFromUTF7(buf, (uint32_t)strlen(buf));
}

u16string bkpUInt2Str(uint32_t v)
{
	char buf[16];
	sprintf(buf, "%u", v);
	return UTF16FromUTF7(buf, (uint32_t)strlen(buf));
}

void EliminateTrailingFloatZeros(char *iValue)
{
	char *p = 0;
	for (p = iValue; *p; ++p)
	{
		if ('.' == *p)
		{
			while (*++p);
			while ('0' == *--p) *p = '\0';
			if (*p == '.') *p = '\0';
			break;
		}
	}
}

wstring bkpNumber2WStr(double v)
{
	char buf[24];
	sprintf(buf, "%f", v);
	EliminateTrailingFloatZeros(buf);
	return UniFromUTF7(buf, strlen(buf));
}

u16string bkpNumber2Str(double v)
{
	char buf[24];
	sprintf(buf, "%f", v);
	EliminateTrailingFloatZeros(buf);
	return UTF16FromUTF7(buf, strlen(buf));
}

std::wstring bkpInt2HexStr(int32_t v)
{
	char buf[24] = "0x";
	sprintf(buf + 2, "%x", v);
	return UniFromUTF7(buf, strlen(buf));
}

wstring bkpStrToLower(const wstring &s)
{
	wstring c;
	c.resize(s.size());
	for (size_t i = 0; i < s.size(); i++)
	{
		c[i] = towlower(s[i]);
	}
	return c;
}

wstring space(int32_t count, wchar_t ch)
{
	return wstring(count, ch);
};

double str2num(const wstring &src)
{
	if (!src.compare(L"true"))return 1;
	if (!src.compare(L"false"))return 0;
	if (!src.compare(L"void"))return 0;
	if (src[0] == '-')
		return -bkpwcstonum(&src[1], NULL);
	if (src[0] == L'#')
		return bkpColor2Int(src);
	return bkpwcstonum(src.c_str(), NULL);
}

double bkpwcstonum(const wchar_t *nptr, const wchar_t **endptr)
{
	double num = 0;
	wchar_t *p = (wchar_t *)nptr;
	bool hasdot = false;
	bool hasexp = false;
	int32_t len = 0;
	int32_t exp = 0;
	double dotvalue = 0.1;
	bool expsign = true;

	//skip whitespace
	while (iswspace(*p))
		p++;

	//handle non-decimal base
	if (*p == L'0')
	{
		//if not "0." then base 8 or 16
		if (*(p + 1) != L'.')
			return bkpwcstoxl(p, endptr, 0);
	}

	//banned number as E9, E9 can be a name of variable
	if (towupper(*p) == L'E')
		goto ret;

	//initialize endptr
	if (endptr)
		*endptr = nptr;

	while (1)
	{
		wchar_t ch = *p++;
		if (len + exp > 308 || exp < -308)
			goto ret;
		switch (ch)
		{
		case L'.':
			if (hasdot || hasexp)
				goto ret;
			else
				hasdot = true;
			break;
		case L'E':
		case L'e':
			if (hasexp)
				goto ret;
			else
			{
				hasexp = true;
				if (*p == L'-')
				{
					p++;
					expsign = false;
				}
			}
			break;
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			//readnum
			//exp->dot->common
			if (hasexp)
			{
				exp *= 10;
				if (expsign)
					exp += ch - L'0';
				else
					exp -= ch - L'0';
			}
			else if (hasdot)
			{
				num += dotvalue*(ch - L'0');
				dotvalue /= 10;
			}
			else
			{
				num *= 10;
				num += ch - L'0';
				len++;
			}
			break;
		default:
			goto ret;
		}
	}
ret:
	num *= pow(10, exp);
	p--;
	if (endptr)
		*endptr = p;
	return num;
}

void defaultLog(const StringVal & str)
{
#if WCHAR_MAX==0xFFFF
	wcout << (wchar_t *)str.c_str();
#else
	wcout << UniFromUTF16(str);
#endif
}

bool Bagel_ReadFile(StringVal &result, const StringVal &filename, int32_t pos)
{
#ifdef _MSC_VER
	ifstream fs((wchar_t*)filename.c_str(), ios_base::binary | ios_base::in);
#else
	string fname = UTF16ToUTF8(filename);
	ifstream fs(fname, ios_base::binary | ios_base::in);
#endif
	if (!fs.good())
		return false;
	fs.seekg(0, ios_base::end);
	string::size_type s = (string::size_type)fs.tellg();
	if (pos >= 0)
		fs.seekg(pos, ios_base::beg);
	else if (pos < 0)
		fs.seekg(pos + 1, ios_base::end);
	string res;
	res.resize(s);
	fs.read(&res[0], s);
	fs.close();
	if (res.size() >= 2 && (unsigned char)res[0] == 0xFF && (unsigned char)res[1] == 0xFE)
	{
		//Unicode16-LE
		result = (char16_t*)&res[2];
	}
	else if (res.size() >= 3 && (unsigned char)res[0] == 0xEF && (unsigned char)res[1] == 0xBB && (unsigned char)res[2] == 0xBF)
	{
		// UTF8-sig
		result = UTF16FromUTF8((const char *)&res[3], s - 3);
	}
	else
	{
		result = UTF16FromUTF8((const char *)&res[0], s);
	}
	return true;
}

bool Bagel_ReadBinary(StringVal &result, int * isStream, const StringVal & filename, int32_t pos)
{
	*isStream = 0;
#ifdef _MSC_VER
	ifstream fs((wchar_t*)filename.c_str(), ios_base::binary | ios_base::in);
#else
	string fname = UTF16ToUTF8(filename);
	ifstream fs(fname, ios_base::binary | ios_base::in);
#endif
	if (!fs.good())
		return false;
	fs.seekg(0, ios_base::end);
	string::size_type s = (string::size_type)fs.tellg();
	if (pos >= 0)
		fs.seekg(pos, ios_base::beg);
	else if (pos < 0)
		fs.seekg(pos + 1, ios_base::end);
	string res;
	res.resize(s);
	fs.read(&res[0], s);
	fs.close();
	if (res.size() > 4 && *(uint32_t*)&res[0] == BAGEL_SERIALIZE_SIG)
	{
		*isStream = 1;
		result.resize(res.size() / 2);
		memcpy(&result[0], res.data(), res.size());
		return true;
	}
	*isStream = 0;
	if (res.size() >= 2 && (unsigned char)res[0] == 0xFF && (unsigned char)res[1] == 0xFE)
	{
		//Unicode16-LE
		result = (char16_t*)&res[2];
	}
	else if (res.size() >= 3 && (unsigned char)res[0] == 0xEF && (unsigned char)res[1] == 0xBB && (unsigned char)res[2] == 0xBF)
	{
		// UTF8-sig
		result = UTF16FromUTF8((const char *)&res[3], s - 3);
	}
	else
	{
		result = UTF16FromUTF8((const char *)&res[0], s);
	}
	return true;
}

bool Bagel_WriteFile(const StringVal &res, const StringVal &filename, int32_t pos, bool compress)
{
	(void)compress;
#ifdef _MSC_VER
	ofstream fs((wchar_t*)filename.c_str(), pos != 0 ? ios_base::binary | ios_base::out : ios_base::binary | ios_base::out | ios_base::ate);
#else
	string fname = UTF16ToUTF8(filename);
	ofstream fs(fname, pos != 0 ? ios_base::binary | ios_base::out : ios_base::binary | ios_base::out | ios_base::ate);
#endif
	const unsigned short magic = 0xFEFF;
	//write in Unicode16-LE
	if (!fs.good())
		return false;
	if (pos > 0)
		fs.seekp(pos, ios_base::beg);
	if (pos < 0)
		fs.seekp(pos, ios_base::end);
	fs.write((const char *)&magic, 2);
	fs.write((const char *)res.c_str(), 2 * res.size());
	return true;
}

bool Bagel_WriteBinary(void * data, int size, const StringVal & filename, int32_t pos, bool compress)
{
	(void)compress;
#ifdef _MSC_VER
	ofstream fs((wchar_t*)filename.c_str(), pos != 0 ? ios_base::binary | ios_base::out : ios_base::binary | ios_base::out | ios_base::ate);
#else
	string fname = UTF16ToUTF8(filename);
	ofstream fs(fname, pos != 0 ? ios_base::binary | ios_base::out : ios_base::binary | ios_base::out | ios_base::ate);
#endif
	if (!fs.good())
		return false;
	if (pos > 0)
		fs.seekp(pos, ios_base::beg);
	if (pos < 0)
		fs.seekp(pos, ios_base::end);
	fs.write((const char*)data, size);
	return true;
}

double getutime()
{
#ifdef _WIN32
	LARGE_INTEGER liTime, liFreq;
	QueryPerformanceFrequency(&liFreq);
	QueryPerformanceCounter(&liTime);
	return liTime.QuadPart * 1000000.0 / liFreq.QuadPart;
#else
	struct timeval tv_begin;
	gettimeofday(&tv_begin, NULL);
	return tv_begin.tv_sec * 1000000 + tv_begin.tv_usec;
#endif
}

#include <random>

std::mt19937 &get_random_pool(){
    static std::mt19937 random_pool((unsigned int)std::time(NULL));
    return random_pool;
}

int32_t bkpRandomInt()
{
	return (int32_t)get_random_pool()();
}
//[min, max)
int32_t bkpRandomInt(int min, int max)
{
	std::uniform_int_distribution<int32_t> u(min, max - 1);
	return u(get_random_pool());
}
double bkpRandomDouble(double min, double max)
{
	std::uniform_real_distribution<double> u(min, max);
	return u(get_random_pool());
}
float bkpRandomFloat(float min, float max)
{
	std::uniform_real_distribution<float> u(min, max);
	return u(get_random_pool());
}

#include "Bagel_String.h"
#include "Bagel_Var.h"
#include "Bagel_Parser.h"

class Bagel_VM;
namespace ParserUtils
{
	void registerExtend(Bagel_VM *p);
}

GlobalStructures::GlobalStructures()
{
	constructed = true;
	init();
}

void GlobalStructures::init()
{
	static bool inited = false;
	if (!constructed)
	{
		//init先于构造函数执行，意味着GLOBALSTRUCTURES_INIT()宏位置不对
		assert(false);
		return;
	}
	if (!inited)
	{
	#if defined(INNER_ST)
		__init_memorypool();
	#elif defined(INNER_MT)
		__init_memorypool();
		__init_memorypool_thread();
	#endif
	}
	//static GC_Manager gc;
	if (!inited)
	{
		//this->GC = &gc;
		this->GC = new GC_Manager();
		this->globalClosure = new Bagel_Closure();
		this->globalClosure->varmap.resizeTableSize(1<<12);
		//this->VM = new Bagel_VM();
		this->VM = NULL;
		this->imports = new BKE_hashset<StringVal>();
		this->stringMap = new GlobalStringMap();

		defaultParser = new Bagel_Parser();

	#ifdef ENABLE_FILE
		readFunc = Bagel_ReadFile;
		writeFunc = Bagel_WriteFile;
		readBinary = Bagel_ReadBinary;
		writeBinary = Bagel_WriteBinary;
	#endif
		logFunc = defaultLog;

		this->GC->addRoot(globalClosure);
		this->GC->addRoot(stringMap->nullString);
		//this->GC->addRoot(VM);

		classid = 0;

		inited = true;
	}
}

GlobalStructures::~GlobalStructures()
{
	delete imports;
	for (auto it = parsers.begin(); it != parsers.end();)
	{
		Bagel_Parser *p = *it;
		it = parsers.erase(it);
		delete p;
	}
	GC->removeAllRoot();
	GC->removeAllRoot2();
	GC->GC_FinalizeAll();//finalize all
	globalClosure = NULL;
	VM = NULL;
	GC->GC_All();
	delete GC;
	delete stringMap;
#if defined(INNER_ST) || defined(INNER_MT)
	__uninit_memorypool();
#endif
}

GlobalStructures _globalStructures;
