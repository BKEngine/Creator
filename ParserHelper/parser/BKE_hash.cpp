#include "BKE_hash.hpp"

template<>
int32_t BKE_hash(const std::wstring &str)
{
	if(str.empty()) return 0;
	const uint32_t _FNV_offset_basis = 2166136261U;
	const uint32_t _FNV_prime = 16777619U;
	const wchar_t *c=str.c_str();
	uint32_t ret = _FNV_offset_basis;
	while(*c)
	{
		ret ^= (uint32_t)*c;
		ret *= _FNV_prime;
		c++;
	}
	return (int32_t)ret;
};

template<>
int32_t BKE_hash(const std::string &str)
{
	if(str.empty()) return 0;
	const uint32_t _FNV_offset_basis = 2166136261U;
	const uint32_t _FNV_prime = 16777619U;
	const char *c=str.c_str();
	uint32_t ret = _FNV_offset_basis;
	while(*c)
	{
		ret ^= (uint32_t)*c;
		ret *= _FNV_prime;
		c++;
	}
	return (int32_t)ret;
};

template<>
int32_t BKE_hash(const wchar_t *str)
{
	if(!*str)
		return 0;
	const uint32_t _FNV_offset_basis = 2166136261U;
	const uint32_t _FNV_prime = 16777619U;
	const wchar_t *c=str;
	uint32_t ret = _FNV_offset_basis;
	while(*c)
	{
		ret ^= (uint32_t)*c;
		ret *= _FNV_prime;
		c++;
	}
	return (int32_t)ret;
}

template<>
int32_t BKE_hash(const char *str)
{
	if(!*str)
		return 0;
	const uint32_t _FNV_offset_basis = 2166136261U;
	const uint32_t _FNV_prime = 16777619U;
	const char *c=str;
	uint32_t ret = _FNV_offset_basis;
	while(*c)
	{
		ret ^= (uint32_t)*c;
		ret *= _FNV_prime;
		c++;
	}
	return (int32_t)ret;
}