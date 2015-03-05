#include "parser.h"
#include <cstdarg>

#define THROW(a,b) throw Var_Except(a,b VAR_EXCEPT_EXT);
#define CHECKEMPTY(index) if(tree->childs[index]==NULL) {runpos=tree->Node.pos; throw Var_Except(L"未知错误，树不该为空" VAR_EXCEPT_EXT);}
#define GETLEFTVAR BKE_Variable &leftvar=innerRun(tree->childs[0], _this, tmpvars)
#define GETRIGHTVAR BKE_Variable &rightvar=innerRun(tree->childs[1], _this, tmpvars)
#define RECORDPOS	runpos=tree->Node.pos;
#define MUSTBEVAR(var) if(!var.isVar() && !var.isTemp()) {throw Var_Except(L"操作数必须是变量" VAR_EXCEPT_EXT);}
#define CHECKCLO(var) if(var.getType()==VAR_CLO || var.getType()==VAR_THIS){throw Var_Except(L"不能赋值为一个闭包" VAR_EXCEPT_EXT);}
#define QUICKRETURNCONST(v) runpos=tree->Node.pos;tmpvars.push_back(v);tmpvars.back().is_var=TEMP_VAR;return tmpvars.back();
#define RETURNNULL return none;
#define TODO RETURNNULL

//#define ADDTOTEMP(a) tempvars.push_back(a);a->release();

#define BUILD_TREE BKE_bytree* &tr=*tree;tr=new BKE_bytree(token);
#define GET_TREE tr->addChild();expression(&tr->childs.back());if(tr->childs.back()==NULL)throw Var_Except(L"此处期待一个输入", curpos - exp VAR_EXCEPT_EXT);
#define GET_TREE_OR_NULL tr->addChild();expression(&tr->childs.back());
#define GET_TREE2(lbp) tr->addChild();expression(&tr->childs.back(), lbp);if(tr->childs.back()==NULL)throw Var_Except(L"此处期待一个输入", curpos - exp VAR_EXCEPT_EXT);
#define GET_TREE2_OR_NULL(lbp) tr->addChild();expression(&tr->childs.back(), lbp);

#define CHECKPUSH (bc.codes[bc.pos-5]==BC_PUSH && *(bkplong*)(bc.codes+bc.pos-4)==bc.constsize-1)
#define CHECKPUSH2 (bc.codes[bc.pos-10]==BC_PUSH && *(bkplong*)(bc.codes+bc.pos-9)==bc.constsize-2)

wchar_t OP_CODE[][15] = {
	L"(end)",
	L"if",
	L"for(quickfor)",
	L"for",
	L"while",
	L"do",
	L"foreach",
	L"constvar",
	L"literal",
	L"function",
	L"class",
	L"propget",
	L"propset",
	L"+",
	L"-",
	L"*",
	L"/",
	L"%",
	L"^",
	L"==",
	L"!=",
	L"===",
	L"!==",
	L">",
	L"<",
	L">=",
	L"<=",
	L"&",
	L"&&",
	L"|",
	L"||",
	L"!",
	L"=",
	L"+=",
	L"-=",
	L"*=",
	L"/=",
	L"%=",
	L"^=",
	L"|=",
	L"++",
	L"--",
	L"(",
	L"[",
	L"%[",
	L"{",
	L".",
	L"continue",
	L"break",
	L"return",
	L"var",
	L"delete",
	L"try",
	L"throw",
	L"?",
	L"this",
	L"super",
	L"global",
	L"int",
	L"string",
	L"number",
	L"const",
	L"typeof",
	L"instanceof",
	L"",
	L"extends",
	L"in",
	L"else",
	L"then",
	L"catch",
	L":",
	L";",
	L")",
	L"]",
	L"}",
	L",",
	L"=>",
	L"with",
	L"incontextof",
	L"static",
	L"(invalid)",
	L""
};

Special_Except::Special_Except(error_type e, const BKE_Variable &v)
{
	type=e;
	res=v;
}

Special_Except::Special_Except(error_type e, BKE_Variable &&v)
{
	type = e;
	res = std::move(v);
}

Special_Except::Special_Except(const Special_Except &s)
{
	type=s.type;
	res=s.res;
}

Special_Except::Special_Except(Special_Except &&s)
{
	type = s.type;
	res = std::move(s.res);
}

Special_Except::~Special_Except()
{
	//res->release();
}

void BKE_VarThis::deleteMember(const BKE_Variable obj)
{
	auto it = clo->varmap.begin();
	while (it != clo->varmap.end())
	{
		if (it->second == obj)
			clo->varmap.erase(it++);
		else
			it++;
	}
};

bkplong BKE_VarThis::getNonVoidCount() const
{
	bkplong num = 0;
	auto it = clo->varmap.begin();
	for (; it != clo->varmap.end(); it++)
	if (!it->second.isVoid())
		num++;
	return num;
};

BKE_Variable BKE_Variable::run(BKE_Variable &self)
{
	if (getType() != VAR_FUNC)
		return BKE_Variable();
	auto *v = (BKE_VarFunction*)obj;
	if (self)
		v->setSelf(self);
	return v->run(NULL);
}

BKE_Variable readFromFile(const wstring &filename)
{
	wstring re;
	auto res = BKE_readFile(re, filename);
	if (res)
	{
		try
		{
			return Parser::getInstance()->eval(re);
		}
		catch (...)
		{
			return BKE_Variable();
		}
	}
	return BKE_Variable();
}

BKE_Variable BKE_VarFunction::run(const BKE_bytree *tr, BKE_VarClosure *_tr)
{
	BKE_VarClosure *tmp = new BKE_VarClosure(getClo());
	BKE_VarObjectAutoReleaser _tmp(tmp);
	int j = 1;
	BKE_array<BKE_Variable> tmpvars;
	//set default value
	for (auto i : initials)
	{
		tmp->setMember(i.first, i.second);
	}
	for (bkplong i = 0; i<(bkplong)paramnames.size(); i++)
	{
		if (paramnames[i][0] != L'*' && j < tr->childs.size())
			tmp->setMember(paramnames[i], Parser::getInstance()->innerRun(tr->childs[j++], _tr, tmpvars));
		else
		{
			wstring name = paramnames[i].substr(1);
			BKE_VarArray *arr = new BKE_VarArray();
			for (; j < tr->childs.size(); j++)
				arr->pushMember(Parser::getInstance()->innerRun(tr->childs[j], _tr, tmpvars));
			tmp->setMember(name, arr);
		}
	}
	try
	{
		BKE_Variable &res = Parser::getInstance()->innerRun(func->code, tmp, tmpvars);
		return res;
	}
	catch (Special_Except &e)
	{
		if (e.type == Special_Except::RETURN)
			return e.res;
		Var_Except ee(L"语法错误，不应当出现continue或break");
#if PARSER_DEBUG
		bkplong pos = Parser::getInstance()->runpos;
		auto it = upper_bound(linestartpos.begin(), linestartpos.end(), pos);
		if (it == linestartpos.end())
			ee.lineinfo = rawexp.substr(*(--it) - linestartpos[0]);
		else
		{
			ee.lineinfo = rawexp.substr(*(it - 1) - linestartpos[0], *it - *(it - 1));
			it--;
		}
		ee.addLine(it - linestartpos.begin() + 1);
		ee.addPos(pos - *it + 1);
		ee.functionname = this->name;
#endif
		throw ee;
	}
	catch (Var_Except &e)
	{
#if PARSER_DEBUG
		bkplong pos = Parser::getInstance()->runpos;
		if (linestartpos.size() == 0)
		{
			e.addPos(pos);
			throw e;
		}
		auto it = upper_bound(linestartpos.begin(), linestartpos.end(), pos);
		if (it == linestartpos.end())
			e.lineinfo = rawexp.substr(*(--it) - linestartpos[0]);
		else
		{
			e.lineinfo = rawexp.substr(*(it - 1) - linestartpos[0], *it - *(it - 1));
			it--;
		}
		e.addLine(it - linestartpos.begin() + 1);
		e.addPos(pos - *it + 1);
		e.functionname = this->name;
#endif
		throw e;
	}
}

BKE_FunctionCode::BKE_FunctionCode(BKE_bytree *code)
	: BKE_VarObject(VAR_FUNCCODE_P)
	, native(NULL)
{
	if (code)
		this->code = (BKE_bytree*)code->addRef();
	else
		this->code=NULL;
};

BKE_Variable BKE_FunctionCode::run(BKE_Variable *self, BKE_VarArray *paramarray, BKE_VarClosure *_this) const
{
	if (_this == NULL)
	{
		_this = BKE_VarClosure::global();
	}
	if(native)
		return (*native)(self, paramarray, _this);
	try
	{
		BKE_array<BKE_Variable> tmpvars;
		return Parser::getInstance()->innerRun(code, _this, tmpvars);
	}
	catch(Special_Except &e)
	{
		if(e.type==Special_Except::RETURN)
			return e.res;
		throw Var_Except(L"语法错误，不应当出现continue或break");
	}
}

BKE_FunctionCode::~BKE_FunctionCode()
{
	if(!MemoryPool().clearflag && code)
		code->release();
}

Parser::Parser()
{
	init();
}

