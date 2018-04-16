#include "Bagel_Parser.h"
#include <cstdarg>
#include "Bagel_VM.h"

#define THROW(a,b) throwParseError(a,b);skipToNextBlock();return;
#define THROW2(a,b) throwParseError(a,b);return;
#define BUILD_TREE Bagel_AST* &tr=*tree;tr=new Bagel_AST(token);
#define GET_TREE tr->addChild();expression(&tr->childs.back());if(tr->childs.back()==NULL){THROW(W("此处期待一个输入"), token.pos);}
#define GET_TREE_OR_NULL tr->addChild();expression(&tr->childs.back());
#define GET_TREE2(lbp) tr->addChild();expression(&tr->childs.back(), lbp);if(tr->childs.back()==NULL){THROW(W("此处期待一个输入"), token.pos);}
#define GET_TREE2_OR_NULL(lbp) tr->addChild();expression(&tr->childs.back(), lbp);
#define JUDGE(op) if(next.opcode!=op){THROW(W("此处需要")+StringVal(OP_CODE[op]), next.pos);}
#define NEED(op) JUDGE(op);readToken();

const BKE_Char* OP_CODE[] =
{
	W("(end)"),
	W("if"),
	W("for(quickfor)"),
	W("for"),
	W("while"),
	W("do"),
	W("foreach"),
	W("constvar"),
	W("literal"),
	W("function"),
	W("class"),
	W("propget"),
	W("propset"),
	W("property"),
	W("setter"),
	W("getter"),
	W("+"),
	W("-"),
	W("*"),
	W("/"),
	W("\\"),
	W("%"),
	W("^"),
	W("=="),
	W("!="),
	W("==="),
	W("!=="),
	W(">"),
	W("<"),
	W(">="),
	W("<="),
	W("&"),
	W("&&"),
	W("|"),
	W("||"),
	W("!"),
	W("="),
	W("+="),
	W("-="),
	W("*="),
	W("/="),
	W("\\="),
	W("%="),
	W("^="),
	W("|="),
	W("++"),
	W("--"),
	W("("),
	W("["),
	W("%["),
	W("[]"),
	W("%[]"),
	W("{"),
	W("."),
	W("?."),
	W("?.["),
	W("?.("),
	W("continue"),
	W("break"),
	W("return"),
	W("var"),
	W("new"),
	W("delete"),
	W("try"),
	W("throw"),
	W("?"),
	W("this"),
	W("super"),
	W("global"),
	W("int"),
	W("string"),
	W("number"),
	W("const"),
	W("typeof"),
	W("instanceof"),
	W("switch"),
	W("case"),
	W("default"),
	W(""),
	W(""),
	W("extends"),
	W("in"),
	W("else"),
	W("then"),
	W("catch"),
	W(":"),
	W(";"),
	W(")"),
	W("]"),
	W("}"),
	W("),"),
	W("=>"),
	W("with"),
	W("incontextof"),
	W("static"),
	W("enum"),
	W("bitand"),
	W("bitor"),
	W("bitxor"),
	W("bitnot"),
	W("yield"),
	W("..."),
	W("(invalid)"),
	W("")
};

Bagel_Var Bagel_Var::run(Bagel_Var &self)
{
	if (getType() != VAR_FUNC)
		return Bagel_Var();
	auto *v = (Bagel_Function*)obj;
	if (self)
		v->setSelf(self);
	return v->VMRun(NULL, 0);
}

Bagel_Var Bagel_Var::readFromFile(const u16string &filename)
{
	StringVal re;
	auto res = Bagel_ReadFile(re, filename);
	if (res)
	{
		try
		{
			return Bagel_VM::getInstance()->Run(re);
		}
		catch (...)
		{
			return Bagel_Var();
		}
	}
	return Bagel_Var();
}

void Bagel_FunctionCode::markChildren()
{
	_GC.GC_Markself(code);
	_GC.GC_Markself(bytecode);
	for (auto &it : initial_stack)
	{
		_GC.GC_Markself(it);
	}
	for (auto &it : paramnames)
	{
		_GC.GC_Markself(it.s);
	}
#if PARSER_DEBUG
	for (auto &it : returnvar)
	{
		_GC.GC_Markself(it);
	}
#endif
}

Bagel_FunctionCode::Bagel_FunctionCode(Bagel_AST *code)
	: Bagel_Object(VAR_FUNCCODE_P)
	, bytecode(NULL), native(NULL), paramarrpos(0)
{
	this->code = code;
};

Bagel_Var Bagel_FunctionCode::VMDebugRun(const Bagel_Var &self, const Bagel_Var *paramarray, int paramcount, Bagel_Closure * _this, Bagel_Stack* clostack, Bagel_StringHolder name, Bagel_ThreadContext * ctx) const
{
	int i = 0;
	int pcount = paramarrpos > 0 ? paramnames.size() - 1 : paramnames.size();
	for (; i < pcount; i++)
	{
		if (i < paramcount)
		{
			_this->forceSetMember(paramnames[i], paramarray[i]);
		}
		else
		{
			_this->forceSetMember(paramnames[i], initial_stack[i]);
		}
	}
	if (paramarrpos > 0)
	{
		Bagel_Array *arr = new Bagel_Array();
		if (paramarray)
		{
			for (; i < paramcount; i++)
				arr->pushMember(paramarray[i]);
		}
		_this->forceSetMember(paramnames[pcount], arr);
	}
	bool newctx = false;
	if (!ctx)
	{
		ctx = Bagel_VM::getInstance()->createThreadHandle(*bytecode, _this, self, clostack, name);
		newctx = true;
	}
	else
	{
		ctx->addCallstack(bytecode, _this, self, clostack, name);
	}
	if (newctx)
		return ctx->Run();
	return Bagel_Var();
}

Bagel_Var Bagel_FunctionCode::VMDebugRunAndBlock(const Bagel_Var &self, const Bagel_Var *paramarray, int paramcount, Bagel_Closure * _this, Bagel_Stack* clostack, Bagel_StringHolder name, Bagel_ThreadContext * ctx) const
{
	int i = 0;
	int pcount = paramarrpos > 0 ? paramnames.size() - 1 : paramnames.size();
	for (; i < pcount; i++)
	{
		if (i < paramcount)
		{
			_this->forceSetMember(paramnames[i], paramarray[i]);
		}
		else
		{
			_this->forceSetMember(paramnames[i], initial_stack[i]);
		}
	}
	if (paramarrpos > 0)
	{
		Bagel_Array *arr = new Bagel_Array();
		if (paramarray)
		{
			for (; i < paramcount; i++)
				arr->pushMember(paramarray[i]);
		}
		_this->forceSetMember(paramnames[pcount], arr);
	}
	int depth = 0;
	if (!ctx)
	{
		ctx = Bagel_VM::getInstance()->createThreadHandle(*bytecode, _this, self, clostack, name);
	}
	else
	{
		depth = ctx->callstack.size();
		ctx->addCallstack(bytecode, _this, self, clostack, name);
	}
	return Bagel_VM::getInstance()->_debugRun(ctx, nullptr, depth);
}

Bagel_Var Bagel_FunctionCode::VMReleaseRun(const Bagel_Var &self, const Bagel_Var * paramarray, int paramcount, Bagel_Closure * _this, Bagel_Stack* clostack, Bagel_StringHolder name, Bagel_ThreadContext * ctx) const
{
	bool newctx = false;
	if (!ctx)
	{
		ctx = Bagel_VM::getInstance()->createThreadHandle(*bytecode, _this, self, clostack, name);
		newctx = true;
	}
	else
	{
		ctx->addCallstack(bytecode, _this, self, clostack, name);
	}
	//setparam
	int i = 0;
	int pcount = paramarrpos > 0 ? paramnames.size() - 1 : paramnames.size();
	for (; i < pcount; i++)
	{
		if (i < paramcount)
		{
			ctx->runningclo->runstack->relativePos[i + 2].forceSet(paramarray[i]);
		}
		else
		{
			ctx->runningclo->runstack->relativePos[i + 2].forceSet(initial_stack[i]);
		}
	}
	if (paramarrpos > 0)
	{
		Bagel_Array *arr = new Bagel_Array();
		if (paramarray)
		{
			for (; i < paramcount; i++)
				arr->pushMember(paramarray[i]);
		}
		ctx->runningclo->runstack->relativePos[paramarrpos] = arr;
	}
	if (newctx)
		return ctx->Run();
	return Bagel_Var();
}

Bagel_Var Bagel_FunctionCode::VMReleaseRunAndBlock(const Bagel_Var &self, const Bagel_Var * paramarray, int paramcount, Bagel_Closure * _this, Bagel_Stack* clostack, Bagel_StringHolder name, Bagel_ThreadContext * ctx) const
{
	int depth = 0;
	if (!ctx)
	{
		ctx = Bagel_VM::getInstance()->createThreadHandle(*bytecode, _this, self, clostack, name);
	}
	else
	{
		depth = ctx->callstack.size();
		ctx->addCallstack(bytecode, _this, self, clostack, name);
	}
	//setparam
	int i = 0;
	int pcount = paramarrpos > 0 ? paramnames.size() - 1 : paramnames.size();
	for (; i < pcount; i++)
	{
		if (i < paramcount)
		{
			ctx->runningclo->runstack->relativePos[i + 2].forceSet(paramarray[i]);
		}
		else
		{
			ctx->runningclo->runstack->relativePos[i + 2].forceSet(initial_stack[i]);
		}
	}
	if (paramarrpos > 0)
	{
		Bagel_Array *arr = new Bagel_Array();
		if (paramarray)
		{
			for (; i < paramcount; i++)
				arr->pushMember(paramarray[i]);
		}
		ctx->runningclo->runstack->relativePos[paramarrpos] = arr;
	}
	return Bagel_VM::getInstance()->_debugRun(ctx, nullptr, depth);
}

Bagel_Var Bagel_FunctionCode::VMRun(const Bagel_Var &self, const Bagel_Var * paramarray, int paramcount, Bagel_Closure * _this, Bagel_Stack* clostack, Bagel_StringHolder  name, Bagel_ThreadContext * ctx)
{
	if (native)
	{
		if (ctx && ctx->runningclo->retpos)
		{
			auto v = ctx->runningclo->retpos;
			v->forceSet((*native)(&self, paramarray, paramcount, _this, ctx));
			return *v;
		}
		return (*native)(&self, paramarray, paramcount, _this, ctx);
	}
	assert(bytecode);
	if (bytecode->release)
		return VMReleaseRun(self, paramarray, paramcount, _this, clostack, name, ctx);
	else
		return VMDebugRun(self, paramarray, paramcount, new Bagel_Closure(_this), clostack, name, ctx);
}

Bagel_Var Bagel_FunctionCode::VMRunAndBlock(const Bagel_Var &self, const Bagel_Var * paramarray, int paramcount, Bagel_Closure * _this, Bagel_Stack* clostack, Bagel_StringHolder  name, Bagel_ThreadContext * ctx)
{
	if (native)
	{
		if (ctx && ctx->runningclo->retpos)
		{
			auto v = ctx->runningclo->retpos;
			v->forceSet((*native)(&self, paramarray, paramcount, _this, ctx));
			return *v;
		}
		return (*native)(&self, paramarray, paramcount, _this, ctx);
	}
	assert(bytecode);
	if (bytecode->release)
		return VMReleaseRunAndBlock(self, paramarray, paramcount, _this, clostack, name, ctx);
	else
		return VMDebugRunAndBlock(self, paramarray, paramcount, new Bagel_Closure(_this), clostack, name, ctx);
}

Bagel_Parser::Bagel_Parser(bool init) : opmap(8)
{
	if (init)
		this->init();
	_globalStructures.parsers.push_back(this);
}

Bagel_Parser::~Bagel_Parser()
{
	_globalStructures.parsers.remove(this);
}

