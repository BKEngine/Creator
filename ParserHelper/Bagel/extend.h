#pragma once


#include "Bagel_Var.h"
#include <fstream>

#ifdef HAS_REGEX
//for regex
#include <regex>
#endif

//class Bagel_Var;
//class Bagel_Closure;
//class Bagel_NativeClass;

#define CURRENTCLASS 

#define NATIVE_FUNC(name) static Bagel_Var nativeFunc_##name(_FUNC_PARAM)
#define NATIVE_GET(name) static Bagel_Var nativeGet_##name(_FUNC_PARAM)
#define NATIVE_SET(name) static Bagel_Var nativeSet_##name(_FUNC_PARAM)
#define NATIVE_INIT() void mark(); virtual void nativeInit(const u16string &name) override
#define NATIVE_SAVE() virtual Bagel_Var nativeSave() override
#define NATIVE_LOAD() virtual void nativeLoad(const Bagel_Var &var) override
#define NATIVE_CREATENEW() virtual Bagel_NativeClass* nativeCreateNew(const Bagel_Class *self, const Bagel_Var *paramarray, int paramcount, Bagel_ThreadContext *ctx) override
#define NATIVE_SUPPERINIT(_class) typedef _class super; const char16_t * nativeSuperName = W(#_class)
#define NATIVE_CREATENULL(classname) virtual Bagel_NativeClass* nativeCreateNULL() override{return new classname();}

#define NATIVECLASS_FUNC(name) Bagel_Var CURRENTCLASS::nativeFunc_##name(_FUNC_PARAM)
#define NATIVECLASS_GET(name) Bagel_Var CURRENTCLASS::nativeGet_##name(_FUNC_PARAM)
#define NATIVECLASS_SET(name) Bagel_Var CURRENTCLASS::nativeSet_##name(_FUNC_PARAM)
#define NATIVECLASS_INIT() void CURRENTCLASS::nativeInit(const u16string &name)
#define NATIVECLASS_SAVE() Bagel_Var CURRENTCLASS::nativeSave()
#define NATIVECLASS_LOAD() void CURRENTCLASS::nativeLoad(const Bagel_Var &var)
#define NATIVECLASS_CREATENEW() Bagel_NativeClass* CURRENTCLASS::nativeCreateNew(const Bagel_Class *self, const Bagel_Var *paramarray, int paramcount, Bagel_ThreadContext *ctx)
#define NATIVECLASS_SUPERINIT() _class = Bagel_Closure::global()->getMember(nativeSuperName).asClassDef();assert(_class); //子类一定要在父类之后初始化

