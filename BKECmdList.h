#pragma once

#include <stdint.h>
#ifdef BKECMDLIST_EXPORTS
#ifdef WIN32
#define BKECMDLIST_API extern "C" __declspec(dllexport)
#else
#define BKECMDLIST_API extern "C" __attribute__((visibility("default")))
#endif
#else
#define BKECMDLIST_API extern "C"
#endif

enum CmdParamType : uint32_t
{
	PT_EVERYTHING = 0, // 啥都行
	PT_VOID = 1 << 0, // Void类型
	PT_INTEGER = 1 << 1, // 整数
	PT_BYTE = 1 << 2,	// 0-255
	PT_STRING = 1 << 3, // 字符串
	PT_POS = 1 << 4, // 坐标型
	PT_RECT = 1 << 5, // 范围型
	PT_BOOL = 1 << 6,	// 布尔型
	PT_VARIABLE = 1 << 7, // 变量
	PT_VOL = 1 << 8,	// 音量型 0-100
	PT_TIME = 1 << 9,	// 时间型 >=0
	PT_INTEGERARRAY = 1 << 10, // 整数数组
	PT_LABELARRAY = 1 << 11, // 标签数组
	PT_LABEL = 1 << 12, // 标签型
	PT_EXPRESSION = 1 << 13,	// 表达式 expression which has any effect, i.e., not a constant
	PT_ARRAY = 1 << 14,	// 数组
	PT_SCRIPT = 1 << 15,	// 文件名 script filename string, ended with W(".bkscr")
	PT_DICTIONARY = 1 << 16, // 字典
	PT_COLOR = 1 << 17,	// 颜色 integer
	PT_COLORARRAY = 1 << 18,	// 颜色数组 array2
	PT_STRINGARRAY = 1 << 19, // 字符串数组
	PT_NUMBER = 1 << 20, // 数型（包括小数）
	PT_EVENT = 1 << 21, // 事件型
	PT_POSARRAY = 1 << 22,

	PT_HIDDEN = 1 << 28, //对外隐藏的
	PT_DEPRECATED = 1 << 29, // 不推荐的
	PT_OPTIONAL = 1 << 30, // Optional，可以省略的参数
	PT_CONST = 1U << 31,
};

enum CmdPropertyType : uint32_t
{
	PROP_NONE,
	PROP_ATMOST_ONE,
	PROP_ONLY_ONE,
};

enum CmdEnumType : uint32_t
{
	ENUM_ANIMATE_LOOP,
};

#pragma pack(push, 4)

struct CmdVersion
{
	uint32_t from;
	uint32_t to;
	bool check(uint32_t v) { return v >= from && v < to; }
};

struct CmdProperty
{
	const char16_t *arg1;
	const char16_t *arg2;
	CmdPropertyType type;
};

struct CmdEntry
{
	const char16_t *name;
	uint32_t argCount;
	const char16_t **argNames;
	uint32_t *argFlags;
	CmdVersion *argVersions;
	const char16_t **argEnums;
	uint32_t propertyCount;
	CmdProperty *argProps;
	CmdVersion version;
	const char16_t *description;
	int priority;
};

#pragma pack(pop)

typedef void __cdecl QueryCmdListFunc(void *opaque, const CmdEntry *entry);
typedef void __cdecl QuerySpeCmdListFunc(void *opaque, const char16_t *name, uint32_t modeEnum, const CmdEntry *entry);
//typedef void __cdecl QueryEnumListFunc(void *opaque, const char16_t *name, uint32_t value);

typedef int(__cdecl *QueryVersionFunc)();
typedef void(__cdecl *PQUERYCMDLIST)(QueryCmdListFunc func, void *opaque);
typedef void(__cdecl *PQUERYSPECMDLIST)(QuerySpeCmdListFunc func, void *opaque);
//typedef void(__cdecl *PQUERYENUMLIST)(QueryEnumListFunc func, CmdEnumType type, void *opaque);

BKECMDLIST_API int __cdecl QueryPluginVersion();
BKECMDLIST_API void __cdecl QueryCmdList(QueryCmdListFunc func, void *opaque);
BKECMDLIST_API void __cdecl QuerySpeCmdList(QuerySpeCmdListFunc func, void *opaque);
//BKECMDLIST_API void __cdecl QueryEnumList(QueryEnumListFunc func, CmdEnumType type, void *opaque);

#define BKE_CMD_VERSION 1001 