void Parser::init()
{
	//init random seed
	srand((unsigned int)time(NULL));
	//init StringMap then Global
	StringMap();
	MemoryPool();

	costTime = 0;
	krmode = false;
	rawstr = false;

	forcequit=false;

	//add const
	none.makeConst();
	addConst(L"true",1);
	addConst(L"false",0);
	addConst(L"void",none);
	//addConst(L"global", BKE_VarClosure::Global()->addRef());

	Parser_Util::registerExtend(this);

	//register operators
	opmap[L"var"] = OP_VAR;
	opmap[L"delete"] = OP_DELETE;
	opmap[L"extends"] = OP_EXTENDS;
	opmap[L"continue"] = OP_CONTINUE;
	opmap[L"break"] = OP_BREAK;
	opmap[L"return"] = OP_RETURN;
	opmap[L"in"] = OP_IN;
	opmap[L"else"] = OP_ELSE;
	opmap[L"then"] = OP_THEN;
	opmap[L"if"] = OP_IF;
	opmap[L"for"] = OP_FOR;
	opmap[L"while"] = OP_WHILE;
	opmap[L"do"] = OP_DO;
	opmap[L"for"] = OP_FOR;
	opmap[L"foreach"] = OP_FOREACH;
	opmap[L"function"] = OP_FUNCTION;
	opmap[L"class"] = OP_CLASS;
	opmap[L"propget"] = OP_PROPGET;
	opmap[L"propset"] = OP_PROPSET;
	opmap[L"try"] = OP_TRY;
	opmap[L"catch"] = OP_CATCH;
	opmap[L"throw"] = OP_THROW;
	opmap[L"global"] = OP_GLOBAL;
	opmap[L"this"] = OP_THIS;
	opmap[L"super"] = OP_SUPER;
	opmap[L"int"] = OP_TOINT;
	opmap[L"string"] = OP_TOSTRING;
	opmap[L"number"] = OP_TONUMBER;
	opmap[L"typeof"] = OP_TYPE;
	opmap[L"const"] = OP_CONST;
	opmap[L"instanceof"] = OP_INSTANCE;
	opmap[L"incontextof"] = OP_INCONTEXTOF;
	opmap[L"static"] = OP_STATIC;
	opmap[L"with"] = OP_WITH;

	//register all functions
	for (int i = 0; i < OP_COUNT * 2; i++)
	{
		funclist[i] = &Parser::reportError;
		runner[i] = NULL;
	}
	funclist[OP_END + OP_COUNT] = &Parser::nud_end;
	funclist[OP_BRACKET + OP_COUNT] = &Parser::nud_bracket;
	funclist[OP_MUL + OP_COUNT] = &Parser::nud_label;
#define REG_NUD(c,a,b) funclist[b + OP_COUNT]=&Parser::nud_##c;runner[b + OP_COUNT]=&Parser::nudrunner_##a;
	REG_NUD(one, preadd, OP_ADD);
	REG_NUD(one, presub, OP_SUB);
	REG_NUD(one, not, OP_NOT);
	//REG_NUD(bracket, bracket, OP_BRACKET);
	REG_NUD(array, array, OP_ARRAY);
	REG_NUD(dic, dic, OP_DIC);
	REG_NUD(block, block, OP_BLOCK);
	REG_NUD(block, specialblock, OP_RESERVE);
	REG_NUD(one, inc, OP_SELFADD);
	REG_NUD(one, dec, OP_SELFDEC);
	//REG_NUD(label, label, OP_MUL);
	REG_NUD(if, if, OP_IF);
	REG_NUD(for, for, OP_FOR);
	REG_NUD(for, forin, OP_QUICKFOR);
	REG_NUD(foreach, foreach, OP_FOREACH);
	REG_NUD(do, do, OP_DO);
	REG_NUD(while, while, OP_WHILE);
	REG_NUD(propget, propget, OP_PROPGET);
	REG_NUD(propset, propset, OP_PROPSET);
	REG_NUD(function, function, OP_FUNCTION);
	REG_NUD(class, class, OP_CLASS);
	REG_NUD(continue, continue, OP_CONTINUE);
	REG_NUD(continue, break, OP_BREAK);
	REG_NUD(return, return, OP_RETURN);
	REG_NUD(var, var, OP_VAR);
	REG_NUD(delete, delete, OP_DELETE);
	REG_NUD(try, try, OP_TRY);
	REG_NUD(this, this, OP_THIS);
	REG_NUD(this, super, OP_SUPER);
	REG_NUD(this, global2, OP_GLOBAL);
	REG_NUD(one, int, OP_TOINT);
	REG_NUD(one, string, OP_TOSTRING);
	REG_NUD(one, number, OP_TONUMBER);
	REG_NUD(one, typeof, OP_TYPE);
	REG_NUD(one, const, OP_CONST);
	REG_NUD(var, unknown, OP_STATIC);
	REG_NUD(one, unknown, OP_THROW);
	REG_NUD(one, dot2, OP_DOT);
	REG_NUD(with, with, OP_WITH);
	REG_NUD(stop, unknown, OP_STOP);
	REG_NUD(stop, unknown, OP_COMMA);
	//REG_NUD(comment, comment, OP_COMMENT);
	REG_NUD(this, literal, OP_LITERAL);		//for any const variables and names
	REG_NUD(this, literal, OP_CONSTVAR);		//for any const variables and names
#undef REG_NUD
	//funclist[OP_END] = &Parser::led_none;
#define REG_LED(c, a,b) funclist[b]=&Parser::led_##c;runner[b]=&Parser::ledrunner_##a;
	REG_LED(two, add, OP_ADD);
	REG_LED(two, sub, OP_SUB);
	REG_LED(two, mul, OP_MUL);
	REG_LED(two, div, OP_DIV);
	REG_LED(two, mod, OP_MOD);
	REG_LED(pow, pow, OP_POW);
	REG_LED(two, eequal, OP_EEQUAL);
	REG_LED(two, equal, OP_EQUAL);
	REG_LED(two, nequal, OP_NEQUAL);
	REG_LED(two, nnequal, OP_NNEQUAL);
	REG_LED(two, larger, OP_LARGER);
	REG_LED(two, smaller, OP_SMALLER);
	REG_LED(two, le, OP_LE);
	REG_LED(two, se, OP_SE);
	REG_LED(two, and, OP_AND);
	REG_LED(two, or, OP_OR);
	REG_LED(two, fastand, OP_FASTAND);
	REG_LED(two, fastor, OP_FASTOR);
	REG_LED(set, set, OP_SET);
	REG_LED(set, setadd, OP_SETADD);
	REG_LED(set, setsub, OP_SETSUB);
	REG_LED(set, setmul, OP_SETMUL);
	REG_LED(set, setdiv, OP_SETDIV);
	REG_LED(set, setpow, OP_SETPOW);
	REG_LED(set, setmod, OP_SETMOD);
	REG_LED(set, setset, OP_SETSET);
	REG_LED(one, selfadd, OP_SELFADD);
	REG_LED(one, selfsub, OP_SELFDEC);
	//REG_LED(comment, comment, OP_COMMENT);
	REG_LED(param, param, OP_BRACKET);	//used in function call
	REG_LED(dot, dot, OP_DOT);
	REG_LED(ele, ele, OP_ARRAY);
	REG_LED(choose, choose, OP_CHOOSE);
	REG_LED(two, instanceof, OP_INSTANCE);
	REG_LED(two, if, OP_IF);
	REG_LED(two, unknown, OP_INCONTEXTOF);
#undef REG_LED

	//register nud functions
//	for (int i = 0; i<OP_COUNT; i++)
//	{
//		nudlist[i] = &Parser::reportError;
//		nud[i] = NULL;
//	}
//	nudlist[OP_BRACKET] = &Parser::nud_bracket;
//	nudlist[OP_MUL]=&Parser::nud_label;
//	nudlist[OP_OTHER]=&Parser::nud_other;
//	nudlist[OP_FUNCTION] = &Parser::nud_function;
//#define REG_NUD(c,a,b) nudlist[b]=&Parser::nud_##c;nud[b]=&Parser::nudrunner_##a;
//	REG_NUD(one, preadd, OP_ADD);
//	REG_NUD(one, presub, OP_SUB);
//	REG_NUD(one, not, OP_NOT);
//	//REG_NUD(bracket, bracket, OP_BRACKET);
//	REG_NUD(array, array, OP_ARRAY);
//	REG_NUD(dic, dic, OP_DIC);
//	REG_NUD(block, block, OP_BLOCK);
//	REG_NUD(block, specialblock, OP_RESERVE);
//	REG_NUD(one, inc, OP_SELFADD);
//	REG_NUD(one, dec, OP_SELFDEC);
//	//REG_NUD(label, label, OP_MUL);
//	REG_NUD(if, if, OP_IF);
//	REG_NUD(for, for, OP_FOR);
//	REG_NUD(foreach, foreach, OP_FOREACH);
//	REG_NUD(do, do, OP_DO);
//	REG_NUD(while, while, OP_WHILE);
//	REG_NUD(function, propget, OP_PROPGET);
//	REG_NUD(function, propset, OP_PROPSET);
//	//REG_NUD(function, function, OP_FUNCTION);
//	REG_NUD(class, class, OP_CLASS);
//	REG_NUD(continue, continue, OP_CONTINUE);
//	REG_NUD(continue, break, OP_BREAK);
//	REG_NUD(return, return, OP_RETURN);
//	REG_NUD(var, var, OP_VAR);
//	REG_NUD(delete, delete, OP_DELETE);
//	REG_NUD(try, try, OP_TRY);
//	//REG_NUD(new, new, OP_CLASS);
//	//REG_NUD(comment, comment, OP_COMMENT);
//	REG_NUD(literal, literal, OP_LITERAL);		//for any const variables and names
//#undef REG_NUD
//
//	//register led functions
//	for(int i=0;i<OP_COUNT;i++)
//	{
//		ledlist[i]=&Parser::reportError;
//		led[i]=NULL;
//	}
//	ledlist[OP_END]=&Parser::led_none;
//	ledlist[OP_LITERAL]=&Parser::led_literal;
//#define REG_LED(c, a,b) ledlist[b]=&Parser::led_##c;led[b]=&Parser::ledrunner_##a;
//	REG_LED(two, add, OP_ADD);
//	REG_LED(two, sub, OP_SUB);
//	REG_LED(two, mul, OP_MUL);
//	REG_LED(two, div, OP_DIV);
//	REG_LED(two, mod, OP_MOD);
//	REG_LED(pow, pow, OP_POW);
//	REG_LED(two, equal, OP_EQUAL);
//	REG_LED(two, nequal, OP_NEQUAL);
//	REG_LED(two, larger, OP_LARGER);
//	REG_LED(two, smaller, OP_SMALLER);
//	REG_LED(two, le, OP_LE);
//	REG_LED(two, se, OP_SE);
//	REG_LED(two, and, OP_AND);
//	REG_LED(two, or, OP_OR);
//	REG_LED(two, fastand, OP_FASTAND);
//	REG_LED(two, fastor, OP_FASTOR);
//	REG_LED(set, set, OP_SET);
//	REG_LED(two, setadd, OP_SETADD);
//	REG_LED(two, setsub, OP_SETSUB);
//	REG_LED(two, setmul, OP_SETMUL);
//	REG_LED(two, setdiv, OP_SETDIV);
//	REG_LED(two, setpow, OP_SETPOW);
//	REG_LED(two, setmod, OP_SETMOD);
//	REG_LED(one, selfadd, OP_SELFADD);
//	REG_LED(one, selfsub, OP_SELFDEC);
//	//REG_LED(comment, comment, OP_COMMENT);
//	REG_LED(param, param, OP_BRACKET);	//used in function call
//	REG_LED(dot, dot, OP_DOT);
//	REG_LED(ele, ele, OP_ARRAY);
//	REG_LED(choose, choose, OP_CHOOSE);
//#undef REG_LED

	//register lbp and pre
	//for(int i=0;i<OP_COUNT;i++)
	//{
	//	lbp[i]=0;
	//	pre[i]=0;
	//}
	memset(lbp, 0, sizeof(lbp));
	pre = lbp + OP_COUNT;
	memset(head, 0, sizeof(head));

	//[ ( %[
	//if(nud) while do for foreach function class propget propset
	//= += -= *= /= %= ^= |=
	//if(led) instanceof incontextof
	//? :
	//&& & || |
	//== === != !==
	//> < >= <=
	//+-
	//*/
	//% ^
	//!
	//+(nud) -(nud) ++(nud) ++ --(nud) --
	//int string number typeof
	//((led) [(led)
	//. *(nud) .(bud)

#define PRE(a) pre[OP_##a]=tmp;
#define LBP(a) lbp[OP_##a]=tmp;
#define INC	tmp+=10;
	int tmp = 0;
	LBP(BLOCK);	//whenever we meet a {, we think it's linehead
	INC;
	PRE(BLOCK);
	PRE(ARRAY);
	PRE(BRACKET);
	PRE(DIC);
	INC;
	PRE(IF);
	PRE(WHILE);
	PRE(DO);
	PRE(FOR);
	PRE(QUICKFOR);
	PRE(FOREACH);
	PRE(FUNCTION);
	PRE(CLASS);
	PRE(PROPGET);
	PRE(PROPSET);
	INC;
	LBP(SET);
	LBP(SETADD);
	LBP(SETSUB);
	LBP(SETMUL);
	LBP(SETDIV);
	LBP(SETMOD);
	LBP(SETPOW);
	LBP(SETSET);
	INC;
	LBP(IF);
	LBP(INSTANCE);
	LBP(INCONTEXTOF);
	INC;
	LBP(CHOOSE);
	INC;
	LBP(AND);
	LBP(OR);
	LBP(FASTAND);
	LBP(FASTOR);
	INC;
	LBP(EQUAL);
	LBP(NEQUAL);
	LBP(EEQUAL);
	LBP(NNEQUAL);
	INC;
	LBP(SMALLER);
	LBP(LARGER);
	LBP(LE);
	LBP(SE);
	INC;
	LBP(ADD);
	LBP(SUB);
	INC;
	LBP(MUL);
	LBP(DIV);
	INC;
	LBP(MOD);
	LBP(POW);
	INC;
	PRE(NOT);
	INC;
	PRE(ADD);
	PRE(SUB);
	PRE(SELFADD);
	PRE(SELFDEC);
	LBP(SELFADD);
	LBP(SELFDEC);
	INC;
	PRE(TOINT);
	PRE(TOSTRING);
	PRE(TONUMBER);
	PRE(TYPE);
	INC;
	LBP(BRACKET);
	LBP(ARRAY);
	INC;
	LBP(DOT);
	PRE(DOT);
	PRE(MUL);

	head[OP_WHILE] = 1;
	head[OP_DO] = 1;
	head[OP_FOR] = 1;
	head[OP_FOREACH] = 1;
	head[OP_IF] = 1;
	head[OP_CLASS] = 1;
	head[OP_PROPGET] = 1;
	head[OP_PROPSET] = 1;
}

#define MATCH(a, b) if(MatchFunc(L##a, &curpos)){ node.opcode = b; return;}
#define QRET(c)	curpos++;node.opcode = c;return;

void Parser::readToken()
{
	//bufnodes.push_back(BKE_Node());
	//BKE_Node &node = bufnodes.back();
	BKE_Node &node = next;
#if PARSER_DEBUG>=2
	o_stderr << L"token at " << *curpos << L'\n';
	if(iswspace(*curpos))
		o_stderr << (int)(*curpos) << L"is white!\n";
#endif
	while ((*curpos) && bkpIsWSpace(*curpos))curpos++;
	node.pos = curpos - exp;
	if (!(*curpos))
	{
		node.opcode = OP_END;
		return;
	}
	wchar_t ch = *curpos;
	//read operators
#if PARSER_DEBUG>=2
	o_stderr<<L"token at "<<ch<<L'\n';
#endif
	switch (ch)
	{
	case L'+':
		MATCH("+=", OP_SETADD);
		MATCH("++", OP_SELFADD);
		QRET(OP_ADD);
	case L'-':
		MATCH("-=", OP_SETSUB);
		MATCH("--", OP_SELFDEC);
		QRET(OP_SUB);
	case L'*':
		MATCH("*=", OP_SETMUL);
		QRET(OP_MUL);
	case L'/':
		if (MatchFunc(L"//", &curpos))
		{
			//skip a line
			while (*curpos != L'\0' && *curpos != L'\r' && *curpos != L'\n')
				curpos++;
			//bufnodes.pop_back();
			readToken();
			return;
		}
		if (MatchFunc(L"/*", &curpos))
		{
			while (*curpos && !(*curpos == L'*' && *(curpos + 1) == L'/'))
				curpos++;
			//bufnodes.pop_back();
			curpos += 2;
			readToken();
			return;
		}
		MATCH("/=", OP_SETDIV);
		QRET(OP_DIV);
	case L'%':
		MATCH("%[", OP_DIC);
		MATCH("%=", OP_SETMOD);
		QRET(OP_MOD);
	case L'^':
		MATCH("^=", OP_SETPOW);
		QRET(OP_POW);
	case L'!':
		MATCH("!==", OP_NNEQUAL);
		MATCH("!=", OP_NEQUAL);
		QRET(OP_NOT);
	case L'=':
		MATCH("===", OP_EEQUAL);
		MATCH("=>", OP_VALUE);
		MATCH("==", OP_EQUAL);
		QRET(OP_SET);
	case L'>':
		MATCH(">=", OP_LE);
		QRET(OP_LARGER);
	case L'<':
		MATCH("<=", OP_SE);
		QRET(OP_SMALLER);
	case L'|':
		MATCH("|=", OP_SETSET);
		MATCH("||", OP_OR);
		QRET(OP_FASTOR);
	case L'&':
		MATCH("&&", OP_AND);
		QRET(OP_FASTAND);
	case L'(':
		QRET(OP_BRACKET);
	case L'[':
		QRET(OP_ARRAY);
	case L'{':
		QRET(OP_BLOCK);
	case L'.':
		NextIsBareWord = true;
#if PARSER_DEBUG>=2
		o_stderr << L"token after dot is " << *(curpos+1) << L'\n';
#endif
		QRET(OP_DOT);
	case L'?':
		QRET(OP_CHOOSE);
	case L':':
		QRET(OP_MAOHAO);
	case L';':
		QRET(OP_STOP);
	case L')':
		QRET(OP_BRACKET2);
	case L']':
		QRET(OP_ARR2);
	case L'}':
		QRET(OP_BLOCK2);
	case L',':
		QRET(OP_COMMA);
	case L'\"':
		if (!krmode)
		{
			//read const string
			wstring tmp = L"";
			while (*(++curpos))
			{
				ch = *curpos;
				if (ch == '\"')
				{
					curpos++;
					if (*curpos != '\"')
						break;
					else
					{
						tmp.push_back(ch);
					}
				}
				else if (!rawstr && (ch == L'\n' || ch == L'\r' || ch == L'\0'))
				{
					throw Var_Except(L"读字符串时遇到意料之外的结尾", curpos - exp);
				}
				else
					tmp.push_back(ch);
			}
			BKE_String _hashtmp(tmp);
			node.opcode = OP_CONSTVAR;
			//node.varindex = getVarIndex(_hashtmp);
			node.var = _hashtmp;
			return;
		}
		break;
	case L'0':
	case L'1':
	case L'2':
	case L'3':
	case L'4':
	case L'5':
	case L'6':
	case L'7':
	case L'8':
	case L'9':
		//read int
		//node.varindex = getVarIndex(str2num(curpos, &curpos));
		node.var = str2num(curpos, &curpos);
		node.opcode = OP_CONSTVAR;
		return;
	case L'#':
		{
			//color number
			//RGBA
			unsigned char n[8];
			int nn = 0;
			bkpulong color = 0;
			ch = *(++curpos);
			while (1)
			{
				if (L'0' <= ch && ch <= L'9')
					n[nn++] = ch - L'0';
				else if (L'a' <= ch && ch <= L'f')
					n[nn++] = ch - L'a' + 10;
				else if (L'A' <= ch && ch <= L'F')
					n[nn++] = ch - L'A' + 10;
				else
					break;
				ch = *(++curpos);
			}
			switch (nn)
			{
			case 3:
				n[3] = 15;
			case 4:
				color = 0x11000000 * n[3];
				color |= 0x110000 * n[0];
				color |= 0x1100 * n[1];
				color |= 0x11 * n[2];
				break;
			case 6:
				n[6] = 0x0F;
				n[7] = 0x0F;
			case 8:
				color |= (0x10000000 * n[7]) | (0x01000000 * n[6]);
				color |= (0x100000 * n[1]) | (0x010000 * n[0]);
				color |= (0x1000 * n[3]) | (0x0100 * n[2]);
				color |= (0x10 * n[5]) | (0x01 * n[4]);
				break;
			default:
				throw Var_Except(L"#后面只能接3,4,6,或8个十六进制数字表示颜色值", curpos - exp);
			}
			//node.varindex = getVarIndex(color);
			node.var = color;
			node.opcode = OP_CONSTVAR;
			return;
		}
	}
	if (ch == L'\'' || (ch=='\"' && krmode))
	{
		wchar_t startch = ch;
		//read const string
		wstring tmp = L"";
		while (*(++curpos))
		{
			ch = *curpos;
			if (ch == '\\')
			{
				bkplong s;
				if (!*(++curpos))
				{
					throw Var_Except(L"读字符串时遇到意料之外的结尾", curpos - exp);
				};
				switch (ch = *curpos)
				{
				case L'n':
					tmp.push_back(L'\n');
					break;
				case L'r':
					tmp.push_back(L'\r');
					break;
				case L't':
					tmp.push_back(L'\t');
					break;
				case L'a':
					tmp.push_back(L'\a');
					break;
				case L'b':
					tmp.push_back(L'\b');
					break;
				case L'f':
					tmp.push_back(L'\f');
					break;
				case L'v':
					tmp.push_back(L'\v');
					break;
				case L'o':
					ch = *(++curpos);
					s = 0;
					if (ch<L'0' || ch>L'9')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s += 64 * (ch - L'0');
					ch = *(++curpos);
					if (ch<L'0' || ch>L'9')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s += 8 * (ch - L'0');
					ch = *(++curpos);
					if (ch<L'0' || ch>L'9')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s += (ch - L'0');
					tmp.push_back((wchar_t)s);
					break;
				case L'x':
					ch = *(++curpos);
					s = 0;
					if (ch<L'0' || (ch>L'9' && towupper(ch)<L'A') || towupper(ch)>L'F')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s += 16 * (ch>L'9' ? towupper(ch) - L'A' + 10 : ch - L'0');
					ch = *(++curpos);
					if (ch<L'0' || (ch>L'9' && towupper(ch)<L'A') || towupper(ch)>L'F')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s += ch>L'9' ? towupper(ch) - L'A' + 10 : ch - L'0';
					tmp.push_back((wchar_t)s);
					break;
				case L'\n':
				case L'\r':
					throw Var_Except(L"读字符串时遇到意料之外的结尾", curpos - exp);
				default:
					tmp.push_back(ch);
				}
			}
			else if (!rawstr && (ch == L'\n' || ch == L'\r' || ch == L'\0'))
			{
				throw Var_Except(L"读字符串时遇到意料之外的结尾", curpos - exp);
			}
			else if (ch == startch)
			{
				curpos++;
				break;
			}
			else
				tmp.push_back(ch);
		}
		BKE_String _hashtmp(tmp);
		node.opcode = OP_CONSTVAR;
		//node.varindex = getVarIndex(_hashtmp);
		node.var = _hashtmp;
		return;
	}
	if (ch >= 0x80 || isalpha(ch) || ch == L'_' /*|| ch==L'$' || ch==L'#'*/)
	{
		//read variable name
#if PARSER_DEBUG>=2
		if (NextIsBareWord)
			o_stderr << ch << L'\n';
#endif
		wstring tmp;
		tmp.push_back(ch);
		while ((ch = *(++curpos)))
		{
			if (ch >= 0x80 || iswalnum(ch) || ch == '_')
				tmp.push_back(ch);
			else
				break;
		}
		if (!NextIsBareWord)
		{
			if (constmap.find(tmp) != constmap.end())
			{
				//node.varindex = getVarIndex(constmap[tmp]);
				node.var = constmap[tmp];
				node.opcode = OP_CONSTVAR;
				return;
			}
			auto it = opmap.find(tmp);
			if (it != opmap.end())
			{
				node.opcode = it->second;
				return;
			}
		}
		NextIsBareWord = false;
		BKE_String _hashtmp(tmp);
		node.opcode = OP_LITERAL;
		//node.varindex = getVarIndex(_hashtmp);
		node.var = _hashtmp;
		return;
	}
	throw Var_Except(wstring(L"读取时遇到非法字符<") + ch + L'>', curpos - exp);
}

