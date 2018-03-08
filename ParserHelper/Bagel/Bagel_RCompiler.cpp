#include "Bagel_RCompiler.h"

#define PUSHCODE(a,b,c,d,e) if(lockcode <= 0)code.code.emplace_back(a, b, c, d, e)
#define PUSHCODE2(a,b,c,d) if(lockcode <= 0)code.code.emplace_back(a, b, c, d)
#define REGSTACK(p) stacks[p]=code.code.size()-1;
#define PUSHCACHE(pos) \
if(jit){		\
PUSHCODE(Bagel_BC::BC_CACHE_NONE, pos, 0, 0, 0); \
PUSHCODE(Bagel_BC::BC_CACHE_NONE, pos, 0, 0, 0); \
PUSHCODE(Bagel_BC::BC_CACHE_NONE, pos, 0, 0, 0);}

#define REG_TYPE(a,b) typeinfo->set(a, b);

#define NEW_CLO(b, c) \
clos.emplace_back(); \
newclo = &clos.back(); \
newclo->withpos = curclo->withpos; \
newclo->breakcode = b; \
newclo->continuecode = c; \
newclo->parent = curclo; \
curclo = newclo;
//newclo->switchtable = curclo->switchtable;

#define END_CLO(tr) \
clos.back().purgeClo(curcode->varPosInfo.get(), tr); \
clos.pop_back(); \
curclo = &clos.back(); 

#define REG_VAR(clo, var, codepos, stackpos) \
clo->regLocal(var, stackpos);  \
if(curcode->varPosInfo)  \
	curcode->varPosInfo->addVar(var, codepos, stackpos);

#define ENSURE(c) if(code.stackDepth < (c)) code.stackDepth = (c);

int Bagel_ReleaseCompiler::getVarPos(Bagel_StringHolder str, int codepos)
{
	int a = curclo->getLocal(str);
	if (a)
		return a;
	if (!parent)
		return a;	//a is 0
	a = parent->getVarPos(str, 0);
	if (!a)
		return a;	//a is 0
	//REG_TYPE(nextlocalpos, (*parent->typeinfo)[a]);
	REG_TYPE(nextlocalpos, VAR_UNKNOWN);	//可能会在其他地方被更改
	if (lockcode > 0)
	{
		nextlocalpos--;
		return nextlocalpos + 1;
	}
	toParentMap[nextlocalpos] = a;
	fixed.insert(nextlocalpos);
	if (parent->fixed.find(a) == parent->fixed.end())
	{
		if (lockcode <= 0)
			linkCode.emplace_back(Bagel_BC::BC_INITSTACKMEMBER, codepos, nextlocalpos, a, 0);
		if (curcode->varPosInfo)
		{
			curcode->varPosInfo->addParentVar(str, codepos, nextlocalpos, a, false);
		}
	}
	else
	{
		if (lockcode <= 0)
			linkCode.emplace_back(Bagel_BC::BC_INITSTACKMEMBER2, codepos, nextlocalpos, a, 0);
		if (curcode->varPosInfo)
		{
			curcode->varPosInfo->addParentVar(str, codepos, nextlocalpos, a, true);
		}
	}
	clos.front().regLocal(str, nextlocalpos--);
	return nextlocalpos + 1;
}