void Bagel_Parser::init()
{
	//init random seed
	srand((unsigned int)time(NULL));

	costTime = 0;
	krmode = false;
	rawstr = false;
	seeStringNotConst = false;

	forcequit = false;

	//add const
	//none.makeConst();
	addConst(W("true"), 1);
	addConst(W("false"), 0);
	addConst(W("void"), none);
	//addConst(W("global"), Bagel_Closure::Global()->addRef());

	//register operators
	opmap[W("property")] = OP_PROPERTY;
	opmap[W("setter")] = OP_SETTER;
	opmap[W("getter")] = OP_GETTER;
	opmap[W("default")] = OP_DEFAULT;
	opmap[W("switch")] = OP_SWITCH;
	opmap[W("case")] = OP_CASE;
	opmap[W("var")] = OP_VAR;
	opmap[W("new")] = OP_NEW;
	opmap[W("delete")] = OP_DELETE;
	opmap[W("extends")] = OP_EXTENDS;
	opmap[W("continue")] = OP_CONTINUE;
	opmap[W("break")] = OP_BREAK;
	opmap[W("return")] = OP_RETURN;
	opmap[W("in")] = OP_IN;
	opmap[W("else")] = OP_ELSE;
	opmap[W("if")] = OP_IF;
	opmap[W("while")] = OP_WHILE;
	opmap[W("do")] = OP_DO;
	opmap[W("for")] = OP_FOR;
	opmap[W("foreach")] = OP_FOREACH;
	opmap[W("function")] = OP_FUNCTION;
	opmap[W("class")] = OP_CLASS;
	opmap[W("propget")] = OP_PROPGET;
	opmap[W("propset")] = OP_PROPSET;
	opmap[W("try")] = OP_TRY;
	opmap[W("catch")] = OP_CATCH;
	opmap[W("throw")] = OP_THROW;
	opmap[W("global")] = OP_GLOBAL;
	opmap[W("this")] = OP_THIS;
	//兼容原版
	opmap[W("__self")] = OP_THIS;

	opmap[W("super")] = OP_SUPER;
	opmap[W("int")] = OP_TOINT;
	opmap[W("string")] = OP_TOSTRING;
	opmap[W("number")] = OP_TONUMBER;
	opmap[W("typeof")] = OP_TYPE;
	opmap[W("const")] = OP_CONST;
	opmap[W("instanceof")] = OP_INSTANCE;
	opmap[W("incontextof")] = OP_INCONTEXTOF;
	opmap[W("static")] = OP_STATIC;
	opmap[W("with")] = OP_WITH;
	opmap[W("enum")] = OP_ENUM;
	opmap[W("bitand")] = OP_BITAND;
	opmap[W("bitor")] = OP_BITOR;
	opmap[W("bitxor")] = OP_BITXOR;
	opmap[W("bitnot")] = OP_BITNOT;
	opmap[W("yield")] = OP_YIELD;

	//register all functions
	for (int i = 0; i < OP_COUNT * 2; i++)
	{
		funclist[i] = &Bagel_Parser::reportError;
		//runner[i] = NULL;
	}
	funclist[OP_END + OP_COUNT] = &Bagel_Parser::nud_end;
#define REG_NUD(c,b) funclist[b + OP_COUNT]=&Bagel_Parser::nud_##c;
	REG_NUD(one, OP_ADD);
	REG_NUD(one, OP_SUB);
	REG_NUD(one, OP_NOT);
	REG_NUD(one, OP_BITNOT);
	REG_NUD(bracket, OP_BRACKET);
	REG_NUD(array, OP_ARRAY);
	REG_NUD(dic, OP_DIC);
	REG_NUD(block, OP_BLOCK);
	REG_NUD(block, OP_RESERVE);
	REG_NUD(one, OP_SELFINC);
	REG_NUD(one, OP_SELFDEC);
	REG_NUD(label, OP_MUL);
	REG_NUD(if, OP_IF);
	REG_NUD(for, OP_FOR);
	REG_NUD(for, OP_QUICKFOR);
	REG_NUD(foreach, OP_FOREACH);
	REG_NUD(do, OP_DO);
	REG_NUD(while, OP_WHILE);
	REG_NUD(propget, OP_PROPGET);
	REG_NUD(propset, OP_PROPSET);
	REG_NUD(property, OP_PROPERTY);
	REG_NUD(function, OP_FUNCTION);
	REG_NUD(class, OP_CLASS);
	REG_NUD(continue, OP_CONTINUE);
	REG_NUD(continue, OP_BREAK);
	REG_NUD(return, OP_RETURN);
	REG_NUD(return, OP_YIELD);
	REG_NUD(var, OP_VAR);
	REG_NUD(new, OP_NEW);
	REG_NUD(delete, OP_DELETE);
	REG_NUD(try, OP_TRY);
	REG_NUD(this, OP_THIS);
	REG_NUD(this, OP_SUPER);
	REG_NUD(this, OP_GLOBAL);
	REG_NUD(one, OP_TOINT);
	REG_NUD(one, OP_TOSTRING);
	REG_NUD(one, OP_TONUMBER);
	REG_NUD(one, OP_TYPE);
	REG_NUD(one, OP_CONST);
	REG_NUD(var, OP_STATIC);
	REG_NUD(one, OP_THROW);
	REG_NUD(one, OP_DOT);
	REG_NUD(one, OP_OPTIONAL_DOT);
	REG_NUD(with, OP_WITH);
	REG_NUD(stop, OP_STOP);
	REG_NUD(stop, OP_COMMA);
	//REG_NUD(comment, comment, OP_COMMENT);
	REG_NUD(this, OP_LITERAL);		//for any const variables and names
	REG_NUD(string, OP_CONSTVAR);		//for any const variables and names
	REG_NUD(enum, OP_ENUM);
	REG_NUD(switch, OP_SWITCH);
	REG_NUD(case, OP_CASE);
	REG_NUD(default, OP_DEFAULT);
#undef REG_NUD
	//funclist[OP_END] = &Bagel_Parser::led_none;
#define REG_LED(c, b) funclist[b]=&Bagel_Parser::led_##c;//runner[b]=&Bagel_Parser::ledrunner_##a;
	REG_LED(two, OP_ADD);
	REG_LED(two, OP_SUB);
	REG_LED(two, OP_MUL);
	REG_LED(two, OP_DIV);
	REG_LED(two, OP_INTDIV);
	REG_LED(two, OP_MOD);
	REG_LED(pow, OP_POW);
	REG_LED(two, OP_EEQUAL);
	REG_LED(two, OP_EQUAL);
	REG_LED(two, OP_NEQUAL);
	REG_LED(two, OP_NNEQUAL);
	REG_LED(two, OP_LARGER);
	REG_LED(two, OP_SMALLER);
	REG_LED(two, OP_LE);
	REG_LED(two, OP_SE);
	REG_LED(two, OP_AND);
	REG_LED(two, OP_OR);
	REG_LED(two, OP_FASTAND);
	REG_LED(two, OP_FASTOR);
	REG_LED(two, OP_BITAND);
	REG_LED(two, OP_BITOR);
	REG_LED(two, OP_BITXOR);
	REG_LED(set, OP_SET);
	REG_LED(set, OP_SETADD);
	REG_LED(set, OP_SETSUB);
	REG_LED(set, OP_SETMUL);
	REG_LED(set, OP_SETDIV);
	REG_LED(set, OP_SETINTDIV);
	REG_LED(set, OP_SETPOW);
	REG_LED(set, OP_SETMOD);
	REG_LED(set, OP_SETSET);
	REG_LED(one, OP_SELFINC);
	REG_LED(one, OP_SELFDEC);
	//REG_LED(comment, comment, OP_COMMENT);
	REG_LED(param, OP_BRACKET);	//used in function call
	REG_LED(param, OP_OPTIONAL_CALL);	//used in function call
	REG_LED(dot, OP_DOT);
	REG_LED(dot, OP_OPTIONAL_DOT);
	REG_LED(ele, OP_ARRAY);
	REG_LED(ele, OP_OPTIONAL_ARR);
	REG_LED(choose, OP_CHOOSE);
	REG_LED(two, OP_INSTANCE);
	REG_LED(two, OP_IF);
	REG_LED(two, OP_INCONTEXTOF);
	REG_LED(comma, OP_COMMA);
	REG_LED(extract, OP_EXTRACTARGS);
#undef REG_LED

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
	//new xxx(params)
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
	//PRE(WHILE);
	//PRE(DO);
	//PRE(FOR);
	//PRE(QUICKFOR);
	//PRE(FOREACH);
	PRE(FUNCTION);
	//PRE(CLASS);
	//PRE(PROPGET);
	//PRE(PROPSET);
	INC;
	expprior = tmp - 5;
	LBP(COMMA);
	commaprior = tmp;
	INC;
	LBP(IF);
	INC;
	LBP(SET);
	LBP(SETADD);
	LBP(SETSUB);
	LBP(SETMUL);
	LBP(SETDIV);
	LBP(SETINTDIV);
	LBP(SETMOD);
	LBP(SETPOW);
	LBP(SETSET);
	INC;
	LBP(INSTANCE);
	LBP(INCONTEXTOF);
	INC;
	LBP(CHOOSE);
	INC;
	LBP(AND);
	LBP(OR);
	LBP(FASTAND);
	LBP(FASTOR);
	LBP(BITAND);
	LBP(BITOR);
	LBP(BITXOR);
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
	LBP(INTDIV);
	INC;
	LBP(MOD);
	LBP(POW);
	INC;
	PRE(NOT);
	PRE(BITNOT);
	INC;
	PRE(ADD);
	PRE(SUB);
	PRE(SELFINC);
	PRE(SELFDEC);
	LBP(SELFINC);
	LBP(SELFDEC);
	INC;
	PRE(TOINT);
	PRE(TOSTRING);
	PRE(TONUMBER);
	PRE(TYPE);
	PRE(CONST);
	INC;
	LBP(EXTRACTARGS);
	LBP(BRACKET);
	LBP(OPTIONAL_CALL);
	LBP(ARRAY);
	LBP(OPTIONAL_ARR);
	INC;
	PRE(NEW);
	LBP(DOT);
	LBP(OPTIONAL_DOT);
	PRE(DOT);
	PRE(OPTIONAL_DOT);
	PRE(MUL);

	head[OP_WHILE] = 1;
	head[OP_DO] = 1;
	head[OP_FOR] = 1;
	head[OP_FOREACH] = 1;
	head[OP_IF] = 1;
	head[OP_CLASS] = 1;
	head[OP_PROPGET] = 1;
	head[OP_PROPSET] = 1;
	head[OP_PROPERTY] = 1;
	head[OP_ENUM] = 1;
	head[OP_SWITCH] = 1;
	head[OP_BREAK] = 1;
	head[OP_CONTINUE] = 1;
	head[OP_RETURN] = 1;
	head[OP_WITH] = 1;
	head[OP_VAR] = 1;
	head[OP_STATIC] = 1;
}

#define MATCH(a, b) if(MatchFunc(W(a), &curpos)){ node.opcode = b; return;}
#define QRET(c)	curpos++;node.opcode = c;return;

void Bagel_Parser::readToken()
{
	BKE_Node &node = next;
#if PARSER_DEBUG>=2
	o_stderr << W("token at ") << *curpos << L'\n';
	if (iswspace(*curpos))
		o_stderr << (int)(*curpos) << W("is white!\n");
#endif
	while ((*curpos) && bkpIsSpace(*curpos))curpos++;
	node.pos = static_cast<uint32_t>(curpos - exp);
	if (!(*curpos))
	{
		node.opcode = OP_END;
		return;
	}
	BKE_Char ch = *curpos;
	//read operators
#if PARSER_DEBUG>=2
	o_stderr << W("token at ") << ch << L'\n';
#endif
	switch (ch)
	{
	case L'@':
		if (krmode)
		{
			++curpos;
			readToken();
			return;
		}
		break;
	case L'+':
		MATCH("+=", OP_SETADD);
		MATCH("++", OP_SELFINC);
		QRET(OP_ADD);
	case L'-':
		MATCH("-=", OP_SETSUB);
		MATCH("--", OP_SELFDEC);
		QRET(OP_SUB);
	case L'*':
		MATCH("*=", OP_SETMUL);
		QRET(OP_MUL);
	case L'/':
		if (MatchFunc(W("//"), &curpos))
		{
			//skip a line
			while (*curpos != L'\0' && *curpos != L'\r' && *curpos != L'\n')
				curpos++;
			readToken();
			return;
		}
		if (MatchFunc(W("/*"), &curpos))
		{
			while (*curpos && !(*curpos == L'*' && *(curpos + 1) == L'/'))
				curpos++;
			curpos += 2;
			readToken();
			return;
		}
		MATCH("/=", OP_SETDIV);
		QRET(OP_DIV);
	case L'\\':
		MATCH("\\=", OP_SETINTDIV);
		QRET(OP_INTDIV);
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
		if (krmode && MatchFunc(W("<->"), &curpos))
		{
			readToken();
			return;
		}
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
		if (krmode && MatchFunc(W("..."), &curpos))
		{
			readToken();
			return;
		}
		MATCH("...", OP_EXTRACTARGS);
		//check number
		++curpos;
		if (*curpos >= '0' && *curpos <= '9')
		{
			--curpos;
			node.var = Bagel_Number::str2num(curpos, &curpos);
			node.opcode = OP_CONSTVAR;
			node.pos2 = static_cast<uint32_t>(curpos - exp);
			return;
		}
		NextIsBareWord = true;
	#if PARSER_DEBUG>=2
		o_stderr << W("token after dot is ") << *(curpos + 1) << L'\n';
	#endif
		node.opcode = OP_DOT;
		return;
	case L'?':
		if (MatchFunc(W("?."), &curpos))
		{
			if (curpos[2] == '[')
				node.opcode = OP_OPTIONAL_ARR;
			else if (curpos[2] == '(')
				node.opcode = OP_OPTIONAL_CALL;
			else if (curpos[2] >= '0' && curpos[2] <= '9')
				node.opcode = OP_CHOOSE;	//for xxx?.3:.4
			else
				node.opcode = OP_OPTIONAL_DOT;
			return;
		}
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
			StringVal tmp;
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
					//throw Bagel_Except(W("读字符串时遇到意料之外的结尾"), static_cast<uint32_t>(curpos - exp));
					throwParseError(W("读字符串时遇到意料之外的结尾"), curpos - exp);
					node.opcode = OP_CONSTVAR;
					node.var = tmp;
					node.pos2 = static_cast<uint32_t>(curpos - exp);
					return;
				}
				else
					tmp.push_back(ch);
			}
			node.opcode = OP_CONSTVAR;
			node.var = tmp;
			node.pos2 = static_cast<uint32_t>(curpos - exp);
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
		node.var = Bagel_Number::str2num(curpos, &curpos);
		node.opcode = OP_CONSTVAR;
		node.pos2 = static_cast<uint32_t>(curpos - exp);
		return;
	case L'#':
		{
			//color number
			//#RGBA
			unsigned char n[8];
			int nn = 0;
			uint32_t color = 0;
			ch = *(++curpos);
			while (nn < 8)
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
				n[3] = 0xF;
			case 4:
				color |= 0x110000 * n[0];
				color |= 0x1100 * n[1];
				color |= 0x11 * n[2];
				color |= 0x11000000 * n[3];
				break;
			case 6:
				n[6] = n[7] = 0xF;
			case 8:
				color |= (0x100000 * n[0]) | (0x010000 * n[1]);
				color |= (0x1000 * n[2]) | (0x0100 * n[3]);
				color |= (0x10 * n[4]) | (0x01 * n[5]);
				color |= (0x10000000 * n[6]) | (0x01000000 * n[7]);
				break;
			default:
				{
					throwParseError(W("#后面只能接3,4,6,或8个十六进制数字表示颜色值"), curpos - exp);
					node.opcode = OP_CONSTVAR;
					node.var = 0;
					return;
				}
			}
			node.var = color;
			node.opcode = OP_CONSTVAR;
			node.pos2 = static_cast<uint32_t>(curpos - exp);
			return;
		}
	}
	if (ch == L'\'' || (ch == '\"' && krmode))
	{
		char16_t startch = ch;
		//read const string
		StringVal tmp;
		while (*(++curpos))
		{
			ch = *curpos;
			if (ch == '\\')
			{
				int32_t s;
				if (!*(++curpos))
				{
					throwParseError(W("读字符串时遇到意料之外的结尾"), curpos - exp);
					node.opcode = OP_CONSTVAR;
					node.var = tmp;
					return;
				};
				switch (ch = *curpos)
				{
				case 'n':
					tmp.push_back('\n');
					break;
				case 'r':
					tmp.push_back('\r');
					break;
				case 't':
					tmp.push_back('\t');
					break;
				case 'a':
					tmp.push_back('\a');
					break;
				case 'b':
					tmp.push_back('\b');
					break;
				case 'f':
					tmp.push_back('\f');
					break;
				case 'v':
					tmp.push_back('\v');
					break;
					//case 'o':
					//	ch = *(++curpos);
					//	s = 0;
					//	if (ch < '0' || ch > '9')
					//	{
					//		tmp.push_back((wchar_t)s);
					//		curpos--;
					//		break;
					//	}
					//	s += 64 * (ch - '0');
					//	ch = *(++curpos);
					//	if (ch < '0' || ch > '9')
					//	{
					//		tmp.push_back((wchar_t)s);
					//		curpos--;
					//		break;
					//	}
					//	s += 8 * (ch - '0');
					//	ch = *(++curpos);
					//	if (ch < '0' || ch > '9')
					//	{
					//		tmp.push_back((wchar_t)s);
					//		curpos--;
					//		break;
					//	}
					//	s += (ch - '0');
					//	tmp.push_back((wchar_t)s);
					//	break;
				case 'x':
				case 'X':
					ch = *(++curpos);
					s = 0;
					if (ch < '0' || (ch > '9' && towupper(ch) < 'A') || towupper(ch) > 'F')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s += (ch > '9' ? towupper(ch) - 'A' + 10 : ch - '0');
					ch = *(++curpos);
					if (ch < '0' || (ch > '9' && towupper(ch) < 'A') || towupper(ch) > 'F')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s *= 16;
					s += ch > '9' ? towupper(ch) - 'A' + 10 : ch - '0';
					ch = *(++curpos);
					if (ch < '0' || (ch > '9' && towupper(ch) < 'A') || towupper(ch) > 'F')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s *= 16;
					s += ch > '9' ? towupper(ch) - 'A' + 10 : ch - '0';
					ch = *(++curpos);
					if (ch < '0' || (ch > '9' && towupper(ch) < 'A') || towupper(ch) > 'F')
					{
						tmp.push_back((wchar_t)s);
						curpos--;
						break;
					}
					s *= 16;
					s += ch > '9' ? towupper(ch) - 'A' + 10 : ch - '0';
					tmp.push_back((wchar_t)s);
					break;
				case '\n':
				case '\r':
					if (!rawstr)
					{
						throwParseError(W("读字符串时遇到意料之外的结尾"), curpos - exp);
						node.opcode = OP_CONSTVAR;
						node.var = tmp;
						return;
					}
				default:
					tmp.push_back(ch);
				}
			}
			else if (!krmode && !rawstr && (ch == L'\n' || ch == L'\r' || ch == L'\0'))
			{
				throwParseError(W("读字符串时遇到意料之外的结尾"), curpos - exp);
				node.opcode = OP_CONSTVAR;
				node.var = tmp;
				node.pos2 = static_cast<uint32_t>(curpos - exp);
				return;
			}
			else if (ch == startch)
			{
				curpos++;
				break;
			}
			else
				tmp.push_back(ch);
		}
		node.opcode = OP_CONSTVAR;
		node.var = tmp;
		node.pos2 = static_cast<uint32_t>(curpos - exp);
		return;
	}
	if (ch >= 0x80 || isalpha(ch) || ch == L'_' /*|| ch==L'$' || ch==L'#'*/)
	{
		//read variable name
	#if PARSER_DEBUG>=2
		if (NextIsBareWord)
			o_stderr << ch << L'\n';
	#endif
		StringVal tmp;
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
				node.var = constmap[tmp];
				node.opcode = OP_CONSTVAR;
				node.pos2 = static_cast<uint32_t>(curpos - exp);
				return;
			}
			auto it = opmap.find(tmp);
			if (it != opmap.end())
			{
				node.opcode = it->second;
				if (node.opcode == OP_NEW || node.opcode == OP_FUNCTION || node.opcode == OP_PROPERTY || node.opcode == OP_PROPGET || node.opcode == OP_PROPSET)
					NextIsBareWord = true;
				return;
			}
		}
		if (krmode && tmp == W("isvalid"))
		{
			readToken();
			return;
		}
		NextIsBareWord = false;
		node.opcode = OP_LITERAL;
		node.var = tmp;
		node.pos2 = static_cast<uint32_t>(curpos - exp);
		return;
	}
	throwParseError(StringVal(W("读取时遇到非法字符<")) + ch + (BKE_Char)'>', curpos - exp);
	++curpos;
	node.opcode = OP_END;
	return;
}

void Bagel_Parser::skipToNextBlock()
{
	//出错的地方就是;或}的话，就跳过。
	token = next;
	readToken();
	if (token.opcode >= OP_COUNT)
		token.opcode -= OP_COUNT;
	int blockcount = 0;
	while (next.opcode != OP_END)
	{
		if (blockcount == 0 && (token.opcode == OP_STOP || token.opcode == OP_BLOCK2))
			break;
		if (token.opcode == OP_BLOCK)
			blockcount++;
		if (token.opcode == OP_BLOCK2)
			blockcount--;
		token = next;
		readToken();
	}
	forcequit = true;
}

