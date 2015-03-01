#pragma once

#include <stdint.h>
#include <limits.h>

typedef char bkpchar;
typedef wchar_t bkpwchar;
typedef unsigned char bkpuchar;
typedef int32_t bkplong;
typedef uint32_t bkpulong;
typedef int16_t bkpshort;
typedef uint16_t bkpushort;
typedef int64_t bkplonglong;
typedef uint64_t bkpulonglong;

#define BKPULONG_MAX ((0x7fffffffUL) * 2UL + 1UL)
#define BKPLONG_MAX 0x7fffffffL
#define BKPLONG_MIN (-BKPLONG_MAX-1L)