int Bagel_ReleaseCompiler::_compile(int dest, Bagel_AST * subtree, Bagel_ByteCode & code, bool need_return, bool jit)
{
	int a, b, c, d, e;
	int d1, d2, d3;
	CompileClo *newclo;
	ENSURE(dest);
	if (!subtree)
	{
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, code.code.empty() ? 0 : code.code.back().pos, dest, 0, 0);
			return dest;
		}
		return -1;
	}
	int pos = subtree->Node.pos;
    {
	switch (subtree->Node.opcode)
	{
	case OP_END:
	case OP_END + OP_COUNT:
		break;
	case OP_IF:
		if (subtree->childs[1] && subtree->childs[1]->Node.opcode >= OP_EQUAL && subtree->childs[1]->Node.opcode <= OP_NOT)
		{
			int op = subtree->childs[1]->Node.opcode;
			d1 = _compile(dest, subtree->childs[1]->childs[0], code, true, jit);
			if (op != OP_NOT)
				d2 = _compile(dest + 1, subtree->childs[1]->childs[1], code, true, jit);
			switch (op)
			{
			case OP_EQUAL:
			case OP_NEQUAL:
				PUSHCODE(Bagel_BC::BC_JUMPEQUAL + OP_NEQUAL - op, pos, 0, d1, d2);
				break;
			case OP_EEQUAL:
			case OP_NNEQUAL:
				PUSHCODE(Bagel_BC::BC_JUMPEEQUAL + OP_NNEQUAL - op, pos, 0, d1, d2);
				break;
			case OP_NOT:
				PUSHCODE(Bagel_BC::BC_JUMPTRUE, pos, 0, d1, 0);
				break;
			default:
				PUSHCODE(Bagel_BC::BC_JUMPLARGER + OP_SE - op, pos, 0, d1, d2);
				break;
			}
		}
		else
		{
			d1 = _compile(dest, subtree->childs[1], code, true, jit);
			PUSHCODE(Bagel_BC::BC_JUMPFALSE, pos, 0, d1, 0);
		}
		a = code.code.size();
		generateTypeLevel();
		_compile(dest, subtree->childs[0], code, need_return, jit);
		deleteTypeLevel();
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_JUMP, (int)code.code.back().pos, 0, 0, 0);
			b = code.code.size();
			if (lockcode <= 0)
				code.code[a - 1].A = b;
			PUSHCODE(Bagel_BC::BC_LOADVOID, (int)code.code.back().pos, dest, 0, 0);
			REG_TYPE(dest, VAR_NONE);
			if (lockcode <= 0)
				code.code[b - 1].A = code.code.size();
		}
		else
		{
			if (lockcode <= 0)
				code.code[a - 1].A = code.code.size();
		}
		break;
	case OP_IF + OP_COUNT:
	case OP_CHOOSE:
		{
			if (subtree->Node.opcode == OP_IF + OP_COUNT)
				need_return = false;
			//语法树生成那里保证childs[1]和childs[2]非空的话一定新建一个子闭包
			if (subtree->childs[0] && subtree->childs[0]->Node.opcode >= OP_EQUAL && subtree->childs[0]->Node.opcode <= OP_NOT)
			{
				int op = subtree->childs[0]->Node.opcode;
				d1 = _compile(dest, subtree->childs[0]->childs[0], code, true, jit);
				if (op != OP_NOT)
					d2 = _compile(dest + 1, subtree->childs[0]->childs[1], code, true, jit);
				switch (op)
				{
				case OP_EQUAL:
				case OP_NEQUAL:
					PUSHCODE(Bagel_BC::BC_JUMPEQUAL + OP_NEQUAL - op, pos, 0, d1, d2);
					break;
				case OP_EEQUAL:
				case OP_NNEQUAL:
					PUSHCODE(Bagel_BC::BC_JUMPEEQUAL + OP_NNEQUAL - op, pos, 0, d1, d2);
					break;
				case OP_NOT:
					PUSHCODE(Bagel_BC::BC_JUMPTRUE, pos, 0, d1, 0);
					break;
				default:
					PUSHCODE(Bagel_BC::BC_JUMPLARGER + OP_SE - op, pos, 0, d1, d2);
					break;
				}
			}
			else
			{
				d1 = _compile(dest, subtree->childs[0], code, true, jit);
				PUSHCODE(Bagel_BC::BC_JUMPFALSE, pos, 0, d1, 0);
			}
			a = code.code.size();
			generateTypeLevel();
			d2 = _compile(dest, subtree->childs[1], code, need_return, jit);
			if (need_return && d2 != dest)
			{
				PUSHCODE(Bagel_BC::BC_MOV, pos, dest, d2, 0);
			}
			auto iftype = *typeinfo;
			deleteTypeLevel();
			if (subtree->childs[2])
			{
				PUSHCODE(Bagel_BC::BC_JUMP, (int)code.code.back().pos, 0, 0, 0);
				b = code.code.size();
				if (lockcode <= 0)
					code.code[a - 1].A = b;
				generateTypeLevel();
				d2 = _compile(dest, subtree->childs[2], code, need_return, jit);
				if (need_return && d2 != dest)
				{
					PUSHCODE(Bagel_BC::BC_MOV, pos, dest, d2, 0);
				}
				mergeTwoTypeinfo(iftype, *typeinfo);
				if (lockcode <= 0)
					code.code[b - 1].A = code.code.size();
			}
			else
			{
				if (lockcode <= 0)
					code.code[a - 1].A = code.code.size();
			}
		}
		break;
	case OP_QUICKFOR + OP_COUNT:
		{
			vector<int> breakcode;
			vector<int> continuecode;
			d1 = _compile(dest, subtree->childs[1], code, true, jit);
			if (d1 != dest)
			{
				PUSHCODE(Bagel_BC::BC_MOV, (int)code.code.back().pos, dest, d1, 0);
			}
			d2 = _compile(dest + 1, subtree->childs[2], code, true, jit);
			if (d2 != dest + 1)
			{
				PUSHCODE(Bagel_BC::BC_MOV, (int)code.code.back().pos, dest + 1, d2, 0);
			}
			if (subtree->childs[3])
			{
				d3 = _compile(dest + 2, subtree->childs[3], code, true, jit);
				if (d3 != dest + 2)
				{
					PUSHCODE(Bagel_BC::BC_MOV, (int)code.code.back().pos, dest + 2, d3, 0);
				}
			}
			else
			{
				PUSHCODE(Bagel_BC::BC_LOADVOID, (int)code.code.back().pos, dest + 2, 0, 0);
			}
			//先运行一遍
			//再编译成循环
			//LOOPSTART -> loop1 -> LOOPSTEP2 -> loop n -> LOOPSTEP
			REG_TYPE(dest, VAR_NUM);
			REG_TYPE(dest + 1, VAR_NUM);
			REG_TYPE(dest + 2, VAR_NUM);
			NEW_CLO(&breakcode, &continuecode);
			int varpos = nextlocalpos--;
			REG_TYPE(varpos, VAR_NUM);
			PUSHCODE(Bagel_BC::BC_LOOPSTART, (int)code.code.back().pos, dest, 0, varpos);
			a = code.code.size();
			REG_VAR(curclo, subtree->childs[0]->Node.var, subtree->childs[0]->Node.pos, varpos);
			int lasttemp = lastlooplevel;
			lastlooplevel = typeinfos.size();
			generateTypeLevel();
			typeinfo->flag = 1;
			int tmp = nextlocalpos;
			_compile(dest + 3, subtree->childs[4], code, false, true);
			nextlocalpos = tmp;
			deleteTypeLevel();
			lastlooplevel = lasttemp;
			PUSHCODE(Bagel_BC::BC_LOOPSTEP2, (int)code.code.back().pos, dest, 0, varpos);

			c = code.code.size();	//loop start pos
			generateTypeLevel();
			//jit loop
			bool tolock = (lockcode == 0);
			if (tolock)
			{
				lockcode = 1;
				int nexttemp = nextlocalpos;
				_compile(dest + 3, subtree->childs[4], code, false, true);
				nextlocalpos = nexttemp;
				deleteTypeLevel();
				lockcode = -1;
				generateTypeLevel();
			}
			_compile(dest + 3, subtree->childs[4], code, false, true);
			if (tolock)
				lockcode = 0;
			deleteTypeLevel();
			if (lockcode <= 0)
			{
				b = code.code.size();
				for (auto &i : continuecode)
				{
					code.code[i].A = b;
				}
				PUSHCODE(Bagel_BC::BC_LOOPSTEP, (int)code.code.back().pos, dest, c, varpos);
				b = code.code.size();
				code.code[a - 1].B = b;
				code.code[c - 1].B = b;
				for (auto &i : breakcode)
				{
					code.code[i].A = b;
				}
			}
			if (need_return)
			{
				PUSHCODE(Bagel_BC::BC_LOADVOID, (int)code.code.back().pos, dest, 0, 0);
			}
			END_CLO(subtree);
		}
		break;
	case OP_FOR + OP_COUNT:
		{
			vector<int> breakcode;
			vector<int> continuecode;
			auto rawb = curclo->breakcode;
			auto rawc = curclo->continuecode;
			NEW_CLO(rawb, rawc);
			_compile(dest, subtree->childs[0], code, false, jit);
			bool tolock = (lockcode == 0);
			generateTypeLevel();
			if (tolock)
			{
				lockcode = 1;
				int nexttemp = nextlocalpos;
				_compile(dest, subtree->childs[1], code, true, true);
				_compile(dest, subtree->childs[3], code, false, true);
				_compile(dest, subtree->childs[2], code, false, true);
				nextlocalpos = nexttemp;
				deleteTypeLevel();
				lockcode = -1;
				generateTypeLevel();
			}
			a = code.code.size();
			//jit loop condition and loop step
			if (subtree->childs[1])
			{
				if (subtree->childs[1] && subtree->childs[1]->Node.opcode >= OP_EQUAL && subtree->childs[1]->Node.opcode <= OP_NOT)
				{
					int op = subtree->childs[1]->Node.opcode;
					d1 = _compile(dest, subtree->childs[1]->childs[0], code, true, true);
					if (op != OP_NOT)
						d2 = _compile(dest + 1, subtree->childs[1]->childs[1], code, true, true);
					switch (op)
					{
					case OP_EQUAL:
					case OP_NEQUAL:
						PUSHCODE(Bagel_BC::BC_JUMPEQUAL + OP_NEQUAL - op, pos, 0, d1, d2);
						break;
					case OP_EEQUAL:
					case OP_NNEQUAL:
						PUSHCODE(Bagel_BC::BC_JUMPEEQUAL + OP_NNEQUAL - op, pos, 0, d1, d2);
						break;
					case OP_NOT:
						PUSHCODE(Bagel_BC::BC_JUMPTRUE, pos, 0, d1, 0);
						break;
					default:
						PUSHCODE(Bagel_BC::BC_JUMPLARGER + OP_SE - op, pos, 0, d1, d2);
						break;
					}
				}
				else
				{
					d1 = _compile(dest, subtree->childs[1], code, true, true);
					PUSHCODE(Bagel_BC::BC_JUMPFALSE, pos, 0, d1, 0);
				}
				b = code.code.size();
			}
			newclo->breakcode = &breakcode;
			newclo->continuecode = &continuecode;
			_compile(dest, subtree->childs[3], code, false, true);
			if (lockcode <= 0)
			{
				for (auto &i : continuecode)
				{
					code.code[i].A = code.code.size();
				}
			}
			newclo->breakcode = rawb;
			newclo->continuecode = rawc;
			_compile(dest, subtree->childs[2], code, false, true);
			PUSHCODE(Bagel_BC::BC_GC, (int)code.code.back().pos, 0, 0, 0);
			PUSHCODE(Bagel_BC::BC_JUMP, (int)code.code.back().pos, a, 0, 0);
			if (tolock)
				lockcode = 0;
			deleteTypeLevel();
			END_CLO(subtree);
			if (lockcode <= 0)
			{
				for (auto &i : breakcode)
				{
					code.code[i].A = code.code.size();
				}
				if (subtree->childs[1])
				{
					code.code[b - 1].A = code.code.size();
				}
				if (need_return)
				{
					PUSHCODE(Bagel_BC::BC_LOADVOID, (int)code.code.back().pos, dest, 0, 0);
				}
			}
		}
		break;
	case OP_WHILE + OP_COUNT:
		{
			vector<int> breakcode;
			vector<int> continuecode;
			bool tolock = (lockcode == 0);
			generateTypeLevel();
			if (tolock)
			{
				lockcode = 1;
				int nexttemp = nextlocalpos;
				_compile(dest, subtree->childs[0], code, true, true);
				NEW_CLO(&breakcode, &continuecode);
				_compile(dest, subtree->childs[1], code, false, true);
				END_CLO(subtree);
				nextlocalpos = nexttemp;
				deleteTypeLevel();
				lockcode = -1;
				generateTypeLevel();
			}
			a = code.code.size();
			if (subtree->childs[0] && subtree->childs[0]->Node.opcode >= OP_EQUAL && subtree->childs[0]->Node.opcode <= OP_NOT)
			{
				int op = subtree->childs[0]->Node.opcode;
				d1 = _compile(dest, subtree->childs[0]->childs[0], code, true, true);
				if (op != OP_NOT)
					d2 = _compile(dest + 1, subtree->childs[0]->childs[1], code, true, true);
				switch (op)
				{
				case OP_EQUAL:
				case OP_NEQUAL:
					PUSHCODE(Bagel_BC::BC_JUMPEQUAL + OP_NEQUAL - op, pos, 0, d1, d2);
					break;
				case OP_EEQUAL:
				case OP_NNEQUAL:
					PUSHCODE(Bagel_BC::BC_JUMPEEQUAL + OP_NNEQUAL - op, pos, 0, d1, d2);
					break;
				case OP_NOT:
					PUSHCODE(Bagel_BC::BC_JUMPTRUE, pos, 0, d1, 0);
					break;
				default:
					PUSHCODE(Bagel_BC::BC_JUMPLARGER + OP_SE - op, pos, 0, d1, d2);
					break;
				}
			}
			else
			{
				d1 = _compile(dest, subtree->childs[0], code, true, true);
				PUSHCODE(Bagel_BC::BC_JUMPFALSE, pos, 0, d1, 0);
			}
			b = code.code.size();
			NEW_CLO(&breakcode, &continuecode);
			_compile(dest, subtree->childs[1], code, false, true);
			if (lockcode <= 0)
			{
				for (auto &i : continuecode)
				{
					code.code[i].A = code.code.size();
				}
			}
			PUSHCODE(Bagel_BC::BC_GC, (int)code.code.back().pos, 0, 0, 0);
			PUSHCODE(Bagel_BC::BC_JUMP, (int)code.code.back().pos, a, 0, 0);
			if (tolock)
				lockcode = 0;
			deleteTypeLevel();
			END_CLO(subtree);
			if (lockcode <= 0)
			{
				for (auto &i : breakcode)
				{
					code.code[i].A = code.code.size();
				}
				code.code[b - 1].A = code.code.size();
				if (need_return)
				{
					PUSHCODE(Bagel_BC::BC_LOADVOID, (int)code.code.back().pos, dest, 0, 0);
					REG_TYPE(dest, VAR_NONE);
				}
			}
		}
		break;
	case OP_DO + OP_COUNT:
		{
			vector<int> breakcode;
			vector<int> continuecode;
			//先运行一遍
			//再编译成循环
			//----------loop1 -> WHILE -> loop n -> JUMP-----------[否决]
			//loop1 -> JUMP -> loop n -> WHILE
			NEW_CLO(&breakcode, &continuecode);
			int lasttemp = lastlooplevel;
			lastlooplevel = typeinfos.size();
			generateTypeLevel();
			typeinfo->flag = 1;
			int tmp = nextlocalpos;
			_compile(dest, subtree->childs[0], code, false, true);
			nextlocalpos = tmp;
			deleteTypeLevel();
			lastlooplevel = lasttemp;
			END_CLO(subtree);
			bool tolock = (lockcode == 0);
			generateTypeLevel();
			if (tolock)
			{
				lockcode = 1;
				int nexttemp = nextlocalpos;
				NEW_CLO(&breakcode, &continuecode);
				_compile(dest, subtree->childs[0], code, false, true);
				END_CLO(subtree);
				_compile(dest, subtree->childs[1], code, false, true);
				nextlocalpos = nexttemp;
				deleteTypeLevel();
				lockcode = -1;
				generateTypeLevel();
			}
			PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			//jump target
			a = code.code.size();
			PUSHCODE(Bagel_BC::BC_GC, pos, 0, 0, 0);
			NEW_CLO(&breakcode, &continuecode);
			_compile(dest, subtree->childs[0], code, false, true);
			if (tolock)
				lockcode = 0;
			deleteTypeLevel();
			END_CLO(subtree);
			//continue pos
			b = code.code.size();
			if (subtree->childs[1] && subtree->childs[1]->Node.opcode >= OP_EQUAL && subtree->childs[1]->Node.opcode <= OP_NOT)
			{
				int op = subtree->childs[1]->Node.opcode;
				d1 = _compile(dest, subtree->childs[1]->childs[0], code, true, true);
				if (op != OP_NOT)
					d2 = _compile(dest + 1, subtree->childs[1]->childs[1], code, true, true);
				switch (op)
				{
				case OP_NOT:
					PUSHCODE(Bagel_BC::BC_JUMPFALSE, pos, 0, d1, 0);
					break;
				default:
					PUSHCODE(Bagel_BC::BC_JUMPEQUAL + op - OP_EQUAL, pos, 0, d1, d2);
					break;
				}
			}
			else
			{
				d1 = _compile(dest, subtree->childs[1], code, true, true);
				PUSHCODE(Bagel_BC::BC_JUMPTRUE, pos, 0, d1, 0);
			}
			//break pos
			c = code.code.size();
			if (lockcode <= 0)
			{
				for (auto &i : continuecode)
				{
					code.code[i].A = b;
				}
				code.code[a - 1].A = b;
				for (auto &i : breakcode)
				{
					code.code[i].A = c;
				}
				code.code[c - 1].A = a;
				if (need_return)
				{
					PUSHCODE(Bagel_BC::BC_LOADVOID, (int)code.code.back().pos, dest, 0, 0);
					REG_TYPE(dest, VAR_NONE);
				}
			}
		}
		break;
	case OP_FOREACH + OP_COUNT:
		{
			vector<int> breakcode;
			vector<int> continuecode;
			d1 = _compile(dest, subtree->childs[2], code, true, jit);
			if (d1 != dest)
			{
				PUSHCODE(Bagel_BC::BC_MOV, pos, dest, d1, 0);
				REG_TYPE(dest, (*typeinfo)[d1]);
			}
			NEW_CLO(&breakcode, &continuecode);
			int varpos1 = nextlocalpos--;
			int varpos2 = nextlocalpos--;
			if (subtree->childs[0])
			{
				REG_VAR(curclo, subtree->childs[0]->Node.var, subtree->childs[0]->Node.pos, varpos1);
			}
			if (subtree->childs[1])
			{
				REG_VAR(curclo, subtree->childs[1]->Node.var, subtree->childs[1]->Node.pos, varpos2);
			}
			PUSHCODE(Bagel_BC::BC_FOREACHSTART, pos, dest, varpos1, 0);
			if ((*typeinfo)[d1] == VAR_ARRAY || (*typeinfo)[d1] == VAR_STR)
			{
				REG_TYPE(dest + 2, VAR_NUM);
			}
			else if ((*typeinfo)[d1] == VAR_DIC)
			{
				REG_TYPE(dest + 2, VAR_STR);
			}
			else
			{
				REG_TYPE(dest + 2, VAR_UNKNOWN);
			}
			REG_TYPE(dest + 1, VAR_UNKNOWN);
			REG_TYPE(dest + 3, (*typeinfo)[dest + 2]);
			REG_TYPE(dest + 4, VAR_UNKNOWN);
			c = code.code.size();
			//jit loop
			bool tolock = (lockcode == 0);
			generateTypeLevel();
			if (tolock)
			{
				lockcode = 1;
				int nexttemp = nextlocalpos;
				_compile(dest, subtree->childs[3], code, false, true);
				nextlocalpos = nexttemp;
				deleteTypeLevel();
				lockcode = -1;
				generateTypeLevel();
			}
			_compile(dest + 5, subtree->childs[3], code, false, true);
			if (tolock)
				lockcode = 0;
			deleteTypeLevel();
			END_CLO(subtree);
			if (lockcode <= 0)
			{
				for (auto &i : continuecode)
				{
					code.code[i].A = code.code.size();
				}
				//code.code[c - 1].B = code.code.size();
				PUSHCODE(Bagel_BC::BC_NEXT, (int)code.code.back().pos, dest, c, varpos1);
				code.code[c - 1].C = code.code.size();
				for (auto &i : breakcode)
				{
					code.code[i].A = code.code.size();
				}
				if (need_return)
				{
					PUSHCODE(Bagel_BC::BC_LOADVOID, (int)code.code.back().pos, dest, 0, 0);
				}
			}
		}
		break;
	case OP_CONSTVAR + OP_COUNT:
		if (!need_return)
			return dest;
		//if (subtree->Node.var.getType() == VAR_NUM)
		//{
		//	double x = subtree->Node.var;
		//	PUSHCODE2(Bagel_BC::BC_LOADNUM, pos, dest, x);
		//	REG_TYPE(dest, VAR_NUM);
		//}
		//else
		{
			a = getConstPos(code, subtree->Node.var);
			if (subtree->Node.var.getType() == VAR_DIC || subtree->Node.var.getType() == VAR_ARRAY)
			{
				PUSHCODE(Bagel_BC::BC_COPYCONST, pos, dest, a, 0);
			}
			else
			{
				PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest, a, 0);
			}
			REG_TYPE(dest, subtree->Node.var.getType());
		}
		break;
	case OP_LITERAL + OP_COUNT:
		a = getVarPos(subtree->Node.var, pos);
		if (a)
		{
			if (fixed.find(a) != fixed.end())
			{
				PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, a, 0);
				return dest;
			}
			return a;
		}
		//a = curclo->getLocal(subtree->Node.var);
		//if (a)
		//{
		//	if (fixed.find(a) != fixed.end())
		//	{
		//		PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, a, 0);
		//		return dest;
		//	}
		//	return a;
		//}
		//if (this->parent)
		//{
		//	//search parent closure
		//	a = this->parent->curclo->getLocal(subtree->Node.var);
		//	if (a)
		//	{
		//		toParentMap[nextlocalpos] = a;
		//		REG_TYPE(nextlocalpos, parent->typeinfo[a]);
		//		REG_TYPE(dest, parent->typeinfo[a]);
		//		fixed.insert(nextlocalpos);
		//		if(parent->fixed.find(a) == parent->fixed.end())
		//		{
		//			PUSHCODE(Bagel_BC::BC_INITSTACKMEMBER, pos, nextlocalpos, a, 0);
		//		}
		//		else
		//		{
		//			PUSHCODE(Bagel_BC::BC_INITSTACKMEMBER2, pos, nextlocalpos, a, 0);
		//		}
		//		PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, nextlocalpos, 0);
		//		clos.front().regLocal(subtree->Node.var, nextlocalpos--);
		//		return dest;
		//	}
		//}
		a = getConstPos(code, subtree->Node.var);
		PUSHCACHE(pos);
		PUSHCODE(Bagel_BC::BC_LOADCLOMEMBER_VALUE, pos, 0, a, dest);
		REG_TYPE(dest, VAR_UNKNOWN);
		break;
	case OP_FUNCTION + OP_COUNT:
		{
			Bagel_Function *func = subtree->Node.var.asFunc();
			code.consts.emplace_back(func);
			bool clofunc = !(inglobal && clos.size() == 1) && !inclass;
			if (clofunc && !func->name.empty())
			{
				REG_VAR(curclo, func->name, pos, nextlocalpos);
				dest = nextlocalpos;
				nextlocalpos--;
			}
			PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest, code.consts.size() - 1, 0);
			PUSHCODE(Bagel_BC::BC_SETCLOSURE, pos, dest, inclass, 0);
			REG_TYPE(dest, VAR_FUNC);
			Bagel_ReleaseCompiler tmp;
			tmp.inglobal = false;
			if (clofunc)
			{
				PUSHCODE(Bagel_BC::BC_SETCLOSURESTACK, pos, dest, 0, 0);
				//closure function
				tmp.parent = this;
				code.needSpecialStack = true;
			}
			//compile function
			{
				//static Bagel_StringHolder __self(W("__self"));
				int pcount = func->func->paramarrpos > 0 ? func->func->paramnames.size() - 1 : func->func->paramnames.size();
				tmp.typeinfo->set(0, VAR_CLO);
				for (int i = 0; i < pcount; i++)
				{
					tmp.curclo->regLocal(func->func->paramnames[i], i + PARAMSTARTPOS);
				}
				for (int i = 0; i < pcount; i++)
				{
					tmp.typeinfo->set(i + PARAMSTARTPOS, VAR_UNKNOWN);
				}
				if (func->func->paramarrpos > 0)
				{
					tmp.curclo->regLocal(func->func->paramnames[pcount], pcount + PARAMSTARTPOS);
					func->func->paramarrpos = pcount + PARAMSTARTPOS;
				}
				if (func->func->paramarrpos > 0)
				{
					tmp.typeinfo->set(func->func->paramarrpos, VAR_ARRAY);
				}
				//tmp.curclo->regLocal(__self, -1);
				tmp.nextlocalpos = -1;
				func->func->bytecode = new Bagel_ByteCode();
				func->func->bytecode->localDepth = 1;
				tmp.startStack = func->func->paramnames.size() + PARAMSTARTPOS;
				tmp.compile(func->func->code, *func->func->bytecode, W(""), true, !!curcode->varPosInfo);
				func->func->bytecode->debugInfo = func->func->info;
				if (curcode->varPosInfo)
				{
					for (int i = 0; i < pcount; i++)
					{
						func->func->bytecode->varPosInfo->addVar(func->func->paramnames[i], 0, i + PARAMSTARTPOS);
					}
					if (func->func->paramarrpos > 0)
					{
						func->func->bytecode->varPosInfo->addVar(func->func->paramnames[pcount], 0, func->func->paramarrpos);
					}
				}
				if (curcode->debugInfo)
				{
					curcode->debugInfo->innerFunc.push_back(func->func->bytecode);
				}
				//func->func->code = nullptr;
				if (code.needSpecialStack)
				{
					for (auto &it : tmp.toParentMap)
					{
						if ((*typeinfo)[it.second] != (*tmp.typeinfo)[it.first])
						{
							typeinfo->set(it.second, VAR_UNKNOWN);
							beFixed.insert(it.second);
						}
					}
				}
			}
			if (!func->name.empty())
			{
				if (!clofunc)
				{
					a = getConstPos(code, func->name);
					PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 1, a, 0);
					PUSHCODE(Bagel_BC::BC_ADDFUNCTION, pos, inclass, dest + 1, dest);
					ENSURE(dest + 1);
				}
			}
		}
		break;
	case OP_CLASS + OP_COUNT:
		{
			a = getConstPos(code, subtree->childs[0]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->Node.pos, dest, a, 0);
			a = 1;
			b = subtree->childs.size();
			while (a < b && subtree->childs[a]->Node.opcode == OP_LITERAL)
			{
				c = getConstPos(code, subtree->childs[a]->Node.var);
				PUSHCACHE(subtree->childs[a]->Node.pos);
				PUSHCODE(Bagel_BC::BC_LOADGLOBALMEMBER_VALUE, subtree->childs[a]->Node.pos, c, 0, dest + a);
				PUSHCODE(Bagel_BC::BC_CHECKTYPE, subtree->childs[a]->Node.pos, dest + a, VAR_CLASSDEF, 0);
				a++;
			}
			PUSHCODE(Bagel_BC::BC_MAKECLASS, pos, dest, dest, a);
			REG_TYPE(dest, VAR_CLASS);
			auto rawclass = inclass;
			inclass = dest;
			while (a < b)
			{
				auto *subtr = subtree->childs[a];
				_compile(dest + 1, subtr, code, false, jit);
				a++;
			}
			PUSHCODE(Bagel_BC::BC_FINISHCLASS, pos, dest, 0, 0);
			inclass = rawclass;
			break;
		}
	case OP_PROPGET + OP_COUNT:
		a = getConstPos(code, subtree->childs[0]->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest, a, 0);
		b = !(inglobal && clos.size() == 1) && !inclass;
		{
			Bagel_FunctionCode *func = subtree->Node.var.forceAsObject<Bagel_FunctionCode>();
			Bagel_ReleaseCompiler tmp;
			tmp.inglobal = false;
			if (b)
			{
				tmp.parent = this;
				code.needSpecialStack = true;
			}
			func->bytecode = new Bagel_ByteCode();
			tmp.compile(func->code, *func->bytecode, W(""), true, !!curcode->varPosInfo);
			func->bytecode->debugInfo = func->info;
			if (curcode->debugInfo)
			{
				curcode->debugInfo->innerFunc.push_back(func->bytecode);
			}
			code.consts.emplace_back(func);
			PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 1, code.consts.size() - 1, 0);
		}
		if (b)
		{
			PUSHCODE(Bagel_BC::BC_MAKELOCALPROPGET, pos, nextlocalpos, dest, dest + 1);
			fixed.insert(nextlocalpos);
			REG_VAR(curclo, subtree->childs[0]->Node.var, pos, nextlocalpos);
			nextlocalpos--;
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_ADDPROPGET, pos, inclass, dest, dest + 1);
		}
		ENSURE(dest + 1);
		break;
	case OP_PROPSET + OP_COUNT:
		a = getConstPos(code, subtree->childs[0]->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->Node.pos, dest, a, 0);
		b = !(inglobal && clos.size() == 1) && !inclass;
		{
			Bagel_FunctionCode *func = subtree->Node.var.forceAsObject<Bagel_FunctionCode>();
			Bagel_ReleaseCompiler tmp;
			tmp.inglobal = false;
			func->bytecode = new Bagel_ByteCode();
			if (b)
			{
				tmp.parent = this;
				code.needSpecialStack = true;
			}
			tmp.curclo->regLocal(func->paramnames[0], PARAMSTARTPOS);
			tmp.startStack = PARAMSTARTPOS + 1;
			tmp.compile(func->code, *func->bytecode, W(""), true, !!curcode->varPosInfo);
			if (curcode->varPosInfo)
			{
				func->bytecode->varPosInfo->addVar(func->paramnames[0], 0, PARAMSTARTPOS);
			}
			if (curcode->debugInfo)
			{
				curcode->debugInfo->innerFunc.push_back(func->bytecode);
			}
			code.consts.emplace_back(func);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->Node.pos, dest + 1, code.consts.size() - 1, 0);
		}
		if (b)
		{
			PUSHCODE(Bagel_BC::BC_MAKELOCALPROPSET, pos, nextlocalpos, dest, dest + 1);
			fixed.insert(nextlocalpos);
			REG_VAR(curclo, subtree->childs[0]->Node.var, pos, nextlocalpos);
			nextlocalpos--;
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_ADDPROPSET, pos, inclass, dest, dest + 1);
		}
		ENSURE(dest + 2);
		break;
	case OP_PROPERTY + OP_COUNT:
		a = getConstPos(code, subtree->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest, a, 0);
		b = !(inglobal && clos.size() == 1) && !inclass;
		if (subtree->childs[0])
		{
			Bagel_FunctionCode *func = subtree->childs[0]->Node.var.forceAsObject<Bagel_FunctionCode>();
			Bagel_ReleaseCompiler tmp;
			tmp.inglobal = false;
			func->bytecode = new Bagel_ByteCode();
			if (b)
			{
				tmp.parent = this;
				code.needSpecialStack = true;
			}
			tmp.compile(func->code, *func->bytecode, W(""), true, !!curcode->varPosInfo);
			if (curcode->debugInfo)
			{
				curcode->debugInfo->innerFunc.push_back(func->bytecode);
			}
			code.consts.emplace_back(func);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->Node.pos, dest + 1, code.consts.size() - 1, 0);
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest + 1, 0, 0);
		}
		if (subtree->childs[1])
		{
			Bagel_FunctionCode *func = subtree->childs[1]->Node.var.forceAsObject<Bagel_FunctionCode>();
			Bagel_ReleaseCompiler tmp;
			tmp.inglobal = false;
			func->bytecode = new Bagel_ByteCode();
			if (b)
			{
				tmp.parent = this;
				code.needSpecialStack = true;
			}
			tmp.curclo->regLocal(func->paramnames[0], PARAMSTARTPOS);
			tmp.startStack = PARAMSTARTPOS + 1;
			tmp.compile(func->code, *func->bytecode, W(""), true, !!curcode->varPosInfo);
			if (curcode->varPosInfo)
			{
				func->bytecode->varPosInfo->addVar(func->paramnames[0], 0, PARAMSTARTPOS);
			}
			if (curcode->debugInfo)
			{
				curcode->debugInfo->innerFunc.push_back(func->bytecode);
			}
			code.consts.emplace_back(func);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[1]->Node.pos, dest + 2, code.consts.size() - 1, 0);
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest + 2, 0, 0);
		}
		if (b)
		{
			PUSHCODE(Bagel_BC::BC_MAKELOCALPROPERTY, pos, nextlocalpos, dest, 0);
			fixed.insert(nextlocalpos);
			REG_VAR(curclo, subtree->Node.var, pos, nextlocalpos);
			nextlocalpos--;
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_ADDPROPERTY, pos, inclass, dest, 0);
		}
		ENSURE(dest + 3);
		break;
	case OP_ADD:
	case OP_SUB:
	case OP_MUL:
	case OP_DIV:
	case OP_INTDIV:
	case OP_MOD:
	case OP_POW:
		if (!need_return)
		{
			_compile(dest, subtree->childs[0], code, false, jit);
			_compile(dest + 1, subtree->childs[1], code, false, jit);
			break;
		}
		if (subtree->childs[0]->Node.opcode == OP_CONSTVAR + OP_COUNT)
		{
			a = getConstPos(code, subtree->childs[0]->Node.var);
			d2 = _compile(dest + 1, subtree->childs[1], code, need_return, jit);
			PUSHCODE(subtree->Node.opcode - OP_ADD + Bagel_BC::BC_ADDCS, pos, dest, a, d2);
			b = subtree->childs[0]->Node.var.getType();
			if (b == VAR_NONE)
				b = (*typeinfo)[d2];
		}
		else
		{
			d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
			b = (*typeinfo)[d1];
			if (subtree->childs[1]->Node.opcode == OP_CONSTVAR + OP_COUNT)
			{
				a = getConstPos(code, subtree->childs[1]->Node.var);
				if (d1 != dest)
				{
					PUSHCODE(subtree->Node.opcode - OP_ADD + Bagel_BC::BC_ADDSC, pos, dest, d1, a);
				}
				else
				{
					PUSHCODE(subtree->Node.opcode - OP_ADD + Bagel_BC::BC_SETADDC, pos, dest, a, 0);
				}
				if (b == VAR_NONE)
					b = subtree->childs[1]->Node.var.getType();
			}
			else
			{
				d2 = _compile(dest + 1, subtree->childs[1], code, need_return, jit);
				if (d1 != dest)
				{
					PUSHCODE(subtree->Node.opcode - OP_ADD + Bagel_BC::BC_ADD, pos, dest, d1, d2);
				}
				else
				{
					PUSHCODE(subtree->Node.opcode - OP_ADD + Bagel_BC::BC_SETADD, pos, dest, d2, 0);
				}
				if (b == VAR_NONE)
					b = (*typeinfo)[d2];
			}
		}
		if (subtree->Node.opcode == OP_ADD || subtree->Node.opcode == OP_SUB)
		{
			REG_TYPE(dest, b);
		}
		else
		{
			REG_TYPE(dest, VAR_NUM);
		}
		break;
	case OP_ADD + OP_COUNT:
	case OP_SUB + OP_COUNT:
		d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
		if (need_return)
		{
			PUSHCODE(subtree->Node.opcode - OP_ADD - OP_COUNT + Bagel_BC::BC_TONUMBER, pos, dest, d1, 0);
			REG_TYPE(dest, VAR_NUM);
		}
		break;
	case OP_EQUAL:
	case OP_NEQUAL:
	case OP_EEQUAL:
	case OP_NNEQUAL:
	case OP_LARGER:
	case OP_SMALLER:
	case OP_LE:
	case OP_SE:
		d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
		d2 = _compile(dest + 1, subtree->childs[1], code, need_return, jit);
		if (need_return)
		{
			PUSHCODE(subtree->Node.opcode - OP_EQUAL + Bagel_BC::BC_EQUAL, pos, dest, d1, d2);
			REG_TYPE(dest, VAR_NUM);
		}
		break;
	case OP_FASTAND:
		d1 = _compile(dest, subtree->childs[0], code, true, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_MOV, pos, dest, d1, 0);
			REG_TYPE(dest, (*typeinfo)[d1]);
		}
		PUSHCODE(Bagel_BC::BC_JUMPVOID, pos, 0, d1, 0);
		a = code.code.size();
		generateTypeLevel();
		d2 = _compile(dest, subtree->childs[1], code, true, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_MOV, pos, dest, d2, 0);
			REG_TYPE(dest, (*typeinfo)[d2]);
		}
		deleteTypeLevel();
		if (lockcode <= 0)
			code.code[a - 1].A = code.code.size();
		break;
	case OP_AND:
		d1 = _compile(dest, subtree->childs[0], code, true, jit);
		PUSHCODE(Bagel_BC::BC_TOBOOL, pos, dest, d1, 0);
		PUSHCODE(Bagel_BC::BC_JUMPFALSE, pos, 0, dest, 0);
		a = code.code.size();
		generateTypeLevel();
		d2 = _compile(dest, subtree->childs[1], code, true, jit);
		PUSHCODE(Bagel_BC::BC_TOBOOL, pos, dest, d2, 0);
		deleteTypeLevel();
		REG_TYPE(dest, VAR_NUM);
		if (lockcode <= 0)
			code.code[a - 1].A = code.code.size();
		break;
	case OP_FASTOR:
		d1 = _compile(dest, subtree->childs[0], code, true, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_MOV, pos, dest, d1, 0);
			REG_TYPE(dest, (*typeinfo)[d1]);
		}
		PUSHCODE(Bagel_BC::BC_JUMPNOTVOID, pos, 0, d1, 0);
		a = code.code.size();
		generateTypeLevel();
		d2 = _compile(dest, subtree->childs[1], code, true, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_MOV, pos, dest, d2, 0);
			REG_TYPE(dest, (*typeinfo)[d2]);
		}
		deleteTypeLevel();
		if (lockcode <= 0)
			code.code[a - 1].A = code.code.size();
		break;
	case OP_OR:
		d1 = _compile(dest, subtree->childs[0], code, true, jit);
		PUSHCODE(Bagel_BC::BC_TOBOOL, pos, dest, d1, 0);
		PUSHCODE(Bagel_BC::BC_JUMPTRUE, pos, 0, dest, 0);
		a = code.code.size();
		generateTypeLevel();
		d2 = _compile(dest, subtree->childs[1], code, true, jit);
		PUSHCODE(Bagel_BC::BC_TOBOOL, pos, dest, d2, 0);
		deleteTypeLevel();
		REG_TYPE(dest, VAR_NUM);
		if (lockcode <= 0)
			code.code[a - 1].A = code.code.size();
		break;
	case OP_NOT + OP_COUNT:
		d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_NOT, pos, dest, d1, 0);
			REG_TYPE(dest, VAR_NUM);
		}
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
		//e表示const版本
		//d1是右边的值的位置
		if (subtree->childs[1]->Node.opcode == OP_CONSTVAR + OP_COUNT && subtree->childs[1]->Node.var.getType() <= VAR_DIC)
		{
			d1 = getConstPos(code, subtree->childs[1]->Node.var);
			e = Bagel_BC::BC_PTR_SETC - Bagel_BC::BC_PTR_SET;
		}
		else
		{
			d1 = _compile(dest, subtree->childs[1], code, true, jit);
			e = 0;
		}
		goto common;
		//if (d1 != dest)
		//{
		//	PUSHCODE(Bagel_BC::BC_MOV, pos, dest, d1, 0);
		//}
	case OP_SELFINC:
	case OP_SELFDEC:
	case OP_SELFINC + OP_COUNT:
	case OP_SELFDEC + OP_COUNT:
		d1 = 0;
		e = 0;
	common:
		a = subtree->Node.opcode - OP_SET;
		if (a > OP_COUNT)
		{
			a -= OP_COUNT - 2;
		}
		if (!need_return && (a == OP_SELFINC - OP_SET || a == OP_SELFDEC - OP_SET))
		{
			//不需要返回值的时候，将后置++--改为前置
			a += 2;
		}
		//bcs的C位置
		d = 0;
		if (a == OP_SELFINC - OP_SET || a == OP_SELFDEC - OP_SET)
		{
			d = dest;
		}
		switch (subtree->childs[0]->Node.opcode)
		{
		case OP_LITERAL + OP_COUNT:
			b = getVarPos(subtree->childs[0]->Node.var, subtree->childs[0]->Node.pos);
			c = 0;//means b is or not a pointer
			if (fixed.find(b) != fixed.end())
			{
				c = b;
			}
			if (b)
			{
				if (c)
				{
					//pointer
					PUSHCODE(Bagel_BC::BC_PTR_SET + a + e, pos, c, d1, d);
					if (a)
					{
						//为了实现yield得把property的计算拆开
						PUSHCODE(Bagel_BC::BC_LOADPOINTER2, pos, dest + 1, c, 0);
						PUSHCODE(Bagel_BC::BC_MOV + a + e, pos, dest + 1, d1, d);
						PUSHCODE(Bagel_BC::BC_SETPOINTER2, pos, c, dest + 1, 0);
					}
					REG_TYPE(dest, VAR_UNKNOWN);//有property的话什么也不知道
					//if (!a || (a == OP_SETSET - OP_SET && (*typeinfo)[c] == VAR_NONE))
					//{
					//	REG_TYPE(c, (*typeinfo)[d1]);
					//}
					//else if (a != OP_SETSET - OP_SET)
					//{
					//	//其余运算都返回数值
					//	REG_TYPE(c, VAR_NUM);
					//}
					if (need_return && !d)
					{
						PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, c, 0);
						//REG_TYPE(dest, (*typeinfo)[c]);
					}
				}
				else
				{
					//local
					if (a == OP_SELFINC - OP_SET || a == OP_SELFDEC - OP_SET)
					{
						PUSHCODE(Bagel_BC::BC_MOV + a, pos, b, 0, dest);
					}
					else if (!e || a)
					{
						PUSHCODE(Bagel_BC::BC_MOV + a + e, pos, b, d1, 0);
						dest = b;
					}
					else
					{
						//右边是个常量
						if (subtree->childs[1]->Node.var.getType() == VAR_DIC || subtree->childs[1]->Node.var.getType() == VAR_ARRAY)
						{
							PUSHCODE(Bagel_BC::BC_COPYCONST, pos, b, d1, 0);
						}
						else
						{
							PUSHCODE(Bagel_BC::BC_LOADCONST, pos, b, d1, 0);
						}
						dest = b;
					}
					if (!a || (a == OP_SETSET - OP_SET && (*typeinfo)[b] == VAR_NONE))
					{
						REG_TYPE(b, (*typeinfo)[d1]);
					}
					else if (a != OP_SETSET - OP_SET && a != OP_SETADD - OP_SET)
					{
						//其余运算都返回数值
						REG_TYPE(b, VAR_NUM);
					}
				}
			}
			else
			{
				b = getConstPos(code, subtree->childs[0]->Node.var);
				PUSHCACHE(pos);
				PUSHCODE(Bagel_BC::BC_GETCLOMEMBER_PTR, pos, 0, b, dest + 1);
				PUSHCODE(Bagel_BC::BC_PTR_SET + a + e, pos, dest + 1, d1, d);
				if (a)
				{
					//为了实现yield得把property的计算拆开
					PUSHCODE(Bagel_BC::BC_LOADPOINTER2, pos, dest + 2, dest + 1, 0);
					PUSHCODE(Bagel_BC::BC_MOV + a + e, pos, dest + 2, d1, d);
					PUSHCODE(Bagel_BC::BC_SETPOINTER2, pos, dest + 1, dest + 2, 0);
				}
				REG_TYPE(dest, VAR_UNKNOWN);//不能确定property的情况
				if (need_return && !d)
				{
					PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, dest + 1, 0);
					//REG_TYPE(dest, VAR_UNKNOWN);
				}
			}
			ENSURE(dest + 2);
			return dest;
		case OP_DOT + OP_COUNT:
			b = getConstPos(code, subtree->childs[0]->childs[0]->Node.var);
			PUSHCACHE(pos);
			if (curclo->withpos >= 0)
			{
				PUSHCODE(Bagel_BC::BC_GETMEMBERSTRIDX_PTR, pos, curclo->withpos, b, dest + 1);
			}
			else
			{
				PUSHCODE(Bagel_BC::BC_GETGLOBALMEMBER_PTR, pos, b, 0, dest + 1);
			}
			//需要多次用到self，存在obj内
			if (need_return)
			{
				PUSHCODE(Bagel_BC::BC_MERGESELF, pos, dest + 1, 0, 0);
			}
			PUSHCODE(Bagel_BC::BC_PTR_SET + a + e, pos, dest + 1, d1, d);
			if (a)
			{
				//为了实现yield得把property的计算拆开
				PUSHCODE(Bagel_BC::BC_LOADPOINTER2, pos, dest + 2, dest + 1, 0);
				PUSHCODE(Bagel_BC::BC_MOV + a + e, pos, dest + 2, d1, d);
				PUSHCODE(Bagel_BC::BC_SETPOINTER2, pos, dest + 1, dest + 2, 0);
			}
			REG_TYPE(dest, VAR_UNKNOWN);//不能确定property的情况
			if (need_return && !d)
			{
				PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, dest + 1, 0);
			}
			ENSURE(dest + 2);
			break;
		case OP_DOT:
			b = getConstPos(code, subtree->childs[0]->childs[1]->Node.var);
			if (subtree->childs[0]->childs[0]->Node.opcode == OP_SUPER + OP_COUNT)
			{
				PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->childs[1]->Node.pos, dest + 1, b, 0);
				PUSHCODE(Bagel_BC::BC_LOADSUPERMEMBER_PTR, pos, 0, dest + 1, dest + 2);
				//需要多次用到self，存在obj内
				if (need_return)
				{
					PUSHCODE(Bagel_BC::BC_MERGESELF, pos, dest + 1, 0, 0);
				}
				PUSHCODE(Bagel_BC::BC_PTR_SET + a + e, pos, dest + 2, c, d);
				if (a)
				{
					//为了实现yield得把property的计算拆开
					PUSHCODE(Bagel_BC::BC_LOADPOINTER2, pos, dest + 3, dest + 2, 0);
					PUSHCODE(Bagel_BC::BC_MOV + a + e, pos, dest + 3, d1, d);
					PUSHCODE(Bagel_BC::BC_SETPOINTER2, pos, dest + 2, dest + 3, 0);
				}
				if (need_return && !d)
				{
					PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, dest + 2, 0);
				}
			}
			else
			{
				d2 = _compile(dest + 2, subtree->childs[0]->childs[0], code, true, jit);
				PUSHCACHE(pos);
				PUSHCODE(Bagel_BC::BC_GETMEMBERSTRIDX_PTR, pos, d2, b, dest + 1);
				//需要多次用到self，存在obj内
				if (need_return)
				{
					PUSHCODE(Bagel_BC::BC_MERGESELF, pos, dest + 1, 0, 0);
				}
				PUSHCODE(Bagel_BC::BC_PTR_SET + a + e, pos, dest + 1, d1, d);
				if (a)
				{
					//为了实现yield得把property的计算拆开
					PUSHCODE(Bagel_BC::BC_LOADPOINTER2, pos, dest + 2, dest + 1, 0);
					PUSHCODE(Bagel_BC::BC_MOV + a + e, pos, dest + 2, d1, d);
					PUSHCODE(Bagel_BC::BC_SETPOINTER2, pos, dest + 1, dest + 2, 0);
				}
				if (need_return && !d)
				{
					PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, dest + 1, 0);
				}
			}
			REG_TYPE(dest, VAR_UNKNOWN);
			ENSURE(dest + 3);
			break;
		case OP_ARRAY:
			d2 = _compile(dest + 1, subtree->childs[0]->childs[0], code, true, jit);
			if (subtree->childs[0]->childs[1]->Node.opcode == OP_CONSTVAR + OP_COUNT)
			{
				auto &v = subtree->childs[0]->childs[1]->Node.var;
				if ((*typeinfo)[d2] == VAR_ARRAY && v.getType() == VAR_NUM)
				{
					PUSHCODE(Bagel_BC::BC_GETARRAYIDX2_PTR, pos, d2, (int32_t)v.asInteger(), dest);
				}
				else if (v.isInteger())
				{
					PUSHCODE(Bagel_BC::BC_GETMEMBERIDX_PTR, pos, d2, (int32_t)v.asInteger(), dest);
				}
				else if (v.getType() == VAR_STR)
				{
					b = getConstPos(code, v.forceAsBKEStr());
					PUSHCACHE(pos);
					PUSHCODE(Bagel_BC::BC_GETMEMBERSTRIDX_PTR, pos, d2, b, dest);
				}
				else
				{
					d3 = getConstPos(code, v);
					PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 2, d3, 0);
					PUSHCODE(Bagel_BC::BC_GETMEMBER_PTR, pos, d2, dest + 2, dest);
				}
			}
			else
			{
				d3 = _compile(dest + 2, subtree->childs[0]->childs[1], code, true, jit);
				if (/*(*typeinfo)[d2] == VAR_ARRAY && */(*typeinfo)[d3] == VAR_NUM)
				{
					PUSHCODE(Bagel_BC::BC_GETARRAYIDX_PTR, pos, d2, d3, dest);
				}
				else
				{
					PUSHCODE(Bagel_BC::BC_GETMEMBER_PTR, pos, d2, d3, dest);
				}
			}
			//需要多次用到self，存在obj内
			if (need_return)
			{
				PUSHCODE(Bagel_BC::BC_MERGESELF, pos, dest + 1, 0, 0);
			}
			PUSHCODE(Bagel_BC::BC_PTR_SET + a + e, pos, dest, d1, d);
			if (a)
			{
				//为了实现yield得把property的计算拆开
				PUSHCODE(Bagel_BC::BC_LOADPOINTER2, pos, dest + 1, dest, 0);
				PUSHCODE(Bagel_BC::BC_MOV + a + e, pos, dest + 1, d1, d);
				PUSHCODE(Bagel_BC::BC_SETPOINTER2, pos, dest, dest + 1, 0);
			}
			if (need_return && !d)
			{
				PUSHCODE(Bagel_BC::BC_LOADPOINTER, pos, dest, dest, 0);
			}
			REG_TYPE(dest, VAR_UNKNOWN);
			ENSURE(dest + 2);
			break;
		}
		break;
	case OP_BRACKET:
		d1 = _compile(dest, subtree->childs[0], code, true, jit);
		//PUSHCODE(Bagel_BC::BC_PRECALL, pos, d1, 0, 0);
		if (d1 != dest)
		{
			PUSHCODE(Bagel_BC::BC_MOV, pos, dest, d1, 0);
		}
		b = 0;	//has ...?
		for (a = 1; a < subtree->childs.size(); a++)
		{
			if (subtree->childs[a] && subtree->childs[a]->Node.opcode == OP_EXTRACTARGS)
			{
				b = 1;
				break;
			}
		}
		if (!b)
		{
			a = 1;
			while (a < subtree->childs.size())
			{
				d1 = _compile(dest + a, subtree->childs[a], code, true, jit);
				if (d1 != dest + a)
				{
					PUSHCODE(Bagel_BC::BC_MOV, pos, dest + a, d1, 0);
				}
				a++;
			}
			//PUSHCODE(Bagel_BC::BC_MAKEARRAY, pos, dest + 1, dest + 1, a - 1);
			PUSHCODE(Bagel_BC::BC_CALL, pos, dest, dest + 1, a - 1);
			if (code.needSpecialStack)
			{
				for (auto &it : beFixed)
				{
					REG_TYPE(it, VAR_UNKNOWN);
				}
			}
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
					d1 = _compile(dest + 2, subtree->childs[a]->childs[0], code, true, jit);
					PUSHCODE(Bagel_BC::BC_VECTORCONCAT, pos, dest + 1, d1, 0);
				}
				else
				{
					d1 = _compile(dest + c, subtree->childs[a], code, true, jit);
					if (d1 != dest + c)
					{
						PUSHCODE(Bagel_BC::BC_MOV, pos, dest + c, d1, 0);
					}
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
			d1 = _compile(dest, subtree->childs[1], code, true, jit);
			PUSHCODE(Bagel_BC::BC_LOADSUPERMEMBER_VALUE, pos, 0, d1, dest);
		}
		else
		{
			d1 = _compile(dest, subtree->childs[0], code, true, jit);
			if (subtree->childs.size() == 2)//Parser保证此时child[1]不为NULL
			{
				if (subtree->childs[1]->Node.opcode == OP_CONSTVAR + OP_COUNT)
				{
					auto &v = subtree->childs[1]->Node.var;
					if (((*typeinfo)[d1] == VAR_ARRAY || (*typeinfo)[d1] == VAR_STR) && v.getType() == VAR_NUM)
					{
						PUSHCODE(Bagel_BC::BC_LOADARRAYIDX2_VALUE, pos, d1, (int32_t)v.asInteger(), dest);
					}
					else if (v.isInteger())
					{
						PUSHCODE(Bagel_BC::BC_LOADMEMBERIDX_VALUE, pos, d1, (int32_t)v.asInteger(), dest);
					}
					else if (v.getType() == VAR_STR)
					{
						a = getConstPos(code, v.forceAsBKEStr());
						PUSHCACHE(pos);
						PUSHCODE(Bagel_BC::BC_LOADMEMBERSTRIDX_VALUE, pos, d1, a, dest);
					}
					else
					{
						d2 = getConstPos(code, v);
						PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 1, d2, 0);
						PUSHCODE(Bagel_BC::BC_LOADMEMBER_VALUE, pos, d1, dest + 1, dest);
					}
				}
				else
				{
					d2 = _compile(dest + 1, subtree->childs[1], code, true, jit);
					if (/*((*typeinfo)[d1] == VAR_ARRAY || (*typeinfo)[d1] == VAR_STR) && */(*typeinfo)[d2] == VAR_NUM)
					{
						PUSHCODE(Bagel_BC::BC_LOADARRAYIDX_VALUE, pos, d1, d2, dest);
					}
					else
					{
						PUSHCODE(Bagel_BC::BC_LOADMEMBER_VALUE, pos, d1, d2, dest);
					}
				}
				break;
			}
			if (subtree->childs[1])
			{
				d2 = _compile(dest + 1, subtree->childs[1], code, true, jit);
				if (d2 != dest + 1)
				{
					PUSHCODE(Bagel_BC::BC_MOV, pos, dest + 1, d2, 0);
				}
			}
			else
			{
				PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest + 1, 0, 0);
			}
			if (subtree->childs.size() > 2 && subtree->childs[2])
			{
				d2 = _compile(dest + 2, subtree->childs[2], code, true, jit);
				if (d2 != dest + 2)
				{
					PUSHCODE(Bagel_BC::BC_MOV, pos, dest + 2, d2, 0);
				}
			}
			else
			{
				PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest + 2, 0, 0);
			}
			if (subtree->childs.size() > 3 && subtree->childs[3])
			{
				d2 = _compile(dest + 3, subtree->childs[3], code, true, jit);
				if (d2 != dest + 3)
				{
					PUSHCODE(Bagel_BC::BC_MOV, pos, dest + 3, d2, 0);
				}
			}
			else
			{
				PUSHCODE2(Bagel_BC::BC_LOADNUM, pos, dest + 3, 1.0);
			}
			PUSHCODE(Bagel_BC::BC_ELEMENT, pos, dest, d1, dest + 1);
			REG_TYPE(dest, VAR_UNKNOWN);
			ENSURE(dest + 3);
		}
		break;
	case OP_ARRAY + OP_COUNT:
		a = -1;
		b = subtree->childs.size();
		while (++a < b)
		{
			d1 = _compile(dest + a, subtree->childs[a], code, true, jit);
			if (d1 != dest + a)
			{
				PUSHCODE(Bagel_BC::BC_MOV, pos, dest + a, d1, 0);
			}
		}
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_MAKEARRAY, pos, dest, dest, a);
			REG_TYPE(dest, VAR_ARRAY);
		}
		break;
	case OP_DIC + OP_COUNT:
		a = 0;
		b = subtree->childs.size();
		while (a < b)
		{
			d1 = _compile(dest + a, subtree->childs[a], code, true, jit);
			if (d1 != dest + a)
			{
				PUSHCODE(Bagel_BC::BC_MOV, pos, dest + a, d1, 0);
			}
			d1 = _compile(dest + a + 1, subtree->childs[a + 1], code, true, jit);
			if (d1 != dest + a + 1)
			{
				PUSHCODE(Bagel_BC::BC_MOV, pos, dest + a + 1, d1, 0);
			}
			a += 2;
		}
		PUSHCODE(Bagel_BC::BC_MAKEDIC, pos, dest, dest, a);
		REG_TYPE(dest, VAR_DIC);
		break;
	case OP_NULLARRAY + OP_COUNT:
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_MAKEARRAY, pos, dest, dest, 0);
			REG_TYPE(dest, VAR_ARRAY);
		}
		break;
	case OP_NULLDIC + OP_COUNT:
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_MAKEDIC, pos, dest, dest, 0);
			REG_TYPE(dest, VAR_DIC);
		}
		break;
	case OP_BLOCK + OP_COUNT:
		NEW_CLO(curclo->breakcode, curclo->continuecode);
		a = -1;
		b = subtree->childs.size();
		while (++a < b)
		{
			_compile(dest, subtree->childs[a], code, false, jit);
		}
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, subtree->Node.pos2, dest, 0, 0);
			REG_TYPE(dest, VAR_NONE);
		}
		END_CLO(subtree);
		break;
	case OP_DOT:
		a = getConstPos(code, subtree->childs[1]->Node.var);
		switch (subtree->childs[0]->Node.opcode - OP_COUNT)
		{
		case OP_SUPER:
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[1]->Node.pos, dest, a, 0);
			PUSHCODE(Bagel_BC::BC_LOADSUPERMEMBER_VALUE, pos, 0, dest, dest);
			break;
		case OP_GLOBAL:
			PUSHCACHE(pos);
			PUSHCODE(Bagel_BC::BC_LOADGLOBALMEMBER_VALUE, pos, a, 0, dest);
			break;
		default:
			d1 = _compile(dest + 1, subtree->childs[0], code, true, jit);
			PUSHCACHE(pos);
			PUSHCODE(Bagel_BC::BC_LOADMEMBERSTRIDX_VALUE, pos, d1, a, dest);
			break;
		}
		REG_TYPE(dest, VAR_UNKNOWN);
		break;
	case OP_DOT + OP_COUNT:
		a = getConstPos(code, subtree->childs[0]->Node.var);
		PUSHCACHE(pos);
		if (curclo->withpos < 0)
		{
			PUSHCODE(Bagel_BC::BC_LOADGLOBALMEMBER_VALUE, pos, a, 0, dest);
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_LOADMEMBERSTRIDX_VALUE, pos, curclo->withpos, a, dest);
		}
		REG_TYPE(dest, VAR_UNKNOWN);
		break;
	case OP_CONTINUE + OP_COUNT:
		if (curclo->continuecode)
		{
			PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			if (lockcode <= 0)
				curclo->continuecode->push_back(code.code.size() - 1);
			if (lastlooplevel > 0)
			{
				//change to normal
				auto it = typeinfos.begin();
				int i = lastlooplevel;
				while (i-- > 0)
				{
					++it;
				}
				auto it2 = it;
				--it2;
				*it2 = *it;
			}
		}
		else if (lockcode <= 0)
		{
			a = getConstPos(code, W("此处不应出现continue"));
			PUSHCODE(Bagel_BC::BC_THROWCONST, pos, a, 0, 0);
		}
		break;
	case OP_BREAK + OP_COUNT:
		if (curclo->breakcode)
		{
			PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			if (lockcode <= 0)
				curclo->breakcode->push_back(code.code.size() - 1);
			if (lastlooplevel > 0)
			{
				//change to normal
				auto it = typeinfos.begin();
				int i = lastlooplevel;
				while (i-- > 0)
				{
					++it;
				}
				auto it2 = it;
				--it2;
				*it2 = *it;
			}
		}
		else if (lockcode <= 0)
		{
			a = getConstPos(code, W("此处不应出现break"));
			PUSHCODE(Bagel_BC::BC_THROWCONST, pos, a, 0, 0);
		}
		break;
	case OP_RETURN + OP_COUNT:
		a = trycount;
		if (a > 0)
		{
			PUSHCODE(Bagel_BC::BC_REMOVETRYHOOKN, pos, a, 0, 0);
		}
		if (subtree->childs[0])
		{
			d1 = _compile(dest, subtree->childs[0], code, true, jit);
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
			d1 = dest;
		}
		PUSHCODE(Bagel_BC::BC_RETURN, pos, d1, 0, 0);
		break;
	case OP_YIELD + OP_COUNT:
		a = trycount;
		if (a > 0)
		{
			PUSHCODE(Bagel_BC::BC_REMOVETRYHOOKN, pos, a, 0, 0);
		}
		if (subtree->childs[0])
		{
			d1 = _compile(dest, subtree->childs[0], code, true, jit);
		}
		else
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
			d1 = dest;
		}
		PUSHCODE(Bagel_BC::BC_YIELD, pos, d1, dest, 0);
		break;
	case OP_VAR + OP_COUNT:
		if (inclass || (inglobal && clos.size() == 1))
		{
			for (int i = 0; i < subtree->childs.size(); i += 2)
			{
				a = getConstPos(code, subtree->childs[i]->Node.var);
				PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[i]->Node.pos, dest, a, 0);
				d1 = _compile(dest + 1, subtree->childs[i + 1], code, true, jit);
				if (inclass)
				{
					PUSHCODE(Bagel_BC::BC_ADDVAR, subtree->childs[i]->Node.pos, inclass, dest, d1);
				}
				else
				{
					PUSHCODE(Bagel_BC::BC_SETMEMBER2, subtree->childs[i]->Node.pos, 0, dest, d1);
				}
			}
		}
		else
		{
			for (int i = 0; i < subtree->childs.size(); i += 2)
			{
				REG_VAR(curclo, subtree->childs[i]->Node.var, subtree->childs[i]->Node.pos, nextlocalpos);
				int varpos = nextlocalpos;
				nextlocalpos--;
				if (!subtree->childs[i + 1] || (subtree->childs[i + 1]->Node.opcode == OP_CONSTVAR + OP_COUNT && subtree->childs[i + 1]->Node.var.isVoid()))
				{
					PUSHCODE(Bagel_BC::BC_LOADVOID, subtree->childs[i]->Node.pos, varpos, 0, 0);
					REG_TYPE(varpos, VAR_NONE);
				}
				else if (subtree->childs[i + 1]->Node.opcode != OP_CONSTVAR + OP_COUNT || subtree->childs[i + 1]->Node.var.getType() > VAR_DIC)
				{
					d1 = _compile(dest, subtree->childs[i + 1], code, true, jit);
					PUSHCODE(Bagel_BC::BC_MOV, subtree->childs[i]->Node.pos, varpos, d1, 0);
					REG_TYPE(varpos, (*typeinfo)[d1]);
				}
				else if (subtree->childs[i + 1]->Node.var.getType() < VAR_ARRAY)
				{
					d1 = getConstPos(code, subtree->childs[i + 1]->Node.var);
					PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[i]->Node.pos, varpos, d1, 0);
					REG_TYPE(varpos, subtree->childs[i + 1]->Node.var.getType());
				}
				else
				{
					d1 = getConstPos(code, subtree->childs[i + 1]->Node.var);
					PUSHCODE(Bagel_BC::BC_COPYCONST, subtree->childs[i]->Node.pos, varpos, d1, 0);
					REG_TYPE(varpos, subtree->childs[i + 1]->Node.var.getType());
				}
			}
		}
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
			REG_TYPE(dest, VAR_NONE);
		}
		break;
	case OP_NEW + OP_COUNT:
		a = getConstPos(code, subtree->childs[0]->Node.var);
		PUSHCACHE(pos);
		PUSHCODE(Bagel_BC::BC_LOADGLOBALMEMBER_VALUE, pos, a, 0, dest);
		PUSHCODE(Bagel_BC::BC_CHECKTYPE, pos, dest, VAR_CLASSDEF, 0);
		REG_TYPE(dest, VAR_CLASS);
		break;
	case OP_DELETE + OP_COUNT:
		if (subtree->childs[0]->Node.opcode == OP_DOT)
		{
			d1 = _compile(dest, subtree->childs[0]->childs[0], code, true, jit);
			a = getConstPos(code, subtree->childs[0]->childs[1]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->childs[1]->Node.pos, dest + 1, a, 0);
			PUSHCODE(Bagel_BC::BC_DELETE, pos, d1, dest + 1, 0);
		}
		else if (subtree->childs[0]->Node.opcode == OP_DOT + OP_COUNT)
		{
			PUSHCODE(Bagel_BC::BC_LOADGLOBAL, pos, dest, a, 0);
			a = getConstPos(code, subtree->childs[0]->childs[1]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->childs[1]->Node.pos, dest + 1, a, 0);
			PUSHCODE(Bagel_BC::BC_DELETE, pos, dest, dest + 1, 0);
		}
		else
		{
			d1 = _compile(dest, subtree->childs[0]->childs[0], code, true, jit);
			d2 = _compile(dest + 1, subtree->childs[0]->childs[1], code, true, jit);
			PUSHCODE(Bagel_BC::BC_DELETE, pos, d1, d2, 0);
		}
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
			REG_TYPE(dest, VAR_NONE);
		}
		ENSURE(dest + 1);
		break;
	case OP_TRY + OP_COUNT:
		{
			generateTypeLevel();//try里随时都可能发生异常，而且由于catch的存在后续的代码还会被执行。所以try块的代码是否执行是不确定的
			vector<int> breakcode;
			vector<int> continuecode;
			auto rawclo = curclo;
			PUSHCODE(Bagel_BC::BC_ADDTRYHOOK, pos, 0, dest, 0);
			a = code.code.size();
			NEW_CLO((curclo->breakcode ? &breakcode : NULL), (curclo->continuecode ? &continuecode : NULL));
			trycount++;
			_compile(dest + 1, subtree->childs[0], code, false, jit);
			trycount--;
			END_CLO(subtree->childs[0]);
			deleteTypeLevel();
			//jump out
			PUSHCODE(Bagel_BC::BC_REMOVETRYHOOK, pos, 0, 0, 0);
			PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			b = code.code.size();
			//handle break
			if (rawclo->breakcode && !breakcode.empty() && lockcode <= 0)
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
			if (rawclo->continuecode && !continuecode.empty() && lockcode <= 0)
			{
				for (auto &it : continuecode)
				{
					code.code[it].A = code.code.size();
				}
				PUSHCODE(Bagel_BC::BC_REMOVETRYHOOK, pos, 0, 0, 0);
				rawclo->continuecode->push_back(code.code.size());
				PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			}
			if (lockcode <= 0)
				code.code[a - 1].A = b;
			generateTypeLevel();
			NEW_CLO(curclo->breakcode, curclo->continuecode);
			if (subtree->childs[1])
			{
				PUSHCODE(Bagel_BC::BC_MOV, subtree->childs[1]->Node.pos, nextlocalpos, dest, 0);
				REG_TYPE(dest, VAR_UNKNOWN);
				REG_TYPE(nextlocalpos, VAR_UNKNOWN);
				REG_VAR(curclo, subtree->childs[1]->Node.var, subtree->childs[1]->Node.pos, nextlocalpos);
				--nextlocalpos;
			}
			_compile(dest + 2, subtree->childs[2], code, false, jit);
			END_CLO(subtree->childs[1]);
			deleteTypeLevel();
			if (lockcode <= 0)
				code.code[b - 1].A = code.code.size();
			if (need_return)
			{
				PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
				REG_TYPE(dest, VAR_NONE);
			}
		}
		break;
	case OP_THROW + OP_COUNT:
		d1 = _compile(dest, subtree->childs[0], code, true, jit);
		PUSHCODE(Bagel_BC::BC_THROW, pos, d1, 0, 0);
		break;
	case OP_THIS + OP_COUNT:
		PUSHCODE(Bagel_BC::BC_CHECKTHIS, pos, 0, 0, 0);
		return 1;//this固定在1位置
	case OP_SUPER + OP_COUNT:
		a = getConstPos(code, W("此处不应出现super"));
		PUSHCODE(Bagel_BC::BC_THROWCONST, pos, a, 0, 0);
		break;
	case OP_GLOBAL + OP_COUNT:
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_LOADGLOBAL, pos, dest, 0, 0);
			REG_TYPE(dest, VAR_CLO);
		}
		break;
	case OP_TOINT + OP_COUNT:
		d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_TOINT, pos, dest, d1, 0);
			REG_TYPE(dest, VAR_NUM);
		}
		break;
	case OP_TOSTRING + OP_COUNT:
		d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_TOSTRING, pos, dest, d1, 0);
			REG_TYPE(dest, VAR_STR);
		}
		break;
	case OP_TONUMBER + OP_COUNT:
		d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_TONUMBER, pos, dest, d1, 0);
			REG_TYPE(dest, VAR_NUM);
		}
		break;
	case OP_CONST + OP_COUNT:
		_compile(dest, subtree->childs[0], code, need_return, jit);
		break;
	case OP_TYPE + OP_COUNT:
		d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_TYPE, pos, dest, d1, 0);
			REG_TYPE(dest, VAR_STR);
		}
		break;
	case OP_INSTANCE:
		d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
		d2 = _compile(dest + 1, subtree->childs[1], code, need_return, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_INSTANCE, pos, dest, d1, d2);
			REG_TYPE(dest, VAR_NUM);
		}
		break;
	case OP_SWITCH + OP_COUNT:
		{
			vector<int> jumptable;
			vector<int> breakcode;
			d1 = _compile(dest, subtree->childs[0], code, true, jit);
			generateTypeLevel();
			NEW_CLO(&breakcode, curclo->continuecode);
			for (auto &it : subtree->childs[1]->childs)
			{
				if (it->Node.opcode == OP_CASE + OP_COUNT)
				{
					d2 = _compile(dest + 1, it->childs[0], code, true, jit);
					jumptable.push_back(code.code.size());
					PUSHCODE(Bagel_BC::BC_JUMPEEQUAL, it->Node.pos, 0, d1, d2);
				}
			}
			//default pos
			b = code.code.size();
			PUSHCODE(Bagel_BC::BC_JUMP, pos, 0, 0, 0);
			deleteTypeLevel();
			//has default
			c = 0;
			a = 0;
			bool begin = false;
			for (auto &it : subtree->childs[1]->childs)
			{
				if (it->Node.opcode == OP_DEFAULT + OP_COUNT)
				{
					if (lockcode <= 0)
						code.code[b].A = code.code.size();
					c = 1;
					if (begin)
						deleteTypeLevel();
					generateTypeLevel();
					begin = true;
					continue;
				}
				else if (it->Node.opcode == OP_CASE + OP_COUNT)
				{
					if (lockcode <= 0)
						code.code[jumptable[a]].A = code.code.size();
					a++;
					if (begin)
						deleteTypeLevel();
					generateTypeLevel();
					begin = true;
					continue;
				}
				else if (begin)
				{
					_compile(dest, it, code, false, jit);
				}
			}
			if (begin)
				deleteTypeLevel();
			if (!c)
			{
				if (lockcode <= 0)
					code.code[b].A = code.code.size();
			}
			//handle breaks
			if (lockcode <= 0)
			{
				for (auto &it : breakcode)
				{
					code.code[it].A = code.code.size();
				}
			}
			END_CLO(subtree->childs[1]);
			if (need_return)
			{
				PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
				REG_TYPE(dest, VAR_NONE);
			}
		}
		break;
	case OP_CASE + OP_COUNT:
		a = getConstPos(code, W("此处不应出现case"));
		PUSHCODE(Bagel_BC::BC_THROWCONST, pos, a, 0, 0);
		break;
	case OP_DEFAULT + OP_COUNT:
		a = getConstPos(code, W("此处不应出现default"));
		PUSHCODE(Bagel_BC::BC_THROWCONST, pos, a, 0, 0);
		break;
	case OP_RESERVE + OP_COUNT:
		a = -1;
		b = subtree->childs.size();
		d1 = dest;
		while (++a < b)
		{
			d1 = _compile(dest, subtree->childs[a], code, a == b - 1, jit);
		}
		return d1;
		//if (need_return)
		//{
		//	PUSHCODE(Bagel_BC::BC_LOADVOID, MINUS_POS, dest, 0, 0);
		//	REG_TYPE(dest, VAR_NONE);
		//}
		break;
	case OP_RESERVE2 + OP_COUNT:
		a = -1;
		b = subtree->childs.size();
		d1 = dest;
		while (++a < b)
		{
			d1 = _compile(dest, subtree->childs[a], code, a == b - 1, jit);
		}
		return d1;
	case OP_WITH + OP_COUNT:
		d1 = _compile(dest, subtree->childs[0], code, true, jit);
		NEW_CLO(curclo->breakcode, curclo->continuecode);
		curclo->withpos = d1;
		_compile(dest + 1, subtree->childs[1], code, false, jit);
		END_CLO(subtree->childs[1]);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
			REG_TYPE(dest, VAR_NONE);
		}
		break;
	case OP_INCONTEXTOF:
		d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
		d2 = _compile(dest + 1, subtree->childs[1], code, need_return, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_INCONTEXTOF, pos, dest, d1, d2);
			REG_TYPE(dest, VAR_FUNC);
		}
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
			a = getConstPos(code, subtree->childs[i]->Node.var);
			PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[i]->Node.pos, dest, a, 0);
			d1 = _compile(dest + 1, subtree->childs[i + 1], code, true, jit);
			PUSHCODE(Bagel_BC::BC_ADDSTATIC, subtree->childs[i]->Node.pos, inclass, dest, d1);
		}
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
			REG_TYPE(dest, VAR_NONE);
		}
		break;
	case OP_ENUM + OP_COUNT:
		if (!subtree->childs[0]->Node.var.isVoid())
		{
			for (int i = 1; i < subtree->childs.size(); i += 2)
			{
				a = getConstPos(code, subtree->childs[i]->Node.var);
				PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[i]->Node.pos, dest + i, a, 0);
				PUSHCODE2(Bagel_BC::BC_LOADNUM, pos, dest + i + 1, subtree->childs[i + 1]->Node.var.forceAsNumber());
			}
			if (!(inglobal && clos.size() == 1) && !inclass)
			{
				REG_VAR(curclo, subtree->childs[0]->Node.var, subtree->childs[0]->Node.pos, nextlocalpos);
				PUSHCODE(Bagel_BC::BC_MAKEDIC, pos, nextlocalpos, dest + 1, subtree->childs.size());
				--nextlocalpos;
			}
			else
			{
				PUSHCODE(Bagel_BC::BC_MAKEDIC, pos, dest, dest + 1, subtree->childs.size());
				a = getConstPos(code, subtree->childs[0]->Node.var);
				PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 1, a, 0);
				PUSHCODE(Bagel_BC::BC_SETMEMBER2, pos, inclass, dest + 1, dest);
			}
			ENSURE(dest + subtree->childs.size());
		}
		else
		{
			if ((inglobal && clos.size() == 1) || inclass)
			{
				for (int i = 1; i < subtree->childs.size(); i += 2)
				{
					a = getConstPos(code, subtree->childs[i]->Node.var);
					PUSHCODE(Bagel_BC::BC_LOADCONST, pos, dest + 1, a, 0);
					PUSHCODE2(Bagel_BC::BC_LOADNUM, pos, dest, subtree->childs[i + 1]->Node.var.forceAsNumber());
					PUSHCODE(Bagel_BC::BC_SETMEMBER2, subtree->childs[i]->Node.pos, inclass, dest + 1, dest);
				}
			}
			else
			{
				for (int i = 1; i < subtree->childs.size(); i += 2)
				{
					REG_VAR(curclo, subtree->childs[i]->Node.var, subtree->childs[i]->Node.pos, nextlocalpos)
					PUSHCODE2(Bagel_BC::BC_LOADNUM, pos, dest, subtree->childs[i + 1]->Node.var.forceAsNumber());
					PUSHCODE(Bagel_BC::BC_MOV, subtree->childs[i]->Node.pos, nextlocalpos, dest, 0);
					nextlocalpos--;
				}
			}
			ENSURE(dest + 1);
		}
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_LOADVOID, pos, dest, 0, 0);
			REG_TYPE(dest, VAR_NONE);
		}
		break;
	case OP_BITAND:
	case OP_BITOR:
	case OP_BITXOR:
		d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
		d2 = _compile(dest + 1, subtree->childs[1], code, need_return, jit);
		if (need_return)
		{
			PUSHCODE(subtree->Node.opcode - OP_BITAND + Bagel_BC::BC_BITAND, pos, dest, d1, d2);
			REG_TYPE(dest, VAR_NUM);
		}
		break;
	case OP_BITNOT + OP_COUNT:
		d1 = _compile(dest, subtree->childs[0], code, need_return, jit);
		if (need_return)
		{
			PUSHCODE(Bagel_BC::BC_BITNOT, pos, dest, d1, 0);
			REG_TYPE(dest, VAR_NUM);
		}
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
	return dest;
}