void Bagel_Parser::throwParseError(const StringVal & v, int pos)
{
	int32_t line, linepos;
	info->getInfo(pos, line, linepos);
	errorMSG += W("在第") + bkpInt2Str(line) + W("行") + bkpInt2Str(linepos) + W("处：") + v;
	errorMSG += W("\n");
}

void Bagel_Parser::expression(Bagel_AST** tree, int rbp)
{
	//while (next.opcode == OP_STOP)
	//	readToken();
restart:
	if (rbp > expprior && head[next.opcode])
	{
		throwParseError(W("该符号必须放在句首"), next.pos);
		skipToNextBlock();
		goto restart;
	}
	token = next;
	readToken();
#if PARSER_DEBUG>=2
	o_stderr << W("readToken : ") << OP_CODE[next.opcode];
	if (next.opcode == OP_CONSTVAR)
		o_stderr << L" " << next.var.save(false);
	else if (next.opcode == OP_LITERAL)
		o_stderr << L" " << next.var.asBKEStr()->getConstStr();
	o_stderr << W("\n");
	if (next.opcode == OP_DOT)
	{
		o_stderr << W("now token after dot is ") << *curpos << L'\n';
	}
#endif
	token.opcode += OP_COUNT;
	(this->*funclist[token.opcode])(tree);
	while (!forcequit && rbp < lbp[next.opcode])
	{
		if (*tree)
		{
			//(*tree)->calcLastPos();
			(*tree)->Node.pos2 = next.pos;
		}
		token = next;
	#if PARSER_DEBUG>=2
		o_stderr << W("readToken2 : ") << OP_CODE[next.opcode];
		if (next.opcode == OP_CONSTVAR)
			o_stderr << L" " << next.var.save(false);
		else if (next.opcode == OP_LITERAL)
			o_stderr << L" " << next.var.asBKEStr()->getConstStr();
		o_stderr << W("\n");
		if (next.opcode == OP_DOT)
		{
			o_stderr << W("now token after dot is ") << *curpos << L'\n';
		}
	#endif
		readToken();
	#if PARSER_DEBUG>=2
		o_stderr << W("readToken3 : ") << OP_CODE[next.opcode];
		if (next.opcode == OP_CONSTVAR)
			o_stderr << L" " << next.var.save(false);
		else if (next.opcode == OP_LITERAL)
			o_stderr << L" " << next.var.asBKEStr()->getConstStr();
		o_stderr << W("\n");
		if (next.opcode == OP_DOT)
		{
			o_stderr << W("now token after dot is ") << *curpos << L'\n';
		}
	#endif
		(this->*funclist[token.opcode])(tree);
	}
	if (*tree)
	{
		//(*tree)->calcLastPos();
		(*tree)->Node.pos2 = next.pos;
	}
	forcequit = false;
}

Bagel_AST *Bagel_Parser::parse(const StringVal &exp, int32_t startpos, int32_t startline, bool parseAlways)
{
	init2(exp, NULL, startpos, startline);
	Bagel_AST *tr = NULL;
	curpos += startpos;
	GC_Locker locker;
	errorMSG.clear();
	inFunctionCall = false;
	readToken();
	if (next.opcode == OP_END)
		return NULL;
	expression(&tr);
	if (next.opcode != OP_END)
	{
		Bagel_AST *subtr = tr;
		tr = new Bagel_AST();
		tr->Node.opcode = OP_RESERVE + OP_COUNT;
		tr->childs.push_back(subtr);
	}
	while (next.opcode != OP_END)
	{
		if (next.opcode == OP_STOP)
		{
			readToken();
			//tr->addChild();
			continue;
		}
		GET_TREE_OR_NULL;
	}
	if (tr)
		tr->Node.pos2 = next.pos;
	if (!parseAlways && !errorMSG.empty())
	{
		throw Bagel_Except(errorMSG);
	}
	if (tr && tr->Node.opcode == OP_RESERVE + OP_COUNT && tr->childs.size() == 1)
	{
		auto tr2 = tr->childs.back();
		tr->childs.back() = NULL;
		return tr2;
	}
	return tr;
}

Bagel_AST * Bagel_Parser::parse(const BKE_Char * exp, int32_t startpos, int32_t startline, bool parseAlways)
{
	init2(exp, NULL, startpos, startline);
	Bagel_AST *tr = NULL;
	curpos += startpos;
	GC_Locker locker;
	errorMSG.clear();
	inFunctionCall = false;
	readToken();
	if (next.opcode == OP_END)
		return NULL;
	expression(&tr);
	if (next.opcode != OP_END)
	{
		Bagel_AST *subtr = tr;
		tr = new Bagel_AST();
		tr->Node.opcode = OP_RESERVE + OP_COUNT;
		tr->childs.push_back(subtr);
	}
	while (next.opcode != OP_END)
	{
		if (next.opcode == OP_STOP)
		{
			readToken();
			//tr->addChild();
			continue;
		}
		GET_TREE_OR_NULL;
	}
	if (tr)
		tr->Node.pos2 = next.pos;
	if (!parseAlways && !errorMSG.empty())
	{
		throw Bagel_Except(errorMSG);
	}
	if (tr && tr->Node.opcode == OP_RESERVE + OP_COUNT && tr->childs.size() == 1)
	{
		auto tr2 = tr->childs.back();
		tr->childs.back() = NULL;
		return tr2;
	}
	return tr;
}

void Bagel_Parser::nud_one(Bagel_AST** tree)
{
	int32_t op = token.opcode;
	BUILD_TREE;
	GET_TREE2(pre[token.opcode - OP_COUNT]);
	if (tr->Node.opcode == OP_SELFINC + OP_COUNT || tr->Node.opcode == OP_SELFDEC + OP_COUNT)
	{
		if (!tr->childs[0]->isVar())
		{
			//这种不属于句法结构错误的不需要skipToNextBlock()
			THROW2(W("等号左边必须是变量"), token.pos);
		}
	}
	else if (tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT)
	{
		try
		{
			Bagel_Var &a = tr->childs[0]->Node.var;
			Bagel_Var &c = tr->Node.var;
			bool opt = true;
			switch (op - OP_COUNT)
			{
			case OP_ADD:
				c.forceSet(a.asNumber());
				break;
			case OP_SUB:
				c.forceSet(-a.asNumber());
				break;
			case OP_NOT:
				c.forceSet(!a.asBoolean());
				break;
			case OP_BITNOT:
				c.forceSet(~a.asInteger());
				break;
			case OP_TOINT:
				c.forceSet(a.asInteger());
				break;
			case OP_TOSTRING:
				c.forceSet(a.asBKEStr());
				break;
			case OP_TONUMBER:
				c.forceSet(a.asNumber());
				break;
			case OP_TYPE:
				c.forceSet(a.getTypeBKEString());
				break;
			case OP_CONST:
				c.forceSet(a);
				break;
			default:
				opt = false;
			}
			if (opt)
			{
				tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
				tr->clearChilds();
			}
		}
		catch (Bagel_Except &e)
		{
			throwParseError(e.getMsgWithoutPos(), token.pos);
		}
	}
}

void Bagel_Parser::nud_bracket(Bagel_AST** tree)
{
	auto rawpos = curpos;
	int32_t p = token.pos;
	if (next.opcode >= OP_TOINT && next.opcode <= OP_CONST)
	{
		next.opcode += OP_COUNT;
		token = next;
		//Bagel_AST* &tr = *tree;
		//tr = new Bagel_AST(next);
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
	expression(tree, expprior);
	if (next.opcode != OP_BRACKET2)
	{
		THROW(getPosInfo(p) + W("的(需要)结尾"), next.pos);
	};
	readToken();
}

//if(xxx)yyy[;] [else zzz]
void Bagel_Parser::nud_if(Bagel_AST** tree)
{
	BUILD_TREE;
	if (next.opcode != OP_BRACKET)
	{
		THROW(W("if后必须接("), next.pos);
	}
	readToken();
	GET_TREE;
	NEED(OP_BRACKET2);
	GET_TREE_OR_NULL;
	tr->addChild();
	if (next.opcode == OP_STOP)
		readToken();
	if (next.opcode == OP_ELSE)
	{
		readToken();
		expression(&tr->childs.back());
	}
	BKE_Node block;
	block.opcode = OP_BLOCK + OP_COUNT;
	if (tr->childs[1] && tr->childs[1]->Node.opcode != OP_BLOCK + OP_COUNT)
	{
		//强行加闭包
		block.pos = tr->childs[1]->Node.pos;
		block.pos2 = tr->childs[2] ? tr->childs[2]->getFirstPos() : next.pos;
		tr->childs[1] = tr->childs[1]->addParent(block);
	}
	if (tr->childs[2] && tr->childs[2]->Node.opcode != OP_BLOCK + OP_COUNT)
	{
		//强行加闭包
		block.pos = tr->childs[2]->Node.pos;
		block.pos2 = next.pos;
		tr->childs[2] = tr->childs[2]->addParent(block);
	}
	if (tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT)
	{
		if ((bool)tr->childs[0]->Node.var)
		{
			(*tree) = tr->childs[1];
		}
		else
		{
			(*tree) = tr->childs[2];
		}
	}
	forcequit = true;
}

//try{...}catch(x){...}
//try ... catch(x) ...
void Bagel_Parser::nud_try(Bagel_AST** tree)
{
	BUILD_TREE;
	GET_TREE;
	if (next.opcode == OP_STOP)
		readToken();
	tr->childs[0]->Node.pos2 = next.pos;
	if (next.opcode != OP_CATCH)
	{
		THROW(W("try必须要有对应的catch块"), next.pos);
	}
	readToken();
	if (next.opcode == OP_BRACKET)
	{
		readToken();
		if (next.opcode != OP_LITERAL)
		{
			THROW(W("catch(后必须是变量名"), next.pos);
		};
		next.opcode += OP_COUNT;
		tr->childs.push_back(new Bagel_AST(next));
		readToken();
		if (next.opcode != OP_BRACKET2)
		{
			THROW(W("语法错误，catch后的括号中只能接合法的变量名，不能接字典或数组中的元素名，也不能接其他东西"), next.pos);
		}
		readToken();
	}
	else
	{
		tr->addChild();
	}
	GET_TREE_OR_NULL;
	if(tr->childs[1])
		tr->childs[1]->Node.pos2 = next.pos;
	if (!tr->childs[0] || tr->childs[0]->Node.opcode == OP_LITERAL)
	{
		//常量不会抛出异常
		tr = NULL;
	}
}

void Bagel_Parser::nud_with(Bagel_AST **tree)
{
	BUILD_TREE;
	if (next.opcode != OP_BRACKET)
	{
		THROW(W("此处需要("), next.pos);
	}
	GET_TREE;
	GET_TREE_OR_NULL;
	if (tr->childs[1] == NULL)
	{
		//那就不用with了
		tr = tr->childs[0];
	}
	else
	{
		tr->childs[1]->Node.pos2 = next.pos;
	}
	forcequit = true;
}

void Bagel_Parser::nud_array(Bagel_AST** tree)
{
	BUILD_TREE;
	int32_t p = token.pos;
	if (next.opcode == OP_ARR2)
	{
		readToken();
		tr->Node.opcode = OP_NULLARRAY + OP_COUNT;
		return;
	}
	tr->addChild();
	bool cancal = true;
	do
	{
		if (next.opcode != OP_COMMA)
		{
			expression(&tr->childs.back(), commaprior);
			if (tr->childs.back() && (tr->childs.back()->Node.opcode != OP_CONSTVAR + OP_COUNT || (seeStringNotConst && tr->childs.back()->Node.var.getType() == VAR_STR)))
				cancal = false;
		}
		tr->addChild();
		if (next.opcode != OP_COMMA)
			break;
		readToken();
	}
	while (next.opcode != OP_ARR2 && next.opcode != OP_END);
	tr->childs.pop_back();
	if (next.opcode != OP_ARR2)
	{
		THROW(getPosInfo(p) + W("的[需要]结尾。"), next.pos);
	};
	tr->Node.pos2 = next.pos;
	readToken();
	if (cancal)
	{
		Bagel_Array *arr = new Bagel_Array();
		for (auto &it : tr->childs)
		{
			if (it)
			{
				arr->pushMember(it->Node.var);
			}
			else
			{
				arr->pushMember(Bagel_Var());
			}
		}
		tr->Node.var.forceSet(arr);
		tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
		//tr->Node.var.makeConst();
		tr->clearChilds();
	}
}

//same as tjs
//a=%[name:value,name2:value2,...]
//a=%[strexp1=>value, strexp2=>value2]
void Bagel_Parser::nud_dic(Bagel_AST** tree)
{
	BUILD_TREE;
	int32_t pos = token.pos;
	if (next.opcode == OP_ARR2)
	{
		readToken();
		tr->Node.opcode = OP_NULLDIC + OP_COUNT;
		return;
	}
	tr->addChild();
	bool cancal = true;
	do
	{
		if (next.opcode != OP_COMMA)
		{
			expression(&tr->childs.back(), commaprior);
			if (tr->childs.back() == NULL)
				continue;
			if (next.opcode == OP_MAOHAO)
			{
				BKE_Node &tmp = tr->childs.back()->Node;
				if (tmp.opcode != OP_LITERAL + OP_COUNT)
				{
					THROW(W("语法错误，:前必须接合法的变量名"), tr->childs.back()->Node.pos);
				}
				tmp.opcode = OP_CONSTVAR + OP_COUNT;
			}
			else if (next.opcode == OP_VALUE)
			{
				if (tr->childs.back()->Node.opcode != OP_CONSTVAR + OP_COUNT || (seeStringNotConst && tr->childs.back()->Node.var.getType() == VAR_STR))
					cancal = false;
			}
			else
			{
				THROW(W("语法错误，这里需要=>或:"), next.pos);
			}
			readToken();
			GET_TREE2(commaprior);
			if (tr->childs.back()->Node.opcode != OP_CONSTVAR + OP_COUNT || (seeStringNotConst && tr->childs.back()->Node.var.getType() == VAR_STR))
				cancal = false;
			tr->addChild();
			//banned
			//else
			//{
			//	Bagel_AST *tmp=new Bagel_AST();
			//	tmp->Node.type=BKE_Node::LITERAL;
			//	tmp->Node.var=1;
			//	tr->childs.push_back(tmp);
			//}
		}
		if (next.opcode != OP_COMMA)
			break;
		readToken();
	}
	while (next.opcode != OP_ARR2 && next.opcode != OP_END);
	tr->childs.pop_back();
	if (next.opcode != OP_ARR2)
	{
		THROW(getPosInfo(pos) + W("的%[需要]结尾。"), next.pos);
	};
	tr->Node.pos2 = next.pos;
	readToken();
	if (cancal)
	{
		try
		{
			Bagel_Dic *dic = new Bagel_Dic();
			for (auto it = tr->childs.begin(); it != tr->childs.end(); it++)
			{
				Bagel_Var &a = (*it)->Node.var;
				++it;
				Bagel_Var &b = (*it)->Node.var;
				dic->setMember(a, b);
			}
			tr->Node.var.forceSet(dic);
			tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
			//tr->Node.var.makeConst();
			tr->clearChilds();
		}
		catch (Bagel_Except &e)
		{
			throwParseError(e.getMsgWithoutPos(), pos);
		}
	}
}

void Bagel_Parser::nud_block(Bagel_AST** tree)
{
	BUILD_TREE;
	//bool cancal=true;
	int32_t p = token.pos;
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
	if (next.opcode != OP_BLOCK2)
	{
		THROW(getPosInfo(p) + W("的{需要}结尾。"), next.pos);
	}
	tr->Node.pos2 = next.pos;
	//override optimization introduce many NULL error

	readToken();
	forcequit = true;
}

void Bagel_Parser::nud_label(Bagel_AST** tree)
{
	BUILD_TREE;
	if (next.opcode != OP_LITERAL)
	{
		if (isalpha(OP_CODE[next.opcode][0]))
		{
			tr->Node.var = W("*") + StringVal(OP_CODE[next.opcode]);
			//tr->Node.var.makeConst();
			tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
			readToken();
		}
		else
		{
			if (krmode)
			{
				tr->Node.var = W("*");
				tr->Node.opcode = OP_LITERAL + OP_COUNT;
				return;
			}
			else
			{
				THROW(W("语法错误，*后面只能直接加合法的变量名"), next.pos);
			}
		}
	}
	else
	{
		tr->Node.var = W("*") + next.var.asString();
		//tr->Node.var.makeConst();
		tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
		readToken();
	}
}

//for(exp1;exp2;exp3)exp4
//for(var xx in xxx)exp
void Bagel_Parser::nud_for(Bagel_AST** tree)
{
	BUILD_TREE;
	if (next.opcode != OP_BRACKET)
	{
		tr->Node.opcode = OP_QUICKFOR + OP_COUNT;
		if (next.opcode != OP_LITERAL)
		{
			THROW(W("此处需要是个变量名。"), next.pos);
		}
		tr->childs.push_back(new Bagel_AST(next));
		readToken();
		if (next.opcode != OP_SET)
		{
			THROW(W("此处需要="), next.pos);
		}
		readToken();
		GET_TREE2(commaprior);
		if (next.opcode != OP_COMMA)
		{
			THROW(W("此处需要,"), next.pos);
		}
		readToken();
		GET_TREE2(commaprior);
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
			if (tr->childs.back())
				tr->childs.back()->Node.opcode = OP_RESERVE + OP_COUNT;
		}
		else
		{
			THROW(W("此处需要{"), next.pos);
		}
		forcequit = true;
		return;
	}
	//normal for
	readToken();
	if (next.opcode == OP_VAR)
	{
		GET_TREE;
	}
	else
	{
		GET_TREE2_OR_NULL(expprior);
	}
	if (next.opcode != OP_STOP)
	{
		THROW(W("语法错误，此处应为;"), next.pos);
	};
	readToken();
	GET_TREE2_OR_NULL(expprior);
	if (next.opcode != OP_STOP)
	{
		THROW(W("语法错误，此处应为;"), next.pos);
	};
	readToken();
	GET_TREE2_OR_NULL(expprior);
	if (next.opcode != OP_BRACKET2)
	{
		THROW(W("语法错误，此处应为)"), next.pos);
	};
	readToken();
	GET_TREE_OR_NULL;
	if (tr->childs.back() && tr->childs.back()->Node.opcode == OP_BLOCK + OP_COUNT)
		tr->childs.back()->Node.opcode = OP_RESERVE + OP_COUNT;
	forcequit = true;
	tr->Node.pos2 = next.pos;
	//no optimization for "for" sentence
}

