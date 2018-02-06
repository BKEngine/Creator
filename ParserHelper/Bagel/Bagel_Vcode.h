#pragma once

#include "Bagel_Config.h"
#include "BKE_array.h"
#include "Bagel_String.h"
#include "Bagel_Var.h"

//define opcode
enum BKE_opcode : int32_t
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
	OP_PROPERTY,
	OP_SETTER,
	OP_GETTER,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_INTDIV,
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
	OP_NOT,
	OP_FASTAND,
	OP_AND,
	OP_FASTOR,
	OP_OR,
	OP_SET,
	OP_SETADD,
	OP_SETSUB,
	OP_SETMUL,
	OP_SETDIV,
	OP_SETINTDIV,
	OP_SETMOD,
	OP_SETPOW,
	OP_SETSET,		//|=
	OP_SELFINC,
	OP_SELFDEC,
	OP_BRACKET,
	OP_ARRAY,
	OP_DIC,
	OP_NULLARRAY,
	OP_NULLDIC,
	OP_BLOCK,		// {
	OP_DOT,
	OP_CONTINUE,
	OP_BREAK,
	OP_RETURN,
	//OP_NEW,
	OP_VAR,
	OP_NEW,
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
	OP_SWITCH,
	OP_CASE,
	OP_DEFAULT,

	//uses when global expression is splitted by L";"
	//also used as a block with no own closure，最后一个表达式的返回值作为整个的返回值
	OP_RESERVE,	
	//标记逗号表达式，最后一个表达式的返回值作为整个的返回值
	OP_RESERVE2,
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
	OP_ENUM,

	OP_BITAND,
	OP_BITOR,
	OP_BITXOR,
	OP_BITNOT,

	OP_YIELD,

	OP_EXTRACTARGS,	//...

	OP_INVALID,

	OP_COUNT,
};

class BKE_Node
{
public:
	/*BKE_opcode*/int32_t opcode;	//we need += operation on it
	Bagel_Var var;
	int32_t pos;
	int32_t pos2;	//记录该语法树结束时的位置

	BKE_Node(BKE_opcode op = OP_END, int var = 0) :opcode(op), pos(0), pos2(0)
	{
	}
	BKE_Node(const BKE_Node &r) :opcode(r.opcode), var(r.var), pos(r.pos), pos2(r.pos2)
	{
	}
	BKE_Node& operator = (const BKE_Node &node)
	{
		pos = node.pos;
		pos2 = node.pos2;
		opcode = node.opcode;
		var = node.var;
		return *this;
	}
};

class Bagel_AST : public Bagel_Object
{
private:
	~Bagel_AST();

public:
	Bagel_AST *parent;
	vector<Bagel_AST*, BKE_Allocator<Bagel_AST*>> childs;

	BKE_Node Node;

	Bagel_AST() :Bagel_Object(VAR_BYTREE_P), parent(NULL)
	{
	}
	Bagel_AST(const BKE_Node &node) :Bagel_Object(VAR_BYTREE_P), parent(NULL), Node(node)
	{
	};
	inline void init(const BKE_Node &node)
	{
		Node = node;
	};
	void clearChilds()
	{
		childs.clear();
	}
	inline void addChild()
	{
		childs.push_back(NULL);
	};
	virtual void markChildren() override
	{
		for (auto &it : childs)
		{
			_GC.GC_Markself(it);
		}
		_GC.GC_Markself(Node.var);
	};
	//warning! this function don't check if parent==NULL
	inline Bagel_AST *addParent(const BKE_Node &node)
	{
		Bagel_AST *p = new Bagel_AST(node);
		p->childs.push_back(this); 
		return p;
	};
	Bagel_AST *clone()
	{
		Bagel_AST *clone = new Bagel_AST(this->Node);
		for (int i = 0; i < (int)childs.size(); i++)
		{
			if (this->childs[i])
				clone->childs.push_back(this->childs[i]->clone());
			else
				clone->childs.push_back(NULL);
		}
		return clone;
	}
	bool isVar() const
	{
		return (Node.opcode == OP_LITERAL + OP_COUNT) || (Node.opcode == OP_DOT) || (Node.opcode == OP_DOT + OP_COUNT) || (Node.opcode == OP_ARRAY && childs.size() == 2);
	}
	int getFirstPos()
	{
		int childpos = Node.pos;
		for (auto &it : childs)
		{
			if (it)
			{
				int pos2 = it->getFirstPos();
				if (childpos > pos2)
					childpos = pos2;
				break;
			}
		}
		return childpos;
	}
	//call this function immediately after the AST has finished
	void calcLastPos()
	{
		for (auto &it = childs.rbegin(); it != childs.rend(); ++it)
		{
			if (*it)
			{
				Node.pos2 = (*it)->Node.pos2;
				return;
			}
		}
	}
};