void Parser::expression(BKE_bytree** tree, int rbp)
{
	//while (next.opcode == OP_STOP)
	//	readToken();
	if (rbp>20 && head[next.opcode])
	{
		throw Var_Except(L"该符号必须放在句首。", next.pos);
	}
	token = next;
	readToken();
#if PARSER_DEBUG>=2
	o_stderr << L"readToken : " << OP_CODE[next.opcode];
	if (next.opcode == OP_CONSTVAR)
		o_stderr << L" " << next.var.save(false);
	else if (next.opcode == OP_LITERAL)
		o_stderr << L" " << next.var.asBKEStr().getConstStr();
	o_stderr << L"\n";
	if (next.opcode == OP_DOT)
	{
		o_stderr << L"now token after dot is " << *curpos << L'\n';
	}
#endif
	token.opcode += OP_COUNT;
	(this->*funclist[token.opcode])(tree);
	while (!forcequit && rbp<lbp[next.opcode])
	{
		token = next;
#if PARSER_DEBUG>=2
		o_stderr << L"readToken2 : " << OP_CODE[next.opcode];
		if (next.opcode == OP_CONSTVAR)
			o_stderr << L" " << next.var.save(false);
		else if (next.opcode == OP_LITERAL)
			o_stderr << L" " << next.var.asBKEStr().getConstStr();
		o_stderr << L"\n";
		if (next.opcode == OP_DOT)
		{
			o_stderr << L"now token after dot is " << *curpos << L'\n';
		}
#endif
		readToken();
#if PARSER_DEBUG>=2
		o_stderr << L"readToken3 : " << OP_CODE[next.opcode];
		if (next.opcode == OP_CONSTVAR)
			o_stderr << L" " << next.var.save(false);
		else if (next.opcode == OP_LITERAL)
			o_stderr << L" " << next.var.asBKEStr().getConstStr();
		o_stderr << L"\n";
		if (next.opcode == OP_DOT)
		{
			o_stderr << L"now token after dot is " << *curpos << L'\n';
		}
#endif
		(this->*funclist[token.opcode])(tree);
	}
	forcequit = false;
}

BKE_Variable& Parser::getVar(const wchar_t *exp, BKE_VarClosure *_this)
{
	init2(exp, _this);
	BKE_bytree *tr=NULL;
	try 
	{
		readToken();
		expression(&tr);
		if (next.opcode != OP_END)
		{
			BKE_bytree *subtr = tr;
			tr = new BKE_bytree();
			tr->Node.opcode = OP_RESERVE + OP_COUNT;
			tr->childs.push_back(subtr);
		}
		while (next.opcode != OP_END)
		{
			if (next.opcode == OP_STOP)
			{
				readToken();
				continue;
			}
			GET_TREE_OR_NULL;
		}
		BKE_array<BKE_Variable> tmpvars;
		BKE_Variable &res = innerRun(tr, _this, tmpvars);
		if (!res.isVar())
		{
			throw Var_Except(L"不是变量");
		}
		if (tr)
			tr->release();
		return res;
	}
	catch(Var_Except &e)
	{
		if(runpos>-1)
			e.addPos(runpos);
		if (tr)
			tr->release();
		throw e;
	}
	catch(Special_Except &)
	{
		if (tr)
			tr->release();
		throw Var_Except(L"continue,break或return出现的时机不对");
	}
}

BKE_bytree *Parser::parse(const wstring &exp, bkplong startpos, bkplong startline)
{
	init2(exp, NULL);
	BKE_bytree *tr=NULL;
	curpos += startpos;
#if PARSER_DEBUG
	if (startline > 0)
		linestartpos.insert(linestartpos.begin(), startline, -1);
#endif
		try
	{
		readToken();
		if (next.opcode == OP_END)
			return NULL;
		expression(&tr);
		//if (tr && tr->Node.opcode == OP_CONSTVAR + OP_COUNT)
		//{
		//	tr->release();
		//	tr = NULL;
		//};
		if(next.opcode != OP_END)
		{
			BKE_bytree *subtr=tr;
			tr=new BKE_bytree();
			tr->Node.opcode = OP_RESERVE + OP_COUNT;
			tr->childs.push_back(subtr);
		}
		while (next.opcode != OP_END)
		{
			if (next.opcode == OP_STOP)
			{
				//release last constexpr
				if (!tr->childs.empty() && tr->childs.back()->Node.opcode == OP_CONSTVAR + OP_COUNT)
				{
					tr->childs.back()->release();
					tr->childs.pop_back();
				}
				readToken();
				continue;
			}
			GET_TREE_OR_NULL;
		}
		if (tr && tr->Node.opcode == OP_RESERVE + OP_COUNT && tr->childs.size() == 1)
		{
			auto tr2=tr->childs.back();
			tr->childs.back()=NULL;
			tr->release();
			return tr2;
		}
		return tr;
	}
	catch(Var_Except &e)
	{
		bkplong pos;
		pos = e.getPos();
#if PARSER_DEBUG
		auto it = upper_bound(linestartpos.begin(), linestartpos.end(), pos);
		if (it == linestartpos.end())
			e.lineinfo = exp.substr(*(--it));
		else
		{
			e.lineinfo = exp.substr(*(it - 1), *it - *(it - 1));
			it--;
		}
		e.addLine(it - linestartpos.begin() + 1);
		e.addPos(pos - *it + 1);
		if (tr)
			tr->release();
		errorstack.push_back(e);
#endif
		throw e;
	}
}

BKE_Variable Parser::evalMultiLineStr(const wstring &exp, BKE_VarClosure *_this)
{
#if PARSER_DEBUG
	double t1 = getutime();
#endif
	if (exp.empty())
		return none;
	init2(exp, _this);
	errorstack.clear();
	BKE_bytree *tr = NULL;
	BKE_Variable res;
	try
	{
		readToken();
		expression(&tr);
		if (next.opcode != OP_END)
		{
			BKE_bytree *subtr = tr;
			tr = new BKE_bytree();
			tr->Node.opcode = OP_RESERVE + OP_COUNT;
			tr->childs.push_back(subtr);
		}
		while (next.opcode != OP_END)
		{
			GET_TREE_OR_NULL;
		}
		res = run(tr, _this);
		if (tr)
			tr->release();
#if PARSER_DEBUG
		costTime += (getutime() - t1);
#endif
		return std::move(res);
	}
	catch (Var_Except &e)
	{
		bkplong pos;
		if (!e.hasPos())
			pos = runpos;
		else
			pos = e.getPos();
#if PARSER_DEBUG
		auto it = upper_bound(linestartpos.begin(), linestartpos.end(), pos);
		if (it == linestartpos.end())
			e.lineinfo = exp.substr(*(--it));
		else
		{
			e.lineinfo = exp.substr(*(it - 1), *it - *(it - 1));
			it--;
		}
		e.addLine(it - linestartpos.begin() + 1);
		e.addPos(pos - *it + 1);
		if (tr)
			tr->release();
		errorstack.push_back(e);
#endif
#if PARSER_DEBUG
		costTime += (getutime() - t1);
#endif
		throw e;
	}
}

BKE_Variable Parser::eval(const wchar_t *exp, BKE_VarClosure *_this)
{
#if PARSER_DEBUG
	double t1 = getutime();
#endif
	init2(exp, _this);
	BKE_bytree *tr=NULL;
	BKE_Variable res;
	try
	{
		readToken();
		if (next.opcode == OP_END)
			return none;
		expression(&tr);
		if (next.opcode != OP_END)
		{
			BKE_bytree *subtr = tr;
			tr = new BKE_bytree();
			tr->Node.opcode = OP_RESERVE + OP_COUNT;
			tr->childs.push_back(subtr);
		}
		while (next.opcode != OP_END)
		{
			if (next.opcode == OP_STOP)
			{
				readToken();
				continue;
			}
			GET_TREE_OR_NULL;
		}
		res = run(tr, _this);
		if (tr)
			tr->release();
#if PARSER_DEBUG
		costTime += (getutime() - t1);
#endif
		return std::move(res);
	}
	catch(Var_Except &e)
	{
		if(runpos>-1)
			e.addPos(runpos);
		if (tr)
			tr->release();
#if PARSER_DEBUG
		costTime += (getutime() - t1);
#endif
		throw e;
	}
	//always catched by Run
	//catch(Special_Except &e)
	//{
	//	if (tr)
	//		tr->release();
	//	throw Var_Except(L"continue,break或return出现的时机不对");
	//}
}

BKE_Variable Parser::run(BKE_bytree *tree, BKE_VarClosure *_this)
{
	try
	{
		if(!tree)
			return BKE_Variable();
		if(tree->Node.opcode==OP_END)
			return BKE_Variable();
		BKE_array<BKE_Variable> tmpvars;
		BKE_Variable &res=innerRun(tree, _this, tmpvars);
		if(res.getType()==VAR_PROP)
			return ((BKE_VarProp*)res.obj)->get();
		return res;
	}
	catch(Special_Except &)
	{
		throw Var_Except(L"continue,break或return出现的时机不对");
	}
}

//BKE_Variable& Parser::innerRun(BKE_bytree *tree, BKE_VarClosure *_this, BKE_array<BKE_Variable> &tmpvars)
//{
//	if(!tree)
//	{
//		return none;
//	}
//	return (this->*runner[tree->Node.opcode])(tree, _this, tmpvars);
//	//if(tree->Node.isnud)
//	//	return (this->*nud[tree->Node.opcode])(tree, _this, tmpvars);
//	//else
//	//	return (this->*led[tree->Node.opcode])(tree, _this, tmpvars);
//}next.opcode == OP_BRACKET2

void Parser::nud_one(BKE_bytree** tree)
{
	BUILD_TREE;
	GET_TREE2(pre[token.opcode - OP_COUNT]);
	if (tr->Node.opcode == OP_SELFADD + OP_COUNT || tr->Node.opcode == OP_SELFDEC + OP_COUNT)
	{
		if (!isVar(tr->childs[0]))
		{
			THROW(L"等号左边必须是变量", token.pos);
		}
	}
	else if (tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT && runner[tr->Node.opcode] != NULL)
	{
		BKE_array<BKE_Variable> tmpvars;
		tr->Node.var.forceSet(innerRun(tr, this->p, tmpvars));
		tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
		tr->clearChilds();
	}
}

void Parser::nud_bracket(BKE_bytree** tree)
{
	auto rawpos = curpos;
	bkplong p = token.pos;
	if (next.opcode >= OP_TOINT && next.opcode <= OP_CONST)
	{
		next.opcode += OP_COUNT;
		token = next;
		//BKE_bytree* &tr = *tree;
		//tr = new BKE_bytree(next);
		readToken();
		if (next.opcode == OP_BRACKET2)
		{
			readToken();
			nud_one(tree);
			//GET_TREE2(pre[OP_TOINT]);
			return;
		}
		else
		{
			//goback
			//GET_TREE2(pre[OP_TOINT]);
			next = token;
			next.opcode -= OP_COUNT;
			curpos = rawpos;
		}
	}
	expression(tree);
	if (next.opcode != OP_BRACKET2)
	{
		THROW(getPos(p) + L"(需要)结尾。", next.pos);
	};
	readToken();
}

