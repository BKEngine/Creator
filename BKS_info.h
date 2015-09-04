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

	~BKE_Info()
	{
	}

	void setProj()
	{
		wstring w;
		bool res = BKE_readFile(w, (BKE_CURRENT_DIR + "/" + BKE_PROJECT_NAME + ".user").toStdWString());
		if (res)
		{
			try
			{
				projsetting = Parser::getInstance()->evalMultiLineStr(w);
			}
			catch (Var_Except &){}
		}
	}

	void save()
	{
		BKE_writeFile(projsetting.saveString(false), (BKE_CURRENT_DIR + "/" + BKE_PROJECT_NAME + ".user").toStdWString());
	}
};

extern BKE_Info global_bke_info;
