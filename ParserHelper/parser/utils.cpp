#include "utils.h"
#include "defines.h"
#include <errno.h>
#include <limits.h>
#include <iostream>
#if BKE_CREATOR
#include "bkutf8.h"
#else
#include <bkutf8.h>
#endif
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#define FL_UNSIGNED   1       /* wcstoul called */
#define FL_NEG        2       /* negative sign found */
#define FL_OVERFLOW   4       /* overflow occured */
#define FL_READDIGIT  8       /* we've read at least one correct digit */

static int bkeWcharToDigit(wchar_t ch)
{
	return (ch>='0' && ch<='9')?ch-'0':-1;
}

static inline bool bkeIsUpper(wchar_t character){
	return character>='A' && character<='Z';
}
static inline bool bkeIsLower(wchar_t character){
	return character>='a' && character<='z';
}

static inline bool bkeIsAlpha(wchar_t character){
	return bkeIsUpper(character) || bkeIsLower(character);
}

static inline wchar_t bkeToUpper(wchar_t character){
	return bkeIsLower(character)?((character)&0xDF):character;
}

bkplong bkpwcstoxl(const wchar_t *nptr, const wchar_t **endptr, int ibase, int flags)
{
	const wchar_t *p;
	wchar_t c;
	bkpulong number;
	bkpulong digval;
	bkpulong maxval;

	/* validation section */
	if (endptr != NULL)
	{
		/* store beginning of string in endptr */
		*endptr = nptr;
	}

	p = nptr;           /* p is our scanning pointer */
	number = 0;         /* start with zero */

	c = *p++;           /* read char */

	while ( iswspace(c) )
		c = *p++;       /* skip whitespace */

	if (c == '-') {
		flags |= FL_NEG;    /* remember minus sign */
		c = *p++;
	}
	else if (c == '+')
		c = *p++;       /* skip sign */

	if (ibase == 0) {
		/* determine base free-lance, based on first two chars of
		   string */
		if (bkeWcharToDigit(c) != 0)
			ibase = 10;
		else if (*p == L'x' || *p == L'X')
			ibase = 16;
		else
			ibase = 8;
	}

	if (ibase == 16) {
		/* we might have 0x in front of number; remove if there */
		if (bkeWcharToDigit(c) == 0 && (*p == L'x' || *p == L'X')) {
			++p;
			c = *p++;   /* advance past prefix */
		}
	}

	/* if our number exceeds this, we will overflow on multiply */
	maxval = BKPULONG_MAX / ibase;


	for (;;) {  /* exit in middle of loop */

		/* convert c to value */
		if ( (digval = bkeWcharToDigit(c)) != -1 )
			;
		else if ( bkeIsAlpha(c))
			digval = bkeToUpper(c) - L'A' + 10;
		else
			break;

		if (digval >= (bkpulong)ibase)
			break;      /* exit loop if bad digit found */

		/* record the fact we have read one digit */
		flags |= FL_READDIGIT;

		/* we now need to compute number = number * base + digval,
		   but we need to know if overflow occured.  This requires
		   a tricky pre-check. */

		if (number < maxval || (number == maxval &&
		(bkpulong)digval <= BKPULONG_MAX % ibase)) {
			/* we won't overflow, go ahead and multiply */
			number = number * ibase + digval;
		}
		else {
			/* we would have overflowed -- set the overflow flag */
			flags |= FL_OVERFLOW;
			if (endptr == NULL) {
				/* no need to keep on parsing if we
				   don't have to return the endptr. */
				break;
			}
		}

		c = *p++;       /* read next digit */
	}

	--p;                /* point to place that stopped scan */

	if (!(flags & FL_READDIGIT)) {
		/* no number there; return 0 and point to beginning of
		   string */
		if (endptr)
			/* store beginning of string in endptr later on */
			p = nptr;
		number = 0L;        /* return 0 */
	}
	else if ( (flags & FL_OVERFLOW) ||
		  ( !(flags & FL_UNSIGNED) &&
			//( ( (flags & FL_NEG) && (number > -LONG_MIN) ) ||
			//  ( !(flags & FL_NEG) && (number > LONG_MAX) ) ) ) )
			(number > BKPLONG_MAX) ) )
	{
		/* overflow or signed overflow occurred */
		errno = ERANGE;
		if ( flags & FL_UNSIGNED )
			number = BKPULONG_MAX;
		else if ( flags & FL_NEG )
			number = (bkpulong)BKPLONG_MAX+1;
		else
			number = BKPULONG_MAX;
	}

	if (endptr != NULL)
		/* store pointer to char that stopped the scan */
		*endptr = p;

	if (flags & FL_NEG)
		/* negate result if there was a neg sign */
		number = (bkpulong)(-(bkplong)number);

	return number;          /* done. */
}

