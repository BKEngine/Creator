#pragma once

#include "Bagel_Utils.h"
#include "Bagel_GC.h"
#include "Bagel_Parser.h"
#include <map>

#define CLOSTACK_FLAG (1UL<<31)
#define CLOSTACK_MASK ((1UL<<31)-1)
#define STACKSIGN_FLAG (1UL<<30)
//编译出来后查询某位置某变量名对应的栈位置信息
class Bagel_VarPosInfo
{
	friend class Bagel_Serializer;

	//0表示变量未定义在栈上
	//& CLOSTACK_FLAG表示定义在clostack上
	//有效stack pos 31位
	BKE_hashmap<Bagel_StringHolder, map<int32_t /*pos*/, pair<int32_t /*stack pos*/, bool /*is pointer?*/>>> data;

	BKE_hashmap<Bagel_StringHolder, stack<pair<int32_t, bool>>> stack_data;

public:
	void addVar(Bagel_StringHolder name, int32_t codepos, int32_t stackpos)
	{
		data[name][codepos] = make_pair(stackpos & CLOSTACK_MASK, false);
		stack_data[name].push(make_pair(stackpos & CLOSTACK_MASK, false));
	};

	//codepos为下一句位置
	void removeVar(Bagel_StringHolder name, int32_t codepos)
	{
		stack_data[name].pop();
		data[name][codepos] = stack_data[name].empty() ? make_pair(0, false) : make_pair(stack_data[name].top().first, false);
	}

	void addParentVar(Bagel_StringHolder name, int32_t codepos, int32_t stackpos, int32_t parentStackPos, bool parentIsPointer)
	{
		data[name][0] = make_pair(parentStackPos & CLOSTACK_FLAG, parentIsPointer);
		stack_data[name].push(make_pair(parentStackPos & CLOSTACK_FLAG, parentIsPointer));
		data[name][codepos] = make_pair(stackpos & CLOSTACK_MASK, true);
		stack_data[name].push(make_pair(stackpos & CLOSTACK_MASK, true));
	}

	pair<int32_t, bool> queryVar(Bagel_StringHolder name, int32_t codepos)
	{
		auto it = data.find(name);
		if (it == data.end())
			return make_pair(0, false);
		auto it2 = it->second.upper_bound(codepos);
		if (it2 == it->second.begin())
			return make_pair(0, false);
		--it2;
		return make_pair(it2->second.first, it2->second.second);
	}
};

struct CompileClo
{
	BKE_hashmap<Bagel_StringHolder, int> locals;
	int withpos;	//with里面变量的位置，没有则为-1，继承父闭包的位置
	vector<int> *breakcode;
	vector<int> *continuecode;

	//BKE_hashmap<Bagel_Var, int> *switchtable;

	CompileClo *parent;

	int getLocal(Bagel_StringHolder name)
	{
		auto it = locals.find(name);
		if (it != locals.end())
			return it->second;
		if (parent)
		{
			return parent->getLocal(name);
		}
		return 0;
	}

	//局部变量给负值，函数参数给正值
	void regLocal(Bagel_StringHolder name, int pos)
	{
		//assert(pos < 0);
		locals[name] = pos;
	}

	void purgeClo(Bagel_VarPosInfo *info, Bagel_AST *tree)
	{
		if (!info || !tree)
			return;
		for (auto &it : locals)
		{
			info->removeVar(it.first, tree->Node.pos2);
		}
	}
};

struct Bagel_TypeInfo
{
	vector<int> stack;	//from 0
	vector<int> local;	//from -1

	int flag;

	Bagel_TypeInfo() : flag(0)
	{
	}

	int& operator [] (int pos)
	{
		if (pos >= 0)
		{
			if (pos >= stack.size())
				stack.resize(pos + 1);
			return stack[pos];
		}
		else
		{
			if (-pos >= local.size())
				local.resize(-pos + 1);
			return local[-pos];
		}
	}

	VarType operator [] (int pos) const
	{
		if (pos >= 0)
		{
			if (pos >= stack.size())
				return VAR_NONE;
			return (VarType)stack[pos];
		}
		else
		{
			if (-pos >= local.size())
				return VAR_NONE;
			return (VarType)local[-pos];
		}
	}

	void set(int pos, int value)
	{
		operator[](pos) = value;
	}