//foreach x,y in yyy{exp}
void Bagel_Parser::nud_foreach(Bagel_AST** tree)
{
	BUILD_TREE;
	bool hasBracket = false;
	if (next.opcode == OP_BRACKET)
	{
		hasBracket = true;
		readToken();
	}
	if (next.opcode == OP_LITERAL)
	{
		next.opcode += OP_COUNT;
		tr->childs.push_back(new Bagel_AST(next));
		readToken();
	}
	else
	{
		tr->addChild();
	}
	if (next.opcode == OP_COMMA)
	{
		readToken();
		if (next.opcode == OP_LITERAL)
		{
			next.opcode += OP_COUNT;
			tr->childs.push_back(new Bagel_AST(next));
			readToken();
		}
		else
		{
			tr->addChild();
		}
	}
	else
	{
		if (tr->childs[0] == NULL)
		{
			THROW(W("此处需要变量或逗号"), next.pos);
		}
		else
		{
			tr->addChild();
		}
	}
	if (next.opcode != OP_IN)
	{
		THROW(W("此处需要in"), next.pos);
	}
	readToken();
	GET_TREE;
	if (hasBracket)
	{
		if (next.opcode != OP_BRACKET2)
		{
			THROW(W("语法错误，需要)"), next.pos);
		};
	}
	//if (next.opcode != OP_BLOCK)
	//{
	//	THROW(W("语法错误，需要{"), next.pos);
	//};
	//readToken();
	GET_TREE_OR_NULL;
	//foreach也新建闭包
	if (tr->childs[0] && tr->childs[0]->Node.opcode == OP_BLOCK + OP_COUNT)
		tr->childs[0]->Node.opcode = OP_RESERVE + OP_COUNT;
	tr->Node.pos2 = next.pos;
	//has been done in OP_BLOCK
	forcequit = true;
	//no optimization for "foreach" sentence
}

//do xxxx while yyy;
void Bagel_Parser::nud_do(Bagel_AST** tree)
{
	BUILD_TREE;
	GET_TREE2_OR_NULL(expprior);
	if (next.opcode == OP_STOP)
		readToken();
	if (next.opcode != OP_WHILE)
	{
		THROW(W("语法错误，此处需要while"), next.pos);
	};
	readToken();
	GET_TREE2(expprior);
	//同下，do也新建闭包
	if (tr->childs[0] && tr->childs[0]->Node.opcode == OP_BLOCK + OP_COUNT)
		tr->childs[0]->Node.opcode = OP_RESERVE + OP_COUNT;
	tr->Node.pos2 = next.pos;
	forcequit = true;
	//no optimization for "do" sentence
}

//while xxxx { yyyy };
void Bagel_Parser::nud_while(Bagel_AST** tree)
{
	BUILD_TREE;
	JUDGE(OP_BRACKET);
	GET_TREE2(expprior);
	//if (next.opcode == OP_STOP)
	//	readToken();
	GET_TREE_OR_NULL;
	if (tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT && (bool)tr->childs[0]->Node.var == false)
	{
		*tree = NULL;
	}
	//If statement is a single statement (not a compound statement), the scope of variables declared in it is limited to the while loop as if it was a compound statement
	//while字句无论如何都新开一个闭包，所以{]被提升为OP_RESERVE（即无闭包{}）
	if (tr->childs[1] && tr->childs[1]->Node.opcode == OP_BLOCK + OP_COUNT)
		tr->childs[1]->Node.opcode = OP_RESERVE + OP_COUNT;
	tr->Node.pos2 = next.pos;
	forcequit = true;
}

void Bagel_Parser::nud_continue(Bagel_AST** tree)
{
	BUILD_TREE;
	forcequit = true;
}

void Bagel_Parser::nud_return(Bagel_AST** tree)
{
	BUILD_TREE;
	GET_TREE2_OR_NULL(expprior);
	forcequit = true;
}

void Bagel_Parser::nud_var(Bagel_AST** tree)
{
	BUILD_TREE;
	if (next.opcode != OP_LITERAL)
	{
		THROW(W("var后必须接合法的变量名"), next.pos);
	}
	tr->childs.push_back(new Bagel_AST(next));
	readToken();
	tr->addChild();
	while (1)
	{
		if (next.opcode == OP_STOP)
			return;
		if (next.opcode == OP_END)
			return;
		if (next.opcode == OP_SET)
		{
			readToken();
			expression(&tr->childs.back(), commaprior);
			continue;
		}
		if (next.opcode == OP_COMMA)
		{
			readToken();
			if (next.opcode != OP_LITERAL)
			{
				THROW(W("var后必须接合法的变量名"), next.pos);
			}
			tr->childs.push_back(new Bagel_AST(next));
			readToken();
			tr->addChild();
			continue;
		}
		THROW(W("语法错误，只能接等号，逗号或分号"), next.pos);
	}
	if (next.opcode != OP_STOP)
	{
		THROW(W("必须要接;"), next.pos);
	}
	forcequit = true;
}

void Bagel_Parser::nud_delete(Bagel_AST** tree)
{
	BUILD_TREE;
	GET_TREE2(expprior);
	if (!tr->childs[0]->isVar())
	{
		THROW(W("delete后必须是个变量"), next.pos);
	}
	if (tr->childs[0]->Node.opcode == OP_LITERAL + OP_COUNT)
	{
		THROW(W("delete只能删除数组或字典成员"), next.pos);
	}
	forcequit = true;
}

void Bagel_Parser::nud_string(Bagel_AST ** tree)
{
	BUILD_TREE;
	while (tr->Node.var.getType() == VAR_STR && next.opcode == OP_CONSTVAR && next.var.getType() == VAR_STR)
	{
		tr->Node.var += next.var;
		readToken();
	}
}

void Bagel_Parser::nud_this(Bagel_AST** tree)
{
	BUILD_TREE;
	//this tree doesn't have any child
}

//function xxx(a,b,...)
//function(a,b,..)
//function(a=2,b=false,..)
//function xxx(*a)
void Bagel_Parser::nud_function(Bagel_AST** tree)
{
	BUILD_TREE;
	Bagel_StringHolder name;
	if (next.opcode == OP_LITERAL)
	{
		name = next.var.asBKEStr();
		readToken();
	}
	vector<Bagel_StringHolder> paramname;
	//BKE_hashmap<Bagel_String, Bagel_Var> initials;
	vector<Bagel_Var> initial_stack;
	int haveArrParam = 0;
	if (next.opcode == OP_BRACKET)
	{
		//param sub-tree
		readToken();
		if (next.opcode != OP_BRACKET2)
		{
			do
			{
				if (next.opcode == OP_LITERAL)
				{
					if (haveArrParam)
					{
						THROW(W("带*的参数名（如果存在）必须是最后一个参数"), next.pos);
					}
					paramname.push_back(next.var);
					readToken();
				}
				else if (next.opcode == OP_MUL)
				{
					readToken();
					if (next.opcode == OP_LITERAL)
					{
						if (!haveArrParam)
						{
							//paramname.push_back(W("*") + next.var.asBKEStr()->getConstStr());
							paramname.push_back(next.var);
							//因为参数在栈上位置本来是从1开始的
							haveArrParam = paramname.size() - 1 + PARAMSTARTPOS;
						}
						else
						{
							THROW(W("只能有一个带*的参数名"), next.pos);
						}
						readToken();
					}
					else if (krmode)
					{
						if (!haveArrParam)
						{
							//paramname.push_back(W("*") + next.var.asBKEStr()->getConstStr());
							paramname.push_back(W("*"));
							//因为参数在栈上位置本来是从1开始的
							haveArrParam = paramname.size();
						}
						else
						{
							THROW(W("只能有一个带*的参数名"), next.pos);
						}
					}
					else
					{
						THROW(W("不合法的参数名"), next.pos);
					}
				}
				else
				{
					THROW(W("不合法的参数名"), next.pos);
				}
				if (next.opcode == OP_SET)
				{
					readToken();
					Bagel_AST *_tr = NULL;
					expression(&_tr, commaprior);
					if (_tr->Node.opcode != OP_CONSTVAR + OP_COUNT)
					{
						THROW(W("缺省值只能是常数"), next.pos);
					}
					initial_stack.push_back(_tr->Node.var);
				}
				else
				{
					initial_stack.emplace_back();
				}
				if (next.opcode != OP_COMMA)
					break;
				readToken();
			}
			while (1);
		}
		if (next.opcode != OP_BRACKET2)
		{
			THROW(W("语法错误，需要)"), next.pos);
		};
		readToken();
	}
	if (next.opcode != OP_BLOCK)
	{
		THROW(W("语法错误，需要{"), next.pos);
	}
	Bagel_AST *tr2 = NULL;
	int32_t startpos = next.pos;
	int32_t line, linepos;
	info->getInfo(startpos, line, linepos);
	expression(&tr2);
	if (tr2)
	{
		tr2->Node.opcode = OP_RESERVE + OP_COUNT;
		//add a null child to force return void
		tr2->addChild();
	}
	Bagel_Function *func = new Bagel_Function(tr2);
	func->func->info.reset(new Bagel_DebugInformation(StringVal(exp + startpos, next.pos - startpos), startpos, line - 1));
	func->func->paramnames = std::move(paramname);
	func->func->initial_stack = std::move(initial_stack);
	func->func->paramarrpos = haveArrParam;
	func->name = name;
	tr->Node.var = func;
	forcequit = !name.empty();
}

//propget xxx(){}
void Bagel_Parser::nud_propget(Bagel_AST **tree)
{
	BUILD_TREE;
	if (next.opcode == OP_LITERAL)
	{
		tr->childs.push_back(new Bagel_AST(next));
		readToken();
	}
	else
	{
		THROW(W("propget必须要有名称"), next.pos);
	}
	if (next.opcode != OP_BRACKET)
	{
		THROW(W("语法错误，需要("), next.pos);
	};
	readToken();
	if (next.opcode != OP_BRACKET2)
	{
		THROW(W("语法错误，需要)"), next.pos);
	};
	readToken();
	Bagel_AST *tr2 = NULL;
	int32_t startpos = next.pos;
	int32_t line, linepos;
	info->getInfo(startpos, line, linepos);
	expression(&tr2);
	if (tr2)
	{
		tr2->Node.opcode = OP_RESERVE + OP_COUNT;
	}
	Bagel_FunctionCode *func = new Bagel_FunctionCode(tr2);
	func->info.reset(new Bagel_DebugInformation(StringVal(exp + startpos, next.pos - 1 - startpos), startpos, line - 1));
	tr->Node.var = func;
	forcequit = true;
}

//propset xxx(a){}
void Bagel_Parser::nud_propset(Bagel_AST **tree)
{
	BUILD_TREE;
	if (next.opcode == OP_LITERAL)
	{
		tr->childs.push_back(new Bagel_AST(next));
		readToken();
	}
	else
	{
		THROW(W("propset必须要有名称"), next.pos);
	}
	if (next.opcode != OP_BRACKET)
	{
		THROW(W("语法错误，需要("), next.pos);
	};
	readToken();
	Bagel_StringHolder param;
	if (next.opcode == OP_LITERAL)
	{
		param = (Bagel_StringHolder)next.var;
		readToken();
	}
	else
	{
		THROW(W("propset有且只能有一个参数"), next.pos);
	}
	if (next.opcode != OP_BRACKET2)
	{
		THROW(W("语法错误，需要)"), next.pos);
	};
	readToken();
	Bagel_AST *tr2 = NULL;
	int32_t startpos = next.pos;
	int32_t line, linepos;
	info->getInfo(startpos, line, linepos);
	expression(&tr2);
	if (tr2)
	{
		tr2->Node.opcode = OP_RESERVE + OP_COUNT;
	}
	Bagel_FunctionCode *func = new Bagel_FunctionCode(tr2);
	func->info.reset(new Bagel_DebugInformation(StringVal(exp + startpos, next.pos - 1 - startpos), startpos, line - 1));
	func->paramnames.push_back(param);
	func->initial_stack.emplace_back();
	tr->Node.var = func;
	forcequit = true;
}

//enum {};
void Bagel_Parser::nud_enum(Bagel_AST **tree)
{
	BUILD_TREE;
	if (next.opcode == OP_LITERAL)
	{
		tr->childs.push_back(new Bagel_AST(next));
		readToken();
	}
	else
	{
		tr->childs.push_back(new Bagel_AST());
	}

	if (next.opcode != OP_BLOCK)
	{
		THROW(W("语法错误，此处只能接enum名称或者{"), next.pos);
	}
	//int32_t p = next.pos;
	double nextValue = 0;
	readToken();
	while (next.opcode != OP_BLOCK2)
	{
		if (next.opcode != OP_LITERAL)
		{
			THROW(W("语法错误，只能接变量名"), next.pos);
		}
		tr->childs.push_back(new Bagel_AST(next));
		readToken();
		if (next.opcode == OP_SET)
		{
			readToken();
			GET_TREE2(commaprior);
			if (tr->childs.back()->Node.opcode != OP_CONSTVAR + OP_COUNT)
			{
				THROW(W("语法错误，此处只能接常量值"), tr->childs.back()->Node.pos);
			}
			if (tr->childs.back()->Node.var.getType() == VAR_NUM)
			{
				nextValue = tr->childs.back()->Node.var.forceAsNumber() + 1;
			}
			else
			{
				THROW(W("语法错误，此处只能接数值"), next.pos);
			}
		}
		else
		{
			BKE_Node tmp;
			tmp.opcode = OP_CONSTVAR + OP_COUNT;
			tmp.var = nextValue;
			nextValue = nextValue + 1;
			tr->childs.push_back(new Bagel_AST(tmp));
		}

		if (next.opcode != OP_COMMA)
		{
			if (next.opcode == OP_BLOCK2)
			{
				break;
			}
			THROW(W("语法错误，此处只能接逗号','"), next.pos);
		}
		readToken();
	}
	readToken();
	forcequit = true;
}

//new xxx(args)
void Bagel_Parser::nud_new(Bagel_AST ** tree)
{
	BUILD_TREE;
	JUDGE(OP_LITERAL);
	next.opcode += OP_COUNT;
	tr->childs.push_back(new Bagel_AST(next));
	readToken();
	if (next.opcode != OP_BRACKET)
	{
		THROW(W("此处必须接着("), next.pos);
	}
}

void Bagel_Parser::nud_switch(Bagel_AST ** tree)
{
	BUILD_TREE;
	JUDGE(OP_BRACKET);
	GET_TREE;
	JUDGE(OP_BLOCK);
	GET_TREE_OR_NULL;
	if (tr->childs[1] == NULL)
	{
		//那就不用switch了
		tr = tr->childs[0];
	}
	forcequit = true;
}

void Bagel_Parser::nud_case(Bagel_AST ** tree)
{
	BUILD_TREE;
	int pos = token.pos;
	GET_TREE2(expprior);
	//if (tr->childs[0]->Node.opcode != OP_CONSTVAR + OP_COUNT || (tr->childs[0]->Node.var.getType() != VAR_NUM && tr->childs[0]->Node.var.getType() != VAR_STR))
	//{
	//	THROW(W("语法错误，case后只能接数值或字符串常量"), pos);
	//}
	NEED(OP_MAOHAO);
	forcequit = true;
}

