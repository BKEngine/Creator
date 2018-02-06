#include "Bagel_Serializer.h"
#include "Bagel_Parser.h"
#include "Bagel_VM.h"
#include "Bagel_RCompiler.h"

void Bagel_Serializer::operator<<(const Bagel_Var & v)
{
	switch (v.getType())
	{
	case VAR_NONE:
		data.push_back(SEL_VOID << 24);
		break;
	case VAR_NUM:
		data.push_back(SEL_NUM << 24);
		{
			int32_t t[2];
			double d = v.forceAsNumber();
			memcpy(t, &d, sizeof(double));
			data.push_back(t[0]);
			data.push_back(t[1]);
		}
		break;
	case VAR_STR:
		{
			auto r = getObject(v.forceAsBKEStr());
			if (!r.isnew)
			{
				data.push_back(SEL_OBJ << 24 | r.pos);
			}
			else
			{
				StringVal s = v.forceAsBKEStr()->getConstStr();
				if (s.empty())
				{
					data.push_back(SEL_NULLSTR << 24);
				}
				else
				{
					data.push_back((SEL_STR << 24) | s.length());
					int t = data.size();
					data.resize(t + (s.length() + 1) / 2);
					memcpy(&data[t], &s[0], s.length() * 2);
				}
			}
		}
		break;
	case VAR_ARRAY:
		{
			auto arr = v.forceAsArray();
			auto r = getObject(arr);
			if(r.isnew)
			{
				//data.push_back((SEL_NEWOBJ << 24) | r.pos);
				data.push_back((SEL_ARR << 24) | arr->getCount());
				for (auto &it : arr->vararray)
				{
					*this << it;
				}
			}
			else
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
		}
		break;
	case VAR_DIC:
		{
			auto dic = v.forceAsDic();
			auto r = getObject(dic);
			if (r.isnew)
			{
				//data.push_back((SEL_NEWOBJ << 24) | r.pos);
				data.push_back(SEL_DIC << 24 | dic->getCount());
				for (auto &it : dic->varmap)
				{
					*this << it.first;
					*this << it.second;
				}
			}
			else
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
		}
		break;
	case VAR_FUNC:
		{
			auto func = v.forceAsFunc();
			auto r = getObject(func);
			if (r.isnew)
			{
				//data.push_back((SEL_NEWOBJ << 24) | r.pos);
				data.push_back(SEL_FUNC << 24);
				*this << func->func;
				*this << func->name;
				*this << func->self;
				if (func->closure == _globalStructures.globalClosure)
					data.push_back(SEL_GLOBALCLO << 24);
				else
					*this << func->closure;
				if (func->default_stack)
					*this << func->default_stack;
				else
					data.push_back(SEL_VOID << 24);
			}
			else
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
		}
		break;
	case VAR_PROP:
		{
			auto prop = v.forceAsProp();
			auto r = getObject(prop);
			if (r.isnew)
			{
				//data.push_back((SEL_NEWOBJ << 24) | r.pos);
				data.push_back(SEL_PROP << 24);
				*this << prop->name;
				*this << prop->setparam;
				*this << prop->self;
				if (prop->closure == _globalStructures.globalClosure)
					data.push_back(SEL_GLOBALCLO << 24);
				else
					*this << prop->closure;
				if (prop->funcget)
					*this << prop->funcget;
				else
					data.push_back(SEL_VOID << 24);
				if (prop->funcset)
					*this << prop->funcset;
				else
					data.push_back(SEL_VOID << 24);
				//if (prop->default_stack)
				//	*this << prop->default_stack;
				//else
				//	data.push_back(SEL_VOID << 24);
			}
			else
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
		}
		break;
	case VAR_CLO:
		{
			auto clo = v.forceAsClosure();
			auto r = getObject(clo);
			if (r.isnew)
			{
				//data.push_back((SEL_NEWOBJ << 24) | r.pos);
				data.push_back((SEL_CLO << 24) | clo->geCount());
				for (auto &it : clo->varmap)
				{
					*this << it.first;
					*this << it.second;
				}
				if (clo->parent == _globalStructures.globalClosure)
					data.push_back(SEL_GLOBALCLO << 24);
				else
					*this << clo->parent;
			}
			else
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
		}
		break;
	case VAR_CLASS:
		{
			auto cla = v.forceAsClass();
			auto r = getObject(cla);
			if (!r.isnew)
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
			else
			{
				//data.push_back((SEL_NEWOBJ << 24) | r.pos);
				data.push_back((SEL_CLASS << 24) | cla->varmap.getCount());
				*this << cla->defclass;
				*this << cla->classname;
				data.push_back(cla->finalized);
				//空的也要存，否则会找不到类成员
				for (auto &it : cla->varmap)
				{
					*this << it.first;
					*this << it.second;
				}
				if(!cla->native)
				{
					data.push_back(SEL_VOID << 24);
				}
				else
				{
					*this << cla->native->nativeName;
					*this << cla->native->nativeSave();
				}
			}
		}
		break;
	case VAR_CLASSDEF:
		{
			auto cla = v.forceAsClassDef();
			auto r = getObject(cla);
			if (!r.isnew)
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
			else
			{
				//native类不用存
				if (!cla->native || cla->native->nativeName != cla->classname)
				{
					data.push_back((SEL_DEFCLASS << 24) | cla->varmap.getCount());
					for (auto &it : cla->parents)
						*this << it;
					*this << cla->classname;
					data.push_back(cla->parents.size());
					for (auto &it : cla->parents)
						*this << it->classname;
					data.push_back(cla->cannotcreate);
					data.push_back(cla->isFinal);
					//空的也要存，否则会找不到类成员
					for (auto &it : cla->varmap)
					{
						*this << it.first;
						*this << it.second;
					}
					data.push_back(cla->classvar.getCount());
					for (auto &it : cla->classvar)
					{
						*this << it.first;
						*this << it.second;
					}
				}
				else
				{
					cache.erase(cla);
					counter--;
				}
			}
		}
		break;
	case VAR_POINTER:
		{
			auto p = v.forceAsPointer();
			auto r = getObject(p);
			if (!r.isnew)
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
			else
			{
				//data.push_back((SEL_NEWOBJ << 24) | r.pos);
				data.push_back(SEL_POINTER << 24);
				*this << p->var;
				*this << p->name;
			}
		}
		break;
	case VAR_BYTREE_P:
		{
			auto tr = v.forceAsObject<Bagel_AST>();
			auto r = getObject(tr);
			if (!r.isnew)
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
			else
			{
				StringVal res;
				Bagel_Parser::getInstance()->unParse(tr, res);
				data.push_back((SEL_BYTREE << 24) | res.length());
				int t = data.size();
				data.resize(t + (res.length() + 1) / 2);
				memcpy(&data[t], &res[0], res.length() * 2);
			}
		}
		break;
	case VAR_BYTECODE_P:
		{
			auto code = v.forceAsObject<Bagel_ByteCode>();
			auto r = getObject(code);
			if (!r.isnew)
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
			else
			{
				Bagel_VM::getInstance()->tempClearDebugInfo(code);
				data.push_back((SEL_BYTECODE << 24) | code->code.size());
				int raw = data.size();
				data.resize(raw + code->code.size() * sizeof(Bagel_ByteCodeStruct) / sizeof(uint32_t));
				memcpy(data.data() + raw, code->code.data(), code->code.size() * sizeof(Bagel_ByteCodeStruct));
				Bagel_VM::getInstance()->tempRestoreDebugInfo(code);
				//无效化cache
				for (Bagel_ByteCodeStruct *start = (Bagel_ByteCodeStruct*)(data.data() + raw); start < (Bagel_ByteCodeStruct*)(data.data() + data.size()); start++)
				{
					if (start->opcode > Bagel_BC::BC_CACHE_NONE)
						start->opcode = Bagel_BC::BC_CACHE_NONE;
				}
				data.push_back(code->consts.size());
				for (auto &it : code->consts)
				{
					*this << it;
				}
				data.push_back(code->stackDepth);
				data.push_back(code->localDepth);
				data.push_back(code->release);
				data.push_back(code->needSpecialStack);
				if (code->debugInfo)
				{
					*this << code->debugInfo->rawexp;
					data.push_back(code->debugInfo->startline);
					data.push_back(code->debugInfo->beginpos);
					data.push_back(code->debugInfo->innerFunc.size());
					for (auto &it : code->debugInfo->innerFunc)
					{
						*this << it;
					}
					if (code->varPosInfo)
					{
						auto &&info = code->varPosInfo->data;
						data.push_back(info.getCount());
						for (auto &it : info)
						{
							*this << it.first;
							data.push_back(it.second.size());
							for (auto &it2 : it.second)
							{
								data.push_back(it2.first);
								data.push_back(it2.second.first);
								data.push_back(it2.second.second ? 1 : 0);
							}
						}
					}
					else
					{
						data.push_back(0);
					}
				}
				else
				{
					data.push_back(SEL_VOID << 24);
				}
			}
		}
		break;
	case VAR_FUNCCODE_P:
		{
			auto func = v.forceAsObject<Bagel_FunctionCode>();
			auto r = getObject(func);
			if (!r.isnew)
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
			else
			{
				//data.push_back((SEL_NEWOBJ << 24) | r.pos);
				data.push_back(SEL_FUNCCODE << 24);
				if (func->bytecode)
				{
					*this << func->bytecode;
				}
				else if(func->code)
				{
					*this << func->code;
				}
				else
				{
					//cannot save native function now
					data.push_back(SEL_VOID << 24);
				}
				data.push_back(func->paramnames.size());
				for (int i = 0; i < func->paramnames.size(); i++)
				{
					*this << func->paramnames[i];
					*this << func->initial_stack[i];
				}
				data.push_back(func->paramarrpos);
				//如果是bytecode，在bytecode里写info，这里不写
				if (func->bytecode)
				{
					data.push_back(SEL_VOID << 24);
				}
				else
				{
					if (func->info)
					{
						*this << func->info->rawexp;
						data.push_back(func->info->startline);
						data.push_back(func->info->beginpos);
					}
					else
					{
						data.push_back(SEL_VOID << 24);
					}
				}
			}
		}
		break;
	case VAR_STACK_P:
		{
			auto s = v.forceAsObject<Bagel_Stack>();;
			auto r = getObject(s);
			if (!r.isnew)
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
			else
			{
				//data.push_back((SEL_NEWOBJ << 24) | r.pos);
				data.push_back((SEL_STACK << 24) | s->stacksize);
				for (auto &it : *s)
				{
					*this << it;
				}
				data.push_back(s->relativePos - s->stack);
				data.push_back(s->isSpecialStack);
			}
		}
		break;
	case VAR_VECTOR_P:
		{
			auto arr = v.forceAsObject<Bagel_Vector>();
			auto r = getObject(arr);
			if (r.isnew)
			{
				//data.push_back((SEL_NEWOBJ << 24) | r.pos);
				data.push_back((SEL_VECTOR << 24) | arr->size());
				for (auto &it : *arr)
				{
					*this << it;
				}
			}
			else
			{
				data.push_back((SEL_OBJ << 24) | r.pos);
			}
		}
		break;
	default:
		assert(false);
	}
}

