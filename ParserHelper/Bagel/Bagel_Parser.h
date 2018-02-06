#pragma once

#include "Bagel_Config.h"
#include "Bagel_Var.h"
#include "Bagel_Vcode.h"

extern const BKE_Char* OP_CODE[];

class Bagel_Parser;

typedef void (Bagel_Parser::*OP_Function)(Bagel_AST**);
//typedef Bagel_Var& (Bagel_Parser::*OP_Runner)(Bagel_AST *node, Bagel_Closure *_this, BKE_array<Bagel_Var> &tmpvars);

//b is just a comment
#define NUD_FUNCTION(a,b) Bagel_Var& nudrunner_##a(Bagel_AST *tree, Bagel_Closure *_this, BKE_array<Bagel_Var> &tmpvars)
#define LED_FUNCTION(a,b) Bagel_Var& ledrunner_##a(Bagel_AST *tree, Bagel_Closure *_this, BKE_array<Bagel_Var> &tmpvars)
//#define NUD_FUNC(a) Bagel_Var& Bagel_Parser::nudrunner_##a(Bagel_AST *tree, Bagel_Closure *_this, BKE_array<Bagel_Var> &tmpvars)
//#define LED_FUNC(a) Bagel_Var& Bagel_Parser::ledrunner_##a(Bagel_AST *tree, Bagel_Closure *_this, BKE_array<Bagel_Var> &tmpvars)

/// <summary>
/// 解释器类，单例，使用 Bagel_Parser::getInstance() 来获取单一实例的指针。
/// </summary>
/// @sa getInstance
class Bagel_Parser
{
	friend class Bagel_FunctionCode;
	friend class Bagel_Function;

protected:
	BKE_hashmap<StringVal, Bagel_Var> constmap;
	BKE_hashmap<StringVal, BKE_opcode> opmap;
	//vector<Bagel_Var> tempvars;
	BKE_Node token, next;
	Bagel_Var none;

	const BKE_Char *exp;
	BKE_Char *curpos;
	int32_t expsize;
	int32_t runpos;
	bool forcequit;
	bool NextIsBareWord;

	StringVal errorMSG;
	
	OP_Function funclist[OP_COUNT * 2];
	//OP_Runner runner[OP_COUNT * 2];
	int32_t bp[OP_COUNT * 2];
	char head[OP_COUNT];

	OP_Function nudlist[OP_COUNT];
	OP_Function ledlist[OP_COUNT];
	//OP_Runner nud[OP_COUNT];
	//OP_Runner led[OP_COUNT];

	int32_t lbp[2 * OP_COUNT];
	int32_t *pre;

	int expprior;
	int commaprior;

	bool inFunctionCall;

	shared_ptr<Bagel_DebugInformation> info;

	void setupDebugInfo(int32_t startpos, int32_t startline)
	{
		info.reset(new Bagel_DebugInformation(exp, startpos, startline));
	}

	StringVal getPos(int32_t pos)
	{
		return info->getPos(pos);
	};

	StringVal getPosInfo(int32_t pos)
	{
		int32_t line, linepos;
		info->getInfo(pos, line, linepos);
		return bkpInt2Str(line) + W("行") + bkpInt2Str(linepos) + W("处");
	};

	inline bool MatchFunc(const BKE_Char *a, BKE_Char **c)
	{
		BKE_Char *b;
		b = *c;
		while ((*a))
		{
			if ((*a) != (*b))
				return false;
			b++;
			a++;
		}
		*c = b;
		return true;
	}