Bagel_ReleaseCompiler::Bagel_ReleaseCompiler()
{
	inclass = 0;
	inglobal = true;
	parent = NULL;
	nextlocalpos = -1;
	startStack = 2;
	clos.emplace_back();
	curclo = &clos.back();
	curclo->parent = NULL;
	curclo->withpos = -1;
	curclo->breakcode = NULL;
	curclo->continuecode = NULL;
	//curclo->switchtable = NULL;
	typelevel = 0;
	trycount = 0;
	curcode = NULL;
	typeinfos.emplace_back();
	typeinfo = &typeinfos.back();
	typeinfo->set(0, VAR_CLO);
	typeinfo->set(1, VAR_UNKNOWN);
	lastlooplevel = -1;
	infirstloop = 0;
	lockcode = 0;
}

Bagel_ReleaseCompiler::~Bagel_ReleaseCompiler()
{
}

void Bagel_ReleaseCompiler::compile(Bagel_AST * tree, Bagel_ByteCode & code, const StringVal & exp, bool jit, bool varPosInfo)
{
	if (varPosInfo)
		code.varPosInfo.reset(new Bagel_VarPosInfo());
	if (!exp.empty())
	{
		code.debugInfo.reset(new Bagel_DebugInformation(exp));
	}
	curcode = &code;
	if (parent)
	{
		//insert code jump to link code
		PUSHCODE(Bagel_BC::BC_JUMP, -1, 0, 0, 0);
	}
	int d1 = _compile(startStack, tree, code, true, jit || !inglobal);
	PUSHCODE(Bagel_BC::BC_RETURN, MINUS_POS, d1, 0, 0);
	code.release = true;
	code.localDepth = -nextlocalpos - 1;
	if (parent)
	{
		if (linkCode.empty())
		{
			code.code[0].A = 1;
		}
		else
		{
			code.code[0].A = code.code.size();
			code.code.insert(code.code.end(), linkCode.begin(), linkCode.end());
			PUSHCODE(Bagel_BC::BC_JUMP, -1, 1, 0, 0);
		}
	}
	//dumpByteCode(code);
}