	void mergeSet(int pos, int value)
	{
		if (operator [] (pos) != value)
			operator [] (pos) = VAR_UNKNOWN;
	}

	bool merge(const Bagel_TypeInfo &t)
	{
		if (stack.size() < t.stack.size())
			stack.resize(t.stack.size());
		if (local.size() < t.local.size())
			local.resize(t.local.size());
		bool ret = false;
		for (int i = 0; i < t.stack.size(); i++)
		{
			if (stack[i] != VAR_UNKNOWN && stack[i] != t.stack[i])
				stack[i] = VAR_UNKNOWN, ret = true;
		}
		for (int i = 0; i < t.local.size(); i++)
		{
			if (local[i] != VAR_UNKNOWN && local[i] != t.local[i])
				local[i] = VAR_UNKNOWN, ret = true;
		}
		return ret;
	}
};

struct CFANode
{
	int begin;
	int end;

	int flag;

	Bagel_TypeInfo in;
	Bagel_TypeInfo out;

	CFANode* nexts[2];

	CFANode* catchblock;

	CFANode() : flag(0), catchblock(nullptr)
	{
		nexts[0] = nexts[1] = nullptr;
	}
};

class CFAAnalysis
{
	CFANode *root;
	CFANode *cur;
	const Bagel_ByteCode *code;
	vector<CFANode*> table;
	Bagel_TypeInfo now;
	list<CFANode*> catchlist;

public:
	CFAAnalysis() :root(new CFANode())
	{
		catchlist.push_back(nullptr);
	}

	void begin(const Bagel_ByteCode &c, const Bagel_TypeInfo &in)
	{
		root->begin = -1;
		root->end = -1;
		root->in = in;
		root->out = in;
		root->nexts[0] = nullptr;
		root->nexts[1] = nullptr;
		code = &c;
		table.resize(code->code.size());
		now = in;
		cur = root;
		for (int i = 0; i < code->code.size(); i++)
		{
			handleOne(i);
		}
	}

	void handleOne(int idx);

	void handleDetail(int idx, CFANode *node);

	void processNode(CFANode *node, Bagel_TypeInfo &info)
	{
		if (!node)
			return;
		if (node->flag > 0 && !node->in.merge(info))
			return;
		node->in = info;
		node->out = node->in;
		for (int i = node->begin; i <= node->end; i++)
		{
			if (i < 0)
				continue;
			handleDetail(i, node);
		}
		node->flag = 1;
		processNode(node->nexts[0], node->out);
		processNode(node->nexts[1], node->out);
	}

	void calcCFA()
	{
		processNode(root, root->in);
	}

	void initAllNode()
	{
		for (auto &i : table)
		{
			if (i->flag > 1)
				continue;
			i->out = i->in;
			i->flag = 2;
			i->catchblock = nullptr;
		}
	}

	VarType getCurrentType(int idx, int stackidx)
	{
		return (VarType)table[idx]->out[stackidx];
	}

	Bagel_TypeInfo& getTypeInfo(int idx)
	{
		return table[idx]->out;
	}

	void regCmd(int idx)
	{
		handleDetail(idx, table[idx]);
	}
};

/*
编译期类型推导
遇到双if分支时，判断分支的情况并合并。
遇到单if分支时，判断分支的情况并与之前的合并。
遇到do while和quickfor时，先执行一遍，剩下的转化为while循环：
	第一次执行时，新建flag=1的typeinfo，如果没有break或continue，结束时直接覆盖原先typeinfo
		如果有，在此之前的部分覆盖原先typeinfo，之后的部分当做if处理。
遇到while和for循环时，先lock住，按if方式执行一遍，获得typeinfo，之后将其作为初始的typeinfo去生成代码。
遇到try catch块时，按if处理。
遇到switch时，每个case块或default块按if处理。
*/

//fast release compiler, generate original bytecode
/// <summary>
/// Bagel_ReleaseCompiler，生成运行用字节码的编译器类，侧重运行时速度。
/// 一个实例只用来编译一个字节码，下次编译需要重新创建实例。
/// </summary>
class Bagel_ReleaseCompiler
{
	list<CompileClo> clos;
	CompileClo *curclo;

