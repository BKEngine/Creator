#include "Bagel_DCompiler.h"

#define PUSHCODE(a,b,c,d,e) code.code.emplace_back(a, b, c, d, e)
#define PUSHCODE2(a,b,c,d) code.code.emplace_back(a, b, c, d)

#define NEW_CLO(th, b, c) \
clos.emplace_back(); \
newclo = &clos.back(); \
newclo->withpos = curclo->withpos; \
newclo->clopos = th; \
newclo->breakcode = b; \
newclo->continuecode = c; \
curclo = newclo;

#define END_CLO \
clos.pop_back(); \
curclo = &clos.back(); 

void Bagel_DebugCompiler::_compile(int dest, Bagel_AST * subtree, Bagel_ByteCode & code)
{
	int a, b, c, d;
	CompileClo *newclo;
	if (code.stackDepth < dest + 10)
		code.stackDepth = dest + 10;
	if (!subtree)
	{
		PUSHCODE(Bagel_BC::BC_LOADVOID, -1, dest, 0, 0);
		return;
	}
	int pos = subtree->Node.pos;
	switch (subtree->Node.opcode)
	{
	case OP_END:
	case OP_END + OP_COUNT:
		break;
	case OP_IF:
		_compile(dest, subtree->childs[1], code);
		PUSHCODE(Bagel_BC::BC_JUMPFALSE, pos, 0, dest, 0);
		a = code.code.size();
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(Bagel_BC::BC_JUMP, -1, 0, 0, 0);
		b = code.code.size();
		code.code[a - 1].A = b;
		PUSHCODE(Bagel_BC::BC_LOADVOID, -1, dest, 0, 0);
		code.code[b - 1].A = code.code.size();
		break;
	case OP_IF + OP_COUNT:
	case OP_CHOOSE:
		//语法树生成那里保证childs[1]和childs[2]非空的话一定新建一个子闭包
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(Bagel_BC::BC_JUMPFALSE, pos, 0, dest, 0);
		a = code.code.size();
		_compile(dest, subtree->childs[1], code);
		PUSHCODE(Bagel_BC::BC_JUMP, -1, 0, 0, 0);
		b = code.code.size();
		code.code[a - 1].A = b;
		_compile(dest, subtree->childs[2], code);
		code.code[b - 1].A = code.code.size();
		break;
	case OP_QUICKFOR + OP_COUNT:
		{
			vector<int> breakcode;
			vector<int> continuecode;
			_compile(dest, subtree->childs[1], code);
			_compile(dest + 1, subtree->childs[2], code);
			if (subtree->childs[3])
			{
				_compile(dest + 2, subtree->childs[3], code);
			}
			else
			{
				PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest + 2, 0, 0);
			}
			//先运行一遍
			//再编译成循环
			//LOOPSTART -> loop1 -> LOOPSTEP2 -> loop n -> LOOPSTEP
			PUSHCODE(Bagel_BC::BC_LOOPSTART, pos, dest, 0, 0);
			a = code.code.size();
			PUSHCODE(Bagel_BC::BC_PUSHCLO, subtree->childs[4]->Node.pos, dest + 4, curclo->clopos, 0);
			NEW_CLO(dest + 4, &breakcode, &continuecode);
			code.consts.emplace_back(subtree->childs[0]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 5, code.consts.size() - 1, 0);
			PUSHCODE(Bagel_BC::BC_SETMEMBER2, subtree->childs[0]->Node.pos, dest + 4, dest + 5, dest);
			_compile(dest + 6, subtree->childs[4], code);
			PUSHCODE(Bagel_BC::BC_LOOPSTEP2, pos, dest, 0, 0);
			END_CLO;

			c = code.code.size();	//loop start pos
			PUSHCODE(Bagel_BC::BC_PUSHCLO, subtree->childs[4]->Node.pos, dest + 4, curclo->clopos, 0);
			NEW_CLO(dest + 4, &breakcode, &continuecode);
			code.consts.emplace_back(subtree->childs[0]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 5, code.consts.size() - 1, 0);
			PUSHCODE(Bagel_BC::BC_SETMEMBER2, subtree->childs[0]->Node.pos, dest + 4, dest + 5, dest + 3);
			_compile(dest + 6, subtree->childs[4], code);
			b = code.code.size();
			for (auto &i : continuecode)
			{
				code.code[i].A = b;
			}
			PUSHCODE(Bagel_BC::BC_LOOPSTEP, pos, dest, c, 0);
			b = code.code.size();
			code.code[a - 1].B = b;
			code.code[c - 1].B = b;
			for (auto &i : breakcode)
			{
				code.code[i].A = b;
			}
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
			END_CLO;
		}
		break;
	case OP_FOR + OP_COUNT:
		{
			vector<int> breakcode;
			vector<int> continuecode;
			PUSHCODE(Bagel_BC::BC_PUSHCLO, pos, dest, curclo->clopos, 0);
			auto rawb = curclo->breakcode;
			auto rawc = curclo->continuecode;
			NEW_CLO(dest, rawb, rawc);
			_compile(dest + 1, subtree->childs[0], code);
			a = code.code.size();
			if (subtree->childs[1])
			{
				_compile(dest + 1, subtree->childs[1], code);
				b = code.code.size();
				PUSHCODE(Bagel_BC::BC_JUMPFALSE, subtree->childs[1]->Node.pos, 0, dest + 1, 0);
			}
			newclo->breakcode = &breakcode;
			newclo->continuecode = &continuecode;
			_compile(dest + 1, subtree->childs[3], code);
			for (auto &i : continuecode)
			{
				code.code[i].A = code.code.size();
			}
			newclo->breakcode = rawb;
			newclo->continuecode = rawc;
			_compile(dest + 1, subtree->childs[2], code);
			PUSHCODE(Bagel_BC::BC_GC, -1, 0, 0, 0);
			PUSHCODE(Bagel_BC::BC_JUMP, -1, a, 0, 0);
			for (auto &i : breakcode)
			{
				code.code[i].A = code.code.size();
			}
			if (subtree->childs[1])
			{
				code.code[b].A = code.code.size();
			}
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
			END_CLO;
		}
		break;
	case OP_WHILE + OP_COUNT:
		{
			vector<int> breakcode;
			vector<int> continuecode;
			a = code.code.size();
			_compile(dest, subtree->childs[0], code);
			b = code.code.size();
			PUSHCODE(Bagel_BC::BC_JUMPFALSE, pos, 0, dest, 0);
			PUSHCODE(Bagel_BC::BC_PUSHCLO, pos, dest, curclo->clopos, 0);
			NEW_CLO(dest, &breakcode, &continuecode);
			_compile(dest + 1, subtree->childs[1], code);
			for (auto &i : continuecode)
			{
				code.code[i].A = code.code.size();
			}
			PUSHCODE(Bagel_BC::BC_GC, -1, 0, 0, 0);
			PUSHCODE(Bagel_BC::BC_JUMP, pos, a, 0, 0);
			for (auto &i : breakcode)
			{
				code.code[i].A = code.code.size();
			}
			code.code[b].A = code.code.size();
			END_CLO;
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		}
		break;
	case OP_DO + OP_COUNT:
		{
			vector<int> breakcode;
			vector<int> continuecode;
			PUSHCODE(Bagel_BC::BC_PUSHCLO, pos, dest, curclo->clopos, 0);
			NEW_CLO(dest, &breakcode, &continuecode);
			a = code.code.size();
			_compile(dest + 1, subtree->childs[0], code);
			for (auto &i : continuecode)
			{
				code.code[i].A = code.code.size();
			}
			PUSHCODE(Bagel_BC::BC_GC, -1, 0, 0, 0);
			END_CLO;
			_compile(dest + 1, subtree->childs[1], code);
			PUSHCODE(Bagel_BC::BC_JUMPTRUE, pos, a, dest + 1, 0);
			for (auto &i : breakcode)
			{
				code.code[i].A = code.code.size();
			}
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		}
		break;
	case OP_FOREACH + OP_COUNT:
		{
			vector<int> breakcode;
			vector<int> continuecode;
			_compile(dest + 3, subtree->childs[2], code);
			PUSHCODE(Bagel_BC::BC_PUSHCLO, pos, dest, curclo->clopos, 0);
			NEW_CLO(dest, &breakcode, &continuecode);
			if (subtree->childs[0])
			{
				code.consts.emplace_back(subtree->childs[0]->Node.var);
				PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 1, code.consts.size() - 1, 0);
				a = dest + 1;
			}
			else
			{
				a = -1;
			}
			if (subtree->childs[1])
			{
				code.consts.emplace_back(subtree->childs[1]->Node.var);
				PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 2, code.consts.size() - 1, 0);
				b = dest + 2;
			}
			else
			{
				b = -1;
			}
			PUSHCODE(Bagel_BC::BC_FOREACHSTART, pos, dest + 3, 0, 0);
			c = code.code.size();
			if (a >= 0)
				PUSHCODE(Bagel_BC::BC_SETMEMBER2, pos, dest, a, dest + 6);
			if (b >= 0)
				PUSHCODE(Bagel_BC::BC_SETMEMBER2, pos, dest, b, dest + 7);
			_compile(dest + 8, subtree->childs[3], code);
			for (auto &i : continuecode)
			{
				code.code[i].A = code.code.size();
			}
			code.code[c - 1].B = code.code.size();
			PUSHCODE(Bagel_BC::BC_NEXT, -1, dest + 3, c, 0);
			code.code[c - 1].C = code.code.size();
			for (auto &i : breakcode)
			{
				code.code[i].A = code.code.size();
			}
			END_CLO;
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		}
		break;
	case OP_CONSTVAR + OP_COUNT:
		if (subtree->Node.var.getType() == VAR_NUM)
		{
			double d2 = subtree->Node.var;
			PUSHCODE2(Bagel_BC::BC_LOADNUM, pos, dest, d2);
		}
		else
		{
			code.consts.emplace_back(subtree->Node.var);
			if (subtree->Node.var.getType() == VAR_DIC || subtree->Node.var.getType() == VAR_ARRAY)
			{
				PUSHCODE(Bagel_BC::BC_COPYCONST, pos, dest, code.consts.size() - 1, 0);
			}
			else
			{
				PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest, code.consts.size() - 1, 0);
			}
		}
		break;
	case OP_LITERAL + OP_COUNT:
		code.consts.emplace_back(subtree->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCLOMEMBER_VALUE, pos, curclo->clopos, code.consts.size() - 1, dest);
		break;
	case OP_FUNCTION + OP_COUNT:
	{
		Bagel_Function *func = subtree->Node.var.asFunc();
		code.consts.emplace_back(func);
		PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest, code.consts.size() - 1, 0);
		PUSHCODE(Bagel_BC::BC_SETCLOSURE, pos, dest, inclass ? inclass : curclo->clopos, 0);
		if (!func->name.empty())
		{
			code.consts.emplace_back(func->name);
			PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 1, code.consts.size() - 1, 0);
			PUSHCODE(Bagel_BC::BC_ADDFUNCTION, pos, inclass, dest + 1, dest);
		}
		//compile function
		{
			Bagel_DebugCompiler tmp;
			func->func->bytecode = new Bagel_ByteCode();
			tmp.compile(func->func->code, *func->func->bytecode);
			func->func->bytecode->debugInfo = func->func->info;
			//func->func->code = nullptr;
		}
	}
	break;
	case OP_CLASS + OP_COUNT:
		{
			code.consts.emplace_back(subtree->childs[0]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->Node.pos, dest, code.consts.size() - 1, 0);
			a = 1;
			b = subtree->childs.size();
			while (a < b && subtree->childs[a]->Node.opcode == OP_LITERAL)
			{
				code.consts.emplace_back(subtree->childs[a]->Node.var);
				PUSHCODE(Bagel_BC::BC_LOADGLOBALMEMBER_VALUE, subtree->childs[a]->Node.pos, code.consts.size() - 1, 0, dest + a);
				PUSHCODE(Bagel_BC::BC_CHECKTYPE, subtree->childs[a]->Node.pos, dest + a, VAR_CLASSDEF, 0);
				a++;
			}
			PUSHCODE(Bagel_BC::BC_MAKECLASS, pos, dest, dest, a);
			auto rawclass = inclass;
			inclass = dest;
			while (a < b)
			{
				auto *subtr = subtree->childs[a];
				_compile(dest + 1, subtr, code);
				a++;
			}
			PUSHCODE(Bagel_BC::BC_FINISHCLASS, pos, dest, 0, 0);
			inclass = rawclass;
			break;
		}
	case OP_PROPGET + OP_COUNT:
		code.consts.emplace_back(subtree->childs[0]->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest, code.consts.size() - 1, 0);
		{
			Bagel_FunctionCode *func = subtree->Node.var.forceAsObject<Bagel_FunctionCode>();
			Bagel_DebugCompiler tmp;
			func->bytecode = new Bagel_ByteCode();
			tmp.compile(func->code, *func->bytecode);
			func->bytecode->debugInfo = func->info;
			code.consts.emplace_back(func);
			PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 1, code.consts.size() - 1, 0);
		}
		PUSHCODE(Bagel_BC::BC_ADDPROPGET, pos, inclass ? inclass : curclo->clopos, dest, dest + 1);
		break;
	case OP_PROPSET + OP_COUNT:
		code.consts.emplace_back(subtree->childs[0]->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest, code.consts.size() - 1, 0);
		{
			Bagel_FunctionCode *func = subtree->Node.var.forceAsObject<Bagel_FunctionCode>();
			Bagel_DebugCompiler tmp;
			func->bytecode = new Bagel_ByteCode();
			tmp.compile(func->code, *func->bytecode);
			func->bytecode->debugInfo = func->info;
			code.consts.emplace_back(func);
			PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 1, code.consts.size() - 1, 0);
		}
		PUSHCODE(Bagel_BC::BC_ADDPROPSET, pos, inclass ? inclass : curclo->clopos, dest, dest + 1);
		break;
	case OP_PROPERTY + OP_COUNT:
		code.consts.emplace_back(subtree->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest, code.consts.size() - 1, 0);
		{
			Bagel_FunctionCode *func = subtree->childs[0]->Node.var.forceAsObject<Bagel_FunctionCode>();
			Bagel_DebugCompiler tmp;
			func->bytecode = new Bagel_ByteCode();
			tmp.compile(func->code, *func->bytecode);
			code.consts.emplace_back(func);
			PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 1, code.consts.size() - 1, 0);
		}
		code.consts.emplace_back(subtree->childs[1]->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 2, code.consts.size() - 1, 0);
		{
			Bagel_FunctionCode *func = subtree->childs[1]->Node.var.forceAsObject<Bagel_FunctionCode>();
			Bagel_DebugCompiler tmp;
			func->bytecode = new Bagel_ByteCode();
			tmp.compile(func->code, *func->bytecode);
			code.consts.emplace_back(func);
			PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 2, code.consts.size() - 1, 0);
		}
		PUSHCODE(Bagel_BC::BC_ADDPROPERTY, pos, inclass, dest, 0);
		break;
	case OP_ADD:
	case OP_SUB:
	case OP_MUL:
	case OP_DIV:
	case OP_INTDIV:
	case OP_MOD:
	case OP_POW:
		_compile(dest, subtree->childs[0], code);
		_compile(dest + 1, subtree->childs[1], code);
		PUSHCODE(subtree->Node.opcode - OP_ADD + Bagel_BC::BC_ADD, pos, dest, dest, dest + 1);
		break;
	case OP_ADD + OP_COUNT:
	case OP_SUB + OP_COUNT:
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(subtree->Node.opcode - OP_ADD - OP_COUNT + Bagel_BC::BC_TONUMBER, pos, dest, dest, 0);
		break;
	case OP_EQUAL:
	case OP_NEQUAL:
	case OP_EEQUAL:
	case OP_NNEQUAL:
	case OP_LARGER:
	case OP_SMALLER:
	case OP_LE:
	case OP_SE:
		_compile(dest, subtree->childs[0], code);
		_compile(dest + 1, subtree->childs[1], code);
		PUSHCODE(subtree->Node.opcode - OP_EQUAL + Bagel_BC::BC_EQUAL, pos, dest, dest, dest + 1);
		break;
	case OP_FASTAND:
		_compile(dest, subtree->childs[0], code);
		a = code.code.size();
		PUSHCODE(Bagel_BC::BC_JUMPVOID, pos, 0, dest, 0);
		_compile(dest, subtree->childs[1], code);
		code.code[a].A = code.code.size();
		break;
	case OP_AND:
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(Bagel_BC::BC_TOBOOL, pos, dest, dest, 0);
		a = code.code.size();
		PUSHCODE(Bagel_BC::BC_JUMPFALSE, pos, 0, dest, 0);
		_compile(dest, subtree->childs[1], code);
		PUSHCODE(Bagel_BC::BC_TOBOOL, pos, dest, dest, 0);
		code.code[a].A = code.code.size();
		break;
	case OP_FASTOR:
		_compile(dest, subtree->childs[0], code);
		a = code.code.size();
		PUSHCODE(Bagel_BC::BC_JUMPNOTVOID, pos, 0, dest, 0);
		_compile(dest, subtree->childs[1], code);
		code.code[a].A = code.code.size();
		break;
	case OP_OR:
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(Bagel_BC::BC_TOBOOL, pos, dest, dest, 0);
		a = code.code.size();
		PUSHCODE(Bagel_BC::BC_JUMPTRUE, pos, 0, dest, 0);
		_compile(dest, subtree->childs[1], code);
		PUSHCODE(Bagel_BC::BC_TOBOOL, pos, dest, dest, 0);
		code.code[a].A = code.code.size();
		break;
	case OP_NOT + OP_COUNT:
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(Bagel_BC::BC_NOT, pos, dest, dest, 0);
		break;
	case OP_SET:
	case OP_SETADD:
	case OP_SETSUB:
	case OP_SETMUL:
	case OP_SETDIV:
	case OP_SETINTDIV:
	case OP_SETMOD:
	case OP_SETPOW:
	case OP_SETSET:		//|=
		_compile(dest, subtree->childs[1], code);
		c = dest;
		goto common;
	case OP_SELFINC:
	case OP_SELFDEC:
	case OP_SELFINC + OP_COUNT:
	case OP_SELFDEC + OP_COUNT:
		c = 0;
		common:
		a = subtree->Node.opcode - OP_SET;
		if (a > OP_COUNT)
			a -= OP_COUNT - 2;
		//bcs的C位置
		d = 0;
		if (a == OP_SELFINC - OP_SET || a == OP_SELFDEC - OP_SET)
		{
			d = dest;
		}
		switch (subtree->childs[0]->Node.opcode)
		{
		case OP_LITERAL + OP_COUNT:
			code.consts.emplace_back(subtree->childs[0]->Node.var);
			PUSHCODE(Bagel_BC::BC_GETCLOMEMBER_PTR, pos, curclo->clopos, code.consts.size() - 1, dest + 1);
			PUSHCODE(Bagel_BC::BC_PTR_SET + a, pos, dest + 1, c, d);
			if (a)
			{
				//为了实现yield得把property的计算拆开
				PUSHCODE(Bagel_BC::BC_LOADPOINTER2, pos, dest + 2, dest + 1, 0);
				PUSHCODE(Bagel_BC::BC_MOV + a, pos, dest + 2, c, d);
				PUSHCODE(Bagel_BC::BC_SETPOINTER2, pos, dest + 1, dest + 2, 0);
			}
			if(!d)
			{
				PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, dest + 1, 0);
			}
			break;
		case OP_DOT + OP_COUNT:
			code.consts.emplace_back(subtree->childs[0]->childs[0]->Node.var);
			if (curclo->withpos >= 0)
			{
				PUSHCODE(Bagel_BC::BC_GETMEMBERSTRIDX_PTR, pos, curclo->withpos, code.consts.size() - 1, dest + 1);
			}
			else
			{
				PUSHCODE(Bagel_BC::BC_GETGLOBALMEMBER_PTR, pos, code.consts.size() - 1, 0, dest + 1);
			}
			PUSHCODE(Bagel_BC::BC_PTR_SET + a, pos, dest + 1, c, d);
			if (a)
			{
				//为了实现yield得把property的计算拆开
				PUSHCODE(Bagel_BC::BC_LOADPOINTER2, pos, dest + 2, dest + 1, 0);
				PUSHCODE(Bagel_BC::BC_MOV + a, pos, dest + 2, c, d);
				PUSHCODE(Bagel_BC::BC_SETPOINTER2, pos, dest + 1, dest + 2, 0);
			}
			if(!d)
			{
				PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, dest + 1, 0);
			}
			break;
		case OP_DOT:
			code.consts.emplace_back(subtree->childs[0]->childs[1]->Node.var);
			b = code.consts.size() - 1;
			if (subtree->childs[0]->childs[0]->Node.opcode == OP_SUPER + OP_COUNT)
			{
				PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->childs[1]->Node.pos, dest + 1, b, 0);
				PUSHCODE(Bagel_BC::BC_LOADSUPERMEMBER_PTR, pos, 0, dest + 1, dest + 2);
			}
			else
			{
				_compile(dest + 1, subtree->childs[0]->childs[0], code);
				PUSHCODE(Bagel_BC::BC_GETMEMBERSTRIDX_PTR, pos, dest + 1, b, dest + 2);
			}
			PUSHCODE(Bagel_BC::BC_PTR_SET + a, pos, dest + 2, c, d);
			if (a)
			{
				//为了实现yield得把property的计算拆开
				PUSHCODE(Bagel_BC::BC_LOADPOINTER2, pos, dest + 3, dest + 2, 0);
				PUSHCODE(Bagel_BC::BC_MOV + a, pos, dest + 3, c, d);
				PUSHCODE(Bagel_BC::BC_SETPOINTER2, pos, dest + 2, dest + 3, 0);
			}
			if(!d)
			{
				PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, dest + 2, 0);
			}
			break;
		case OP_ARRAY:
			_compile(dest + 1, subtree->childs[0]->childs[0], code);
			_compile(dest + 2, subtree->childs[0]->childs[1], code);
			PUSHCODE(Bagel_BC::BC_GETMEMBER_PTR, pos, dest + 1, dest + 2, dest + 3);
			PUSHCODE(Bagel_BC::BC_PTR_SET + a, pos, dest + 3, c, d);
			if (a)
			{
				//为了实现yield得把property的计算拆开
				PUSHCODE(Bagel_BC::BC_LOADPOINTER2, pos, dest + 4, dest + 3, 0);
				PUSHCODE(Bagel_BC::BC_MOV + a, pos, dest + 4, c, d);
				PUSHCODE(Bagel_BC::BC_SETPOINTER2, pos, dest + 3, dest + 4, 0);
			}
			if(!d)
			{
				PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, dest + 3, 0);
			}
			break;
		}
		break;
	case OP_BRACKET:
		_compile(dest, subtree->childs[0], code);
		b = 0;	//has ...?
		for (a = 1; a < subtree->childs.size(); a++)
		{
			if (subtree->childs[a]->Node.opcode == OP_EXTRACTARGS)
			{
				b = 1;
				break;
			}
		}
		//PUSHCODE(Bagel_BC::BC_PRECALL, pos, dest, 0, 0);
		if(!b)
		{
			a = 1;
			while (a < subtree->childs.size())
			{
				_compile(dest + a, subtree->childs[a], code);
				a++;
			}
			//PUSHCODE(Bagel_BC::BC_MAKEARRAY, pos, dest + 1, dest + 1, a - 1);
			PUSHCODE(Bagel_BC::BC_CALL, pos, dest, dest + 1, a - 1);
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_NULLVECTOR, pos, dest + 1, 0, 0);
			a = 1;
			c = 2;
			while (a < subtree->childs.size())
			{
				if (subtree->childs[a]->Node.opcode == OP_EXTRACTARGS)
				{
					if (c > 2)
					{
						PUSHCODE(Bagel_BC::BC_VECTORPUSHBACK, pos, dest + 1, dest + 2, c - 2);
					}
					c = 2;
					_compile(dest + 2, subtree->childs[a]->childs[0], code);
					PUSHCODE(Bagel_BC::BC_VECTORCONCAT, pos, dest + 1, dest + 2, 0);
				}
				else
				{
					_compile(dest + c, subtree->childs[a], code);
					c++;
				}
				a++;
			}
			PUSHCODE(Bagel_BC::BC_CALLVECTOR, pos, dest, dest + 1, 0);
		}
		break;
	case OP_ARRAY:
		if (subtree->childs[0]->Node.opcode == OP_SUPER + OP_COUNT)
		{
			_compile(dest, subtree->childs[1], code);
			PUSHCODE(Bagel_BC::BC_LOADSUPERMEMBER_VALUE, pos, 0, dest, dest);
		}
		else
		{
			_compile(dest, subtree->childs[0], code);
			if (subtree->childs[1])
				_compile(dest + 1, subtree->childs[1], code);
			else
				PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest + 1, 0, 0);
			if (subtree->childs.size() == 2)
			{
				PUSHCODE(Bagel_BC::BC_LOADMEMBER_VALUE, pos, dest, dest + 1, dest);
				break;
			}
			if (subtree->childs.size() > 2 && subtree->childs[2])
				_compile(dest + 2, subtree->childs[2], code);
			else
				PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest + 2, 0, 0);
			if (subtree->childs.size() > 3 && subtree->childs[3])
				_compile(dest + 3, subtree->childs[3], code);
			else
				PUSHCODE2(Bagel_BC::BC_LOADNUM, pos, dest + 3, 1.0);
			PUSHCODE(Bagel_BC::BC_ELEMENT, pos, dest, dest, dest + 1);
		}
		break;
	case OP_ARRAY + OP_COUNT:
		a = -1;
		b = subtree->childs.size();
		while (++a < b)
		{
			_compile(dest + a, subtree->childs[a], code);
		}
		PUSHCODE(Bagel_BC::BC_MAKEARRAY, pos, dest, dest, a);
		break;
	case OP_DIC + OP_COUNT:
		a = 0;
		b = subtree->childs.size();
		while (a < b)
		{
			_compile(dest + a, subtree->childs[a], code);
			_compile(dest + a + 1, subtree->childs[a + 1], code);
			a += 2;
		}
		PUSHCODE(Bagel_BC::BC_MAKEDIC, pos, dest, dest, a);
		break;
	case OP_NULLARRAY + OP_COUNT:
		PUSHCODE(Bagel_BC::BC_MAKEARRAY, pos, dest, dest, 0);
		break;
	case OP_NULLDIC + OP_COUNT:
		PUSHCODE(Bagel_BC::BC_MAKEDIC, pos, dest, dest, 0);
		break;
	case OP_BLOCK + OP_COUNT:
		PUSHCODE(Bagel_BC::BC_PUSHCLO, pos, dest, curclo->clopos, 0);
		NEW_CLO(dest, curclo->breakcode, curclo->continuecode);
		a = -1;
		b = subtree->childs.size();
		while (++a < b)
		{
			_compile(dest + 1, subtree->childs[a], code);
		}
		PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		END_CLO;
		break;
	case OP_DOT:
		code.consts.emplace_back(subtree->childs[1]->Node.var);
		a = code.consts.size() - 1;
		switch (subtree->childs[0]->Node.opcode - OP_COUNT)
		{
		case OP_SUPER:
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[1]->Node.pos, dest, a, 0);
			PUSHCODE(Bagel_BC::BC_LOADSUPERMEMBER_VALUE, pos, 0, dest, dest);
			break;
		case OP_GLOBAL:
			PUSHCODE(Bagel_BC::BC_LOADGLOBALMEMBER_VALUE, pos, a, 0, dest);
			break;
		default:
			_compile(dest + 1, subtree->childs[0], code);
			PUSHCODE(Bagel_BC::BC_LOADMEMBERSTRIDX_VALUE, pos, dest + 1, a, dest);
			break;
		}
		break;
	case OP_DOT + OP_COUNT:
		code.consts.emplace_back(subtree->childs[0]->Node.var);
		if(curclo->withpos < 0)
			PUSHCODE(Bagel_BC::BC_LOADGLOBALMEMBER_VALUE, pos, code.consts.size() - 1, 0, dest);
		else
			PUSHCODE(Bagel_BC::BC_LOADMEMBERSTRIDX_VALUE, pos, curclo->withpos, code.consts.size() - 1, dest);
		break;
	case OP_CONTINUE + OP_COUNT:
		if (curclo->continuecode)
		{
			PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			curclo->continuecode->push_back(code.code.size() - 1);
		}
		else
		{
			code.consts.emplace_back(W("此处不应出现continue"));
			PUSHCODE(Bagel_BC::BC_THROWCONST, pos, code.consts.size() - 1, 0, 0);
		}
		break;
	case OP_BREAK + OP_COUNT:
		if (curclo->breakcode)
		{
			PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			curclo->breakcode->push_back(code.code.size() - 1);
		}
		else
		{
			code.consts.emplace_back(W("此处不应出现break"));
			PUSHCODE(Bagel_BC::BC_THROWCONST, pos, code.consts.size() - 1, 0, 0);
		}
		break;
	case OP_RETURN + OP_COUNT:
		a = trycount;
		while (a-- > 0)
		{
			PUSHCODE(Bagel_BC::BC_REMOVETRYHOOK, pos, 0, 0, 0);
		}
		if (subtree->childs[0])
		{
			_compile(dest, subtree->childs[0], code);
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		}
		PUSHCODE(Bagel_BC::BC_RETURN, pos, dest, 0, 0);
		break;
	case OP_YIELD + OP_COUNT:
		a = trycount;
		while (a-- > 0)
		{
			PUSHCODE(Bagel_BC::BC_REMOVETRYHOOK, pos, 0, 0, 0);
		}
		if (subtree->childs[0])
		{
			_compile(dest, subtree->childs[0], code);
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		}
		PUSHCODE(Bagel_BC::BC_YIELD, pos, dest, dest, 0);
		break;
	case OP_VAR + OP_COUNT:
		for (int i = 0; i < subtree->childs.size(); i += 2)
		{
			code.consts.emplace_back(subtree->childs[i]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[i]->Node.pos, dest, code.consts.size() - 1, 0);
			_compile(dest + 1, subtree->childs[i + 1], code);
			if(inclass)
			{
				PUSHCODE(Bagel_BC::BC_ADDVAR, subtree->childs[i]->Node.pos, inclass, dest, dest + 1);
			}
			else
			{
				PUSHCODE(Bagel_BC::BC_SETMEMBER2, subtree->childs[i]->Node.pos, curclo->clopos, dest, dest + 1);				
			}
		}
		PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		break;
	case OP_NEW + OP_COUNT:
		code.consts.emplace_back(subtree->childs[0]->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADGLOBALMEMBER_VALUE, pos, code.consts.size() - 1, 0, dest);
		PUSHCODE(Bagel_BC::BC_CHECKTYPE, pos, dest, VAR_CLASSDEF, 0);
		break;
	case OP_DELETE + OP_COUNT:
		if (subtree->childs[0]->Node.opcode == OP_DOT)
		{
			_compile(dest, subtree->childs[0]->childs[0], code);
			code.consts.emplace_back(subtree->childs[0]->childs[1]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->childs[1]->Node.pos, dest + 1, code.consts.size() - 1, 0);
			PUSHCODE(Bagel_BC::BC_DELETE, pos, dest, dest + 1, 0);
		}
		else if (subtree->childs[0]->Node.opcode == OP_DOT + OP_COUNT)
		{
			PUSHCODE(Bagel_BC::BC_LOADGLOBAL, pos, dest, a, 0);
			code.consts.emplace_back(subtree->childs[0]->childs[1]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->childs[1]->Node.pos, dest + 1, code.consts.size() - 1, 0);
			PUSHCODE(Bagel_BC::BC_DELETE, pos, dest, dest + 1, 0);
		}
		else
		{
			_compile(dest, subtree->childs[0]->childs[0], code);
			_compile(dest + 1, subtree->childs[0]->childs[1], code);
			PUSHCODE(Bagel_BC::BC_DELETE, pos, dest, dest + 1, 0);
		}
		PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		break;
	case OP_TRY + OP_COUNT:
		{
			PUSHCODE(Bagel_BC::BC_ADDTRYHOOK, pos, 0, dest, 0);
			a = code.code.size();
			PUSHCODE(Bagel_BC::BC_PUSHCLO, pos, dest, curclo->clopos, 0);
			vector<int> breakcode;
			vector<int> continuecode;
			auto rawclo = curclo;
			NEW_CLO(dest, (curclo->breakcode ? &breakcode : NULL), (curclo->continuecode ? &continuecode : NULL));
			trycount++;
			_compile(dest + 1, subtree->childs[0], code);
			trycount--;
			END_CLO;
			//handle break
			if (rawclo->breakcode)
			{
				for (auto &it : breakcode)
				{
					code.code[it].A = code.code.size();
				}
				PUSHCODE(Bagel_BC::BC_REMOVETRYHOOK, pos, 0, 0, 0);
				rawclo->breakcode->push_back(code.code.size());
				PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			}
			//handle continue
			if (rawclo->continuecode)
			{
				for (auto &it : continuecode)
				{
					code.code[it].A = code.code.size();
				}
				PUSHCODE(Bagel_BC::BC_REMOVETRYHOOK, pos, 0, 0, 0);
				rawclo->continuecode->push_back(code.code.size());
				PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			}
			PUSHCODE(Bagel_BC::BC_REMOVETRYHOOK, pos, 0, 0, 0);
			PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			b = code.code.size();
			code.code[a - 1].A = b;
			PUSHCODE(Bagel_BC::BC_PUSHCLO, pos, dest + 1, curclo->clopos, 0);
			NEW_CLO(dest + 1, curclo->breakcode, curclo->continuecode);
			if(subtree->childs[1])
			{
				code.consts.emplace_back(subtree->childs[1]->Node.var);
				PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[1]->Node.pos, dest + 2, code.consts.size() - 1, 0);
				PUSHCODE(Bagel_BC::BC_SETMEMBER2, subtree->childs[1]->Node.pos, dest + 1, dest + 2, dest);
			}
			_compile(dest + 2, subtree->childs[2], code);
			END_CLO;
			code.code[b - 1].A = code.code.size();
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		}
		break;
	case OP_THROW + OP_COUNT:
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(Bagel_BC::BC_THROW, pos, dest, 0, 0);
		break;
	case OP_THIS + OP_COUNT:
		PUSHCODE(Bagel_BC::BC_CHECKTHIS, pos, 0, 0, 0);
		PUSHCODE(Bagel_BC::BC_MOV, pos, dest, 1, 0);
		break;
	case OP_SUPER + OP_COUNT:
		code.consts.emplace_back(W("此处不应出现super"));
		PUSHCODE(Bagel_BC::BC_THROWCONST, pos, code.consts.size() - 1, 0, 0);
		break;
	case OP_GLOBAL + OP_COUNT:
		PUSHCODE(Bagel_BC::BC_LOADGLOBAL, pos, dest, 0, 0);
		break;
	case OP_TOINT + OP_COUNT:
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(Bagel_BC::BC_TOINT, pos, dest, dest, 0);
		break;
	case OP_TOSTRING + OP_COUNT:
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(Bagel_BC::BC_TOSTRING, pos, dest, dest, 0);
		break;
	case OP_TONUMBER + OP_COUNT:
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(Bagel_BC::BC_TONUMBER, pos, dest, dest, 0);
		break;
	case OP_CONST + OP_COUNT:
		_compile(dest, subtree->childs[0], code);
		break;
	case OP_TYPE + OP_COUNT:
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(Bagel_BC::BC_TYPE, pos, dest, dest, 0);
		break;
	case OP_INSTANCE:
		_compile(dest, subtree->childs[0], code);
		_compile(dest + 1, subtree->childs[1], code);
		PUSHCODE(Bagel_BC::BC_INSTANCE, pos, dest, dest, dest + 1);
		break;
	case OP_SWITCH + OP_COUNT:
		{
			vector<int> jumptable;
			_compile(dest, subtree->childs[0], code);
			PUSHCODE(Bagel_BC::BC_PUSHCLO, pos, dest + 1, curclo->clopos, 0);
			vector<int> breakcode;
			NEW_CLO(dest + 1, &breakcode, curclo->continuecode);
			for (auto &it : subtree->childs[1]->childs)
			{
				if (it->Node.opcode == OP_CASE + OP_COUNT)
				{
					_compile(dest + 2, it->childs[0], code);
					jumptable.push_back(code.code.size());
					PUSHCODE(Bagel_BC::BC_JUMPEEQUAL, it->Node.pos, 0, dest, dest + 2);
				}
			}
			//default pos
			b = code.code.size();
			PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			//has default
			c = 0;
			a = 0;
			for (auto &it : subtree->childs[1]->childs)
			{
				if (it->Node.opcode == OP_DEFAULT + OP_COUNT)
				{
					code.code[b].A = code.code.size();
					c = 1;
					continue;
				}
				else if (it->Node.opcode == OP_CASE + OP_COUNT)
				{
					code.code[jumptable[a]].A = code.code.size();
					a++;
					continue;
				}
				else
				{
					_compile(dest, it, code);
				}
			}
			if (!c)
			{
				code.code[b].A = code.code.size();
			}
			//handle breaks
			for (auto &it : breakcode)
			{
				code.code[it].A= code.code.size();
			}
			END_CLO;
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		}
		break;
	case OP_CASE + OP_COUNT:
		code.consts.emplace_back(W("此处不应出现case"));
		PUSHCODE(Bagel_BC::BC_THROWCONST, pos, code.consts.size() - 1, 0, 0);
		break;
	case OP_DEFAULT + OP_COUNT:
		code.consts.emplace_back(W("此处不应出现default"));
		PUSHCODE(Bagel_BC::BC_THROWCONST, pos, code.consts.size() - 1, 0, 0);
		break;
	case OP_RESERVE + OP_COUNT:
		a = -1;
		b = subtree->childs.size();
		while (++a < b)
		{
			_compile(dest, subtree->childs[a], code);
		}
		PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		break;
	case OP_RESERVE2 + OP_COUNT:
		a = -1;
		b = subtree->childs.size();
		while (++a < b)
		{
			_compile(dest, subtree->childs[a], code);
		}
		break;
	case OP_WITH + OP_COUNT:
		_compile(dest, subtree->childs[0], code);
		clos.emplace_back();
		newclo = &clos.back();
		newclo->withpos = dest;
		newclo->clopos = curclo->clopos;
		newclo->breakcode = curclo->breakcode;
		newclo->continuecode = curclo->continuecode;
		curclo = newclo;
		_compile(dest + 1, subtree->childs[1], code);
		clos.pop_back();
		curclo = &clos.back();
		PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		break;
	case OP_INCONTEXTOF:
		_compile(dest, subtree->childs[0], code);
		_compile(dest + 1, subtree->childs[1], code);
		PUSHCODE(Bagel_BC::BC_INCONTEXTOF, pos, dest, dest, dest + 1);
		break;
	case OP_STATIC + OP_COUNT:
		if (!inclass && !subtree->childs.empty())
		{
			code.consts.emplace_back(W("此处不应出现static"));
			PUSHCODE(Bagel_BC::BC_THROWCONST, pos, code.consts.size() - 1, 0, 0);
			break;
		}
		for (int i = 0; i < subtree->childs.size(); i += 2)
		{
			code.consts.emplace_back(subtree->childs[i]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[i]->Node.pos, dest, code.consts.size() - 1, 0);
			_compile(dest + 1, subtree->childs[i + 1], code);
			PUSHCODE(Bagel_BC::BC_ADDSTATIC, subtree->childs[i]->Node.pos, inclass, dest, dest + 1);
		}
		PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
		break;
	case OP_ENUM + OP_COUNT:
		if (!subtree->childs[0]->Node.var.isVoid())
		{
			PUSHCODE(Bagel_BC::BC_MAKEDIC, pos, dest, dest, 0);
			a = dest;
		}
		else
		{
			a = curclo->clopos;
		}
		for (int i = 1; i < subtree->childs.size(); i += 2)
		{
			code.consts.push_back(subtree->childs[i]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[i]->Node.pos, dest + 1, code.consts.size() - 1, 0);
			int32_t buf[2];
			*(double*)buf = subtree->childs[i + 1]->Node.var.forceAsNumber();
			PUSHCODE(Bagel_BC::BC_LOADNUM, pos, dest + 2, buf[0], buf[1]);
			PUSHCODE(Bagel_BC::BC_SETMEMBER2, subtree->childs[i]->Node.pos, a, dest + 1, dest + 2);
		}
		if (!subtree->childs[0]->Node.var.isVoid())
		{
			code.consts.push_back(subtree->childs[0]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 1, code.consts.size() - 1, 0);
			PUSHCODE(Bagel_BC::BC_SETMEMBER2, pos, curclo->clopos, dest + 1, dest);
		}
		break;
	case OP_BITAND:
	case OP_BITOR:
	case OP_BITXOR:
		_compile(dest, subtree->childs[0], code);
		_compile(dest + 1, subtree->childs[1], code);
		PUSHCODE(subtree->Node.opcode - OP_BITAND + Bagel_BC::BC_BITAND, pos, dest, dest, dest + 1);
		break;
	case OP_BITNOT + OP_COUNT:
		_compile(dest, subtree->childs[0], code);
		PUSHCODE(Bagel_BC::BC_BITNOT, pos, dest, dest, 0);
		break;
	case OP_EXTRACTARGS:
		assert(false);
		code.consts.emplace_back(W("这里不应该出现..."));
		PUSHCODE(Bagel_BC::BC_THROWCONST, pos, code.consts.size() - 1, 0, 0);
		break;
	default:
		code.consts.emplace_back(W("出现了意料之外的Opcode") + bkpInt2Str(subtree->Node.opcode));
		PUSHCODE(Bagel_BC::BC_THROWCONST, pos, code.consts.size() - 1, 0, 0);
		//throw Bagel_Except(W("出现了意料之外的Opcode") + bkpInt2Str(subtree->Node.opcode));
		break;
	}
}



