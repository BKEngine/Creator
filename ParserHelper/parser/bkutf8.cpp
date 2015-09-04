#include "bkutf8.h"
#include <wchar.h>
#include <memory>

typedef unsigned char bkuchar;
typedef unsigned int bkulong;

#ifndef BOM8A
#define BOM8A ((bkuchar)0xEF)
#define BOM8B ((bkuchar)0xBB)
#define BOM8C ((bkuchar)0xBF)
#endif

int ConvertSingleUTF8Character(wchar_t &dst, const bkuchar *src, bkulong size, bkulong &bytes_used)
{
	bytes_used = 0;
	bkuchar byte = src[bytes_used++];
	bkulong c = 0;
	if (!(byte & 0x80))
		c = byte;
	else if ((byte & 0xC0) == 0x80){
		bytes_used = 1;
		return 0;
	}
	else if ((byte & 0xE0) == 0xC0){
		if (size < 2){
			bytes_used = size;
			return 0;
		}
		c = byte & 0x1F;
		c <<= 6;
		c |= src[bytes_used++] & 0x3F;
	}
	else if ((byte & 0xF0) == 0xE0){
		if (size < 3){
			bytes_used = size;
			return 0;
		}
		c = byte & 0x0F;
		c <<= 6;
		c |= src[bytes_used++] & 0x3F;
		c <<= 6;
		c |= src[bytes_used++] & 0x3F;
	}
	else if ((byte & 0xF8) == 0xF0){
		if (size < 4){
			bytes_used = size;
			return 0;
		}
		c = byte & 0x07;
		c <<= 6;
		c |= src[bytes_used++] & 0x3F;
		c <<= 6;
		c |= src[bytes_used++] & 0x3F;
		c <<= 6;
		c |= src[bytes_used++] & 0x3F;
	}
	//ignore char beyond 0x10FFFF
#if WCHAR_MAX==0xFFFF
	if(c<0x10000)
	{
		dst=(wchar_t)c;
		return 1;
	}
	else
	{
		c -= 0x10000;
		dst = (wchar_t)(0xD800 + ((c & 0xFFC00) >> 10));
		*(&dst + 1) = (wchar_t)(0xDC00 + (c & 0x3FF));
		return 2;
	}
#else
	dst = (wchar_t)c;
	return 1;
#endif
}


void ConvertSingleCharacterToUTF8(char dst[6], wchar_t src, bkulong &bytes_used){
	bytes_used = 0;
	if (src < 0x80)
		dst[bytes_used++] = (char)src;
	else if (src < 0x800){
		dst[bytes_used++] = (src >> 6) | 0xC0;
		dst[bytes_used++] = (src & 0x3F) | 0x80;
#if WCHAR_MAX==0xFFFF
	}
	else{
#else
	}
	else if (src < 0x10000){
#endif
		dst[bytes_used++] = (src >> 12) | 0xE0;
		dst[bytes_used++] = ((src >> 6) & 0x3F) | 0x80;
		dst[bytes_used++] = (src & 0x3F) | 0x80;
#if WCHAR_MAX==0xFFFF
	}
#else
}
	else{
		dst[bytes_used++] = (src >> 18) | 0xF0;
		dst[bytes_used++] = ((src >> 12) & 0x3F) | 0x80;
		dst[bytes_used++] = ((src >> 6) & 0x3F) | 0x80;
		dst[bytes_used++] = (src & 0x3F) | 0x80;
	}
#endif
}

bool UTF8_WC(wchar_t *dst, const bkuchar *src, bkulong srclen){
	bkulong advance;
	for (bkulong a = 0; a < srclen; a += advance)
	{
		if (!ConvertSingleUTF8Character(*dst++, src + a, srclen - a, advance))
			return false;
	}
	return true;
}


bkulong UniToUTF8_Size(const wchar_t *buffer, bkulong size){
	bkulong res = 0;
	for (bkulong a = 0; a < size; a++){
		if (buffer[a] < 0x80){
			res++;
		}
		else if (buffer[a] < 0x800)
			res += 2;
#if WCHAR_MAX==0xFFFF
		else
#else
		else if (buffer[a] < 0x10000)
#endif
			res += 3;
#if WCHAR_MAX!=0xFFFF
		else
			res += 4;
#endif
	}
	return res;
}