void Bagel_ReleaseCompiler::compileAddr(Bagel_AST * tree, Bagel_ByteCode & code, const StringVal & exp, bool jit, bool varPosInfo)
{
	if (varPosInfo)
		code.varPosInfo.reset(new Bagel_VarPosInfo());
	if (!exp.empty())
	{
		code.debugInfo.reset(new Bagel_DebugInformation(exp));
	}
	curcode = &code;
	if (parent)
	{
		//insert code jump to link code
		PUSHCODE(Bagel_BC::BC_JUMP, -1, 0, 0, 0);
	}
	inclass = 0;
	auto subtree = tree;
	auto dest = 1;
	auto pos = tree->Node.pos;
	int a, b, c;
	switch (tree->Node.opcode)
	{
	case OP_LITERAL + OP_COUNT:
		c = getConstPos(code, subtree->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->Node.pos, dest + 1, c, 0);
		a = 0;
		b = dest + 1;
		break;
	case OP_DOT + OP_COUNT:
		c = getConstPos(code, subtree->childs[0]->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[0]->Node.pos, dest + 1, c, 0);
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
		c = getConstPos(code, subtree->childs[1]->Node.var);
		PUSHCODE(Bagel_BC::BC_LOADCONST, subtree->childs[1]->Node.pos, dest + 1, c, 0);
		a = _compile(dest + 2, subtree->childs[0], code, true, jit);
		b = dest + 1;
		break;
	case OP_ARRAY:
		a = _compile(dest + 1, subtree->childs[0], code, true, jit);
		b = _compile(dest + 2, subtree->childs[1], code, true, jit);
		break;
	}
	PUSHCODE(Bagel_BC::BC_CREATEPOINTER, MINUS_POS, dest, a, b);
	PUSHCODE(Bagel_BC::BC_RETURN, MINUS_POS, dest, 0, 0);
	if (parent)
	{
		if (linkCode.empty())
		{
			code.code[0].A = 1;
		}
		else
		{
			code.code[0].A = code.code.size();
			code.code.insert(code.code.end(), linkCode.begin(), linkCode.end());
			PUSHCODE(Bagel_BC::BC_JUMP, -1, 1, 0, 0);
		}
	}
}

