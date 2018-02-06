#pragma once

#include "Bagel_Utils.h"
#include "Bagel_GC.h"
#include "Bagel_Parser.h"

/// <summary>
/// Bagel_DebugCompiler，生成调试字节码的编译器类，调试字节码主要供调试器进行单步调试分析时用。
/// 一个实例只用来编译一个字节码，下次编译需要重新创建实例。
/// </summary>
class Bagel_DebugCompiler
{
	struct CompileClo
	{
		int clopos;	//当前闭包的位置
		int withpos;	//with里面变量的位置，没有则为-1，继承父闭包的位置
		//bool cancontinue;
		//bool canbreak;
		vector<int> *breakcode;
		vector<int> *continuecode;
	};

	list<CompileClo> clos;
	CompileClo *curclo;

	int trycount;	//当前try的层级，return前手动清空

	int inclass;	//表示class的位置，=0表示不再class里

	void _compile(int dest, Bagel_AST* subtree, Bagel_ByteCode &code);

public:
	/// <summary>
	/// 生成字节码。
	/// </summary>
	/// <param name="tree">抽象语法树。</param>
	/// <param name="code">字节码。</param>
	/// <param name="exp">原字符串，用来生成位置的调试信息。也可以后期手动给code的相应变量赋值。</param>
	/// @sa compileAddr
	void compile(Bagel_AST *tree, Bagel_ByteCode &code, const StringVal &exp = StringVal(), bool = false);

	/// <summary>
	/// 针对一个返回左值的表达式生成字节码，最后返回的结果是一个Bagel_Pointer，用于之后的赋值。
	/// </summary>
	/// <param name="tree">抽象语法树。</param>
	/// <param name="code">字节码。</param>
	/// <param name="exp">原字符串，用来生成位置的调试信息。也可以后期手动给code的相应变量赋值。</param>
	/// @sa compile
	void compileAddr(Bagel_AST *tree, Bagel_ByteCode &code, const StringVal &exp = StringVal(), bool = false);
};