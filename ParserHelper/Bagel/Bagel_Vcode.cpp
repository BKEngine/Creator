#include "Bagel_Vcode.h"
#include "Bagel_Utils.h"

const BKE_Char* op_name[]=
{
	W("BC_INVALID"),			//release编译时用来标记将要被移除的字节码
						//arithmetic
	W("BC_ADD"),				//[A]=[B]+[C]
	W("BC_SUB"),				//[A]=[B]-[C]
	W("BC_MUL"),				//[A]=[B]*[C]
	W("BC_DIV"),				//[A]=[B]/[C]
	W("BC_INTDIV"),				//[A]=[B]/[C]
	W("BC_MOD"),				//[A]=[B]%[C]
	W("BC_POW"),				//[A]=[B]^[C]
	W("BC_ADDCS"),				//[A]=const[B]+[C]
	W("BC_SUBCS"),				//[A]=const[B]-[C]
	W("BC_MULCS"),				//[A]=const[B]*[C]
	W("BC_DIVCS"),				//[A]=const[B]/[C]
	W("BC_INTDIVCS"),			//[A]=const[B]\[C]
	W("BC_MODCS"),				//[A]=const[B]%[C]
	W("BC_POWCS"),				//[A]=const[B]^[C]
	W("BC_ADDSC"),				//[A]=[B]+const[C]
	W("BC_SUBSC"),				//[A]=[B]-const[C]
	W("BC_MULSC"),				//[A]=[B]*const[C]
	W("BC_DIVSC"),				//[A]=[B]/const[C]
	W("BC_INTDIVSC"),			//[A]=[B]\const[C]
	W("BC_MODSC"),				//[A]=[B]%const[C]
	W("BC_POWSC"),				//[A]=[B]^const[C]

						//boolean
	W("BC_EQUAL"),			//[A]=[B]==[C]
	W("BC_NEQUAL"),			//[A]=[B]!=[C]
	W("BC_EEQUAL"),			//[A]=[B]===[C]
	W("BC_NNEQUAL"),			//[A]=[B]!==[C]
	W("BC_LARGER"),			//[A]=[B]>[C]
	W("BC_SMALLER"),			//[A]=[B]<[C]
	W("BC_LE"),				//[A]=[B]>=[C]
	W("BC_SE"),				//[A]=[B]<=[C]

						//logic
	W("BC_NOT"),				//[A]=![B]
	W("BC_BITAND"),			//[A]=[B] bitand [C]
	W("BC_BITOR"),			//[A]=[B] bitor [C]
	W("BC_BITXOR"),			//[A]=[B] bitxor [C]
	W("BC_BITNOT"),			//[A]=bitnot [B]

						//convert
	W("BC_CLONE"),			//[A]=[B].clone()
	W("BC_TOBOOL"),			//[A]=(bool)[B]
	W("BC_TOINT"),			//[A]=(int)[B]
	W("BC_TOSTRING"),		//[A]=(string)[B]
	W("BC_TONUMBER"),		//[A]=(number)[B]
	W("BC_TOMINUSNUMBER"),	//[A]=-(number)[B]

						//stack set
	W("BC_MOV"),				//[A].forceSet([B])
	W("BC_SETADD"),			//[A]+=[B]
	W("BC_SETSUB"),			//[A]-=[B]
	W("BC_SETMUL"),			//[A]*=[B]
	W("BC_SETDIV"),			//[A]/=[B]
	W("BC_SETINTDIV"),		//[A]=[B]/[C]
	W("BC_SETMOD"),			//[A]%=[B]
	W("BC_SETPOW"),			//[A]^=[B]
	W("BC_SETSET"),			//[A]|=[B]
	W("BC_SELFINC"),			//[C]=([A]++)
	W("BC_SELFDEC"),			//[C]=([A]--)
	W("BC_PREINC"),			//++[A]
	W("BC_PREDEC"),			//--[A]
	W("BC_MOVC"),			//[A].forceSet(const[B]), 和BC_LOADCONST一样
	W("BC_SETADDC"),			//[A]+=const[B]
	W("BC_SETSUBC"),			//[A]-=const[B]
	W("BC_SETMULC"),			//[A]*=const[B]
	W("BC_SETDIVC"),			//[A]/=const[B]
	W("BC_SETINTDIVC"),		//[A]\=const[B]
	W("BC_SETMODC"),			//[A]%=const[B]
	W("BC_SETPOWC"),			//[A]^=const[B]
	W("BC_SETSETC"),			//[A]|=const[B]

						//load
	W("BC_COPYCONST"),		//[A].copyFrom(const[[B]])
	W("BC_LOADCONST"),		//A=const[[B]]
	W("BC_LOADGLOBAL"),		//A=new global
	W("BC_LOADVOID"),		//A=new void
	W("BC_LOADNUM"),			//A=new num(*(double*)B)
	W("BC_INITSTACKMEMBER"),	//A=pointer(clostack[B]);
	W("BC_INITSTACKMEMBER2"),	//A=pointer(clostack[B].tag);
	W("BC_LOADPOINTER"),		//A=*B;B is pointer
	W("BC_LOADPOINTER2"),		//A=*B;B is pointer	//PTR_SETxxx系专用，使用且不清空self信息
	W("BC_SETPOINTER"),		//*A=B;A is pointer
	W("BC_SETPOINTER2"),		//*A=B;A is pointer	//PTR_SETxxx系专用，使用self信息

						//member
	W("BC_SETMEMBER2"),				//[C]=([A].[B]=[C])不往上级闭包寻找，强制创建当前闭包局部变量。
	W("BC_LOADCLOMEMBER_VALUE"),		//[C]=[A].[B],不需要dotFunc
	W("BC_LOADMEMBER_VALUE"),		//[C]=[A].[B]
	W("BC_LOADGLOBALMEMBER_VALUE"),	//[C]=global.const[A]
	W("BC_LOADSUPERMEMBER_VALUE"),	//[C]=super([A]).[B]
	W("BC_LOADSUPERMEMBER_PTR"),	//[C]=super([A]).[B]
	W("BC_LOADMEMBERIDX_VALUE"),	//[C]=[A][(B)]
	W("BC_LOADMEMBERSTRIDX_VALUE"),	//[C]=[A].const[B]
	W("BC_LOADARRAYIDX_VALUE"),	//[C]=pointer([A],[B]), [A] is array or str
	W("BC_LOADARRAYIDX2_VALUE"),	//[C]=pointer([A],B), [A] is array or str

								//memberptr
	W("BC_GETMEMBER_PTR"),			//[C]=pointer([A],[B])
	W("BC_GETCLOMEMBER_PTR"),		//[C]=pointer(Closure([A]),[B])
	W("BC_GETGLOBALMEMBER_PTR"),		//[C]=pointer(global,const[A])
	W("BC_GETMEMBERIDX_PTR"),		//[C]=pointer([A],B)
	W("BC_GETMEMBERSTRIDX_PTR"),		//[C]=pointer([A],const[B])
	W("BC_GETARRAYIDX_PTR"),		//pointer([A],[B]), [A] is array or str
	W("BC_GETARRAYIDX2_PTR"),		//pointer([A],B), [A] is array

								//ptrset
	W("BC_PTR_SET"),					//*[A]=[B]
	W("BC_PTR_SETADD"),				//*[A]+=[B]
	W("BC_PTR_SETSUB"),				//*[A]-=[B]
	W("BC_PTR_SETMUL"),				//*[A]*=[B]
	W("BC_PTR_SETDIV"),				//*[A]/=[B]
	W("BC_PTR_SETINTDIV"),			//*[A]\=[B]
	W("BC_PTR_SETMOD"),				//*[A]%=[B]
	W("BC_PTR_SETPOW"),				//*[A]^=[B]
	W("BC_PTR_SETSET"),				//*[A]|=[B]
	W("BC_PTR_SELFINC"),				//[C]=(*[A]++)
	W("BC_PTR_SELFDEC"),				//[C]=(*[A]--)
	W("BC_PTR_PREINC"),				//*[A]
	W("BC_PTR_PREDEC"),				//*[A]
	W("BC_PTR_SETC"),				//*[A]=const[B]
	W("BC_PTR_SETADDC"),				//*[A]+=const[B]
	W("BC_PTR_SETSUBC"),				//*[A]-=const[B]
	W("BC_PTR_SETMULC"),				//*[A]*=const[B]
	W("BC_PTR_SETDIVC"),				//*[A]/=const[B]
	W("BC_PTR_SETINTDIVC"),			//*[A]\=const[B]
	W("BC_PTR_SETMODC"),				//*[A]%=const[B]
	W("BC_PTR_SETPOWC"),				//*[A]^=const[B]
	W("BC_PTR_SETSETC"),				//*[A]|=const[B]

								//construct
	W("BC_MAKEARRAY"),		//A=new array([B], ..., [B+C-1])
	W("BC_MAKEDIC"),			//A=new dic([B], ..., [B+C-1])
	W("BC_MAKECLASS"),		//A=new class([B], ..., [B+C-1])
	W("BC_SETCLOSURE"),		//[A].setClosure([B])
	W("BC_SETCLOSURESTACK"),	//[A].setStack()
	W("BC_ADDPROPGET"),		//[A].addPropGet([B] as name, [C] as bytecode)
	W("BC_ADDPROPSET"),		//[A].addPropSet([B] as name, [B+1] as paramname, [B+2] as bytecode)
	W("BC_ADDPROPERTY"),	//[A].addProperty([B] as name, [B+1] as get_bytecode, [B+2] as setter name, [B+3] as set_bytecode
	W("BC_ADDSTATIC"),		//[A].addStaticVar([B] as name, [C] as value)
	W("BC_ADDVAR"),			//[A].addClassVar([B] as name, [C] as value)
	W("BC_ADDFUNCTION"),		//[A].addFunction([B] as name, [C] as value)
	W("BC_FINISHCLASS"),		//[A]的所有children refreshAll()
	W("BC_MAKELOCALPROPGET"),	//[A]=prop, [A].tag=&[A], [B] as name, [C] as bytecode
	W("BC_MAKELOCALPROPSET"),	//[A]=prop, [A].tag=&[A], [B] as name, [B+1] as paramname, [B+2] as bytecode
	W("BC_MAKELOCALPROPERTY"),	//[A]=prop, [A].tag=&[A], [B] as name, [B+1] as get_bytecode, [B+2] as setter name, [B+3] as set_bytecode

						//control
	W("BC_PUSHCLO"),			//A=new clo([B] as parent)
	W("BC_JUMPFALSE"),		//if(![B])code=A
	W("BC_JUMPTRUE"),		//if([B])code=A
	W("BC_JUMPVOID"),		//if([B]===void)code=A
	W("BC_JUMPNOTVOID"),		//if([B]!==void)code=A
	W("BC_JUMP"),			//code=A
	W("BC_JUMPEQUAL"),		//if([B]==[C])code=A;
	W("BC_JUMPNEQUAL"),		//if([B]!=[C])code=A;
	W("BC_JUMPEEQUAL"),		//if([B]===[C])code=A;
	W("BC_JUMPNNEQUAL"),		//if([B]!==[C])code=A;
	W("BC_JUMPLARGER"),		//if([B]>[C])code=A;
	W("BC_JUMPSMALLER"),		//if([B]<[C])code=A;
	W("BC_JUMPLE"),			//if([B]>=[C])code=A;
	W("BC_JUMPSE"),			//if([B]<=[C])code=A;
//	W("BC_PRECALL"),			//check whether [A] is function or class
	W("BC_CALL"),			//[A] = call [A]([B],...,[B+C-1]),C=0表示无参数。
	W("BC_RETURN"),			//return [A];
	W("BC_YIELD"),			//yield return [A];
	W("BC_NULLVECTOR"),		//[A] = new Bagel_Vector()
	W("BC_VECTORPUSHBACK"),	//[A].pushback([B],...,[B+C-1])
	W("BC_VECTORCONCAT"),	//check [B] is array, then [A].concat([B])
	W("BC_CALLVECTOR"),		//[A] = call [A](extract array [B])
	W("BC_LOOPSTART"),		//约定[A]=start,[A+1]=stop,[A+2]=step,[C]=loopvar=start,若[A+2]为void，则[A+2]=start>stop?-1:1,若[A=2]==0抛出异常.B为对应LOOP_JUDGE后面的位置
	W("BC_LOOPSTEP"),		//[A]+=[A+2],判断是否满足([A]<=[A+1] && [A+2]>0) || ([A]>=[A+1] && [A+2]<0),满足则[C]=[A],code=B(继续循环)
	W("BC_LOOPSTEP2"),		//[A]+=[A+2],判断是否满足([A]<=[A+1] && [A+2]>0) || ([A]>=[A+1] && [A+2]<0),满足则[C]=[A],不满足则code=B(跳出)
							//W("BC_LOOPSTEP1"),		//判断是否满足[A]<[A+1],满足则[A]+=[A+2],[A+3]=[A],code=B
	//W("BC_LOOPSTEP2"),		//判断是否满足[A]<[A+1],满足则[A]+=[A+2],[A+3]=[A],code=B
	W("BC_FOREACHSTART"),	//if[A]为空, code=C, 如果[A]是[A]为dic时[A+1]=iterator,[A+1].tick=deletetick,否则[A+1]=void, [A+2].tick=inner key, [B]=key, [B-1]=value
	W("BC_NEXT"),			//按照[A]的类型执行下面三种
		//BC_NEXTSTRING,		//[A+2]++, [C]=[A+2], [C-1]=value, 若没结束([A+1]<[A].length)则code=B
		//BC_NEXTARRAY,		//[A+2]++, [C]=[A+2], [C-1]=value, 若没结束([A+1]<[A].length)则code=B
		//BC_NEXTDIC,			//如果[A].deletetick!=[A+1].tick,[A+1].it=[A].find([A+2]),[A+1].it++,[A+2]=*[A+1].it, [C]=[A+2], [C-1]=value, 若没结束([A+1]<[A].length)则code=B
	W("BC_ADDTRYHOOK"),		//trystack.emplace_back(A,B)
	W("BC_REMOVETRYHOOK"),	//trystack.pop_back()
	W("BC_REMOVETRYHOOKN"),	//trystack.pop_back() A times

						//functional
	W("BC_DELETE"),			//delete [A].[B]
	W("BC_THROW"),			//throw [A]
	W("BC_THROWCONST"),		//throw const[[A]]
	W("BC_TYPE"),			//[A]=typeof [B]
	W("BC_INSTANCE"),		//[A]=[B] instanceof [C]
	W("BC_INCONTEXTOF"),		//[A]=[B] incontextof [C]
	W("BC_ELEMENT"),			//[A]=[B][[C]:[C+1]:[C+2]]
	W("BC_MERGESELF"),			//if(clo->hasSelf), merge self to [A].tag

						//typecheck
	W("BC_CHECKTYPE"),		//[A].type != B时报异常
	W("BC_CHECKTHIS"),		//[1] === void时报异常

						//pointer
	W("BC_CREATEPOINTER"),	//[A]=Bagel_Pointer([B], [C])

						//compile tag
	W("BC_RESERVE_STACK"),	//栈上留下连续的[A]-[A]+[B]的空间，放函数的参数
						//force GC
	W("BC_GC"),
		//cache instruction (long)，cache都是横跨三条指令
	W("BC_CACHE_NONE"),				//code+=3,skip cache instruction,cache占位符
								//cache成功会再跳过两个指令
	W("BC_CACHE_GLOBAL_MEMBER"),		//code+=3,if(lobal.varmap.delete==A && global == ptr2)[C3]=dst;
	W("BC_CACHE_DIC_MEMBER"),			//if([A3].obj==ptr && global_class([A3]).varmap.insert==A && [A3].varmap.delete==A2)[C3]=dst2
	W("BC_CACHE_CLASS_VAR"),			//if([A3].obj==ptr && [A3].varmap.delete==A)[C3]=dst2,用在自身varmap已经有值的情况。（比如被赋值后）
	W("BC_CACHE_CLASS_FUNC"),			//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].def.varmap.delete==A2)[C3]=dst2
	W("BC_CACHE_TYPE_MEMBER"),			//if([A3].type==A && global_class([A3]).varmap.delete==C)[C3]=dst2(property and function需要复制一份新self的)
	W("BC_CACHE_CLO_MEMBER1"),			//if([A3].obj==ptr && [A3].varmap.delete==A)[C3]=dst2
	W("BC_CACHE_CLO_MEMBER2"),			//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].parent.varmap.delete==A2)[C3]=dst2,一个闭包的parent是不能被更改的
	W("BC_CACHE_CLO_MEMBER_EXT"),		//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].parent.varmap.insert==A2)[C3]=[A3].parent.parent[B3]
	W("BC_CACHE_GLOBAL_INCLASS"),		//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].defclass.varmap.insert==A2)[C3]=dst2,dst2 is in global
	W("BC_CACHE_GLOBAL_INCLASSDEF"),	//if([A3].obj==ptr && [A3].varmap.insert==A)[C3]=dst2,dst2 is in global
	W("BC_CACHE_CLASS_VARINDEX"),		//if([A3].type == VAR_CLASS && [A3].defclass.classid==A && [A3].varmap.delete==0)[C3]=[A3].cache[B]
	W("BC_CACHE_NEWCLASS_FUNC"),		//if([A3].type == VAR_CLASS && [A3].defclass.classid==A && [A3].varmap.insert==1)[C3]=dst

	W("BC_CACHE_GLOBAL_MEMBERV"),		//code+=3,if(global.varmap.delete==A && global == ptr2)[C3]=*dst;
	W("BC_CACHE_DIC_MEMBERV"),			//if([A3].obj==ptr && global_class([A3]).varmap.insert==A && [A3].varmap.delete==A2)[C3]=*dst2
	W("BC_CACHE_CLASS_VARV"),			//if([A3].obj==ptr && [A3].varmap.delete==A)[C3]=*dst2,用在自身varmap已经有值的情况。（比如被赋值后）
	W("BC_CACHE_CLASS_FUNCV"),			//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].def.varmap.delete==A2)[C3]=*dst2
	W("BC_CACHE_TYPE_MEMBERV"),			//if([A3].type==A && global_class([A3]).varmap.delete==C)[C3]=*dst2(property and function需要复制一份新self的)
	W("BC_CACHE_CLO_MEMBER1V"),			//if([A3].obj==ptr && [A3].varmap.delete==A)[C3]=*dst2
	W("BC_CACHE_CLO_MEMBER2V"),			//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].parent.varmap.delete==A2)[C3]=*dst2,一个闭包的parent是不能被更改的
	W("BC_CACHE_CLO_MEMBER_EXTV"),		//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].parent.varmap.insert==A2)[C3]=[A3].parent.parent[B3]
	W("BC_CACHE_GLOBAL_INCLASSV"),		//if([A3].obj==ptr && [A3].varmap.insert==A && [A3].defclass.varmap.insert==A2)[C3]=*dst2,*dst2 is in global
	W("BC_CACHE_GLOBAL_INCLASSDEFV"),	//if([A3].obj==ptr && [A3].varmap.insert==A)[C3]=*dst2,*dst2 is in global
	W("BC_CACHE_CLASS_VARINDEXV"),		//if([A3].type == VAR_CLASS && [A3].defclass.classid==A && [A3].varmap.delete==0)[C3]=[A3].cache[B]
	W("BC_CACHE_NEWCLASS_FUNCV"),		//if([A3].type == VAR_CLASS && [A3].defclass.classid==A && [A3].varmap.insert==1)[C3]=*dst
};

