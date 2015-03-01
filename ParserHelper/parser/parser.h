#pragma once

#include "BKE_variable.h"
#include "vcode.h"
#include "extend.h"
#include "defines.h"

extern wchar_t OP_CODE[][15];

class Parser;

typedef void (Parser::*OP_Function)(BKE_bytree**);
typedef BKE_Variable& (Parser::*OP_Runner)(BKE_bytree *node, BKE_VarClosure *_this, BKE_array<BKE_Variable> &tmpvars);

//b is just a comment
#define NUD_FUNCTION(a,b) BKE_Variable& nudrunner_##a(BKE_bytree *tree, BKE_VarClosure *_this, BKE_array<BKE_Variable> &tmpvars)
#define LED_FUNCTION(a,b) BKE_Variable& ledrunner_##a(BKE_bytree *tree, BKE_VarClosure *_this, BKE_array<BKE_Variable> &tmpvars)
#define NUD_FUNC(a) BKE_Variable& Parser::nudrunner_##a(BKE_bytree *tree, BKE_VarClosure *_this, BKE_array<BKE_Variable> &tmpvars)
#define LED_FUNC(a) BKE_Variable& Parser::ledrunner_##a(BKE_bytree *tree, BKE_VarClosure *_this, BKE_array<BKE_Variable> &tmpvars)

class Parser
{
	friend class BKE_FunctionCode;
	friend class BKE_VarFunction;

private:
	BKE_hashmap<wstring, BKE_Variable> constmap;
	BKE_hashmap<wstring, BKE_opcode> opmap;
	//vector<BKE_Variable> tempvars;
	BKE_Node token, next;
	BKE_Variable none;

	BKE_VarClosure *p;
	const wchar_t *exp;
	wchar_t *curpos;
	bkplong expsize;
	bkplong runpos;
	bool forcequit;
	bool NextIsBareWord;

	vector<Var_Except> errorstack;

	wstring errorMSG;
	
	OP_Function funclist[OP_COUNT * 2];
	OP_Runner runner[OP_COUNT * 2];
	bkplong bp[OP_COUNT * 2];
	char head[OP_COUNT];

	OP_Function nudlist[OP_COUNT];
	OP_Function ledlist[OP_COUNT];
	OP_Runner nud[OP_COUNT];
	OP_Runner led[OP_COUNT];

	bkplong lbp[2 * OP_COUNT];
	bkplong *pre;

#if PARSER_DEBUG
	vector<bkplong> linestartpos;
#endif
	void setupDebugInfo()
	{
#if PARSER_DEBUG
		linestartpos.clear();
		const wchar_t *start = exp;
		wchar_t ch;
		linestartpos.push_back(0);
		while ((ch = *(start++))!=0)
		{
			if (ch == 0x0A)
			{
				linestartpos.push_back(start - exp);
			}
			else if (ch == 0x0D)
			{
				if (*start == 0x0A)
				{
					start++;
					linestartpos.push_back(start - exp);
				}
				else
					linestartpos.push_back(start - exp);
			}
		}
#endif
	}

	wstring getPos(bkplong pos)
	{
		wstring s;
#if PARSER_DEBUG
		auto it = upper_bound(linestartpos.begin(), linestartpos.end(), pos);
		s += L"第" + bkpInt2Str(it - linestartpos.begin()) + L"行，第" + bkpInt2Str((int)((it == linestartpos.end() ? pos : *it) - (*(it - 1)) + 1)) + L"处的";
#endif
		return s;
	};

	inline bool MatchFunc(const wchar_t *a, wchar_t **c)
	{
		wchar_t *b;
		b = *c;
		while ((*a) && (*b))
		{
			if ((*a) != (*b))
				return false;
			b++;
			a++;
		}
		*c = b;
		return true;
	}

	inline bool isVar(BKE_bytree *t)
	{
		return (t->Node.opcode == OP_LITERAL + OP_COUNT) || (t->Node.opcode == OP_DOT) || (t->Node.opcode == OP_DOT + OP_COUNT) || (t->Node.opcode == OP_ARRAY && t->childs.size() == 2);
	}

	void init();
	void init2(const wchar_t *e, BKE_VarClosure *_this)
	{
		this->exp = e;
		curpos = const_cast<wchar_t *>(e);
		expsize = wcslen(e);
		p = _this;
		runpos = 0;
		NextIsBareWord = false;
		setupDebugInfo();
	}
	void init2(const wstring &e, BKE_VarClosure *_this)
	{
		this->exp = e.c_str();
		curpos = const_cast<wchar_t *>(exp);
		expsize = e.size();
		p = _this;
		runpos = 0;
		NextIsBareWord = false;
		setupDebugInfo();
	}