#define CLOPOS 0
#define THISPOS 1
#define PARAMSTARTPOS 2

//所有jump类指令跳转为绝对位置，跳转之后跳过正常执行执行后的code++，相对算起来真麻烦
enum Bagel_BC : unsigned char
{
	BC_INVALID,			//release编译时用来标记将要被移除的字节码
	//arithmetic
	BC_ADD,				//[A]=[B]+[C]
	BC_SUB,				//[A]=[B]-[C]
	BC_MUL,				//[A]=[B]*[C]
	BC_DIV,				//[A]=[B]/[C]
	BC_INTDIV,			//[A]=[B]\[C]
	BC_MOD,				//[A]=[B]%[C]
	BC_POW,				//[A]=[B]^[C]
	BC_ADDCS,				//[A]=const[B]+[C]
	BC_SUBCS,				//[A]=const[B]-[C]
	BC_MULCS,				//[A]=const[B]*[C]
	BC_DIVCS,				//[A]=const[B]/[C]
	BC_INTDIVCS,			//[A]=const[B]\[C]
	BC_MODCS,				//[A]=const[B]%[C]
	BC_POWCS,				//[A]=const[B]^[C]
	BC_ADDSC,				//[A]=[B]+const[C]
	BC_SUBSC,				//[A]=[B]-const[C]
	BC_MULSC,				//[A]=[B]*const[C]
	BC_DIVSC,				//[A]=[B]/const[C]
	BC_INTDIVSC,			//[A]=[B]\const[C]
	BC_MODSC,				//[A]=[B]%const[C]
	BC_POWSC,				//[A]=[B]^const[C]

	//boolean
	BC_EQUAL,			//[A]=[B]==[C]
	BC_NEQUAL,			//[A]=[B]!=[C]
	BC_EEQUAL,			//[A]=[B]===[C]
	BC_NNEQUAL,			//[A]=[B]!==[C]
	BC_LARGER,			//[A]=[B]>[C]
	BC_SMALLER,			//[A]=[B]<[C]
	BC_LE,				//[A]=[B]>=[C]
	BC_SE,				//[A]=[B]<=[C]

	//logic
	BC_NOT,				//[A]=![B]
	BC_BITAND,			//[A]=[B] bitand [C]
	BC_BITOR,			//[A]=[B] bitor [C]
	BC_BITXOR,			//[A]=[B] bitxor [C]
	BC_BITNOT,			//[A]=bitnot [B]

	//convert
	BC_CLONE,			//[A]=[B].clone()
	BC_TOBOOL,			//[A]=(bool)[B]
	BC_TOINT,			//[A]=(int)[B]
	BC_TOSTRING,		//[A]=(string)[B]
	BC_TONUMBER,		//[A]=(number)[B]
	BC_TOMINUSNUMBER,	//[A]=-(number)[B]

						//stack set
	BC_MOV,				//[A].forceSet([B])
	BC_SETADD,			//[A]+=[B]
	BC_SETSUB,			//[A]-=[B]
	BC_SETMUL,			//[A]*=[B]
	BC_SETDIV,			//[A]/=[B]
	BC_SETINTDIV,		//[A]\=[B]
	BC_SETMOD,			//[A]%=[B]
	BC_SETPOW,			//[A]^=[B]
	BC_SETSET,			//[A]|=[B]
	BC_SELFINC,			//[C]=([A]++)
	BC_SELFDEC,			//[C]=([A]--)
	BC_PREINC,			//++[A]
	BC_PREDEC,			//--[A]
	BC_MOVC,			//[A].forceSet(const[B]), 和BC_LOADCONST一样
	BC_SETADDC,			//[A]+=const[B]
	BC_SETSUBC,			//[A]-=const[B]
	BC_SETMULC,			//[A]*=const[B]
	BC_SETDIVC,			//[A]/=const[B]
	BC_SETINTDIVC,		//[A]\=const[B]
	BC_SETMODC,			//[A]%=const[B]
	BC_SETPOWC,			//[A]^=const[B]
	BC_SETSETC,			//[A]|=const[B]