void dumpByteCode(Bagel_ByteCode & code)
{
	StringVal res;
	int i = 0;
	for (auto &it : code.code)
	{
		res += bkpInt2Str(i++);
		res += '.';
		res += '\t';
		StringVal name = op_name[it.opcode];
		name.append(StringVal(28 - name.length(), ' '));
		res += name;
		res += '\t';
		switch (it.opcode)
		{
		case Bagel_BC::BC_LOADNUM:
			res += bkpInt2Str(it.A);
			res += '\t';
			res += Bagel_Number::toString(it.D);
			break;
		case Bagel_BC::BC_LOADCONST:
		case Bagel_BC::BC_COPYCONST:
			res += bkpInt2Str(it.A);
			res += '\t';
			res += bkpInt2Str(it.B);
			res += '(';
			if (code.consts[it.B].getType() == VAR_STR)
				res += code.consts[it.B].saveString(false);
			else
				res += code.consts[it.B].getTypeString().getConstStr();
			res += ')';
			break;
		case Bagel_BC::BC_ADDCS:
		case Bagel_BC::BC_SUBCS:
		case Bagel_BC::BC_MULCS:
		case Bagel_BC::BC_DIVCS:
		case Bagel_BC::BC_INTDIVCS:
		case Bagel_BC::BC_MODCS:
		case Bagel_BC::BC_POWCS:
			res += bkpInt2Str(it.A);
			res += '\t';
			res += bkpInt2Str(it.B);
			res += '(';
			if (code.consts[it.B].getType() == VAR_STR)
				res += code.consts[it.B].saveString(false);
			else
				res += code.consts[it.B].getTypeString().getConstStr();
			res += ')';
			res += '\t';
			res += bkpInt2Str(it.C);
			break;
		case Bagel_BC::BC_SETADDC:
		case Bagel_BC::BC_SETSUBC:
		case Bagel_BC::BC_SETMULC:
		case Bagel_BC::BC_SETDIVC:
		case Bagel_BC::BC_SETINTDIVC:
		case Bagel_BC::BC_SETMODC:
		case Bagel_BC::BC_SETPOWC:
			res += bkpInt2Str(it.A);
			res += '\t';
			res += bkpInt2Str(it.B);
			res += '(';
			if (code.consts[it.B].getType() == VAR_STR)
				res += code.consts[it.B].saveString(false);
			else
				res += code.consts[it.B].getTypeString().getConstStr();
			res += ')';
			break;
		case Bagel_BC::BC_ADDSC:
		case Bagel_BC::BC_SUBSC:
		case Bagel_BC::BC_MULSC:
		case Bagel_BC::BC_DIVSC:
		case Bagel_BC::BC_INTDIVSC:
		case Bagel_BC::BC_MODSC:
		case Bagel_BC::BC_POWSC:
			res += bkpInt2Str(it.A);
			res += '\t';
			res += bkpInt2Str(it.B);
			res += '\t';
			res += bkpInt2Str(it.C);
			res += '(';
			if (code.consts[it.C].getType() == VAR_STR)
				res += code.consts[it.C].saveString(false);
			else
				res += code.consts[it.C].getTypeString().getConstStr();
			res += ')';
			break;
		default:
			res += bkpInt2Str(it.A);
			res += '\t';
			res += bkpInt2Str(it.B);
			res += '\t';
			res += bkpInt2Str(it.C);
			break;
		}
		res += '\n';
	}
	_globalStructures.logFunc(res);
}