//if(xxx)yyy[;] [else zzz]
void Parser::nud_if(BKE_bytree** tree)
{
	BUILD_TREE;
	if(next.opcode!=OP_BRACKET)
	{
		THROW(L"if后必须接(", next.pos);
	}
	readToken();
	GET_TREE;
	if(next.opcode != OP_BRACKET2)
	{
		THROW(wstring(L"语法错误，在需要)的地方读到") + OP_CODE[next.opcode%OP_COUNT], next.pos);
	}
	readToken();
	GET_TREE_OR_NULL;
	tr->addChild();
	if(next.opcode == OP_STOP)
		readToken();
	if(next.opcode == OP_ELSE)
	{
		readToken();
		expression(&tr->childs.back());
	}
	if(tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT)
	{
		if((bool)tr->childs[1]->Node.var)
		{
			BKE_bytree *res=tr->childs[1];
			tr->childs[1]=NULL;
			tr->release();
			(*tree)=res;
		}
		else
		{
			BKE_bytree *res=tr->childs[2];
			tr->childs[2]=NULL;
			tr->release();
			(*tree)=res;
		}
	}
	forcequit = true;
}

//try{...}catch(x){...}
//try ... catch(x) ...
void Parser::nud_try(BKE_bytree** tree)
{
	BUILD_TREE;
	GET_TREE;
	if (next.opcode == OP_STOP)
		readToken();
	if (next.opcode != OP_CATCH)
	{
		THROW(L"try必须要有对应的catch块", next.pos);
	}
	readToken();
	if (next.opcode != OP_BRACKET)
	{
		THROW(L"catch后必须接(", next.pos);
	}
	readToken();
	if (next.opcode != OP_LITERAL)
	{
		THROW(L"catch(后必须是变量名", next.pos);
	};
	next.opcode += OP_COUNT;
	tr->childs.push_back(new BKE_bytree(next));
	readToken();
	if (next.opcode != OP_BRACKET2)
	{
		THROW(L"语法错误，catch后的括号中只能接合法的变量名，不能接字典或数组中的元素名，也不能接其他东西。", next.pos);
	}
	readToken();
	GET_TREE_OR_NULL;
	if (!tr->childs[0] || tr->childs[0]->Node.opcode == OP_LITERAL)
	{
		//常量不会抛出异常
		tr->release();
		tr = NULL;
	}
}

void Parser::nud_with(BKE_bytree **tree)
{
	BUILD_TREE;
	if (next.opcode != OP_BRACKET)
	{
		THROW(L"此处需要(", next.pos);
	}
	GET_TREE;
	GET_TREE_OR_NULL;
	forcequit = true;
}

void Parser::nud_array(BKE_bytree** tree)
{
	BUILD_TREE;
	bkplong p = token.pos;
	if (next.opcode == OP_ARR2)
	{
		readToken();
		tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
		tr->Node.var.makeVar();
		tr->Node.var = BKE_Variable::array();
		tr->Node.var.makeConst();
		return;
	}
	tr->addChild();
	bool cancal=true;
	do
	{
		if (next.opcode != OP_COMMA)
		{
			expression(&tr->childs.back(), pre[OP_ARRAY]-1);
			if (tr->childs.back() && tr->childs.back()->Node.opcode != OP_CONSTVAR + OP_COUNT)
				cancal = false;
		}
		tr->addChild();
		if (next.opcode != OP_COMMA)
			break;
		readToken();
	} while (next.opcode != OP_ARR2);
	if (next.opcode != OP_ARR2)
	{
		THROW(getPos(p) + L"[需要]结尾。", next.pos);
	};
	readToken();
	if(cancal)
	{
		BKE_array<BKE_Variable> tmpvars;
		tr->Node.var.forceSet(innerRun(tr, this->p, tmpvars));
		tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
		tr->Node.var.makeConst();
		tr->clearChilds();
	}
}

//same as tjs
//a=%[name:value,name2:value2,...]
//a=%[strexp1=>value, strexp2=>value2]
void Parser::nud_dic(BKE_bytree** tree)
{
	BUILD_TREE;
	bkplong p = token.pos;
	if (next.opcode == OP_ARR2)
	{
		readToken();
		tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
		tr->Node.var.makeVar();
		tr->Node.var=BKE_Variable::dic();
		tr->Node.var.makeConst();
		return;
	}
	tr->addChild();
	bool cancal=true;
	do
	{
		if (next.opcode != OP_COMMA)
		{
			expression(&tr->childs.back(), pre[OP_DIC] - 1);
			if (tr->childs.back() == NULL)
				continue;
			if (next.opcode == OP_MAOHAO)
			{
				BKE_Node &tmp = tr->childs.back()->Node;
				if (tmp.opcode != OP_LITERAL + OP_COUNT)
				{
					THROW(L"语法错误，:前必须接合法的变量名", tr->childs.back()->Node.pos);
				}
				tmp.opcode = OP_CONSTVAR + OP_COUNT;
			}
			else if (next.opcode == OP_VALUE)
			{
				if (tr->childs.back()->Node.opcode != OP_CONSTVAR + OP_COUNT)
					cancal = false;
			}
			else
			{
				THROW(L"语法错误，这里需要=>或:", next.pos);
			}
			readToken();
			GET_TREE2(pre[OP_DIC] - 1);
			if (tr->childs.back()->Node.opcode != OP_CONSTVAR + OP_COUNT)
				cancal = false;
			tr->addChild();
			//banned
			//else
			//{
			//	BKE_bytree *tmp=new BKE_bytree();
			//	tmp->Node.type=BKE_Node::LITERAL;
			//	tmp->Node.var=1;
			//	tr->childs.push_back(tmp);
			//}
		}
		if (next.opcode != OP_COMMA)
			break;
		readToken();
	} while (next.opcode != OP_ARR2);
	if (next.opcode != OP_ARR2)
	{
		THROW(getPos(p) + L"%[需要]结尾。", next.pos);
	};
	readToken();
	if(cancal)
	{
		BKE_array<BKE_Variable> tmpvars;
		tr->Node.var.forceSet(innerRun(tr, this->p, tmpvars));
		tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
		tr->Node.var.makeConst();
		tr->clearChilds();
	}
}

void Parser::nud_block(BKE_bytree** tree)
{
	BUILD_TREE;
	//bool cancal=true;
	bkplong p = token.pos;
	//try
	//{
		while (next.opcode != OP_BLOCK2)
		{
			GET_TREE_OR_NULL;
			//if (cancal && tr->childs.back() && tr->childs.back()->Node.opcode != OP_LITERAL + OP_COUNT)
			//	cancal = false;
			if (next.opcode == OP_STOP)
			{
				readToken();
			}
			if (next.opcode == OP_END)
				break;
		}
//	}
//	catch (Var_Except &e)
//	{
//#if PARSER_DEBUG
//		bkplong pos;
//		if (!e.hasPos())
//			pos = runpos;
//		else
//			pos = e.getPos();
//		auto it = upper_bound(linestartpos.begin(), linestartpos.end(), pos);
//		if (it == linestartpos.end())
//			e.lineinfo = exp+(*(--it));
//		else
//		{
//			e.lineinfo = wstring(exp + (*(it - 1)), (*it) - *(it - 1));
//			it--;
//		}
//		e.addLine(it - linestartpos.begin() + 1);
//		e.addPos(pos - *it + 1);
//		errorstack.push_back(e);
//#endif
//		e.setMsg(e.getMsg() + L" " + getPos(p) + L"{需要}结尾。");
//		throw e;
//	}
	if (next.opcode != OP_BLOCK2)
	{
		THROW(getPos(p) + L"{需要}结尾。", next.pos);
	}
	//override optimization introduce many NULL error

	//if(cancal)
	//{
	//	if(tr->childs.empty() || tr->childs.back()==NULL)
	//	{
	//		//end with ;
	//		tr->release();
	//		tr=NULL;
	//		readToken();
	//		forcequit=true;
	//		return;
	//	}
	//	tr->Node.var=tr->childs.back()->Node.var;
	//	tr->Node.var.makeConst();
	//	tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
	//	tr->clearChilds();
	//}
	readToken();
	forcequit = true;
}

void Parser::nud_label(BKE_bytree** tree)
{
	BUILD_TREE;
	if (next.opcode != OP_LITERAL)
	{
		switch (next.opcode)
		{
		case OP_VAR:
		case OP_DELETE:
		case OP_EXTENDS:
		case OP_CONTINUE:
		case OP_BREAK:
		case OP_RETURN:
		case OP_IN:
		case OP_ELSE:
		case OP_THEN:
		case OP_IF:
		case OP_FOR:
		case OP_WHILE:
		case OP_DO:
		case OP_FOREACH:
		case OP_FUNCTION:
		case OP_CLASS:
		case OP_PROPGET:
		case OP_PROPSET:
		case OP_TRY:
		case OP_CATCH:
		case OP_THROW:
		case OP_GLOBAL:
		case OP_THIS:
		case OP_SUPER:
		case OP_TOINT:
		case OP_TOSTRING:
		case OP_TONUMBER:
		case OP_TYPE:
		case OP_CONST:
		case OP_INSTANCE:
		case OP_INCONTEXTOF:
		case OP_STATIC:
			next.var = OP_CODE[next.opcode];
			break;
		default:
			{
				THROW(L"语法错误，*后面只能直接加合法的变量名", next.pos);
			}
		}
	};
	tr->Node.var = L"*" + next.var.asBKEStr().getConstStr();
	tr->Node.var.makeConst();
	tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
	readToken();
}

//for(exp1;exp2;exp3)exp4
//for(var xx in xxx)exp
void Parser::nud_for(BKE_bytree** tree)
{
	BUILD_TREE;
	if (next.opcode != OP_BRACKET)
	{
		tr->Node.opcode = OP_QUICKFOR + OP_COUNT;
		if (next.opcode != OP_LITERAL)
		{
			THROW(L"此处需要是个变量名。", next.pos);
		}
		tr->childs.push_back(new BKE_bytree(next));
		readToken();
		if (next.opcode != OP_SET)
		{
			THROW(L"此处需要=", next.pos);
		}
		readToken();
		GET_TREE;
		if (next.opcode != OP_COMMA)
		{
			THROW(L"此处需要,", next.pos);
		}
		readToken();
		GET_TREE;
		if (next.opcode == OP_COMMA)
		{
			readToken();
			GET_TREE;
		}
		else
			tr->addChild();
		if (next.opcode == OP_BLOCK)
		{
			GET_TREE_OR_NULL;
		}
		else
		{
			THROW(L"此处需要{", next.pos);
		}
		forcequit = true;
		return;
	}
	//normal for
	readToken();
	GET_TREE_OR_NULL;
	if (next.opcode != OP_STOP)
	{
		THROW(L"语法错误，此处应为;", next.pos);
	};
	readToken();
	GET_TREE_OR_NULL;
	if (next.opcode != OP_STOP)
	{
		THROW(L"语法错误，此处应为;", next.pos);
	};
	readToken();
	GET_TREE2_OR_NULL(pre[OP_BRACKET] - 1);
	if (next.opcode != OP_BRACKET2)
	{
		THROW(L"语法错误，此处应为)", next.pos);
	};
	readToken();
	GET_TREE_OR_NULL;
	forcequit = true;
	//no optimization for "for" sentence
}

//foreach x,y in yyy{exp}
void Parser::nud_foreach(BKE_bytree** tree)
{
	BUILD_TREE;
	if (next.opcode == OP_LITERAL)
	{
		next.opcode += OP_COUNT;
		tr->childs.push_back(new BKE_bytree(next));
		readToken();
	}
	else
		tr->addChild();
	if (next.opcode == OP_COMMA)
	{
		readToken();
		if (next.opcode == OP_LITERAL)
		{
			next.opcode += OP_COUNT;
			tr->childs.push_back(new BKE_bytree(next));
			readToken();
		}
		else
		{
			THROW(L"此处需要变量", next.pos);
		}
	}
	else
	{
		if (tr->childs[0] == NULL)
		{
			THROW(L"此处需要变量或逗号", next.pos);
		}
		else
		{
			tr->addChild();
		}
	}
	if (next.opcode != OP_IN)
	{
		THROW(L"此处需要in", next.pos);
	}
	readToken();
	GET_TREE;
	if (next.opcode != OP_BLOCK)
	{
		THROW(L"语法错误，需要{", next.pos);
	};
	//readToken();
	GET_TREE_OR_NULL;
	//has been done in OP_BLOCK
	forcequit = true;
	//no optimization for "foreach" sentence
}

//do xxxx while yyy;
void Parser::nud_do(BKE_bytree** tree)
{
	BUILD_TREE;
	GET_TREE2_OR_NULL(pre[OP_DO]-1);
	if(next.opcode == OP_STOP)
		readToken();
	if (next.opcode != OP_WHILE)
	{
		THROW(L"语法错误，此处需要while", next.pos);
	};
	readToken();
	GET_TREE;
	forcequit = true;
	//no optimization for "do" sentence
}

//while xxxx { yyyy };
void Parser::nud_while(BKE_bytree** tree)
{
	BUILD_TREE;
	GET_TREE2(pre[OP_WHILE]-1);
	if(next.opcode == OP_STOP)
		readToken();
	if (next.opcode != OP_BLOCK)
	{
		THROW(L"语法错误，此处需要{", next.pos);
	};
	GET_TREE_OR_NULL;
	if(tr->childs[0]->Node.opcode != OP_CONSTVAR + OP_COUNT && (bool)tr->childs[0]->Node.var==false)
	{
		tr->release();
		*tree=NULL;
	}
	forcequit = true;
}

void Parser::nud_continue(BKE_bytree** tree)
{
	BUILD_TREE;
	if (next.opcode != OP_STOP)
	{
		THROW(L"语法错误,continue或break后必须接;", next.pos);
	};
	readToken();
	forcequit = true;
}

void Parser::nud_return(BKE_bytree** tree)
{
	BUILD_TREE;
	tr->addChild();
	if (next.opcode == OP_STOP)
	{
		forcequit = true;
		return;
	}
	expression(&tr->childs.back());
	if (next.opcode != OP_STOP)
	{
		THROW(L"语法错误,需要;", next.pos);
	}
	readToken();
	forcequit = true;
}

