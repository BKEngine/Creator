#pragma once
#include <QString>
#include <QHash>
#include <QSet>
#include <QList>
#include <QStringList>
#include "BKECmdList.h"

struct QBkeCmdVersion
{
	uint32_t from;
	uint32_t to;
	bool check(uint32_t v) { return v >= from && v < to; }
};

struct QBkeCmdProperty
{
	QString arg1;
	QString arg2;
	CmdPropertyType type;
};

struct QBkeCmdInfo 
{
	QString name;
	QList<QString> argNames;
	QList<uint32_t> argFlags;
	QList<QBkeCmdVersion> argVersions;
	QList<QBkeCmdProperty> properties;
	QList<QString> argEnums;
	QBkeCmdVersion version;
	QString description;
	int priority;
};

class CmdListLoader
{
public:
	static bool load();
};

extern QHash<QString, QBkeCmdInfo> CmdList;
extern QHash<QString, QHash<QString, QBkeCmdInfo>> SpecialCmdList;
//extern QHash<QString, uint32_t> CmdAnimateLoopModeEnumList;