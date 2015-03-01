#pragma once

#include "utils.h"
#include "defines.h"
#include <cmath>

//#ifdef _DEBUG_MEM
//#include "debug_new.h"
//#endif

class BKE_Number
{
private:
	static const bkpulonglong nan = 0xFFF8000000000000;
	static const bkpulonglong inf = 0x7FF0000000000000;

public:
	double var;

	inline BKE_Number(bkpshort a){ var = a; };
	inline BKE_Number(bkpushort a){ var = a; };
	inline BKE_Number(float a){ var = a; };
	inline BKE_Number(double a){ var = a; };
	inline BKE_Number(bkplonglong a){ var = (double)a; };
	inline BKE_Number(bkplong a){ var = a; };
	inline BKE_Number(bkpulong a){ var = a; };
	inline BKE_Number(){ var = 0; };

	inline std::wstring tostring() const
	{
		if (isNan())
			return L"NaN";
		else if (isInf())
			return L"Inf";
		else
		{
			//return std::to_wstring(var);
#ifdef _MSC_VER
			wchar_t buf[40];
			swprintf(buf, 40, L"%.14g", var);
			return buf;
#else
			//gcc has some bugs in swprintf
			char buf[40];
			wchar_t buf2[40];
			sprintf(buf, "%.14g", var);
			char *s = buf;
			wchar_t *ss = buf2;
			while (*s)
			{
				*ss++ = *s++;
			}
			*ss = 0;
			return buf2;
#endif
		}
	}

	inline bool zero() const { return isZero(var); }

	inline bool isNan() const{ 
		return std::isnan(var);
	}

	inline bool isInf() const{ return std::isinf(var); }

	inline void setNan(){ var = *(double*)nan; }

	inline void setInf(){ var = *(double*)inf; }

	inline double asNumber() const { return var; }

	inline bkplonglong asInteger() const
	{
		//return var > 0.0 ? (long long)(var + 0.5) : (long long)(var - 0.5);
		return (bkplonglong)var;
	}

	inline bool isInteger() const { return isInt(var); }

	inline bkplonglong Round() const { return (bkplonglong)round(var); }

	inline bool operator == (const BKE_Number &v) const { return isZero(var-v.var); }

	inline bool operator <= (const BKE_Number &v) const { return var <= v.var || isZero(var - v.var); }

	inline bool operator >= (const BKE_Number &v) const { return var >= v.var || isZero(var - v.var); }

	inline bool operator < (const BKE_Number &v) const { return var < v.var; }

	inline bool operator > (const BKE_Number &v) const { return var > v.var; }

	inline BKE_Number operator + (const BKE_Number &v) const{ return var + v.var; }

	inline BKE_Number operator - (const BKE_Number &v) const{ return var - v.var; }

	inline BKE_Number operator * (const BKE_Number &v) const{ return var * v.var; }

	inline BKE_Number operator / (const BKE_Number &v) const{ return var / v.var; }

	inline BKE_Number operator % (const BKE_Number &v) const{ return fmod(var, v.var); }

	inline BKE_Number operator ^ (const BKE_Number &v) const{ return pow(var, v.var); }

	inline BKE_Number& operator += (const BKE_Number &v){ var += v.var; return *this; }

	inline BKE_Number& operator -= (const BKE_Number &v){ var -= v.var; return *this; }

	inline BKE_Number& operator *= (const BKE_Number &v){ var *= v.var; return *this; }

	inline BKE_Number& operator /= (const BKE_Number &v){ var /= v.var; return *this; }

	inline BKE_Number& operator %= (const BKE_Number &v){ var = fmod(var, v.var); return *this; }

	inline BKE_Number& operator ^= (const BKE_Number &v){ var = pow(var, v.var); return *this; }
	
	inline BKE_Number operator + (const double &v) const{ return var + v; }

	inline BKE_Number operator - (const double &v) const{ return var - v; }

	inline BKE_Number operator * (const double &v) const{ return var * v; }

	inline BKE_Number operator / (const double &v) const{ return var / v; }

	inline BKE_Number operator % (const double &v) const{ return fmod(var, v); }

	inline BKE_Number operator ^ (const double &v) const{ return pow(var, v); }

	inline BKE_Number& operator += (const double &v){ var += v; return *this; }

	inline BKE_Number& operator -= (const double &v){ var -= v; return *this; }

	inline BKE_Number& operator *= (const double &v){ var *= v; return *this; }

	inline BKE_Number& operator /= (const double &v){ var /= v; return *this; }

	inline BKE_Number& operator %= (const double &v){ var = fmod(var, v); return *this; }

	inline BKE_Number& operator ^= (const double &v){ var = pow(var, v); return *this; }

	inline operator double() const{ return var; }

};

//convert from wchar_t*
BKE_Number str2num(const wchar_t *src, wchar_t **end = NULL);