	//Pointer位置，这些位置的变量仅tag有效，指向父级闭包
	set<int> fixed;
	//所有变量位置不重复
	int nextlocalpos;

	//在编译闭包函数时，上一个闭包
	Bagel_ReleaseCompiler *parent;

	int inclass;	//表示class的位置，=0表示不再class里
	bool inglobal;	//顶级var不优化

	int startStack;	//初始栈位置，一般为2，函数由于有参数占位会往大的方向移动

	int trycount;	//当前try的层级，return前手动清空

	BKE_hashmap<Bagel_Var, int> constpos;

	int getConstPos(Bagel_ByteCode &code, const Bagel_Var &v)
	{
		auto it = constpos.find(v);
		if (it == constpos.end())
		{
			int res = code.consts.size();
			constpos[v] = res;
			code.consts.emplace_back(v);
			return res;
		}
		else
		{
			return it->second;
		}
	}

	int getVarPos(Bagel_StringHolder str, int codepos);

	Bagel_TypeInfo *typeinfo;	//编译期类型推导
	list<Bagel_TypeInfo> typeinfos;

	//当前确定性的层级。（0表示当前代码一定被执行，>0表示不一定执行，如if，while...do，for，foreach，catch，switch。
	//另外如果不存在闭包函数，或闭包函数不改变类型，则函数调用不会影响局部变量类型。
	//否则对应变量们类型全置不确定（-1）。
	int typelevel;

	BKE_hashmap<int, int> toParentMap;	//闭包变量位置和原位置的对应
	set<int> beFixed;	//被子闭包函数改变类型的变量位置
	vector<Bagel_ByteCodeStruct> linkCode;	//初始化闭包变量的代码

	Bagel_ByteCode* curcode;
	
	//return must <= dest
	int _compile(int dest, Bagel_AST* subtree, Bagel_ByteCode &code, bool need_return, bool jit = false);

	void generateTypeLevel()
	{
		typelevel++;
		typeinfos.emplace_back(typeinfos.back());
		typeinfo = &typeinfos.back();
	}

	void deleteTypeLevel()
	{
		typelevel--;
		auto it = typeinfos.end();
		--it;
		if (it->flag == 0)
		{
			--it;
			it->merge(typeinfos.back());
		}
		else
		{
			--it;
			*it = typeinfos.back();
		}
		typeinfos.pop_back();
		typeinfo = &typeinfos.back();
	}

	int lastlooplevel;	//上一个firstloop的idx
	int infirstloop;	//是否在quickfor的第一个loop里，是的话，遇到break或continue将新开一个TypeLevel
	int lockcode;		//不生成代码，只做类型分析

	void mergeTwoTypeinfo(Bagel_TypeInfo &a, Bagel_TypeInfo &b)
	{
		a.merge(b);
		typelevel--;
		typeinfos.pop_back();
		typeinfos.back() = a;
		typeinfo = &typeinfos.back();
	}

public:
	Bagel_ReleaseCompiler();

	~Bagel_ReleaseCompiler();

	//jit在编译一些会重复运行的全局代码段时设为true
/// <summary>
/// 生成字节码。
/// </summary>
/// <param name="tree">抽象语法树。</param>
/// <param name="code">字节码。</param>
/// <param name="exp">原字符串，用来生成位置的调试信息。也可以后期手动给code的相应变量赋值。</param>
/// <param name="jit">是否进行cache优化，默认为false。编译只运行一次的字节码时，不需要设为true。当编译到函数或循环内的代码时，会自动打开。</param>
	void compile(Bagel_AST *tree, Bagel_ByteCode &code, const StringVal &exp = StringVal(), bool jit = false, bool varPosInfo = false);

	/// <summary>
	/// 生成字节码。
	/// </summary>
	/// <param name="tree">抽象语法树。</param>
	/// <param name="code">字节码。</param>
	/// <param name="exp">原字符串，用来生成位置的调试信息。也可以后期手动给code的相应变量赋值。</param>
	/// <param name="jit">是否进行cache优化，默认为false。编译只运行一次的字节码时，不需要设为true。当编译到函数或循环内的代码时，会自动打开。</param>
	void compileAddr(Bagel_AST *tree, Bagel_ByteCode &code, const StringVal &exp = StringVal(), bool jit = false, bool varPosInfo = false);
};