void WC_UTF8(bkuchar *dst, const bkulong *src, bkulong srcl){
	for (bkulong a = 0; a < srcl; a++){
		bkulong character = *src++;
		if (character < 0x80)
			*dst++ = (bkuchar)character;
		else if (character < 0x800){
			*dst++ = (character >> 6) | 0xC0;
			*dst++ = (character & 0x3F) | 0x80;
		}
		else if (character < 0x10000){
			*dst++ = (character >> 12) | 0xE0;
			*dst++ = ((character >> 6) & 0x3F) | 0x80;
			*dst++ = (character & 0x3F) | 0x80;
		}
		else{
			*dst++ = (character >> 18) | 0xF0;
			*dst++ = ((character >> 12) & 0x3F) | 0x80;
			*dst++ = ((character >> 6) & 0x3F) | 0x80;
			*dst++ = (character & 0x3F) | 0x80;
		}
	}
}

void U16_UTF8(bkuchar *dst, const unsigned short *src, bkulong srcl){
	for (bkulong a = 0; a < srcl; a++){
		bkulong character = *src++;
		if (character >= 0xD800 && character <= 0xDFFF)
		{
			bkulong c1 = character - 0xD800;
			if (a >= srcl)
				return;
			bkulong c2 = (*src++) - 0xDC00;
			character = 0x10000 + (c1 << 10) + c2;
			a++;
		}
		if (character < 0x80)
			*dst++ = (bkuchar)character;
		else if (character < 0x800){
			*dst++ = (character >> 6) | 0xC0;
			*dst++ = (character & 0x3F) | 0x80;
		}
		else if (character < 0x10000){
			*dst++ = (character >> 12) | 0xE0;
			*dst++ = ((character >> 6) & 0x3F) | 0x80;
			*dst++ = (character & 0x3F) | 0x80;
		}
		else{
			*dst++ = (character >> 18) | 0xF0;
			*dst++ = ((character >> 12) & 0x3F) | 0x80;
			*dst++ = ((character >> 6) & 0x3F) | 0x80;
			*dst++ = (character & 0x3F) | 0x80;
		}
	}
}

std::string UniToUTF8(const wchar_t *str, unsigned int len, bool addBOM)
{
	std::string res;
	res.resize(UniToUTF8_Size(str, len) + (addBOM ? 3 : 0));
	if (addBOM){
		res.push_back(BOM8A);
		res.push_back(BOM8B);
		res.push_back(BOM8C);
	}
	UniToUTF8(&res[(addBOM ? 3 : 0)], &str[0], len);
	return std::move(res);
}

void UniToUTF8(char *dst, const wchar_t *str, unsigned int len)
{
#if WCHAR_MAX==0xFFFF
	U16_UTF8((bkuchar *)dst, (unsigned short *)str, len);
#else
	WC_UTF8((bkuchar *)dst, (bkulong*)str, len);
#endif
}

unsigned int UniFromUTF8_Size(const char *str, unsigned int len)
{
    unsigned char *start = (unsigned char *)str;
    unsigned char *end = start + len;
    unsigned int size = 0;
    for (;start < end; start++)
    {
#if WCHAR_MAX == 0xFFFF
		if ((*start & 0xF0) == 0xF0)
			size += 2;
		else
#endif
        if ((*start < 128) || (*start & 192) == 192)
            size++;
    }
    return size;
}

std::wstring UniFromUTF8(const char *str, bkulong len)
{
    const char *start = str;
	if (len>=3 && (bkuchar)str[0]==BOM8A && (bkuchar)str[1]==BOM8B && (bkuchar)str[2]==BOM8C)
    {
        start += 3;
        len -= 3;
    }
    bkulong size = UniFromUTF8_Size(start, len);
	std::wstring res;
	res.resize(size);
	bool UTF8_WC(wchar_t *dst, const bkuchar *src, bkulong srcl);
	UTF8_WC(&res[0], (const unsigned char *)start, len);
	return std::move(res);
}

