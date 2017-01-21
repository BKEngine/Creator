#pragma once

#include <weh.h>
#include <unordered_map>

#include "ScriptEnums.h"

enum bkCmdParamType
{
	ptEverything = 0,
	ptVoid = 1,
	ptInteger = 2,
	ptByte = 4,	//0-255
	ptString = 8,
	ptPos = 0x10,
	ptRect = 0x20,
	ptBool = 0x40,	//non-void
	ptVariable = 0x80,
	ptVol = 0x100,	//0-100
	ptTime = 0x200,	//>=0
	ptIntegerArray = 0x400,
	ptLabelArray = 0x800,
	ptLabel = 0x1000,
	ptExpression = 0x2000,	//expression which has any effect, i.e., not a constant
	ptArray = 0x4000,	//array
	ptScript = 0x8000,	//script filename string, ended with L".bkscr"
	ptDictionary = 0x10000,
	ptColor = 0x20000,	//integer
	ptColorArray = 0x40000,	//array2
	ptStringArray = 0x80000,
	ptNumber = 0x100000,
	ptEvent = 0x200000,
	ptDeprecated = 0x20000000,
	ptOptional = 0x40000000,
	ptConst = 0x80000000,
};

struct BKECmdInfo
{
	QString name;
	QString detail;//描述信息
	QStringList argAutoList;	//对于有限字符串取值的参数，其自动完成列表
	QStringList argNames;	//参数名称
	vector<bkpulong> argFlags;
	//almostOne之类的以后再说……
};

struct BKESpecialCmdInfo
{
	map<QString, pair<int, BKECmdInfo>> modes;
};

extern QHash<QString, BKECmdInfo> CmdList;
extern QHash<QString, BKESpecialCmdInfo> SpecialCmdList;
extern bool cmd_inited;

void initCmd();