void CFAAnalysis::handleOne(int idx)
{
	auto &c = code->code[idx];
	//先判断跳转
	switch (c.opcode)
	{
	case Bagel_BC::BC_JUMP:
		if (table[c.A] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = c.A;
			r->in = cur->out;
			r->catchblock = cur->catchblock;
			table[c.A] = r;
		}
		if (table[idx + 1] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = idx + 1;
			r->in = cur->out;
			r->catchblock = cur->catchblock;
			table[idx + 1] = r;
		}
		cur->end = idx;
		cur->nexts[0] = table[c.A];
		break;
	case Bagel_BC::BC_JUMPFALSE:
	case Bagel_BC::BC_JUMPTRUE:
	case Bagel_BC::BC_JUMPVOID:
	case Bagel_BC::BC_JUMPNOTVOID:
	case Bagel_BC::BC_JUMPEQUAL:
	case Bagel_BC::BC_JUMPNEQUAL:
	case Bagel_BC::BC_JUMPEEQUAL:
	case Bagel_BC::BC_JUMPNNEQUAL:
	case Bagel_BC::BC_JUMPLARGER:
	case Bagel_BC::BC_JUMPSMALLER:
	case Bagel_BC::BC_JUMPLE:
	case Bagel_BC::BC_JUMPSE:
		if (table[idx] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = idx;
			r->in = r->out = cur->out;
			r->catchblock = cur->catchblock;
			table[idx] = r;
		}
		if (table[c.A] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = c.A;
			r->in = cur->out;
			r->catchblock = cur->catchblock;
			table[c.A] = r;
		}
		if (table[idx + 1] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = idx + 1;
			r->in = cur->out;
			r->catchblock = cur->catchblock;
			table[idx + 1] = r;
		}
		table[idx]->nexts[0] = table[idx + 1];
		table[idx]->nexts[1] = table[c.A];
		cur = table[idx];
		break;
	case Bagel_BC::BC_FOREACHSTART:
		if (table[c.C] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = c.C;
			r->in = cur->out;
			r->catchblock = cur->catchblock;
			table[c.C] = r;
		}
		if (table[idx + 1] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = idx + 1;
			r->in = cur->out;
			r->catchblock = cur->catchblock;
			table[idx + 1] = r;
		}
		cur->end = idx;
		cur->nexts[0] = table[idx + 1];
		cur->nexts[1] = table[c.C];
		cur = table[idx];
		break;
	case Bagel_BC::BC_LOOPSTEP:
	case Bagel_BC::BC_LOOPSTEP2:
	case Bagel_BC::BC_NEXT:
		if (table[idx] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = idx;
			r->in = r->out = cur->out;
			r->catchblock = cur->catchblock;
			table[idx] = r;
		}
		if (table[c.B] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = c.B;
			r->in = cur->out;
			r->catchblock = cur->catchblock;
			table[c.B] = r;
		}
		if (table[idx + 1] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = idx + 1;
			r->in = cur->out;
			r->catchblock = cur->catchblock;
			table[idx + 1] = r;
		}
		cur->nexts[0] = table[idx];
		table[idx]->nexts[0] = table[idx + 1];
		table[idx]->nexts[1] = table[c.B];
		cur = table[idx];
		break;
	case Bagel_BC::BC_ADDTRYHOOK:
		if (table[c.A] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = c.A;
			r->in = cur->out;
			r->catchblock = cur->catchblock;
			table[c.A] = r;
		}
		if (table[idx + 1] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = idx + 1;
			r->in = cur->out;
			table[idx + 1] = r;
		}
		table[idx + 1]->catchblock = table[c.A];
		cur->end = idx;
		cur->nexts[0] = table[idx + 1];
		cur->nexts[1] = table[c.A];
		cur = table[idx];
		break;
	case Bagel_BC::BC_REMOVETRYHOOK:
		if (table[idx] == nullptr)
		{
			auto r = new CFANode();
			r->begin = r->end = idx;
			r->in = cur->out;
			r->catchblock = cur->catchblock;
			table[idx] = r;
		}
		{
			auto it = std::find(catchlist.begin(), catchlist.end(), table[idx]->catchblock);
		#if PARSER_DEBUG && defined(WIN32)
			if (it == catchlist.end())
				__debugbreak();
		#endif
			--it;
			table[idx]->catchblock = *it;
		}
		cur = table[idx];
		break;
	default:
		if (!table[idx])
		{
			cur->end = idx;
			table[idx] = cur;
		}
		else
		{
			//continue a new block
			if(!cur->nexts[0])
				cur->nexts[0] = table[idx];
			cur = table[idx];
		}
		break;
	}
}