	void expression(BKE_bytree** tree, int rbp=0);
	void readToken();
  
	void nud_this(BKE_bytree **tree);
	void nud_var(BKE_bytree **tree);
	void nud_delete(BKE_bytree **tree);
	void nud_one(BKE_bytree **tree);
	void nud_bracket(BKE_bytree **tree);
	void nud_if(BKE_bytree **tree);
	void nud_array(BKE_bytree **tree);
	void nud_dic(BKE_bytree **tree);
	void nud_block(BKE_bytree **tree);
	void nud_label(BKE_bytree **tree);
	void nud_for(BKE_bytree **tree);
	void nud_foreach(BKE_bytree **tree);
	void nud_do(BKE_bytree **tree);
	void nud_while(BKE_bytree **tree);
	void nud_function(BKE_bytree **tree);
	void nud_propget(BKE_bytree **tree);
	void nud_propset(BKE_bytree **tree);
	void nud_class(BKE_bytree **tree);
	void nud_continue(BKE_bytree **tree);
	void nud_return(BKE_bytree **tree);
	void nud_try(BKE_bytree **tree);
	//void nud_new();
	//void nud_literal(BKE_bytree **tree);
	void nud_with(BKE_bytree **tree);
	void nud_stop(BKE_bytree **tree)
	{
		*tree = NULL;
		forcequit = true;
		return;
	}
	//void nud_other(BKE_bytree **tree);
	void nud_end(BKE_bytree **tree)
	{
		if (*tree == NULL)
		{
			forcequit = true;
			return;
		}
		else
			throw Var_Except(L"此处不应当遇到结尾", token.pos);
	}

	//void led_none(BKE_bytree **tree);
	void led_one(BKE_bytree **tree);
	void led_two(BKE_bytree **tree);
	void led_mul(BKE_bytree **tree);
	void led_pow(BKE_bytree **tree);
	void led_dot(BKE_bytree **tree);
	void led_ele(BKE_bytree **tree);
	void led_set(BKE_bytree **tree);
	//void led_comment();
	void led_param(BKE_bytree **tree);
	void led_choose(BKE_bytree **tree);		//for ?: expression

 	NUD_FUNCTION(preadd, "+");
	NUD_FUNCTION(presub, "-");
	NUD_FUNCTION(not, "!");
	//NUD_FUNCTION(bracket, "(");
	NUD_FUNCTION(array, "[");
	NUD_FUNCTION(dic, "%[");
	NUD_FUNCTION(block, "{");
	NUD_FUNCTION(specialblock, "");	//for reserve
	NUD_FUNCTION(inc, "++");
	NUD_FUNCTION(dec, "--");
//  actually do not need this function because it is handled by nud_label
//	NUD_FUNCTION(label, "*"){return BKE_Variable();};
	NUD_FUNCTION(if, "if");
	NUD_FUNCTION(for, "for");
	NUD_FUNCTION(forin, "forin");
	NUD_FUNCTION(foreach, "foreach");
	NUD_FUNCTION(do, "do");
	NUD_FUNCTION(while, "while");
	NUD_FUNCTION(propget, "propget");
	NUD_FUNCTION(propset, "propset");
	NUD_FUNCTION(function, "function");
	NUD_FUNCTION(class, "class");
	NUD_FUNCTION(continue, "continue");
	NUD_FUNCTION(break, "break");
	NUD_FUNCTION(return, "return");
	NUD_FUNCTION(try, "try");
	//NUD_FUNCTION(new, "new");
	NUD_FUNCTION(literal, "literal");		//for any const variables and names
	//NUD_FUNCTION(comment, "//");
	NUD_FUNCTION(var, "return");
	NUD_FUNCTION(delete, "return");
	NUD_FUNCTION(this, "this");
	NUD_FUNCTION(super, "super");
	//NUD_FUNCTION(global, "$");
	NUD_FUNCTION(global2, "global");
	NUD_FUNCTION(int, "int");
	NUD_FUNCTION(string, "string");
	NUD_FUNCTION(number, "number");
	NUD_FUNCTION(typeof, "typeof");
	NUD_FUNCTION(const, "const");
	NUD_FUNCTION(dot2, ".");
	NUD_FUNCTION(with, "with");

