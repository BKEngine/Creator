#include "weh.h"
#include "BKS_info.h"

GLOBALSTRUCTURES_INIT();

BKE_Info global_bke_info;

BKE_Info::BKE_Info()
{
	//Parser_GlobalInit();
	Parser::getInstance();
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
	BagelVars["tf"] = BKE_Variable::dic();
	BagelVars["sf"] = BKE_Variable::dic();
	BagelVars["f"] = BKE_Variable::dic();
	BagelVars["basic_layer"] = -1;
	BagelVars["message_layer"] = -1;
	BagelVars["bgm"] = -1;
	BagelVars["voice"] = -2;
}