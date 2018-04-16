#include "Bagel_Include.h"

void Bagel_VM::continueBreakPoint(Bagel_ThreadHandle handle, bool eraseThisBP)
{
	Bagel_ThreadContext *ctx = (Bagel_ThreadContext*)handle;
	if (ctx->fetchLastCommand()->opcode != Bagel_BC::BC_DEBUG || ctx->runningclo == nullptr)
		return;
	if (!eraseThisBP)
	{
		ctx->runningclo->codepos = ctx->runningclo->codepos->realcode;
	}
	else
	{
		int groupsize = 1;
		auto rawop = ctx->runningclo->codepos->realcode->opcode;
		if ((rawop > Bagel_BC::BC_PTR_SET && rawop <= Bagel_BC::BC_PTR_PREDEC) || (rawop > Bagel_BC::BC_PTR_SETC && rawop <= Bagel_BC::BC_PTR_SETSETC))
		{
			groupsize = 4;
		}
		else if (rawop >= Bagel_BC::BC_CACHE_NONE)
		{
			groupsize = 3;
		}
		memcpy(ctx->runningclo->codepos, ctx->runningclo->codepos->realcode, groupsize * sizeof(Bagel_ByteCodeStruct));
		//这样bp中留下了废弃的debug信息，不过没什么事反正
	}
}

void Bagel_VM::cacheCloMemberValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx)
{
	Bagel_Var *dv;
	auto bcs2 = bcs - 3;
	//如果那个位置正好是debug指令，则写到对应的存储位置
	if (bcs2->opcode == Bagel_BC::BC_DEBUG)
	{
		bcs2 = bcs2->realcode;
	}
	auto bcs3 = bcs - 2;
	auto bcs_ = bcs - 1;
	auto A = bcs->A;
	auto clo = stack[A].forceAsClosure();
	if (clo->vt == VAR_CLASSDEF)
	{
		auto it = clo->varmap.find(str);
		if (it == clo->varmap.end())
		{
			dv = &getCurrentGlobal()->getMember(str);
			bcs2->opcode = Bagel_BC::BC_CACHE_GLOBAL_INCLASSDEFV;
			bcs2->ptr = stack[A].forceAsObject()->getID();
			bcs2->A = clo->varmap.inserttick;
			bcs3->dst = dv;
			bcs_->ptr = getCurrentGlobal()->getID();
		}
		else
		{
			dv = &it->second;
			bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VARV;
			bcs2->ptr = stack[A].forceAsObject()->getID();
			bcs2->A = clo->varmap.deletetick;
			bcs3->dst = dv;
		}
	}
	else if (clo->vt == VAR_CLASS)
	{
		auto c = (Bagel_Class*)clo;
		auto it = c->varmap.find(str);
		if (it == c->varmap.end())
		{
			it = c->defclass->varmap.find(str);
			if (it == c->defclass->varmap.end())
			{
				dv = &getCurrentGlobal()->getMember(str);
				bcs2->opcode = Bagel_BC::BC_CACHE_GLOBAL_INCLASSV;
				bcs2->ptr = stack[A].forceAsObject()->getID();
				bcs2->A = c->varmap.inserttick;
				bcs3->A = c->defclass->varmap.inserttick;
				bcs3->dst = dv;
				bcs_->ptr = getCurrentGlobal()->getID();
			}
			else
			{
				if (c->varmap.inserttick == 1)
				{
					bcs2->opcode = Bagel_BC::BC_CACHE_NEWCLASS_FUNCV;
					bcs2->A = c->defclass->classid;
					bcs2->dst = dv = &it->second;
				}
				else
				{
					bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_FUNCV;
					bcs2->ptr = stack[A].forceAsObject()->getID();
					bcs2->A = c->varmap.inserttick;
					bcs3->A = c->defclass->varmap.deletetick;
					bcs3->dst = &it->second;
				}
				if (it->second.getType() == VAR_PROP)
				{
					ctx->runningclo->hasSelf = true;
					ctx->runningclo->self.forceSet(stack[A]);
					ctx->runningclo->clo = c;
					stack[bcs->C].forceSet(it->second);
					//ctx->runningclo->retpos = &stack[bcs->C];
					//_GC.writeBarrierStack(ctx->runningclo->runstack);
					//it->second.forceAsProp()->VMGetWithSelf(stack[A], c, ctx);
					return;
					//c->tempvar.forceSet(new Bagel_Prop(*it->second.forceAsProp(), stack[A].forceAsClosure(), stack[A]));
					//dv = &c->tempvar;
				}
				else if (it->second.getType() == VAR_FUNC)
				{
					c->tempvar.forceSet(new Bagel_Function(*it->second.forceAsFunc(), stack[A].forceAsClosure(), stack[A]));
					dv = &c->tempvar;
				}
				else
				{
					dv = &it->second;
				}
			}
		}
		else
		{
			dv = &it->second;
			bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VARV;
			bcs2->ptr = stack[A].forceAsObject()->getID();
			bcs2->A = c->varmap.deletetick;
			bcs3->dst = dv;
		}
	}
	else
	{
		//release compile下的字节码，不会创建函数运行闭包，因此最深的闭包应当是[当前"global"]下一个类定义的一个类。
		//不会出现闭包的父闭包是类的情况。
		auto it = clo->varmap.find(str);
		if (it != clo->varmap.end() || clo->parent == NULL)
		{
			if (it == clo->varmap.end())
				dv = &clo->varmap[str];
			else
				dv = &it->second;
			bcs2->opcode = Bagel_BC::BC_CACHE_CLO_MEMBER1V;
			bcs2->ptr = stack[A].forceAsObject()->getID();
			bcs2->A = clo->varmap.deletetick;
			bcs3->dst = dv;
		}
		else
		{
			clo = clo->parent;
			it = clo->varmap.find(str);
			if (it != clo->varmap.end() || clo->parent == NULL)
			{
				if (it == clo->varmap.end())
					dv = &clo->varmap[str];
				else
					dv = &it->second;
				bcs2->opcode = Bagel_BC::BC_CACHE_CLO_MEMBER2V;
				bcs2->ptr = stack[A].forceAsObject()->getID();
				bcs2->A = stack[A].forceAsClosure()->varmap.inserttick;
				bcs3->A = clo->varmap.deletetick;
				bcs3->dst = dv;
			}
			else
			{
				dv = &clo->parent->getMember(str);
				bcs2->opcode = Bagel_BC::BC_CACHE_CLO_MEMBER_EXTV;
				bcs2->ptr = stack[A].forceAsObject()->getID();
				bcs2->A = stack[A].forceAsClosure()->varmap.inserttick;
				bcs3->A = clo->varmap.inserttick;
				bcs3->dst = dv;
			}
		}
	}
	stack[bcs->C].forceSet(*dv);
}

void Bagel_VM::cacheCloMemberAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx)
{
	ctx->parentObj = nullptr;
	Bagel_Var *dv;
	auto bcs2 = bcs - 3;
	//如果那个位置正好是debug指令，则写到对应的存储位置
	if (bcs2->opcode == Bagel_BC::BC_DEBUG)
	{
		bcs2 = bcs2->realcode;
	}
	auto bcs3 = bcs - 2;
	auto bcs_ = bcs - 1;
	auto A = bcs->A;
	auto c1 = stack[A].forceAsClosure();
	assert(c1);
	if (c1->vt == VAR_CLASSDEF)
	{
		auto it = c1->varmap.find(str);
		if (it == c1->varmap.end())
		{
			dv = &getCurrentGlobal()->getMember(str);
			bcs2->opcode = Bagel_BC::BC_CACHE_GLOBAL_INCLASSDEF;
			bcs2->ptr = stack[A].forceAsObject()->getID();
			bcs2->A = c1->varmap.inserttick;
			bcs3->dst = dv;
			bcs_->ptr = getCurrentGlobal()->getID();
		}
		else
		{
			dv = &it->second;
			bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VAR;
			bcs2->ptr = stack[A].forceAsObject()->getID();
			bcs2->A = c1->varmap.deletetick;
			bcs3->dst = dv;
			ctx->parentObj = c1;
			ctx->member.forceSet(str);
		}
	}
	else if (c1->vt == VAR_CLASS)
	{
		auto c = (Bagel_Class*)c1;
		auto it = c->varmap.find(str);
		if (it == c->varmap.end())
		{
			it = c->defclass->varmap.find(str);
			if (it == c->defclass->varmap.end())
			{
				dv = &getCurrentGlobal()->getMember(str);
				bcs2->opcode = Bagel_BC::BC_CACHE_GLOBAL_INCLASS;
				bcs2->ptr = stack[A].forceAsObject()->getID();
				bcs2->A = c->varmap.inserttick;
				bcs3->A = c->defclass->varmap.inserttick;
				bcs3->dst = dv;
				bcs_->ptr = getCurrentGlobal()->getID();
			}
			else if (it->second.getType() == VAR_FUNC)
			{
				dv = &c->varmap[str];
				bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VAR;
				bcs2->ptr = stack[A].forceAsObject()->getID();
				bcs2->A = c->varmap.deletetick;
				bcs3->dst = dv;
			}
			else
			{
				ctx->parentObj = c->defclass;
				ctx->member.forceSet(str);
				if (c->varmap.inserttick == 1)
				{
					bcs2->opcode = Bagel_BC::BC_CACHE_NEWCLASS_FUNC;
					bcs2->A = c->defclass->classid;
					bcs2->dst = dv = &it->second;
				}
				else
				{
					bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_FUNC;
					bcs2->ptr = stack[A].forceAsObject()->getID();
					bcs2->A = c->varmap.inserttick;
					bcs3->A = c->defclass->varmap.deletetick;
					bcs3->dst = dv = &it->second;
				}
				if (it->second.getType() == VAR_PROP)
				{
					ctx->runningclo->hasSelf = true;
					ctx->runningclo->self.forceSet(c);
					ctx->runningclo->clo = c;
					//c->tempvar.forceSet(new Bagel_Prop(*it->second.forceAsProp(), stack[A].forceAsClosure(), stack[A]));
					//dv = &c->tempvar;
				}
			}
		}
		else
		{
			dv = &it->second;
			bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VAR;
			bcs2->ptr = stack[A].forceAsObject()->getID();
			bcs2->A = c->varmap.deletetick;
			bcs3->dst = dv;
		}
	}
	else
	{
		auto it = c1->varmap.find(str);
		if (it != c1->varmap.end() || c1->parent == NULL)
		{
			if (it == c1->varmap.end())
				dv = &c1->varmap[str];
			else
				dv = &it->second;
			bcs2->opcode = Bagel_BC::BC_CACHE_CLO_MEMBER1;
			bcs2->ptr = stack[A].forceAsObject()->getID();
			bcs2->A = c1->varmap.deletetick;
			bcs3->dst = dv;
		}
		else
		{
			auto clo = c1->parent;
			it = clo->varmap.find(str);
			if (it != clo->varmap.end() || clo->parent == NULL)
			{
				if (it == clo->varmap.end())
					dv = &clo->varmap[str];
				else
					dv = &it->second;
				bcs2->opcode = Bagel_BC::BC_CACHE_CLO_MEMBER2;
				bcs2->ptr = stack[A].forceAsObject()->getID();
				bcs2->A = stack[A].forceAsClosure()->varmap.inserttick;
				bcs3->A = clo->varmap.deletetick;
				bcs3->dst = dv;
			}
			else
			{
				dv = &clo->parent->getMember(str);
				bcs2->opcode = Bagel_BC::BC_CACHE_CLO_MEMBER_EXT;
				bcs2->ptr = stack[A].forceAsObject()->getID();
				bcs2->A = stack[A].forceAsClosure()->varmap.inserttick;
				bcs3->A = clo->varmap.inserttick;
				bcs3->dst = dv;
			}
		}
	}
	stack[bcs->C].tag = dv;
}

void Bagel_VM::noCacheCloMemberAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx)
{
	ctx->parentObj = nullptr;
	Bagel_Var *dv;
	auto A = bcs->A;
	auto c1 = stack[A].forceAsClosure();
	assert(c1);
	if (c1->vt == VAR_CLASSDEF)
	{
		auto it = c1->varmap.find(str);
		if (it == c1->varmap.end())
		{
			dv = &getCurrentGlobal()->getMember(str);
		}
		else
		{
			dv = &it->second;
			ctx->parentObj = c1;
			ctx->member.forceSet(str);
		}
	}
	else if (c1->vt == VAR_CLASS)
	{
		auto c = (Bagel_Class*)c1;
		auto it = c->varmap.find(str);
		if (it == c->varmap.end())
		{
			it = c->defclass->varmap.find(str);
			if (it == c->defclass->varmap.end())
			{
				dv = &getCurrentGlobal()->getMember(str);
			}
			else if (it->second.getType() == VAR_FUNC)
			{
				dv = &c->varmap[str];
			}
			else
			{
				dv = &it->second;
				ctx->parentObj = c->defclass;
				ctx->member.forceSet(str);
				if (it->second.getType() == VAR_PROP)
				{
					ctx->runningclo->hasSelf = true;
					ctx->runningclo->self.forceSet(c);
					ctx->runningclo->clo = c;
					//c->tempvar.forceSet(new Bagel_Prop(*it->second.forceAsProp(), stack[A].forceAsClosure(), stack[A]));
					//dv = &c->tempvar;
				}
			}
		}
		else
		{
			dv = &it->second;
		}
	}
	else
	{
		dv = &c1->getMember(str);
	}
	stack[bcs->C].tag = dv;
}

void Bagel_VM::cacheClassMemberValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx)
{
	Bagel_Var *dv;
	auto bcs2 = bcs - 3;
	//如果那个位置正好是debug指令，则写到对应的存储位置
	if (bcs2->opcode == Bagel_BC::BC_DEBUG)
	{
		bcs2 = bcs2->realcode;
	}
	auto bcs3 = bcs - 2;
	auto A = bcs->A;
	auto clo = stack[A].forceAsClosure();
	auto it = clo->varmap.find(str);
	if (clo->vt == VAR_CLASSDEF)
	{
		if (it == clo->varmap.end())
		{
			throw Bagel_Except(W("成员") + str.getConstStr() + W("不存在"));
		}
		bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VARV;
		bcs2->ptr = stack[A].forceAsObject()->getID();
		bcs2->A = clo->varmap.deletetick;
		bcs3->dst = &it->second;
		stack[bcs->C].forceSet(it->second);
		return;
	}
	auto c = (Bagel_Class*)clo;
	if (it == clo->varmap.end())
	{
		it = c->defclass->varmap.find(str);
		if (it == c->defclass->varmap.end())
		{
			throw Bagel_Except(W("成员") + str.getConstStr() + W("不存在"));
			//dv = &c->varmap[str];
			//bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VARV;
			//bcs2->ptr = stack[A].forceAsObject()->getID();
			//bcs2->A = c->varmap.deletetick;
			//bcs3->dst = dv;
		}
		if (c->varmap.inserttick == 1)
		{
			bcs2->opcode = Bagel_BC::BC_CACHE_NEWCLASS_FUNCV;
			bcs2->A = c->defclass->classid;
			bcs2->dst = dv = &it->second;
		}
		else
		{
			bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_FUNCV;
			bcs2->ptr = stack[A].forceAsObject()->getID();
			bcs2->A = c->varmap.inserttick;
			bcs3->A = c->defclass->varmap.deletetick;
			bcs3->dst = dv = &it->second;
		}
		if (it->second.getType() == VAR_PROP)
		{
			ctx->runningclo->hasSelf = true;
			ctx->runningclo->self.forceSet(stack[A]);
			ctx->runningclo->clo = c;
			stack[bcs->C].forceSet(it->second);
			//stack[bcs->C].setVoid();
			//ctx->runningclo->retpos = &stack[bcs->C];
			//_GC.writeBarrierStack(ctx->runningclo->runstack);
			//it->second.forceAsProp()->VMGetWithSelf(stack[A], c, ctx);
			return;
		}
		else if (it->second.getType() == VAR_FUNC)
		{
			c->tempvar.forceSet(new Bagel_Function(*it->second.forceAsFunc(), stack[A].forceAsClosure(), stack[A]));
			dv = &c->tempvar;
		}
		else
		{
			dv = &it->second;
		}
	}
	else
	{
		dv = &it->second;
		if (c->varmap.deletetick == 1)
		{
			int find = -1;
			for (int i = 0; i < c->cache.size(); i++)
			{
				if (c->cache[i] == dv)
				{
					find = i;
					break;
				}
			}
			if (find >= 0)
			{
				//class cache is valid
				bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VARINDEXV;
				bcs2->A = c->defclass->classid;;
				bcs2->B = find;
				stack[bcs->C].forceSet(*dv);
				return;
			}
		}
		bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VARV;
		bcs2->ptr = stack[A].forceAsObject()->getID();
		bcs2->A = c->varmap.deletetick;
		bcs3->dst = dv;
	}
	stack[bcs->C].forceSet(*dv);
}