	NUD_FUNCTION(unknown, "未实现"){ runpos = tree->Node.pos; throw Var_Except(L"该功能尚未实现"); };

	LED_FUNCTION(add, "+");
	LED_FUNCTION(sub, "-");
	LED_FUNCTION(mul, "*");
	LED_FUNCTION(div, "/");
	LED_FUNCTION(mod, "%");
	LED_FUNCTION(pow, "^");
	LED_FUNCTION(equal, "==");
	LED_FUNCTION(eequal, "===");
	LED_FUNCTION(nequal, "!=");
	LED_FUNCTION(nnequal, "!==");
	LED_FUNCTION(larger, ">");
	LED_FUNCTION(smaller, "<");
	LED_FUNCTION(le, ">=");
	LED_FUNCTION(se, "<=");
	LED_FUNCTION(and, "&&");
	LED_FUNCTION(fastand, "&");
	LED_FUNCTION(or, "||");
	LED_FUNCTION(fastor, "|");
	LED_FUNCTION(set, "=");
	LED_FUNCTION(setadd, "+=");
	LED_FUNCTION(setsub, "-=");
	LED_FUNCTION(setmul, "*=");
	LED_FUNCTION(setdiv, "/=");
	LED_FUNCTION(setpow, "^=");
	LED_FUNCTION(setmod, "%=");
	LED_FUNCTION(setset, "|=");
	LED_FUNCTION(selfadd, "++");
	LED_FUNCTION(selfsub, "--");
	//LED_FUNCTION(comment, "//");
	LED_FUNCTION(param, "(");	//used in function call
	LED_FUNCTION(dot, ".");
	LED_FUNCTION(ele, "[");
	LED_FUNCTION(choose, "[");	//for ?: expression
	LED_FUNCTION(instanceof, "instanceof");	//xx instanceof str
	LED_FUNCTION(if, "if");	//后置if

	LED_FUNCTION(unknown, "未实现"){ runpos = tree->Node.pos; throw Var_Except(L"该功能尚未实现"); };

    inline void reportError(BKE_bytree** tree)
	{
		BKE_UNUSED_PARAM(tree);
		{
			throw Var_Except(wstring(L"语法错误，操作符") + OP_CODE[token.opcode % OP_COUNT] + L"不应出现在此处", curpos - exp);
		};
	};

	inline BKE_Variable& innerRun(BKE_bytree *tree, BKE_VarClosure *_this, BKE_array<BKE_Variable> &tmpvars)
	{
		if (!tree)
		{
			return none;
		}
		return (this->*runner[tree->Node.opcode])(tree, _this, tmpvars);
	}

	Parser();

	double costTime;

public:
	static Parser* getInstance(){static Parser p;return &p;}

	inline void addConst(const wstring &name, const BKE_Variable &var){ BKE_Variable &v = constmap[name]; v = var; v.makeConst(); };

	BKE_Variable eval(const wchar_t *exp, BKE_VarClosure *_this= BKE_VarClosure::global());
	inline BKE_Variable eval(const wstring &exp, BKE_VarClosure *_this= BKE_VarClosure::global()){return std::move(eval(exp.c_str(), _this));};
	BKE_Variable run(BKE_bytree *tree, BKE_VarClosure *_this= BKE_VarClosure::global());
	BKE_Variable run(BKE_VarFunction *func);
	BKE_Variable& getVar(const wchar_t *exp, BKE_VarClosure *_this= BKE_VarClosure::global());
	inline BKE_Variable& getVar(const wstring &exp, BKE_VarClosure *_this= BKE_VarClosure::global()){return getVar(exp.c_str(), _this);};

	BKE_Variable evalMultiLineStr(const wstring &exp, BKE_VarClosure *_this = BKE_VarClosure::global());

	BKE_bytree *parse(const wstring &exp, bkplong startpos = 0, bkplong startline = 0);
	void unParse(BKE_bytree *tree, wstring &res);

	inline void setErrorMsg(const wstring &msg){errorMSG=msg;}
	inline wstring& getErrorMsg(){return errorMSG;}

	inline void registerClass(const wstring &name, BKE_NativeClass *obj)
	{
		obj->nativeInit(name);
	}

	inline wstring getVersion()
	{
		return L"2014.11.29-0143";
	}

	wstring getTraceString();

#ifdef PARSER_DEBUG
	inline double getCostTime(){ auto re = costTime; costTime = 0; return re; }
#endif

	bool krmode;	//双引号转义
	bool rawstr;	//字符串可跨行
};
