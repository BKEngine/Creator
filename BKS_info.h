#pragma once

//记录系统指令，已注册的宏，已注册的变量，Bagel的关键字等等
#include <QString>
#include <QSet>
#include <QHash>

#include "ParserHelper/parser/parser.h"

#include "function.h"

struct ParamInfo
{
	enum
	{
		VOID,
		NUM,
		BOOL,
		STRING,
		LABEL,
		LITERAL,
		EXP,
		POS,
		RECT,
		IGNORE
	}type;

	QStringList options;
};

class BkeProject;

class BKE_Info
{
public:
	BKE_Info();

	BKE_VarClosure *glo;

	QSet<QString> syscmd;
	QSet<QString> macros;

	QSet<QString> BagelWords;
	QSet<QString> BagelOperators;
	QString OperatorAncestor;	//所有构成运算符的字符

	BKE_Variable projsetting;

	BkeProject *pro;

	~BKE_Info()
	{
	}

	void setProj(BkeProject *p);

	void save();
};

extern BKE_Info global_bke_info;
