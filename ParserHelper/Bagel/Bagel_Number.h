#pragma once

#include "Bagel_Utils.h"
#include <cmath>

//#ifdef _DEBUG_MEM
//#include "debug_new.h"
//#endif


//Utils function for number
class Bagel_Number
{
private:
	static const uint64_t nan = 0xFFF8000000000000;
	static const uint64_t inf = 0x7FF0000000000000;

public:
	double var;

	inline Bagel_Number(int16_t a){ var = a; };
	inline Bagel_Number(uint16_t a){ var = a; };
	inline Bagel_Number(float a){ var = a; };
	inline Bagel_Number(double a){ var = a; };
	inline Bagel_Number(int64_t a){ var = (double)a; };
	inline Bagel_Number(int32_t a){ var = a; };
	inline Bagel_Number(uint32_t a){ var = a; };
	inline Bagel_Number(){ var = 0; };

	static inline std::wstring toWString(double var)
	{
		if (std::isnan(var))
			return L"NaN";
		else if (std::isinf(var))
			return L"Inf";
		else
			return bkpNumber2WStr(var);
	}

	static inline StringVal toString(double var)
	{
		if (std::isnan(var))
			return W("NaN");
		else if (std::isinf(var))
			return W("Inf");
		else
			return bkpNumber2Str(var);
	}

	static inline void setNan(double &var){ var = *(double*)nan; }

	static inline void setInf(double &var){ var = *(double*)inf; }

	inline double asNumber() const { return var; }

	inline int64_t asInteger() const
	{
		//return var > 0.0 ? (long long)(var + 0.5) : (long long)(var - 0.5);
		return (int64_t)var;
	}

	inline bool isInteger() const { return isInt(var); }

	inline int64_t Round() const { return (int64_t)round(var); }

	template<class T>
	static inline double str2num(const T* src, T** end = NULL)
	{
		double var = 0;
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
			while (*src <= '9' && *src >= '0')
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
				double p = 0.1;
				while (*src <= '9' && *src >= '0')
				{
					var += (*src - '0') * p;
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
			double pp = 0;
			while (*src <= '9' && *src >= '0')
			{
				pp *= 10;
				pp += *src - '0';
				src++;
			}
			pp *= psign;
			var *= pow(10, pp);
		}
		else
		{
			while ((*src <= '9' && *src >= '0') || ('a' <= *src && *src <= 'f') || ('A' <= *src && *src <= 'F'))
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
				double p = 1.0 / 16;
				while ((*src <= '9' && *src >= '0') || ('a' <= *src && *src <= 'f') || ('A' <= *src && *src <= 'A'))
				{
					int x;
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
			double  pp = 0;
			while (iswdigit(*src))
			{
				pp *= 10;
				pp += *src - '0';
				src++;
			}
			pp *= psign;
			var *= pow(2, pp);
		}
	endend:
		if (end)
			*end = (T *)src;
		return var * sign;
	}
};