	//load
	BC_COPYCONST,		//[A].copyFrom(const[B])
	BC_LOADCONST,		//A=const[B]
	BC_LOADGLOBAL,		//A=new global
	BC_LOADVOID,		//A=new void
	BC_LOADNUM,			//A=new num(*(double*)B)
	BC_INITSTACKMEMBER,	//A=pointer(clostack[B]);
	BC_INITSTACKMEMBER2,//A=pointer(clostack[B].tag);
	BC_LOADPOINTER,		//A=*B;B is pointer
	BC_LOADPOINTER2,	//A=*B;B is pointer	//PTR_SETxxx系专用，使用且不清空self信息
	BC_SETPOINTER,		//*A=B;A is pointer
	BC_SETPOINTER2,		//*A=B;A is pointer	//PTR_SETxxx系专用，使用self信息

	//member
	BC_SETMEMBER2,				//[C]=([A].[B]=[C])不往上级闭包寻找，强制创建当前闭包局部变量。
	BC_LOADCLOMEMBER_VALUE,		//[C]=[A].const[B],不需要dotFunc
	BC_LOADMEMBER_VALUE,		//[C]=[A].[B]
	BC_LOADGLOBALMEMBER_VALUE,	//[C]=global.const[A]
	BC_LOADSUPERMEMBER_VALUE,	//[C]=super([THISPOS]).[B]
	BC_LOADSUPERMEMBER_PTR,			//[C]=pointer(super([THISPOS]).[B] incontextof [THISPOS]),并且保留property，对于其他类型则报错，用于赋值
	BC_LOADMEMBERIDX_VALUE,		//[C]=[A][(B)]
	BC_LOADMEMBERSTRIDX_VALUE,//[C]=[A].const[B]
	BC_LOADARRAYIDX_VALUE,		//[C]=pointer([A],[B]), [A] is array or str, [B] is num
	BC_LOADARRAYIDX2_VALUE,		//[C]=pointer([A],B), [A] is array or str

	//memberptr
	BC_GETMEMBER_PTR,			//[C]=pointer([A],[B])
	BC_GETCLOMEMBER_PTR,		//[C]=pointer(Closure([A]),const[B])
	BC_GETGLOBALMEMBER_PTR,		//[C]=pointer(global,const[A])
	BC_GETMEMBERIDX_PTR,		//[C]=pointer([A],B)
	BC_GETMEMBERSTRIDX_PTR,	//[C]=pointer([A],const[B])
	BC_GETARRAYIDX_PTR,		//[C]=pointer([A],[B]), [A] is array or str
	BC_GETARRAYIDX2_PTR,		//[C]=pointer([A],B), [A] is array

	//ptrset
	BC_PTR_SET,					//*[A]=[B]
	BC_PTR_SETADD,				//*[A]+=[B]
	BC_PTR_SETSUB,				//*[A]-=[B]
	BC_PTR_SETMUL,				//*[A]*=[B]
	BC_PTR_SETDIV,				//*[A]/=[B]
	BC_PTR_SETINTDIV,			//*[A]\=[B]
	BC_PTR_SETMOD,				//*[A]%=[B]
	BC_PTR_SETPOW,				//*[A]^=[B]
	BC_PTR_SETSET,				//*[A]|=[B]
	BC_PTR_SELFINC,				//[C]=(*[A]++)
	BC_PTR_SELFDEC,				//[C]=(*[A]--)
	BC_PTR_PREINC,				//*++[A]
	BC_PTR_PREDEC,				//*--[A]
	BC_PTR_SETC,				//*[A]=const[B]
	BC_PTR_SETADDC,				//*[A]+=const[B]
	BC_PTR_SETSUBC,				//*[A]-=const[B]
	BC_PTR_SETMULC,				//*[A]*=const[B]
	BC_PTR_SETDIVC,				//*[A]/=const[B]
	BC_PTR_SETINTDIVC,			//*[A]\=const[B]
	BC_PTR_SETMODC,				//*[A]%=const[B]
	BC_PTR_SETPOWC,				//*[A]^=const[B]
	BC_PTR_SETSETC,				//*[A]|=const[B]