void Parser::nud_var(BKE_bytree** tree)
{
	BUILD_TREE;
	if (next.opcode != OP_LITERAL)
	{
		THROW(L"var后必须接合法的变量名", next.pos);
	}
	tr->addChild();
	tr->childs.back()=new BKE_bytree(next);
	tr->addChild();
	readToken();
	while (1)
	{
		if (next.opcode == OP_STOP)
			return;
		if (next.opcode == OP_END)
			return;
		if (next.opcode == OP_SET)
		{
			readToken();
			expression(&tr->childs.back());
			if (tr->childs.back() == NULL)
			{
				THROW(L"=后不能接空语句", next.pos);
			}
			continue;
		}
		if (next.opcode == OP_COMMA)
		{
			readToken();
			if (next.opcode != OP_LITERAL)
			{
				THROW(L"var后必须接合法的变量名", next.pos);
			}
			tr->addChild();
			tr->childs.back() = new BKE_bytree(next);
			tr->addChild();
			readToken();
			continue;
		}
		THROW(L"语法错误，只能接等号，逗号或分号", next.pos);
	}
	if (next.opcode != OP_STOP)
	{
		THROW(L"必须要接;", next.pos);
	}
	forcequit = true;
	//readToken();
	//if(next.opcode == OP_STOP)
	//	return;
	//if(next.opcode==OP_SET)
	//{
	//	readToken();
	//	expression(&tr->childs.back());
	//	return;
	//}
	//THROW(L"语法错误，只能接=或;", next.pos);
}

void Parser::nud_delete(BKE_bytree** tree)
{
	BUILD_TREE;
	GET_TREE;
	if (!isVar(tr->childs[0]))
	{
		THROW(L"delete后必须是个变量", next.pos);
	}
	if (next.opcode != OP_STOP)
	{
		THROW(L"语法错误,需要;", next.pos);
	};
	readToken();
	forcequit = true;
}

void Parser::nud_this(BKE_bytree** tree)
{
	BUILD_TREE;
	//this tree doesn't have any child
}

//function xxx(a,b,...)
//function(a,b,..)
//function(a=2,b=false,..)
//function xxx(*a)
void Parser::nud_function(BKE_bytree** tree)
{
	BUILD_TREE;
	BKE_String name;
	if(next.opcode == OP_LITERAL)
	{
		name = next.var.asBKEStr();
		readToken();
	}
	if (next.opcode != OP_BRACKET)
	{
		THROW(L"语法错误，需要(", next.pos);
	};
	//param sub-tree
	readToken();
	vector<BKE_String> paramname;
	BKE_hashmap<BKE_String, BKE_Variable, 4> initials;
	if(next.opcode!=OP_BRACKET2)
	{
		do
		{
			if(next.opcode==OP_LITERAL)
			{
				paramname.push_back(next.var);
			}
			else if(next.opcode==OP_MUL)
			{
				readToken();
				if (next.opcode == OP_LITERAL)
				{
					paramname.push_back(L"*" + next.var.asBKEStr().getConstStr());
				}
				else
				{
					THROW(L"不合法的参数名", next.pos);
				}
			}
			else
			{
				THROW(L"不合法的参数名", next.pos);
			}
			readToken();
			if (next.opcode == OP_SET)
			{
				readToken();
				BKE_bytree *_tr = NULL;
				try
				{
					expression(&_tr);
					if (_tr->Node.opcode != OP_CONSTVAR + OP_COUNT)
					{
						THROW(L"缺省值只能是常数", next.pos);
					}
					initials[paramname.back()] = _tr->Node.var;
					_tr->release();
				}
				catch (Var_Except &e)
				{
					if (_tr)
						_tr->release();
					throw e;
				}
			}
			if (next.opcode != OP_COMMA)
				break;
			readToken();
		}while(1);
	}
	if (next.opcode != OP_BRACKET2)
	{
		THROW(L"语法错误，需要)", next.pos);
	};
	readToken();
	if (next.opcode != OP_BLOCK)
	{
		THROW(L"语法错误，需要{", next.pos);
	}
	BKE_bytree *tr2 = NULL;
#if PARSER_DEBUG
	bkplong startpos = next.pos;
#endif
	expression(&tr2);
	if (tr2)
	{
		tr2->Node.opcode = OP_RESERVE + OP_COUNT;
	}
	BKE_VarFunction *func = new BKE_VarFunction(tr2);
#if PARSER_DEBUG
	func->name = tr->Node.var.asBKEStr();
	func->rawexp = wstring(exp + startpos, next.pos - 1 - startpos);
	func->linestartpos.push_back(startpos);
	while (startpos < curpos - exp)
	{
		if (exp[startpos] == '\r')
		{
			startpos++;
			if (exp[startpos] == '\n')
				startpos++;
			func->linestartpos.push_back(startpos);
		}
		else if (exp[startpos] == '\n')
		{
			startpos++;
			func->linestartpos.push_back(startpos);
		}
		else
			startpos++;
	}
#endif
	if (tr2)
		tr2->release();
	func->paramnames = std::move(paramname);
	func->initials = std::move(initials);
	tr->Node.var = func;
	((BKE_VarFunction*)tr->Node.var.obj)->name = name;
	forcequit = true;
}

//propget xxx(){}
void Parser::nud_propget(BKE_bytree **tree)
{
	BUILD_TREE;
	if (next.opcode == OP_LITERAL)
	{
		tr->childs.push_back(new BKE_bytree(next));
		readToken();
	}
	else
	{
		THROW(L"propget必须要有名称", next.pos);
	}
	if (next.opcode != OP_BRACKET)
	{
		THROW(L"语法错误，需要(", next.pos);
	};
	readToken();
	if (next.opcode != OP_BRACKET2)
	{
		THROW(L"语法错误，需要)", next.pos);
	};
	readToken();
	GET_TREE_OR_NULL;
}

//propset xxx(a){}
void Parser::nud_propset(BKE_bytree **tree)
{
	BUILD_TREE;
	if (next.opcode == OP_LITERAL)
	{
		tr->childs.push_back(new BKE_bytree(next));
		readToken();
	}
	else
	{
		THROW(L"propset必须要有名称", next.pos);
	}
	if (next.opcode != OP_BRACKET)
	{
		THROW(L"语法错误，需要(", next.pos);
	};
	readToken();
	if (next.opcode == OP_LITERAL)
	{
		tr->childs.push_back(new BKE_bytree(next));
		readToken();
	}
	else
	{
		THROW(L"propset有且只能有一个参数", next.pos);
	}
	if (next.opcode != OP_BRACKET2)
	{
		THROW(L"语法错误，需要)", next.pos);
	};
	readToken();
	GET_TREE_OR_NULL;
}

//class xxx [extends yyy]{};
void Parser::nud_class(BKE_bytree** tree)
{
	BUILD_TREE;
	if (next.opcode != OP_LITERAL)
	{
		THROW(L"语法错误，只能接类名", next.pos);
	};
	tr->childs.push_back(new BKE_bytree(next));
	readToken();
	if (next.opcode == OP_EXTENDS)
	{
		readToken();
		if (next.opcode != OP_LITERAL)
		{
			THROW(L"语法错误，此处只能接类名", next.pos);
		};
		tr->childs.push_back(new BKE_bytree(next));
		readToken();
		while (next.opcode == OP_COMMA)
		{
			readToken();
			if (next.opcode != OP_LITERAL)
			{
				THROW(L"语法错误，此处只能接类名", next.pos);
			};
			tr->childs.push_back(new BKE_bytree(next));
			readToken();
		}
	}
	if (next.opcode != OP_BLOCK)
	{
		THROW(L"语法错误，需要{", next.pos);
	};
	bkplong p = next.pos;
	//read block manually
	readToken();
	while (next.opcode != OP_BLOCK2)
	{
		GET_TREE;
		//check var, function, propget, propset
		switch (tr->childs.back()->Node.opcode)
		{
		case OP_STATIC + OP_COUNT:
		case OP_VAR + OP_COUNT:
		case OP_FUNCTION + OP_COUNT:
		case OP_PROPGET + OP_COUNT:
		case OP_PROPSET + OP_COUNT:
			break;
		default:
			{THROW(L"语法错误，只能接static, var，function，propget或propset子句", next.pos); };
		}
		if (next.opcode == OP_STOP)
		{
			readToken();
			continue;
		}
	}
	if (next.opcode != OP_BLOCK2)
	{
		THROW(bkpInt2Str(p) + L"处的{需要}结尾", next.pos);
	}
	readToken();
	forcequit = true;
}

//only used in ++ and --
//require left must be a variable
void Parser::led_one(BKE_bytree** tree)
{
	//if((*tree)->Node.type!=BKE_Node::NAME)
	//	{THROW(L"++和--前面必须为变量", next.pos);};
	if (!isVar(*tree))
	{
		THROW(L"++和--前面必须为变量", next.pos);
	};
	(*tree) = (*tree)->addParent(token);
}

void Parser::led_choose(BKE_bytree** tree)
{
	bkplong op = token.opcode;
	(*tree) = (*tree)->addParent(token);
	BKE_bytree* &tr = *tree;
	GET_TREE;
	if (next.opcode != OP_MAOHAO)
	{
		THROW(L"此处需要冒号:", next.pos);
	}
	readToken();
	GET_TREE;
	if (tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT)
	{
		bool cond = (bool)tr->childs[0]->Node.var;
		BKE_bytree *res;
		if (cond)
		{
			res = tr->childs[1];
			tr->childs[1] = NULL;
			tr->release();
			(*tree) = res;
		}
		else
		{
			res = tr->childs[2];
			tr->childs[2] = NULL;
			tr->release();
			(*tree) = res;
		}
	}
}

//void Parser::led_literal(BKE_bytree **tree)
//{
//	if((*tree)->Node.opcode != OP_CONSTVAR + OP_COUNT && (*tree)->Node.var.getType()==VAR_STR && token.var.getType()==VAR_STR)
//	{
//		(*tree)->Node.var.makeVar();
//		(*tree)->Node.var+=token.var;
//		(*tree)->Node.var.makeConst();
//	}
//	else
//	{
//		THROW(L"语法错误，只有字符串可以直接连接", token.pos);
//	}
//}

void Parser::led_two(BKE_bytree** tree)
{
	bkplong op=token.opcode;
	(*tree)=(*tree)->addParent(token);
	BKE_bytree* &tr=*tree;
	GET_TREE2(lbp[op]);
	if(tr->childs[1]->Node.opcode == OP_CONSTVAR + OP_COUNT && tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT && runner[op]!=NULL)
	{
		BKE_array<BKE_Variable> tmpvars;
		tr->Node.var.forceSet(innerRun(tr, this->p, tmpvars));
		tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
		tr->clearChilds();
	}
}

void Parser::led_mul(BKE_bytree** tree)
{
	bkplong op = token.opcode;
	(*tree) = (*tree)->addParent(token);
	BKE_bytree* &tr = *tree;
	if (next.opcode != OP_LITERAL)
	{
		auto it = opmap.find(next.var.asBKEStr().getConstStr());
		if (it != opmap.end())
			next.opcode = it->second;
	}
	GET_TREE2(lbp[op]);
	if (tr->childs[1]->Node.opcode == OP_CONSTVAR + OP_COUNT && tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT && runner[op] != NULL)
	{
		BKE_array<BKE_Variable> tmpvars;
		tr->Node.var.forceSet(innerRun(tr, this->p, tmpvars));
		tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
		tr->clearChilds();
	}
}

void Parser::led_pow(BKE_bytree** tree)
{
	bkplong op=token.opcode;
	(*tree)=(*tree)->addParent(token);
	BKE_bytree* &tr=*tree;
	GET_TREE2(lbp[op]-1);
	if(tr->childs[1]->Node.opcode == OP_CONSTVAR + OP_COUNT && tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT && led[op]!=NULL)
	{
		BKE_array<BKE_Variable> tmpvars;
		tr->Node.var.forceSet(innerRun(tr, this->p, tmpvars));
		tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
		tr->clearChilds();
	}
}

void Parser::led_dot(BKE_bytree** tree)
{
	//left don't need to be a NAME because we enable syntax such as "233".type
	if (next.opcode != OP_LITERAL)
	{
#if PARSER_DEBUG>=2
	o_stderr << L"after dot:" << next.opcode << L" " << OP_CODE[next.opcode];
#endif
		THROW(L"操作数必须是合法的变量名", next.pos);
	};
	(*tree)=(*tree)->addParent(token);
	next.opcode += OP_COUNT;
	BKE_bytree *right=new BKE_bytree(next);
	(*tree)->childs.push_back(right);
	readToken();
}

void Parser::led_ele(BKE_bytree** tree)
{
	bkplong op = token.opcode;
	(*tree) = (*tree)->addParent(token);
	BKE_bytree* &tr = *tree;
	tr->addChild();
	while (next.opcode != OP_ARR2)
	{
		if (next.opcode == OP_STOP)
		{
			readToken();
			tr->addChild();
		}
		else
		{
			expression(&tr->childs.back());
			if (next.opcode != OP_MAOHAO && next.opcode != OP_ARR2)
			{
				THROW(L"语法错误，期望:或]", next.pos);
			}
			if (next.opcode == OP_MAOHAO)
			{
				readToken();
				tr->addChild();
			}
		}
	}
	if (next.opcode != OP_ARR2)
	{
		THROW(L"中括号不匹配", next.pos);
	}
	if (tr->childs[1] == NULL && tr->childs.size() == 2)
	{
		THROW(L"中括号里不能为空", next.pos);
	}
	if (tr->childs.size() > 4)
	{
		THROW(L"中括号里项数太多。", next.pos);
	}
	readToken();
}

void Parser::led_set(BKE_bytree** tree)
{
	bkplong op=token.opcode;
	(*tree)=(*tree)->addParent(token);
	BKE_bytree* &tr=*tree;
	if (!isVar(tr->childs[0]))
	{
		THROW(L"等号左边必须是变量", token.pos);
	}
	GET_TREE2(lbp[op]-1);	
}

void Parser::led_param(BKE_bytree** tree)
{
	(*tree)=(*tree)->addParent(token);
	BKE_bytree* &tr=*tree;
	while (next.opcode != OP_BRACKET2)
	{
		GET_TREE2_OR_NULL(pre[OP_BRACKET] - 1);
		if (next.opcode == OP_COMMA)
			readToken();
		else
			break;
	}
	if (next.opcode != OP_BRACKET2)
	{
		THROW(L"语法错误，需要)", next.pos);
	};
	//no optimization for function call
	readToken();
}

NUD_FUNC(preadd)
{
	GETLEFTVAR;
	RECORDPOS;
	QUICKRETURNCONST(leftvar.asNumber());
}

NUD_FUNC(presub)
{
	GETLEFTVAR;
	RECORDPOS;
	QUICKRETURNCONST(-leftvar.asNumber());
}

NUD_FUNC(not)
{
	GETLEFTVAR;
	RECORDPOS;
	QUICKRETURNCONST(!leftvar);
}

NUD_FUNC(array)
{
	RECORDPOS;
	BKE_VarArray *vararray = new BKE_VarArray();
	int i = -1, s = tree->childs.size();
	while(++i < s - 1)
	{
		BKE_Variable var=run(tree->childs[i], _this);
		vararray->pushMember(var);
	}
	QUICKRETURNCONST(vararray);
}