bkplong bkpStr2Int(const wstring &src)
{
	if(!src.compare(L"true"))return 1;
	if(!src.compare(L"false"))return 0;
	if(!src.compare(L"void"))return 0;
	if (src[0] == '-')
		return -(bkplong)bkpwcstoxl(&src[1], NULL, 0);
	if (src[0] == L'#')
		return bkpColor2Int(src);
	return bkpwcstoxl(src.c_str(), NULL, 0);
}

//src don't include #
bkplong bkpColor2Int(const wstring &src)
{
	const wchar_t *curpos = src.c_str();
	unsigned char n[8];
	int nn = 0;
	bkpulong color = 0;
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

wstring bkpInt2Str(bkplong v)
{
	char buf[16];
#ifdef _MSC_VER
	itoa(v, buf, 10);
#else
	sprintf(buf, "%d", v);
#endif
	return UniFromUTF7(buf, (bkpulong)strlen(buf));
}

void EliminateTrailingFloatZeros(char *iValue)
{
	char *p = 0;
	for (p = iValue; *p; ++p)
	{
		if ('.' == *p) {
			while (*++p);
			while ('0' == *--p) *p = '\0';
			if (*p == '.') *p = '\0';
			break;
		}
	}
}

wstring bkpInt2Str(double v)
{
	char buf[24];
	sprintf(buf, "%f", v);
	EliminateTrailingFloatZeros(buf);
	return UniFromUTF7(buf, strlen(buf));
}

wstring bkpInt2HexStr(double v)
{
	char buf[24] = "0x";
	sprintf(buf+2, "%x", (bkplong)v);
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

wstring space(bkplong count, wchar_t ch)
{
	return std::move(wstring(count, ch));
};

double str2num(const wstring &src)
{
	if(!src.compare(L"true"))return 1;
	if(!src.compare(L"false"))return 0;
	if(!src.compare(L"void"))return 0;
	if (src[0] == '-')
		return -bkpwcstonum(&src[1], NULL);
	if (src[0] == L'#')
		return bkpColor2Int(src);
	return bkpwcstonum(src.c_str(), NULL);
}

double bkpwcstonum(const wchar_t *nptr, const wchar_t **endptr)
{
	double num=0;
	wchar_t *p=(wchar_t *)nptr;
	bool hasdot=false;
	bool hasexp=false;
	bkplong len=0;
	bkplong exp=0;
	double dotvalue=0.1;
	bool expsign=true;

	//skip whitespace
	while(iswspace(*p))
		p++;
	
	//handle non-decimal base
	if(*p==L'0')
	{
		//if not "0." then base 8 or 16
		if(*(p+1)!=L'.')
			return bkpwcstoxl(p, endptr, 0);
	}

	//banned number as E9, E9 can be a name of variable
	if(towupper(*p)==L'E')
		goto ret;

	//initialize endptr
	if(endptr)
		*endptr=nptr;

	while(1)
	{
		wchar_t ch=*p++;
		if(len+exp>308 || exp<-308)
			goto ret;
		switch(ch)
		{
		case L'.':
			if(hasdot || hasexp)
				goto ret;
			else
				hasdot=true;
			break;
		case L'E':
		case L'e':
			if(hasexp)
				goto ret;
			else
			{
				hasexp=true;
				if(*p==L'-')
				{
					p++;
					expsign=false;
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
			if(hasexp)
			{
				exp*=10;
				if(expsign)
					exp+=ch-L'0';
				else
					exp-=ch-L'0';
			}
			else if(hasdot)
			{
				num+=dotvalue*(ch-L'0');
				dotvalue/=10;
			}
			else
			{
				num*=10;
				num+=ch-L'0';
				len++;
			}
			break;
		default:
			goto ret;
		}
	}
ret:
	num*=pow(10, exp);
	p--;
	if(endptr)
		*endptr=p;
	return num;
}

bool BKE_readFile(wstring &result, const wstring &filename)
{
#ifdef _MSC_VER
	ifstream fs(filename, ios_base::binary | ios_base::out);
#else
	string fname = UniToUTF8(filename);
	ifstream fs(fname, ios_base::binary | ios_base::out);
#endif
	if (!fs.good())
		return false;
	fs.seekg(0, ios_base::end);
	string::size_type s = (string::size_type)fs.tellg();
	fs.seekg(0, ios_base::beg);
	string res;
	res.resize(s);
	fs.read(&res[0], s);
	fs.close();
	if (res.size() >= 2 && (unsigned char)res[0] == 0xFF && (unsigned char)res[1] == 0xFE)
	{
		//Unicode16-LE
		result = UniFromUTF16((const unsigned short *)(&res[2]), s / 2 - 1);
	}
	else if (res.size() >= 3 && (unsigned char)res[0] == 0xEF && (unsigned char)res[1] == 0xBB && (unsigned char)res[2] == 0xBF)
	{
		// UTF8-sig
		result = UniFromUTF8((const char *)&res[3], s - 3);
	}
	else
	{
		result = UniFromUTF8((const char *)&res[0], s);
	}
	return true;
}

static bool writeFile(const wstring &res, ofstream &fs, bkplong pos)
{
	static unsigned short magic = 0xFEFF;
	//write in Unicode16-LE
	if (!fs.good())
		return false;
	if(pos == 0)
		fs.seekp(0, ios_base::beg),fs.write((const char *)&magic, 2);
	else if (pos > 0)
		fs.seekp(pos, ios_base::beg);
	else
		fs.seekp(pos + 1, ios_base::end);
	bkplong s = UniToUTF16_Size(res);
	unsigned short * ch = new unsigned short[s];
	UniToUTF16(ch, res);
	fs.write((const char *)ch, 2 * s);
	delete[] ch;
	return true;
}

bool BKE_writeFile(const wstring &res, const wstring &filename, bkplong pos)
{
#ifdef _MSC_VER
	ofstream fs(filename, ios_base::binary | ios_base::out);
#else
	string fname = UniToUTF8(filename);
	ofstream fs(fname, ios_base::binary | ios_base::out);
#endif
	return writeFile(res, fs, pos);
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

std::mt19937 random_pool((unsigned int)std::time(NULL));

bkplong bkpRandomInt()
{
	return (bkplong)random_pool();
}
double bkpRandomDouble(double min, double max)
{
	std::uniform_real_distribution<double> u(min, max);
	return u(random_pool);
}

#include "BKE_string.h"
#include "BKE_variable.h"

GlobalStructures::GlobalStructures()
{
	init();
}

void GlobalStructures::init()
{
	static GlobalStringMap m;
	static GlobalMemoryPool p;
	static BKE_VarClosure g;
	this->globalStringMap = &m;
	this->globalMemoryPool = &p;
	this->globalClosure = &g;
	g.varmap.resizeTableSize(12);
}

GlobalStructures _globalStructures;