void Bagel_Parser::nud_default(Bagel_AST ** tree)
{
	BUILD_TREE;
	NEED(OP_MAOHAO);
	forcequit = true;
}

void Bagel_Parser::nud_property(Bagel_AST ** tree)
{
	int pos = token.pos;
	BUILD_TREE;
	if (next.opcode != OP_LITERAL)
	{
		THROW(W("语法错误，此处需要一个合法的变量名"), next.pos);
	};
	tr->Node.var = next.var;
	readToken();
	NEED(OP_BLOCK);
	//getter
	tr->addChild();
	//setter
	tr->addChild();
	while (next.opcode != OP_BLOCK2)
	{
		switch (next.opcode)
		{
		case OP_GETTER:
			if (tr->childs[0])
			{
				THROW(W("该property已经有一个getter"), next.pos);
			}
			tr->childs[0] = new Bagel_AST();
			tr->childs[0]->Node.opcode = OP_CONSTVAR + OP_COUNT;
			readToken();
			if (next.opcode == OP_BRACKET)
			{
				readToken();
				NEED(OP_BRACKET2);
			}
			JUDGE(OP_BLOCK);
			{
				Bagel_AST *tr2 = NULL;
				int32_t startpos = next.pos;
				int32_t line, linepos;
				info->getInfo(startpos, line, linepos);
				expression(&tr2);
				if (tr2)
				{
					tr2->Node.opcode = OP_RESERVE + OP_COUNT;
				}
				Bagel_FunctionCode *func = new Bagel_FunctionCode(tr2);
				func->info.reset(new Bagel_DebugInformation(StringVal(exp + startpos, next.pos - 1 - startpos), startpos, line - 1));
				tr->childs[0]->Node.var = func;
			}
			break;
		case OP_SETTER:
			if (tr->childs[1])
			{
				THROW(W("该property已经有一个setter"), next.pos);
			}
			tr->childs[1] = new Bagel_AST();
			tr->childs[1]->Node.opcode = OP_CONSTVAR + OP_COUNT;
			readToken();
			NEED(OP_BRACKET);
			if (next.opcode != OP_LITERAL)
			{
				THROW(W("语法错误，此处需要一个合法的变量名"), next.pos);
			};
			{
				Bagel_StringHolder param = (Bagel_StringHolder)next.var;
				readToken();
				NEED(OP_BRACKET2);
				JUDGE(OP_BLOCK);
				Bagel_AST *tr2 = NULL;
				int32_t startpos = next.pos;
				int32_t line, linepos;
				info->getInfo(startpos, line, linepos);
				expression(&tr2);
				if (tr2)
				{
					tr2->Node.opcode = OP_RESERVE + OP_COUNT;
				}
				Bagel_FunctionCode *func = new Bagel_FunctionCode(tr2);
				func->info.reset(new Bagel_DebugInformation(StringVal(exp + startpos, next.pos - 1 - startpos), startpos, line - 1));
				func->paramnames.push_back(param);
				func->initial_stack.emplace_back();
				tr->childs[1]->Node.var = func;
			}
			break;
		default:
			THROW(W("语法错误，此处只能接getter或setter"), next.pos);
		}
	}
	if (!tr->childs[0] && !tr->childs[1])
	{
		THROW(W("property不能为空"), pos);
	}
	readToken();
	forcequit = true;
}

//class xxx [extends yyy]{};
void Bagel_Parser::nud_class(Bagel_AST** tree)
{
	BUILD_TREE;
	if (next.opcode != OP_LITERAL)
	{
		THROW(W("语法错误，此处只能接类名"), next.pos);
	};
	tr->childs.push_back(new Bagel_AST(next));
	readToken();
	if (next.opcode == OP_EXTENDS)
	{
		readToken();
		if (next.opcode != OP_LITERAL)
		{
			THROW(W("语法错误，此处只能接类名"), next.pos);
		};
		tr->childs.push_back(new Bagel_AST(next));
		readToken();
		while (next.opcode == OP_COMMA)
		{
			readToken();
			if (next.opcode != OP_LITERAL)
			{
				THROW(W("语法错误，此处只能接类名"), next.pos);
			};
			tr->childs.push_back(new Bagel_AST(next));
			readToken();
		}
	}
	int32_t p = next.pos;
	NEED(OP_BLOCK);
	//read block manually
	while (next.opcode != OP_BLOCK2)
	{
		GET_TREE;
		//check
		switch (tr->childs.back()->Node.opcode)
		{
		case OP_STATIC + OP_COUNT:
		case OP_VAR + OP_COUNT:
		case OP_FUNCTION + OP_COUNT:
		case OP_PROPGET + OP_COUNT:
		case OP_PROPSET + OP_COUNT:
		case OP_PROPERTY + OP_COUNT:
		case OP_ENUM + OP_COUNT:
			break;
		default:
			{
				THROW(W("语法错误，只能接static, var，function，propget，propset或property子句"), next.pos);
			};
		}
		while (next.opcode == OP_STOP)
		{
			readToken();
			continue;
		}
	}
	if (next.opcode != OP_BLOCK2)
	{
		THROW(getPosInfo(p) + W("的{需要}结尾"), next.pos);
	}
	readToken();
	forcequit = true;
}

//only used in ++ and --
//require left must be a variable
void Bagel_Parser::led_one(Bagel_AST** tree)
{
	if (!(*tree)->isVar())
	{
		THROW(W("++和--前面必须为变量"), next.pos);
	};
	(*tree) = (*tree)->addParent(token);
}

void Bagel_Parser::led_choose(Bagel_AST** tree)
{
	//int32_t op = token.opcode;
	(*tree) = (*tree)->addParent(token);
	Bagel_AST* &tr = *tree;
	GET_TREE2(lbp[OP_CHOOSE] - 1);
	NEED(OP_MAOHAO);
	GET_TREE2(lbp[OP_CHOOSE] - 1);
	if (tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT)
	{
		bool cond = (bool)tr->childs[0]->Node.var;
		if (cond)
		{
			(*tree) = tr->childs[1];
		}
		else
		{
			(*tree) = tr->childs[2];
		}
	}
}

void Bagel_Parser::led_comma(Bagel_AST ** tree)
{
	(*tree) = (*tree)->addParent(token);
	Bagel_AST* &tr = *tree;
	tr->Node.opcode = OP_RESERVE2 + OP_COUNT;
	do
	{
		GET_TREE2_OR_NULL(commaprior);
		if (next.opcode == OP_COMMA)
			readToken();
		else
			break;
	}
	while (1);
}

void Bagel_Parser::led_extract(Bagel_AST ** tree)
{
	if (!inFunctionCall)
	{
		THROW(W("...只能用在函数调用里，表示展开数组"), next.pos);
	}
	(*tree) = (*tree)->addParent(token);
	forcequit = true;
}

void Bagel_Parser::led_two(Bagel_AST** tree)
{
	int32_t op = token.opcode;
	int32_t pos = token.pos;
	(*tree) = (*tree)->addParent(token);
	Bagel_AST* &tr = *tree;
	GET_TREE2(lbp[op]);
	if (tr->childs[1]->Node.opcode == OP_CONSTVAR + OP_COUNT && tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT)
	{
		try
		{
			Bagel_Var &a = tr->childs[0]->Node.var;
			Bagel_Var &b = tr->childs[1]->Node.var;
			Bagel_Var &c = tr->Node.var;
			bool opt = true;
			switch (op)
			{
			case OP_ADD:
				c.forceSet(a + b);
				break;
			case OP_SUB:
				c.forceSet(a - b);
				break;
			case OP_MUL:
				c.forceSet(a * b);
				break;
			case OP_DIV:
				c.forceSet(a / b);
				break;
			case OP_MOD:
				c.forceSet(a % b);
				break;
			case OP_EQUAL:
				c.forceSet(a == b);
				break;
			case OP_NEQUAL:
				c.forceSet(a != b);
				break;
			case OP_EEQUAL:
				c.forceSet(a.strictEqual(b));
				break;
			case OP_NNEQUAL:
				c.forceSet(!a.strictEqual(b));
				break;
			case OP_LARGER:
				c.forceSet(a > b);
				break;
			case OP_SMALLER:
				c.forceSet(a < b);
				break;
			case OP_LE:
				c.forceSet(a >= b);
				break;
			case OP_SE:
				c.forceSet(a <= b);
				break;
			case OP_AND:
				c.forceSet(a.asBoolean() && b.asBoolean());
				break;
			case OP_OR:
				c.forceSet(a.asBoolean() || b.asBoolean());
				break;
			case OP_FASTAND:
				c.forceSet(a.isVoid() ? a : b);
				break;
			case OP_FASTOR:
				c.forceSet(a.isVoid() ? b : a);
				break;
			case OP_BITAND:
				c.forceSet(a.asInteger() && b.asInteger());
				break;
			case OP_BITOR:
				c.forceSet(a.asInteger() || b.asInteger());
				break;
			case OP_INSTANCE:
				c.forceSet(a.instanceOf(b));
				break;
			case OP_IF:
				c.forceSet(b.asBoolean() ? a : Bagel_Var());
				break;
			default:
				opt = false;
			}
			if (opt)
			{
				tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
				//tr->Node.var.makeConst();
				tr->clearChilds();
			}
		}
		catch (Bagel_Except &e)
		{
			if (info)
			{
				int32_t line, linepos;
				StringVal lineinfo;
				info->getInfo(pos, lineinfo, line, linepos);
				errorMSG += W("在第") + bkpInt2Str(line) + W("行") + bkpInt2Str(linepos) + W("处：") + e.getMsgWithoutPos();
			}
			else
			{
				errorMSG += W("在第") + bkpInt2Str(pos) + W("处：") + e.getMsgWithoutPos();
			}
		}
	}
}

void Bagel_Parser::led_pow(Bagel_AST** tree)
{
	int32_t op = token.opcode;
	int32_t pos = token.pos;
	(*tree) = (*tree)->addParent(token);
	Bagel_AST* &tr = *tree;
	GET_TREE2(lbp[op] - 1);
	if (tr->childs[1]->Node.opcode == OP_CONSTVAR + OP_COUNT && tr->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT)
	{
		try
		{
			Bagel_Var &a = tr->childs[0]->Node.var;
			Bagel_Var &b = tr->childs[1]->Node.var;
			Bagel_Var &c = tr->Node.var;
			c.forceSet(a ^ b);
			tr->Node.opcode = OP_CONSTVAR + OP_COUNT;
			tr->clearChilds();
		}
		catch (Bagel_Except &e)
		{
			if (info)
			{
				int32_t line, linepos;
				StringVal lineinfo;
				info->getInfo(pos, lineinfo, line, linepos);
				errorMSG += W("在第") + bkpInt2Str(line) + W("行") + bkpInt2Str(linepos) + W("处：") + e.getMsgWithoutPos();
			}
			else
			{
				errorMSG += W("在第") + bkpInt2Str(pos) + W("处：") + e.getMsgWithoutPos();
			}
			errorMSG += W("\n");
		}
	}
}

void Bagel_Parser::led_dot(Bagel_AST** tree)
{
	if (IS_OPTIONAL((*tree)->Node.opcode))
	{
		token.opcode = OP_OPTIONAL_DOT;
	}
	(*tree) = (*tree)->addParent(token);
	//left don't need to be a NAME because we enable syntax such as "233".type
	if (next.opcode != OP_LITERAL)
	{
	#if PARSER_DEBUG>=2
		o_stderr << W("after dot:") << next.opcode << L" " << OP_CODE[next.opcode];
	#endif
		THROW(W(".右边必须是合法的变量名"), next.pos);
	};
	next.opcode += OP_COUNT;
	Bagel_AST *right = new Bagel_AST(next);
	(*tree)->childs.push_back(right);
	readToken();
}

void Bagel_Parser::led_ele(Bagel_AST** tree)
{
	if (IS_OPTIONAL((*tree)->Node.opcode))
	{
		token.opcode = OP_OPTIONAL_ARR;
	}
	(*tree) = (*tree)->addParent(token);
	int32_t p = token.pos;
	Bagel_AST* &tr = *tree;
	tr->addChild();
	while (next.opcode != OP_ARR2)
	{
		if (next.opcode == OP_STOP)
		{
			readToken();
			tr->addChild();
		}
		else if (next.opcode == OP_MAOHAO)
		{
			readToken();
			tr->addChild();
		}
		else
		{
			expression(&tr->childs.back(), expprior);
			if (next.opcode != OP_MAOHAO && next.opcode != OP_ARR2)
			{
				THROW(W("语法错误，期望:或]：") + getPosInfo(p) + W("的[需要]结尾。"), next.pos);
			}
		}
	}
	if (next.opcode != OP_ARR2)
	{
		THROW(W("中括号不匹配") + getPosInfo(p) + W("的[需要]结尾。"), next.pos);
	}
	if (tr->childs[1] == NULL && tr->childs.size() == 2)
	{
		THROW(W("中括号里不能为空"), next.pos);
	}
	if (tr->childs.size() > 4)
	{
		THROW(W("中括号里项数太多。"), next.pos);
	}
	if (tr->childs.size() > 2)
	{
		if (tr->childs[0]->Node.opcode == OP_THIS + OP_COUNT || tr->childs[0]->Node.opcode == OP_SUPER + OP_COUNT)
		{
			THROW(W("中括号里项数太多。"), next.pos);
		}
	}
	readToken();
}

void Bagel_Parser::led_set(Bagel_AST** tree)
{
	if (!*tree)
	{
		THROW(W("左边不能为无效语句"), token.pos);
	}
	int32_t op = token.opcode;
	(*tree) = (*tree)->addParent(token);
	Bagel_AST* &tr = *tree;
	if (!tr->childs[0]->isVar())
	{
		THROW(W("等号左边必须是变量"), token.pos);
	}
	GET_TREE2(lbp[op] - 1);
}

void Bagel_Parser::led_param(Bagel_AST** tree)
{
	if (IS_OPTIONAL((*tree)->Node.opcode))
	{
		token.opcode = OP_OPTIONAL_CALL;
	}
	(*tree) = (*tree)->addParent(token);
	Bagel_AST* &tr = *tree;
	auto oldInFunction = inFunctionCall;
	inFunctionCall = true;
	while (next.opcode != OP_BRACKET2)
	{
		tr->addChild();
		if (next.opcode != OP_COMMA)
		{
			expression(&tr->childs.back(), commaprior);
			if (next.opcode == OP_BRACKET2)
				break;
			NEED(OP_COMMA);
		}
		else
		{
			readToken();
		}
	}
	if (next.opcode != OP_BRACKET2)
	{
		inFunctionCall = oldInFunction;
		THROW(W("语法错误，需要)"), next.pos);
	};
	//no optimization for function call
	readToken();
	inFunctionCall = oldInFunction;
}

