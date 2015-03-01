#include "utils.h"
#include "defines.h"
#include <errno.h>
#include <limits.h>
#include <iostream>

#if _MSC_VER>1600
#define HAVE_CODECVT
#endif
#ifdef HAVE_CODECVT
#include <codecvt>
#endif


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
            number = LONG_MAX;
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
		color |= 0x01000000 * (*(unsigned short*)(n + 6));
		color |= 0x010000 * (*(unsigned short*)(n));
		color |= 0x0100 * (*(unsigned short*)(n + 2));
		color |= 0x01 * (*(unsigned short*)(n + 4));
		break;
	default:
		break;
	}
	return color;
}

wstring bkpInt2Str(bkplong v)
{
	/*static*/ wstringstream strstream;
	wstring str;
	strstream<<v;
	strstream>>str;
	return std::move(str);
}
wstring bkpInt2Str(double v)
{
	/*static*/ wstringstream strstream;
	wstring str;
	strstream<<v;
	strstream>>str;
	return std::move(str);
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
	char fname[FILENAME_MAX];
	bkpwcstombs(fname, filename.c_str(), FILENAME_MAX);
	ifstream fs(fname, ios_base::binary | ios_base::out);
#endif
	if (!fs.good())
		return false;
	fs.seekg(0, ios_base::end);
	string::size_type s = (string::size_type)fs.tellg();
	fs.seekg(0, ios_base::beg);
	string res;
	res.resize(s + 1);
	fs.read(&res[0], s);
	fs.close();
	if (res.size() >= 2 && (unsigned char)res[0] == 0xFF && (unsigned char)res[1] == 0xFE)
	{
		//Unicode16-LE
		result.clear();
		result.resize(s+1);
#ifdef HAVE_CODECVT
		auto& f = use_facet<codecvt<wchar_t, char16_t, mbstate_t>>(locale());
		mbstate_t mb=mbstate_t();
		const char16_t* from_next;
		wchar_t* to_next;
		f.in(mb, (char16_t*)&res[2], (char16_t*)&res[s], from_next, &result[0], &result[s], to_next);
#else
		utf16toucs4((uint32_t *)&result[0], (uint16_t *)&res[2], s / 2 - 1);
#endif
		return true;
	}
#ifndef WIN32
	mbstowcs(NULL, NULL, 0); // reset the conversion state
#endif
	if (res.size() >= 3 && (unsigned char)res[0] == 0xEF && (unsigned char)res[1] == 0xBB && (unsigned char)res[2] == 0xBF)
	{
		//UTF8-sig
		result.clear();
		result.resize(s + 1);
#ifdef WIN32
		//must use WINAPI on windows
		MultiByteToWideChar(CP_UTF8, 0, (LPCCH)(&res[3]), s - 3, &result[0], s);
#else
		setlocale(LC_ALL, "en_US.UTF-8");
		mbstowcs(&result[0], &res[3], s - 3);
#endif
		return true;
	}
	else
	{
		//use UTF8-unsig in linux and GB2312 in win
		result.resize(s + 1);
#ifdef WIN32
		setlocale(LC_ALL, "zh-CN.GB2312");
#else
		setlocale(LC_ALL, "en_US.UTF-8");
#endif
		mbstowcs(&result[0], &res[0], s);
		return true;
	}
}

static bool writeFile(const wstring &res, ofstream &fs, bkplong pos)
{
	static unsigned short magic = 0xFEFF;
	//write in Unicode16-LE
	if (!fs.good())
		return false;
	if (pos >= 0)
		fs.seekp(pos, ios_base::beg);
	else
		fs.seekp(pos + 1, ios_base::end);
	fs.write((const char *)&magic, 2);
	u16string result;
	bkplong s = res.size();
	bkplong s2 = s * sizeof(wchar_t) / sizeof(char16_t);
	result.resize(s2);
#ifdef HAVE_CODECVT
	auto& f = use_facet<codecvt<wchar_t, char16_t, mbstate_t>>(locale());
	mbstate_t mb = mbstate_t();
	char16_t* from_next;
	const wchar_t* to_next;
	f.out(mb, &res[0], &res[s], to_next, &result[0], &result[s2], from_next);
#else
	ucs4toutf16((uint16_t *)&result[0], (uint32_t *)&res[0], s);
#endif
	fs.write((const char *)result.c_str(), 2 * result.length());
	return true;
}

bool BKE_writeFile(const wstring &res, const wstring &filename, bkplong pos)
{
#ifdef _MSC_VER
	ofstream fs(filename, ios_base::binary | ios_base::out);
#else
	char fname[FILENAME_MAX];
	bkpwcstombs(fname, filename.c_str(), FILENAME_MAX);
	ofstream fs(fname, ios_base::binary | ios_base::out);
#endif
	return writeFile(res, fs, pos);
}