void Bagel_VM::cacheClassMemberAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx)
{
	ctx->parentObj = nullptr;
	Bagel_Var *dv;
	auto bcs2 = bcs - 3;
	//如果那个位置正好是debug指令，则写到对应的存储位置
	if (bcs2->opcode == Bagel_BC::BC_DEBUG)
	{
		bcs2 = bcs2->realcode;
	}
	auto bcs3 = bcs - 2;
	auto A = bcs->A;
	auto clo = stack[A].forceAsClosure();
	if (clo->vt == VAR_CLASSDEF)
	{
		bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VAR;
		bcs2->ptr = stack[A].forceAsObject()->getID();
		bcs2->A = clo->varmap.deletetick;
		bcs3->dst = &clo->varmap[str];
		stack[bcs->C].tag = bcs3->dst;
		ctx->parentObj = clo;
		ctx->member.forceSet(str);
		return;
	}
	auto it = clo->varmap.find(str);
	auto c = (Bagel_Class*)clo;
	if (it == c->varmap.end())
	{
		it = c->defclass->varmap.find(str);
		if (it == c->defclass->varmap.end() || it->second.getType() == VAR_FUNC)
		{
			dv = &c->varmap[str];
			bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VAR;
			bcs2->ptr = stack[A].forceAsObject()->getID();
			bcs2->A = c->varmap.deletetick;
			bcs3->dst = dv;
		}
		else
		{
			ctx->parentObj = c->defclass;
			ctx->member.forceSet(str);
			if (c->varmap.inserttick == 1)
			{
				bcs2->opcode = Bagel_BC::BC_CACHE_NEWCLASS_FUNC;
				bcs2->A = c->defclass->classid;
				bcs2->dst = dv = &it->second;
			}
			else
			{
				bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_FUNC;
				bcs2->ptr = stack[A].forceAsObject()->getID();
				bcs2->A = c->varmap.inserttick;
				bcs3->A = c->defclass->varmap.deletetick;
				bcs3->dst = dv = &it->second;
			}
			if (dv->getType() == VAR_PROP)
			{
				ctx->runningclo->hasSelf = true;
				ctx->runningclo->self.forceSet(c);
				ctx->runningclo->clo = c;
				//c->tempvar.forceSet(new Bagel_Prop(*it->second.forceAsProp(), stack[A].forceAsClosure(), stack[A]));
				//dv = &c->tempvar;
			}
		}
	}
	else
	{
		dv = &it->second;
		if (c->varmap.deletetick == 1)
		{
			int find = -1;
			for (int i = 0; i < c->cache.size(); i++)
			{
				if (c->cache[i] == dv)
				{
					find = i;
					break;
				}
			}
			if (find >= 0)
			{
				//class cache is valid
				bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VARINDEX;
				bcs2->A = c->defclass->classid;
				bcs2->B = find;
				stack[bcs->C].tag = dv;
				return;
			}
		}
		bcs2->opcode = Bagel_BC::BC_CACHE_CLASS_VAR;
		bcs2->ptr = stack[A].forceAsObject()->getID();
		bcs2->A = c->varmap.deletetick;
		bcs3->dst = dv;
	}
	stack[bcs->C].tag = dv;
}

void Bagel_VM::noCacheMemberValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_ThreadContext *ctx)
{
	auto A = bcs->A;
	auto B = bcs->B;
	auto C = bcs->C;
	auto type = stack[A].getType();
	Bagel_ClassDef *cla = _globalStructures.typeclass[type];
	Bagel_Var *vv;
	if (!((type == VAR_ARRAY || type == VAR_STR) && stack[B].getType() == VAR_NUM) && cla && cla->hasClassMember(stack[B], &vv))
	{
		if (vv->getType() == VAR_FUNC)
		{
			stack[C].forceSet(new Bagel_Function(*vv->forceAsFunc(), Bagel_Closure::global(), stack[A]));
		}
		else if (vv->getType() == VAR_PROP)
		{
			ctx->runningclo->hasSelf = true;
			ctx->runningclo->self.forceSet(stack[A]);
			ctx->runningclo->clo = Bagel_Closure::global();
			stack[bcs->C].forceSet(*vv);
			//stack[bcs->C].setVoid();
			//ctx->runningclo->retpos = &stack[bcs->C];
			//_GC.writeBarrierStack(ctx->runningclo->runstack);
			//vv->forceAsProp()->VMGetWithSelf(stack[A], Bagel_Closure::global(), ctx);
		}
		else
		{
			stack[C].forceSet(*vv);
		}
	}
	else if (type == VAR_ARRAY && stack[B].canBeNumber())
	{
		stack[C].forceSet(stack[A].forceAsArray()->getMember((int)stack[B].asInteger()));
	}
	else if (type == VAR_STR && stack[B].canBeNumber())
	{
		stack[C].forceSet(stack[A].forceAsBKEStr()->getElement((int)stack[B].asInteger()));
	}
	else
	{
		Bagel_StringHolder str(stack[B]);
		switch (type)
		{
		case VAR_NONE:
			//对void取任何值都返回void。有时这种方式可以省去空对象（空数组或字典）的创建
			stack[C].setVoid();
			break;
		case VAR_NUM:
		case VAR_STR:
		case VAR_ARRAY:
			throw Bagel_Except(W("对于") + stack[A].saveShortString() + W("：指定的成员") + str.getConstStr() + W("不存在"));
		case VAR_DIC:
			stack[C].forceSet(stack[A].forceAsDic()->getMember(str));
			break;
		case VAR_CLO:
			stack[C].forceSet(stack[A].forceAsClosure()->getMember(str));
			break;
		case VAR_CLASS:
			stack[A].forceAsClass()->VMgetClassMember(str, &stack[C], ctx, false);
			//stack[C].forceSet(stack[A].forceAsClass()->getClassMemberValue(str));
			break;
		case VAR_CLASSDEF:
			stack[C].forceSet(stack[A].forceAsClassDef()->VMgetClassMemberValue(str));
			break;
		default:
			throw Bagel_Except(W("遇到非法的类型") + Bagel_Var::getTypeString(type).getConstStr());
		}
	}
}

void Bagel_VM::noCacheMemberAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_ThreadContext *ctx)
{
	ctx->parentObj = nullptr;
	auto A = bcs->A;
	auto B = bcs->B;
	auto C = bcs->C;
	auto type = stack[A].getType();
	Bagel_ClassDef *cla = _globalStructures.typeclass[type];
	Bagel_Var *vv;
	if (!((type == VAR_ARRAY || type == VAR_STR) && stack[B].getType() == VAR_NUM) && cla && cla->hasClassMember(stack[B], &vv))
	{
		ctx->parentObj = cla;
		ctx->member.forceSet(stack[B]);
		if (vv->getType() == VAR_FUNC)
		{
			stack[C].tag = vv;
		}
		else if (vv->getType() == VAR_PROP)
		{
			stack[C].tag = vv;
			ctx->runningclo->hasSelf = true;
			ctx->runningclo->self.forceSet(stack[A]);
			ctx->runningclo->clo = Bagel_Closure::global();
			//cla->tempvar.forceSet(new Bagel_Prop(*vv->forceAsProp(), NULL, stack[A]));
			//stack[C].tag = &cla->tempvar;
		}
		else
		{
			stack[C].tag = vv;
		}
	}
	else if (type == VAR_STR)
	{
		throw Bagel_Except(W("字符串不支持赋值"));
	}
	else
	{
		//手动内联后速度反而慢了

		switch (type)
		{
		case VAR_ARRAY:
			if (stack[B].canBeNumber())
				stack[C].tag = &stack[A].forceAsArray()->getMemberAddr((int32_t)stack[B].asInteger());
			else
				throw Bagel_Except(Bagel_Var::_getThrowString(W("没有成员") + stack[B].saveShortString(), &stack[A]) BAGEL_EXCEPT_EXT);
			break;
		case VAR_DIC:
			stack[C].tag = &stack[A].forceAsDic()->getMember(stack[B]);
			break;
		case VAR_CLO:
			stack[C].tag = &stack[A].forceAsClosure()->getMember(stack[B]);
			break;
		case VAR_CLASS:
			{
				auto c = stack[A].forceAsClass();
				Bagel_StringHolder str = (Bagel_StringHolder)stack[B];
				auto it = c->varmap.find(str);
				if (it != c->varmap.end())
					stack[C].tag = &it->second;
				auto it2 = c->defclass->varmap.find(str);
				if (it2 == c->defclass->varmap.end() || it2->second.getType() == VAR_FUNC)
				{
					stack[C].tag = &c->varmap[str];
				}
				else if (it2->second.getType() == VAR_PROP)
				{
					stack[C].tag = &it2->second;
					ctx->runningclo->hasSelf = true;
					ctx->runningclo->self.forceSet(c);
					ctx->runningclo->clo = c;
				}
				else
				{
					stack[C].tag = &it2->second;
					ctx->parentObj = c->defclass;
					ctx->member.forceSet(str);
				}
			}
			break;
		case VAR_CLASSDEF:
			stack[C].tag = &stack[A].forceAsClassDef()->getClassMemberAddr(stack[B]);
			ctx->parentObj = stack[A].forceAsClassDef();
			ctx->member.forceSet(stack[B]);
			break;
		default:
			throw Bagel_Except(W("对于") + stack[A].saveShortString() + W("：指定的成员") + stack[B].saveShortString() + W("不存在"));
		}

		//stack[C].tag = &stack[A][stack[B]];
	}
}

void Bagel_VM::cacheMemberStrValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx)
{
	auto A = bcs->A;
	auto C = bcs->C;
	auto type = stack[A].getType();
	Bagel_ClassDef *cla = _globalStructures.typeclass[type];
	Bagel_Var *vv;
	auto bcs2 = bcs - 3;
	//如果那个位置正好是debug指令，则写到对应的存储位置
	if (bcs2->opcode == Bagel_BC::BC_DEBUG)
	{
		bcs2 = bcs2->realcode;
	}
	auto bcs3 = bcs - 2;
	if (cla && cla->hasClassMember(str, &vv))
	{
		//type-function
		//set cache
		bcs2->opcode = Bagel_BC::BC_CACHE_TYPE_MEMBERV;
		bcs2->A = type;
		bcs2->C = cla->varmap.deletetick;
		bcs3->dst = vv;
		if (vv->getType() == VAR_FUNC)
		{
			stack[C].forceSet(new Bagel_Function(*vv->forceAsFunc(), Bagel_Closure::global(), stack[A]));
		}
		else if (vv->getType() == VAR_PROP)
		{
			ctx->runningclo->hasSelf = true;
			ctx->runningclo->self.forceSet(stack[A]);
			ctx->runningclo->clo = Bagel_Closure::global();
			stack[bcs->C].forceSet(*vv);
			//ctx->runningclo->retpos = &stack[bcs->C];
			//_GC.writeBarrierStack(ctx->runningclo->runstack);
			//vv->forceAsProp()->VMGetWithSelf(stack[A], Bagel_Closure::global(), ctx);
		}
		else
		{
			stack[C].forceSet(*vv);
		}
	}
	else if (type == VAR_ARRAY && str.s->canBeNumber())
	{
		stack[C].forceSet(stack[A].forceAsArray()->getMember((int)str.asNumber()));
	}
	else if (type == VAR_STR && str.s->canBeNumber())
	{
		stack[C].forceSet(stack[A].forceAsBKEStr()->getElement((int)str.asNumber()));
	}
	else
	{
		Bagel_Var *dv;
		switch (type)
		{
		case VAR_NONE:
			//对void取任何值都返回void。有时这种方式可以省去空对象（空数组或字典）的创建
			stack[C].setVoid();
			break;
		case VAR_NUM:
		case VAR_STR:
		case VAR_ARRAY:
			throw Bagel_Except(W("对于") + stack[A].saveShortString() + W("：指定的成员") + str.getConstStr() + W("不存在"));
		case VAR_DIC:
			{
				bool r = stack[A].forceAsDic()->getMemberValue(str, &dv);
				if (!r)
				{
					stack[C].setVoid();
					break;
				}
				bcs2->opcode = Bagel_BC::BC_CACHE_DIC_MEMBERV;
				bcs2->ptr = stack[A].forceAsObject()->getID();
				bcs2->A = cla->varmap.inserttick;
				bcs3->A = stack[A].forceAsDic()->varmap.deletetick;
				bcs3->dst = dv;
				stack[C].forceSet(*dv);
				break;
			}
		case VAR_CLASS:
		case VAR_CLASSDEF:
			cacheClassMemberValue(stack, bcs, str, ctx);
			break;
		case VAR_CLO:
			cacheCloMemberValue(stack, bcs, str, ctx);
			break;
		default:
			throw Bagel_Except(W("遇到非法的类型") + Bagel_Var::getTypeString(type).getConstStr());
		}
	}
}

void Bagel_VM::noCacheMemberStrValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx)
{
	auto A = bcs->A;
	auto C = bcs->C;
	auto type = stack[A].getType();
	Bagel_ClassDef *cla = _globalStructures.typeclass[type];
	Bagel_Var *vv;
	if (cla && cla->hasClassMember(str, &vv))
	{
		if (vv->getType() == VAR_FUNC)
		{
			stack[C].forceSet(new Bagel_Function(*vv->forceAsFunc(), Bagel_Closure::global(), stack[A]));
		}
		else if (vv->getType() == VAR_PROP)
		{
			ctx->runningclo->hasSelf = true;
			ctx->runningclo->self.forceSet(stack[A]);
			ctx->runningclo->clo = Bagel_Closure::global();
			stack[bcs->C].forceSet(*vv);
			//ctx->runningclo->retpos = &stack[bcs->C];
			//_GC.writeBarrierStack(ctx->runningclo->runstack);
			//vv->forceAsProp()->VMGetWithSelf(stack[A], Bagel_Closure::global(), ctx);
		}
		else
		{
			stack[C].forceSet(*vv);
		}
	}
	else if (type == VAR_ARRAY && str.s->canBeNumber())
	{
		stack[C].forceSet(stack[A].forceAsArray()->getMember((int)str.asNumber()));
	}
	else if (type == VAR_STR && str.s->canBeNumber())
	{
		stack[C].forceSet(stack[A].forceAsBKEStr()->getElement((int)str.asNumber()));
	}
	else
	{
		switch (type)
		{
		case VAR_NONE:
			//对void取任何值都返回void。有时这种方式可以省去空对象（空数组或字典）的创建
			stack[C].setVoid();
			break;
		case VAR_NUM:
		case VAR_STR:
		case VAR_ARRAY:
			throw Bagel_Except(W("对于") + stack[A].saveShortString() + W("：指定的成员") + str.getConstStr() + W("不存在"));
		case VAR_DIC:
			stack[C].forceSet(stack[A].forceAsDic()->getMemberValue(str));
			break;
		case VAR_CLO:
			stack[C].forceSet(stack[A].forceAsClosure()->getMember(str));
			break;
		case VAR_CLASS:
			stack[A].forceAsClass()->VMgetClassMember(str, &stack[C], ctx, false);
			//stack[C].forceSet(stack[A].forceAsClass()->getClassMemberValue(str));
			break;
		case VAR_CLASSDEF:
			stack[C].forceSet(stack[A].forceAsClassDef()->VMgetClassMemberValue(str));
			break;
		default:
			throw Bagel_Except(W("遇到非法的类型") + Bagel_Var::getTypeString(type).getConstStr());
		}
	}
}

void Bagel_VM::cacheMemberStrAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx)
{
	ctx->parentObj = nullptr;
	auto A = bcs->A;
	auto C = bcs->C;
	auto type = stack[A].getType();
	Bagel_ClassDef *cla = _globalStructures.typeclass[type];
	Bagel_Var *vv;
	auto bcs2 = bcs - 3;
	//如果那个位置正好是debug指令，则写到对应的存储位置
	if (bcs2->opcode == Bagel_BC::BC_DEBUG)
	{
		bcs2 = bcs2->realcode;
	}
	auto bcs3 = bcs - 2;
	if (cla && cla->hasClassMember(str, &vv))
	{
		//type-function
		//set cache
		bcs2->opcode = Bagel_BC::BC_CACHE_TYPE_MEMBER;
		bcs2->A = type;
		bcs2->C = cla->varmap.deletetick;
		bcs3->dst = vv;
		if (vv->getType() == VAR_FUNC)
		{
			//cla->tempvar.forceSet(new Bagel_Function(*vv->forceAsFunc(), Bagel_Closure::global(), stack[A]));
			//stack[C].tag = &cla->tempvar;
			throw Bagel_Except(W("不能给成员类型的函数赋值。"));
		}
		else if (vv->getType() == VAR_PROP)
		{
			stack[C].tag = vv;
			ctx->runningclo->hasSelf = true;
			ctx->runningclo->self.forceSet(stack[A]);
			ctx->runningclo->clo = Bagel_Closure::global();
			//cla->tempvar.forceSet(new Bagel_Prop(*vv->forceAsProp(), Bagel_Closure::global(), stack[A]));
			//stack[C].tag = &cla->tempvar;
		}
		else
		{
			stack[C].tag = vv;
			ctx->parentObj = cla;
			ctx->member.forceSet(str);
		}
	}
	else if (type == VAR_ARRAY && str.s->canBeNumber())
	{
		stack[C].tag = &stack[A].forceAsArray()->getMemberAddr((int)str.asNumber());
	}
	else
	{
		switch (type)
		{
		case VAR_NONE:
		case VAR_NUM:
		case VAR_ARRAY:
			throw Bagel_Except(W("对于") + stack[A].saveShortString() + W("：指定的成员") + str.getConstStr() + W("不存在"));
		case VAR_STR:
			throw Bagel_Except(W("字符串子串不能被赋值。"));
		case VAR_DIC:
			{
				Bagel_Var *dv;
				dv = &stack[A].forceAsDic()->getMember(str);
				bcs2->opcode = Bagel_BC::BC_CACHE_DIC_MEMBER;
				bcs2->ptr = stack[A].forceAsObject()->getID();
				bcs2->A = cla->varmap.inserttick;
				bcs3->A = stack[A].forceAsDic()->varmap.deletetick;
				bcs3->dst = dv;
				stack[C].tag = dv;
				break;
			}
		case VAR_CLASS:
		case VAR_CLASSDEF:
			//get addr
			cacheClassMemberAddr(stack, bcs, str, ctx);
			break;
		case VAR_CLO:
			cacheCloMemberAddr(stack, bcs, str, ctx);
			break;
		default:
			throw Bagel_Except(W("遇到非法的类型") + Bagel_Var::getTypeString(type).getConstStr());
		}
	}
}

