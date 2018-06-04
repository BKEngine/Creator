#include <weh.h>
#include "BKS_info.h"
#include "bkeproject.h"
#include "dia/LangOpt.h"

GLOBALSTRUCTURES_INIT();

BKE_Info global_bke_info;

void BKE_Info::init()
{
	//Parser_GlobalInit();
	//Parser::getInstance() included by registerExtendFunction
	Bagel_VM::initVM();
	//registerExtendFunction();
	glo = Bagel_Closure::global();
	QStringList ls = QString("new enum if for while do foreach function class property propget propset continue break return var delete try throw this super global int string number const typeof instanceof extends in else then catch with static switch case true false void").split(' ');
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
	glo->setConstMember(L"tf", Bagel_Var::dic());
	glo->setConstMember(L"sf", Bagel_Var::dic());
	glo->setConstMember(L"f", Bagel_Var::dic());
	try
	{
		Bagel_VM::getInstance()->RunFile((BKE_CURRENT_DIR + "/BKE_dump.bkpsr").toStdU16String());
	}
	catch (Bagel_Except &e)
	{
		QMessageBox::warning(NULL, "执行BKE_dump出错:", QString::fromStdU16String(e.getMsg()));
	}
	//glo->setConstMember(L"basic_layer", -1);
	//glo->setConstMember(L"message_layer", -2);
	//glo->setConstMember(L"bgm", -1);
	//glo->setConstMember(L"voice", -2);
}

void BKE_Info::setProj(BkeProject *p)
{
	pro = p;
	StringVal w;
	bool res = Bagel_ReadFile(w, (p->ProjectFile() + ".user").toStdU16String());
	if (res && !w.empty())
	{
		auto runclo = new Bagel_Closure();
		Bagel_VM::getInstance()->setCurrentGlobal(runclo);
		try
		{
			projsetting = Bagel_VM::getInstance()->Run(w, runclo);
		}
		catch (Bagel_Except &){}
		Bagel_VM::getInstance()->setCurrentGlobal(glo);
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
	Bagel_WriteFile(projsetting->saveString(false), (pro->ProjectFile() + ".user").toStdU16String());
}