NUD_FUNC(dic)
{
	RECORDPOS;
	BKE_VarDic *vardic = new BKE_VarDic();
	int i = 0;
	while(tree->childs[i]!=NULL)
	{
		vardic->setMember(innerRun(tree->childs[i], _this, tmpvars), run(tree->childs[i+1], _this));
		i+=2;
	}
	QUICKRETURNCONST(vardic);
}

NUD_FUNC(block)
{
	RECORDPOS;
	BKE_VarClosure *tmp = new BKE_VarClosure(_this);
	BKE_Variable temp=tmp;
	if(tree->childs.empty())
	{
		return none;
	}
	int i=0,size=tree->childs.size();
	for(;i<size;i++)
		innerRun(tree->childs[i], tmp, tmpvars);
	RETURNNULL;
}

NUD_FUNC(specialblock)
{
	RECORDPOS;
	if (tree->childs.empty())
	{
		return none;
	}
	int i=0,size=tree->childs.size();
	for(;i<size;i++)
		innerRun(tree->childs[i], _this, tmpvars);
	RETURNNULL;
}

NUD_FUNC(inc)
{
	GETLEFTVAR;
	RECORDPOS;
	return ++leftvar;
}

NUD_FUNC(dec)
{
	GETLEFTVAR;
	RECORDPOS;
	return --leftvar;
}

NUD_FUNC(for)
{
	BKE_VarClosure *tmp = new BKE_VarClosure(_this);
	BKE_VarObjectAutoReleaser ar(tmp);
	innerRun(tree->childs[0], tmp, tmpvars);
	while ((bool)innerRun(tree->childs[1], tmp, tmpvars))
	{
		try
		{
			BKE_array<BKE_Variable> _tmpvars;
			innerRun(tree->childs[3], tmp, _tmpvars);
		}
		catch (Special_Except &e)
		{
			if (e.type == Special_Except::BREAK)
				break;
			else if (e.type != Special_Except::CONTINUE)
				throw e;
		}
		innerRun(tree->childs[2], tmp, tmpvars);
	}
	RETURNNULL;
}

NUD_FUNC(forin)
{
	BKE_Number start = innerRun(tree->childs[1], _this, tmpvars).asBKENum();
	BKE_Number stop = innerRun(tree->childs[2], _this, tmpvars).asBKENum();
	BKE_Number step = tree->childs[3] == NULL ? BKE_Number(start > stop ? -1 : 1) : innerRun(tree->childs[3], _this, tmpvars).asBKENum();
	if (step.zero())
		throw Var_Except(L"for的步长不能为0。");
	BKE_VarClosure *tmp = new BKE_VarClosure(_this);
	BKE_VarObjectAutoReleaser ar(tmp);
	BKE_String name = tree->childs[0]->Node.var.asBKEStr();
	BKE_Variable &var = tmp->varmap[name];
	tree->childs[4]->Node.opcode = OP_RESERVE + OP_COUNT;
	while (1)
	{
		var = start;
		if (start > stop && step > BKE_Number(0))
			break;
		if (start < stop && step < BKE_Number(0))
			break;
		try
		{
			BKE_array<BKE_Variable> _tmpvars;
			innerRun(tree->childs[4], tmp, _tmpvars);
		}
		catch (Special_Except &e)
		{
			if (e.type == Special_Except::BREAK)
				break;
			else if (e.type == Special_Except::CONTINUE)
			{
				start += step;
				continue;
			}
			else
				throw e;
		}
		start += step;
	}
	RETURNNULL;
}

NUD_FUNC(foreach)
{
	RECORDPOS;
	BKE_VarClosure *tmp = new BKE_VarClosure(_this);
	BKE_String n1, n2;
	if (tree->childs[0])
		n1 = tree->childs[0]->Node.var.asBKEStr();
	if (tree->childs[1])
		n2 = tree->childs[1]->Node.var.asBKEStr();
	//the variable after "foreach" is seen as a local variable
	BKE_VarObjectAutoReleaser ar(tmp);
	//tmp->release();
	BKE_Variable clo = run(tree->childs[2], _this).clone();
	if (!tree->childs[3])
		RETURNNULL;
	tree->childs[3]->Node.opcode = OP_RESERVE + OP_COUNT;
	switch (clo.getType())
	{
		case VAR_ARRAY:
		{
			auto s = ((BKE_VarArray*)clo.obj)->getCount();
			for(int i=0;i<s;i++)
			{
				if (!n1.isVoid())
					tmp->setMember(n1, i);
				if (!n2.isVoid())
					tmp->setMember(n2, ((BKE_VarArray*)clo.obj)->vararray[i]);
				try
				{
					BKE_array<BKE_Variable> _tmpvars;
					innerRun(tree->childs[3], tmp, _tmpvars);
				}
				catch(Special_Except &e)
				{
					if(e.type==Special_Except::BREAK)
						break;
					else if(e.type==Special_Except::CONTINUE)
						continue;
					else
						throw e;
				}
			}
		}
		break;
		case VAR_DIC:
		{
			auto it=((BKE_VarDic*)clo.obj)->varmap.begin();
			for(;it!=((BKE_VarDic*)clo.obj)->varmap.end();it++)
			{
				if (!n1.isVoid())
					tmp->setMember(n1, it->first);
				if (!n2.isVoid())
					tmp->setMember(n2, it->second);
				try
				{
					BKE_array<BKE_Variable> _tmpvars;
					innerRun(tree->childs[3], tmp, _tmpvars);
				}
				catch(Special_Except &e)
				{
					if(e.type==Special_Except::BREAK)
						break;
					else if(e.type==Special_Except::CONTINUE)
						continue;
					else
						throw e;
				}
			}
		}
		break;
	}
	RETURNNULL;
}

NUD_FUNC(if)
{
	if ((bool)innerRun(tree->childs[0], _this, tmpvars))
	{
		return (innerRun(tree->childs[1], _this, tmpvars));
	}
	else
	{
		return (innerRun(tree->childs[2], _this, tmpvars));
	}
}

NUD_FUNC(do)
{
	BKE_bytree *&tr1 = tree->childs[0];
	BKE_bytree *&tr2 = tree->childs[1];
	do
	{
		try
		{
			BKE_array<BKE_Variable> tmp;
			innerRun(tr1, _this, tmp);
		}
		catch(Special_Except &e)
		{
			if(e.type==Special_Except::BREAK)
				break;
			else if(e.type==Special_Except::CONTINUE)
				continue;
			else
				throw e;
		}
	}while((bool)run(tr2, _this));
	RETURNNULL;
}

NUD_FUNC(while)
{
	BKE_bytree *&tr1 = tree->childs[0];
	BKE_bytree *&tr2 = tree->childs[1];
	while ((bool)run(tr1, _this))
	{
		try
		{
			BKE_array<BKE_Variable> tmp;
			innerRun(tr2, _this, tmp);
		}
		catch(Special_Except &e)
		{
			if(e.type==Special_Except::BREAK)
				break;
			else if(e.type==Special_Except::CONTINUE)
				continue;
			else
				throw e;
		}
	}
	RETURNNULL;
}

NUD_FUNC(continue)
{
	RECORDPOS;
	throw Special_Except(Special_Except::CONTINUE);
}

NUD_FUNC(break)
{
	RECORDPOS;
	throw Special_Except(Special_Except::BREAK);
}

NUD_FUNC(return)
{
	GETLEFTVAR;
	RECORDPOS;
	throw Special_Except(Special_Except::RETURN, leftvar);
}

NUD_FUNC(literal)
{
	if (tree->Node.opcode == OP_CONSTVAR + OP_COUNT)
	{
		return tree->Node.var;
	}
	RECORDPOS;
	return _this->getMember(tree->Node.var.asBKEStr());
}

NUD_FUNC(propget)
{
	RECORDPOS;
	BKE_Variable &var = _this->varmap[tree->childs[0]->Node.var.asBKEStr()];
	if (var.getType() != VAR_PROP)
	{
		var = new BKE_VarProp(NULL);
	}
	static_cast<BKE_VarProp*>(var.obj)->addPropGet(tree->childs[1]);
	RETURNNULL;
}

NUD_FUNC(propset)
{
	RECORDPOS;
	BKE_Variable &var = _this->varmap[tree->childs[0]->Node.var.asBKEStr()];
	if (var.getType() != VAR_PROP)
	{
		var = new BKE_VarProp(NULL);
	}
	static_cast<BKE_VarProp*>(var.obj)->addPropSet(tree->childs[1]->Node.var.asBKEStr(), tree->childs[2]);
	RETURNNULL;
}

NUD_FUNC(function)
{
	RECORDPOS;
	((BKE_VarFunction*)tree->Node.var.obj)->setClosure(_this);
	if (!tree->Node.var.isVoid())
	{
		return /*BKE_VarClosure::Global()*/_this->setMember(((BKE_VarFunction*)tree->Node.var.obj)->name, tree->Node.var);
	}
	return tree->Node.var;
}

NUD_FUNC(this)
{
	RECORDPOS;
	throw Var_Except(L"此处单独使用this无效");
}

NUD_FUNC(super)
{
	RECORDPOS;
	throw Var_Except(L"此处单独使用super无效");
}

NUD_FUNC(var)
{
	for (int i = 0; i < tree->childs.size(); i += 2)
	{
		BKE_Variable &v = innerRun(tree->childs[i + 1], _this, tmpvars);
		RECORDPOS;
		if (v.getType() != VAR_PROP)
			_this->setMember(tree->childs[i]->Node.var.asBKEStr(), v);
		else
			_this->setMember(tree->childs[i]->Node.var.asBKEStr(), ((BKE_VarProp*)v.obj)->get());
	}
	RETURNNULL;
}

NUD_FUNC(delete)
{
	RECORDPOS;
	BKE_bytree *tr = tree->childs[0];
	BKE_Node &node=tr->Node;
	runpos = node.pos;
	if(node.opcode == OP_LITERAL + OP_COUNT)
		_this->deleteMemberIndex(node.var.asBKEStr());
	else if(node.opcode==OP_DOT)
	{
		//only dic use dot
		const BKE_String &str = tr->childs[1]->Node.var.asBKEStr();
		if (tr->childs[0]->Node.opcode == OP_THIS + OP_COUNT)
		{
			runpos = node.pos;
			auto thisvar = _this->getThisClosure();
			if (!thisvar)
				throw Var_Except(L"this在当前环境下无意义");
			thisvar->deleteThisMember(str);
			RETURNNULL;
		}
		if (tr->childs[0]->Node.opcode == OP_SUPER + OP_COUNT)
		{
			runpos = node.pos;
			throw Var_Except(L"不能删除super的成员");
		}
		BKE_Variable leftvar = run(tr->childs[0], _this);
		switch (leftvar.getType())
		{
		case VAR_DIC:
			((BKE_VarDic *)leftvar.obj)->deleteMemberIndex(str);
			break;
		case VAR_CLO:
			((BKE_VarClosure*)leftvar.obj)->deleteMemberIndex(str);
			break;
		case VAR_CLASS:
			((BKE_VarClass*)leftvar.obj)->deleteThisMember(str);
			break;
		default:
			throw Var_Except(L"语法错误，无法取变量");
		}
	}
	else if(node.opcode==OP_ARRAY)
	{
		if (tr->childs.size()>2)
			throw Var_Except(L"语法错误，这里的中括号里只能有一项");
		//dic and array can use [
		BKE_Variable rightvar = run(tr->childs[1], _this);
		if (tr->childs[0]->Node.opcode == OP_THIS + OP_COUNT)
		{
			auto thisvar = _this->getThisClosure();
			runpos = node.pos;
			if (!thisvar)
				throw Var_Except(L"this在当前环境下无意义");
			thisvar->deleteThisMember(rightvar.asBKEStr());
			RETURNNULL;
		}
		if (tr->childs[0]->Node.opcode == OP_SUPER + OP_COUNT)
		{
			runpos = node.pos;
			throw Var_Except(L"不能删除super的成员");
		}
		BKE_Variable leftvar = run(tr->childs[0], _this);
		runpos = node.pos;
		if (leftvar.getType() != VAR_DIC && leftvar.getType() != VAR_ARRAY)
		{
			throw Var_Except(L"语法错误，[前面的变量必须可以取成员。");
		}
		switch (leftvar.getType())
		{
		case VAR_DIC:
			((BKE_VarDic *)leftvar.obj)->deleteMemberIndex(rightvar.asBKEStr());
			break;
		case VAR_CLO:
			((BKE_VarClosure*)leftvar.obj)->deleteMemberIndex(rightvar.asBKEStr());
			break;
		case VAR_CLASS:
			((BKE_VarClass*)leftvar.obj)->deleteThisMember(rightvar.asBKEStr());
			break;
		case VAR_ARRAY:
			((BKE_VarArray*)leftvar.obj)->deleteMemberIndex(rightvar.asInteger());
			break;
		default:
			throw Var_Except(L"语法错误，无法取变量");
		}
	}
	RETURNNULL;
}

NUD_FUNC(try)
{
	try
	{
		innerRun(tree->childs[0], _this, tmpvars);
	}
	catch (Var_Except &e)
	{
		_this->setMember(tree->childs[1]->Node.var.asBKEStr(), e.getMsg());
		innerRun(tree->childs[2], _this, tmpvars);
	}
	RETURNNULL;
}

NUD_FUNC(class)
{
	RECORDPOS;
	BKE_VarClass *cla;
	BKE_String n = tree->childs[0]->Node.var.asBKEStr();
	int i = 1;
	bkplong s = tree->childs.size();
	BKE_array<BKE_VarClass *> extends;
	while (i < s && tree->childs[i]->Node.opcode == OP_LITERAL)
	{
		BKE_bytree *tr2 = tree->childs[i];
		BKE_Variable &var = _this->getMember(tr2->Node.var.asBKEStr());
		if (var.getType() == VAR_CLASS)
			extends.push_back(static_cast<BKE_VarClass*>(var.obj));
		else
		{
			runpos = tr2->Node.pos;
			throw Var_Except(L"extends后接的" + tr2->Node.var.asBKEStr().getConstStr() + L"不是类名。");
		}
		i++;
	}
	if (extends.size() > 0)
		cla = new BKE_VarClass(n, extends, _this);
	else
		cla = new BKE_VarClass(n, _this);
	BKE_Variable claclo = cla;
	for (; i < s; i++)
	{
		BKE_bytree *&subtr = tree->childs[i];
		runpos = subtr->Node.pos;
		switch (subtr->Node.opcode)
		{
		case OP_STATIC + OP_COUNT:
			{
				bkplong ss = subtr->childs.size();
				for (bkplong j = 0; j < ss; j += 2)
					cla->setMember(subtr->childs[j]->Node.var.asBKEStr(), innerRun(subtr->childs[j + 1], cla, tmpvars));
			}
			break;
		case OP_VAR + OP_COUNT:
			{
				bkplong ss = subtr->childs.size();
				for (bkplong j = 0; j < ss; j += 2)
					cla->classvar[subtr->childs[j]->Node.var.asBKEStr()] = innerRun(subtr->childs[j + 1], cla, tmpvars);
			}
			break;
		case OP_FUNCTION + OP_COUNT:
			{
				((BKE_VarFunction*)subtr->Node.var.obj)->setClosure(cla);
				cla->setMember(((BKE_VarFunction*)subtr->Node.var.obj)->name, subtr->Node.var);
			}
			break;
		case OP_PROPGET + OP_COUNT:
			{
				BKE_Variable &var = cla->varmap[subtr->childs[0]->Node.var.asBKEStr()];
				if (var.getType() != VAR_PROP)
				{
					var = new BKE_VarProp(cla);
				}
				static_cast<BKE_VarProp*>(var.obj)->addPropGet(subtr->childs[1]);
				//((BKE_VarProp*)var.obj)->setClosure(cla);
			}
			break;
		case OP_PROPSET + OP_COUNT:
			{
				BKE_Variable &var = cla->varmap[subtr->childs[0]->Node.var.asBKEStr()];
				if (var.getType() != VAR_PROP)
				{
					var = new BKE_VarProp(cla);
				}
				static_cast<BKE_VarProp*>(var.obj)->addPropSet(subtr->childs[1]->Node.var.asBKEStr(), subtr->childs[2]);
				//((BKE_VarProp*)var.obj)->setClosure(cla);
			}
			break;
		}
	}
	_this->setConstMember(n, claclo);
	RETURNNULL;
}

