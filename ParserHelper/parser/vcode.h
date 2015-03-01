#ifndef BKE_VCODE
#define BKE_VCODE

#ifdef _DEBUG_MEM
#include <debug_new.h>
#endif

#include <string>
//#include <vector>
#include "BKE_array.h"
#include "BKE_string.h"
#include "BKE_variable.h"

//define opcode
enum BKE_opcode : bkplong
{
	OP_END,
	//OP_COMMENT,
	OP_IF,
	OP_QUICKFOR,	//for xx in [start, stop, step]
	OP_FOR,
	OP_WHILE,
	OP_DO,
	OP_FOREACH,
	OP_CONSTVAR,	//常量
	OP_LITERAL,		//变量
	OP_FUNCTION,
	OP_CLASS,
	OP_PROPGET,
	OP_PROPSET,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_POW,
	OP_EQUAL,
	OP_NEQUAL,
	OP_EEQUAL,
	OP_NNEQUAL,
	OP_LARGER,
	OP_SMALLER,
	OP_LE,
	OP_SE,
	OP_FASTAND,
	OP_AND,
	OP_FASTOR,
	OP_OR,
	OP_NOT,
	OP_SET,
	OP_SETADD,
	OP_SETSUB,
	OP_SETMUL,
	OP_SETDIV,
	OP_SETMOD,
	OP_SETPOW,
	OP_SETSET,		//|=
	OP_SELFADD,
	OP_SELFDEC,
	OP_BRACKET,
	OP_ARRAY,
	OP_DIC,
	OP_BLOCK,		// {
	OP_DOT,
	OP_CONTINUE,
	OP_BREAK,
	OP_RETURN,
	//OP_NEW,
	OP_VAR,
	OP_DELETE,
	//OP_OTHER,			//such as ],),},:,in,
	OP_TRY,
	OP_THROW,
	OP_CHOOSE,		//for ?: expression
	OP_THIS,
	OP_SUPER,
	OP_GLOBAL,		//global
	OP_TOINT,		//int
	OP_TOSTRING,	//string
	OP_TONUMBER,	//number
	OP_CONST,		//const	//compatible with krkr
	OP_TYPE,		//typeof
	OP_INSTANCE,	//instanceof

	OP_RESERVE,		//uses when global expression is splitted by L";"
	//also used as a block with no own closure
	OP_EXTENDS,
	OP_IN,
	OP_ELSE,
	OP_THEN,
	OP_CATCH,
	OP_MAOHAO,		//:
	OP_STOP,		//;
	OP_BRACKET2,	//)
	OP_ARR2,		//]
	OP_BLOCK2,		//}
	OP_COMMA,
	OP_VALUE,		//=>

	OP_WITH,
	OP_INCONTEXTOF,
	OP_STATIC,

	OP_INVALID,

	OP_COUNT,
};

class BKE_Node
{
public:
	/*BKE_opcode*/bkplong opcode;	//we need += operation on it
	BKE_Variable var;
	bkplong pos;

	BKE_Node(BKE_opcode op = OP_END, int var = 0) :opcode(op), pos(0){}
	BKE_Node(const BKE_Node &r) :opcode(r.opcode), var(r.var), pos(r.pos){}
	BKE_Node& operator = (const BKE_Node &node)
	{
		pos = node.pos;
		opcode = node.opcode;
		var = node.var;
		return *this;
	}
};

class BKE_bytree : public BKE_VarObject
{
private:
	~BKE_bytree();

public:
	BKE_bytree *parent;
	//	vector<BKE_bytree*> childs;
	BKE_array<BKE_bytree*> childs;

	BKE_Node Node;

	BKE_bytree():BKE_VarObject(VAR_BYTREE_P), parent(NULL){}
	BKE_bytree(const BKE_Node &node) :BKE_VarObject(VAR_BYTREE_P), parent(NULL), Node(node){};
	inline void init(const BKE_Node &node){ Node = node; };
	void clearChilds()
	{
		for (int i = 0; i<(int)childs.size(); i++)
		if (childs[i])
			childs[i]->release();
		childs.clear();
	}
	inline void addChild(){ childs.push_back(NULL); };
	//warning! this function don't check if parent==NULL
	inline BKE_bytree *addParent(const BKE_Node &node){ BKE_bytree *p = new BKE_bytree(node); p->childs.push_back(this); return p; };
	BKE_bytree *clone()
	{
		BKE_bytree *clone = new BKE_bytree(this->Node);
		for (int i = 0; i<(int)childs.size(); i++)
			clone->childs.push_back(this->childs[i]->clone());
		return clone;
	}
};

#endif