void Bagel_VM::noCacheMemberStrAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, Bagel_ThreadContext *ctx)
{
	ctx->parentObj = nullptr;
	auto A = bcs->A;
	auto C = bcs->C;
	auto type = stack[A].getType();
	Bagel_ClassDef *cla = _globalStructures.typeclass[type];
	Bagel_Var *vv;
	if (cla && cla->hasClassMember(str, &vv))
	{
		if (vv->getType() == VAR_FUNC)
		{
			throw Bagel_Except(W("不能给成员类型的函数赋值。"));
			//cla->tempvar.forceSet(new Bagel_Function(*vv->forceAsFunc(), Bagel_Closure::global(), stack[A]));
			//stack[C].tag = &cla->tempvar;
		}
		else if (vv->getType() == VAR_PROP)
		{
			stack[C].tag = vv;
			ctx->runningclo->hasSelf = true;
			ctx->runningclo->self.forceSet(stack[A]);
			ctx->runningclo->clo = Bagel_Closure::global();
			//cla->tempvar.forceSet(new Bagel_Prop(*vv->forceAsProp(), Bagel_Closure::global(), stack[A]));
			//stack[C].tag = &cla->tempvar;
		}
		else
		{
			stack[C].tag = vv;
			ctx->parentObj = cla;
			ctx->member.forceSet(str);
		}
	}
	else if (type == VAR_ARRAY && str.s->canBeNumber())
	{
		stack[C].tag = &stack[A].forceAsArray()->getMemberAddr((int)str.asNumber());
	}
	else
	{
		switch (type)
		{
		case VAR_NONE:
		case VAR_NUM:
		case VAR_ARRAY:
			throw Bagel_Except(W("对于") + stack[A].saveShortString() + W("：指定的成员") + str.getConstStr() + W("不存在"));
		case VAR_STR:
			throw Bagel_Except(W("字符串子串不能被赋值。"));
		case VAR_DIC:
			stack[C].tag = &stack[A].forceAsDic()->getMember(str);
			break;
		case VAR_CLASS:
			{
				auto c = stack[A].forceAsClass();
				auto it = c->varmap.find(str);
				if (it != c->varmap.end())
					stack[C].tag = &it->second;
				auto it2 = c->defclass->varmap.find(str);
				if (it2 == c->defclass->varmap.end() || it2->second.getType() == VAR_FUNC)
				{
					stack[C].tag = &c->varmap[str];
				}
				else if (it2->second.getType() == VAR_PROP)
				{
					stack[C].tag = &it2->second;
					ctx->runningclo->hasSelf = true;
					ctx->runningclo->self.forceSet(c);
					ctx->runningclo->clo = c;
				}
				else
				{
					stack[C].tag = &it2->second;
					ctx->parentObj = c->defclass;
					ctx->member.forceSet(str);
				}
			}
			break;
		case VAR_CLASSDEF:
			stack[C].tag = &stack[A].forceAsClassDef()->getClassMemberAddr(str);
			ctx->parentObj = stack[A].forceAsClassDef();
			ctx->member.forceSet(str);
			break;
		case VAR_CLO:
			stack[C].tag = &stack[A].forceAsClosure()->getMember(str);
			break;
		default:
			throw Bagel_Except(W("遇到非法的类型") + Bagel_Var::getTypeString(type).getConstStr());
		}
	}
}

void Bagel_VM::noCacheMemberIdxValue(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_ThreadContext *ctx)
{
	auto A = bcs->A;
	auto B = bcs->B;
	auto C = bcs->C;
	auto type = stack[A].getType();
	if (type == VAR_ARRAY)
	{
		stack[C].forceSet(stack[A].forceAsArray()->getMember(B));
	}
	else if (type == VAR_STR)
	{
		stack[C].forceSet(stack[A].forceAsBKEStr()->getElement(B));
	}
	else
	{
		Bagel_ClassDef *cla = _globalStructures.typeclass[type];
		Bagel_Var *vv;
		Bagel_StringHolder str = B;
		if (cla && cla->hasClassMember(str, &vv))
		{
			if (vv->getType() == VAR_FUNC)
			{
				stack[C].forceSet(new Bagel_Function(*vv->forceAsFunc(), Bagel_Closure::global(), stack[A]));
			}
			else if (vv->getType() == VAR_PROP)
			{
				ctx->runningclo->hasSelf = true;
				ctx->runningclo->self.forceSet(stack[A]);
				ctx->runningclo->clo = Bagel_Closure::global();
				stack[bcs->C].forceSet(*vv);
				//stack[C].forceSet(vv->forceAsProp()->VMGetWithSelf(stack[A], Bagel_Closure::global(), ctx));
			}
			else
			{
				stack[C].forceSet(*vv);
			}
		}
		else
		{
			switch (type)
			{
			case VAR_NONE:
				//对void取任何值都返回void。有时这种方式可以省去空对象（空数组或字典）的创建
				stack[C].setVoid();
				break;
			case VAR_NUM:
				throw Bagel_Except(W("对于") + stack[A].saveShortString() + W("：指定的成员") + str.getConstStr() + W("不存在"));
			case VAR_DIC:
				stack[C].forceSet(stack[A].forceAsDic()->getMemberValue(str));
				break;
			case VAR_CLO:
				stack[C].forceSet(stack[A].forceAsClosure()->getMember(str));
				break;
			case VAR_CLASS:
				stack[A].forceAsClass()->VMgetClassMember(str, &stack[C], ctx, false);
				//stack[C].forceSet(stack[A].forceAsClass()->getClassMemberValue(str));
				break;
			case VAR_CLASSDEF:
				stack[C].forceSet(stack[A].forceAsClassDef()->VMgetClassMemberValue(str));
				break;
			}
		}
	}
}

void Bagel_VM::noCacheMemberIdxAddr(Bagel_Var *stack, Bagel_ByteCodeStruct *bcs, Bagel_ThreadContext *ctx)
{
	ctx->parentObj = nullptr;
	auto A = bcs->A;
	auto B = bcs->B;
	auto C = bcs->C;
	auto type = stack[A].getType();
	if (type == VAR_ARRAY)
	{
		stack[C].tag = &stack[A].forceAsArray()->getMemberAddr(B);
	}
	else if (type == VAR_STR)
	{
		throw Bagel_Except(W("字符串不支持赋值"));
	}
	else
	{
		Bagel_ClassDef *cla = _globalStructures.typeclass[type];
		Bagel_Var *vv;
		Bagel_StringHolder s = B;
		if (cla && cla->hasClassMember(s, &vv))
		{
			if (vv->getType() == VAR_FUNC)
			{
				throw Bagel_Except(W("不能给成员类型的函数赋值。"));
				//cla->tempvar.forceSet(new Bagel_Function(*vv->forceAsFunc(), Bagel_Closure::global(), stack[A]));
				//stack[C].tag = &cla->tempvar;
			}
			else if (vv->getType() == VAR_PROP)
			{
				stack[C].tag = vv;
				ctx->runningclo->hasSelf = true;
				ctx->runningclo->self.forceSet(cla);
				ctx->runningclo->clo = cla;
				//cla->tempvar.forceSet(new Bagel_Prop(*vv->forceAsProp(), Bagel_Closure::global(), stack[A]));
				//stack[C].tag = &cla->tempvar;
			}
			else
			{
				stack[C].tag = vv;
				ctx->parentObj = cla;
				ctx->member.forceSet(s);
			}
		}
		else
		{
			//			stack[C].tag = &stack[A][s];
			switch (type)
			{
			case VAR_NONE:
			case VAR_NUM:
			case VAR_ARRAY:
				throw Bagel_Except(W("对于") + stack[A].saveShortString() + W("：指定的成员") + s.getConstStr() + W("不存在"));
			case VAR_DIC:
				stack[C].tag = &stack[A].forceAsDic()->getMember(s);
				break;
			case VAR_CLASS:
				{
					auto c = stack[A].forceAsClass();
					auto it = c->varmap.find(s);
					if (it != c->varmap.end())
						stack[C].tag = &it->second;
					auto it2 = c->defclass->varmap.find(s);
					if (it2 == c->defclass->varmap.end() || it2->second.getType() == VAR_FUNC)
					{
						stack[C].tag = &c->varmap[s];
					}
					else if (it2->second.getType() == VAR_PROP)
					{
						stack[C].tag = &it2->second;
						ctx->runningclo->hasSelf = true;
						ctx->runningclo->self.forceSet(c);
						ctx->runningclo->clo = c;
					}
					else
					{
						stack[C].tag = &it2->second;
						ctx->parentObj = c->defclass;
						ctx->member.forceSet(s);
					}
				}
				break;
			case VAR_CLASSDEF:
				stack[C].tag = &stack[A].forceAsClassDef()->getClassMemberAddr(s);
				ctx->parentObj = stack[A].forceAsClassDef();
				ctx->member.forceSet(s);
				break;
			case VAR_CLO:
				stack[C].tag = &stack[A].forceAsClosure()->getMember(s);
				break;
			default:
				throw Bagel_Except(W("遇到非法的类型") + Bagel_Var::getTypeString(type).getConstStr());
			}
		}
	}
}

Bagel_Var& Bagel_VM::getGlobalMemberValue(Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, bool hasCache, Bagel_ThreadContext *ctx)
{
	Bagel_Var &v = getCurrentGlobal()->getMember(str);
	if (hasCache)
	{
		auto bcs2 = bcs - 3;
		//如果那个位置正好是debug指令，则写到对应的存储位置
		if (bcs2->opcode == Bagel_BC::BC_DEBUG)
		{
			bcs2 = bcs2->realcode;
		}
		auto bcs3 = bcs - 2;
		bcs2->opcode = Bagel_BC::BC_CACHE_GLOBAL_MEMBERV;
		bcs2->A = getCurrentGlobal()->varmap.deletetick;
		bcs2->dst = &v;
		bcs3->ptr = getCurrentGlobal()->getID();
	}
	return v;
}

Bagel_Var& Bagel_VM::getGlobalMemberAddr(Bagel_ByteCodeStruct *bcs, Bagel_StringHolder str, bool hasCache, Bagel_ThreadContext *ctx)
{
	Bagel_Var &v = getCurrentGlobal()->getMember(str);
	if (hasCache)
	{
		auto bcs2 = bcs - 3;
		//如果那个位置正好是debug指令，则写到对应的存储位置
		if (bcs2->opcode == Bagel_BC::BC_DEBUG)
		{
			bcs2 = bcs2->realcode;
		}
		auto bcs3 = bcs - 2;
		bcs2->opcode = Bagel_BC::BC_CACHE_GLOBAL_MEMBER;
		bcs2->A = getCurrentGlobal()->varmap.deletetick;
		bcs2->dst = &v;
		bcs3->ptr = getCurrentGlobal()->getID();
	}
	return v;
}

FORCEINLINE void RefreshClass(Bagel_Object *parentObj, const Bagel_Var &member)
{
	//if (parentObj->vt == VAR_CLASSDEF)
	{
		//一方面，父类的新增函数可能在子类新增了同名函数之后。
		//另外，新增函数时需要新建一个当前闭包的函数版本，而不是简单的拷贝对象指针。（这点应用于类定义/*或所有类*/上），类的实例不需要新建
		for (auto &it : ((Bagel_ClassDef*)parentObj)->children)
			it->refresh(member);
	}
}

//#define HANDLE_PROPGET(src, dst)					\
//if (stack[src].getType() == VAR_PROP)		\
//{											\
//	if(stack[src].forceAsProp()->hasGet())	\
//	{										\
//		clo->retpos = &stack[dst];				\
//		_GC.writeBarrierStack(clo->runstack);	\
//		stack[src].forceAsProp()->VMGet(btc);	\
//		goto loop;								\
//	}										\
//	else{stack[dst].setVoid();}				\
//}
//
//#define HANDLE_PROPSET(src, value)					\
//if (stack[src].tag->getType() == VAR_PROP)		\
//{											\
//	clo->retpos = nullptr;				\
//	_GC.writeBarrierStack(clo->runstack);	\
//	stack[src].tag->forceAsProp()->VMSet(value, btc);	\
//	goto loop;								\
//}

#define HANDLE_PROPGET_WITHSELF(src, dst)				\
if (stack[src].getType() == VAR_PROP)			\
{												\
	if(stack[src].forceAsProp()->hasGet())	\
	{										\
		clo->retpos = &stack[dst];					\
		_GC.writeBarrierStack(clo->runstack);		\
		if(clo->hasSelf)							\
			stack[src].forceAsProp()->VMGetWithSelf(clo->self, clo->clo, btc);		\
		else										\
			stack[src].forceAsProp()->VMGet(btc);	\
		clo->hasSelf = false;						\
		goto loop;									\
	}										\
	else{stack[dst].setVoid();}				\
}

#define HANDLE_PROPSET_WITHSELF(src, value)				\
if (stack[src].tag->getType() == VAR_PROP)			\
{												\
	clo->retpos = nullptr;					\
	_GC.writeBarrierStack(clo->runstack);		\
	if(clo->hasSelf)							\
		stack[src].tag->forceAsProp()->VMSetWithSelf(value, clo->self, clo->clo, btc);		\
	else										\
		stack[src].tag->forceAsProp()->VMSet(value, btc);	\
	clo->hasSelf = false;						\
	goto loop;									\
}

//#define HANDLE_PROP_PTR(ptr, exp)						\
//_GC.writeBarrierStack(clo->runstack);					\
//if (clo->hasSelf)										\
//	stack[ptr].forceSet(stack[ptr].tag->forceAsProp()->VMGetWithSelf(clo->self, clo->clo, btc, true));\
//else													\
//	stack[ptr].forceSet(stack[ptr].tag->forceAsProp()->VMGet(btc, true));\
//(exp);													\
//if (clo->hasSelf)										\
//	stack[ptr].tag->forceAsProp()->VMSetWithSelf(stack[ptr], clo->self, clo->clo, btc);\
//else													\
//	stack[ptr].tag->forceAsProp()->VMSet(stack[ptr], btc);	\
//goto loop;

#define QUICK_ARITH(a, b, c, op) if((b).getType() == VAR_NUM && (c).getType() == VAR_NUM)(a).forceSet((b).forceAsNumber() op (c).forceAsNumber());else (a).forceSet((b) op (c));
#define QUICK_ARITH2(a, b, op) if((a).getType() == VAR_NUM && (b).getType() == VAR_NUM)(a).forceAsNumber() op (b).forceAsNumber();else (a) op (b);