	//construct
	BC_MAKEARRAY,		//A=new array([B], ..., [B+C-1])
	BC_MAKEDIC,			//A=new dic([B], ..., [B+C-1])
	BC_MAKECLASS,		//A=new class([B], ..., [B+C-1])
	BC_SETCLOSURE,		//[A].setClosure([B])
	BC_SETCLOSURESTACK,	//[A].setStack()
	BC_ADDPROPGET,		//[A].addPropGet([B] as name, [C] as funccode)
	BC_ADDPROPSET,		//[A].addPropSet([B] as name, [C] as funccode)
	BC_ADDPROPERTY,		//[A].addProperty([B] as name, [B+1] as get_funccode, [B+2] as set_funccode)
	BC_ADDSTATIC,		//[A].addStaticVar([B] as name, [C] as value)
	BC_ADDVAR,			//[A].addClassVar([B] as name, [C] as value)
	BC_ADDFUNCTION,		//[A].addFunction([B] as name, [C] as value)
	BC_FINISHCLASS,		//[A]的所有children refreshAll()
	BC_MAKELOCALPROPGET,//[A]=prop, [A].tag=&[A], [B] as name, [C] as funccode
	BC_MAKELOCALPROPSET,//[A]=prop, [A].tag=&[A], [B] as name, [C] as funccode
	BC_MAKELOCALPROPERTY,	//[A]=prop, [A].tag=&[A], [B] as name, [B+1] as get_funccode, [B+2] as set_funccode

	//control
	BC_PUSHCLO,			//A=new clo([B] as parent)
	BC_JUMPFALSE,		//if(![B])code=A
	BC_JUMPTRUE,		//if([B])code=A
	BC_JUMPVOID,		//if([B]===void)code=A
	BC_JUMPNOTVOID,		//if([B]!==void)code=A
	BC_JUMP,			//code=A
	BC_JUMPEQUAL,		//if([B]==[C])code=A;
	BC_JUMPNEQUAL,		//if([B]!=[C])code=A;
	BC_JUMPEEQUAL,		//if([B]===[C])code=A;
	BC_JUMPNNEQUAL,		//if([B]!==[C])code=A;
	BC_JUMPLARGER,		//if([B]>[C])code=A;
	BC_JUMPSMALLER,		//if([B]<[C])code=A;
	BC_JUMPLE,			//if([B]>=[C])code=A;
	BC_JUMPSE,			//if([B]<=[C])code=A;
//	BC_PRECALL,			//check whether [A] is function or class
	BC_CALL,			//[A] = call [A]([B],...,[B+C-1]),C=0表示无参数。
	BC_RETURN,			//return [A];
	BC_YIELD,			//[B] = yield return [A]
	BC_NULLVECTOR,		//[A] = new Bagel_Vector()
	BC_VECTORPUSHBACK,	//[A].pushback([B],...,[B+C-1])
	BC_VECTORCONCAT,	//check [B] is array, then [A].concat([B])
	BC_CALLVECTOR,		//[A] = call [A](extract array [B])
	BC_LOOPSTART,		//约定[A]=start,[A+1]=stop,[A+2]=step,[C]=loopvar=start,若[A+2]为void，则[A+2]=start>stop?-1:1,若[A=2]==0抛出异常.B为对应LOOP_JUDGE后面的位置
	BC_LOOPSTEP,		//[A]+=[A+2],判断是否满足([A]<=[A+1] && [A+2]>0) || ([A]>=[A+1] && [A+2]<0),满足则[C]=[A],code=B(继续循环)
	BC_LOOPSTEP2,		//[A]+=[A+2],判断是否满足([A]<=[A+1] && [A+2]>0) || ([A]>=[A+1] && [A+2]<0),满足则[C]=[A],不满足则code=B(跳出)
							//运行时改变字节码的行为是危险的，xxx(){for i=a,b,c{xxx()}}这样的就会发生错误
		//BC_LOOPSTEP1,		//判断是否满足[A]<[A+1],满足则[A]+=[A+2],[A+3]=[A],code=B
		//BC_LOOPSTEP2,		//判断是否满足[A]<[A+1],满足则[A]+=[A+2],[A+3]=[A],code=B
	BC_FOREACHSTART,	//if[A]为空, code=C, 如果[A]是[A]为dic时[A+1]=iterator,[A+1].tick=deletetick,否则[A+1]=void, [A+2].tick=inner key, [B]=key, [B-1]=value
	BC_NEXT,			//按照[A]的类型执行下面三种
		//BC_NEXTSTRING,		//[A+2]++, [C]=[A+2], [C-1]=value, 若没结束([A+1]<[A].length)则code=B
		//BC_NEXTARRAY,		//[A+2]++, [C]=[A+2], [C-1]=value, 若没结束([A+1]<[A].length)则code=B
		//BC_NEXTDIC,			//如果[A].deletetick!=[A+1].tick,[A+1].it=[A].find([A+2]),[A+1].it++,[A+2]=*[A+1].it, [C]=[A+2], [C-1]=value, 若没结束([A+1]<[A].length)则code=B
	BC_ADDTRYHOOK,		//trystack.emplace_back(A,B)
	BC_REMOVETRYHOOK,	//trystack.pop_back()
	BC_REMOVETRYHOOKN,	//trystack.pop_back() A times

