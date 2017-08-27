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
	PT_EVERYTHING = 0, // ɶ����
	PT_VOID = 1 << 0, // Void����
	PT_INTEGER = 1 << 1, // ����
	PT_BYTE = 1 << 2,	// 0-255
	PT_STRING = 1 << 3, // �ַ���
	PT_POS = 1 << 4, // ������
	PT_RECT = 1 << 5, // ��Χ��
	PT_BOOL = 1 << 6,	// ������
	PT_VARIABLE = 1 << 7, // ����
	PT_VOL = 1 << 8,	// ������ 0-100
	PT_TIME = 1 << 9,	// ʱ���� >=0
	PT_INTEGERARRAY = 1 << 10, // ��������
	PT_LABELARRAY = 1 << 11, // ��ǩ����
	PT_LABEL = 1 << 12, // ��ǩ��
	PT_EXPRESSION = 1 << 13,	// ���ʽ expression which has any effect, i.e., not a constant
	PT_ARRAY = 1 << 14,	// ����
	PT_SCRIPT = 1 << 15,	// �ļ��� script filename string, ended with W(".bkscr")
	PT_DICTIONARY = 1 << 16, // �ֵ�
	PT_COLOR = 1 << 17,	// ��ɫ integer
	PT_COLORARRAY = 1 << 18,	// ��ɫ���� array2
	PT_STRINGARRAY = 1 << 19, // �ַ�������
	PT_NUMBER = 1 << 20, // ���ͣ�����С����
	PT_EVENT = 1 << 21, // �¼���
	PT_POSARRAY = 1 << 22,

	PT_HIDDEN = 1 << 28, //�������ص�
	PT_DEPRECATED = 1 << 29, // ���Ƽ���
	PT_OPTIONAL = 1 << 30, // Optional������ʡ�ԵĲ���
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