	void init();
	void init2(const BKE_Char *e, Bagel_Closure *_this, int32_t startpos = 0, int32_t startline = 0)
	{
		this->exp = e;
		curpos = const_cast<BKE_Char *>(e);
		expsize = static_cast<uint32_t>(u16len(e));
		runpos = 0;
		NextIsBareWord = false;
		setupDebugInfo(startpos, startline);
	}
	void init2(const StringVal &e, Bagel_Closure *_this, int32_t startpos = 0, int32_t startline = 0)
	{
		this->exp = e.c_str();
		curpos = const_cast<BKE_Char *>(exp);
		expsize = static_cast<uint32_t>(e.size());
		runpos = 0;
		NextIsBareWord = false;
		setupDebugInfo(startpos, startline);
	}

//#if BKE_CREATOR
//	virtual void expression(Bagel_AST** tree, int rbp = 0);
//	virtual void readToken();
//#else
	void expression(Bagel_AST** tree, int rbp=0);
	void readToken();
//#endif
	void skipToNextBlock();
	void throwParseError(const StringVal &v, int pos);
  
	void nud_string(Bagel_AST **tree);
	void nud_this(Bagel_AST **tree);
	void nud_var(Bagel_AST **tree);
	void nud_delete(Bagel_AST **tree);
	void nud_one(Bagel_AST **tree);
	void nud_bracket(Bagel_AST **tree);
	void nud_if(Bagel_AST **tree);
	void nud_array(Bagel_AST **tree);
	void nud_dic(Bagel_AST **tree);
	void nud_block(Bagel_AST **tree);
	void nud_label(Bagel_AST **tree);
	void nud_for(Bagel_AST **tree);
	void nud_foreach(Bagel_AST **tree);
	void nud_do(Bagel_AST **tree);
	void nud_while(Bagel_AST **tree);
	void nud_function(Bagel_AST **tree);
	void nud_propget(Bagel_AST **tree);
	void nud_propset(Bagel_AST **tree);
	void nud_class(Bagel_AST **tree);
	void nud_continue(Bagel_AST **tree);
	void nud_return(Bagel_AST **tree);
	void nud_try(Bagel_AST **tree);
	//void nud_new();
	//void nud_literal(Bagel_AST **tree);
	void nud_with(Bagel_AST **tree);
	void nud_stop(Bagel_AST **tree)
	{
		*tree = NULL;
		forcequit = true;
		return;
	}
	//void nud_other(Bagel_AST **tree);
	void nud_end(Bagel_AST **tree)
	{
		if (*tree != NULL)
		{
			throwParseError(W("此处不应当遇到结尾"), token.pos);
		}
		forcequit = true;
	}
	void nud_enum(Bagel_AST **tree);
	void nud_new(Bagel_AST **tree);
	void nud_switch(Bagel_AST **tree);
	void nud_case(Bagel_AST **tree);
	void nud_default(Bagel_AST **tree);
	void nud_property(Bagel_AST **tree);

	//void led_none(Bagel_AST **tree);
	void led_one(Bagel_AST **tree);
	void led_two(Bagel_AST **tree);
	void led_pow(Bagel_AST **tree);
	void led_dot(Bagel_AST **tree);
	void led_ele(Bagel_AST **tree);
	void led_set(Bagel_AST **tree);
	//void led_comment();
	void led_param(Bagel_AST **tree);
	void led_choose(Bagel_AST **tree);		//for ?: expression
	void led_comma(Bagel_AST **tree);
	void led_extract(Bagel_AST **tree);		//...

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
//	NUD_FUNCTION(label, "*"){return Bagel_Var();};
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
	NUD_FUNCTION(enum, "enum");
	NUD_FUNCTION(bitnot, "bitnot");

	NUD_FUNCTION(unknown, "未实现"){ runpos = tree->Node.pos; throw Bagel_Except(W("该功能尚未实现")); };

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
	LED_FUNCTION(choose, "?:");	//for ?: expression
	LED_FUNCTION(instanceof, "instanceof");	//xx instanceof str
	LED_FUNCTION(if, "if");	//后置if
	LED_FUNCTION(bitand, "bitand,");	//bitand
	LED_FUNCTION(bitor , "bitor");	//bitor

	LED_FUNCTION(unknown, "未实现"){ runpos = tree->Node.pos; throw Bagel_Except(L"该功能尚未实现"); };