void Bagel_DebugCompiler::compile(Bagel_AST * tree, Bagel_ByteCode & code, const StringVal &exp, bool)
{
	code.stackDepth = 0;
	code.runTimes = 0;
	inclass = 0;
	trycount = 0;
	clos.emplace_back();
	curclo = &clos.back();
	curclo->clopos = 0;
	curclo->withpos = -1;
	curclo->breakcode = NULL;
	curclo->continuecode = NULL;
	//1位置留给this
	_compile(2, tree, code);
	PUSHCODE(Bagel_BC::BC_RETURN, 0, 2, 0, 0);
	if (!exp.empty())
	{
		code.debugInfo.reset(new Bagel_DebugInformation(exp));
	}
}

void Bagel_DebugCompiler::compileAddr(Bagel_AST * tree, Bagel_ByteCode & code, const StringVal & exp, bool)
{
	code.stackDepth = 0;
	code.runTimes = 0;
	inclass = 0;
	clos.emplace_back();
	curclo = &clos.back();
	curclo->clopos = 0;
	curclo->withpos = -1;
	curclo->breakcode = NULL;
	curclo->continuecode = NULL;
	auto subtree = tree;
	auto dest = 1;
	auto pos = tree->Node.pos;
	int a, b;
	switch (tree->Node.opcode)
	{
	case OP_LITERAL + OP_COUNT:
		code.consts.emplace_back(subtree->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->Node.pos, dest + 1, code.consts.size() - 1, 0);
		a = 0;
		b = dest + 1;
		break;
	case OP_DOT + OP_COUNT:
		code.consts.emplace_back(subtree->childs[0]->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->Node.pos, dest + 1, code.consts.size() - 1, 0);
		if (curclo->withpos >= 0)
		{
			a = curclo->withpos;
			b = dest + 1;
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_LOADGLOBAL, pos, dest + 2, 0, 0);
			a = dest + 2;
			b = dest + 1;
		}
		break;
	case OP_DOT:
		code.consts.emplace_back(subtree->childs[1]->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[1]->Node.pos, dest + 1, code.consts.size() - 1, 0);
		_compile(dest + 2, subtree->childs[0], code);
		a = dest + 2;
		b = dest + 1;
		break;
	case OP_ARRAY:
		_compile(dest + 1, subtree->childs[0], code);
		_compile(dest + 2, subtree->childs[1], code);
		a = dest + 1;
		b = dest + 2;
		break;
	}
	PUSHCODE(Bagel_BC::BC_CREATEPOINTER, pos, dest, a, b);
	PUSHCODE(Bagel_BC::BC_RETURN, 0, 1, 0, 0);
	if (!exp.empty())
	{
		code.debugInfo.reset(new Bagel_DebugInformation(exp));
	}
}