#define CHECK_CATCH(pos) if(node->catchblock && node->catchblock->in[pos] != node->out[pos])node->catchblock->in[pos] = VAR_UNKNOWN;

void CFAAnalysis::handleDetail(int idx, CFANode *node)
{
	auto &c = code->code[idx];
	auto A = c.A;
	auto B = c.B;
	auto C = c.C;
	switch (c.opcode)
	{
	case Bagel_BC::BC_ADD:
		if (node->out[B] != VAR_NONE)
			node->out[A] = node->out[B];
		else
			node->out[A] = node->out[C];
		CHECK_CATCH(A);
		break;
	case Bagel_BC::BC_SUB:
	case Bagel_BC::BC_SUBSC:
		if (node->out[B] > VAR_STR || node->out[B] == VAR_UNKNOWN)
			node->out[A] = node->out[B];
		else
			node->out[A] = VAR_NUM;
		CHECK_CATCH(A);
		break;
	case Bagel_BC::BC_MUL:
	case Bagel_BC::BC_MULCS:
	case Bagel_BC::BC_MULSC:
	case Bagel_BC::BC_DIV:
	case Bagel_BC::BC_DIVCS:
	case Bagel_BC::BC_DIVSC:
	case Bagel_BC::BC_INTDIV:
	case Bagel_BC::BC_INTDIVCS:
	case Bagel_BC::BC_INTDIVSC:
	case Bagel_BC::BC_MOD:
	case Bagel_BC::BC_MODCS:
	case Bagel_BC::BC_MODSC:
	case Bagel_BC::BC_POW:
	case Bagel_BC::BC_POWCS:
	case Bagel_BC::BC_POWSC:
	case Bagel_BC::BC_EQUAL:
	case Bagel_BC::BC_NEQUAL:
	case Bagel_BC::BC_EEQUAL:
	case Bagel_BC::BC_NNEQUAL:
	case Bagel_BC::BC_LARGER:
	case Bagel_BC::BC_SMALLER:
	case Bagel_BC::BC_LE:
	case Bagel_BC::BC_SE:
	case Bagel_BC::BC_NOT:
	case Bagel_BC::BC_BITAND:
	case Bagel_BC::BC_BITOR:
	case Bagel_BC::BC_BITXOR:
	case Bagel_BC::BC_BITNOT:
	case Bagel_BC::BC_TOBOOL:
	case Bagel_BC::BC_TOINT:
	case Bagel_BC::BC_TONUMBER:
	case Bagel_BC::BC_TOMINUSNUMBER:
	case Bagel_BC::BC_SETMUL:
	case Bagel_BC::BC_SETDIV:
	case Bagel_BC::BC_SETINTDIV:
	case Bagel_BC::BC_SETMOD:
	case Bagel_BC::BC_SETPOW:
	case Bagel_BC::BC_SELFINC:
	case Bagel_BC::BC_SELFDEC:
	case Bagel_BC::BC_PREINC:
	case Bagel_BC::BC_PREDEC:
	case Bagel_BC::BC_SETMULC:
	case Bagel_BC::BC_SETDIVC:
	case Bagel_BC::BC_SETINTDIVC:
	case Bagel_BC::BC_SETMODC:
	case Bagel_BC::BC_SETPOWC:
	case Bagel_BC::BC_SETSETC:
	case Bagel_BC::BC_LOADNUM:
	case Bagel_BC::BC_INSTANCE:
		node->out[A] = VAR_NUM;
		CHECK_CATCH(A);
		break;
	case Bagel_BC::BC_ADDCS:
		if (code->consts[B].getType() != VAR_NONE)
			node->out[A] = code->consts[B].getType();
		else
			node->out[A] = node->out[C];
		CHECK_CATCH(A);
		break;
	case Bagel_BC::BC_SUBCS:
		if (code->consts[B].getType() > VAR_STR)
			node->out[A] = code->consts[B].getType();
		else
			node->out[A] = VAR_NUM;
		CHECK_CATCH(A);
		break;
		node->out[A] = VAR_NUM;
		CHECK_CATCH(A);
		break;
	case Bagel_BC::BC_ADDSC:
		if (node->out[B] != VAR_NONE)
			node->out[A] = node->out[B];
		else
			node->out[A] = code->consts[C].getType();
		CHECK_CATCH(A);
		break;

		break;

		//convert
	case Bagel_BC::BC_CLONE:
	case Bagel_BC::BC_MOV:
		node->out[A] = node->out[B];
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_TOSTRING:
		node->out[A] = VAR_STR;
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_SETADD:
	case Bagel_BC::BC_SETSET:
		if (node->out[A] == VAR_NONE)
			node->out[A] = node->out[B];
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_SETSUB:
	case Bagel_BC::BC_SETSUBC:
		if (node->out[A] == VAR_NONE || node->out[A] == VAR_STR)
			node->out[A] = VAR_NUM;
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_MOVC:
	case Bagel_BC::BC_COPYCONST:
	case Bagel_BC::BC_LOADCONST:
		node->out[A] = code->consts[B].getType();
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_SETADDC:
		if (node->out[A] == VAR_NONE)
			node->out[A] = node->out[B];
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_LOADGLOBAL:
		node->out[A] = VAR_CLO;
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_LOADVOID:
		node->out[A] = VAR_NONE;
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_INITSTACKMEMBER:
	case Bagel_BC::BC_INITSTACKMEMBER2:
	case Bagel_BC::BC_LOADPOINTER:
	case Bagel_BC::BC_LOADCLOMEMBER_VALUE:
	case Bagel_BC::BC_LOADMEMBER_VALUE:
	case Bagel_BC::BC_LOADGLOBALMEMBER_VALUE:
	case Bagel_BC::BC_LOADSUPERMEMBER_VALUE:
	case Bagel_BC::BC_LOADMEMBERIDX_VALUE:
	case Bagel_BC::BC_LOADMEMBERSTRIDX_VALUE:
	case Bagel_BC::BC_LOADARRAYIDX_VALUE:
	case Bagel_BC::BC_LOADARRAYIDX2_VALUE:
	case Bagel_BC::BC_MAKELOCALPROPGET:
	case Bagel_BC::BC_MAKELOCALPROPSET:
	case Bagel_BC::BC_MAKELOCALPROPERTY:
	case Bagel_BC::BC_CALL:
	case Bagel_BC::BC_CALLVECTOR:
	case Bagel_BC::BC_ELEMENT:
		node->out[A] = VAR_UNKNOWN;
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_SETPOINTER:
	case Bagel_BC::BC_SETMEMBER2:
	case Bagel_BC::BC_LOADSUPERMEMBER_PTR:
	case Bagel_BC::BC_GETMEMBER_PTR:
	case Bagel_BC::BC_GETCLOMEMBER_PTR:
	case Bagel_BC::BC_GETGLOBALMEMBER_PTR:
	case Bagel_BC::BC_GETMEMBERIDX_PTR:
	case Bagel_BC::BC_GETMEMBERSTRIDX_PTR:
	case Bagel_BC::BC_GETARRAYIDX_PTR:
	case Bagel_BC::BC_GETARRAYIDX2_PTR:
	case Bagel_BC::BC_PTR_SET:
	case Bagel_BC::BC_PTR_SETADD:
	case Bagel_BC::BC_PTR_SETSUB:
	case Bagel_BC::BC_PTR_SETMUL:
	case Bagel_BC::BC_PTR_SETDIV:
	case Bagel_BC::BC_PTR_SETINTDIV:
	case Bagel_BC::BC_PTR_SETMOD:
	case Bagel_BC::BC_PTR_SETPOW:
	case Bagel_BC::BC_PTR_SETSET:
	case Bagel_BC::BC_PTR_SELFINC:
	case Bagel_BC::BC_PTR_SELFDEC:
	case Bagel_BC::BC_PTR_PREINC:
	case Bagel_BC::BC_PTR_PREDEC:
	case Bagel_BC::BC_PTR_SETC:
	case Bagel_BC::BC_PTR_SETADDC:
	case Bagel_BC::BC_PTR_SETSUBC:
	case Bagel_BC::BC_PTR_SETMULC:
	case Bagel_BC::BC_PTR_SETDIVC:
	case Bagel_BC::BC_PTR_SETINTDIVC:
	case Bagel_BC::BC_PTR_SETMODC:
	case Bagel_BC::BC_PTR_SETPOWC:
	case Bagel_BC::BC_PTR_SETSETC:
	case Bagel_BC::BC_SETCLOSURE:
	case Bagel_BC::BC_SETCLOSURESTACK:
	case Bagel_BC::BC_ADDPROPGET:
	case Bagel_BC::BC_ADDPROPSET:
	case Bagel_BC::BC_ADDPROPERTY:
	case Bagel_BC::BC_ADDSTATIC:
	case Bagel_BC::BC_ADDVAR:
	case Bagel_BC::BC_ADDFUNCTION:
	case Bagel_BC::BC_FINISHCLASS:
	case Bagel_BC::BC_JUMPFALSE:
	case Bagel_BC::BC_JUMPTRUE:
	case Bagel_BC::BC_JUMPVOID:
	case Bagel_BC::BC_JUMPNOTVOID:
	case Bagel_BC::BC_JUMP:
	case Bagel_BC::BC_JUMPEQUAL:
	case Bagel_BC::BC_JUMPNEQUAL:
	case Bagel_BC::BC_JUMPEEQUAL:
	case Bagel_BC::BC_JUMPNNEQUAL:
	case Bagel_BC::BC_JUMPLARGER:
	case Bagel_BC::BC_JUMPSMALLER:
	case Bagel_BC::BC_JUMPLE:
	case Bagel_BC::BC_JUMPSE:
	case Bagel_BC::BC_RETURN:
	case Bagel_BC::BC_YIELD:
	case Bagel_BC::BC_VECTORPUSHBACK:
	case Bagel_BC::BC_VECTORCONCAT:
	case Bagel_BC::BC_ADDTRYHOOK:
	case Bagel_BC::BC_REMOVETRYHOOK:
	case Bagel_BC::BC_REMOVETRYHOOKN:
	case Bagel_BC::BC_DELETE:
	case Bagel_BC::BC_THROW:
	case Bagel_BC::BC_THROWCONST:
	case Bagel_BC::BC_CHECKTYPE:
	case Bagel_BC::BC_CHECKTHIS:
	case Bagel_BC::BC_RESERVE_STACK:
	case Bagel_BC::BC_GC:
	case Bagel_BC::BC_CACHE_NONE:
	case Bagel_BC::BC_CACHE_GLOBAL_MEMBER:
	case Bagel_BC::BC_CACHE_DIC_MEMBER:
	case Bagel_BC::BC_CACHE_CLASS_VAR:
	case Bagel_BC::BC_CACHE_CLASS_FUNC:
	case Bagel_BC::BC_CACHE_TYPE_MEMBER:
	case Bagel_BC::BC_CACHE_CLO_MEMBER1:
	case Bagel_BC::BC_CACHE_CLO_MEMBER2:
	case Bagel_BC::BC_CACHE_CLO_MEMBER_EXT:
	case Bagel_BC::BC_CACHE_GLOBAL_INCLASS:
	case Bagel_BC::BC_CACHE_GLOBAL_INCLASSDEF:
	case Bagel_BC::BC_CACHE_CLASS_VARINDEX:
	case Bagel_BC::BC_CACHE_NEWCLASS_FUNC:
	case Bagel_BC::BC_CACHE_GLOBAL_MEMBERV:
	case Bagel_BC::BC_CACHE_DIC_MEMBERV:
	case Bagel_BC::BC_CACHE_CLASS_VARV:
	case Bagel_BC::BC_CACHE_CLASS_FUNCV:
	case Bagel_BC::BC_CACHE_TYPE_MEMBERV:
	case Bagel_BC::BC_CACHE_CLO_MEMBER1V:
	case Bagel_BC::BC_CACHE_CLO_MEMBER2V:
	case Bagel_BC::BC_CACHE_CLO_MEMBER_EXTV:
	case Bagel_BC::BC_CACHE_GLOBAL_INCLASSV:
	case Bagel_BC::BC_CACHE_GLOBAL_INCLASSDEFV:
	case Bagel_BC::BC_CACHE_CLASS_VARINDEXV:
	case Bagel_BC::BC_CACHE_NEWCLASS_FUNCV:
		break;

	case Bagel_BC::BC_MAKEARRAY:
		node->out[A] = VAR_ARRAY;
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_MAKEDIC:
		node->out[A] = VAR_DIC;
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_MAKECLASS:
		node->out[A] = VAR_CLASS;
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_PUSHCLO:
		node->out[A] = VAR_CLO;
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_NULLVECTOR:
		node->out[A] = VAR_VECTOR_P;
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_LOOPSTART:
		node->out[A] = VAR_NUM;
		node->out[A + 1] = VAR_NUM;
		node->out[A + 2] = VAR_NUM;
		node->out[A + 3] = VAR_NUM;
		CHECK_CATCH(A);
		CHECK_CATCH(A + 1);
		CHECK_CATCH(A + 2);
		CHECK_CATCH(A + 3);
		break;

	case Bagel_BC::BC_LOOPSTEP:
	case Bagel_BC::BC_LOOPSTEP2:
		node->out[A + 3] = VAR_NUM;
		CHECK_CATCH(A + 3);
		break;

	case Bagel_BC::BC_FOREACHSTART:
		node->out[A + 1] = VAR_UNKNOWN;
		node->out[A + 2] = VAR_UNKNOWN;
		node->out[A + 3] = VAR_UNKNOWN;
		node->out[A + 4] = VAR_UNKNOWN;
		CHECK_CATCH(A + 1);
		CHECK_CATCH(A + 2);
		CHECK_CATCH(A + 3);
		CHECK_CATCH(A + 4);
		break;

	case Bagel_BC::BC_NEXT:
		node->out[A + 2] = VAR_UNKNOWN;
		node->out[A + 3] = VAR_UNKNOWN;
		node->out[A + 4] = VAR_UNKNOWN;
		CHECK_CATCH(A + 2);
		CHECK_CATCH(A + 3);
		CHECK_CATCH(A + 4);
		break;

	case Bagel_BC::BC_TYPE:
		node->out[A] = VAR_STR;
		CHECK_CATCH(A);
		break;

	case Bagel_BC::BC_INCONTEXTOF:
		node->out[A] = VAR_FUNC;
		CHECK_CATCH(A);
		break;

		//pointer
	case Bagel_BC::BC_CREATEPOINTER:
		node->out[A] = VAR_POINTER;
		CHECK_CATCH(A);
		break;

	default:
		break;
	}
}