void Bagel_Parser::unParse(Bagel_AST *tree, StringVal &res)
{
	if (!tree)
		return;
	if (tree->Node.opcode == OP_CONSTVAR + OP_COUNT)
		return tree->Node.var.save(res, false);
	if (tree->Node.opcode == OP_LITERAL + OP_COUNT)
	{
		res += tree->Node.var.asBKEStr()->getConstStr();
		return;
	}
	int32_t bp = 0;
	for (int i = 0; i < tree->childs.size(); i++)
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
	if (tree->Node.opcode < OP_COUNT && lbp[tree->Node.opcode] < bp)
		addbracket = true;
	if (tree->Node.opcode == OP_POW && tree->parent != NULL && tree->parent->Node.opcode == OP_POW)
		addbracket = true;
	//if(tree->Node.opcode==OP_BRACKET)
	//	addbracket=false;
	if (addbracket)
		res += '(';
#define MAKE_TWO(a) unParse(tree->childs[0], res);res+=W(a);unParse(tree->childs[1], res);break;
	switch (tree->Node.opcode)
	{
	case OP_END:
	case OP_END + OP_COUNT:
	case OP_IF:
		unParse(tree->childs[0], res);
		res += W(" if ");
		unParse(tree->childs[1], res);
		break;
	case OP_IF + OP_COUNT:
		res += W("if(");
		unParse(tree->childs[0], res);
		res += ')';
		if (tree->childs[1] != NULL)
		{
			unParse(tree->childs[1], res);
		}
		res += ';';
		if (tree->childs[2] != NULL)
		{
			res += W("else ");
			unParse(tree->childs[2], res);
		}
		break;
	case OP_FOR + OP_COUNT:
		res += W("for(");
		unParse(tree->childs[0], res);
		res += ';';
		unParse(tree->childs[1], res);
		res += ';';
		unParse(tree->childs[2], res);
		res += W("){");
		unParse(tree->childs[3], res);
		res += '}';
		break;
	case OP_QUICKFOR + OP_COUNT:
		res += W("for ") + tree->childs[0]->Node.var.asBKEStr()->getConstStr();
		res += '=';
		unParse(tree->childs[1], res);
		res += ',';
		unParse(tree->childs[2], res);
		if (tree->childs[3] != NULL)
		{
			res += ',';
			unParse(tree->childs[3], res);
		}
		res += '{';
		unParse(tree->childs[4], res);
		res += '}';
		break;
	case OP_WHILE + OP_COUNT:
		res += W("while ");
		unParse(tree->childs[0], res);
		res += '{';
		unParse(tree->childs[1], res);
		res += '}';
		break;
	case OP_DO + OP_COUNT:
		res += W("do{");
		unParse(tree->childs[0], res);
		res += W("}while ");
		unParse(tree->childs[1], res);
		break;
	case OP_FOREACH + OP_COUNT:
		res += W("foreach ");
		if (tree->childs[0] != NULL)
			unParse(tree->childs[0], res);
		if (tree->childs[1] != NULL)
		{
			res += ',';
			unParse(tree->childs[1], res);
		}
		res += W(" in ");
		unParse(tree->childs[2], res);
		//res += L":";
		unParse(tree->childs[3], res);
		break;
	case OP_FUNCTION + OP_COUNT:
		{
			auto func = tree->Node.var.forceAsFunc();
			if (func->name.empty())
				res += W("function(");
			else
				res += W("function ") + func->name.getConstStr() + (char16_t)'(';
			auto &&p = func->func->paramnames;
			auto &&m = func->func->initial_stack;
			if (p.size() > 0)
			{
				res += p[0].getConstStr();
				if (!m[0].isVoid())
				{
					res += '=';
					m[0].save(res, false);
				}
			}
			for (int i = 1; i < (int)p.size(); i++)
			{
				res += (char16_t)',' + p[i].getConstStr();
				if (!m[i].isVoid())
				{
					res += '=';
					m[i].save(res, false);
				}
			}
			res += W("){");
			unParse(func->func->code, res);
			res += '}';
			break;
		}
	case OP_CLASS + OP_COUNT:
		{
			res += W("class ") + tree->childs[0]->Node.var.asBKEStr()->getConstStr();
			int32_t s = tree->childs.size();
			int i = 1;
			while (i < s)
			{
				if (tree->childs[i]->Node.opcode == OP_LITERAL)
				{
					if (i == 1)
						res += W(" extends ") + tree->childs[i]->Node.var.asBKEStr()->getConstStr();
					else
						res += (char16_t)',' + tree->childs[i]->Node.var.asBKEStr()->getConstStr();
				}
				else
					break;
				i++;
			}
			res += '{';
			while (i < s)
			{
				unParse(tree->childs[i], res);
				res += ';';
				i++;
			}
			res += '}';
		}
		break;
	case OP_ENUM + OP_COUNT:
		{
			res += W("enum ") + tree->childs[0]->Node.var.asBKEStr()->getConstStr();
			res += '{';
			for (int32_t i = 1, count = tree->childs.size(); i < count; i += 2)
			{
				res += tree->childs[i]->Node.var.forceAsBKEStr()->getConstStr();
				res += '=';
				tree->childs[i + 1]->Node.var.save(res, false);
				res += ',';
			}
			res += W("};");
		}
		break;
	case OP_PROPGET + OP_COUNT:
		{
			res += W("propget ") + tree->childs[0]->Node.var.asBKEStr()->getConstStr() + W("()");
			Bagel_FunctionCode *func = tree->Node.var.forceAsObject<Bagel_FunctionCode>();
			unParse(func->code, res);
		}
		break;
	case OP_PROPSET + OP_COUNT:
		{
			Bagel_FunctionCode *func = tree->Node.var.forceAsObject<Bagel_FunctionCode>();
			res += W("propset ") + tree->childs[0]->Node.var.asBKEStr()->getConstStr() + (char16_t)'(' + func->paramnames[0].getConstStr() + (char16_t)')';
			unParse(func->code, res);
		}
		break;
	case OP_PROPERTY + OP_COUNT:
		{
			res += W("property ") + tree->Node.var.asBKEStr()->getConstStr() + (char16_t)'{';
			if (tree->childs[0])
			{
				res += W("getter");
				unParse(tree->childs[0]->Node.var.forceAsObject<Bagel_FunctionCode>()->code, res);
			}
			if (tree->childs[1])
			{
				Bagel_FunctionCode *func = tree->childs[1]->Node.var.forceAsObject<Bagel_FunctionCode>();
				res += W("setter(") + func->paramnames[0].getConstStr() + (char16_t)')';
				unParse(func->code, res);
			}
		}
		break;
	case OP_ADD + OP_COUNT:
		res += '+';
		unParse(tree->childs[0], res);
		break;
	case OP_SUB + OP_COUNT:
		res += '-';
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
	case OP_INTDIV:
		MAKE_TWO("\\");
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
		res += '!';
		unParse(tree->childs[0], res);
		break;
	case OP_BITAND:
		MAKE_TWO(" bitand ");
	case OP_BITOR:
		MAKE_TWO(" bitor ");
	case OP_BITNOT + OP_COUNT:
		res += W("bitnot ");
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
	case OP_SETINTDIV:
		MAKE_TWO("\\=");
	case OP_SETMOD:
		MAKE_TWO("%=");
	case OP_SETPOW:
		MAKE_TWO("^=");
	case OP_SETSET:
		MAKE_TWO("|=");
	case OP_SELFINC + OP_COUNT:
	case OP_SELFINC:
		if (tree->Node.opcode >= OP_COUNT)
		{
			res += W("++");
			unParse(tree->childs[0], res);
		}
		else
		{
			unParse(tree->childs[0], res);
			res += W("++");
		}
		break;
	case OP_SELFDEC + OP_COUNT:
	case OP_SELFDEC:
		if (tree->Node.opcode >= OP_COUNT)
		{
			res += W("--");
			unParse(tree->childs[0], res);
		}
		else
		{
			unParse(tree->childs[0], res);
			res += W("--");
		}
		break;
	case OP_ARRAY:
		unParse(tree->childs[0], res);
		res += '[';
		unParse(tree->childs[1], res);
		if (tree->childs.size() > 2)
		{
			res += ':';
			unParse(tree->childs[2], res);
		}
		if (tree->childs.size() > 3)
		{
			res += ':';
			unParse(tree->childs[3], res);
		}
		res += ']';
		break;
	case OP_ARRAY + OP_COUNT:
		res += '[';
		if (tree->childs.size() > 1)
			unParse(tree->childs[0], res);
		for (int i = 1; i < tree->childs.size(); i++)
		{
			res += ',';
			unParse(tree->childs[i], res);
		}
		res += ']';
		break;
	case OP_NULLARRAY + OP_COUNT:
		res += W("[]");
		break;
	case OP_BRACKET:
		//case OP_BRACKET + OP_COUNT:
		unParse(tree->childs[0], res);
		res += '(';
		if (tree->childs.size() > 1)
			unParse(tree->childs[1], res);
		for (int i = 2; i < tree->childs.size(); i++)
		{
			res += ',';
			unParse(tree->childs[i], res);
		}
		res += ')';
		break;
	case OP_DIC + OP_COUNT:
		res += W("%[");
		if (tree->childs.size() > 1)
		{
			unParse(tree->childs[0], res);
			res += W("=>");
			unParse(tree->childs[1], res);
		}
		for (int i = 2; tree->childs[i] != NULL; i += 2)
		{
			res += ',';
			unParse(tree->childs[i], res);
			res += W("=>");
			unParse(tree->childs[i + 1], res);
		}
		res += ']';
		break;
	case OP_NULLDIC + OP_COUNT:
		res += W("%[]");
		break;
	case OP_BLOCK + OP_COUNT:
		res += '{';
		if (tree->childs.size() > 0)
			unParse(tree->childs[0], res);
		for (int i = 1; i < tree->childs.size(); i++)
		{
			res += ';';
			unParse(tree->childs[i], res);
		}
		res += '}';
		break;
	case OP_DOT + OP_COUNT:
		res += '.';
		unParse(tree->childs[0], res);
		break;
	case OP_DOT:
		MAKE_TWO(".");
	case OP_CONTINUE + OP_COUNT:
		res += W("continue;");
		break;
	case OP_BREAK + OP_COUNT:
		res += W("break;");
		break;
	case OP_RETURN + OP_COUNT:
		res += W("return");
		if (tree->childs[0] != NULL)
			res += ' ';
		unParse(tree->childs[0], res);
		res += ';';
		break;
	case OP_VAR + OP_COUNT:
		res += W("var ");
		for (int i = 0; i < tree->childs.size(); i += 2)
		{
			if (i > 0)
				res += ',';
			res += tree->childs[i]->Node.var.asBKEStr()->getConstStr();
			if (tree->childs[i + 1] != NULL)
			{
				res += '=';
				unParse(tree->childs[i + 1], res);
			}
		}
		break;
	case OP_DELETE + OP_COUNT:
		res += W("delete ");
		unParse(tree->childs[0], res);
		break;
	case OP_TRY + OP_COUNT:
		res += W("try  ");
		unParse(tree->childs[0], res);
		res += W(" catch(");
		unParse(tree->childs[1], res);
		res += ')';
		unParse(tree->childs[2], res);
		break;
	case OP_THROW + OP_COUNT:
		res += W("throw ");
		unParse(tree->childs[0], res);
		break;
	case OP_CHOOSE:
		unParse(tree->childs[0], res);
		res += '?';
		unParse(tree->childs[1], res);
		res += ':';
		unParse(tree->childs[2], res);
		break;
	case OP_THIS + OP_COUNT:
		res += W("this");
		break;
	case OP_SUPER + OP_COUNT:
		res += W("super");
		break;
	case OP_GLOBAL + OP_COUNT:
		res += W("global");
		break;
	case OP_TOINT + OP_COUNT:
		res += W("int(");
		unParse(tree->childs[0], res);
		res += ')';
		break;
	case OP_TOSTRING + OP_COUNT:
		res += W("string(");
		unParse(tree->childs[0], res);
		res += ')';
		break;
	case OP_TONUMBER + OP_COUNT:
		res += W("number(");
		unParse(tree->childs[0], res);
		res += ')';
		break;
	case OP_CONST + OP_COUNT:
		res += W("const(");
		unParse(tree->childs[0], res);
		res += ')';
		break;
	case OP_TYPE + OP_COUNT:
		res += W("typeof(");
		unParse(tree->childs[0], res);
		res += ')';
		break;
	case OP_INSTANCE:
		unParse(tree->childs[0], res);
		res += W(" instanceof ");
		unParse(tree->childs[1], res);
		break;
	case OP_RESERVE:
	case OP_RESERVE + OP_COUNT:
		if (tree->childs.size() > 0)
			unParse(tree->childs[0], res);
		for (int i = 1; i < tree->childs.size(); i++)
		{
			res += ';';
			unParse(tree->childs[i], res);
		}
		break;
	case OP_WITH + OP_COUNT:
		res += W("with(");
		unParse(tree->childs[0], res);
		res += ')';
		unParse(tree->childs[1], res);
		break;
	case OP_INCONTEXTOF:
		unParse(tree->childs[0], res);
		res += W(" incontextof ");
		unParse(tree->childs[1], res);
		break;
	case OP_STATIC + OP_COUNT:
		res += W("static ") + tree->childs[0]->Node.var.asBKEStr()->getConstStr() + (char16_t)'=';
		unParse(tree->childs[1], res);
		break;
	default:
		//fatal error
		assert(0);
	}
	if (addbracket)
		res += ')';
	return;
#undef MAKE_TWO
}

void Bagel_Parser::getKeywordsWithPrefix(std::map<StringVal, PromptType>& result, const StringVal & prefix)
{
	for (auto &it : constmap)
	{
		if (it.first.length() >= prefix.length())
		{
			if (prefix.empty() || !memcmp(it.first.c_str(), prefix.c_str(), 2 * prefix.length()))
			{
				//see constvar as keywords
				result[it.first] = PromptType::API_KEYWORDS;
			}
		}
	}
	for (auto &it : opmap)
	{
		if (it.first.length() >= prefix.length())
		{
			if (prefix.empty() || !memcmp(it.first.c_str(), prefix.c_str(), 2 * prefix.length()))
			{
				//see constvar as keywords
				result[it.first] = PromptType::API_KEYWORDS;
			}
		}
	}
}

#define EXIST_CHILD(n) (tree->childs.size() > n && tree->childs[n])
#define EXIST_CHILD_DO(n, sentence) if(tree->childs.size() > n && tree->childs[n]){auto &subtree = tree->childs[n];sentence;}
#define EXIST_CHILD_DOALL(sentence) for(auto &subtree : tree->childs){if(!subtree)continue; sentence;}

