#include <weh.h>
#include "BKS_info.h"
#include "ParserHelper/parser/parserextend.h"
#include "bkeproject.h"
#include "dia/LangOpt.h"

GLOBALSTRUCTURES_INIT();

BKE_Info global_bke_info;

void BKE_Info::init()
{
	//Parser_GlobalInit();
	Parser::getInstance();
	registerExtendFunction();
	glo = BKE_VarClosure::global();
	QStringList ls = QString("if for while do foreach function class propget propset continue break return var delete try throw this super global int string number const typeof instanceof extends in else then catch with static switch case true false void").split(' ');
	for (auto &&v : ls)
	{
		BagelWords.insert(v);
	}
	QStringList ls2 = QString("+ - * / % ^ == != === !== > >= < <= & && | || ! = += -=*= /= %= ^= |= ++ -- ( [ %[ { . ? : ; ) ] } , =>").split(' ');
	for (auto &&v : ls2)
	{
		BagelOperators.insert(v);
	}
	OperatorAncestor = "+-*/%^=!><&|([{)]}.?,:;";
	glo->setConstMember(L"tf", BKE_Variable::dic());
	glo->setConstMember(L"sf", BKE_Variable::dic());
	glo->setConstMember(L"f", BKE_Variable::dic());
	glo->setConstMember(L"basic_layer", -1);
	glo->setConstMember(L"message_layer", -2);
	glo->setConstMember(L"bgm", -1);
	glo->setConstMember(L"voice", -2);
}

void BKE_Info::setProj(BkeProject *p)
{
	pro = p;
	wstring w;
	bool res = BKE_readFile(w, (p->ProjectFile() + ".user").toStdWString());
	if (res && !w.empty())
	{
		try
		{
			projsetting = Parser::getInstance()->evalMultiLineStr(w);
		}
		catch (Var_Except &){}
	}
	else
	{
		CLangEdit edit;
		edit.reset();
		edit.save();
	}
}

void BKE_Info::save()
{
	BKE_writeFile(projsetting.saveString(false), (pro->ProjectFile() + ".user").toStdWString());
}