Bagel_Var Bagel_Serializer::parse(const vector<uint32_t>& save)
{
	//invalid data
	if (save.empty() || save[0] != BAGEL_SERIALIZE_SIG || save[1] != save.size() || save.size() < 4)
		return Bagel_Var();
	//hash check
	uint32_t h = 0;
	for (unsigned int i = 3; i < save.size(); i++)
		h ^= save[i];
	if(save[2] != h)
		return Bagel_Var();
	const uint32_t *s = &save[3];
	GC_Locker locker;
	try
	{
		auto v = _parse(s);
		recache.clear();
		return v;
	}
	catch(Bagel_Except &)
	{
		recache.clear();
		return Bagel_Var();
	}
}

Bagel_Var Bagel_Serializer::parse(const uint32_t * save, int size)
{
	lastError = SEL_NOERROR;
	//invalid data
	if (!save || save[0] != BAGEL_SERIALIZE_SIG)
	{
		lastError = SEL_SIGERROR;
		return Bagel_Var();
	}
	if (save[1] != size || size < 4)
	{
		lastError = SEL_LENGTHERROR;
		return Bagel_Var();
	}
	//hash check
	uint32_t h = 0;
	for (unsigned int i = 3; i < size; i++)
		h ^= save[i];
	if (save[2] != h)
	{
		lastError = SEL_CRCERROR;
		return Bagel_Var();
	}
	const uint32_t *s = &save[3];
	GC_Locker locker;
	try
	{
		auto v = _parse(s);
		recache.clear();
		return v;
	}
	catch (Bagel_Except &)
	{
		recache.clear();
		return Bagel_Var();
	}
}