Bagel_Var Bagel_AST_Analysis::_analysis(Bagel_AST * tree, Bagel_Closure * glo, Bagel_Closure * thiz, bool getaddr)
{
	if (!tree)
		return Bagel_Var();
	auto op = tree->Node.opcode;
	switch (op)
	{
	case OP_END:
	case OP_END + OP_COUNT:
		break;
	case OP_IF:
	case OP_IF + OP_COUNT:
	case OP_CHOOSE:
	case OP_DELETE + OP_COUNT:
	case OP_THROW + OP_COUNT:
	case OP_RESERVE + OP_COUNT:
		EXIST_CHILD_DOALL(_analysis(subtree, glo, thiz, false));
		break;
	case OP_QUICKFOR + OP_COUNT:
		EXIST_CHILD_DO(1, _analysis(subtree, glo, thiz, false));
		EXIST_CHILD_DO(2, _analysis(subtree, glo, thiz, false));
		EXIST_CHILD_DO(3, _analysis(subtree, glo, thiz, false));
		{
			auto *thiz2 = new Bagel_Closure(thiz);
			EXIST_CHILD_DO(0, auto name = subtree->Node.var.getString(); if (!name.empty()) thiz2->setMember(name, Bagel_Var()));
			EXIST_CHILD_DO(4, _analysis(subtree, glo, thiz2, false));
		}
		break;
	case OP_FOR + OP_COUNT:
	case OP_BLOCK + OP_COUNT:
	case OP_TRY + OP_COUNT:
		{
			auto *thiz2 = new Bagel_Closure(thiz);
			EXIST_CHILD_DOALL(_analysis(subtree, glo, thiz2, false));
		}
		break;
	case OP_WHILE + OP_COUNT:
		EXIST_CHILD_DO(0, _analysis(subtree, glo, thiz, false));
		{
			auto *thiz2 = new Bagel_Closure(thiz);
			EXIST_CHILD_DO(1, _analysis(subtree, glo, thiz2, false));
		}
		break;
	case OP_DO + OP_COUNT:
		{
			auto *thiz2 = new Bagel_Closure(thiz);
			EXIST_CHILD_DO(0, _analysis(subtree, glo, thiz2, false));
		}
		EXIST_CHILD_DO(1, _analysis(subtree, glo, thiz, false));
		break;
	case OP_FOREACH + OP_COUNT:
		EXIST_CHILD_DO(2, _analysis(subtree, glo, thiz, false));
		{
			auto *thiz2 = new Bagel_Closure(thiz);
			EXIST_CHILD_DO(0, auto name = subtree->Node.var.getString(); if (!name.empty()) thiz2->setMember(name, Bagel_Var()));
			EXIST_CHILD_DO(1, auto name = subtree->Node.var.getString(); if (!name.empty()) thiz2->setMember(name, Bagel_Var()));
			EXIST_CHILD_DO(3, _analysis(subtree, glo, thiz2, false));
		}
		break;
	case OP_CONSTVAR + OP_COUNT:
		return tree->Node.var;
	case OP_LITERAL + OP_COUNT:
		if (getaddr)
			return new Bagel_Pointer(thiz, tree->Node.var, false);
		else
			return thiz->getMember(tree->Node.var);
	case OP_FUNCTION + OP_COUNT:
		{
			if (tree->Node.var.getType() == VAR_FUNC)
			{
				auto func = tree->Node.var.forceAsFunc();
				if (func->func->code)
				{
					Bagel_AST_Analysis ana;
					ana.analysis(func->func->code, glo, new Bagel_Closure(thiz));
				#if PARSER_DEBUG
					func->func->returnvar = ana.ret;
				#endif
				}
				if (!func->name.empty())
				{
					thiz->varmap[func->name] = func;
				}
				func->setClosure(thiz);
				func->setSelf(thiz);
				return func;
			}
		}
		break;
	case OP_CLASS + OP_COUNT:
		{
			if (tree->childs.size() > 0 && tree->childs[0])
			{
				auto classname = tree->childs[0]->Node.var.getBKEStr();
				if (classname->empty())
					break;
				vector<Bagel_StringHolder> name;
				int i = 1;
				int s = tree->childs.size();
				for (; i < s; i++)
				{
					if (!tree->childs[i] || tree->childs[i]->Node.opcode != OP_LITERAL)
					{
						break;
					}
					name.push_back(tree->childs[i]->Node.var.getBKEStr());
				}
				Bagel_ClassDef *cla = NULL;
				try
				{
					Bagel_ClassDef::checkParent(name);
				}
				catch (Bagel_Except &)
				{
					return Bagel_Var();
				}
				cla = new Bagel_ClassDef(classname, name);
				glo->setMember(classname, cla);
				while (i < s)
				{
					auto subtree = tree->childs[i++];
					if (!subtree)
						continue;
					switch (subtree->Node.opcode)
					{
					case OP_VAR + OP_COUNT:
						for (int j = 1; j < subtree->childs.size(); j += 2)
						{
							if (!subtree->childs[j - 1])
								continue;
							auto name2 = subtree->childs[j - 1]->Node.var.getString();
							if (!name2.empty())
								cla->classvar[name2] = _analysis(subtree->childs[j], glo, thiz, false);
						}
						break;
					case OP_STATIC + OP_COUNT:
						for (int j = 1; j < subtree->childs.size(); j += 2)
						{
							if (!subtree->childs[j - 1])
								continue;
							auto name2 = subtree->childs[j - 1]->Node.var.getString();
							if (!name2.empty())
								cla->varmap[name2] = _analysis(subtree->childs[j], glo, thiz, false);
						}
						break;
					case OP_FUNCTION + OP_COUNT:
					case OP_PROPGET + OP_COUNT:
					case OP_PROPSET + OP_COUNT:
						_analysis(subtree, glo, cla, false);
						break;
					}
				}
			}
		}
		break;
	case OP_PROPGET + OP_COUNT:
		{
			if (tree->childs.size() >= 1)
			{
				if (!tree->childs[0])
					break;
				auto name = tree->childs[0]->Node.var.getBKEStr();
				if (!name->empty())
				{
					auto &v = thiz->varmap[name];
					if (v.getType() != VAR_PROP)
					{
						v = new Bagel_Prop();
						v.forceAsProp()->name = name;
						v.forceAsProp()->setClosure(thiz);
						v.forceAsProp()->setSelf(thiz);
					}
					v.forceAsProp()->funcget = tree->Node.var.forceAsObject<Bagel_FunctionCode>();
				}
			}
		}
		break;
	case OP_PROPSET + OP_COUNT:
		{
			if (tree->childs.size() >= 1)
			{
				if (!tree->childs[0])
					break;
				auto name = tree->childs[0]->Node.var.getString();
				if (!name.empty())
				{
					auto &v = thiz->varmap[name];
					if (v.getType() != VAR_PROP)
					{
						v = new Bagel_Prop();
						v.forceAsProp()->name = name;
						v.forceAsProp()->setClosure(thiz);
						v.forceAsProp()->setSelf(thiz);
					}
					v.forceAsProp()->funcset = tree->Node.var.forceAsObject<Bagel_FunctionCode>();
				}
			}
		}
		break;
	case OP_PROPERTY + OP_COUNT:
		{
			if (tree->childs.size() >= 2)
			{
				auto name = tree->Node.var.getBKEStr();
				if (!name->empty())
				{
					auto &v = thiz->varmap[name];
					if (v.getType() != VAR_PROP)
					{
						v = new Bagel_Prop();
						v.forceAsProp()->name = name;
						v.forceAsProp()->setClosure(thiz);
						v.forceAsProp()->setSelf(thiz);
					}
					if (tree->childs[0])
					{
						v.forceAsProp()->funcget = tree->childs[0]->Node.var.forceAsObject<Bagel_FunctionCode>();
					}
					if (tree->childs[1])
					{
						v.forceAsProp()->funcset = tree->childs[1]->Node.var.forceAsObject<Bagel_FunctionCode>();
					}
				}
			}
		}
		break;
	case OP_ADD:
	case OP_SUB:
	case OP_MUL:
	case OP_DIV:
	case OP_MOD:
	case OP_POW:
	case OP_ADD + OP_COUNT:
	case OP_SUB + OP_COUNT:
	case OP_FASTAND:
	case OP_FASTOR:
		{
			Bagel_Var v;
			EXIST_CHILD_DOALL(if (v.isVoid()) v.forceSet(_analysis(subtree, glo, thiz, false)));
			return v;
		}
	case OP_EQUAL:
	case OP_NEQUAL:
	case OP_EEQUAL:
	case OP_NNEQUAL:
	case OP_LARGER:
	case OP_SMALLER:
	case OP_LE:
	case OP_SE:
	case OP_AND:
	case OP_OR:
	case OP_NOT + OP_COUNT:
	case OP_BITAND:
	case OP_BITOR:
	case OP_BITNOT + OP_COUNT:
		EXIST_CHILD_DOALL(_analysis(subtree, glo, thiz, false));
		return false;
	case OP_SET:
	case OP_SETADD:
	case OP_SETSUB:
	case OP_SETMUL:
	case OP_SETDIV:
	case OP_SETMOD:
	case OP_SETPOW:
		{
			Bagel_Var v1, v2;
			EXIST_CHILD_DO(1, v2 = _analysis(subtree, glo, thiz, false));
			try
			{
				EXIST_CHILD_DO(0, v1.forceSet(_analysis(subtree, glo, thiz, true)));
				if (v1.getType() == VAR_POINTER)
				{
					auto p = v1.forceAsPointer();
					p->set(v2);
					if (getaddr)
						return v1;
					else
						return p->get();
				}
			}
			catch (Bagel_Except &)
			{
			}
		}
		break;
	case OP_SETSET:		//|=
		{
			Bagel_Var v1, v2;
			EXIST_CHILD_DO(1, v2 = _analysis(subtree, glo, thiz, false));
			try
			{
				EXIST_CHILD_DO(0, v1.forceSet(_analysis(subtree, glo, thiz, true)));
				if (v1.getType() == VAR_POINTER)
				{
					auto p = v1.forceAsPointer();
					if (p->get().isVoid())
						p->set(v2);
					if (getaddr)
						return v1;
					else
						return p->get();
				}
			}
			catch (Bagel_Except &)
			{
			}
		}
		break;
	case OP_DOT + OP_COUNT:
		{
			Bagel_StringHolder name;
			if (tree->childs.size() > 0 && tree->childs[0])
			{
				auto subtree = tree->childs[0];
				if (subtree->Node.opcode == OP_LITERAL + OP_COUNT)
				{
					name = subtree->Node.var.getBKEStr();
				}
			}
			if (!name.empty())
			{
				if (getaddr)
					return new Bagel_Pointer(withvar, name, false);
				else
					return withvar[name];
			}
		}
		break;
	case OP_SELFINC:
	case OP_SELFDEC:
	case OP_SELFINC + OP_COUNT:
	case OP_SELFDEC + OP_COUNT:
		{
			Bagel_Var v1;
			EXIST_CHILD_DO(0, v1.forceSet(_analysis(subtree, glo, thiz, true)));
			try
			{
				if (v1.getType() == VAR_POINTER)
				{
					auto p = v1.forceAsPointer();
					p->set(0);
					return 0;
				}
			}
			catch (Bagel_Except&)
			{
				return 0;
			}
		}
		break;
	case OP_BRACKET:
		{
			Bagel_Var v;
			EXIST_CHILD_DO(0, v = _analysis(subtree, glo, thiz, false));
			if (v.getType() == VAR_CLASS)
			{
				return new Bagel_Class(v.forceAsClass()->defclass);
			}
			else if (v.getType() == VAR_CLASSDEF)
			{
				return new Bagel_Class(v.forceAsClassDef());
			}
			else if (v.getType() == VAR_FUNC)
			{
				auto func = v.forceAsFunc()->func;
			#if PARSER_DEBUG
				if (func->returnvar.size() == 1)
					return func->returnvar[0];
			#endif
			}
		}
		break;
	case OP_ARRAY + OP_COUNT:
		{
			auto arr = new Bagel_Array();
			EXIST_CHILD_DOALL(arr->pushMember(_analysis(subtree, glo, thiz, false)));
			return arr;
		}
		break;
	case OP_NULLARRAY + OP_COUNT:
		{
			auto arr = new Bagel_Array();
			return arr;
		}
		break;
	case OP_DIC + OP_COUNT:
		{
			auto dic = new Bagel_Dic();
			for (int i = 1; i < tree->childs.size(); i += 2)
			{
				if (!tree->childs[i - 1])
					continue;
				auto name = tree->childs[i - 1]->Node.var.getString();
				if (!name.empty())
					dic->varmap[name] = _analysis(tree->childs[i], glo, thiz, false);
			}
			return dic;
		}
		break;
	case OP_NULLDIC + OP_COUNT:
		{
			auto arr = new Bagel_Dic();
			return arr;
		}
		break;
	case OP_ARRAY:
	case OP_DOT:
		{
			Bagel_Var v1;
			Bagel_StringHolder name;
			EXIST_CHILD_DO(0, v1 = _analysis(subtree, glo, thiz, false));
			if (tree->childs.size() > 1 && tree->childs[1])
			{
				auto subtree = tree->childs[1];
				if (subtree->Node.opcode == OP_LITERAL + OP_COUNT)
				{
					name = subtree->Node.var.getBKEStr();
				}
			}
			try
			{
				if (!name.empty())
				{
					if (getaddr)
						return new Bagel_Pointer(v1, name, false);
					else
						return v1[name];
				}
			}
			catch (Bagel_Except&)
			{
			}
		}
		break;
	case OP_CONTINUE + OP_COUNT:
	case OP_BREAK + OP_COUNT:
	case OP_STATIC + OP_COUNT:
		break;
	case OP_RETURN + OP_COUNT:
		EXIST_CHILD_DO(0, auto v = _analysis(subtree, glo, thiz, false); if (!v.isVoid())ret.push_back(v););
		break;
	case OP_VAR + OP_COUNT:
		for (int j = 1; j < tree->childs.size(); j += 2)
		{
			if (!tree->childs[j - 1])
				continue;
			auto name = tree->childs[j - 1]->Node.var.getString();
			if (!name.empty())
				thiz->varmap[name] = _analysis(tree->childs[j], glo, thiz, false);
		}
		break;
	case OP_THIS + OP_COUNT:
		return thiz;
	case OP_SUPER + OP_COUNT:
		break;
	case OP_GLOBAL + OP_COUNT:
		return glo;
	case OP_TOINT + OP_COUNT:
	case OP_TONUMBER + OP_COUNT:
		EXIST_CHILD_DO(0, _analysis(subtree, glo, thiz, false));
		return 0;
	case OP_TOSTRING + OP_COUNT:
		EXIST_CHILD_DO(0, _analysis(subtree, glo, thiz, false));
		return W("");
	case OP_CONST + OP_COUNT:
		EXIST_CHILD_DO(0, return _analysis(subtree, glo, thiz, false));
		break;
	case OP_TYPE + OP_COUNT:
		EXIST_CHILD_DO(0, return _analysis(subtree, glo, thiz, false).getTypeBKEString());
		break;
	case OP_INSTANCE + OP_COUNT:
	case OP_INCONTEXTOF:
		EXIST_CHILD_DOALL(_analysis(subtree, glo, thiz, false));
		return 0;
	case OP_WITH + OP_COUNT:
		{
			auto old = withvar;
			try
			{
				EXIST_CHILD_DO(0, withvar = _analysis(subtree, glo, thiz, false));
				EXIST_CHILD_DO(1, _analysis(subtree, glo, thiz, false));
			}
			catch (Bagel_Except &)
			{
			}
			withvar = old;
		}
		break;
	case OP_ENUM + OP_COUNT:
		{
			Bagel_StringHolder name;
			EXIST_CHILD_DO(0, name = _analysis(subtree, glo, thiz, false).getBKEStr());
			BKE_hashmap<Bagel_StringHolder, Bagel_Var> *m = NULL;
			if (name.empty())
			{
				m = &thiz->varmap;
			}
			else
			{
				auto dic = new Bagel_Dic();
				thiz->varmap[name] = dic;
				m = &dic->varmap;
			}
			for (int i = 2; i < tree->childs.size(); i += 2)
			{
				if (!tree->childs[i - 1])
					continue;
				auto name = tree->childs[i - 1]->Node.var.getString();
				if (!name.empty())
					(*m)[name] = _analysis(tree->childs[i], glo, thiz, false);
			}
		}
		break;
	}
	return Bagel_Var();
}

