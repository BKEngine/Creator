#include <weh.h>
#include "CmdListLoader.h"
#include <QLibrary>

QHash<QString, QBkeCmdInfo> CmdList;
QHash<QString, QHash<QString, QBkeCmdInfo>> SpecialCmdList;
//QHash<QString, uint32_t> CmdAnimateLoopModeEnumList;

static QBkeCmdInfo buildCmdInfo(const CmdEntry *entry)
{
	QBkeCmdInfo cmdInfo;
	cmdInfo.name = QString::fromUtf16(entry->name);
	for (uint32_t i = 0; i < entry->argCount; i++)
	{
		cmdInfo.argNames.push_back(QString::fromUtf16(entry->argNames[i]));
		cmdInfo.argFlags.push_back(entry->argFlags[i]);
		cmdInfo.argVersions.push_back({ entry->argVersions[i].from, entry->argVersions[i].to });
		cmdInfo.argEnums.push_back(QString::fromUtf16(entry->argEnums[i]));
	}
	for (uint32_t i = 0; i < entry->propertyCount; i++)
	{
		cmdInfo.properties.push_back({ QString::fromUtf16(entry->argProps[i].arg1), QString::fromUtf16(entry->argProps[i].arg2), entry->argProps[i].type });
	}
	cmdInfo.version = { entry->version.from, entry->version.to };
	cmdInfo.description = QString::fromUtf16(entry->description);
	cmdInfo.priority = entry->priority;
	return cmdInfo;
}

static void __cdecl QueryCmdListF(void *opaque, const CmdEntry *entry)
{
	QBkeCmdInfo info = buildCmdInfo(entry);
	CmdList.insert(info.name, info);
}

static void __cdecl QuerySpeCmdListF(void *opaque, const char16_t *name, uint32_t modeEnum, const CmdEntry *entry)
{
	QBkeCmdInfo info = buildCmdInfo(entry);
	QHash<QString, QBkeCmdInfo> &m = SpecialCmdList[QString::fromUtf16(name)];
	m.insert(info.name, info);
}

//static void __cdecl QueryEnumListF(void *opaque, const char16_t *name, uint32_t value)
//{
//	QHash<QString, uint32_t> *list = (QHash<QString, uint32_t> *)opaque;
//	list->insert(QString::fromUtf16(name), value);
//}

//struct
//{
//	QHash<QString, uint32_t> *list;
//	CmdEnumType type;
//} enums[] = {
//	{&CmdAnimateLoopModeEnumList, ENUM_ANIMATE_LOOP},
//};

static bool isLoadedCmd = false;

bool CmdListLoader::load()
{
	if (isLoadedCmd)
		return true;
#ifndef Q_OS_MAC
	QLibrary library(BKE_CURRENT_DIR + "/tool/" + "BKECmdList");
#else
    QLibrary library(BKE_CURRENT_DIR + "/" + "BKECmdList");
#endif
	if (!library.load())
	{
		return false;
	}
	QueryVersionFunc QueryVersion = (QueryVersionFunc)library.resolve("QueryPluginVersion");
	PQUERYCMDLIST QueryCmdList = (PQUERYCMDLIST)library.resolve("QueryCmdList");
	PQUERYSPECMDLIST QuerySpeCmdList = (PQUERYSPECMDLIST)library.resolve("QuerySpeCmdList");
	//PQUERYENUMLIST QueryEnumList = (PQUERYENUMLIST)library.resolve("QueryEnumList");
	if (!QueryVersion)
	{
		return false;
	}
	if (QueryVersion() != BKE_CMD_VERSION)
	{
		QMessageBox::warning(NULL, "警告", "找到BKECmdList插件但版本不匹配");
		return false;
	}
	CmdList.clear();
	SpecialCmdList.clear();
	//CmdAnimateLoopModeEnumList.clear();
	QueryCmdList(QueryCmdListF, nullptr);
	QuerySpeCmdList(QuerySpeCmdListF, nullptr);
	//for (auto &&s : enums)
	//{
	//	QueryEnumList(QueryEnumListF, s.type, s.list);
	//}
	isLoadedCmd = true;
	return true;
}