	//functional
	BC_DELETE,			//delete [A].[B]
	BC_THROW,			//throw [A]
	BC_THROWCONST,		//throw const[[A]]
	BC_TYPE,			//[A]=typeof [B]
	BC_INSTANCE,		//[A]=[B] instanceof [C]
	BC_INCONTEXTOF,		//[A]=[B] incontextof [C]
	BC_ELEMENT,			//[A]=[B][[C]:[C+1]:[C+2]]
	BC_MERGESELF,		//if(clo->hasSelf), merge self to [A].tag

	//typecheck
	BC_CHECKTYPE,		//[A].type != B时报异常
	BC_CHECKTHIS,		//[1] === void时报异常

	//pointer
	BC_CREATEPOINTER,	//[A]=Bagel_Pointer([B], [C])

	//compile tag
	BC_RESERVE_STACK,	//栈上留下连续的[A]-[A]+[B]的空间，放函数的参数

	//force GC
	BC_GC,

	//debugpoint flag
	BC_DEBUG,			//A:对应的原语句的索引位置，realcode:对应语句的地址

	//cache instruction (long)，cache都是横跨三条指令
	BC_CACHE_NONE,				//code+=3,skip cache instruction,cache占位符
	//cache成功会再跳过两个指令
	BC_CACHE_GLOBAL_MEMBER,		//code+=3,if(global.varmap.delete==A && global == ptr2)[C3]=dst;
	BC_CACHE_DIC_MEMBER,		//if([A3].obj==ptr && global_class([A3]).varmap.insert==A && [A3].varmap.delete==A2)[C3]=dst2
	BC_CACHE_CLASS_VAR,			//if([A3].obj==ptr && [A3].varmap.delete==A)[C3]=dst2,用在自身varmap已经有值的情况。（比如被赋值后）
	BC_CACHE_CLASS_FUNC,		//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].def.varmap.delete==A2)[C3]=dst2
	BC_CACHE_TYPE_MEMBER,		//if([A3].type==A && global_class([A3]).varmap.delete==C)[C3]=dst2(property and function需要复制一份新self的)
	BC_CACHE_CLO_MEMBER1,		//if([A3].obj==ptr && [A3].varmap.delete==A)[C3]=dst2
	BC_CACHE_CLO_MEMBER2,		//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].parent.varmap.delete==A2)[C3]=dst2,一个闭包的parent是不能被更改的
	BC_CACHE_CLO_MEMBER_EXT,	//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].parent.varmap.insert==A2)[C3]=[A3].parent.parent[B3]
	BC_CACHE_GLOBAL_INCLASS,	//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].defclass.varmap.insert==A2)[C3]=dst2,dst2 is in global
	BC_CACHE_GLOBAL_INCLASSDEF,	//if([A3].obj==ptr && [A3].varmap.insert==A)[C3]=dst2,dst2 is in global
	BC_CACHE_CLASS_VARINDEX,	//if([A3].type == VAR_CLASS && [A3].defclass.classid==A && [A3].varmap.delete==1)[C3]=[A3].cache[B]
	BC_CACHE_NEWCLASS_FUNC,		//if([A3].type == VAR_CLASS && [A3].defclass.classid==A && [A3].varmap.insert==1)[C3]=dst

