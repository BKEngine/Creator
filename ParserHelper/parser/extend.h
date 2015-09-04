#pragma once

#include "BKE_variable.h"
#include <fstream>

#ifdef HAS_REGEX
//for regex
#include <regex>
#endif

//class BKE_Variable;
//class BKE_VarClosure;
//class BKE_NativeClass;

#define _FUNC_PARAM		const BKE_Variable *self, const BKE_VarArray *paramarray, BKE_VarClosure *_this

#define CURRENTCLASS 

#define _BKETOWSTRING(x) L###x

#define NATIVE_FUNC(name) static BKE_Variable nativeFunc_##name(_FUNC_PARAM)
#define NATIVE_GET(name) static BKE_Variable nativeGet_##name(_FUNC_PARAM)
#define NATIVE_SET(name) static BKE_Variable nativeSet_##name(_FUNC_PARAM)
#define NATIVE_INIT() void mark(); virtual void nativeInit(const wstring &name) override
#define NATIVE_SAVE() virtual BKE_Variable nativeSave() override
#define NATIVE_LOAD() virtual void nativeLoad(const BKE_Variable &var) override
#define NATIVE_CREATENEW() virtual BKE_NativeClass* nativeCreateNew(const BKE_VarClass *self, const BKE_VarArray *paramarray) override
#define NATIVE_SUPPERINIT(_class) typedef _class super; const wchar_t * nativeSuperName = L###_class

#define NATIVECLASS_FUNC(name) BKE_Variable CURRENTCLASS::nativeFunc_##name(_FUNC_PARAM)
#define NATIVECLASS_GET(name) BKE_Variable CURRENTCLASS::nativeGet_##name(_FUNC_PARAM)
#define NATIVECLASS_SET(name) BKE_Variable CURRENTCLASS::nativeSet_##name(_FUNC_PARAM)
#define NATIVECLASS_INIT() void CURRENTCLASS::nativeInit(const wstring &name)
#define NATIVECLASS_SAVE() BKE_Variable CURRENTCLASS::nativeSave()
#define NATIVECLASS_LOAD() void CURRENTCLASS::nativeLoad(const BKE_Variable &var)
#define NATIVECLASS_CREATENEW() BKE_NativeClass* CURRENTCLASS::nativeCreateNew(const BKE_VarClass *self, const BKE_VarArray *paramarray)
#define NATIVECLASS_SUPERINIT() super::nativeInit(nativeSuperName)

#define REG_FUNC(name) _class->addNativeFunction(L###name, &CURRENTCLASS::nativeFunc_##name);
#define REG_FUNC_ALIAS(alias, name) _class->addNativeFunction(L###name, &CURRENTCLASS::nativeFunc_##name);
#define REG_GET(name) _class->addNativePropGet(L###name, &CURRENTCLASS::nativeGet_##name);
#define REG_SET(name) _class->addNativePropSet(L###name, &CURRENTCLASS::nativeSet_##name);

template<class T>
void _bkpCheckClassInstance(BKE_VarObject *__obj, T && __classname)
{
	if (!dynamic_cast<BKE_VarClass*>(__obj) || !((BKE_VarClass*)__obj)->isInstanceof(__classname))
		throw Var_Except(wstring(L"传入的类需要") + __classname + wstring(L"类或其派生类。"));
}

template < class T, class T2 >
T *_bkpGetClassInstance(BKE_VarObject *__obj, T2 && __classname)
{
	_bkpCheckClassInstance(__obj, __classname); 
	if (((BKE_VarClass*)__obj)->isdef)
		throw Var_Except(L"调用该方法的不是对象实例"); 
	T *__name = dynamic_cast<T*>(((BKE_VarClass*)__obj)->native);
	return __name;
}

#define GETCLASSINSTANCE(__obj, __class, __name) __class *__name = _bkpGetClassInstance<__class>(__obj, _BKETOWSTRING(__class))
#define CREATECLASSINSTANCE(__instance, __native, __class, ...) auto *__native=new __class(__VA_ARGS__); auto *__instance=new BKE_VarClass((BKE_VarClass *)(BKE_VarClosure::global()->getMember(L###__class).obj), __native); __native->_class = __instance;

#define CHECKCLASS(__classname) _bkpCheckClassInstance((_this), __classname)
#define GETINSTANCE() GETCLASSINSTANCE((_this), CURRENTCLASS, instance)

#define RETURNDEFAULT return BKE_Variable();
#define PARAMEXIST(n) (paramarray && paramarray->getCount() > (n))
#define PARAM(n) (paramarray->getMember(n))
#define PARAMDEFAULT(n, v) (PARAMEXIST(n)?(decltype(v))PARAM(n):v)
#define PARAMCOUNT() (paramarray ? paramarray->getCount() : 0);
#define MINIMUMPARAMNUM(n) if(!PARAMEXIST(n-1)) throw Var_Except(L"参数数目不足");