void utf16toucs4(uint32_t *ucs4, const uint16_t* utf16, int size)
{
	auto *in = utf16;
	auto *out = ucs4;
	while (*in && (size-- != 0))
	{
		if ((*in)<0xD800 || (*in)>0xDFFF)
			*out++ = *in++;
		else
		{
			auto a1 = *in++ - 0xD800;
			auto a2 = *in++ - 0xDC00;
			if (a1 >= 0 && a2 >= 0 && a1 < 0x400 && a2 < 0x400)
			{
				*out++ = (a1 << 10) + a2 + 0x10000;
			}
			//we meet illegal character
			else
				break;
		}
	}
	*out = 0;
}

void ucs4toutf16(uint16_t* utf16, const uint32_t *ucs4, int size)
{
	auto *in = ucs4;
	auto *out = utf16;
	while (*in && (size-- != 0))
	{
		if ((*in) < 0x10000)
			*out++ = (uint16_t)*in++;
		else
		{
			auto a = *in++ - 0x10000;
			*out++ = (uint16_t)(0xD800 + ((a & 0xFFC00) >> 10));
			*out++ = 0xDC00 + (a & 0x3FF);
		}
	}
	*out = 0;
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
	return tv_begin.tv_sec * 10000000 + tv_begin.tv_usec;
#endif
}

void bkpwcstombs(char *d, const wchar_t *src, int srclen)
{
	unsigned char *dst = (unsigned char *)d;
	for (int a = 0; (*src) && (srclen < 0 || a < srclen); a++)
	{
		wchar_t character = *src++;
		if (character < 0x80)
			*dst++ = (char)character;
		else if (character < 0x800){
			*dst++ = (character >> 6) | 0xC0;
			*dst++ = (character & 0x3F) | 0x80;
		}
#if WCHAR_MAX == 0xFFFFFFFF
		else if (character < 0x10000){
			*dst++ = (character >> 12) | 0xE0;
			*dst++ = ((character >> 6) & 0x3F) | 0x80;
			*dst++ = (character & 0x3F) | 0x80;
		}
		else
		{
			*dst++ = (character >> 18) | 0xF0;
			*dst++ = ((character >> 12) & 0x3F) | 0x80;
			*dst++ = ((character >> 6) & 0x3F) | 0x80;
			*dst++ = (character & 0x3F) | 0x80;
		}
#endif
	}
	*dst = 0;
}

void bkpmbstowcs(wchar_t *dst, const char *s, int srclen)
{
	const unsigned char *src = (const unsigned char *)s;
	int status = 0;
	uint32_t wc = 0;
	for (int a = 0; (*src) && (srclen < 0 || a < srclen); a++)
	{
		if (!status)
		{
			if (src[a] < 128)
				*dst++ = (wchar_t)*src++;
			else if ((src[a] & 0xe0) == 0xc0)
			{
				status = 1;
				wc = src[a] & 0x1f;
			}
			else if ((src[a] & 0xf0) == 0xe0)
			{
				status = 2;
				wc = src[a] & 0x0f;
			}
			else if ((src[a] & 0xf8) == 0xf0)
			{
				status = 3;
				wc = src[a] & 0x07;
			}
			else if ((src[a] & 0xfc) == 0xf8)
			{
				status = 4;
				wc = src[a] & 0x03;
			}
			else if ((src[a] & 0xfe) == 0xfc)
			{
				status = 5;
				wc = src[a] & 0x01;
			}
		}
		else
		{
			if ((src[a] & 0xc0) == 0x80)
			{
				status--;
				wc <<= 6;
				wc |= (src[a] & 0x3f);
				if (!status)
				{
					*dst++ = (wchar_t)wc;
				}
			}
			else
				break;
		}
	}
	*dst = 0;
}

static bkpulong s_seed = (bkpulong)(time(NULL) * 45568);

void bkpRandomSeed(bkpulong seed)
{
	if (!seed) s_seed = time(NULL) * 45568;
	else s_seed = seed;
}
bkplong bkpRandomInt()
{
//	if (!s_seed) s_seed = time(NULL) * 45568;
	s_seed = 214013 * s_seed + 2531011;
	return (s_seed ^ (s_seed >> 15));
}
double bkpRandomDouble(double min, double max)
{
//	if (!s_seed) s_seed = time(NULL) * 45568;
//	s_seed = 214013 * s_seed + 2531011;
//	return min + (s_seed >> 16)*(1.0f / 65535.0f)*(max - min);
	return min + ((bkpulong)bkpRandomInt()) / (double)BKPULONG_MAX * (max - min);
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
}

GlobalStructures _globalStructures;