//NUD_FUNC(local)
//{
//	BKE_bytree *&tr=tree->childs[0];
//	if (tr->Node.opcode == OP_LITERAL + OP_COUNT)
//		return _this->varmap[tr->Node.var.asBKEStr()];
//	BKE_Variable &name = innerRun(tr, _this, tmpvars);
//	RECORDPOS;
//	return _this->varmap[name.asBKEStr()];
//}
//
//NUD_FUNC(global)
//{
//	BKE_bytree *&tr = tree->childs[0];
//	if (tr->Node.opcode == OP_LITERAL + OP_COUNT)
//		return BKE_VarClosure::Global()->getMember(tr->Node.var.asBKEStr());
//	BKE_Variable &name = innerRun(tr, _this, tmpvars);
//	RECORDPOS;
//	return BKE_VarClosure::Global()->getMember(name.asBKEStr());
//}

NUD_FUNC(global2)
{
	QUICKRETURNCONST(BKE_VarClosure::global()->addRef());
}

NUD_FUNC(dot2)
{
	const BKE_String &str = tree->childs[0]->Node.var.asBKEStr();
	auto v = _this->getWithVar();
	if (!v)
		return BKE_VarClosure::global()->getMember(str);
	if (v->vt == VAR_CLASS)
		return ((BKE_VarClass*)v)->getClassMember(str);
	else
	{
		BKE_Variable vv = v->addRef();
		auto res = vv.dotFunc(str);
		if (res.isVoid())
			return ((BKE_VarDic*)v)->getMember(str);
		else
			QUICKRETURNCONST(res);
	}
}

NUD_FUNC(with)
{
	BKE_Variable v;
	auto rawwith = _this->withvar;
	if (tree->childs[0]->Node.opcode == OP_THIS + OP_COUNT)
	{
		auto thisvar = _this->getThisClosure();
		if(!thisvar)
			throw Var_Except(L"this在此处无定义");
		_this->withvar = thisvar;
	}
	else if (tree->childs[0]->Node.opcode == OP_SUPER + OP_COUNT)
	{
		throw Var_Except(L"with里面的变量不能是super");
	}
	else
	{
		v = run(tree->childs[0], _this);
		if (v.getType() != VAR_DIC && v.getType() != VAR_CLASS)
			throw Var_Except(L"with里面的变量必须是字典或类");
		_this->withvar = v.obj;
	}
	try
	{
		innerRun(tree->childs[1], _this, tmpvars);
		_this->withvar = rawwith;
		RETURNNULL;
	}
	catch (Var_Except &e)
	{
		_this->withvar = rawwith;
		throw e;
	}
}

NUD_FUNC(int)
{
	GETLEFTVAR;
	RECORDPOS;
	QUICKRETURNCONST((bkplong)leftvar);
}

NUD_FUNC(string)
{
	GETLEFTVAR;
	RECORDPOS;
	QUICKRETURNCONST(leftvar.asBKEStr());
}

NUD_FUNC(number)
{
	GETLEFTVAR;
	RECORDPOS;
	QUICKRETURNCONST(leftvar.asBKENum());
}

NUD_FUNC(typeof)
{
	GETLEFTVAR;
	RECORDPOS;
	QUICKRETURNCONST(leftvar.getTypeBKEString());
}

NUD_FUNC(const)
{
	return innerRun(tree->childs[0], _this, tmpvars);
}

LED_FUNC(add)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	if (leftvar.isTemp())
	{
		RECORDPOS;
		return (leftvar += rightvar);
	}
	QUICKRETURNCONST(leftvar+rightvar);
}

LED_FUNC(sub)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	if (leftvar.isTemp())
	{
		RECORDPOS;
		return (leftvar -= rightvar);
	}
	QUICKRETURNCONST(leftvar - rightvar);
}

LED_FUNC(mul)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	if (leftvar.isTemp())
	{
		RECORDPOS;
		return (leftvar *= rightvar);
	}
	QUICKRETURNCONST(leftvar*rightvar);
}

LED_FUNC(div)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	if (leftvar.isTemp())
	{
		RECORDPOS;
		return (leftvar /= rightvar);
	}
	QUICKRETURNCONST(leftvar / rightvar);
}

LED_FUNC(mod)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	if (leftvar.isTemp())
	{
		RECORDPOS;
		return (leftvar %= rightvar);
	}
	QUICKRETURNCONST(leftvar%rightvar);
}

LED_FUNC(pow)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	if (leftvar.isTemp())
	{
		RECORDPOS;
		return (leftvar ^= rightvar);
	}
	QUICKRETURNCONST(leftvar^rightvar);
}

LED_FUNC(equal)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	QUICKRETURNCONST(leftvar==rightvar);
}

LED_FUNC(eequal)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	QUICKRETURNCONST(leftvar.getType() == rightvar.getType() && leftvar == rightvar);
}

LED_FUNC(nequal)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	QUICKRETURNCONST(!(leftvar==rightvar));
}

LED_FUNC(nnequal)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	QUICKRETURNCONST(!(leftvar.getType() == rightvar.getType() && leftvar == rightvar));
}

LED_FUNC(larger)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	QUICKRETURNCONST(leftvar>rightvar);
}

LED_FUNC(smaller)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	QUICKRETURNCONST(leftvar<rightvar);
}

LED_FUNC(le)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	QUICKRETURNCONST(leftvar>=rightvar);
}

LED_FUNC(se)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	QUICKRETURNCONST(leftvar<=rightvar);
}

LED_FUNC(and)
{
	GETLEFTVAR;
	if(!leftvar)
	{
		QUICKRETURNCONST(0);
	}
	GETRIGHTVAR;
	QUICKRETURNCONST((bkplong)rightvar.asBoolean());
}

LED_FUNC(fastand)
{
	GETLEFTVAR;
	if(leftvar.isVoid())
	{
		return leftvar;
	}
	GETRIGHTVAR;
	return rightvar;
}

LED_FUNC(or)
{
	GETLEFTVAR;
	if(leftvar)
	{
		QUICKRETURNCONST(1);
	}
	GETRIGHTVAR;
	QUICKRETURNCONST((bkplong)rightvar.asBoolean());
}

LED_FUNC(fastor)
{
	GETLEFTVAR;
	if(!leftvar.isVoid())
	{
		return leftvar;
	}
	GETRIGHTVAR;
	return rightvar;
}

LED_FUNC(set)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	RECORDPOS;
	MUSTBEVAR(leftvar);
	CHECKCLO(rightvar);
	return (leftvar=rightvar);
}

LED_FUNC(setadd)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	RECORDPOS;
	MUSTBEVAR(leftvar);
	CHECKCLO(rightvar);
	return (leftvar += rightvar);
}

LED_FUNC(setsub)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	RECORDPOS;
	MUSTBEVAR(leftvar);
	CHECKCLO(rightvar);
	return (leftvar -= rightvar);
}

LED_FUNC(setmul)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	RECORDPOS;
	MUSTBEVAR(leftvar);
	CHECKCLO(rightvar);
	return (leftvar *= rightvar);
}

LED_FUNC(setdiv)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	RECORDPOS;
	MUSTBEVAR(leftvar);
	CHECKCLO(rightvar);
	return (leftvar /= rightvar);
}

LED_FUNC(setpow)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	RECORDPOS;
	MUSTBEVAR(leftvar);
	CHECKCLO(rightvar);
	return (leftvar ^= rightvar);
}

LED_FUNC(setmod)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	RECORDPOS;
	MUSTBEVAR(leftvar);
	CHECKCLO(rightvar);
	return (leftvar %= rightvar);
}

LED_FUNC(setset)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	RECORDPOS;
	MUSTBEVAR(leftvar);
	CHECKCLO(rightvar);
	if (leftvar.isVoid())
		leftvar = rightvar;
	return leftvar;
}

LED_FUNC(selfadd)
{
	GETLEFTVAR;
	RECORDPOS;
	MUSTBEVAR(leftvar);
	QUICKRETURNCONST(leftvar++);
}

LED_FUNC(selfsub)
{
	GETLEFTVAR;
	RECORDPOS;
	MUSTBEVAR(leftvar);
	QUICKRETURNCONST(leftvar--);
}

LED_FUNC(param)
{
	GETLEFTVAR;
	if(leftvar.getType()!=VAR_FUNC && leftvar.getType()!=VAR_CLASS)
	{
		runpos=tree->Node.pos;
		throw Var_Except(L"左操作数必须是函数");
	}
	try
	{
		if (leftvar.getType() == VAR_FUNC && !static_cast<BKE_VarFunction*>(leftvar.obj)->isNativeFunction())
		{
			QUICKRETURNCONST(static_cast<BKE_VarFunction*>(leftvar.obj)->run(tree, _this));
		}
		BKE_VarArray *arr=new BKE_VarArray();
		for(int i=1;i<(bkplong)tree->childs.size();i++)
			arr->pushMember(innerRun(tree->childs[i], _this, tmpvars));
		BKE_Variable params=arr;
		//arr->release();
		//may change value of next variable
		//so store it first
		runpos = tree->Node.pos;
		if (leftvar.getType() == VAR_FUNC)
		{
			tmpvars.push_back(static_cast<BKE_VarFunction*>(leftvar.obj)->run(arr));
		}
		else
		{
			tmpvars.push_back(static_cast<BKE_VarClass*>(leftvar.obj)->createInstance(arr));
		}
		tmpvars.back().is_var = TEMP_VAR;
		return tmpvars.back();
	}
	catch (Var_Except &e)
	{
		if (!e.hasPos())
			throw e;
		errorstack.push_back(e);
		runpos = tree->Node.pos;
		Var_Except ee(L"", runpos);
		throw ee;
	}
}

LED_FUNC(dot)
{
	auto tr = tree->childs[0];
	const BKE_String &str = tree->childs[1]->Node.var.asBKEStr();
	RECORDPOS;
	if (tr->Node.opcode == OP_THIS + OP_COUNT)
	{
		auto thisvar = _this->getThisClosure();
		if (!thisvar)
			throw Var_Except(L"this在当前环境下无定义");
		return thisvar->getClassMember(str);
	}
	if (tr->Node.opcode == OP_SUPER + OP_COUNT)
	{
		auto thisvar = _this->getThisClosure();
		if (!thisvar)
			throw Var_Except(L"super在当前环境下无定义");
		QUICKRETURNCONST(thisvar->getSuperMember(str));
	}
	GETLEFTVAR;
	CHECKEMPTY(1);
	RECORDPOS;
	if (leftvar.getType() == VAR_PROP)
	{
		tmpvars.push_back(((BKE_VarProp*)leftvar.obj)->get());
		auto v = tmpvars.back().dotFunc(str);
		if (v.isVoid())
			return tmpvars.back().dot(str);
		else
		{
			RECORDPOS;
			return tmpvars.push_back(v);
		}
	}
	auto v = leftvar.dotFunc(str);
	if (v.isVoid())
		return leftvar.dot(str);
	else
	{
		RECORDPOS;
		return tmpvars.push_back(v);
	}
}

LED_FUNC(ele)
{
	auto tr = tree->childs[0];
	RECORDPOS;
	if (tr->Node.opcode == OP_THIS + OP_COUNT)
	{
		auto thisvar = _this->getThisClosure();
		if (!thisvar)
			throw Var_Except(L"this在当前环境下无定义");
		if (tree->childs.size() == 2)
		{
			GETRIGHTVAR;
			return thisvar->getClassMember(rightvar.asBKEStr());
		}
		throw Var_Except(L"语法错误，中括号里只能有一项");
	}
	if (tr->Node.opcode == OP_SUPER + OP_COUNT)
	{
		auto thisvar = _this->getThisClosure();
		if (!thisvar)
			throw Var_Except(L"super在当前环境下无定义");
		if (tree->childs.size() == 2)
		{
			GETRIGHTVAR;
			QUICKRETURNCONST(thisvar->getSuperMember(rightvar.asBKEStr()));
		}
		throw Var_Except(L"语法错误，中括号里只能有一项");
	}
	GETLEFTVAR;
	RECORDPOS;
	if(tree->childs.size()==2)
	{
		GETRIGHTVAR;
		if (leftvar.getType() == VAR_STR)
		{
			//special handle to string
			//a="233", then a[2]=="3" is true, but cannot write a[2]="3"
			//that is, this cannot be set a new value
			wstring res;
			res += leftvar.str.getConstStr()[(bkplong)rightvar];
			QUICKRETURNCONST(res);
		}
		else
		{
			if (leftvar.getType() == VAR_PROP)
			{
				tmpvars.push_back(((BKE_VarProp*)leftvar.obj)->get());
				return tmpvars.back()[rightvar];
			}
			return leftvar[rightvar];
		}
	}
	else
	{
		bkplong start, stop, step = 1;
		if (tree->childs[1] != NULL)
			start = innerRun(tree->childs[1], _this, tmpvars);
		if (tree->childs[2] != NULL)
			stop = innerRun(tree->childs[2], _this, tmpvars);
		if (tree->childs.size() > 3 && tree->childs[3] != NULL)
			step = innerRun(tree->childs[3], _this, tmpvars);
		if (step == 0)
			throw Var_Except(L"步长不能为0");
		QUICKRETURNCONST(leftvar.getMid(tree->childs[1] ? &start : NULL, tree->childs[2] ? &stop : NULL, step));
	}
}

LED_FUNC(choose)
{
	if ((bool)innerRun(tree->childs[0], _this, tmpvars))
	{
		return (innerRun(tree->childs[1], _this, tmpvars));
	}
	else
	{
		return (innerRun(tree->childs[2], _this, tmpvars));
	}
}