Bagel_Var Bagel_VM::_debugRun(Bagel_ThreadContext * btc, Bagel_ThreadHandle * out, int returnlevel)
{
	if (btc->callstack.size() <= returnlevel)
		return Bagel_Var();
	lastThread = btc;
	btc->status = Bagel_ThreadContext::THREAD_RUNNING;
	Bagel_ByteCodeStruct *bcs;
	//ptr所属parent
	//Bagel_Object *parentObj = NULL;
	//ptr对应member
	//Bagel_Var member;
	//前面是否有cache指令需要被重写
	bool &hasCache = btc->hasCache;
	Bagel_Var retvar;
	while (1)
	{
	loop:
		Bagel_RunClosure *clo = btc->runningclo;
		auto bc = clo->bc;
		auto code = bc->code.data();
		auto &&pos = clo->codepos;
		auto stack = clo->runstack->relativePos;
	//#if defined(_DEBUG) || defined(DEBUG)
	//	int bcsize = bc->code.size();
	//	auto codeend = code + bcsize;
	//#endif
		auto constpos = bc->consts.data();
		try
		{
			while (1)
			{
				//被DEBUG重定向的代码确实会可能超过codeend，所以这比较没啥意义了
			//#if defined(_DEBUG) || defined(DEBUG)
			//	//assert(pos < codeend);
			//	//if (pos >= codeend)
			//	//	__debugbreak();
			//#endif
				bcs = pos++;
			#if DEBUG_CYCLE
				cycles++;
			#endif
				//可能是少计算了的原因，比下面的赋值版本更快
			#define A (bcs->A)
			#define B (bcs->B)
			#define C (bcs->C)
			#define D (bcs->D)
				//auto A = bcs->A;
				//auto B = bcs->B;
				//auto C = bcs->C;
				switch (bcs->opcode)
				{
					//arithmetic
				case Bagel_BC::BC_ADD:
					QUICK_ARITH(stack[A], stack[B], stack[C], +);
					//stack[A].forceSet(stack[B] + stack[C]);
					break;
				case Bagel_BC::BC_SUB:
					QUICK_ARITH(stack[A], stack[B], stack[C], -);
					//stack[A].forceSet(stack[B] - stack[C]);
					break;
				case Bagel_BC::BC_MUL:
					QUICK_ARITH(stack[A], stack[B], stack[C], *);
					//stack[A].forceSet(stack[B] * stack[C]);
					break;
				case Bagel_BC::BC_DIV:
					QUICK_ARITH(stack[A], stack[B], stack[C], / );
					//stack[A].forceSet(stack[B] / stack[C]);
					break;
				case Bagel_BC::BC_INTDIV:
					{
						int t = stack[C].asInteger();
						if (t == 0)
							throw Bagel_Except(W("0不能被整除"));
						stack[A].forceSet(stack[B].asInteger() / t);
					}
					break;
				case Bagel_BC::BC_MOD:
					if (stack[B].getType() == VAR_NUM && stack[C].getType() == VAR_NUM)
						stack[A].forceSet(fmod(stack[B].forceAsNumber(), stack[C].forceAsNumber()));
					else
						stack[A].forceSet(stack[B] % stack[C]);
					break;
				case Bagel_BC::BC_POW:
					if (stack[B].getType() == VAR_NUM && stack[C].getType() == VAR_NUM)
						stack[A].forceSet(pow(stack[B].forceAsNumber(), stack[C].forceAsNumber()));
					else
						stack[A].forceSet(stack[B] ^ stack[C]);
					break;
				case Bagel_BC::BC_ADDCS:
					QUICK_ARITH(stack[A], constpos[B], stack[C], +);
					//stack[A].forceSet(constpos[B] + stack[C]);
					break;
				case Bagel_BC::BC_SUBCS:
					QUICK_ARITH(stack[A], constpos[B], stack[C], -);
					//stack[A].forceSet(constpos[B] - stack[C]);
					break;
				case Bagel_BC::BC_MULCS:
					QUICK_ARITH(stack[A], constpos[B], stack[C], *);
					//stack[A].forceSet(constpos[B] * stack[C]);
					break;
				case Bagel_BC::BC_DIVCS:
					QUICK_ARITH(stack[A], constpos[B], stack[C], / );
					//stack[A].forceSet(constpos[B] / stack[C]);
					break;
				case Bagel_BC::BC_INTDIVCS:
					{
						int t = stack[C].asInteger();
						if (t == 0)
							throw Bagel_Except(W("0不能被整除"));
						stack[A].forceSet(constpos[B].asInteger() / t);
					}
					break;
				case Bagel_BC::BC_MODCS:
					if (constpos[B].getType() == VAR_NUM && stack[C].getType() == VAR_NUM)
						stack[A].forceSet(fmod(constpos[B].forceAsNumber(), stack[C].forceAsNumber()));
					else
						stack[A].forceSet(constpos[B] % stack[C]);
					break;
				case Bagel_BC::BC_POWCS:
					if (constpos[B].getType() == VAR_NUM && stack[C].getType() == VAR_NUM)
						stack[A].forceSet(pow(constpos[B].forceAsNumber(), stack[C].forceAsNumber()));
					else
						stack[A].forceSet(constpos[B] ^ stack[C]);
					break;
				case Bagel_BC::BC_ADDSC:
					QUICK_ARITH(stack[A], stack[B], constpos[C], +);
					//stack[A].forceSet(stack[B] + constpos[C]);
					break;
				case Bagel_BC::BC_SUBSC:
					QUICK_ARITH(stack[A], stack[B], constpos[C], -);
					//stack[A].forceSet(stack[B] - constpos[C]);
					break;
				case Bagel_BC::BC_MULSC:
					QUICK_ARITH(stack[A], stack[B], constpos[C], *);
					//stack[A].forceSet(stack[B] * constpos[C]);
					break;
				case Bagel_BC::BC_DIVSC:
					QUICK_ARITH(stack[A], stack[B], constpos[C], / );
					//stack[A].forceSet(stack[B] / constpos[C]);
					break;
				case Bagel_BC::BC_INTDIVSC:
					{
						int t = constpos[C].asInteger();
						if (t == 0)
							throw Bagel_Except(W("0不能被整除"));
						stack[A].forceSet(stack[B].asInteger() / t);
					}
					break;
				case Bagel_BC::BC_MODSC:
					if (stack[B].getType() == VAR_NUM && constpos[C].getType() == VAR_NUM)
						stack[A].forceSet(fmod(stack[B].forceAsNumber(), constpos[C].forceAsNumber()));
					else
						stack[A].forceSet(stack[B] % constpos[C]);
					break;
				case Bagel_BC::BC_POWSC:
					if (stack[B].getType() == VAR_NUM && constpos[C].getType() == VAR_NUM)
						stack[A].forceSet(pow(stack[B].forceAsNumber(), constpos[C].forceAsNumber()));
					else
						stack[A].forceSet(stack[B] ^ constpos[C]);
					break;
					//boolean
				case Bagel_BC::BC_EQUAL:
					stack[A].forceSet(stack[B].normalEqual(stack[C]));
					break;
				case Bagel_BC::BC_NEQUAL:
					stack[A].forceSet(!stack[B].normalEqual(stack[C]));
					break;
				case Bagel_BC::BC_EEQUAL:
					stack[A].forceSet(stack[B].strictEqual(stack[C]));
					break;
				case Bagel_BC::BC_NNEQUAL:
					stack[A].forceSet(!stack[B].strictEqual(stack[C]));
					break;
				case Bagel_BC::BC_LARGER:
					stack[A].forceSet(stack[B] > stack[C]);
					break;
				case Bagel_BC::BC_SMALLER:
					stack[A].forceSet(stack[B] < stack[C]);
					break;
				case Bagel_BC::BC_LE:
					stack[A].forceSet(stack[B] >= stack[C]);
					break;
				case Bagel_BC::BC_SE:
					stack[A].forceSet(stack[B] <= stack[C]);
					break;
					//logic
				case Bagel_BC::BC_NOT:
					stack[A].forceSet(!stack[B]);
					break;
				case Bagel_BC::BC_BITAND:
					stack[A].forceSet(stack[B].asInteger() & stack[C].asInteger());
					break;
				case Bagel_BC::BC_BITOR:
					stack[A].forceSet(stack[B].asInteger() | stack[C].asInteger());
					break;
				case Bagel_BC::BC_BITXOR:
					stack[A].forceSet(stack[B].asInteger() ^ stack[C].asInteger());
					break;
				case Bagel_BC::BC_BITNOT:
					stack[A].forceSet(~stack[B].asInteger());
					break;
					//convert
				case Bagel_BC::BC_CLONE:
					stack[A].copyFrom(stack[B]);
					break;
				case Bagel_BC::BC_TOBOOL:
					stack[A].forceSet((bool)stack[B]);
					break;
				case Bagel_BC::BC_TOINT:
					stack[A].forceSet(stack[B].asInteger());
					break;
				case Bagel_BC::BC_TOSTRING:
					stack[A].forceSet(stack[B].asBKEStr());
					break;
				case Bagel_BC::BC_TONUMBER:
					stack[A].forceSet(stack[B].asNumber());
					break;
				case Bagel_BC::BC_TOMINUSNUMBER:
					stack[A].forceSet(-stack[B].asNumber());
					break;
				case Bagel_BC::BC_MOV:
					//暂时好像没发现需要判断prop的地方，出bug了再说
					stack[A].forceSet(stack[B]);
					//if (stack[B].getType() != VAR_PROP)
					//{
					//	stack[A].forceSet(stack[B]);
					//}
					//else
					//{
					//	HANDLE_PROPGET(B, A);
					//}
					break;
				case Bagel_BC::BC_SETADD:
					QUICK_ARITH2(stack[A], stack[B], +=);
					//stack[A] += stack[B];
					break;
				case Bagel_BC::BC_SETSUB:
					QUICK_ARITH2(stack[A], stack[B], -=);
					//stack[A] -= stack[B];
					break;
				case Bagel_BC::BC_SETMUL:
					QUICK_ARITH2(stack[A], stack[B], *=);
					//stack[A] *= stack[B];
					break;
				case Bagel_BC::BC_SETDIV:
					QUICK_ARITH2(stack[A], stack[B], /=);
					//stack[A] /= stack[B];
					break;
				case Bagel_BC::BC_SETINTDIV:
					{
						int t = stack[B].asInteger();
						if (t == 0)
							throw Bagel_Except(W("0不能被整除"));
						stack[A].forceSet(stack[A].asInteger() / t);
					}
					break;
				case Bagel_BC::BC_SETMOD:
					if (stack[A].getType() == VAR_NUM && stack[B].getType() == VAR_NUM)
						stack[A].forceAsNumber() = fmod(stack[A].forceAsNumber(), stack[B].forceAsNumber());
					else
						stack[A] %= stack[B];
					break;
				case Bagel_BC::BC_SETPOW:
					if (stack[A].getType() == VAR_NUM && stack[B].getType() == VAR_NUM)
						stack[A].forceAsNumber() = pow(stack[A].forceAsNumber(), stack[B].forceAsNumber());
					else
						stack[A] ^= stack[B];
					break;
				case Bagel_BC::BC_SETSET:
					if (stack[A].isVoid())
						stack[A].forceSet(stack[B]);
					break;
				case Bagel_BC::BC_SELFINC:
					stack[C].forceSet(stack[A]++);
					break;
				case Bagel_BC::BC_SELFDEC:
					stack[C].forceSet(stack[A]--);
					break;
				case Bagel_BC::BC_PREINC:
					++stack[A];
					break;
				case Bagel_BC::BC_PREDEC:
					--stack[A];
					break;
				case Bagel_BC::BC_MOVC:
					if (constpos[B].getType() == VAR_ARRAY || constpos[B].getType() == VAR_DIC)
						stack[A].copyFrom(constpos[B]);
					else
						stack[A].forceSet(constpos[B]);
					break;
				case Bagel_BC::BC_SETADDC:
					if (stack[A].isVoid() && constpos[B].getType() >= VAR_ARRAY)
						stack[A].copyFrom(constpos[B]);
					QUICK_ARITH2(stack[A], constpos[B], +=);
					//stack[A] += constpos[B];
					break;
				case Bagel_BC::BC_SETSUBC:
					QUICK_ARITH2(stack[A], constpos[B], -=);
					//stack[A] -= constpos[B];
					break;
				case Bagel_BC::BC_SETMULC:
					QUICK_ARITH2(stack[A], constpos[B], *=);
					//stack[A] *= constpos[B];
					break;
				case Bagel_BC::BC_SETDIVC:
					QUICK_ARITH2(stack[A], constpos[B], /=);
					//stack[A] /= constpos[B];
					break;
				case Bagel_BC::BC_SETINTDIVC:
					{
						int t = constpos[B].asInteger();
						if (t == 0)
							throw Bagel_Except(W("0不能被整除"));
						stack[A].forceSet(stack[A].asInteger() / t);
					}
					break;
				case Bagel_BC::BC_SETMODC:
					if (stack[A].getType() == VAR_NUM && constpos[B].getType() == VAR_NUM)
						stack[A].forceAsNumber() = fmod(stack[A].forceAsNumber(), constpos[B].forceAsNumber());
					else
						stack[A] %= constpos[B];
					break;
				case Bagel_BC::BC_SETPOWC:
					if (stack[A].getType() == VAR_NUM && constpos[B].getType() == VAR_NUM)
						stack[A].forceAsNumber() = pow(stack[A].forceAsNumber(), constpos[B].forceAsNumber());
					else
						stack[A] ^= constpos[B];
					break;
				case Bagel_BC::BC_SETSETC:
					if (stack[A].isVoid())
						stack[A].forceSet(constpos[B]);
					break;
					//load
				case Bagel_BC::BC_COPYCONST:
					stack[A].copyFrom(constpos[B]);
					break;
				case Bagel_BC::BC_LOADCONST:
					stack[A].forceSet(constpos[B]);
					break;
					//case Bagel_BC::BC_LOADTHIS:
					//	{
					//		auto c = stack[B].forceAsClosure()->getThisClosure();
					//		if (!c)
					//			throw W("该上下文不存在this闭包。");
					//		stack[A].forceSet(c);
					//	}
					//	break;
				case Bagel_BC::BC_LOADGLOBAL:
					stack[A].forceSet(getCurrentGlobal());
					break;
				case Bagel_BC::BC_LOADVOID:
					stack[A].setVoid();
					break;
				case Bagel_BC::BC_LOADNUM:
					stack[A].forceSet(D);
					break;
				case Bagel_BC::BC_INITSTACKMEMBER:
					stack[A].tag = &btc->runningclo->clostack->relativePos[B];
					break;
				case Bagel_BC::BC_INITSTACKMEMBER2:
					stack[A].tag = btc->runningclo->clostack->relativePos[B].tag;
					break;
				case Bagel_BC::BC_LOADPOINTER:
					stack[A].forceSet(*stack[B].tag);
					if (stack[A].getType() == VAR_PROP)
					{
						HANDLE_PROPGET_WITHSELF(A, A);
					}
					break;
				case Bagel_BC::BC_LOADPOINTER2:
					if (stack[B].tag->forceAsProp()->hasGet())
					{
						clo->retpos = &stack[A];
						_GC.writeBarrierStack(clo->runstack);
						if (clo->hasSelf)
							stack[B].tag->forceAsProp()->VMGetWithSelf(clo->self, clo->clo, btc);
						else
							stack[B].tag->forceAsProp()->VMGet(btc);
						goto loop;
					}
					else
					{
						stack[A].setVoid();
					}
					break;
				case Bagel_BC::BC_SETPOINTER:
					if (stack[A].tag->getType() == VAR_PROP)
					{
						HANDLE_PROPSET_WITHSELF(A, stack[B]);
					}
					else
					{
						stack[A].tag->forceSet(stack[B]);
					}
					//always add to GC
					if (stack[B].isObject())
						_GC.GC_Markself(stack[B].forceAsObject());
					break;
				case Bagel_BC::BC_SETPOINTER2:
					HANDLE_PROPSET_WITHSELF(A, stack[B]);
					//always add to GC
					if (stack[B].isObject())
						_GC.GC_Markself(stack[B].forceAsObject());
					break;
					//member
				case Bagel_BC::BC_SETMEMBER2:
					stack[A].setMember(stack[B], stack[C]);
					if (stack[C].isObject())
						_GC.writeBarrier(stack[A].forceAsObject(), stack[C].forceAsObject());
					break;
				case Bagel_BC::BC_LOADCLOMEMBER_VALUE:
					{
						if (!hasCache)
						{
							Bagel_Var *var;
							stack[A].forceAsClosure()->hasMember(constpos[B].forceAsBKEStr(), &var);
							stack[C].forceSet(*var);
						}
						else
						{
							cacheCloMemberValue(stack, bcs, constpos[B].forceAsBKEStr(), btc);
						}
						hasCache = false;
						HANDLE_PROPGET_WITHSELF(C, C);
					}
					break;
				case Bagel_BC::BC_LOADMEMBER_VALUE:
					noCacheMemberValue(stack, bcs, btc);
					hasCache = false;
					HANDLE_PROPGET_WITHSELF(C, C);
					break;
				case Bagel_BC::BC_LOADGLOBALMEMBER_VALUE:
					stack[C].forceSet(getGlobalMemberValue(bcs, constpos[A].forceAsBKEStr(), hasCache, btc));
					hasCache = false;
					HANDLE_PROPGET_WITHSELF(C, C);
					break;
				case Bagel_BC::BC_LOADSUPERMEMBER_VALUE:
					{
						if (stack[THISPOS].getType() == VAR_CLASS)
						{
							stack[C].forceSet(stack[THISPOS].forceAsClass()->getSuperMember(stack[B]));
						}
						else if (stack[THISPOS].getType() == VAR_CLASSDEF)
						{
							stack[C].forceSet(stack[THISPOS].forceAsClassDef()->getSuperMember(stack[B]));
						}
						else
						{
							throw Bagel_Except(W("this不是一个类。"));
						}
					}
					hasCache = false;
					HANDLE_PROPGET_WITHSELF(C, C);
					break;
				case Bagel_BC::BC_LOADSUPERMEMBER_PTR:
					{
						if (stack[THISPOS].getType() == VAR_CLASS)
						{
							stack[C].tag = &stack[THISPOS].forceAsClass()->getSuperMemberAddr(stack[B]);
						}
						else if (stack[THISPOS].getType() == VAR_CLASSDEF)
						{
							stack[C].tag = &stack[THISPOS].forceAsClassDef()->getSuperMemberAddr(stack[B]);
						}
						else
						{
							throw Bagel_Except(W("this不是一个类。"));
						}
						clo->hasSelf = true;
						clo->self.forceSet(stack[THISPOS]);
						clo->clo = stack[THISPOS].getClosure(nullptr);
						//出来的必定是property
						btc->parentObj = nullptr;
					}
					break;
				case Bagel_BC::BC_LOADMEMBERIDX_VALUE:
					noCacheMemberIdxValue(stack, bcs, btc);
					HANDLE_PROPGET_WITHSELF(C, C);
					break;
				case Bagel_BC::BC_LOADMEMBERSTRIDX_VALUE:
					if (hasCache)
					{
						cacheMemberStrValue(stack, bcs, constpos[B].forceAsBKEStr(), btc);
					}
					else
					{
						noCacheMemberStrValue(stack, bcs, constpos[B].forceAsBKEStr(), btc);
					}
					hasCache = false;
					HANDLE_PROPGET_WITHSELF(C, C);
					break;
				case Bagel_BC::BC_LOADARRAYIDX_VALUE:
					//assert(stack[A].getType() == VAR_ARRAY || stack[A].getType() == VAR_STR);
					if (stack[A].getType() == VAR_ARRAY)
					{
						stack[C].forceSet(stack[A].forceAsArray()->getMember((int)stack[B].forceAsNumber()));
					}
					else if (stack[A].getType() == VAR_STR)
					{
						stack[C].forceSet(stack[A].forceAsBKEStr()->getElement((int)stack[B].forceAsNumber()));
					}
					else
					{
						noCacheMemberValue(stack, bcs, btc);
					}
					break;
				case Bagel_BC::BC_LOADARRAYIDX2_VALUE:
					assert(stack[A].getType() == VAR_ARRAY || stack[A].getType() == VAR_STR);
					if (stack[A].getType() == VAR_ARRAY)
					{
						stack[C].forceSet(stack[A].forceAsArray()->getMember(B));
					}
					else
					{
						stack[C].forceSet(stack[A].forceAsBKEStr()->getElement(B));
					}
					break;
				case Bagel_BC::BC_GETMEMBER_PTR:
					noCacheMemberAddr(stack, bcs, btc);
					hasCache = false;
					break;
				case Bagel_BC::BC_GETCLOMEMBER_PTR:
					{
						Bagel_StringHolder str(constpos[B]);
						if (!hasCache)
						{
							noCacheCloMemberAddr(stack, bcs, str, btc);
						}
						else
						{
							cacheCloMemberAddr(stack, bcs, str, btc);
						}
						hasCache = false;
					}
					break;
				case Bagel_BC::BC_GETGLOBALMEMBER_PTR:
					btc->parentObj = nullptr;
					stack[C].tag = &getGlobalMemberAddr(bcs, constpos[A].forceAsBKEStr(), hasCache, btc);
					hasCache = false;
					break;
				case Bagel_BC::BC_GETMEMBERIDX_PTR:
					noCacheMemberIdxAddr(stack, bcs, btc);
					break;
				case Bagel_BC::BC_GETMEMBERSTRIDX_PTR:
					if (hasCache)
					{
						cacheMemberStrAddr(stack, bcs, constpos[B].forceAsBKEStr(), btc);
					}
					else
					{
						noCacheMemberStrAddr(stack, bcs, constpos[B].forceAsBKEStr(), btc);
					}
					hasCache = false;
					break;
				case Bagel_BC::BC_GETARRAYIDX_PTR:
					//assert(stack[A].getType() == VAR_ARRAY);
					if (stack[A].getType() == VAR_ARRAY)
					{
						stack[C].tag = &stack[A].forceAsArray()->getMemberAddr((int)stack[B].forceAsNumber());
						btc->parentObj = nullptr;
					}
					else if (stack[A].getType() == VAR_STR)
					{
						throw Bagel_Except(W("字符串不支持赋值"));
					}
					else
					{
						noCacheMemberAddr(stack, bcs, btc);
					}
					break;
				case Bagel_BC::BC_GETARRAYIDX2_PTR:
					assert(stack[A].getType() == VAR_ARRAY);
					stack[C].tag = &stack[A].forceAsArray()->getMemberAddr(B);
					btc->parentObj = nullptr;
					break;
				case Bagel_BC::BC_PTR_SET:
					if (stack[A].tag->getType() == VAR_PROP)
					{
						HANDLE_PROPSET_WITHSELF(A, stack[B]);
					}
					else if (btc->parentObj && /*btc->parentObj->vt == VAR_CLASSDEF && */stack[B].getType() == VAR_FUNC)
					{
						//重设closure和self
						(*stack[A].tag) = new Bagel_Function(*stack[B].forceAsFunc(), (Bagel_Closure*)btc->parentObj, btc->parentObj);
						_GC.writeBarrier(btc->parentObj, stack[A].tag->forceAsObject());
						RefreshClass(btc->parentObj, btc->member);
					}
					else
					{
						(*stack[A].tag).forceSet(stack[B]);
						if (stack[B].isObject())
							_GC.forceBarrier(stack[B].forceAsObject());
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
					}
					break;
				case Bagel_BC::BC_PTR_SETADD:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) += stack[B];
						if (stack[B].isObject())
							_GC.forceBarrier(stack[B].forceAsObject());
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] += stack[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETSUB:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) -= stack[B];
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] -= stack[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETMUL:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) *= stack[B];
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] *= stack[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETDIV:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) /= stack[B];
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] /= stack[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETINTDIV:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						stack[A].tag->forceSet(stack[A].tag->asInteger() / stack[B].asInteger());
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A].forceSet(stack[A].asInteger() / stack[B].asInteger()));
					}
					break;
				case Bagel_BC::BC_PTR_SETMOD:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) %= stack[B];
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] %= stack[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETPOW:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) ^= stack[B];
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] ^= stack[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETSET:
					{
						Bagel_Var &v = *stack[A].tag;
						if (v.getType() == VAR_PROP)
						{
							//_GC.writeBarrierStack(clo->runstack);
							//if (clo->hasSelf)
							//	stack[A].forceSet(stack[A].tag->forceAsProp()->VMGetWithSelf(clo->self, clo->clo, btc, true));
							//else
							//	stack[A].forceSet(stack[A].tag->forceAsProp()->VMGet(btc, true));
							//if (stack[A].isVoid())
							//{
							//	if (clo->hasSelf)
							//		stack[A].tag->forceAsProp()->VMSetWithSelf(stack[B], clo->self, clo->clo, btc);
							//	else
							//		stack[A].tag->forceAsProp()->VMSet(stack[B], btc);
							//	goto loop;
							//}
						}
						else if (v.isVoid())
						{
							if (btc->parentObj/* && parentObj->vt == VAR_CLASSDEF*/ && stack[B].getType() == VAR_FUNC)
							{
								//重设closure和self
								v.forceSet(new Bagel_Function(*stack[B].forceAsFunc(), (Bagel_Closure*)btc->parentObj, btc->parentObj));
							}
							else
							{
								v.forceSet(stack[B]);
							}
							if (stack[B].isObject())
								_GC.forceBarrier(v.forceAsObject());
							if (btc->parentObj)
								RefreshClass(btc->parentObj, btc->member);
							pos += 3;
						}
						else
						{
							pos += 3;
						}
					}
					break;
				case Bagel_BC::BC_PTR_SELFINC:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						stack[C].forceSet((*stack[A].tag)++);
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[C].forceSet(stack[A]++));
					}
					break;
				case Bagel_BC::BC_PTR_SELFDEC:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						stack[C].forceSet((*stack[A].tag)--);
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[C].forceSet(stack[A]--));
					}
					break;
				case Bagel_BC::BC_PTR_PREINC:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						++(*stack[A].tag);
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, ++stack[A]);
					}
					break;
				case Bagel_BC::BC_PTR_PREDEC:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						--(*stack[A].tag);
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, --stack[A]);
					}
					break;
				case Bagel_BC::BC_PTR_SETC:
					//常量的VarType<=VAR_DIC
					if (stack[A].tag->getType() != VAR_PROP)
					{
						if (constpos[B].getType() == VAR_ARRAY || constpos[B].getType() == VAR_DIC)
							(*stack[A].tag).copyFrom(constpos[B]);
						else
							(*stack[A].tag).forceSet(constpos[B]);
						if (stack[A].tag->isObject())
							_GC.forceBarrier(stack[A].tag->forceAsObject());
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
					}
					else
					{
						HANDLE_PROPSET_WITHSELF(A, constpos[B]);
						//不需要barrier也不需要refresh
					}
					break;
				case Bagel_BC::BC_PTR_SETADDC:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) += constpos[B];
						if (constpos[B].isObject())
							_GC.forceBarrier(constpos[B].forceAsObject());
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] += constpos[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETSUBC:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) -= constpos[B];
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] -= constpos[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETMULC:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) *= constpos[B];
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] *= constpos[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETDIVC:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) /= constpos[B];
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] /= constpos[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETINTDIVC:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						stack[A].tag->forceSet(stack[A].tag->asInteger() / constpos[B].asInteger());
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A].forceSet(stack[A].asInteger() / constpos[B].asInteger()));
					}
					break;
				case Bagel_BC::BC_PTR_SETMODC:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) %= constpos[B];
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] %= constpos[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETPOWC:
					if (stack[A].tag->getType() != VAR_PROP)
					{
						(*stack[A].tag) ^= constpos[B];
						if (btc->parentObj)
							RefreshClass(btc->parentObj, btc->member);
						pos += 3;
					}
					else
					{
						//HANDLE_PROP_PTR(A, stack[A] ^= constpos[B]);
					}
					break;
				case Bagel_BC::BC_PTR_SETSETC:
					{
						Bagel_Var &v = *stack[A].tag;
						if (v.getType() == VAR_PROP)
						{
							//_GC.writeBarrierStack(clo->runstack);
							//if (clo->hasSelf)
							//	stack[A].forceSet(stack[A].tag->forceAsProp()->VMGetWithSelf(clo->self, clo->clo, btc, true));
							//else
							//	stack[A].forceSet(stack[A].tag->forceAsProp()->VMGet(btc, true));
							//if (stack[A].isVoid())
							//{
							//	if (clo->hasSelf)
							//		stack[A].tag->forceAsProp()->VMSetWithSelf(constpos[B], clo->self, clo->clo, btc);
							//	else
							//		stack[A].tag->forceAsProp()->VMSet(constpos[B], btc);
							//	goto loop;
							//}
						}
						else if (v.isVoid())
						{
							if (constpos[B].getType() == VAR_ARRAY || constpos[B].getType() == VAR_DIC)
								v.copyFrom(constpos[B]);
							else
								v.forceSet(constpos[B]);
							if (constpos[B].isObject())
								_GC.forceBarrier(v.forceAsObject());
							if (btc->parentObj)
								RefreshClass(btc->parentObj, btc->member);
							pos += 3;
						}
						else
						{
							pos += 3;
						}
					}
					break;
					//construct
				case Bagel_BC::BC_MAKEARRAY:
					{
						auto a = new Bagel_Array();
						auto &arr = a->vararray;
						arr.resize(C);
						for (int i = 0; i < C; i++)
							arr[i].forceSet(stack[B + i]);
						stack[A].forceSet(a);
					}
					break;
				case Bagel_BC::BC_MAKEDIC:
					{
						auto a = new Bagel_Dic();
						for (int i = 0; i < C; i += 2)
							a->setMember(stack[B + i], stack[B + i + 1]);
						stack[A].forceSet(a);
					}
					break;
				case Bagel_BC::BC_MAKECLASS:
					{
						Bagel_String *name = stack[B];
						vector<Bagel_ClassDef*> v;
						for (int i = 1; i < C; i++)
						{
							v.push_back(stack[B + i].forceAsClassDef());
						}
						auto &cla = getCurrentGlobal()->varmap[name];
						if (cla.getType() == VAR_CLASSDEF)
						{
							if (cla.forceAsClassDef()->isFinal)
							{
								throw Bagel_Except(W("该Class(") + name->getConstStr() + W(")不能被重写"));
							}
							//需要更新class
							cla.forceAsClassDef()->redefineClass(v);
						}
						else
						{
							cla.forceSet(new Bagel_ClassDef(name, v));
						}
						stack[A].forceSet(cla);
						//cla.makeConst();
					}
					break;
				case Bagel_BC::BC_SETCLOSURE:
					{
						assert(stack[A].getType() == VAR_FUNC && !stack[A].forceAsFunc()->isNativeFunction());
						auto c = stack[B].forceAsClosure();
						assert(c);
						//if (!c)
						//	throw Bagel_Except(W("右操作数需要为一个闭包。"));
						//需重新建造function
						auto func = stack[A].forceAsFunc();
						stack[A].forceSet(new Bagel_Function(*func));
						stack[A].forceAsFunc()->setClosure(c);
					}
					break;
				case Bagel_BC::BC_SETCLOSURESTACK:
					{
						assert((stack[A].getType() == VAR_FUNC && !stack[A].forceAsFunc()->isNativeFunction()) || stack[A].getType() == VAR_PROP);
						//需重新建造function(已在SETCLOSURE中建造过)
						if (stack[A].getType() == VAR_FUNC)
							stack[A].forceAsFunc()->default_stack = clo->runstack;
						else
							stack[A].forceAsProp()->default_stack = clo->runstack;
					}
					break;
				case Bagel_BC::BC_ADDPROPGET:
					{
						auto c = stack[A].forceAsClosure();
						if (!c)
							throw Bagel_Except(W("左操作数需要为一个闭包。"));
						Bagel_StringHolder str(stack[B]);
						auto &v = c->varmap[str];
						if (v.getType() != VAR_PROP)
						{
							v.forceSet(new Bagel_Prop());
							v.forceAsProp()->name = str;
							v.forceAsProp()->setClosure(c);
						}
						if (c->vt == VAR_CLASSDEF)
						{
							v.forceAsProp()->getname = W("propget ") + ((Bagel_Class*)c)->classname + W(".") + v.forceAsProp()->name;
							((Bagel_ClassDef*)c)->ownvar.insertKey(str);
							v.forceAsProp()->setSelf(stack[A]);
						}
						else
						{
							v.forceAsProp()->getname = W("propget ") + v.forceAsProp()->name;
						}
						v.forceAsProp()->funcget = new Bagel_FunctionCode(*stack[C].forceAsObject<Bagel_FunctionCode>());
					}
					break;
				case Bagel_BC::BC_ADDPROPSET:
					{
						auto c = stack[A].forceAsClosure();
						if (!c)
							throw Bagel_Except(W("左操作数需要为一个闭包。"));
						Bagel_StringHolder str(stack[B]);
						auto &v = c->varmap[str];
						if (v.getType() != VAR_PROP)
						{
							v.forceSet(new Bagel_Prop());
							v.forceAsProp()->name = str;
							v.forceAsProp()->setClosure(c);
						}
						if (c->vt == VAR_CLASSDEF)
						{
							v.forceAsProp()->setname = W("propset ") + ((Bagel_Class*)c)->classname + W(".") + v.forceAsProp()->name;
							((Bagel_ClassDef*)c)->ownvar.insertKey(str);
							v.forceAsProp()->setSelf(stack[A]);
						}
						else
						{
							v.forceAsProp()->setname = W("propset ") + v.forceAsProp()->name;
						}
						v.forceAsProp()->funcset = new Bagel_FunctionCode(*stack[C].forceAsObject<Bagel_FunctionCode>());
					}
					break;
				case Bagel_BC::BC_ADDPROPERTY:
					{
						auto c = stack[A].forceAsClosure();
						if (!c)
							throw Bagel_Except(W("左操作数需要为一个闭包。"));
						Bagel_StringHolder str(stack[B]);
						auto &v = c->varmap[str];
						v.forceSet(new Bagel_Prop());
						v.forceAsProp()->name = str;
						v.forceAsProp()->setClosure(c);
						if (c->vt == VAR_CLASSDEF)
						{
							v.forceAsProp()->getname = W("propget ") + ((Bagel_Class*)c)->classname + W(".") + v.forceAsProp()->name;
							v.forceAsProp()->setname = W("propset ") + ((Bagel_Class*)c)->classname + W(".") + v.forceAsProp()->name;
							((Bagel_ClassDef*)c)->ownvar.insertKey(str);
							v.forceAsProp()->setSelf(stack[A]);
						}
						else
						{
							v.forceAsProp()->getname = W("propget ") + v.forceAsProp()->name;
							v.forceAsProp()->setname = W("propset ") + v.forceAsProp()->name;
						}
						if (stack[B + 1].isObject())
						{
							v.forceAsProp()->funcget = new Bagel_FunctionCode(*stack[B + 1].forceAsObject<Bagel_FunctionCode>());
						}
						if (stack[B + 2].isObject())
						{
							v.forceAsProp()->funcset = new Bagel_FunctionCode(*stack[B + 2].forceAsObject<Bagel_FunctionCode>());
						}
					}
					break;
				case Bagel_BC::BC_ADDSTATIC:
					{
						auto c = stack[A].forceAsClassDef();
						Bagel_StringHolder str(stack[B]);
						c->varmap[str].forceSet(stack[C]);
						c->ownvar.insertKey(str);
					}
					break;
				case Bagel_BC::BC_ADDVAR:
					{
						auto c = stack[A].forceAsClassDef();
						Bagel_StringHolder str(stack[B]);
						c->classvar[str].forceSet(stack[C]);
						c->ownvar.insertKey(str);
					}
					break;
				case Bagel_BC::BC_ADDFUNCTION:
					{
						auto c = stack[A].forceAsClosure();
						if (!c)
							throw Bagel_Except(W("左操作数需要为一个闭包。"));
						Bagel_StringHolder str(stack[B]);
						c->varmap[str].forceSet(stack[C]);
						if (c->vt == VAR_CLASSDEF)
						{
							stack[C].forceAsFunc()->setSelf(stack[A]);
							stack[C].forceAsFunc()->fullname = W("function ") + ((Bagel_Class*)c)->classname + W(".") + stack[C].forceAsFunc()->name;
							((Bagel_ClassDef*)c)->ownvar.insertKey(str);
						}
						else
						{
							stack[C].forceAsFunc()->fullname = W("function ") + stack[C].forceAsFunc()->name;
						}
					}
					break;
				case Bagel_BC::BC_FINISHCLASS:
					{
						auto cla = stack[A].forceAsClassDef();
						for (auto &it : cla->children)
						{
							it->refreshAll();
						}
					}
					break;
				case Bagel_BC::BC_MAKELOCALPROPGET:
					{
						Bagel_StringHolder str(stack[B]);
						auto &v = stack[A];
						if (v.getType() != VAR_PROP)
						{
							v.forceSet(new Bagel_Prop());
							v.forceAsProp()->name = str;
							v.forceAsProp()->setClosure(stack[0].forceAsClosure());
						}
						v.forceAsProp()->getname = W("propget ") + v.forceAsProp()->name;
						v.forceAsProp()->funcget = new Bagel_FunctionCode(*stack[C].forceAsObject<Bagel_FunctionCode>());
						v.forceAsProp()->default_stack = clo->runstack;
						v.tag = &v;
					}
					break;
				case Bagel_BC::BC_MAKELOCALPROPSET:
					{
						Bagel_StringHolder str(stack[B]);
						auto &v = stack[A];
						if (v.getType() != VAR_PROP)
						{
							v.forceSet(new Bagel_Prop());
							v.forceAsProp()->name = str;
							v.forceAsProp()->setClosure(stack[0].forceAsClosure());
						}
						v.forceAsProp()->setname = W("propset ") + v.forceAsProp()->name;
						v.forceAsProp()->funcset = new Bagel_FunctionCode(*stack[C].forceAsObject<Bagel_FunctionCode>());
						v.forceAsProp()->default_stack = clo->runstack;
						v.tag = &v;
					}
					break;
				case Bagel_BC::BC_MAKELOCALPROPERTY:
					{
						Bagel_StringHolder str(stack[B]);
						auto &v = stack[A];
						v.forceSet(new Bagel_Prop());
						v.forceAsProp()->name = str;
						v.forceAsProp()->setClosure(stack[0].forceAsClosure());
						v.forceAsProp()->getname = W("propget ") + v.forceAsProp()->name;
						v.forceAsProp()->setname = W("propset ") + v.forceAsProp()->name;
						if (stack[B + 1].isObject())
						{
							v.forceAsProp()->funcget = new Bagel_FunctionCode(*stack[B + 1].forceAsObject<Bagel_FunctionCode>());
						}
						if (stack[B + 2].isObject())
						{
							v.forceAsProp()->funcset = new Bagel_FunctionCode(*stack[B + 2].forceAsObject<Bagel_FunctionCode>());
						}
						v.forceAsProp()->default_stack = clo->runstack;
						v.tag = &v;
					}
					break;
					//control
				case Bagel_BC::BC_PUSHCLO:
					assert(stack[B].getType() == VAR_CLO);
					stack[A].forceSet(new Bagel_Closure(stack[B].forceAsClosure()));
					break;
				case Bagel_BC::BC_JUMPFALSE:
					if (!stack[B].asBoolean())
					{
						pos = code + A;
					}
					break;
				case Bagel_BC::BC_JUMPTRUE:
					if (stack[B].asBoolean())
					{
						pos = code + A;
					}
					break;
				case Bagel_BC::BC_JUMPVOID:
					if (stack[B].isVoid())
					{
						pos = code + A;
					}
					break;
				case Bagel_BC::BC_JUMPVOIDANDSET:
					if (stack[B].isVoid())
					{
						pos = code + A;
						stack[C].setVoid();
					}
					break;
				case Bagel_BC::BC_JUMPNOTVOID:
					if (!stack[B].isVoid())
					{
						pos = code + A;
					}
					break;
				case Bagel_BC::BC_JUMP:
					pos = code + A;
					break;
				case Bagel_BC::BC_JUMPEQUAL:
					if (stack[B].normalEqual(stack[C]))
						pos = code + A;
					break;
				case Bagel_BC::BC_JUMPNEQUAL:
					if (!stack[B].normalEqual(stack[C]))
						pos = code + A;
					break;
				case Bagel_BC::BC_JUMPEEQUAL:
					if (stack[B].strictEqual(stack[C]))
						pos = code + A;
					break;
				case Bagel_BC::BC_JUMPNNEQUAL:
					if (!stack[B].strictEqual(stack[C]))
						pos = code + A;
					break;
				case Bagel_BC::BC_JUMPLARGER:
					if (stack[B] > stack[C])
						pos = code + A;
					break;
				case Bagel_BC::BC_JUMPSMALLER:
					if (stack[B] < stack[C])
						pos = code + A;
					break;
				case Bagel_BC::BC_JUMPLE:
					if (stack[B] >= stack[C])
						pos = code + A;
					break;
				case Bagel_BC::BC_JUMPSE:
					if (stack[B] <= stack[C])
						pos = code + A;
					break;
					//case Bagel_BC::BC_PRECALL:
					//	if (stack[A].getType() != VAR_FUNC && stack[A].getType() != VAR_CLASS)
					//		throw Bagel_Except(W("(前必须是函数或类名"));
					//	break;
				case Bagel_BC::BC_CALL:
					//native function
					if (stack[A].getType() == VAR_FUNC)
					{
						//if (stack[A].forceAsFunc()->isNativeFunction())
						//{
						//	_GC.writeBarrierStack(clo->runstack, &stack[0], B + C - 1);
						//	stack[A].forceSet(stack[A].forceAsFunc()->VMDebugRun(&stack[B], C, btc));
						//}
						//else
						//{
							//TODO
						clo->retpos = &stack[A];
						_GC.writeBarrierStack(clo->runstack);
						stack[A].forceAsFunc()->VMRun(&stack[B], C, btc);
						//_GC.GC_little(GC_SPEED);
						goto loop;
						//}
					}
					else if (stack[A].getType() == VAR_CLASS)
					{
						//class
						clo->retpos = NULL;
						_GC.writeBarrierStack(clo->runstack);
						stack[A].forceSet(stack[A].forceAsClass()->defclass->VMcreateInstance(&stack[B], C, btc));
						//_GC.GC_little(GC_SPEED);
						goto loop;
					}
					else if (stack[A].getType() == VAR_CLASSDEF)
					{
						//class
						clo->retpos = NULL;
						_GC.writeBarrierStack(clo->runstack);
						stack[A].forceSet(stack[A].forceAsClassDef()->VMcreateInstance(&stack[B], C, btc));
						//_GC.GC_little(GC_SPEED);
						goto loop;
					}
					else
					{
						throw Bagel_Except(W("(前必须是函数或类名"));
					}
					break;
				case Bagel_BC::BC_RETURN:
					//最后一层则跳出
					if (btc->callstack.size() == returnlevel + 1)
					{
						retvar.forceSet(stack[A]);
						btc->popCallStack();
						_GC.GC_little(GC_SPEED);
						//_GC.GC_All();
						goto out;
					}
					else
					{
						auto clo2 = btc->callstack[btc->callstack.size() - 2];
						if (clo2->retpos)
							clo2->retpos->forceSet(stack[A]);
						btc->popCallStack();
						_GC.GC_little(GC_SPEED);
						goto loop;
					}
					break;
				case Bagel_BC::BC_YIELD:
					btc->status = Bagel_ThreadContext::THREAD_SLEEP;
					clo->retpos = &stack[B];
					return stack[A];
				case Bagel_BC::BC_NULLVECTOR:
					stack[A].forceSet(new Bagel_Vector());
					break;
				case Bagel_BC::BC_VECTORPUSHBACK:
					{
						auto arr = (Bagel_Vector*)stack[A].forceAsObject();
						arr->reserve(C);
						int i = 0;
						while (i < C)
						{
							arr->emplace_back(stack[B + i]);
						}
					}
					break;
				case Bagel_BC::BC_VECTORCONCAT:
					if (stack[B].getType() != VAR_ARRAY)
					{
						throw Bagel_Except(W("在参数中被展开的必须是数组"));
					}
					{
						auto arr = (Bagel_Vector*)stack[A].forceAsObject();
						auto arr2 = stack[B].forceAsArray();
						arr->reserve(arr->size() + arr2->getCount());
						for (auto &it : arr2->vararray)
						{
							arr->emplace_back(it);
						}
					}
					break;
				case Bagel_BC::BC_CALLVECTOR:
					//native function
					if (stack[A].getType() == VAR_FUNC)
					{
						clo->retpos = &stack[A];
						_GC.writeBarrierStack(clo->runstack);
						auto arr = stack[B].forceAsObject<Bagel_Vector>();
						stack[A].forceAsFunc()->VMRun(arr->data(), arr->size(), btc);
						goto loop;
					}
					else if (stack[A].getType() == VAR_CLASS)
					{
						//class
						clo->retpos = NULL;
						_GC.writeBarrierStack(clo->runstack);
						auto arr = stack[B].forceAsObject<Bagel_Vector>();
						stack[A].forceSet(stack[A].forceAsClass()->defclass->VMcreateInstance(arr->data(), arr->size(), btc));
						goto loop;
					}
					else if (stack[A].getType() == VAR_CLASSDEF)
					{
						//class
						clo->retpos = NULL;
						_GC.writeBarrierStack(clo->runstack);
						auto arr = stack[B].forceAsObject<Bagel_Vector>();
						stack[A].forceSet(stack[A].forceAsClassDef()->VMcreateInstance(arr->data(), arr->size(), btc));
						goto loop;
					}
					else
					{
						throw Bagel_Except(W("(前必须是函数或类名"));
					}
					break;
				case Bagel_BC::BC_LOOPSTART:
					stack[A].toNumber();
					stack[A + 1].toNumber();
					if (stack[A + 2].isVoid())
					{
						stack[A + 2].forceSetNum(stack[A].forceAsNumber() > stack[A + 1].forceAsNumber() ? -1 : 1);
					}
					else
					{
						stack[A + 2].toNumber();
					}
					//说不定人家里面有break，总之管他呢233
					//if (isZero(stack[A + 2].forceAsNumber()))
					//	throw Bagel_Except(W("步长不能为0"));
					stack[C].forceSet(stack[A]);
					break;
				case Bagel_BC::BC_LOOPSTEP:
					_GC.writeBarrierStack(clo->runstack);
					_GC.GC_little(GC_SPEED);
					stack[A].forceAsNumber() += stack[A + 2].forceAsNumber();
					if ((stack[A + 2].forceAsNumber() > 0 && stack[A].forceAsNumber() <= stack[A + 1].forceAsNumber()) ||
						(stack[A + 2].forceAsNumber() < 0 && stack[A].forceAsNumber() >= stack[A + 1].forceAsNumber()))
					{
						stack[C].forceSet(stack[A]);
						pos = code + B;
					}
					break;
				case Bagel_BC::BC_LOOPSTEP2:
					stack[A].forceAsNumber() += stack[A + 2].forceAsNumber();
					if ((stack[A + 2].forceAsNumber() > 0 && stack[A].forceAsNumber() <= stack[A + 1].forceAsNumber()) ||
						(stack[A + 2].forceAsNumber() < 0 && stack[A].forceAsNumber() >= stack[A + 1].forceAsNumber()))
					{
						stack[C].forceSet(stack[A]);
					}
					else
					{
						pos = code + B;
					}
					break;
				case Bagel_BC::BC_FOREACHSTART:
					{
						switch (stack[A].getType())
						{
						case VAR_ARRAY:
							{
								auto a = stack[A].forceAsArray();
								if (a->getCount() == 0)
								{
									pos = code + C;
									break;
								}
								stack[A + 2].tick = 0;
								stack[B].forceSetNum(0);
								stack[B - 1].forceSet(a->quickGetMember(0));
							}
							break;
						case VAR_DIC:
							{
								auto a = stack[A].forceAsDic();
								if (a->varmap.empty())
								{
									pos = code + C;
									break;
								}
								stack[A + 1].setVoid();
								stack[A + 1].it = a->varmap.begin();
								stack[A + 1].tick = a->varmap.deletetick;
								stack[A + 2].forceSet(stack[A + 1].it->first);
								stack[B].forceSet(stack[A + 1].it->first);
								stack[B - 1].forceSet(stack[A + 1].it->second);
							}
							break;
						case VAR_STR:
							{
								auto a = stack[A].forceAsBKEStr();
								if (a->empty())
								{
									pos = code + C;
									break;
								}
								stack[A + 2].tick = 0;
								stack[B].forceSetNum(0);
								stack[B - 1].forceSet(a->getConstStr()[0]);
							}
							break;
						default:
							pos = code + C;
							break;
						}
					}
					break;
				case Bagel_BC::BC_NEXT:
					_GC.writeBarrierStack(clo->runstack);
					_GC.GC_little(GC_SPEED);
					switch (stack[A].getType())
					{
					case VAR_ARRAY:
						{
							auto a = stack[A].forceAsArray();
							int &tick = stack[A + 2].tick;
							++tick;
							if (tick >= a->getCount())
							{
								break;
							}
							stack[C].forceSetNum(tick);
							stack[C - 1].forceSet(a->quickGetMember(tick));
							pos = code + B;
						}
						break;
					case VAR_DIC:
						{
							auto dic = stack[A].forceAsDic();
							if (dic->varmap.deletetick != stack[A + 1].tick)
							{
								stack[A + 1].it = dic->varmap.find(stack[A + 2].forceAsBKEStr());
								if (stack[A + 1].it == dic->varmap.end())
								{
									break;
								}
								stack[A + 1].tick = dic->varmap.deletetick;
							}
							++stack[A + 1].it;
							if (stack[A + 1].it == dic->varmap.end())
							{
								break;
							}
							stack[A + 2].forceSet(stack[A + 1].it->first);
							stack[C].forceSet(stack[A + 1].it->first);
							stack[C - 1].forceSet(stack[A + 1].it->second);
							pos = code + B;
						}
						break;
					case VAR_STR:
						{
							auto a = stack[A].forceAsBKEStr();
							int &tick = stack[A + 2].tick;
							++tick;
							if (tick >= a->size())
							{
								break;
							}
							stack[C].forceSetNum(tick);
							stack[C - 1].forceSet(a->getConstStr()[tick]);
							pos = code + B;
						}
						break;
					default:
						break;
					}
					break;
				case Bagel_BC::BC_ADDTRYHOOK:
					//clo->trystack.emplace_back(A, B);
					btc->trystack.emplace_back(A, B, btc->callstack.size());
					break;
				case Bagel_BC::BC_REMOVETRYHOOK:
					//clo->trystack.pop_back();
					btc->trystack.pop_back();
					break;
				case Bagel_BC::BC_REMOVETRYHOOKN:
					//clo->trystack.pop_back();
					{
						int i = A;
						while (i-- > 0)
							btc->trystack.pop_back();
					}
					break;
					//functional
				case Bagel_BC::BC_DELETE:
					switch (stack[A].getType())
					{
					case VAR_DIC:
						stack[A].forceAsDic()->deleteMemberIndex(stack[B]);
						break;
					case VAR_CLASS:
					case VAR_CLO:
						stack[A].forceAsClosure()->deleteMemberIndex(stack[B]);
						break;
					case VAR_CLASSDEF:
						stack[A].forceAsClosure()->deleteMemberIndex(stack[B]);
						for (auto &it : stack[A].forceAsClassDef()->children)
							it->refresh(stack[B]);
						break;
					case VAR_ARRAY:
						stack[A].forceAsArray()->deleteMemberIndex((short)stack[B].asInteger());
						break;
					default:
						throw Bagel_Except(W("语法错误，无法取变量"));
					}
					break;
				case Bagel_BC::BC_THROW:
					if (!btc->handleExcept(stack[A], returnlevel))
					{
						if (returnlevel)
							btc->expectInfo.forceSet(stack[A]);
						else
							btc->expectInfo = W("手动抛出的异常") + stack[A].saveString(false) + W("未被捕获");
						goto except;
					}
					else
					{
						goto loop;
					}
					break;
				case Bagel_BC::BC_THROWCONST:
					throw Bagel_Except(clo->bc->consts[A].forceAsBKEStr()->getConstStr());
					break;
				case Bagel_BC::BC_TYPE:
					stack[A].forceSet(stack[B].getTypeBKEString());
					break;
				case Bagel_BC::BC_INSTANCE:
					stack[A].forceSet(stack[B].instanceOf(stack[C]));
					break;
				case Bagel_BC::BC_INCONTEXTOF:
					switch (stack[B].getType())
					{
					case VAR_FUNC:
						stack[A].forceSet(new Bagel_Function(*stack[B].forceAsFunc(), stack[C].forceAsClosure() ? stack[C].forceAsClosure() : stack[B].forceAsFunc()->closure, stack[C]));
						break;
						//栈上没有prop
						//case VAR_PROP:
						//	stack[A].forceSet(new Bagel_Prop(*stack[B].forceAsProp(), stack[B].forceAsProp()->closure, stack[C]));
						//	break;
					default:
						throw Bagel_Except(W("incontextof只能针对function。"));
					}
					break;
				case Bagel_BC::BC_ELEMENT:
					{
						int32_t a = stack[C];
						int32_t b = stack[C + 1];
						stack[A].forceSet(stack[B].getMid(stack[C].isVoid() ? nullptr : &a, stack[C + 1].isVoid() ? nullptr : &b, stack[C + 2]));
					}
					break;
				case Bagel_BC::BC_MERGESELF:
					{
						if (clo->hasSelf && stack[A].tag->getType() == VAR_PROP)
						{
							Bagel_Prop* p = new Bagel_Prop(*stack[A].tag->forceAsProp(), clo->clo, clo->self);
							clo->self = p;
							stack[A].tag = &clo->self;
							clo->hasSelf = false;
						}
					}
					break;
					//typecheck
				case Bagel_BC::BC_CHECKTYPE:
					if (stack[A].getType() != B)
						throw Bagel_Except(W("此处类型要求为") + Bagel_Var::getTypeString(B).getConstStr());
					break;
				case Bagel_BC::BC_CHECKTHIS:
					if (stack[1].isVoid())
						throw Bagel_Except(W("this在此处无定义"));
					break;
				case Bagel_BC::BC_CREATEPOINTER:
					stack[A].forceSet(new Bagel_Pointer(stack[B], stack[C], false));
					break;
				case Bagel_BC::BC_GC:
					_GC.writeBarrierStack(clo->runstack);
					_GC.GC_little(GC_SPEED);
					break;
				case Bagel_BC::BC_DEBUG:
					if (debugMode)
					{
						btc->status = Bagel_ThreadContext::THREAD_SLEEP;
						clo->retpos = nullptr;
						pos--;
						return Bagel_Var();
					}
					else
					{
						pos = bcs->realcode;
					}
					break;
				#undef A
				#undef B
				#undef C
				#undef D
					//cache
				case Bagel_BC::BC_CACHE_NONE:
					hasCache = true;
					pos += 2;
					break;
				case Bagel_BC::BC_CACHE_GLOBAL_MEMBER:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (getCurrentGlobal()->varmap.deletetick == bcs->A && getCurrentGlobal()->getID() == bcs2->ptr)
						{
							//cache success
							stack[bcs3->C].tag = bcs->dst;
							btc->parentObj = nullptr;
							//member.forceSet(constpos[bcs3->A]);
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_DIC_MEMBER:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr &&
							_globalStructures.typeclass[VAR_DIC]->varmap.inserttick == bcs->A && stack[bcs3->A].forceAsDic()->varmap.deletetick == bcs2->A)
						{
							//cache success
							stack[bcs3->C].tag = bcs2->dst;
							btc->parentObj = nullptr;
							//member.forceSet(constpos[bcs3->B]);
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLASS_VAR:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClass()->varmap.deletetick == bcs->A)
						{
							//cache success
							stack[bcs3->C].tag = bcs2->dst;
							btc->parentObj = nullptr;
							//member.forceSet(constpos[bcs3->B]);
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLASS_FUNC:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClass()->varmap.inserttick == bcs->A
							&& stack[bcs3->A].forceAsClass()->defclass->varmap.deletetick == bcs2->A)
						{
							btc->parentObj = stack[bcs3->A].forceAsClass()->defclass;
							btc->member.forceSet(constpos[bcs3->B]);
							//cache success
							stack[bcs3->C].tag = bcs2->dst;
							if (stack[bcs3->C].tag->getType() == VAR_PROP)
							{
								clo->hasSelf = true;
								clo->self.forceSet(stack[bcs3->A]);
								clo->clo = stack[bcs3->A].forceAsClosure();
							}
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_TYPE_MEMBER:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getType() == bcs->A && _globalStructures.typeclass[bcs->A]->varmap.deletetick == bcs->C)
						{
							btc->parentObj = _globalStructures.typeclass[bcs->A];
							btc->member.forceSet(constpos[bcs3->B]);
							//cache success
							stack[bcs3->C].tag = bcs2->dst;
							Bagel_Class *cla = (Bagel_Class*)btc->parentObj;
							if (stack[bcs3->C].tag->getType() == VAR_FUNC)
							{
								throw Bagel_Except(W("不能给成员类型的函数赋值。"));
								//cla->tempvar.forceSet(new Bagel_Function(*bcs2->dst->forceAsFunc(), Bagel_Closure::global(), stack[bcs3->A]));
								//stack[bcs3->C].tag = &cla->tempvar;
							}
							else if (stack[bcs3->C].tag->getType() == VAR_PROP)
							{
								clo->hasSelf = true;
								clo->self.forceSet(stack[bcs3->A]);
								clo->clo = Bagel_Closure::global();
							}
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLO_MEMBER1:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClosure()->varmap.deletetick == bcs->A)
						{
							btc->parentObj = nullptr;
							//member.forceSet(constpos[bcs3->B]);
							//cache success
							stack[bcs3->C].tag = bcs2->dst;
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLO_MEMBER2:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClosure()->varmap.inserttick == bcs->A
							&& stack[bcs3->A].forceAsClosure()->parent->varmap.deletetick == bcs2->A)
						{
							btc->parentObj = nullptr;
							//member.forceSet(constpos[bcs3->B]);
							//cache success
							stack[bcs3->C].tag = bcs2->dst;
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLO_MEMBER_EXT:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClosure()->varmap.inserttick == bcs->A
							&& stack[bcs3->A].forceAsClosure()->parent->varmap.inserttick == bcs2->A)
						{
							btc->parentObj = nullptr;
							//member.forceSet(constpos[bcs3->B]);
							//cache success
							stack[bcs3->C].tag = &(stack[bcs3->A].forceAsClosure())->parent->parent->getMember(constpos[bcs3->B].forceAsBKEStr());
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_GLOBAL_INCLASS:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						auto bcs_ = bcs + 2;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClass()->varmap.inserttick == bcs->A
							&& stack[bcs3->A].forceAsClass()->defclass->varmap.inserttick == bcs2->A && bcs_->ptr == getCurrentGlobal()->getID())
						{
							btc->parentObj = nullptr;
							//parentObj = bcs->ptr;
							//member.forceSet(constpos[bcs3->B]);
							//cache success
							stack[bcs3->C].tag = bcs2->dst;
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_GLOBAL_INCLASSDEF:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						auto bcs_ = bcs + 2;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClass()->varmap.inserttick == bcs->A && bcs_->ptr == getCurrentGlobal()->getID())
						{
							btc->parentObj = nullptr;
							//parentObj = bcs->ptr;
							//member.forceSet(constpos[bcs3->B]);
							//cache success
							stack[bcs3->C].tag = bcs2->dst;
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLASS_VARINDEX:
					{
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getType() == VAR_CLASS && stack[bcs3->A].forceAsClass()->defclass->classid == bcs->A && stack[bcs3->A].forceAsClass()->varmap.deletetick == 1)
						{
							//cache success
							btc->parentObj = nullptr;
							stack[bcs3->C].tag = stack[bcs3->A].forceAsClass()->cache[bcs->B];
							//member.forceSet(constpos[bcs3->B]);
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_NEWCLASS_FUNC:
					{
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getType() == VAR_CLASS && stack[bcs3->A].forceAsClass()->defclass->classid == bcs->A && stack[bcs3->A].forceAsClass()->varmap.inserttick == 1)
						{
							//cache success
							btc->parentObj = stack[bcs3->A].forceAsClass()->defclass;
							btc->member.forceSet(constpos[bcs3->B]);
							stack[bcs3->C].tag = bcs->dst;
							pos += 3;
							if (stack[bcs3->C].tag->getType() == VAR_PROP)
							{
								clo->hasSelf = true;
								clo->self.forceSet(stack[bcs3->A]);
								clo->clo = stack[bcs3->A].forceAsClosure();
							}
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
					//value cache
				case Bagel_BC::BC_CACHE_GLOBAL_MEMBERV:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (getCurrentGlobal()->varmap.deletetick == bcs->A && getCurrentGlobal()->getID() == bcs2->ptr)
						{
							//cache success
							stack[bcs3->C].forceSet(*bcs->dst);
							pos += 3;
							if (stack[bcs3->C].getType() == VAR_PROP)
							{
								clo->retpos = &stack[bcs3->C];
								_GC.writeBarrierStack(clo->runstack);
								stack[bcs3->C].forceAsProp()->VMGet(btc);
								goto loop;
							}
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_DIC_MEMBERV:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr &&
							_globalStructures.typeclass[VAR_DIC]->varmap.inserttick == bcs->A && stack[bcs3->A].forceAsDic()->varmap.deletetick == bcs2->A)
						{
							//cache success
							stack[bcs3->C].forceSet(*bcs2->dst);
							pos += 3;
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLASS_VARV:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClass()->varmap.deletetick == bcs->A)
						{
							//cache success
							stack[bcs3->C].forceSet(*bcs2->dst);
							pos += 3;
							if (stack[bcs3->C].getType() == VAR_PROP)
							{
								clo->retpos = &stack[bcs3->C];
								_GC.writeBarrierStack(clo->runstack);
								stack[bcs3->C].forceAsProp()->VMGet(btc);
								goto loop;
							}
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLASS_FUNCV:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClass()->varmap.inserttick == bcs->A
							&& stack[bcs3->A].forceAsClass()->defclass->varmap.deletetick == bcs2->A)
						{
							//cache success
							stack[bcs3->C].forceSet(*bcs2->dst);
							pos += 3;
							if (stack[bcs3->C].getType() == VAR_FUNC)
							{
								stack[bcs3->C].forceSet(new Bagel_Function(*stack[bcs3->C].forceAsFunc(), stack[bcs3->A].forceAsClosure(), stack[bcs3->A]));
							}
							else if (stack[bcs3->C].getType() == VAR_PROP)
							{
								clo->retpos = &stack[bcs3->C];
								_GC.writeBarrierStack(clo->runstack);
								stack[bcs3->C].forceAsProp()->VMGetWithSelf(stack[bcs3->A], stack[bcs3->A].forceAsClosure(), btc);
								goto loop;
							}
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_TYPE_MEMBERV:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getType() == bcs->A && _globalStructures.typeclass[bcs->A]->varmap.deletetick == bcs->C)
						{
							//cache success
							//maybe bcs3->A==bcs3->C,so we must store stack[bcs3->A] first
							btc->runningclo->self.forceSet(stack[bcs3->A]);
							stack[bcs3->C].forceSet(*bcs2->dst);
							pos += 3;
							if (stack[bcs3->C].getType() == VAR_FUNC)
							{
								stack[bcs3->C].forceSet(new Bagel_Function(*stack[bcs3->C].forceAsFunc(), Bagel_Closure::global(), btc->runningclo->self));
							}
							else if (stack[bcs3->C].getType() == VAR_PROP)
							{
								clo->retpos = &stack[bcs3->C];
								_GC.writeBarrierStack(clo->runstack);
								stack[bcs3->C].forceAsProp()->VMGetWithSelf(btc->runningclo->self, Bagel_Closure::global(), btc);
								goto loop;
							}
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLO_MEMBER1V:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClosure()->varmap.deletetick == bcs->A)
						{
							//cache success
							stack[bcs3->C].forceSet(*bcs2->dst);
							pos += 3;
							if (stack[bcs3->C].getType() == VAR_PROP)
							{
								clo->retpos = &stack[bcs3->C];
								_GC.writeBarrierStack(clo->runstack);
								stack[bcs3->C].forceAsProp()->VMGet(btc);
								goto loop;
							}
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLO_MEMBER2V:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClosure()->varmap.inserttick == bcs->A
							&& stack[bcs3->A].forceAsClosure()->parent->varmap.deletetick == bcs2->A)
						{
							//cache success
							stack[bcs3->C].forceSet(*bcs2->dst);
							pos += 3;
							if (stack[bcs3->C].getType() == VAR_PROP)
							{
								clo->retpos = &stack[bcs3->C];
								_GC.writeBarrierStack(clo->runstack);
								stack[bcs3->C].forceAsProp()->VMGet(btc);
								goto loop;
							}
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLO_MEMBER_EXTV:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClosure()->varmap.inserttick == bcs->A
							&& stack[bcs3->A].forceAsClosure()->parent->varmap.inserttick == bcs2->A)
						{
							//cache success
							stack[bcs3->C].forceSet(stack[bcs3->A].forceAsClosure()->parent->parent->getMember(constpos[bcs3->B]));
							pos += 3;
							if (stack[bcs3->C].getType() == VAR_PROP)
							{
								clo->retpos = &stack[bcs3->C];
								_GC.writeBarrierStack(clo->runstack);
								stack[bcs3->C].forceAsProp()->VMGet(btc);
								goto loop;
							}
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_GLOBAL_INCLASSV:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						auto bcs_ = bcs + 2;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClass()->varmap.deletetick == bcs->A
							&& stack[bcs3->A].forceAsClass()->varmap.inserttick == bcs2->A && bcs_->ptr == getCurrentGlobal()->getID())
						{
							//cache success
							stack[bcs3->C].forceSet(*bcs2->dst);
							pos += 3;
							if (stack[bcs3->C].getType() == VAR_PROP)
							{
								clo->retpos = &stack[bcs3->C];
								_GC.writeBarrierStack(clo->runstack);
								stack[bcs3->C].forceAsProp()->VMGet(btc);
								goto loop;
							}
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_GLOBAL_INCLASSDEFV:
					{
						auto bcs2 = bcs + 1;
						auto bcs3 = bcs + 3;
						auto bcs_ = bcs + 2;
						if (stack[bcs3->A].getObjectID() == bcs->ptr && stack[bcs3->A].forceAsClass()->varmap.deletetick == bcs->A && bcs_->ptr == getCurrentGlobal()->getID())
						{
							//cache success
							stack[bcs3->C].forceSet(*bcs2->dst);
							pos += 3;
							if (stack[bcs3->C].getType() == VAR_PROP)
							{
								clo->retpos = &stack[bcs3->C];
								_GC.writeBarrierStack(clo->runstack);
								stack[bcs3->C].forceAsProp()->VMGet(btc);
								goto loop;
							}
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_CLASS_VARINDEXV:
					{
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getType() == VAR_CLASS && stack[bcs3->A].forceAsClass()->defclass->classid == bcs->A && stack[bcs3->A].forceAsClass()->varmap.deletetick == 1)
						{
							//cache success
							stack[bcs3->C].forceSet(*stack[bcs3->A].forceAsClass()->cache[bcs->B]);
							pos += 3;
							//好像class的var不可能是property
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;
				case Bagel_BC::BC_CACHE_NEWCLASS_FUNCV:
					{
						auto bcs3 = bcs + 3;
						if (stack[bcs3->A].getType() == VAR_CLASS && stack[bcs3->A].forceAsClass()->defclass->classid == bcs->A && stack[bcs3->A].forceAsClass()->varmap.inserttick == 1)
						{
							//cache success
							//maybe bcs3->A==bcs3->C,so we must store stack[bcs3->A] first
							btc->runningclo->self.forceSet(stack[bcs3->A]);
							btc->runningclo->clo = stack[bcs3->A].forceAsClosure();
							stack[bcs3->C].forceSet(*bcs->dst);
							pos += 3;
							if (stack[bcs3->C].getType() == VAR_FUNC)
							{
								stack[bcs3->C].forceSet(new Bagel_Function(*stack[bcs3->C].forceAsFunc(), btc->runningclo->clo, btc->runningclo->self));
							}
							else if (stack[bcs3->C].getType() == VAR_PROP)
							{
								clo->retpos = &stack[bcs3->C];
								_GC.writeBarrierStack(clo->runstack);
								stack[bcs3->C].forceAsProp()->VMGetWithSelf(btc->runningclo->self, btc->runningclo->clo, btc);
								goto loop;
							}
						}
						else
						{
							hasCache = true;
							pos += 2;
						}
					}
					break;

				default:
					throw Bagel_Except(W("非法的字节码") + bkpInt2Str(bcs->opcode));
				}
			}
		}
		catch (Bagel_ManualExcept &)
		{
			bool handle = btc->handleExcept(btc->expectInfo, returnlevel);
			if (!handle)
			{
				if (!returnlevel)
				{
					btc->expectInfo = W("手动抛出的异常") + btc->expectInfo.saveString(false) + W("未被捕获");
				}
				goto except;
			}
		}
		catch (Bagel_Except &e)
		{
			//带cache的指令可能因为异常没正确结束所以没重置hasCache
			btc->hasCache = false;
			bool handle = btc->handleExcept(e.getMsgWithoutPos(), returnlevel);
			if (!handle)
			{
				btc->expectInfo = e.getMsgWithoutPos();
				goto except;
			}
		}
	}
out:
	if (out)
		*out = INVALID_BAGEL_HANDLE;
	if (!returnlevel)
	{
		stop((Bagel_ThreadHandle)btc);
	}
	return retvar;
except:
	//整理下出错信息。然后throw
	if (!returnlevel)
	{
		if (out)
			*out = INVALID_BAGEL_HANDLE;
		auto v = btc->makeErrorInfo();
		stop((Bagel_ThreadHandle)btc);
		throw Bagel_Except(v);
	}
	throw Bagel_ManualExcept();
}

Bagel_ThreadContext * Bagel_VM::createNULLThreadHandle()
{
	if (!cache.empty() && cache.front()->status == Bagel_ThreadContext::THREAD_FINISH)
	{
		auto runclo = cache.front();
		cache.erase(cache.begin());
		cache.push_back(runclo);
		return runclo;
	}
	Bagel_ThreadContext* runclo = new Bagel_ThreadContext();
	cache.push_back(runclo);
	return runclo;
}

Bagel_ThreadContext* Bagel_VM::createThreadHandle(Bagel_ByteCode & code, Bagel_Closure * clo, const Bagel_Var &_this, Bagel_Stack* clostack, Bagel_StringHolder modulename)
{
	if (!clo)
		clo = getCurrentGlobal();
	Bagel_ThreadContext* runclo = createNULLThreadHandle();
	runclo->addCallstack(&code, clo, _this, clostack, modulename);
	return runclo;
}

Bagel_Var Bagel_VM::Run(Bagel_ByteCode *code, Bagel_Closure * clo, const Bagel_Var &_this, Bagel_Stack* clostack, Bagel_StringHolder modulename, Bagel_ThreadHandle * out, Bagel_ThreadHandle in)
{
	if (!clo)
		clo = getCurrentGlobal();
	Bagel_ThreadContext* runclo;
	if (in)
	{
		runclo = (Bagel_ThreadContext*)in;
		cache.push_back(runclo);
	}
	else
	{
		runclo = createNULLThreadHandle();
	}
	if (out)
		*out = (Bagel_ThreadHandle)runclo;
	runclo->addCallstack(code, clo, _this, clostack, modulename);
	return _debugRun(runclo, out, runclo->callstack.size() - 1);
}

Bagel_Var Bagel_VM::Run(const StringVal & exp, Bagel_Closure * clo, const Bagel_Var &_this, Bagel_Stack* clostack, Bagel_StringHolder modulename, Bagel_ThreadHandle * out, Bagel_ThreadHandle in)
{
	if (!clo)
		clo = getCurrentGlobal();
	if (out)
		*out = INVALID_BAGEL_HANDLE;
	auto ast = Bagel_Parser::getInstance()->parse(exp);
	Bagel_Compiler DC;
	Bagel_Handler<Bagel_ByteCode> code = new Bagel_ByteCode();
	DC.compile(ast, *code, exp);
	return Run(code, clo, _this, clostack, modulename, out, in);
}

Bagel_Var Bagel_VM::RunFile(const StringVal & file, Bagel_Closure * clo, const Bagel_Var & _this, Bagel_Stack * clostack, Bagel_StringHolder modulename, Bagel_ThreadHandle * out, Bagel_ThreadHandle in)
{
	if (!clo)
		clo = getCurrentGlobal();
	Bagel_Var val;
	StringVal res;
	int isStream;
	bool re = _globalStructures.readBinary(res, &isStream, file, 0);
	if (!re)
		throw Bagel_Except(W("打开文件失败"));
	if (isStream)
	{
		static Bagel_Serializer sel;
		val = sel.parse((const uint32_t *)&res[0], res.size() / 2);
		if (val.getType() == VAR_BYTECODE_P)
		{
			val = Run(val.forceAsObject<Bagel_ByteCode>(), clo, _this, clostack, modulename, out, in);
		}
	}
	else
	{
		val = Bagel_VM::getInstance()->Run(res, clo, _this, clostack, modulename, out, in);
	}
	return val;
}

Bagel_Handler<Bagel_Pointer> Bagel_VM::getVar(const StringVal & exp, Bagel_Closure * clo, const Bagel_Var &_this, Bagel_Stack* clostack, Bagel_StringHolder modulename, Bagel_Stack * stack, Bagel_ThreadHandle * out, Bagel_ThreadHandle in)
{
	if (!clo)
		clo = getCurrentGlobal();
	if (out)
		*out = INVALID_BAGEL_HANDLE;
	auto ast = Bagel_Parser::getInstance()->parse(exp);
	if (!ast || !ast->isVar())
		throw Bagel_Except(W("表达式不是一个合法变量"));
	Bagel_DebugCompiler DC;
	Bagel_Handler<Bagel_ByteCode> code = new Bagel_ByteCode();
	DC.compileAddr(ast, *code, exp);
	auto v = Run(code, clo, _this, clostack, modulename, out, in);
	return v.forceAsPointer();
}

Bagel_Var Bagel_VM::resume(Bagel_ThreadHandle handle, Bagel_ThreadHandle *out)
{
	if (!handle)
		handle = (Bagel_ThreadHandle)lastThread;
#if PARSER_DEBUG
	bool f = false;
	for (auto &clo : cache)
	{
		if ((Bagel_ThreadHandle)clo == handle)
		{
			f = true;
			break;
		}
	}
	assert(f);
#endif
	if (!handle)
	{
		return Bagel_Var();
	}
	return ((Bagel_ThreadContext*)handle)->Run(out);
}

Bagel_Var Bagel_VM::resumeLast(Bagel_ThreadHandle * out)
{
	if (!lastThread)
	{
		if (out)
			*out = INVALID_BAGEL_HANDLE;
		return Bagel_Var();
	}
	if (out)
		*out = (Bagel_ThreadHandle)lastThread;
	return lastThread->Run(out);
}

void Bagel_VM::stop(Bagel_ThreadHandle handle)
{
	if (!handle)
		return;
#if PARSER_DEBUG
	bool f = false;
	for (auto &clo : cache)
	{
		if ((Bagel_ThreadHandle)clo == handle)
		{
			f = true;
			break;
		}
	}
	assert(f);
#endif
	((Bagel_ThreadContext*)handle)->clearCallStack();
	((Bagel_ThreadContext*)handle)->status = Bagel_ThreadContext::THREAD_FINISH;
	cache.remove((Bagel_ThreadContext*)handle);
	cache.push_front((Bagel_ThreadContext*)handle);
}

void Bagel_VM::markChildren()
{
	for (auto &it : cache)
	{
		_GC.GC_Markself(it);
	}
	for (auto &it : strRefs)
	{
		_GC.GC_Markself(it.first);
	}
}

StringVal Bagel_ThreadContext::makeErrorInfo()
{
	//从后往前输出trace
	StringVal err;
	for (auto &it : callstack)
	{
		auto info = it->bc->debugInfo.get();
		StringVal err2;
		if (!info)
		{
			err2 = W("在") + it->name.getConstStr() + W("中:\n");
		}
		else
		{
			StringVal lineinfo;
			int32_t line;
			int32_t pos = (it->codepos - 1)->pos;
			info->getInfo(pos, lineinfo, line, pos);
			err2 = W("在") + it->name.getConstStr() + W("中第") + bkpInt2Str(line) + W("行第") + bkpInt2Str(pos) + W("处:\n");
			if (!lineinfo.empty())
			{
				err2 += lineinfo;
				if (err2.back() != '\n' && err2.back() != '\r')
					err2 += '\n';
				err2 += getLengthStr(lineinfo, pos);
				err2 += W("^\n");
			}
		}
		err = err2 + err;
	}
	StringVal err2 = expectInfo;
	err2 += W("\ntrace:\n");
	err = err2 + err;
	if (err.back() != '\n' && err.back() != '\r')
		err.push_back('\n');
	return err;
}

void Bagel_ThreadContext::markChildren()
{
	_GC.GC_Markself(expectInfo);
	for (auto &it : callstack)
	{
		_GC.GC_Markself(it->bc);
		_GC.GC_Markself(it->name.s);
		//否则self残留着前一次运行留下的变量
		if (it->hasSelf)
		{
			_GC.GC_Markself(it->self);
			_GC.GC_Markself(it->clo);
		}
		if (!it->runstack->isSpecialStack)
		{
			for (auto &it2 : it->default_stack)
			{
				_GC.GC_Markself(it2);
			}
		}
		else
		{
			_GC.GC_Markself(it->runstack);
		}
		//_GC.GC_Markself(it->runstack);
		_GC.GC_Markself(it->clostack);
	}
}

void Bagel_ThreadContext::addCallstack(Bagel_ByteCode * code, Bagel_Closure * clo, const Bagel_Var &_this, Bagel_Stack* clostack, Bagel_StringHolder modulename)
{
	if (callstack.size() > MAX_STACK)
		throw Bagel_Except(W("stack overflow"));
	//runningclo = new Bagel_RunClosure();
	runningclo = alloc2.allocate(1);
	callstack.push_back(runningclo);
	this->runningclo->hasSelf = false;
	this->runningclo->bc = code;
	runningclo->retpos = NULL;
	this->runningclo->codepos = &code->code.front();
	this->runningclo->name = modulename;
	//this->runningclo->runstack = new Bagel_Stack();
	if (!code->needSpecialStack)
	{
		this->runningclo->runstack = &this->runningclo->default_stack;
		this->runningclo->runstack->stacksize = code->stackDepth + 1 + code->localDepth;
		this->runningclo->runstack->stack = alloc.allocate(this->runningclo->runstack->stacksize);
		//清除原有变量，防止原有变量已被GC回收现在却又被标记。
		for (auto &it : this->runningclo->default_stack)
			it.setVoid();
	}
	else
	{
		this->runningclo->runstack = new Bagel_Stack();
		this->runningclo->runstack->stacksize = code->stackDepth + 1 + code->localDepth;
		this->runningclo->runstack->stack = new Bagel_Var[this->runningclo->runstack->stacksize];
	}
	this->runningclo->runstack->relativePos = &this->runningclo->runstack->stack[code->localDepth];
	this->runningclo->runstack->relativePos[CLOPOS].forceSet(clo);
	this->runningclo->runstack->relativePos[THISPOS].forceSet(_this);
	this->runningclo->runstack->isSpecialStack = code->needSpecialStack;
	this->runningclo->clostack = clostack;
	code->runTimes++;
}

void Bagel_ThreadContext::popCallStack()
{
	assert(runningclo);
	runningclo->hasSelf = false;
	if (!runningclo->runstack->isSpecialStack)
		alloc.deallocate(runningclo->runstack->stacksize);
	alloc2.deallocate(1);
	callstack.pop_back();
	runningclo = callstack.empty() ? NULL : callstack.back();
}

bool Bagel_ThreadContext::handleExcept(const Bagel_Var &info, int returnlevel)
{
	if (trystack.empty())
		return false;
	auto &&t = trystack.back();
	if (t.callDepth <= returnlevel)
		return false;
	while (t.callDepth < callstack.size())
	{
		popCallStack();
	}
	runningclo->runstack->relativePos[t.exceptvar].forceSet(info);
	runningclo->codepos = &runningclo->bc->code[t.codepos];
	trystack.pop_back();
	return true;
}

Bagel_Var Bagel_ThreadContext::Run(Bagel_ThreadHandle *out)
{
	return Bagel_VM::getInstance()->_debugRun(this, out);
}

int Bagel_ThreadContext::getCurLine() const
{
	if (!runningclo || !runningclo->bc->debugInfo)
		return 0;
	int32_t line, linepos;
	runningclo->bc->debugInfo->getInfo((int32_t)runningclo->codepos->pos, line, linepos);
	return line - 1;
}

bool Bagel_ThreadContext::fetchVarAtDebugPos(Bagel_StringHolder varname, Bagel_Var & outvar)
{
	if (!runningclo)
	{
		outvar.forceSet(Bagel_VM::getInstance()->getCurrentGlobal()->getMember(varname));
		return true;
	}
	if (runningclo->bc->varPosInfo)
	{
		auto res = runningclo->bc->varPosInfo->queryVar(varname, fetchLastCommand()->pos);
		if (res.first == 0)
		{
			auto clo = runningclo->runstack->relativePos[CLOPOS].forceAsClosure();
			outvar.forceSet(clo->getMember(varname));
		}
		else if (res.first & CLOSTACK_FLAG)
		{
			//最高位设回原先的值
			if (!(res.first & STACKSIGN_FLAG))
				res.first &= ~CLOSTACK_FLAG;
			if (res.second)
				outvar.forceSet(*runningclo->clostack->relativePos[res.first].tag);
			else
				outvar.forceSet(runningclo->clostack->relativePos[res.first]);
		}
		else
		{
			//最高位设回原先的值
			if (!(res.first & STACKSIGN_FLAG))
				res.first &= ~CLOSTACK_FLAG;
			if (res.second)
				outvar.forceSet(*runningclo->runstack->relativePos[res.first].tag);
			else
				outvar.forceSet(runningclo->runstack->relativePos[res.first]);
		}
		return true;
	}
	else
	{
		outvar.forceSet(Bagel_VM::getInstance()->getCurrentGlobal()->getMember(varname));
		return false;
	}
}

Bagel_Stack::~Bagel_Stack()
{
	if (isSpecialStack)
		delete[] stack;
}

void Bagel_BreakPoints::initMap(Bagel_ByteCode * bc)
{
	if (!bc->debugInfo || inited[bc])
		return;
	auto &&code = bc->code;
	auto &&info = bc->debugInfo->linestartpos;
	auto offset = bc->debugInfo->startline;
	auto it = 0;
	auto itc = 0;
	if (codemap.size() < info.size() + offset)
		codemap.resize(info.size() + offset);
	int lb = info[0];
	int ub = info.size() > 1 ? info[1] : INT32_MAX;
	while (it < info.size() && itc < code.size())
	{
		if (code[itc].pos == MINUS_POS)
		{
			++itc;
		}
		else if (code[itc].pos >= lb && code[itc].pos < ub)
		{
			codemap[it + offset].emplace_back(bc, itc);
			++itc;
			while (itc < code.size() && code[itc].pos >= lb && code[itc].pos < ub)
				++itc;
			++it;
			lb = ub;
			ub = info.size() > it + 1 ? info[it + 1] : INT32_MAX;
		}
		else
		{
			auto info_it = upper_bound(info.begin(), info.end(), code[itc].pos);
			if (info_it == info.begin())
			{
				//按理info.begin应该为0，不太可能出这种情况，故忽略
				++itc;
				continue;
			}
			it = info_it - info.begin() - 1;
			lb = info[it];
			ub = info.size() > it + 1 ? info[it + 1] : INT32_MAX;
		}
	}

	inited[bc] = true;
}

void Bagel_BreakPoints::setBreakPointAtLine(int line)
{
	if (line < 0)
		return;
	while (line < codemap.size() && codemap[line].empty())
		++line;
	if (line >= codemap.size())
		return;
	for (auto &&idx : codemap[line])
	{
		int groupsize = 1;
		auto code = &idx.first->code[idx.second];
		if ((code->opcode > Bagel_BC::BC_PTR_SET && code->opcode <= Bagel_BC::BC_PTR_PREDEC) || (code->opcode > Bagel_BC::BC_PTR_SETC && code->opcode <= Bagel_BC::BC_PTR_SETSETC))
		{
			groupsize = 4;
		}
		else if (code->opcode >= Bagel_BC::BC_CACHE_NONE)
		{
			groupsize = 3;
		}
		auto &&cmd = rawcmd[idx.first][idx.second];
		cmd.resize(groupsize + 1);
		memcpy(&cmd[0], code, groupsize * sizeof(code[0]));
		cmd[groupsize].opcode = Bagel_BC::BC_JUMP;
		cmd[groupsize].pos = code->pos;
		cmd[groupsize].A = idx.second + groupsize;
		cmd[groupsize].B = 0;
		cmd[groupsize].C = 0;
		code->opcode = Bagel_BC::BC_DEBUG;
		code->A = idx.second;
		code->realcode = cmd.data();

		Bagel_VM::getInstance()->bpcode[idx.first] = this;
	}
}

bool Bagel_BreakPoints::eraseBreakPointAtLine(int line)
{
	if (line < 0)
		return false;
	while (line < codemap.size() && codemap[line].empty())
		++line;
	if (line >= codemap.size())
		return false;
	bool ret = false;
	for (auto &&idx : codemap[line])
	{
		if (!rawcmd.count(idx.first) || !rawcmd[idx.first].count(idx.second))
			continue;
		auto &&cmd = rawcmd[idx.first][idx.second];
		auto code = &idx.first->code[idx.second];
		if (code->opcode != Bagel_BC::BC_DEBUG)
		{
			//这是一个曾经的断点，已经作废
			rawcmd[idx.first].erase(idx.second);
			continue;
		}
		memcpy(code, &cmd[0], (cmd.size() - 1) * sizeof(code[0]));
		rawcmd[idx.first].erase(idx.second);
		ret = true;
	}
	return ret;
}