	BC_CACHE_GLOBAL_MEMBERV,	//code+=3,if(global.varmap.delete==A && global == ptr2)[C3]=*dst;
	BC_CACHE_DIC_MEMBERV,		//if([A3].obj==ptr && global_class([A3]).varmap.insert==A && [A3].varmap.delete==A2)[C3]=*dst2
	BC_CACHE_CLASS_VARV,		//if([A3].obj==ptr && [A3].varmap.delete==A)[C3]=*dst2,用在自身varmap已经有值的情况。（比如被赋值后）
	BC_CACHE_CLASS_FUNCV,		//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].def.varmap.delete==A2)[C3]=*dst2
	BC_CACHE_TYPE_MEMBERV,		//if([A3].type==A && global_class([A3]).varmap.delete==C)[C3]=*dst2(property and function需要复制一份新self的)
	BC_CACHE_CLO_MEMBER1V,		//if([A3].obj==ptr && [A3].varmap.delete==A)[C3]=*dst2
	BC_CACHE_CLO_MEMBER2V,		//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].parent.varmap.delete==A2)[C3]=*dst2,一个闭包的parent是不能被更改的
	BC_CACHE_CLO_MEMBER_EXTV,	//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].parent.varmap.insert==A2)[C3]=[A3].parent.parent[B3]
	BC_CACHE_GLOBAL_INCLASSV,	//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].defclass.varmap.insert==A2)[C3]=*dst2,*dst2 is in global
	BC_CACHE_GLOBAL_INCLASSDEFV,//if([A3].obj==ptr && [A3].varmap.insert==A)[C3]=*dst2,*dst2 is in global
	BC_CACHE_CLASS_VARINDEXV,	//if([A3].type == VAR_CLASS && [A3].defclass.classid==A && [A3].varmap.delete==1)[C3]=*[A3].cache[B]
	BC_CACHE_NEWCLASS_FUNCV,	//if([A3].type == VAR_CLASS && [A3].defclass.classid==A && [A3].varmap.insert==1)[C3]=*dst
};

//bytecode
//8|24			32	32	32
//opcode|pos	A	B	C
#define MINUS_POS ((1<<24)-1)

#pragma pack(push)
#pragma pack(1)
struct Bagel_ByteCodeStruct
{
	unsigned int opcode : 8;
	unsigned int pos : 24;
	struct
	{
		int32_t A;
		union
		{
			struct
			{
				int32_t B;
				int32_t C;
			};
			double D;
			Bagel_ObjectID ptr;
			Bagel_Var *dst;
			Bagel_ByteCodeStruct *realcode;
		};
	};

	Bagel_ByteCodeStruct(int op = 0, int p = 0, int a = 0, int b = 0, int c = 0) :opcode(op), pos(p), A(a), B(b), C(c)
	{
	}

	Bagel_ByteCodeStruct(int op, int p, int a, double d) :opcode(op), pos(p), A(a), D(d)
	{
	}

	bool operator == (const Bagel_ByteCodeStruct & c)
	{
		return opcode == c.opcode && A == c.A && B == c.B && C == c.C;
	}
};
#pragma pack(pop)

struct Bagel_DebugInformation
{
	//debugInfo
	vector<int32_t> linestartpos;
	StringVal rawexp;
	int startline;
	int beginpos;

	vector<Bagel_ByteCode *> innerFunc;