	inline void reportError(Bagel_AST** tree)
	{
		BKE_UNUSED_PARAM(tree);
		int32_t pos = token.pos;
		if (info)
		{
			int32_t line, linepos;
			StringVal lineinfo;
			info->getInfo(pos, lineinfo, line, linepos);
			errorMSG += W("在") + bkpInt2Str(line) + W("行") + bkpInt2Str(linepos) + W("处：") + StringVal(W("语法错误，操作符")) + OP_CODE[token.opcode % OP_COUNT] + W("不应出现在此处");
		}
		else
		{
			errorMSG += W("在") + bkpInt2Str(pos) + W("处：") + StringVal(W("语法错误，操作符")) + OP_CODE[token.opcode % OP_COUNT] + W("不应出现在此处");
		}
		errorMSG += W("\n");
		skipToNextBlock();
	};

	//inline Bagel_Var& innerRun(Bagel_AST *tree, Bagel_Closure *_this, BKE_array<Bagel_Var> &tmpvars)
	//{
	//	if (!tree)
	//	{
	//		return none;
	//	}
	//	return (this->*runner[tree->Node.opcode])(tree, _this, tmpvars);
	//}



	double costTime;

	//__declspec(deprecated) Bagel_Var eval(const BKE_Char *exp, Bagel_Closure *_this= Bagel_Closure::global(), int line = 0);
	//__declspec(deprecated) inline Bagel_Var eval(const StringVal &exp, Bagel_Closure *_this = Bagel_Closure::global(), int line = 0){ return std::move(eval(exp.c_str(), _this, line)); };
	//__declspec(deprecated) Bagel_Var run(Bagel_AST *tree, Bagel_Closure *_this= Bagel_Closure::global());
	//__declspec(deprecated) Bagel_Var evalMultiLineStr(const StringVal &exp, Bagel_Closure *_this = Bagel_Closure::global(), int startline = 0);
	//__declspec(deprecated) Bagel_Var evalFile(const u16string &filename, Bagel_Closure *_this = Bagel_Closure::global());
	//Bagel_Var& getVar(const BKE_Char *exp, Bagel_Closure *_this= Bagel_Closure::global());
	//inline Bagel_Var& getVar(const u16string &exp, Bagel_Closure *_this= Bagel_Closure::global()){return getVar(exp.c_str(), _this);};

public:
	Bagel_Parser(bool init = true);

	~Bagel_Parser();

	/// <summary>
	/// 获取单例指针。
	/// </summary>
	/// <returns>单例的指针</returns>
	static Bagel_Parser* getInstance()
	{
		return _globalStructures.defaultParser;
	}

	inline void addConst(const StringVal &name, const Bagel_Var &var){ Bagel_Var &v = constmap[name]; v = var; /*v.makeConst();*/ };

	/// <summary>
	/// 解析指定的表达式
	/// </summary>
	/// <param name="exp">要解析的表达式。</param>
	/// <param name="startpos">开始解析的位置，默认为0。</param>
	/// <param name="startline">指定脚本一开始的行号，默认为0。这个参数在表达式字串不从文件的首行起，同时希望报错的行数和文件行数一致时使用。</param>
	/// <param name="parseAlways">是否在遇到错误时仍然返回包含尽量多正确部分的抽象语法树。如果设为false，则遇到错误时抛出异常。默认为false。</param>
	/// <returns>解析完成的语法树</returns>
	Bagel_AST *parse(const StringVal &exp, int32_t startpos = 0, int32_t startline = 0, bool parseAlways = false);
	/// <summary>
	/// 解析指定的表达式
	/// </summary>
	/// <param name="exp">要解析的表达式。</param>
	/// <param name="startpos">开始解析的位置，默认为0。</param>
	/// <param name="startline">指定脚本一开始的行号，默认为0。这个参数在表达式字串不从文件的首行起，同时希望报错的行数和文件行数一致时使用。</param>
	/// <param name="parseAlways">是否在遇到错误时仍然返回包含尽量多正确部分的抽象语法树。如果设为false，则遇到错误时抛出异常。默认为false。</param>
	/// <returns>解析完成的语法树</returns>
	Bagel_AST *parse(const BKE_Char* exp, int32_t startpos = 0, int32_t startline = 0, bool parseAlways = false);
	/// <summary>
	/// 将解析好的语法树回退为表达式。
	/// </summary>
	/// <param name="tree">抽象语法树。</param>
	/// <param name="res">结果字符串。</param>
	void unParse(Bagel_AST *tree, StringVal &res);

