#pragma once

#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4018)
#pragma warning(disable:4244)
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>
#include <limits.h>
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
#include <iterator>
#include <atomic>
#include <memory>
#include <errno.h>
#include <list>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>

#if defined(_WIN32) && !defined(WIN32)
	#define WIN32
#endif

//comment this line if you have your own file system handle class
#define ENABLE_FILE
#define HAS_REGEX

#if BKE_CREATOR
#define PARSER_MULTITHREAD 1
#else
#define PARSER_MULTITHREAD 0
#endif

//single-thread memorypool
//#define INNER_ST
//multi-thread memorypool
#define INNER_MT
//#define INNER_MT_ST
//standard malloc function
//#define STANDARD

#ifndef WIN32
#include <sys/time.h>
#endif

#define EPS 0.00000001

#define MAX_POOL_STRING_LEN 20

#ifndef FORCEINLINE
#if _MSC_VER > 1200
#define FORCEINLINE __forceinline
#elif (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
#define FORCEINLINE inline __attribute__((always_inline))
#else
#define FORCEINLINE inline
#endif
#endif

#ifndef FASTCALL
#if defined(_M_IX86) || defined(__i386__)
#if _MSC_VER > 1200
#define FASTCALL __fastcall
#elif ((defined(__GNUC__) && __GNUC__ >= 4) || defined(__CLANG__))
#define FASTCALL __attribute__((fastcall))
#else
#error we need a way to define __fastcall function
#endif
#else
#define FASTCALL
#endif
#endif

#if !defined(_MSC_VER) || _MSC_VER > 1800
#define W(x) u##x
#define W2(x) u###x
#elif WCHAR_MAX == 0xFFFF
#define W(x) (char16_t*)L##x
#define W2(x) (char16_t*)L###x
#else
#define W(x) UniToUTF16(L##x)
#define W2(x) UniToUTF16(L###x)
#endif

typedef std::u16string StringVal;
typedef char16_t BKE_Char;

#ifndef PARSER_DEBUG
#define PARSER_DEBUG 0
#endif

//#if PARSER_DEBUG
//#define Bagel_Compiler Bagel_DebugCompiler
//#else
#define Bagel_Compiler Bagel_ReleaseCompiler
//#endif

#if PARSER_MULTITHREAD
#define Bagel_atomic atomic
#else
template <class T> class Bagel_atomic
{
private:
	T inner;
public:
	Bagel_atomic()
	{
	}

	Bagel_atomic(const T& t) :inner(t)
	{
	}

	T load(std::memory_order unused = std::memory_order_seq_cst)
	{
		return inner;
	}

	void store(const T& t, std::memory_order unused = std::memory_order_seq_cst)
	{
		inner = t;
	}

	bool is_lock_free() const
	{
		return true;
	}

	T operator = (const T& t)
	{
		inner = t;
		return inner;
	}

	operator T ()
	{
		return inner;
	}

	T exchange(const T& t, std::memory_order unused = std::memory_order_seq_cst)
	{
		auto raw = inner;
		inner = t;
		return raw;
	}

	bool compare_exchange_weak(T& expected, const T& desired, std::memory_order unused = std::memory_order_seq_cst)
	{
		if (inner == expected)
		{
			inner = desired;
			return true;
		}
		else
		{
			expected = inner;
			return false;
		}
	}
};
#endif

void* FASTCALL BagelMalloc(size_t size);
void FASTCALL BagelFree(void *p);
#if defined(INNER_ST)
#define BKE_Allocator BKE_allocator
#elif defined(INNER_MT)
#define BKE_Allocator BKE_PoolAllocator
#else
template<class T>
class BKE_Allocator : public std::allocator<T>
{
public:
	T* op_new()
	{
		auto p = allocate();
		new ((void*)p) T();
		return p;
	}

	T* allocate(size_t count = 1)
	{
		return (T*)typename std::allocator<T>::allocate(count);
	}

	void deallocate(T* p, size_t count = 1)
	{
		typename std::allocator<T>::deallocate(p, count);
	}

	template <class ...args>
	T* op_new(args&&... arg)
	{
		auto p = allocate();
		new ((void*)p) T(std::forward<args>(arg)...);
		return p;
	}

	void op_delete(T *p)
	{
		destroy(p);
		deallocate(p);
	}
};
#endif