	void setupInfo(int begin = 0)
	{
		beginpos = begin;
		linestartpos.clear();
		const BKE_Char *start = rawexp.c_str();
		const BKE_Char *startpos = start;
		BKE_Char ch;
		linestartpos.push_back(begin);
		while ((ch = *(start++)) != 0)
		{
			if (ch == 0x0A)
			{
				linestartpos.push_back(start - startpos + begin);
			}
			else if (ch == 0x0D)
			{
				if (*start == 0x0A)
				{
					start++;
					linestartpos.push_back(start - startpos + begin);
				}
				else
					linestartpos.push_back(start - startpos + begin);
			}
		}
	}

	Bagel_DebugInformation(const StringVal &exp, int begin = 0, int start = 0) : rawexp(exp), startline(start)
	{
		setupInfo(begin);
	}

	Bagel_DebugInformation(StringVal &&exp, int begin = 0, int start = 0) : rawexp(std::move(exp)), startline(start)
	{
		setupInfo(begin);
	}

	Bagel_DebugInformation()
	{
	};

	Bagel_DebugInformation(Bagel_DebugInformation &&info) :linestartpos(std::move(info.linestartpos)), rawexp(std::move(info.rawexp))
	{
	};

	u16string getPos(int32_t pos)
	{
		u16string s;
		if (pos < linestartpos.front())
			pos += linestartpos.front();
	#if PARSER_DEBUG
		auto it = upper_bound(linestartpos.begin(), linestartpos.end(), pos);
		s += W("第") + bkpInt2Str(startline + (int32_t)(it - linestartpos.begin())) + W("行，第") + bkpInt2Str((int32_t)((it == linestartpos.end() ? pos : *it) - (*(it - 1)) + 1)) + W("处的");
	#endif
		return s;
	};

	void getInfo(int32_t pos, StringVal &lineinfo, int32_t &line, int32_t &linepos)
	{
		if (pos < 0)
		{
			line = pos = 0;
			return;
		}
		if (pos < linestartpos.front())
			pos += linestartpos.front();
		auto it = upper_bound(linestartpos.begin(), linestartpos.end(), pos);
		if (it == linestartpos.end())
			lineinfo = rawexp.substr(*(--it) - linestartpos[0]);
		else
		{
			lineinfo = rawexp.substr(*(it - 1) - linestartpos[0], *it - *(it - 1));
			it--;
		}
		line = it - linestartpos.begin() + 1 + startline;
		linepos = pos - *it + 1;
	}

	//这里出来的line是从1开始数的行数，linepos是从1数的列数
	void getInfo(int32_t pos, int32_t &line, int32_t &linepos)
	{
		if (pos < 0)
		{
			line = pos = 0;
			return;
		}
		if (pos < linestartpos.front())
			pos += linestartpos.front();
		auto it = upper_bound(linestartpos.begin(), linestartpos.end(), pos);
		if (it != linestartpos.begin())
			it--;
		line = it - linestartpos.begin() + 1 + startline;
		linepos = pos - *it + 1;
	}

	//代码范围是[lb, ub]
	void getLineRange(int32_t &lb, int32_t &ub)
	{
		lb = startline;
		ub = linestartpos.size() + startline - 1;
	}
};

class Bagel_VarPosInfo;

struct Bagel_ByteCode : public Bagel_Object
{
	vector<Bagel_ByteCodeStruct> code;
	vector<Bagel_Var> consts;
	int stackDepth;
	int localDepth;
	bool release;
	bool needSpecialStack;	//是否包含闭包函数，从而该字节码运行的栈不会被立即释放

	int runTimes;

	shared_ptr<Bagel_DebugInformation> debugInfo;
	shared_ptr<Bagel_VarPosInfo> varPosInfo;

	Bagel_ByteCode() : Bagel_Object(VAR_BYTECODE_P), stackDepth(0), localDepth(0), release(false), needSpecialStack(false), runTimes(0), debugInfo(NULL), varPosInfo(NULL)
	{
	}

	~Bagel_ByteCode()
	{
	}

	virtual void markChildren() override
	{
		for (auto &it : consts)
			_GC.GC_Markself(it);
	};
};

void dumpByteCode(Bagel_ByteCode &code);