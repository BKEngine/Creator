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
#define REG_GET(name) _class->addNativePropGet(L###name, &CURRENTCLASS::nativeGet_##name);
#define REG_SET(name) _class->addNativePropSet(L###name, &CURRENTCLASS::nativeSet_##name);

#define CHECKCLASSINSTANCE(__obj, __classname) if(!dynamic_cast<BKE_VarClass*>(__obj) || !((BKE_VarClass*)__obj)->isInstanceof(__classname))throw Var_Except(wstring(L"传入的类需要")+__classname+wstring(L"类或其派生类。"))
#define GETCLASSINSTANCE(__obj, __class, __name) CHECKCLASSINSTANCE(__obj, _BKETOWSTRING(__class)); if(((BKE_VarClass*)__obj)->isdef)throw Var_Except(L"调用该方法的不是对象实例");__class *__name=dynamic_cast<__class*>(((BKE_VarClass*)__obj)->native);
#define CREATECLASSINSTANCE(__instance, __native, __class, ...) auto *__native=new __class(__VA_ARGS__); auto *__instance=new BKE_VarClass((BKE_VarClass *)(BKE_VarClosure::global()->getMember(L###__class).obj), __native); __native->_class = __instance;

#define CHECKCLASS(__classname) CHECKCLASSINSTANCE((_this), __classname)
#define GETINSTANCE() GETCLASSINSTANCE((_this), CURRENTCLASS, instance)

#define RETURNDEFAULT return BKE_Variable();
#define PARAMEXIST(n) (paramarray && paramarray->getMember(n).getType()!=VAR_NONE)
#define PARAM(n) (paramarray->getMember(n))
#define PARAMDEFAULT(n, v) (PARAMEXIST(n)?(decltype(v))PARAM(n):v)
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

		NATIVE_FUNC(getYear);
		NATIVE_FUNC(getMonth);
		NATIVE_FUNC(getDay);
		NATIVE_FUNC(getHour);
		NATIVE_FUNC(getMinute);
		NATIVE_FUNC(getSecond);
	};

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

	class Math:public BKE_NativeClass
	{
	public:
		Math():BKE_NativeClass(L"Math"){};

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }

		NATIVE_GET(PI);
		NATIVE_GET(E);
		NATIVE_FUNC(sgn);
		NATIVE_FUNC(abs);
		NATIVE_FUNC(sqrt);
		NATIVE_FUNC(lg);
		NATIVE_FUNC(ln);
		NATIVE_FUNC(log);
		NATIVE_FUNC(floor);
		NATIVE_FUNC(ceil);
		NATIVE_FUNC(round);
		NATIVE_FUNC(sin);
		NATIVE_FUNC(cos);
		NATIVE_FUNC(tan);
		NATIVE_FUNC(asin);
		NATIVE_FUNC(acos);
		NATIVE_FUNC(atan);
		NATIVE_FUNC(random);
	};

#ifdef HAS_REGEX
	class Regex :public BKE_NativeClass
	{
	public:
		std::wregex reg;

		Regex(const wstring &r = L"", std::wregex::flag_type type = std::regex_constants::ECMAScript | std::regex_constants::optimize) :BKE_NativeClass(L"Regex"), reg(r, type){};

		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_FUNC(search);
		NATIVE_FUNC(matchAll);
		NATIVE_FUNC(getSubMatch);
		NATIVE_FUNC(replaceFirst);
		NATIVE_FUNC(replaceAll);
	};
#endif
};