void UniFromUTF8(wchar_t *dst, const char *str, bkulong len)
{
	bool UTF8_WC(wchar_t *dst, const bkuchar *src, bkulong srcl);
    UTF8_WC(dst, (const bkuchar *)str, len);
}

unsigned int UniFromUTF16_Size(const unsigned short *str, unsigned int len)
{
#if WCHAR_MAX==0xFFFF
    (void)str;
    return len;
#else
    const unsigned short *start = str;
    const unsigned short *end = str + len;
    unsigned int size = 0;
    while (start < end)
    {
        if ((*start)<0xD800 || (*start)>0xDFFF)
        {
            size++;
            start++;
        }
        else
        {
            auto a1 = *start++ - 0xD800;
            auto a2 = *start++ - 0xDC00;
            if (a1 >= 0 && a2 >= 0 && a1 < 0x400 && a2 < 0x400)
            {
                size++;
            }
            //we meet illegal character
            else
                break;
        }
    }
    return size;
#endif
}

std::wstring UniFromUTF16(const unsigned short *str, unsigned int len)
{
    std::wstring res;
    if (!len) {
        return res;
    }
    res.resize(UniFromUTF16_Size(str, len));
    UniFromUTF16(&res[0], str, len);
	return std::move(res);
}

bool UniFromUTF16(wchar_t *dst, const unsigned short *str, unsigned int len)
{
#if WCHAR_MAX == 0xFFFF
    memcpy(dst, str, len * 2);
	return true;
#else
    const unsigned short *start = str;
    const unsigned short *end = str + len;
    while (start < end)
    {
        if ((*start)<0xD800 || (*start)>0xDFFF)
        {
            *dst++ = *start++;
        }
        else
        {
            auto a1 = *start++ - 0xD800;
			if(start >= end)
				return false;
            auto a2 = *start++ - 0xDC00;
            if (a1 >= 0 && a2 >= 0 && a1 < 0x400 && a2 < 0x400)
            {
                *dst++ = (a1 << 10) + a2 + 0x10000;
            }
            //we meet illegal character
            else
                return false;
        }
    }
	return true;
#endif
}

unsigned int UniToUTF16_Size(const wchar_t *str, unsigned int len)
{
#if WCHAR_MAX == 0xFFFF
    (void)str;
    return len;
#else
    const wchar_t *start = str;
    const wchar_t *end = str + len;
    unsigned int size = 0;
    while (start < end)
    {
        if ((*start) < 0x10000)
            size++, start++;
        else if(*start < 0xF0000)
        {
            size += 2;
        }
        else
            break;
    }
    return size;
#endif
}

void UniToUTF16(unsigned short *dst, const wchar_t *str, unsigned int len)
{
#if WCHAR_MAX == 0xFFFF
    memcpy(dst, str, len * 2);
#else
    const wchar_t *start = str;
    const wchar_t *end = str + len;
    while (start < end)
    {
        if ((*start) < 0x10000)
            *dst++ = (unsigned short)*start++;
        else if(*start < 0xF0000)
        {
            auto a = *start++ - 0x10000;
            *dst++ = (unsigned short)(0xD800 + ((a & 0xFFC00) >> 10));
            *dst++ = (unsigned short)(0xDC00 + (a & 0x3FF));
        }
        else
            break;
    }
#endif
}

bool IsValidUTF8(const char *str, unsigned int len)
{
	const bkuchar *unsigned_buffer = (const bkuchar *)str;
	for (bkulong a = 0; a < len; a++){
		bkulong char_len;
		if (!(*unsigned_buffer & 128))
			char_len = 1;
		else if ((*unsigned_buffer & 224) == 192)
			char_len = 2;
		else if ((*unsigned_buffer & 240) == 224)
			char_len = 3;
		else if ((*unsigned_buffer & 248) == 240)
			char_len = 4;
		else
			return 0;
		unsigned_buffer++;
		if (char_len < 2)
			continue;
		a++;
		for (bkulong b = 1; b < char_len; b++, a++, unsigned_buffer++)
			if (*unsigned_buffer < 0x80 || (*unsigned_buffer & 0xC0) != 0x80)
				return 0;
	}
	return 1;
}