#define QUICKFUNC(f) L###f, &Parser_Util::nativeFunc_##f
#define QUICKGETTER(f) L###f, &Parser_Util::nativeGet_##f
#define QUICKSETTER(f) L###f, &Parser_Util::nativeSet_##f
#define QUICKCLASS(f) L###f, new Parser_Util::f()

#define PREPARECLASS() \
	auto &v=BKE_VarClosure::global()->getMember(name);	\
	if(v.getType()!=VAR_CLASS)							\
	{													\
		v.setVoid();									\
		v.vt=VAR_CLASS;									\
		if(_class)										\
			v.obj=new BKE_VarClass(name, _class);		\
		else											\
			v.obj = new BKE_VarClass(name);				\
		if(((BKE_VarClass*)v.obj)->native)				\
			delete ((BKE_VarClass*)v.obj)->native;		\
		((BKE_VarClass*)v.obj)->native = this;		\
	}													\
	_class = (BKE_VarClass*)v.obj;

#define DISABLECREATE() _class->cannotcreate=true;
#define ENABLECREATE() _class->cannotcreate=false;

namespace Parser_Util
{
	
	void registerExtend(Parser *p);

	/**
	*	数据类。
	*	{@b 可实例化。}
	*/
	class Date:public BKE_NativeClass
	{
	public:
		tm* _tm;
		
		NATIVE_INIT();

		NATIVE_CREATENEW();

		Date() :BKE_NativeClass(L"Date")
		{
			auto t = time(NULL);
			_tm = localtime(&t);
		};

		NATIVE_SAVE()
		{
			time_t t = mktime(_tm);
			return (bkplonglong)t;
		}

		NATIVE_LOAD()
		{
			time_t t = var.asInteger();
			_tm = localtime(&t);
		}

		/**
		 *  @return 数型 年份
		 *  @brief  返回当前的年份。
		 */
		NATIVE_FUNC(getYear);

		/**
		*  @param value
		*  @brief 设置当前的年份。
		*/
		NATIVE_FUNC(setYear);

		/**
		 *  @return 数型 月份
		 *  @brief  返回当前的月份(1-12)。
		 */
		NATIVE_FUNC(getMonth);

		/**
		*  @param value
		*  @brief 设置当前的月份(1-12)。
		*/
		NATIVE_FUNC(setMonth);

		/**
		 *  @return 数型 日期
		 *  @brief  返回当前的日期(1-31)。
		 */
		NATIVE_FUNC(getDay);

		/**
		*  @param value
		*  @brief 设置当前的日期(1-31)。
		*/
		NATIVE_FUNC(setDay);

		/**
		 *  @return 数型 小时
		 *  @brief  返回现在的小时(0-23)。
		 */
		NATIVE_FUNC(getHour);

		/**
		*  @param value
		*  @brief 设置当前的小时(0-23)。
		*/
		NATIVE_FUNC(setHour);

		/**
		 *  @return 数型 分钟
		 *  @brief  返回现在的分钟(0-59)。
		 */
		NATIVE_FUNC(getMinute);

		/**
		*  @param value
		*  @brief 设置当前的分钟(0-59)。
		*/
		NATIVE_FUNC(setMinute);

		/**
		 *  @return 数型 秒数
		 *  @brief  返回现在的秒数(0-59)。
		 */
		NATIVE_FUNC(getSecond);

		/**
		*  @param value
		*  @brief 设置当前的秒数(0-59)。
		*/
		NATIVE_FUNC(setSecond);

		/**
		*  @param  formatString
		*  @return 格式化后的字符串
		*  @brief  根据提供的格式化字串返回格式化后的字符串。
		           格式化字串里：%Y，%M，%D，%h，%m，%s分别表示全宽度的年份（四位）、月份（两位）、日期（两位）、24小时制的小时（两位）、分钟（两位）、秒（两位）
		                         %#Y，%#M，%#D，%#h，%#m，%#s分别表示压缩宽度，其中年份固定为两位，其余的在前面有无效0的时候会省略0
								 %%代表一个%
		*/
		NATIVE_FUNC(format);
	};

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

	/**
	*   @数学类。
	*   @singleton
	*   {@b 不可实例化。}
	*/
	class Math:public BKE_NativeClass
	{
	public:
		Math():BKE_NativeClass(L"Math"){};

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }

		/**
		 *  @brief 只读属性。
		 */
		NATIVE_GET(PI);

		/**
		 *  @brief 只读属性。
		 */
		NATIVE_GET(E);

		/**
		 *  @param var
		 *  @brief 符号函数,大于0返回1,等于0(误差10^-8)返回0,小于0返回-1。
		 */
		NATIVE_FUNC(sgn);

