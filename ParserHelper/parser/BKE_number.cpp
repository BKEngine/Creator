#include "BKE_number.h"

BKE_Number str2num(const wchar_t *src, wchar_t **end)
{
	BKE_Number var = 0;
	if (!src || !*src)
		return var;
	int sign = 1;
	switch (*src)
	{
	case '-':sign = -1;
	case '+':src++;
	}
	int dig = 10;
	if (*src == '0' && (*(src + 1) == 'x' || *(src + 1) == 'X'))
	{
		dig = 16;
		src += 2;
	}
	if (dig == 10)
	{
		while (iswdigit(*src))
		{
			var *= 10;
			var += *src - '0';
			src++;
		}
		if (*src != '.' && *src != 'e' && *src != 'E')
			goto endend;
		if (*src == '.')
		{
			src++;
			BKE_Number p(0.1);
			while (iswdigit(*src))
			{
				BKE_Number x(*src - '0');
				var += x*p;
				p /= 10;
				src++;
			}
		}
		if (*src != 'e' && *src != 'E')
			goto endend;
		src++;
		int psign = 1;
		switch (*src)
		{
		case '-':psign = -1;
		case '+':src++;
		}
		BKE_Number pp(0);
		while (iswdigit(*src))
		{
			pp *= 10;
			pp += *src - '0';
			src++;
		}
		pp *= psign;
		var *= BKE_Number(10) ^ pp;
	}
	else
	{
		while (iswdigit(*src) || ('a' <= *src && *src <= 'f') || ('A' <= *src && *src <= 'F'))
		{
			var *= 16;
			switch (*src)
			{
			case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':var += *src - '0'; break;
			case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':var += *src - 'a' + 10; break;
			case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':var += *src - 'A' + 10; break;
			}
			src++;
		}
		if (*src != '.' && *src != 'p' && *src != 'P')
			goto endend;
		if (*src == '.')
		{
			src++;
			BKE_Number p(1.0/16);
			while (iswdigit(*src) || ('a' <= *src && *src <= 'f') || ('A' <= *src && *src <= 'A'))
			{
				BKE_Number x;
				switch (*src)
				{
				case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':x = *src - '0'; break;
				case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':x = *src - 'a' + 10; break;
				case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':x = *src - 'A' + 10; break;
				}
				var += x*p;
				p /= 16;
				src++;
			}
		}
		if (*src != 'p' && *src != 'P')
			goto endend;
		src++;
		int psign = 1;
		switch (*src)
		{
		case '-':psign = -1;
		case '+':src++;
		}
		BKE_Number pp(0);
		while (iswdigit(*src))
		{
			pp *= 10;
			pp += *src - '0';
			src++;
		}
		pp *= psign;
		var *= BKE_Number(2) ^ pp;
	}
endend:
	if (end)
		*end = (wchar_t *)src;
	return var * BKE_Number(sign);
}