/*
返回true表示当前tree的pos已超过要分析的pos，false表示需要继续分析
tree：要分析的AST
glo：伪global闭包，global关键字重定向到这个闭包
thiz：当前tree的闭包
pos：要分析到的位置
out：输出的候选列表
getaddr：true时，当前分析结果写入outvar中。
*/
bool Bagel_AST_Analysis::_analysisVar(Bagel_AST * tree, Bagel_Closure * glo, Bagel_Closure * thiz, int pos, std::map<StringVal, PromptType> & out, Bagel_Var &outvar, bool getaddr)
{
	if (!tree)
		return false;
	auto op = tree->Node.opcode;
	if (tree->Node.pos2 < pos)
	{
		if (getaddr)
			outvar.forceSet(_analysis(tree, glo, thiz, true));
		else
			_analysis(tree, glo, thiz, false);
		return false;
	}
	if (tree->getFirstPos() > pos)
	{
		return true;
	}
	switch (op)
	{
	case OP_END:
	case OP_END + OP_COUNT:
		break;
	case OP_IF:
	case OP_IF + OP_COUNT:
	case OP_CHOOSE:
	case OP_DELETE + OP_COUNT:
	case OP_THROW + OP_COUNT:
	case OP_RESERVE + OP_COUNT:
		EXIST_CHILD_DOALL(
			if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
				return true;
		);
		break;
	case OP_QUICKFOR + OP_COUNT:
		EXIST_CHILD_DO(
			1,
			if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
				return true;
		);
		EXIST_CHILD_DO(
			2,
			if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
				return true;
		);
		EXIST_CHILD_DO(
			3,
			if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
				return true;
		);
		{
			auto *thiz2 = new Bagel_Closure(thiz);
			EXIST_CHILD_DO(0, auto name = subtree->Node.var.getString(); if (!name.empty()) thiz2->setMember(name, Bagel_Var()));
			EXIST_CHILD_DO(
				4,
				if (_analysisVar(subtree, glo, thiz2, pos, out, outvar, false))
					return true;
			);
		}
		break;
	case OP_FOR + OP_COUNT:
	case OP_BLOCK + OP_COUNT:
	case OP_TRY + OP_COUNT:
		{
			auto *thiz2 = new Bagel_Closure(thiz);
			EXIST_CHILD_DOALL(
				if (_analysisVar(subtree, glo, thiz2, pos, out, outvar, false))
					return true;
			);
		}
		break;
	case OP_WHILE + OP_COUNT:
		EXIST_CHILD_DO(0, 
			if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
				return true;
		);
		{
			auto *thiz2 = new Bagel_Closure(thiz);
			EXIST_CHILD_DO(1,
				if (_analysisVar(subtree, glo, thiz2, pos, out, outvar, false))
					return true;
			);
		}
		break;
	case OP_DO + OP_COUNT:
		{
			auto *thiz2 = new Bagel_Closure(thiz);
			EXIST_CHILD_DO(0,
				if (_analysisVar(subtree, glo, thiz2, pos, out, outvar, false))
					return true;
			);
		}
		EXIST_CHILD_DO(1,
			if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
				return true;
		);
		break;
	case OP_FOREACH + OP_COUNT:
		EXIST_CHILD_DO(2,
			if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
				return true;
		);
		{
			auto *thiz2 = new Bagel_Closure(thiz);
			EXIST_CHILD_DO(0, auto name = subtree->Node.var.getString(); if (!name.empty()) thiz2->setMember(name, Bagel_Var()));
			EXIST_CHILD_DO(1, auto name = subtree->Node.var.getString(); if (!name.empty()) thiz2->setMember(name, Bagel_Var()));
			EXIST_CHILD_DO(3,
				if (_analysisVar(subtree, glo, thiz2, pos, out, outvar, false))
					return true;
			);
		}
		break;
	case OP_CONSTVAR + OP_COUNT:
		if (getaddr)
			outvar = tree->Node.var;
		//对于常量后面不给出任何推荐
		out.clear();
		return true;
	case OP_LITERAL + OP_COUNT:
		//列出thiz下面可能的做候选字
		{
			std::map<StringVal, PromptType> outset;
			out.clear();
			thiz->getAllVariablesWithPrefix(outset, tree->Node.var.getString());
			for (auto &i : outset)
			{
				out.emplace(i.first, i.second);
			}
		}
		if (getaddr)
		{
			outvar = new Bagel_Pointer(thiz, tree->Node.var, false);
		}
		return true;
	case OP_FUNCTION + OP_COUNT:
		{
			if (tree->Node.var.getType() == VAR_FUNC)
			{
				auto func = tree->Node.var.forceAsFunc();
				if (func->func->code)
				{
					auto thiz2 = new Bagel_Closure(thiz);
					for (auto &p : func->func->paramnames)
						thiz2->setMember(p, Bagel_Var());
					if (_analysisVar(func->func->code, glo, thiz2, pos, out, outvar, false))
						return true;
				}
				if (!func->name.empty())
				{
					thiz->varmap[func->name] = func;
				}
				func->setClosure(thiz);
				func->setSelf(thiz);
				break;
			}
		}
		break;
	case OP_CLASS + OP_COUNT:
		{
			if (tree->childs.size() > 0 && tree->childs[0])
			{
				auto classname = tree->childs[0]->Node.var.getString();
				Bagel_StringHolder name;
				vector<Bagel_ClassDef*> parents;
				int i = 1;
				int s = tree->childs.size();
				for (; i < s; i++)
				{
					if (!tree->childs[i] || tree->childs[i]->Node.opcode != OP_LITERAL)
					{
						break;
					}
					if (tree->childs[i]->Node.pos2 == pos)
					{
						//这是最后一个位置，我们给出候选字
						std::map<StringVal, PromptType> outset;
						out.clear();
						thiz->getAllVariablesWithPrefixAndType(outset, tree->Node.var.getString(), VAR_CLASSDEF);
						for (auto &i : outset)
						{
							out.emplace(i.first, i.second);
						}
						return true;
					}
					name = tree->childs[i]->Node.var.getBKEStr();
					//忽略未定义的classname
					if (thiz->getMember(name).getType() == VAR_CLASSDEF)
					{
						parents.push_back(thiz->getMember(name).forceAsClassDef());
					}
				}
				Bagel_ClassDef *cla = NULL;
				cla = new Bagel_ClassDef(classname, parents);
				for (int i2 = i; i2 < s; i2++)
				{
					//reg all members
					auto subtree = tree->childs[i++];
					if (!subtree)
						continue;
					switch (subtree->Node.opcode)
					{
					case OP_VAR + OP_COUNT:
						for (int j = 1; j < subtree->childs.size(); j += 2)
						{
							if (!subtree->childs[j - 1])
								continue;
							auto name2 = subtree->childs[j - 1]->Node.var.getString();
							if (!name2.empty())
							{
								cla->classvar[name2] = Bagel_Var();
							}
						}
						break;
					case OP_STATIC + OP_COUNT:
						for (int j = 1; j < subtree->childs.size(); j += 2)
						{
							if (!subtree->childs[j - 1])
								continue;
							auto name2 = subtree->childs[j - 1]->Node.var.getString();
							if (!name2.empty())
							{
								cla->varmap[name2] = Bagel_Var();
							}
						}
						break;
					case OP_FUNCTION + OP_COUNT:
					case OP_PROPGET + OP_COUNT:
					case OP_PROPSET + OP_COUNT:
						_analysis(subtree, glo, cla, false);
						break;
					}
				}
				while (i < s)
				{
					auto subtree = tree->childs[i++];
					if (!subtree)
						continue;
					switch (subtree->Node.opcode)
					{
					case OP_VAR + OP_COUNT:
						for (int j = 1; j < subtree->childs.size(); j += 2)
						{
							if (!subtree->childs[j - 1])
								continue;
							auto name2 = subtree->childs[j - 1]->Node.var.getString();
							if (!name2.empty())
							{
								if (_analysisVar(subtree->childs[j], glo, cla, pos, out, outvar, true))
									return true;
								cla->classvar[name2] = outvar;
							}
						}
						break;
					case OP_STATIC + OP_COUNT:
						for (int j = 1; j < subtree->childs.size(); j += 2)
						{
							if (!subtree->childs[j - 1])
								continue;
							auto name2 = subtree->childs[j - 1]->Node.var.getString();
							if (!name2.empty())
							{
								if (_analysisVar(subtree->childs[j], glo, cla, pos, out, outvar, true))
									return true;
								cla->varmap[name2] = outvar;
							}
						}
						break;
					case OP_FUNCTION + OP_COUNT:
					case OP_PROPGET + OP_COUNT:
					case OP_PROPSET + OP_COUNT:
						if (_analysisVar(subtree, glo, cla, pos, out, outvar, true))
							return true;
						break;
					}
				}
			}
		}
		break;
	case OP_PROPGET + OP_COUNT:
		{
			if (tree->childs.size() >= 2)
			{
				StringVal name;
				if(tree->childs[0])
					name = tree->childs[0]->Node.var.getString();
				if (_analysisVar(tree->childs[1], glo, new Bagel_Closure(thiz), pos, out, outvar, false))
					return true;
				if (!name.empty() && tree->childs[1])
				{
					auto &v = thiz->varmap[name];
					if (v.getType() != VAR_PROP)
					{
						v = new Bagel_Prop();
						v.forceAsProp()->name = name;
						v.forceAsProp()->setClosure(thiz);
						v.forceAsProp()->setSelf(thiz);
					}
					v.forceAsProp()->addPropGet(tree->childs[1]);
				}
			}
		}
		break;
	case OP_PROPSET + OP_COUNT:
		{
			if (tree->childs.size() >= 3)
			{
				StringVal name;
				StringVal name2;
				if(tree->childs[0])
					auto name = tree->childs[0]->Node.var.getString();
				if(tree->childs[1])
					auto name2 = tree->childs[1]->Node.var.getString();
				auto thiz2 = new Bagel_Closure(thiz);
				if(!name2.empty())
					thiz2->setMember(name2, Bagel_Var());
				if (_analysisVar(tree->childs[1], glo, thiz2, pos, out, outvar, false))
					return true;
				if (!name.empty() && !name2.empty() && tree->childs[2])
				{
					auto &v = thiz->varmap[name];
					if (v.getType() != VAR_PROP)
					{
						v = new Bagel_Prop();
						v.forceAsProp()->name = name;
						v.forceAsProp()->setClosure(thiz);
						v.forceAsProp()->setSelf(thiz);
					}
					v.forceAsProp()->addPropSet(name2, tree->childs[2]);
				}
			}
		}
		break;
	case OP_PROPERTY + OP_COUNT:
		{
			if (tree->childs.size() >= 2)
			{
				bool res = false;
				if (tree->childs[0] && tree->childs[0]->Node.var.getType() == VAR_FUNCCODE_P)
				{
					auto code = tree->childs[0]->Node.var.forceAsObject<Bagel_FunctionCode>();
					auto thiz2 = new Bagel_Closure(thiz);
					res |= _analysisVar(code->code, glo, thiz2, pos, out, outvar, false);
				}
				if (tree->childs[1] && tree->childs[1]->Node.var.getType() == VAR_FUNCCODE_P)
				{
					auto code = tree->childs[1]->Node.var.forceAsObject<Bagel_FunctionCode>();
					auto thiz2 = new Bagel_Closure(thiz);
					if(!code->paramnames.empty() && !code->paramnames[0].empty())
						thiz2->setMember(code->paramnames[0], Bagel_Var());
					res |= _analysisVar(code->code, glo, thiz2, pos, out, outvar, false);
				}
				return res;
			}
		}
		break;
	case OP_ADD:
	case OP_SUB:
	case OP_MUL:
	case OP_DIV:
	case OP_MOD:
	case OP_POW:
	case OP_ADD + OP_COUNT:
	case OP_SUB + OP_COUNT:
	case OP_FASTAND:
	case OP_FASTOR:
		{
			EXIST_CHILD_DOALL(
				if (_analysisVar(subtree, glo, thiz, pos, out, outvar, true))
					return true;
			);
			break;
		}
	case OP_EQUAL:
	case OP_NEQUAL:
	case OP_EEQUAL:
	case OP_NNEQUAL:
	case OP_LARGER:
	case OP_SMALLER:
	case OP_LE:
	case OP_SE:
	case OP_AND:
	case OP_OR:
	case OP_NOT + OP_COUNT:
	case OP_BITAND:
	case OP_BITOR:
	case OP_BITNOT + OP_COUNT:
		{
			if(getaddr)
				outvar = false;
			break;
		}
	case OP_SET:
	case OP_SETADD:
	case OP_SETSUB:
	case OP_SETMUL:
	case OP_SETDIV:
	case OP_SETMOD:
	case OP_SETPOW:
	case OP_SETSET:		//|=
		{
			EXIST_CHILD_DOALL(
				if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
					return true;
			);
		}
		break;
	case OP_DOT + OP_COUNT:
		{
			//列出withvar下面可能的做候选字
			Bagel_StringHolder name;
			EXIST_CHILD_DO(0, 
				if (subtree->Node.pos <= pos && subtree->Node.pos2 > pos)
					name = subtree->Node.var.asString().substr(pos - subtree->Node.pos);
			);
			std::map<StringVal, PromptType> outset;
			out.clear();
			if (withvar.getType() == VAR_POINTER)
			{
				withvar = withvar.forceAsPointer()->get();
			}
			switch (withvar.getType())
			{
			case VAR_CLO:
			case VAR_CLASS:
			case VAR_CLASSDEF:
				withvar.forceAsClosure()->getAllVariablesWithPrefix(outset, name);
				break;
			default:
				{
					auto cla = _globalStructures.typeclass[outvar.getType()];
					if (cla)
					{
						cla->getAllVariablesWithPrefix(outset, name);
					}
					if (outvar.getType() == VAR_DIC)
					{
						for (auto &s : outvar.forceAsDic()->varmap)
						{
							if (s.first.beginWith(name))
								outset.emplace(s.first, getPromptTypeFromVar(s.second));
						}
					}
				}
				break;
			}
			for (auto &i : outset)
			{
				out.emplace(i.first, i.second);
			}
		}
		return true;
	case OP_SELFINC:
	case OP_SELFDEC:
	case OP_SELFINC + OP_COUNT:
	case OP_SELFDEC + OP_COUNT:
		{
			EXIST_CHILD_DO(0,
				if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
					return true;
			);
		}
		break;
	case OP_BRACKET:
		{
			EXIST_CHILD_DOALL(
				if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
					return true;
			);
		}
		break;
	case OP_ARRAY + OP_COUNT:
		{
			Bagel_Array *arr = new Bagel_Array();
			EXIST_CHILD_DOALL(
				outvar.setVoid();
				if (_analysisVar(subtree, glo, thiz, pos, out, outvar, true))
					return true;
				arr->pushMember(outvar);
			);
			if (getaddr)
				outvar = arr;
		}
		break;
	case OP_DIC + OP_COUNT:
		{
			Bagel_Dic* dic = new Bagel_Dic();
			for (int i = 1; i < tree->childs.size(); i += 2)
			{
				if (!tree->childs[i - 1])
					continue;
				if (_analysisVar(tree->childs[i - 1], glo, thiz, pos, out, outvar, false))
					return true;
				outvar.setVoid();
				if (_analysisVar(tree->childs[i], glo, thiz, pos, out, outvar, true))
					return true;
				auto name = tree->childs[i - 1]->Node.var.getString();
				if (!name.empty())
					dic->varmap[name] = outvar;
			}
			if (getaddr)
				outvar = dic;
		}
		break;
	case OP_NULLDIC + OP_COUNT:
		{
			if (getaddr)
				outvar = new Bagel_Dic();
		}
		break;
	case OP_ARRAY:
		{
			EXIST_CHILD_DOALL(
				if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
					return true;
			);
		}
		break;
	case OP_DOT:
		{
			EXIST_CHILD_DO(0,
				if (_analysisVar(subtree, glo, thiz, pos, out, outvar, true))
					return true;
			);
			Bagel_StringHolder name;
			EXIST_CHILD_DO(1,
				if (subtree->Node.pos <= pos && subtree->Node.pos2 > pos)
					name = subtree->Node.var.asString().substr(pos - subtree->Node.pos);
			);
			std::map<StringVal, PromptType> outset;
			out.clear();
			if (outvar.getType() == VAR_POINTER)
			{
				outvar.forceSet(outvar.forceAsPointer()->get());
			}
			switch (outvar.getType())
			{
			case VAR_CLO:
			case VAR_CLASS:
			case VAR_CLASSDEF:
				outvar.forceAsClosure()->getAllVariablesWithPrefix(outset, name);
				break;
			default:
				{
					auto cla = _globalStructures.typeclass[outvar.getType()];
					if (cla)
					{
						cla->getAllVariablesWithPrefix(outset, name);
					}
					if (outvar.getType() == VAR_DIC)
					{
						for (auto &s : outvar.forceAsDic()->varmap)
						{
							if (s.first.beginWith(name))
								outset.emplace(s.first, getPromptTypeFromVar(s.second));
						}
					}
				}
				break;
			}
			for (auto &i : outset)
			{
				out.emplace(i.first, i.second);
			}
		}
		break;
	case OP_CONTINUE + OP_COUNT:
	case OP_BREAK + OP_COUNT:
	case OP_STATIC + OP_COUNT:
		break;
	case OP_RETURN + OP_COUNT:
		{
			EXIST_CHILD_DO(0,
				if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
					return true;
			);
		}
		break;
	case OP_VAR + OP_COUNT:
		for (int i = 1; i < tree->childs.size(); i += 2)
		{
			if (!tree->childs[i - 1])
				continue;
			if (_analysisVar(tree->childs[i - 1], glo, thiz, pos, out, outvar, false))
				return true;
			outvar.setVoid();
			if (_analysisVar(tree->childs[i], glo, thiz, pos, out, outvar, true))
				return true;
			auto name = tree->childs[i - 1]->Node.var.getString();
			if (!name.empty())
				thiz->varmap[name] = outvar;
		}
		break;
	case OP_THIS + OP_COUNT:
		if (getaddr)
		{
			outvar.setVoid();
			auto cla = thiz->getThisClosure();
			if (cla)
			{
				outvar = cla;
			}
		}
		break;
	case OP_SUPER + OP_COUNT:
		if (getaddr)
		{
			outvar.setVoid();
			auto cla = thiz->getThisClosure();
			if (cla)
			{
				if (cla->vt == VAR_CLASS)
					cla = static_cast<Bagel_Class*>(cla)->defclass;
				Bagel_ClassDef *def = static_cast<Bagel_ClassDef*>(cla);
				if (def->parents.size() == 1)
				{
					outvar = def->parents[0];
				}
			}
		}
		break;
	case OP_GLOBAL + OP_COUNT:
		if (getaddr)
			outvar = glo;
		break;
	case OP_TOINT + OP_COUNT:
	case OP_TONUMBER + OP_COUNT:
	case OP_TOSTRING + OP_COUNT:
	case OP_CONST + OP_COUNT:
	case OP_TYPE + OP_COUNT:
	case OP_INSTANCE + OP_COUNT:
	case OP_INCONTEXTOF:
		EXIST_CHILD_DOALL(
			if (_analysisVar(subtree, glo, thiz, pos, out, outvar, false))
				return true;
		);
		break;
	case OP_WITH + OP_COUNT:
		{
			auto old = withvar;
			EXIST_CHILD_DO(0,
				if (_analysisVar(subtree, glo, thiz, pos, out, outvar, true))
					return true;
			);
			withvar = outvar;
			bool ret = false;
			EXIST_CHILD_DO(1, 
				ret = _analysisVar(subtree, glo, thiz, pos, out, outvar, false);
			);
			withvar = old;
			return ret;
		}
		break;
	case OP_ENUM + OP_COUNT:
		{
			Bagel_StringHolder name;
			EXIST_CHILD_DO(0, name = subtree->Node.var);
			BKE_hashmap<Bagel_StringHolder, Bagel_Var> *m = NULL;
			if (name.empty())
			{
				m = &thiz->varmap;
			}
			else
			{
				auto dic = new Bagel_Dic();
				thiz->varmap[name] = dic;
				m = &dic->varmap;
			}
			for (int i = 2; i < tree->childs.size(); i += 2)
			{
				if (!tree->childs[i - 1])
					continue;
				if (_analysisVar(tree->childs[i - 1], glo, thiz, pos, out, outvar, false))
					return true;
				outvar.setVoid();
				if (_analysisVar(tree->childs[i], glo, thiz, pos, out, outvar, true))
					return true;
				auto name = tree->childs[i - 1]->Node.var.getString();
				if (!name.empty())
					(*m)[name] = outvar;
			}
		}
		break;
	}
	return false;
}

void Bagel_AST_Analysis::analysis(Bagel_AST * tree, Bagel_Closure * glo, Bagel_Closure * thiz)
{
	GC_Locker lock;
	withvar = glo;
	_analysis(tree, glo, thiz, false);
}

Bagel_Var Bagel_AST_Analysis::analysisVar(Bagel_AST * tree, Bagel_Closure * glo, Bagel_Closure * thiz, int pos, std::map<StringVal, PromptType> & out)
{
	GC_Locker lock;
	withvar = glo;
	Bagel_Var v;
	out.clear();
	_analysisVar(tree, glo, thiz, pos, out, v, false);
	return v;
}