LED_FUNC(instanceof)
{
	GETLEFTVAR;
	GETRIGHTVAR;
	BKE_String s = rightvar.asBKEStr();
	RECORDPOS;
	QUICKRETURNCONST(leftvar.instanceOf(s));
}

LED_FUNC(if)
{
	if ((bool)innerRun(tree->childs[1], _this, tmpvars))
	{
		return innerRun(tree->childs[0], _this, tmpvars);
	}
	RETURNNULL;
}

void Parser::unParse(BKE_bytree *tree, wstring &res)
{
	if (!tree)
		return;
	if (tree->Node.opcode == OP_CONSTVAR + OP_COUNT)
		return tree->Node.var.save(res, false);
	if (tree->Node.opcode == OP_LITERAL + OP_COUNT)
	{
		res += tree->Node.var.asBKEStr().getConstStr();
		return;
	}
	bkplong bp = 0;
	for (int i = 0; i<tree->childs.size(); i++)
		if (tree->childs[i])
			tree->childs[i]->parent = tree;
	if (tree->parent != NULL)
	{
		if (tree->parent->Node.opcode >= OP_COUNT)
			bp = pre[tree->parent->Node.opcode - OP_COUNT];
		else
			bp = lbp[tree->parent->Node.opcode];
		if (tree->parent->childs[0] != tree)
			bp++;
	}
	bool addbracket = false;
	if (tree->Node.opcode<OP_COUNT && lbp[tree->Node.opcode]<bp)
		addbracket = true;
	if (tree->Node.opcode == OP_POW && tree->parent != NULL && tree->parent->Node.opcode == OP_POW)
		addbracket = true;
	//if(tree->Node.opcode==OP_BRACKET)
	//	addbracket=false;
	if (addbracket)
		res += L"(";
#define MAKE_TWO(a) unParse(tree->childs[0], res);res+=L##a;unParse(tree->childs[1], res);break;
	switch (tree->Node.opcode)
	{
	case OP_END:
	case OP_END + OP_COUNT:
	case OP_IF:
		unParse(tree->childs[0], res);
		res += L" if ";
		unParse(tree->childs[1], res);
		break;
	case OP_IF + OP_COUNT:
		res += L"if(";
		unParse(tree->childs[0], res);
		res += L')';
		if (tree->childs[1] != NULL)
		{
			unParse(tree->childs[1], res);
		}
		res += L';';
		if (tree->childs[2] != NULL)
		{
			res += L"else ";
			unParse(tree->childs[2], res);
		}
		break;
	case OP_FOR + OP_COUNT:
		res += L"for(";
		unParse(tree->childs[0], res);
		res += L";";
		unParse(tree->childs[1], res);
		res += L";";
		unParse(tree->childs[2], res);
		res += L")";
		unParse(tree->childs[3], res);
		break;
	case OP_QUICKFOR + OP_COUNT:
		res += L"for " + tree->childs[0]->Node.var.asBKEStr().getConstStr();
		res += L"=";
		unParse(tree->childs[1], res);
		res += L',';
		unParse(tree->childs[2], res);
		if (tree->childs[3] != NULL)
		{
			res += L',';
			unParse(tree->childs[3], res);
		}
		unParse(tree->childs[4], res);
		break;
	case OP_WHILE + OP_COUNT:
		res += L"while ";
		unParse(tree->childs[0], res);
		//res += L" do ";
		unParse(tree->childs[1], res);
		break;
	case OP_DO + OP_COUNT:
		res += L"do ";
		unParse(tree->childs[0], res);
		res += L" while ";
		unParse(tree->childs[1], res);
		break;
	case OP_FOREACH + OP_COUNT:
		res += L"foreach ";
		if (tree->childs[0]!=NULL)
			unParse(tree->childs[0], res);
		if (tree->childs[1] != NULL)
		{
			res += L',';
			unParse(tree->childs[1], res);
		}
		res += L" in ";
		unParse(tree->childs[2], res);
		//res += L":";
		unParse(tree->childs[3], res);
		break;
	case OP_FUNCTION + OP_COUNT:
	{
		if (((BKE_VarFunction *)tree->Node.var.obj)->name.isVoid())
			res += L"function(";
		else
			res += L"function " + ((BKE_VarFunction *)tree->Node.var.obj)->name.getConstStr() + L"(";
		auto &&p = ((BKE_VarFunction *)tree->Node.var.obj)->paramnames;
		auto &&m = ((BKE_VarFunction *)tree->Node.var.obj)->initials;
		if (p.size() > 0)
		{
			res += p[0].getConstStr();
			if (m.find(p[0]) != m.end())
			{
				res += L'=';
				m[p[0]].save(res, false);
			}
		}
		for (int i = 1; i < (int)p.size(); i++)
		{
			res += L"," + p[i].getConstStr();
			if (m.find(p[i]) != m.end())
			{
				res += L'=';
				m[p[i]].save(res, false);
			}
		}
		res += L"){";
		unParse(((BKE_VarFunction *)tree->Node.var.obj)->func->code, res);
		res += L'}';
		break;
	}
	case OP_CLASS + OP_COUNT:
	{
		res += L"class " + tree->childs[0]->Node.var.asBKEStr().getConstStr();
		bkplong s = tree->childs.size();
		int i = 1;
		while (i < s)
		{
			if (i>1)
				res += L',';
			if (tree->childs[i]->Node.opcode == OP_LITERAL)
				res += L" extends " + tree->childs[i]->Node.var.asBKEStr().getConstStr();
			else
				break;
			i++;
		}
		res += L"{";
		while (i < s)
		{
			unParse(tree->childs[i], res);
			res += L";";
			i++;
		}
		res += L"}";
	}
		break;
	case OP_PROPGET + OP_COUNT:
		res += L"propset " + tree->childs[0]->Node.var.asBKEStr().getConstStr() + L"()";
		unParse(tree->childs[1], res);
		break;
	case OP_PROPSET + OP_COUNT:
		res += L"propset " + tree->childs[0]->Node.var.asBKEStr().getConstStr() + L"(" + tree->childs[1]->Node.var.asBKEStr().getConstStr() + L")";
		unParse(tree->childs[2], res);
		break;
	case OP_ADD + OP_COUNT:
		res += L"+";
		unParse(tree->childs[0], res);
		break;
	case OP_SUB + OP_COUNT:
		res += L"-";
		unParse(tree->childs[0], res);
		break;
	case OP_ADD:
		MAKE_TWO("+");
	case OP_SUB:
		MAKE_TWO("-");
	case OP_MUL:
		MAKE_TWO("*");
	case OP_DIV:
		MAKE_TWO("/");
	case OP_MOD:
		MAKE_TWO("%");
	case OP_POW:
		MAKE_TWO("^");
	case OP_EQUAL:
		MAKE_TWO("==");
	case OP_NEQUAL:
		MAKE_TWO("!=");
	case OP_EEQUAL:
		MAKE_TWO("===");
	case OP_NNEQUAL:
		MAKE_TWO("!==");
	case OP_LARGER:
		MAKE_TWO(">");
	case OP_SMALLER:
		MAKE_TWO("<");
	case OP_LE:
		MAKE_TWO(">=");
	case OP_SE:
		MAKE_TWO("<=");
	case OP_FASTAND:
		MAKE_TWO("&");
	case OP_AND:
		MAKE_TWO("&&");
	case OP_FASTOR:
		MAKE_TWO("|");
	case OP_OR:
		MAKE_TWO("||");
	case OP_NOT + OP_COUNT:
		res += L"!";
		unParse(tree->childs[0], res);
		break;
	case OP_SET:
		MAKE_TWO("=");
	case OP_SETADD:
		MAKE_TWO("+=");
	case OP_SETSUB:
		MAKE_TWO("-=");
	case OP_SETMUL:
		MAKE_TWO("*=");
	case OP_SETDIV:
		MAKE_TWO("/=");
	case OP_SETMOD:
		MAKE_TWO("%=");
	case OP_SETPOW:
		MAKE_TWO("^=");
	case OP_SETSET:
		MAKE_TWO("|=");
	case OP_SELFADD + OP_COUNT:
	case OP_SELFADD:
		if (tree->Node.opcode >= OP_COUNT)
		{
			res += L"++";
			unParse(tree->childs[0], res);
		}
		else
		{
			unParse(tree->childs[0], res);
			res += L"++";
		}
		break;
	case OP_SELFDEC + OP_COUNT:
	case OP_SELFDEC:
		if (tree->Node.opcode >= OP_COUNT)
		{
			res += L"--";
			unParse(tree->childs[0], res);
		}
		else
		{
			unParse(tree->childs[0], res);
			res += L"--";
		}
		break;
	case OP_ARRAY:
		unParse(tree->childs[0], res);
		res += L'[';
		unParse(tree->childs[1], res);
		if (tree->childs.size() > 2)
		{
			res += L':';
			unParse(tree->childs[2], res);
		}
		if (tree->childs.size() > 3)
		{
			res += L':';
			unParse(tree->childs[3], res);
		}
		res += L']';
		break;
	case OP_ARRAY + OP_COUNT:
		res += L"[";
		if (tree->childs.size()>1)
			unParse(tree->childs[0], res);
		for (int i = 1; i<tree->childs.size(); i++)
		{
			res += L",";
			unParse(tree->childs[i], res);
		}
		res += L"]";
		break;
	case OP_BRACKET:
		//case OP_BRACKET + OP_COUNT:
		unParse(tree->childs[0], res);
		res += L"(";
		if (tree->childs.size()>1)
			unParse(tree->childs[1], res);
		for (int i = 2; i < tree->childs.size(); i++)
		{
			res += L",";
			unParse(tree->childs[i], res);
		}
		res += L")";
		break;
	case OP_DIC + OP_COUNT:
		res += L"%[";
		if (tree->childs.size()>1)
		{
			unParse(tree->childs[0], res);
			res += L"=>";
			unParse(tree->childs[1], res);
		}
		for (int i = 2; tree->childs[i] != NULL; i += 2)
		{
			res += L",";
			unParse(tree->childs[i], res);
			res += L"=>";
			unParse(tree->childs[i + 1], res);
		}
		res += L"]";
		break;
	case OP_BLOCK + OP_COUNT:
		res += L"{";
		if (tree->childs.size() > 0)
			unParse(tree->childs[0], res);
		for (int i = 1; i<tree->childs.size(); i++)
		{
			res += L";";
			unParse(tree->childs[i], res);
		}
		res += L"}";
		break;
	case OP_DOT + OP_COUNT:
		res += L'.';
		unParse(tree->childs[0], res);
		break;
	case OP_DOT:
		MAKE_TWO(".");
	case OP_CONTINUE + OP_COUNT:
		res += L"continue;";
		break;
	case OP_BREAK + OP_COUNT:
		res += L"break;";
		break;
	case OP_RETURN + OP_COUNT:
		res += L"return";
		if (tree->childs[0] != NULL)
			res += L' ';
		unParse(tree->childs[0], res);
		res += L';';
		break;
	case OP_VAR + OP_COUNT:
		res += L"var ";
		for (int i = 0; i < tree->childs.size(); i += 2)
		{
			if (i>0)
				res += L",";
			res += tree->childs[i]->Node.var.asBKEStr().getConstStr();
			if (tree->childs[i + 1] != NULL)
			{
				res += L"=";
				unParse(tree->childs[i + 1], res);
			}
		}
		break;
	case OP_DELETE + OP_COUNT:
		res += L"delete ";
		unParse(tree->childs[0], res);
		break;
	case OP_TRY + OP_COUNT:
		res += L"try  ";
		unParse(tree->childs[0], res);
		res += L" catch(";
		unParse(tree->childs[1], res);
		res += L')';
		unParse(tree->childs[2], res);
		break;
	case OP_THROW + OP_COUNT:
		res += L"throw ";
		unParse(tree->childs[0], res);
		break;
	case OP_CHOOSE:
		unParse(tree->childs[0], res);
		res += L"?";
		unParse(tree->childs[1], res);
		res += L":";
		unParse(tree->childs[2], res);
		break;
	case OP_THIS + OP_COUNT:
		res += L"this";
		break;
	case OP_SUPER + OP_COUNT:
		res += L"super";
		break;
	case OP_GLOBAL + OP_COUNT:
		res += L"global";
		break;
	case OP_TOINT + OP_COUNT:
		res += L"int ";
		unParse(tree->childs[0], res);
		break;
	case OP_TOSTRING + OP_COUNT:
		res += L"string ";
		unParse(tree->childs[0], res);
		break;
	case OP_TONUMBER + OP_COUNT:
		res += L"number ";
		unParse(tree->childs[0], res);
		break;
	case OP_CONST + OP_COUNT:
		res += L"const ";
		unParse(tree->childs[0], res);
		break;
	case OP_TYPE + OP_COUNT:
		res += L"typeof ";
		unParse(tree->childs[0], res);
		break;
	case OP_INSTANCE:
		unParse(tree->childs[0], res); res += L" instanceof "; unParse(tree->childs[1], res); break;
	case OP_RESERVE:
	case OP_RESERVE + OP_COUNT:
		if (tree->childs.size() > 0)
			unParse(tree->childs[0], res);
		for (int i = 1; i<tree->childs.size(); i++)
		{
			res += L";";
			unParse(tree->childs[i], res);
		}
		break;
	case OP_WITH + OP_COUNT:
		res += L"with(";
		unParse(tree->childs[0], res);
		res += L')';
		unParse(tree->childs[1], res);
		break;
	case OP_INCONTEXTOF:
		unParse(tree->childs[0], res); res += L" incontextof "; unParse(tree->childs[1], res); break;
	case OP_STATIC + OP_COUNT:
		res += L"static ";
		unParse(tree->childs[0], res);
		break;
	default:
		//fatal error
		assert(0);
	}
	if (addbracket)
		res += L")";
	return;
#undef MAKE_TWO
}

inline bkplong getlength(const wstring &str, bkplong dstpos)
{
	bkplong s = 0;
	for (bkplong i = 0; i < dstpos - 1; i++)
	{
		if (i >= (bkplong)str.size())
			return s;
		if (str[i] == '\t')
			s += 8;
		else if (str[i] >= 32 && str[i] < 256)
			s++;
		else if (str[i] >= 256)
			s += 2;
	}
	return s;
}

wstring Parser::getTraceString()
{
#if PARSER_DEBUG == 0
	return L"";
#else
	wstring err;
	for (unsigned int i = 0; i < errorstack.size(); i++)
	{
		err += L"trace:";
		auto &&e = errorstack[i];
		if (!e.functionname.empty())
			err += L"在function " + e.functionname + L"中，";
		err += e.getMsg() + L"\n";
		err += e.lineinfo;
		if (!e.lineinfo.empty() && e.lineinfo.back() != L'\n' && e.lineinfo.back() != L'\r')
			err += L'\n';
		err += wstring(getlength(e.lineinfo, e.getPos()), L' ');
		err += L"^\n";
	}
	return err;
#endif
}