	/// <summary>
	/// 获取上一次解析的DebugInformation。（即行号和命令位置的对应关系）
	/// </summary>
	/// <returns>DebugInformation</returns>
	const shared_ptr<Bagel_DebugInformation> getDebugInfo() const
	{
		return info;
	}

	//inline void setErrorMsg(const StringVal &msg){errorMSG=msg;}

/// <summary>
/// 获取上一次解析的错误信息。没有则返回空字符串。
/// </summary>
/// <returns>错误信息。</returns>
	inline StringVal& getErrorMsg()
	{
		return errorMSG;
	}

#ifdef PARSER_DEBUG
	inline double getCostTime(){ auto re = costTime; costTime = 0; return re; }
#endif

	//for analysis
	void getKeywordsWithPrefix(std::map<StringVal, PromptType>& result, const StringVal &prefix);

	/// <summary>
	/// 双引号和单引号字符串解析使用krkr的规则。
	/// </summary>
	bool krmode;	//双引号转义
	/// <summary>
	/// 双引号和单引号字符串可以跨行。
	/// </summary>
	bool rawstr;	//字符串可跨行

	//将带string的array和dic不const化
	bool seeStringNotConst;
};

class Bagel_AST_Analysis
{
protected:
	//返回该表达式的值，对于常量返回常量，否则应为void
	Bagel_Var _analysis(Bagel_AST *tree, Bagel_Closure* glo, Bagel_Closure *thiz, bool getaddr);

	bool _analysisVar(Bagel_AST *tree, Bagel_Closure* glo, Bagel_Closure *thiz, int pos, std::map<StringVal, PromptType> & out, Bagel_Var &outvar, bool getaddr);

	Bagel_Var withvar;

public:
	vector<Bagel_Var> ret;

	//分析一段代码，记录下全局变量
	void analysis(Bagel_AST *tree, Bagel_Closure* glo, Bagel_Closure *thiz);

	//分析一段代码，记录下全局变量
	void analysis(const StringVal &exp, Bagel_Closure* glo, Bagel_Closure *thiz, Bagel_Parser *parser = NULL)
	{
		if (!parser)
			parser = Bagel_Parser::getInstance();
		Bagel_AST *tree = parser->parse(exp, 0, 0, true);
		if (tree)
		{
			analysis(tree, glo, thiz);
		}
	}

	Bagel_Var analysisVar(Bagel_AST *tree, Bagel_Closure* glo, Bagel_Closure *thiz, int pos, std::map<StringVal, PromptType> & out);

	Bagel_Var analysisVar(const StringVal &exp, Bagel_Closure* glo, Bagel_Closure *thiz, int pos, std::map<StringVal, PromptType> & out, Bagel_Parser *parser = NULL)
	{
		if (!parser)
			parser = Bagel_Parser::getInstance();
		Bagel_AST *tree = parser->parse(exp, 0, 0, true);
		if (tree)
		{
			return analysisVar(tree, glo, thiz, pos, out);
		}
		return Bagel_Var();
	}

	//分析一段字符串，判断其值是否为一个常数
	Bagel_Var tryGetConstVar(const StringVal &exp)
	{
		Bagel_AST *tree = Bagel_Parser::getInstance()->parse(exp, 0, 0, true);
		if (tree && tree->Node.opcode == OP_CONSTVAR + OP_COUNT)
		{
			return tree->Node.var;
		}
		return Bagel_Var();
	}
};