Bagel_Var Bagel_Serializer::_parse(const uint32_t * &save)
{
	restart:
	switch (save[0] >> 24)
	{
	case SEL_VOID:
		save++;
		return Bagel_Var();
	case SEL_NUM:
		save += 3;
		return *(double*)(save - 2);
	case SEL_NULLSTR:
		save++;
		recache.push_back(_globalStructures.stringMap->nullString);
		return _globalStructures.stringMap->nullString;
	case SEL_STR:
		{
			unsigned int len = save[0] & 0xFFFFFF;
			save++;
			StringVal s((char16_t*)save, len);
			save += (len + 1) / 2;
			Bagel_StringHolder s2(s);
			recache.push_back(s2.s);
			return s2.s;
		}
	case SEL_OBJ:
		{
			unsigned int idx = (*save++) & 0xFFFFFF;
			return recache[idx];
		}
	//case SEL_NEWOBJ:
	//	{
	//		//unsigned int idx = save[0] & 0xFFFFFF;
	//		save++;
	//		auto v = _parse(save);
	//		return v;
	//	}
	case SEL_ARR:
		{
			Bagel_Array *arr = new Bagel_Array();
			recache.push_back(arr);
			unsigned int len = save[0] & 0xFFFFFF;
			save++;
			for (unsigned int i = 0; i < len; i++)
			{
				arr->vararray.push_back(_parse(save));
			}
			return arr;
		}
	case SEL_DIC:
		{
			Bagel_Dic *dic = new Bagel_Dic();
			recache.push_back(dic);
			unsigned int len = save[0] & 0xFFFFFF;
			save++;
			for (unsigned int i = 0; i < len; i++)
			{
				auto key = _parse(save);
				auto val = _parse(save);
				dic->varmap[key] = val;
			}
			return dic;
		}
	case SEL_FUNC:
		{
			save++;
			Bagel_Function *func = new Bagel_Function((Bagel_FunctionCode*)nullptr);
			recache.push_back(func);
			auto funccode = _parse(save).forceAsObject<Bagel_FunctionCode>();
			assert(funccode);
			func->func = funccode;
			func->name = (StringVal)_parse(save);
			func->self = _parse(save);
			func->closure = _parse(save).forceAsClosure();
			func->default_stack = (Bagel_Stack*)_parse(save).getObject();
			return func;
		}
	case SEL_PROP:
		{
			save++;
			auto prop = new Bagel_Prop();
			recache.push_back(prop);
			prop->name = (StringVal)_parse(save);
			prop->setparam = _parse(save).asBKEStr();
			prop->self = _parse(save);
			{
				auto v = _parse(save);
				prop->closure = v.getClosure();
			}
			{
				auto v = _parse(save);
				prop->funcget = (Bagel_FunctionCode*)v.getObject();
			}
			{
				auto v = _parse(save);
				prop->funcset = (Bagel_FunctionCode*)v.getObject();
			}
			//prop->default_stack = (Bagel_Stack*)_parse(save).obj;
			return prop;
		}
	case SEL_CLO:
		{
			unsigned int len = save[0] & 0xFFFFFF;
			save++;
			auto clo = new Bagel_Closure();
			recache.push_back(clo);
			for (unsigned int i = 0; i < len; i++)
			{
				auto key = _parse(save);
				auto val = _parse(save);
				clo->varmap[key] = val;
			}
			clo->parent = _parse(save).forceAsClosure();
			return clo;
		}
	case SEL_DEFCLASS:
		{
			unsigned int len = save[0] & 0xFFFFFF;
			save++;
			auto cla = new Bagel_ClassDef();
			recache.push_back(cla);
			cla->classname = _parse(save).forceAsBKEStr();
			vector<Bagel_StringHolder> v;
			uint32_t vsize = *save++;
			for (uint32_t i = 0; i < vsize; i++)
			{
				v.push_back(_parse(save));
			}
			cla->redefineClass(v);
			cla->cannotcreate = !!(*save++);
			cla->isFinal = !!(*save++);
			for (uint32_t i = 0; i < len; i++)
			{
				auto key = _parse(save);
				auto val = _parse(save);
				cla->varmap[key] = val;
			}
			len = *save++;
			for (uint32_t i = 0; i < len; i++)
			{
				auto key = _parse(save);
				auto val = _parse(save);
				cla->classvar[key] = val;
			}
			auto &var = _globalStructures.globalClosure->getMember(cla->classname);
			if (var.getType() != VAR_CLASSDEF)
				var.forceSet(cla);
			goto restart;
		}
	case SEL_CLASS:
		{
			unsigned int len = save[0] & 0xFFFFFF;
			save++;
			auto cla = new Bagel_Class();
			recache.push_back(cla);
			cla->classname = _parse(save).forceAsBKEStr();
			auto &var = _globalStructures.globalClosure->getMember(cla->classname);
			if (var.getType() != VAR_CLASSDEF)
			{
				lastError = SEL_NODEFINECLASS;
				save++;	//pass finalized
				for (uint32_t i = 0; i < len; i++)
				{
					_parse(save);
					_parse(save);
				}
				auto v = _parse(save);
				if (!v.isVoid())
				{
					_parse(save);
				}
				return Bagel_Var();
			}
			cla->defclass = var.forceAsClassDef();
			cla->finalized = !!(*save++);
			for (uint32_t i = 0; i < len; i++)
			{
				auto key = _parse(save);
				auto val = _parse(save);
				cla->varmap[key] = val;
			}
			auto v = _parse(save);
			if (!v.isVoid())
			{
				cla->native = _globalStructures.globalClosure->varmap[v].forceAsClass()->native->nativeCreateNULL();
				cla->native->nativeLoad(_parse(save));
			}
			return cla;
		}
	case SEL_POINTER:
		{
			save++;
			auto pt = new Bagel_Pointer();
			recache.push_back(pt);
			pt->var = _parse(save);
			pt->name = _parse(save);
			pt->dir = NULL;
			return pt;
		}
	case SEL_FUNCCODE:
		{
			save++;
			Bagel_FunctionCode *code = new Bagel_FunctionCode();
			recache.push_back(code);
			auto f = _parse(save);
			if (f.getType() == VAR_BYTREE_P)
			{
				code->code = f.forceAsObject<Bagel_AST>();
				code->bytecode = NULL;
			}
			else if (f.getType() == VAR_BYTECODE_P)
			{
				code->bytecode = f.forceAsObject<Bagel_ByteCode>();
				code->code = NULL;
			}
			else
			{
				//TODO with native function, maybe we can save its name
				return Bagel_Var();
			}
			unsigned int psize = save[0];
			save++;
			for (unsigned int i = 0; i < psize; i++)
			{
				code->paramnames.push_back(_parse(save));
				code->initial_stack.push_back(_parse(save));
			}
			code->paramarrpos = (int)*save++;
			auto dbg = _parse(save);
			if (!dbg.isVoid())
			{
				code->info.reset(new Bagel_DebugInformation(dbg.asString(), save[1], save[0]));
				save += 2;
			}
			if (code->bytecode)
			{
				code->info = code->bytecode->debugInfo;
			}
			return code;
		}
	case SEL_BYTECODE:
		{
			unsigned int len = save[0] & 0xFFFFFF;
			save++;
			auto code = new Bagel_ByteCode();
			recache.push_back(code);
			code->code.resize(len);
			memcpy(&code->code[0], save, len * sizeof(Bagel_ByteCodeStruct));
			save += len * sizeof(Bagel_ByteCodeStruct) / sizeof(uint32_t);
			uint32_t csize = *save++;
			code->consts.reserve(csize);
			for (uint32_t i = 0; i < csize; i++)
			{
				code->consts.emplace_back(_parse(save));
			}
			code->stackDepth = *save++;
			code->localDepth = *save++;
			code->release = !!(*save++);
			code->needSpecialStack = !!(*save++);
			auto v = _parse(save);
			if (!v.isVoid())
			{
				code->debugInfo.reset(new Bagel_DebugInformation(v.asString(), save[1], save[0]));
				save += 2;
				auto count = *save++;
				for (uint32_t i = 0; i < count; i++)
				{
					code->debugInfo->innerFunc.push_back(_parse(save).forceAsObject<Bagel_ByteCode>());
				}
				count = *save++;
				if (count)
				{
					code->varPosInfo.reset(new Bagel_VarPosInfo());
					auto &&data = code->varPosInfo->data;
					for (uint32_t i = 0; i < count; i++)
					{
						auto name = _parse(save);
						auto count2 = *save++;
						for (uint32_t j = 0; j < count2; j++)
						{
							data[name][save[0]] = make_pair(save[1], !!save[2]);
							save += 3;
						}
					}
				}
			}
			return code;
		}
	case SEL_BYTREE:
		{
			unsigned int len = save[0] & 0xFFFFFF;
			save++;
			StringVal exp = StringVal((char16_t*)save, len);
			save += (len + 1) / 2;
			auto tree = Bagel_Parser::getInstance()->parse(exp);
			recache.push_back(tree);
			return tree;
		}
	case SEL_GLOBALCLO:
		save++;
		return _globalStructures.globalClosure;
	case SEL_STACK:
		{
			unsigned int len = save[0] & 0xFFFFFF;
			save++;
			auto st = new Bagel_Stack();
			recache.push_back(st);
			st->stack = new Bagel_Var[len];
			st->isSpecialStack = true;
			for (uint32_t i = 0; i < len; i++)
			{
				st->stack[i].forceSet(_parse(save));
			}
			st->relativePos = &st->stack[*save++];
			st->isSpecialStack = !!(*save++);
			return st;
		}
	case SEL_VECTOR:
		{
			auto arr = new Bagel_Vector();
			recache.push_back(arr);
			unsigned int len = save[0] & 0xFFFFFF;
			save++;
			for (unsigned int i = 0; i < len; i++)
			{
				arr->push_back(_parse(save));
			}
			return arr;
		}
	default:
		assert(false);
	}
	return Bagel_Var();
}