		/*
		 *  @param var
		 *  @brief 返回绝对值。
		 */
		NATIVE_FUNC(abs);

		/**
		 *  @param var
		 *  @brief 返回平方根。
		 */
		NATIVE_FUNC(sqrt);

		/**
		 *  @param var
		 *  @brief 返回以10为底的对数。
		 */
		NATIVE_FUNC(lg);

		/**
		 *  @param var
		 *  @brief 返回自然数。
		 */
		NATIVE_FUNC(ln);

		/**
		 *  @param base,var
		 *  @brief 以base为底的对数值。
		 */
		NATIVE_FUNC(log);

		/**
		 *  @param var
		 *  @brief 返回小于或等于var的最大整数。
		 */
		NATIVE_FUNC(floor);

		/**
		 *  @param var
		 *  @brief 返回大于或等于var的最小整数。
		 */
		NATIVE_FUNC(ceil);

		/**
		 *  @param var
		 *  @brief 返回返回四舍五入后的整数。
		 */
		NATIVE_FUNC(round);

		/**
		 *  @param var
		 *  @brief 返回返回正弦值,参数单位为弧度。
		 */
		NATIVE_FUNC(sin);

		/**
		 *  @param var
		 *  @brief 返回余弦值,参数单位为弧度。
		 */
		NATIVE_FUNC(cos);

		/**
		 *  @param var
		 *  @brief 返回正切值,参数单位为弧度。
		 */
		NATIVE_FUNC(tan);

		/**
		 *  @param var
		 *  @brief 返回反正弦值,参数单位为弧度。
		 */
		NATIVE_FUNC(asin);

		/**
		 *  @param var
		 *  @brief 返回反余弦值,参数单位为弧度。
		 */
		NATIVE_FUNC(acos);

		/**
		 *  @param var
		 *  @brief 返回反正切值,参数单位为弧度。
		 */
		NATIVE_FUNC(atan);

		/**
		 *  @return float 0-1的浮点数
		 *  @brief 返回0-1的浮点数。
		 */
		NATIVE_FUNC(random);

	};

#ifdef HAS_REGEX

	/**
	*   @正则表达式类。
	*   {@b 可实例化。}
	*   @brief 使用Regex(regstr[,ignorecase-false])创建。
	*/
	class Regex :public BKE_NativeClass
	{
	public:
		std::wregex reg;

		Regex(const wstring &r = L"", std::wregex::flag_type type = std::regex_constants::ECMAScript | std::regex_constants::optimize) :BKE_NativeClass(L"Regex"), reg(r, type){};

		NATIVE_INIT();

		NATIVE_CREATENEW();

		/**
		 *  @param string
		 *  @return array
		 *  @brief 返回所有匹配到的数组
		 *  @example 以下将演示一个从内容为
		 *  "a1b2c3"
		 *  的字符串中匹配数组的示例
		 *  @example_code Regex("\d").search("a1b2c3")
		 *  @example_result [1,2,3]
		 */
		NATIVE_FUNC(search);

		/**
		 *  @param string
		 *  @return bool
		 *  @brief 判断这个正则表达式是否可以全部匹配这个字符串,可以返回true,否则返回false
		 */
		NATIVE_FUNC(matchAll);

		/**
		 *  @param string
		 *  @return array
		 *  @brief 返回子匹配(即正则表达式里用()括住的字符串的匹配)数组。
		 *  @example 以下将演示一个从内容为
		 *  “abcd”
		 *  的字符串中匹配到的数组
		 *  @example_code Regex("(.)bc(.)").getSubMatch("abcd")
		 *  @example_result ["abcd","a","d"]
		 */
		NATIVE_FUNC(getSubMatch);

		/**
		 *  @param string,dst
		 *  @return string
		 *  @brief 将src里第一个匹配替换成dst，dst里可以使用\1,\2等表示子匹配，\0表示整个的匹配串。
		 *  @example 以下将演示一个从内容为
		 *  “abcd”
		 *  的字符串中匹配到的字符串
		 *  @example_code Regex("(.)bc(.)").replaceFirst("abcd","\0\1\2")
		 *  @example_result “abcdad”
		 */
		NATIVE_FUNC(replaceFirst);

		/**
		 *  @param string,dst
		 *  @return string
		 *  @brief 同replaceFirst,只是对所有匹配串都进行替换
		 */
		NATIVE_FUNC(replaceAll);
	};
#endif

	class Flags : public BKE_NativeClass
	{
	public:
		Flags() :BKE_NativeClass(L"Flags"){};
		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_FUNC(contains);
		NATIVE_FUNC(split);
		NATIVE_FUNC(mask);
		NATIVE_FUNC(merge);
	};
};