#define _REG(c,name) \
		for(auto ptr = c##Native##name##List, end = c##Native##name##List + sizeof(c##Native##name##List)/sizeof(c##Native##name##List[0]); ptr < end; ptr++) \
		{ \
			 _class->addNative##name((*ptr).first, (*ptr).second); \
		} \

#define REG_FUNC_BEGIN(c)  static const pair<const char16_t *, Bagel_NativeFunction> c##NativeFunctionList[] =
#define REG_SET_BEGIN(c)  static const pair<const char16_t *, Bagel_NativeFunction> c##NativePropSetList[] =
#define REG_GET_BEGIN(c)  static const pair<const char16_t *, Bagel_NativeFunction> c##NativePropGetList[] =
#define REG_FUNC_END  
#define REG_SET_END  
#define REG_GET_END  

#define REG_FUNC(c) _REG(c,Function)
#define REG_SET(c) _REG(c,PropSet)
#define REG_GET(c) _REG(c,PropGet)

#define FUNC_INFO(name) {W2(name), &CURRENTCLASS::nativeFunc_##name}
#define FUNC_ALIAS_INFO(alias, name) {W2(alias), &CURRENTCLASS::nativeFunc_##name}
#define GET_INFO(name) {W2(name), &CURRENTCLASS::nativeGet_##name}
#define SET_INFO(name) {W2(name), &CURRENTCLASS::nativeSet_##name}

inline void _bkpCheckClassInstance(Bagel_Class *__obj, const char16_t *__classname)
{
	if (!__obj->isInstanceof(__classname))
		throw Bagel_Except(u16string(W("传入的类需要")) + __classname + W("类或其派生类。"));
}
inline void _bkpCheckClassInstance(Bagel_ClassDef *__obj, const StringVal &__classname)
{
	if (!__obj->isInstanceof(__classname))
		throw Bagel_Except(u16string(W("传入的类需要")) + __classname + W("类或其派生类。"));
}

template < class T >
T *_bkpGetClassInstance(Bagel_Object *__obj, const char16_t * __classname)
{
	if (__obj->vt == VAR_CLASS)
	{
		_bkpCheckClassInstance((Bagel_Class*)__obj, __classname);
		T *__name = dynamic_cast<T*>(((Bagel_Class*)__obj)->native);
		return __name;
	}
	else if (__obj->vt == VAR_CLASSDEF)
	{
		_bkpCheckClassInstance((Bagel_ClassDef*)__obj, __classname);
		T *__name = dynamic_cast<T*>(((Bagel_ClassDef*)__obj)->native);
		return __name;
	}
	else
	{
		throw Bagel_Except(W("无法转化为类"));
	}
}

#define GETCLASSINSTANCE(__obj, __class, __name) __class *__name = _bkpGetClassInstance<__class>(__obj, W2(__class))
#define CREATECLASSINSTANCE(__instance, __native, __class, ...) auto *__native=new __class(__VA_ARGS__); auto __instance=new Bagel_Class(Bagel_Closure::global()->getMember(W(#__class)).asClassDef()); __instance->native = __native; __native->_class = __instance;

#define CHECKCLASS(__classname) _bkpCheckClassInstance((_this), __classname)
#define GETINSTANCE() GETCLASSINSTANCE((self->forceAsObject()), CURRENTCLASS, instance)

#define RETURNDEFAULT return Bagel_Var();
#define PARAMEXIST(n) (paramcount > (n))
#define PARAM(n) (paramarray[n])
#define PARAMDEFAULT(n, v) (PARAMEXIST(n)?(decltype(v))paramarray[n]:v)
#define PARAMCOUNT() (paramcount)
#define MINIMUMPARAMNUM(n) if(paramcount<n) throw Bagel_Except(W("参数数目不足"));

#define QUICKFUNC(f) W2(f), &ParserUtils::nativeFunc_##f
#define QUICKGETTER(f) W2(f), &ParserUtils::nativeGet_##f
#define QUICKSETTER(f) W2(f), &ParserUtils::nativeSet_##f
#define QUICKCLASS(f) W2(f), new ParserUtils::f()

#define PREPARECLASS() this->_prepareClass(name)
	

#define DISABLECREATE() ((Bagel_ClassDef*)_class)->cannotcreate=true;
#define ENABLECREATE() ((Bagel_ClassDef*)_class)->cannotcreate=false;

class Bagel_VM;

namespace ParserUtils
{
	
	void registerExtend(Bagel_VM *p);

	/**
		\class Date
	 *	\brief 时间类。
	 *	 **可实例化**
	 *  \details 记录创建那一刻的时间。
		\construct Date()
		\{
	*/
	class Date:public Bagel_NativeClass
	{
	public:
		tm* _tm;
		
		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_CREATENULL(Date);

		Date() :Bagel_NativeClass(W("Date"))
		{
			auto t = time(NULL);
			_tm = localtime(&t);
		};

		NATIVE_SAVE()
		{
			time_t t = mktime(_tm);
			return (int64_t)t;
		}

		NATIVE_LOAD()
		{
			time_t t = var.asInteger();
			_tm = localtime(&t);
		}

		/**
			\function getYear
		 *  @return number 年份
		 *  @brief  返回当前的年份。
		 */
		NATIVE_FUNC(getYear);

		/**
			\function setYear
		*  @param number year
		*  @brief 设置当前的年份。
		*/
		NATIVE_FUNC(setYear);

		/**
		\function getMonth
		*  @return number 月份
		 *  @brief  返回当前的月份(1-12)。
		 */
		NATIVE_FUNC(getMonth);

		/**
		\function setMonth
		*  @param value
		*  @brief 设置当前的月份(1-12)。
		*/
		NATIVE_FUNC(setMonth);

		/**
		\function getDay
		*  @return number 日期
		 *  @brief  返回当前的日期(1-31)。
		 */
		NATIVE_FUNC(getDay);

		/**
		\function setDay
		*  @param value
		*  @brief 设置当前的日期(1-31)。
		*/
		NATIVE_FUNC(setDay);

		/**
		\function getHour
		*  @return number 小时
		 *  @brief  返回现在的小时(0-23)。
		 */
		NATIVE_FUNC(getHour);

		/**
		\function setHour
		*  @param value
		*  @brief 设置当前的小时(0-23)。
		*/
		NATIVE_FUNC(setHour);

		/**
		\function getMinute
		*  @return number 分钟
		 *  @brief  返回现在的分钟(0-59)。
		 */
		NATIVE_FUNC(getMinute);

		/**
		\function setMinute
		*  @param value
		*  @brief 设置当前的分钟(0-59)。
		*/
		NATIVE_FUNC(setMinute);

		/**
		\function getSecond
		*  @return number 秒数
		 *  @brief  返回现在的秒数(0-59)。
		 */
		NATIVE_FUNC(getSecond);

		/**
		\function setSecond
		*  @param value
		*  @brief 设置当前的秒数(0-59)。
		*/
		NATIVE_FUNC(setSecond);

		/**
		\function format
		*  @param  string formatString 格式字符串
		*  @return 格式化后的字符串
		*  @brief  根据提供的格式化字串返回格式化后的字符串。
		*  @body   格式化字串里：%Y，%M，%D，%h，%m，%s分别表示全宽度的年份（四位）、月份（两位）、日期（两位）、24小时制的小时（两位）、分钟（两位）、秒（两位）
								 %#Y，%#M，%#D，%#h，%#m，%#s分别表示压缩宽度，其中年份固定为两位，其余的在前面有无效0的时候会省略0
								 %%代表一个%
		*/
		NATIVE_FUNC(format);
	};

	/**
	@}
	*/

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

	/**
	*   数学类。
	*   @singleton
	*   {@b 不可实例化。}
	*/
	class Math:public Bagel_NativeClass
	{
	public:
		Math():Bagel_NativeClass(W("Math")){};

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }

		/**
		 *  @static
		 *  @brief 只读属性。
		 */
		NATIVE_GET(PI);

		/**
		 *  @static
		 *  @brief 只读属性。
		 */
		NATIVE_GET(E);

		/**
		 *  @static
		 *  @param number x
		 *  @return integer
		 *  @brief 符号函数,大于0返回1,等于0(误差10^-8)返回0,小于0返回-1。
		 */
		NATIVE_FUNC(sgn);

		/**
		 *  @static
		 *  @param number x
		 *  @param number y
		 *  @return number
		 *  @brief 指数函数，返回x^y
		*/
		NATIVE_FUNC(pow);

		/*
		 *  @static
		 *  @param number x
		 *  @return number
		 *  @brief 返回绝对值。
		 */
		NATIVE_FUNC(abs);

		/**
		 *  @static
		 *  @param number x
		 *  @return number
		 *  @brief 返回平方根。
		 */
		NATIVE_FUNC(sqrt);

		/**
		 *  @static
		 *  @param number x
		 *  @return number
		 *  @brief 返回以10为底的对数。
		 */
		NATIVE_FUNC(lg);

		/**
		 *  @static
		 *  @param number x
		 *  @return number
		 *  @brief 返回自然对数。
		 */
		NATIVE_FUNC(ln);

		/**
		 *  @static
		 *  @param number base
		 *  @param number x
		 *  @return number
		 *  @brief 返回以base为底的x的对数值。
		 */
		NATIVE_FUNC(log);

		/**
		 *  @static
		 *  @param number x
		 *  @return integer
		 *  @brief 返回小于或等于x的最大整数。
		 */
		NATIVE_FUNC(floor);

		/**
		 *  @static
		 *  @param number x
		 *  @return integer
		 *  @brief 返回大于或等于x的最小整数。
		 */
		NATIVE_FUNC(ceil);

		/**
		 *  @static
		 *  @param number x
		 *  @return integer
		 *  @brief 返回返回四舍五入后的整数。
		 */
		NATIVE_FUNC(round);

		/**
		*  @static
		*  @param number x
		*  @param number min
		*  @param number max
		*  @return integer
		*  @brief 返回以[min,max]的范围截取x的结果。
		*/
		NATIVE_FUNC(clamp);

		/**
		 *  @static
		 *  @param number x
		 *  @return number
		 *  @brief 返回返回正弦值,参数单位为弧度。
		 */
		NATIVE_FUNC(sin);

		/**
		 *  @static
		 *  @param number x
		 *  @return number
		 *  @brief 返回余弦值,参数单位为弧度。
		 */
		NATIVE_FUNC(cos);

		/**
		 *  @static
		 *  @param number x
		 *  @return number
		 *  @brief 返回正切值,参数单位为弧度。
		 */
		NATIVE_FUNC(tan);

		/**
		 *  @static
		 *  @param number x
		 *  @return number
		 *  @brief 返回反正弦值,参数单位为弧度。
		 */
		NATIVE_FUNC(asin);

		/**
		 *  @static
		 *  @param number x
		 *  @return number
		 *  @brief 返回反余弦值,参数单位为弧度。
		 */
		NATIVE_FUNC(acos);

		/**
		 *  @static
		 *  @param number x
		 *  @return number
		 *  @brief 返回反正切值,参数单位为弧度。
		 */
		NATIVE_FUNC(atan);

		/**
		 *  @static
		 *  @return number 0-1的浮点数
		 *  @brief 返回0-1的浮点数。和全局的random相比，这里采用的是随机数性能更好速度更慢些的mt19937算法。
		 */
		NATIVE_FUNC(random);

		/**
		 *  @static
		 *  @param number a
		 *  @param number b
		 *  @return number
		 *  @brief 返回a和b中的最小值。
		 */
		NATIVE_FUNC(min);

		/**
		 *  @static
		 *  @param number a
		 *  @param number b
		 *  @return number
		 *  @brief 返回a和b中的最大值。
		 */
		NATIVE_FUNC(max);

	};

#ifdef HAS_REGEX

	/**
	*   正则表达式类。
	*   {@b 可实例化。}
	*   @brief 使用Regex(regstr[,ignorecase=false])创建。
	*/
	class Regex :public Bagel_NativeClass
	{
	public:
		std::wregex reg;

		Regex(const wstring &r = wstring(), std::wregex::flag_type type = std::regex_constants::ECMAScript | std::regex_constants::optimize) :Bagel_NativeClass(W("Regex"))
		{
			try
			{
				reg = std::wregex(r, type);
			}
			catch (std::exception &e)
			{
				throw Bagel_Except(UTF16FromUTF7(e.what()));
			}
		};

		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_CREATENULL(Regex);

		/**
		 *  @param string str
		 *  @return array
		 *  @brief 返回所有匹配到的数组，匹配失败将返回空数组。
		 *  @example 以下将演示一个从内容为
		 *  "a1b2c3"
		 *  的字符串中匹配数组的示例
		 *  @example_code Regex("\d").search("a1b2c3")
		 *  @example_result [1,2,3]
		 */
		NATIVE_FUNC(search);

		/**
		 *  @param string str
		 *  @return bool
		 *  @brief 判断这个正则表达式是否可以全部匹配这个字符串,可以返回true,否则返回false。
		 */
		NATIVE_FUNC(matchAll);

		/**
		 *  @param string str
		 *  @return array
		 *  @brief 返回子匹配(即正则表达式里用()括住的字符串的匹配)数组，没有子匹配将返回空数组。
		 *  @example 以下将演示一个从内容为
		 *  "abcd"
		 *  的字符串中匹配到的数组
		 *  @example_code Regex("(.)bc(.)").getSubMatch("abcd")
		 *  @example_result ["abcd","a","d"]
		 */
		NATIVE_FUNC(getSubMatch);

		/**
		 *  @param string src 源字符串
		 *  @param string dst 目标字符串
		 *  @return string
		 *  @brief 将src里第一个匹配替换成dst，dst里可以使用\1,\2等表示子匹配，\0表示整个的匹配串。
		 *  @example 以下将演示一个从内容为
		 *  "abcd"
		 *  的字符串中匹配到的字符串
		 *  @example_code Regex("(.)bc(.)").replaceFirst("abcd","\0\1\2")
		 *  @example_result “abcdad”
		 */
		NATIVE_FUNC(replaceFirst);

		/**
		 *  @param string src 源字符串
		 *  @param string dst 目标字符串
		 *  @return string
		 *  @brief 同{@link replaceFirst},只是对所有匹配串都进行替换
		 */
		NATIVE_FUNC(replaceAll);
	};
#endif

	/**
	*   标记类。
	*   @singleton
	*   {@b 不可实例化。}
	*   @brief 用于处理按位设置标记的情况，最多0-31位共32个标记。
	*/
	class Flags : public Bagel_NativeClass
	{
	public:
		Flags() :Bagel_NativeClass(W("Flags")){};

		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_CREATENULL(Flags);

		/**
		*  @param number num
		*  @param number flags 要查找的标记的合集
		*  @return bool
		*  @brief 判断num里有没有flags里的所有标记
		*  @example 以下将演示一个从内容为
		*  0xF0 的标记中判断是否存在 0xC1 中的所有标记
		*  @example_code Flags.contains(0xF0, 0xC1)
		*  @example_result false
		*/
		NATIVE_FUNC(contains);

		/**
		*  @param number num
		*  @return array
		*  @brief 返回一个数组，包含num里所有存在的标记。
		*/
		NATIVE_FUNC(split);

		/**
		*  @param number num
		*  @param number flags 要查找的标记的合集
		*  @return bool
		*  @brief 返回num是否包含flags的某一标记。
		*  @example 以下将演示一个从内容为
		*  0xF0 的标记中判断是否存在 0xC1 中的某一标记
		*  @example_code Flags.mask(0xF0, 0xC1)
		*  @example_result true
		*/
		NATIVE_FUNC(mask);

		/**
		*  @prototype number num(, number flag1, number flag2, ...)
		*  @return number
		*  @brief 将num设置完所有参数中的标记后返回。
		*  @example 以下将演示一个从内容为
		*  0x02 的标记中合并上 0x01, 0x10
		*  @example_code Flags.merge(0x02, 0x01, 0x10)
		*  @example_result 0x13
		*/
		NATIVE_FUNC(merge);
	};

	/**
	*   协程类。
	*   @singleton
	*   {@b 不可实例化。}
	*   @brief 提供协程。
	*/
	class Coroutine : public Bagel_NativeClass
	{
	protected:
		Bagel_ThreadContext *ctx;
		Bagel_Function *func;

	public:
		Coroutine(Bagel_ThreadContext *c = NULL, Bagel_Function *f = NULL) :Bagel_NativeClass(W("Coroutine")), ctx(c), func(f)
		{
		};

		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_CREATENULL(Coroutine);

		virtual void markChildren() override;

		NATIVE_FUNC(resume);

		NATIVE_FUNC(restart);

		NATIVE_FUNC(stop);

		NATIVE_FUNC(finalize);

		NATIVE_GET(status);
	};
};