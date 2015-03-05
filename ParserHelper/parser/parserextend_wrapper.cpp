#include "parser.h"
//#include "Script.h"
//#include "Text/Font.h"
//#include "Live2D/Live2DSprite.h"
#include "parserextend.h"
//#include "Archive/Archive.h"
//#include "Audio/Audio.h"
//#include "version.h"
//#include "Graphic.h"
//#include "Node/Button.h"
//#include "Node/Checkbox.h"
//#include "Node/Image.h"
//#include "Node/Layer.h"
//#include "Node/Slider.h"
//#include "Node/Sprite.h"
//#include "Node/MessageLayer.h"
//#include "Node/TextureCache.h"
//#include "Node/TextSprite.h"
//#include "ConfigExtension/JsonSupport.h"
//#include "Text/InputBox.h"
//#include "Network/HttpSupport.h"
//#include "BTL/ParserHelper.h"



//#if BKE_SYS_ANDROID
//#include <jnifunc.h>
//#endif

//BKE_Variable BKE_EvalFile(const wstring &filename, BKE_VarClosure *clo)
//{
//	wstring res;
//	int re = BKE_ReadPSRFile(filename, res);
//	if (re == 1)
//		throw Var_Except(L"打开文件失败");
//	else if (re == 2)
//		throw Var_Except(L"文件格式不正确或文件已损坏");
	//o_stderr<<L"file "<<filename<<L"\n"<<res<<L"\n";
//#if DEVELOP_VERSION
//	BKE_Variable a = Parser::getInstance()->evalMultiLineStr(res, clo);
//#else
//	BKE_Variable a = Parser::getInstance()->eval(res, clo);
//#endif
	//o_stderr<<L"file "<<filename<<L"\n"<<res<<L"\nresult:\n"<<a.save(true)<<L"\n";
//	return a;
//}

namespace Parser_Util
{
	//array.load(filename)
	NATIVE_FUNC(load)
	{
//		MINIMUMPARAMNUM(1);
/*//		return loadArray(PARAM(0));*/	RETURNDEFAULT;
	}

	//array.save(filename, mode)
	NATIVE_FUNC(save)
	{
//		MINIMUMPARAMNUM(1);
//		saveArray(*self, PARAM(0), PARAM(1));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//array.savestruct(filename, mode)
	//dic.savestruct(filename, mode)
	NATIVE_FUNC(saveStruct)
	{
//		MINIMUMPARAMNUM(1);
//		saveVariable(*self, PARAM(0), PARAM(1));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//loadFile(filename) return string
	NATIVE_FUNC(loadFile)
	{
//		MINIMUMPARAMNUM(1);
//		wstring fname = PARAM(0);
//		wstring res;
//		int re = BKE_ReadPSRFile(fname, res);
//		if (re == 1)
//			throw Var_Except(L"打开文件失败");
//		else if (re == 2)
//			throw Var_Except(L"文件格式不正确或文件已损坏");
/*//		return res;*/	RETURNDEFAULT;
	}

	//bool saveFile(filename, string, pos)
	//pos==-1, append
	NATIVE_FUNC(saveFile)
	{
//		MINIMUMPARAMNUM(2);
//		wstring fname = PARAM(0);
//		const wstring &res = PARAM(1).asBKEStr().getConstStr();
//		bklong pos = 0;
//		if (PARAMEXIST(2))
//			pos = PARAM(2);
//		BKE_File f;
		//if pos=0, we create a new file always
//		if (!pos)
//			f.open(fname, false, false);
//		else
//			f.open(fname, false, true);
//		if (!f)
//			throw Var_Except(L"打开文件失败");
//		if (pos >= 0)
//			f.seek(pos, SEEK_SET);
//		else
//			f.seek(pos + 1, SEEK_END);
//		string buf = UniToUTF8(res, true);
//		f.write(buf.c_str(), buf.size(), false);
//		f.close();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//appendFile(filename, string)
	NATIVE_FUNC(appendFile)
	{
//		MINIMUMPARAMNUM(2);
//		wstring fname = PARAM(0);
//		const wstring &res = PARAM(1).asBKEStr().getConstStr();
//		BKE_File f;
//		f.open(fname, false, true);
//		if (!f)
//			throw Var_Except(L"打开文件失败");
//		string buf = UniToUTF8(res, false);
//		f.write(buf.c_str(), buf.size(), true);
//		f.close();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//xxx = evalFile(filename)
	NATIVE_FUNC(evalFile)
	{
//		if (!PARAMEXIST(0))
//			return BKE_Variable();
//		bool krmode = Parser::getInstance()->krmode;
//		bool rawstr = Parser::getInstance()->rawstr;
//		Parser::getInstance()->krmode = PARAM(1);
//		Parser::getInstance()->rawstr = PARAM(2);
//		BKE_Variable res;
//		try
//		{
//			res = BKE_EvalFile(PARAM(0));
//		}
//		catch (Var_Except &e)
//		{
//			Parser::getInstance()->krmode = krmode;
//			Parser::getInstance()->rawstr = rawstr;
//			throw e;
//		}
//		Parser::getInstance()->krmode = krmode;
//		Parser::getInstance()->rawstr = rawstr;
/*//		return res;*/	RETURNDEFAULT;
	}

	//bool fileExist(filename)
	NATIVE_FUNC(fileExist)
	{
//		MINIMUMPARAMNUM(1);
//		auto s = BKE_Archive::getInstance()->exists(PARAM(0));
//		if (!s)
//			return false;
/*//		return true;*/	RETURNDEFAULT;
	}

	//array.load(filename)
	NATIVE_FUNC(log_ol)
	{
//		if (!PARAMEXIST(0))
//			RETURNDEFAULT;
//		wstring res;
//		PARAM(0).save(res);
//		FORCEDIRECTADD(res);
//		for (int i = 1; i < paramarray->getCount(); i++)
//		{
//			res.clear();
//			PARAM(i).save(res);
//			FORCEDIRECTADD(L',', res);
//		}
//		FORCECOMMIT();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//System object
#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS System
#endif
//	BKE_SystemConfig *System::cfg = NULL;

	void System::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
//		cfg = BKE_SystemConfig::getInstance();
		REG_GET(appName);
		REG_GET(appPath);
		REG_GET(appFullName);
		REG_GET(showingText);
		REG_GET(transCount);
		REG_GET(actionCount);
		REG_GET(gameName);
		REG_GET(gameTitle);
		REG_SET(gameTitle);
		REG_GET(saveHistory);
		REG_SET(saveHistory);
		REG_GET(autoMode);
		REG_SET(autoMode);
		REG_GET(autoModeTime);
		REG_SET(autoModeTime);
		REG_GET(skipMode);
		REG_SET(skipMode);
		REG_GET(skipAll);
		REG_SET(skipAll);
		REG_GET(maxFrameSpeed);
		REG_SET(maxFrameSpeed);
		REG_GET(textSpeed);
		REG_SET(textSpeed);
		REG_GET(transitionEnabled);
		REG_SET(transitionEnabled);
		REG_GET(fontSize);
		REG_SET(fontSize);
		REG_GET(fontColor);
		REG_SET(fontColor);
		REG_GET(fontStyle);
		REG_SET(fontStyle);
		REG_GET(fullScreen);
		REG_SET(fullScreen);
		REG_GET(FPSEnabled);
		REG_SET(FPSEnabled);
		REG_GET(fontSizeFactor);
		REG_SET(fontSizeFactor);
		REG_GET(touchPointEnabled);
		REG_SET(touchPointEnabled);
		REG_GET(windowSize);
		REG_SET(windowSize);
		REG_GET(mouseCursor);
		REG_SET(mouseCursor);
		REG_GET(mouseCursorAutoHideTime);
		REG_SET(mouseCursorAutoHideTime);
		REG_GET(mouseWheel);
		REG_GET(mouseStatus);
		REG_GET(resolutionSize);
		REG_GET(stable);

		REG_GET(version);
		REG_GET(platform);

		REG_FUNC(getKeyState);
		REG_FUNC(getKeyboardState);
		REG_FUNC(getEnv);
		REG_FUNC(setEnv);
		REG_FUNC(exit);
		REG_FUNC(run);
		REG_FUNC(openUrl);
	}

	NATIVECLASS_GET(appName)
	{
/*//		return cfg->exeName;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(appPath)
	{
/*//		return cfg->exePath;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(appFullName)
	{
/*//		return cfg->exeFullName;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(showingText)
	{
/*//		return (bool)cfg->isShowingText;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(transCount)
	{
/*//		return (int)cfg->trans.size();*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(actionCount)
	{
/*//		return (int)CCDirector::sharedDirector()->getActionManager()->count;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(gameName)
	{
/*//		return cfg->gameName;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(gameTitle)
	{
/*//		return cfg->getGameTitle();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(gameTitle)
	{
//		MINIMUMPARAMNUM(1);
//		cfg->setGameTitle(PARAM(0).asString());
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(saveHistory)
	{
/*//		return (bool)cfg->saveHistory;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(saveHistory)
	{
//		MINIMUMPARAMNUM(1);
//		cfg->saveHistory = (bool)PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(autoMode)
	{
/*//		return sAutomode;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(autoMode)
	{
//		MINIMUMPARAMNUM(1);
//		sAutomode = (bool)PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(autoModeTime)
	{
/*//		return cfg->getAutoModeTime();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(autoModeTime)
	{
//		MINIMUMPARAMNUM(1);
//		cfg->setAutoModeTime(PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(skipMode)
	{
/*//		return sSkip;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(skipMode)
	{
//		MINIMUMPARAMNUM(1);
//		sSkip = (bool)PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(skipAll)
	{
/*//		return sSkipAll;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(skipAll)
	{
//		MINIMUMPARAMNUM(1);
//		sSkipAll = (bool)PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(maxFrameSpeed)
	{
/*//		return cfg->getFrameSpeed();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(maxFrameSpeed)
	{
//		MINIMUMPARAMNUM(1);
//		cfg->setFrameSpeed(PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(textSpeed)
	{
/*//		return cfg->getTextSpeed();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(textSpeed)
	{
//		MINIMUMPARAMNUM(1);
//		cfg->setTextSpeed(PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(windowSize)
	{
//		CCSize s = cfg->getWindowSize();
//		BKE_Variable v = BKE_Variable::array(2);
//		v[0] = s.width;
//		v[1] = s.height;
/*//		return v;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(windowSize)
	{
//		MINIMUMPARAMNUM(1);
//		auto &v = PARAM(0);
//		CCSize s = CCSizeMake(v[0], v[1]);
//		cfg->setWindowSize(s);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(mouseCursor)
	{
//		MINIMUMPARAMNUM(1);
//		cfg->setCursor(PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(mouseCursor)
	{
/*//		return cfg->getCursor();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(mouseCursorAutoHideTime)
	{
//		MINIMUMPARAMNUM(1);
//		cfg->setCursorAutoHideTime(PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(mouseCursorAutoHideTime)
	{
/*//		return cfg->getCursorAutoHideTime();*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(transitionEnabled)
	{
/*//		return (bool)cfg->isTransitionEnabled;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(transitionEnabled)
	{
//		MINIMUMPARAMNUM(1);
//		cfg->isTransitionEnabled = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontSize)
	{
/*//		return BKE_Graphic::getInstance()->getCurrentMessageLayer()->getFontSize();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontSize)
	{
//		MINIMUMPARAMNUM(1);
//		BKE_Graphic::getInstance()->getCurrentMessageLayer()->setFontSize(PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}
	NATIVECLASS_GET(fontColor)
	{
/*//		return varFromFontColor(BKE_Graphic::getInstance()->getCurrentMessageLayer()->getFontColor());*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontColor)
	{
//		MINIMUMPARAMNUM(1);
//		BKE_FontColor c;
//		if (varToFontColor(c,PARAM(0)))
//		{
//			BKE_Graphic::getInstance()->getCurrentMessageLayer()->setFontColor(c);
//			RETURNDEFAULT;
//		}
//		throw(Var_Except(L"错误的FontColor类型。应该是一个整数或者含有两个整数的数组。"));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}
	NATIVECLASS_GET(fontStyle)
	{
/*//		return BKE_Graphic::getInstance()->getCurrentMessageLayer()->getFontStyle();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontStyle)
	{
//		MINIMUMPARAMNUM(1);
//		BKE_Graphic::getInstance()->getCurrentMessageLayer()->setFontStyle((bkulong)PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(fullScreen)
	{
/*//		return cfg->getFullScreen();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(fullScreen)
	{
//		MINIMUMPARAMNUM(1);
//		auto &v = PARAM(0);
//		cfg->setFullScreen(v);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(FPSEnabled)
	{
/*//		return cfg->getFPSEnabled();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(FPSEnabled)
	{
//		MINIMUMPARAMNUM(1);
//		auto &v = PARAM(0);
//		cfg->setFPSEnabled(v);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontSizeFactor)
	{
/*//		return cfg->getFontSizeFactor();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontSizeFactor)
	{
//		MINIMUMPARAMNUM(1);
//		auto &v = PARAM(0);
//		cfg->setFontSizeFactor(v);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(touchPointEnabled)
	{
/*//		return cfg->getTouchPointEnabled();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(touchPointEnabled)
	{
//		MINIMUMPARAMNUM(1);
//		auto &v = PARAM(0);
//		cfg->setTouchPointEnabled(v);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(mouseWheel)
	{
/*//		return (int)cfg->mousewheel;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(mouseStatus)
	{
//		auto arr = new BKE_VarArray();
//		arr->pushMember((int)cfg->mousestatus[0]);
//		arr->pushMember((int)cfg->mousestatus[1]);
//		arr->pushMember((int)cfg->mousestatus[2]);
//		arr->pushMember((int)cfg->mousestatus[3]);
//		arr->pushMember((int)cfg->mousestatus[4]);
/*//		return arr;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(resolutionSize)
	{
//		BKE_VarArray * arr = new BKE_VarArray;
//		arr->pushMember(cfg->resolutionSize.width);
//		arr->pushMember(cfg->resolutionSize.height);
/*//		return arr;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(stable)
	{
/*//		return BKE_Script::sharedScript()->isStable();*/	RETURNDEFAULT;
	}
		
	NATIVECLASS_GET(version)
	{
/*//		return BKE_VERSION;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(platform)
	{
/*//		return getCurrentPlatformString();*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getKeyState)
	{
//#if BKE_SYS_WINDOWS
//		MINIMUMPARAMNUM(1);
//		return (int)(GetKeyState(PARAM(0)) & 0xFF);
//#else
//		RETURNDEFAULT;
/*#endif*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getKeyboardState)
	{
//#if BKE_SYS_WINDOWS
//		BYTE status[256];
//		GetKeyboardState(status);
//		auto arr = new BKE_VarArrayTemplate<8>();
//		arr->setLength(256);
//		for(int i=0;i<256;i++)
//			arr->quickSetMember(i, (int)(status[i] & 0xFF));
//		return arr;
//#else
//		RETURNDEFAULT;
/*#endif*/	RETURNDEFAULT;
	}

	//getEnv(name)
	NATIVECLASS_FUNC(getEnv)
	{
//		MINIMUMPARAMNUM(1);
// 		auto it = cfg->env.find(PARAM(0).asBKEStr());
// 		if (it != cfg->env.end())
// 			return it->second;
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//setEnv(name, value)
	NATIVECLASS_FUNC(setEnv)
	{
//		MINIMUMPARAMNUM(2);
//		/*auto it = cfg->env[PARAM(0).asBKEStr()] = PARAM(1).asBKEStr();*/
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(exit)
	{
//		bool force = true;
//		if (paramarray != NULL)
//			force = PARAM(0);
//		if (force)
//		{
//			CCDirector::sharedDirector()->end();
//		}
//		else
//		{
//			CCApplication::sharedApplication()->onExit();
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(run)
	{
//		MINIMUMPARAMNUM(1);
//#if BKE_SYS_WINDOWS
//		ShellExecuteW(NULL, NULL, PARAM(0).asBKEStr().c_str(), PARAMEXIST(1) ? PARAM(1).asBKEStr().c_str() : NULL, L"", SW_SHOWNORMAL);
//#elif !BKE_SYS_WP8
//		string cmd = UniToUTF8(PARAM(0).asBKEStr().getConstStr());
//		return system(cmd.c_str());
//#else 
//#endif
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(openUrl)
	{
//		MINIMUMPARAMNUM(1);
//#if BKE_SYS_WINDOWS
//		ShellExecuteW(NULL, NULL, PARAM(0).asBKEStr().c_str(), NULL, L"", SW_SHOWNORMAL);
//#elif BKE_SYS_ANDROID
//		string cmd = UniToUTF8(PARAM(0).asBKEStr().getConstStr());
//		openUrl(cmd.c_str());
//		RETURNDEFAULT;
//#else
//#endif
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}
	
//	BKE_Script *Scripts::script = NULL;

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Scripts
#endif

	void Scripts::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
//		script = BKE_Script::sharedScript();
		REG_FUNC(showText);
		REG_FUNC(runCmd);
		REG_FUNC(goto);
		REG_FUNC(go);
		_class->addNativeFunction(L"jump", &Scripts::nativeFunc_goto);
		REG_FUNC(call);
		REG_FUNC(callback);
		REG_FUNC(return);
		REG_FUNC(returnTo);
		REG_FUNC(getCurrentRead);
		REG_FUNC(isLabelReaded);
		REG_GET(curLabel);
		REG_GET(curScript);
		REG_GET(actualCurLabel);
		REG_GET(actualCurScript);
	}

	//showText(text)
	NATIVECLASS_FUNC(showText)
	{
//		MINIMUMPARAMNUM(1);
//		if (PARAM(1).getType()==VAR_FUNC)
//			script->showText(PARAM(0), (BKE_VarFunction*)PARAM(1).obj);
//		else
//			script->showText(PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

//	void handleError(bkeLogLevel err)
//	{
//		if (err != bkeLogNone)
//		{
//			if (err == bkeLogError)
//			{
//				wstring str = BKE_LogHandlerCenter::getInstance()->getCurrentCache();
//				BKE_LogHandlerCenter::getInstance()->clearCache();
//				throw Var_Except(str);
//			}
//			Scripts::script->handleError(err);
//		}
//	}

	//runCmd(name, param)
	NATIVECLASS_FUNC(runCmd)
	{
//		MINIMUMPARAMNUM(1);
//		bkeLogLevel err;
//		if (PARAM(1).getType() != VAR_DIC)
//		{
//			BKE_hashmap<BKE_String, BKE_Variable> tmpmap;
//			err = script->callCommandHandler(PARAM(0), tmpmap, _this, PARAM(2).getType() == VAR_FUNC ? (BKE_VarFunction *)PARAM(2).obj : NULL);
			//throw Var_Except(L"参数2必须是字典。");
//		}
//		else
//		{
//			err = script->callCommandHandler(PARAM(0), ((BKE_VarDic *)(PARAM(1).obj))->varmap, _this, PARAM(2).getType() == VAR_FUNC ? (BKE_VarFunction *)PARAM(2).obj : NULL);
//		}
//#ifdef DEVELOP_VERSION
//		Native_log(self, paramarray, _this);
//		wcout << L"result:" << (err == BKE_NO_ERROR) << L'\n';
//#endif
//		handleError(err);
/*//		return true;*/	RETURNDEFAULT;
	}

	//Goto(label, script)
	NATIVECLASS_FUNC(goto)
	{
//		MINIMUMPARAMNUM(2);
//		auto err = script->jumpHandler(PARAM(0), PARAM(1));
//		handleError(err);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(go)
	{
//		script->getCurWaitInfo().waitFlag = BKE_Script::NOWAITFlag;
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//Call(label, script)
	NATIVECLASS_FUNC(call)
	{
//		MINIMUMPARAMNUM(1);
//		auto err = script->callHandler(PARAM(0).asString(), PARAM(1).asString());
//		handleError(err);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(callback)
	{
//		MINIMUMPARAMNUM(1);
//		auto err = script->callbackHandler(PARAM(0).asString(), PARAM(1).asString(), PARAMDEFAULT(2, true), PARAMDEFAULT(3, L""), PARAM(4).getType() == VAR_DIC ? (BKE_VarDic *)PARAM(4).obj : NULL, PARAMDEFAULT(5, false));
//		handleError(err);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//Return()
	NATIVECLASS_FUNC(return)
	{
//		auto err = script->Return();
//		if (err != bkeLogNone)
//		{
//			script->handleError(err);
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//ReturnTo(label, script)
	NATIVECLASS_FUNC(returnTo)
	{
//		MINIMUMPARAMNUM(2);
//		auto err = script->Return();
//		if (err != bkeLogNone)
//		{
//			script->handleError(err);
//			RETURNDEFAULT;
//		}
//		err = script->jumpHandler(PARAM(0), PARAM(1));
//		handleError(err);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getCurrentRead)
	{
/*//		return script->getCurrentRead();*/	RETURNDEFAULT;
	}

	//IsLabelReaded(label, script)
	NATIVECLASS_FUNC(isLabelReaded)
	{
/*//		return script->isLabelReaded(PARAM(0), PARAM(1));*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(curLabel)
	{
//		if (script->cur_script == NULL)
//			throw Var_Except(L"当前没有在执行的脚本");
/*//		return script->curlabel;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(curScript)
	{
//		if (script->cur_script == NULL)
//			return L"";
/*//		return script->curscript;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(actualCurLabel)
	{
//		if (script->cur_script == NULL)
//			throw Var_Except(L"当前没有在执行的脚本");
/*//		return script->offsets.back().label;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(actualCurScript)
	{
//		if (script->cur_script == NULL)
//			return L"";
/*//		return script->cur_script->getRawFileName();*/	RETURNDEFAULT;
	}

//	BKE_Script *SaveData::script = NULL;

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS SaveData
#endif

	void SaveData::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
//		script = BKE_Script::sharedScript();
		REG_FUNC(exist);
		REG_FUNC(getTime);
		REG_FUNC(getText);
		REG_FUNC(getCurrentSave);
	}

	//exist(num)
	NATIVECLASS_FUNC(exist)
	{
//		MINIMUMPARAMNUM(1);
//		bklong num = PARAM(0);
//		if (num < 0)
//			return false;
//		num %= script->cfg->maxSaveDataNum;
/*//		return !script->savedata[num].isVoid();*/	RETURNDEFAULT;
	}

	//getTime(num)
	NATIVECLASS_FUNC(getTime)
	{
//		MINIMUMPARAMNUM(1);
//		bklong num = PARAM(0);
//		if (num < 0)
//			RETURNDEFAULT;
//		num %= script->cfg->maxSaveDataNum;
//		auto &&t = script->savedata[num];
//		if (t.isVoid())
//			RETURNDEFAULT;
//		bklonglong tt = t[s_savetime];
//		CREATECLASSINSTANCE(d, n, Date);
//		n->_tm = localtime((time_t *)&tt);
/*//		return d;*/	RETURNDEFAULT;
	}

	//getText(num)
	NATIVECLASS_FUNC(getText)
	{
//		MINIMUMPARAMNUM(1);
//		bklong num = PARAM(0);
//		if (num < 0)
//			RETURNDEFAULT;
//		num %= script->cfg->maxSaveDataNum;
//		auto &&t = script->savedata[num];
//		if (t.isVoid())
//			RETURNDEFAULT;
/*//		return t[s_text];*/	RETURNDEFAULT;
	}

	//GetCurrentSave()
	NATIVECLASS_FUNC(getCurrentSave)
	{
/*//		return script->savedata.clone();*/	RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Channel
#endif

//	BKE_Mutex *Channel::mu = NULL;
//	BKE_Audio *Channel::audio = NULL;

	void Channel::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
//		audio = BKE_Audio::getInstance();
//		mu = &audio->mutex;
		REG_GET(isPlaying);
		REG_GET(isPaused);
		REG_GET(loopTimes);
		REG_SET(loopTimes);
		REG_GET(loopTo);
		REG_SET(loopTo);
		REG_GET(volume);
		REG_SET(volume);
		REG_GET(fileName);
		REG_GET(musicVolume);
		REG_SET(musicVolume);
		REG_GET(soundVolume);
		REG_SET(soundVolume);
		REG_GET(voiceVolume);
		REG_SET(voiceVolume);
		REG_FUNC(load);
		REG_FUNC(play);
		REG_FUNC(pause);
		REG_FUNC(resume);
		REG_FUNC(stop);
		REG_FUNC(tell);
		REG_FUNC(seek);
	}

	NATIVECLASS_CREATENEW()
	{
		//if (paramarray == NULL)
		//	return new Channel(0);
		//if (PARAM(0) == s_bgm)
		//	return new Channel(-1);
		//if (PARAM(0) == s_voice)
		//	return new Channel(-2);
		return new Channel(paramarray->getMember(0));
	};

	NATIVECLASS_GET(isPlaying)
	{
//		GETINSTANCE();
//		BKE_MutexLocker ml(*mu);
//		auto s = audio->get_channel(instance->channel_index);
//		if (!s)
//			return false;
/*//		return s->is_playing();*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(isPaused)
	{
//		GETINSTANCE();
//		BKE_MutexLocker ml(*mu);
//		auto s = audio->get_channel(instance->channel_index);
//		if (!s)
//			return false;
/*//		return s->is_paused();*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(loopTimes)
	{
//		GETINSTANCE();
//		BKE_MutexLocker ml(*mu);
//		auto s = audio->get_channel(instance->channel_index);
//		if (!s)
//			return false;
/*//		return s->loop;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(loopTimes)
	{
//		GETINSTANCE();
//		BKE_MutexLocker ml(*mu);
//		auto s = audio->get_channel(instance->channel_index);
//		if (s)
//			s->loop = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(loopTo)
	{
//		GETINSTANCE();
//		BKE_MutexLocker ml(*mu);
//		auto s = audio->get_channel(instance->channel_index);
//		if (!s)
//			return false;
/*//		return s->loopto;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(loopTo)
	{
//		GETINSTANCE();
//		BKE_MutexLocker ml(*mu);
//		auto s = audio->get_channel(instance->channel_index);
//		if (s)
//			s->loopto = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(volume)
	{
//		GETINSTANCE();
//		BKE_MutexLocker ml(*mu);
//		auto s = audio->get_channel(instance->channel_index);
//		if (!s)
//			return false;
/*//		return s->get_volume();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(volume)
	{
//		GETINSTANCE();
//		BKE_MutexLocker ml(*mu);
//		auto s = audio->get_channel(instance->channel_index);
//		if (s)
//			s->set_volume(PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(fileName)
	{
//		GETINSTANCE();
//		BKE_MutexLocker ml(*mu);
//		auto s = audio->get_channel(instance->channel_index);
//		if (!s)
//			return false;
/*//		return s->filename;*/	RETURNDEFAULT;
	}

	//for static function
	NATIVECLASS_GET(musicVolume)
	{
/*//		return audio->music_volume(-1);*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(musicVolume)
	{
//		MINIMUMPARAMNUM(1);
/*//		return audio->music_volume(PARAM(0));*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(soundVolume)
	{
/*//		return audio->sound_volume(-1);*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(soundVolume)
	{
//		MINIMUMPARAMNUM(1);
/*//		return audio->sound_volume(PARAM(0));*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(voiceVolume)
	{
/*//		return audio->voice_volume(-1);*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(voiceVolume)
	{
//		MINIMUMPARAMNUM(1);
/*//		return audio->voice_volume(PARAM(0));*/	RETURNDEFAULT;
	}

	//load(file)
	NATIVECLASS_FUNC(load)
	{
//		GETINSTANCE();
//		bkeLogLevel err = audio->load_sound_on_a_channel(PARAM(0), instance->channel_index);
//		handleError(err);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//play([file], [looptime])
	NATIVECLASS_FUNC(play)
	{
//		GETINSTANCE();
//		wstring found_path;
//		bkeLogLevel e;
//		if (PARAMEXIST(0))
//		{
//			bklong vol = !PARAMEXIST(1) ? 100 : PARAM(1).asInteger();
//			switch (instance->channel_index)
//			{
//			case MUSIC_CHANNEL:
//				e = audio->play_music(PARAM(0).asString(), vol, PARAM(2));
//				break;
//			case VOICE_CHANNEL:
//				e = audio->play_voice(PARAM(0).asString(), vol);
//				break;
//			default:
//				e = audio->play_sound(PARAM(0), instance->channel_index, vol, PARAM(2), true);
//				break;
//			}
//		}
//		else
//		{
//			bklong vol = !PARAMEXIST(1) ? 100 : PARAM(1).asInteger();
//			e = audio->play(instance->channel_index, vol, 0, true);
//		}
//		handleError(e);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//pause()
	NATIVECLASS_FUNC(pause)
	{
//		GETINSTANCE();
//		audio->pause(instance->channel_index);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//resume()
	NATIVECLASS_FUNC(resume)
	{
//		GETINSTANCE();
//		audio->resume(instance->channel_index);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//stop(fadetime)
	NATIVECLASS_FUNC(stop)
	{
//		GETINSTANCE();
//		audio->stop(instance->channel_index, !paramarray ? 0 : (bklong)PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//seek(time)
	NATIVECLASS_FUNC(seek)
	{
//		GETINSTANCE();
//		BKE_MutexLocker ml(*mu);
//		auto s = audio->get_channel(instance->channel_index);
//		if (!s)
//		{
//			return false;
//		}
//		s->seek(PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//double tell()
	NATIVECLASS_FUNC(tell)
	{
//		GETINSTANCE();
//		BKE_MutexLocker ml(*mu);
//		auto s = audio->get_channel(instance->channel_index);
//		if (!s)
//		{
//			return false;
//		}
/*//		return s->tell();*/	RETURNDEFAULT;
	}

//	BKE_History *History::his = NULL;

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS History
#endif

	void History::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
//		his = BKE_History::getInstance();
		REG_GET(data);
		REG_GET(startIndex);
		REG_GET(curIndex);
		REG_GET(nextIndex);
		REG_GET(count);
		REG_GET(enabled);
		REG_SET(enabled);
		REG_GET(maxPage);
		REG_SET(maxPage);
		REG_GET(recordMode);
		REG_SET(recordMode);
		REG_GET(maxChars);
		REG_SET(maxChars);
		REG_FUNC(getDataAtLine);
		REG_FUNC(reline);
		REG_FUNC(repage);
		REG_FUNC(add);
	}

	NATIVECLASS_GET(data)
	{
/*//		return his->data;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(enabled)
	{
/*//		return his->enable;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(enabled)
	{
//		MINIMUMPARAMNUM(1);
//		his->enable = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(maxPage)
	{
/*//		return his->maxPages;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(maxPage)
	{
//		MINIMUMPARAMNUM(1);
//		his->maxPages = PARAM(0);
//		if (his->startIndex > 0)
//			his->clear();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(startIndex)
	{
/*//		return his->startIndex;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(curIndex)
	{
/*//		return his->nextIndex == 0 ? his->maxPages : his->nextIndex - 1;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(nextIndex)
	{
/*//		return his->nextIndex;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(count)
	{
/*//		return his->getHistoryCount();*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(recordMode)
	{
//		static const BKE_String mode[] = { L"pageMode", L"lineMode", L"pageTagMode" };
/*//		return mode[his->recordMode];*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(recordMode)
	{
//		static const BKE_String mode[] = { L"pageMode", L"lineMode", L"pageTagMode" };
//		MINIMUMPARAMNUM(1);
//		if (PARAM(0).getType() == VAR_STR)
//		{
//			if (PARAM(0) == mode[his->recordMode])
//				RETURNDEFAULT;
//			if (PARAM(0) == mode[0])
//				his->changeRecodeMode(0);
//			else if (PARAM(0) == mode[1])
//				his->changeRecodeMode(1);
//			else if (PARAM(0) == mode[2])
//				his->changeRecodeMode(2);
//			RETURNDEFAULT;
//		}
//		bklong mode2 = PARAM(0).asInteger();
//		if ((bklong)mode2 == his->recordMode)
//			RETURNDEFAULT;
//		switch ((bklong)mode2)
//		{
//		case 0:
//			his->changeRecodeMode(0);
//			break;
//		case 1:
//			his->changeRecodeMode(1);
//			break;
//		case 2:
//			his->changeRecodeMode(2);
//			break;
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(maxChars)
	{
/*//		return his->forceMaxChars;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(maxChars)
	{
//		MINIMUMPARAMNUM(1);
//		his->forceMaxChars = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getDataAtLine)
	{
//		MINIMUMPARAMNUM(1);
/*//		return his->getData(PARAM(0));*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(reline)
	{
//		his->reline();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(repage)
	{
//		his->repage();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//add(text, reline=false)
	NATIVECLASS_FUNC(add)
	{
//		MINIMUMPARAMNUM(1);
//		his->recordText(PARAM(0));
//		if (PARAM(1))
//			his->reline();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

//	BKE_Graphic *Sprite::graphic = NULL;

	/**
	 * @class Sprite
	 * @brief 精灵类
	 */
#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Sprite
#endif

	void Sprite::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
//		graphic = BKE_Graphic::getInstance();

		REG_GET(type);
		REG_GET(index);

		REG_GET(value);
		REG_GET(x);
		REG_GET(y);
		REG_GET(width);
		REG_GET(height);
		REG_GET(imageWidth);
		REG_GET(imageHeight);
		REG_GET(opacity);
		REG_GET(zorder);
		REG_GET(rect);

		REG_SET(value);
		REG_SET(x);
		REG_SET(y);
		REG_SET(width);
		REG_SET(height);
		REG_SET(opacity);
		REG_SET(zorder);
		REG_SET(rect);

		REG_SET(disabled);
		REG_GET(disabled);
		REG_SET(disabledRecursive);
// 		REG_SET(ignoreEvent);
// 		REG_GET(ignoreEvent);
// 		REG_SET(ignoreEventRecursive);

		REG_SET(onEnter);
		REG_SET(onHover);
		REG_SET(onLeave);
		REG_SET(onTouchBegan);
		REG_SET(onTouchMoved);
		REG_SET(onTouchEnded);
		REG_SET(onClick);
		REG_GET(onEnter);
		REG_GET(onHover);
		REG_GET(onLeave);
		REG_GET(onTouchBegan);
		REG_GET(onTouchMoved);
		REG_GET(onTouchEnded);
		REG_GET(onClick);

		REG_FUNC(getParent);
		REG_FUNC(saveImage);

		REG_FUNC(idlesp);
		REG_FUNC(create);
	}

	/**
	 * @property_getter type
	 * @brief 获取类型
	 */
	NATIVECLASS_GET(type)
	{
//		static BKE_String types[] = {L"Sprite", L"TextSprite", L"Layer", L"BasicLayer", L"MessageLayer", L"Button", L"Animation", L"Frames", L"Slider", L"ColorLayer", L"CheckBox", L"Unknown"};
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//			return types[sp->m_SpriteType];
//		else
/*//			RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(index)
	{
//		GETINSTANCE();
/*//		return instance->index;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(value)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp)
//			RETURNDEFAULT;
//		switch (sp->m_SpriteType)
//		{
//		case BKE_Sprite::T_SLIDER:
//			return static_cast<BKE_Slider *>(sp)->getValue();
//		case BKE_Sprite::T_INPUTBOX:
//			return static_cast<BKE_InputBox *>(sp)->getString();
//		case BKE_Sprite::T_CHECKBOX:
//			return static_cast<BKE_CheckBox *>(sp)->getState();
//		default:
//			RETURNDEFAULT;
/*//		}*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(value)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp)
//			RETURNDEFAULT;
//		switch (sp->m_SpriteType)
//		{
//		case BKE_Sprite::T_SLIDER:
//			static_cast<BKE_Slider *>(sp)->setValue(PARAM(0));
//			break;;
//		case BKE_Sprite::T_INPUTBOX:
//			static_cast<BKE_InputBox *>(sp)->setString(PARAM(0).asBKEStr().getConstStr());
//			break;
//		case BKE_Sprite::T_CHECKBOX:
//			static_cast<BKE_CheckBox *>(sp)->changeState(PARAM(0));
//			break;
//		default:
//			break;
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(x)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp)
//			RETURNDEFAULT;
/*//		return sp->getPosInView().x;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(x)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			auto a = sp->getPosInView();
//			a.x = (float)PARAM(0);
//			sp->setPosInView(a);
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(rect)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp)
//			RETURNDEFAULT;
//		auto v = new BKE_VarArray();
//		auto &r = sp->getTextureRect();
//		v->pushMember(r.origin.x);
//		v->pushMember(r.origin.y);
//		v->pushMember(r.size.width);
//		v->pushMember(r.size.height);
/*//		return v;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(rect)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		if (PARAM(0).getType() != VAR_ARRAY)
//			throw Var_Except(L"必须赋值为一个数组");
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			CCRect r;
//			auto v = (BKE_VarArray*)(PARAM(0).obj);
//			r.origin.x = v->getMember(0);
//			r.origin.y = v->getMember(1);
//			r.size.width = v->getMember(2);
//			r.size.height = v->getMember(3);
//			sp->setTextureRect(r);
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(width)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp)
//			RETURNDEFAULT;
/*//		return sp->getContentSize().width * sp->getScaleX();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(width)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			auto a = sp->getTextureRect();
//			a.size.width = (float)PARAM(0);
//			sp->setTextureRect(a);
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(height)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp)
//			RETURNDEFAULT;
/*//		return sp->getContentSize().height * sp->getScaleY();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(height)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			auto a = sp->getTextureRect();
//			a.size.height = (float)PARAM(0);
//			sp->setTextureRect(a);
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(imageWidth)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp || !sp->getTexture())
//			RETURNDEFAULT;
/*//		return sp->getTexture()->getPixelsWide();*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(imageHeight)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp || !sp->getTexture())
//			RETURNDEFAULT;
/*//		return sp->getTexture()->getPixelsHigh();*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(y)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp)
//			RETURNDEFAULT;
/*//		return sp->getPosInView().y;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(y)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			auto a = sp->getPosInView();
//			a.y = (float)PARAM(0);
//			sp->setPosInView(a);
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(opacity)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp)
//			RETURNDEFAULT;
/*//		return sp->getOpacity();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(opacity)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			sp->setOpacity((int)PARAM(0));
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(zorder)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp)
//			RETURNDEFAULT;
/*//		return sp->getZOrder();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(zorder)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			sp->setZOrder((int)PARAM(0));
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(disabled)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			sp->setDisabled(PARAM(0), false);
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(disabled)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			return sp->isDisabled();
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(disabledRecursive)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			sp->setDisabled(PARAM(0), true);
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}
/*

	NATIVECLASS_SET(ignoreEvent)
	{
		GETINSTANCE();
		MINIMUMPARAMNUM(1);
		auto sp = graphic->getSpriteByID(instance->index);
		if (sp)
		{
			sp->setIgnoreEvent(PARAM(0), false);
		}
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(ignoreEvent)
	{
		GETINSTANCE();
		auto sp = graphic->getSpriteByID(instance->index);
		if (sp)
		{
			return sp->isIgnoreEvent();
		}
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(ignoreEventRecursive)
	{
		GETINSTANCE();
		MINIMUMPARAMNUM(1);
		auto sp = graphic->getSpriteByID(instance->index);
		if (sp)
		{
			sp->setIgnoreEvent(PARAM(0), true);
		}
		RETURNDEFAULT;
	}*/

//	static BKE_Event varClassToEvent(const BKE_Variable &var) throw(Var_Except)
//	{
//		if (var.getType()==VAR_CLASS)
//		{
//			GETCLASSINSTANCE(var.obj, Event, c);
//			return c->getEvent();
//		}
//		BKE_Event e;
//		varToEvent(e, var, BKE_Script::sharedScript()->curscript);
//		return e;
//	}

	NATIVECLASS_SET(onEnter)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			sp->setEnterEvent(varClassToEvent(PARAM(0)));
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(onHover)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			sp->setHoverEvent(varClassToEvent(PARAM(0)));
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(onLeave)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			sp->setLeaveEvent(varClassToEvent(PARAM(0)));
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(onTouchBegan)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			sp->setTouchBeganEvent(varClassToEvent(PARAM(0)));
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(onTouchMoved)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			sp->setTouchMovedEvent(varClassToEvent(PARAM(0)));
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(onTouchEnded)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			sp->setTouchEndedEvent(varClassToEvent(PARAM(0)));
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(onClick)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			sp->setClickEvent(varClassToEvent(PARAM(0)));
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(onEnter)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			CREATECLASSINSTANCE(i, n, Event, sp->getEnterEvent());
//			return i;
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(onHover)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			CREATECLASSINSTANCE(i, n, Event, sp->getHoverEvent());
//			return i;
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(onLeave)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			CREATECLASSINSTANCE(i, n, Event, sp->getLeaveEvent());
//			return i;
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(onTouchBegan)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			CREATECLASSINSTANCE(i, n, Event, sp->getTouchBeganEvent());
//			return i;
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(onTouchMoved)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			CREATECLASSINSTANCE(i, n, Event, sp->getTouchMovedEvent());
//			return i;
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(onTouchEnded)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			CREATECLASSINSTANCE(i, n, Event, sp->getTouchEndedEvent());
//			return i;
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(onClick)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			CREATECLASSINSTANCE(i, n, Event, sp->getClickEvent());
//			return i;
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getParent)
	{
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (!sp)
//			RETURNDEFAULT;
//		auto sp2 = dynamic_cast<BKE_Sprite*>(sp->getParent());
//		if (sp2 && sp2->getID() != BKE_INVALID_INDEX)
//		{
//			CREATECLASSINSTANCE(s, n, Sprite, sp2->getID());
//			return s;
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//saveImage(filename, hasChildren=false)	//save as png
	NATIVECLASS_FUNC(saveImage)
	{
//		MINIMUMPARAMNUM(1);
//		GETINSTANCE();
//		auto sp = graphic->getSpriteByID(instance->index);
//		if (sp)
//		{
//			return sp->saveToFile((wstring)PARAM(0), PARAM(1));
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(idlesp)
	{
/*//		return graphic->idlesp();*/	RETURNDEFAULT;
	}

	//create(index, file, rect)
	NATIVECLASS_FUNC(create)
	{
//		MINIMUMPARAMNUM(2);
//		bklong idx = PARAM(0);
//		if (idx < 0)
//			return false;
//		auto re = BKE_RectZero;
//		BKE_String f = PARAM(1).asBKEStr();
//		auto &v = PARAM(2);
//		if (v.getType() == VAR_ARRAY)
//		{
//			re.origin.x = v[0];
//			re.origin.y = v[1];
//			re.size.width = v[2];
//			re.size.height = v[3];
//		}
//		auto err = graphic->createSprite(idx, f.getConstStr(), re);
//		handleError(err);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

#define GETORRETURNVOID(name, __class) GETINSTANCE();auto name=dynamic_cast<BKE_##__class*>(graphic->getSpriteByID(instance->index));if(!name)RETURNDEFAULT;

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS MessageLayer
#endif

	void MessageLayer::mark(){}

	NATIVECLASS_INIT()
	{
//		NATIVECLASS_SUPERINIT();
		PREPARECLASS();
		//same as parent class
		//_class->cannotcreate = false;
		//this->graphic = BKE_Graphic::sharedGraphic();

		REG_GET(curposX);
		REG_GET(curposY);
		//REG_GET(xInterval);
		//REG_GET(yInterval);

		REG_SET(curposX);
		REG_SET(curposY);
		//REG_SET(xInterval);
		//REG_SET(yInterval);
	}

	NATIVECLASS_GET(curposX)
	{
//		GETORRETURNVOID(sp, MessageLayer);
/*//		return sp->curxpos;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(curposX)
	{
//		MINIMUMPARAMNUM(1);
//		GETORRETURNVOID(sp, MessageLayer);
//		sp->curxpos = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(curposY)
	{
//		GETORRETURNVOID(sp, MessageLayer);
/*//		return sp->curypos;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(curposY)
	{
//		MINIMUMPARAMNUM(1);
//		GETORRETURNVOID(sp, MessageLayer);
//		sp->curypos = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	/*NATIVECLASS_GET(xInterval)
	{
		GETORRETURNVOID(sp, MessageLayer);
		return sp->xinterval;
	}

	NATIVECLASS_SET(xInterval)
	{
		MINIMUMPARAMNUM(1);
		GETORRETURNVOID(sp, MessageLayer);
		sp->xinterval = PARAM(0);
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(yInterval)
	{
		GETORRETURNVOID(sp, MessageLayer);
		return sp->yinterval;
	}

	NATIVECLASS_SET(yInterval)
	{
		MINIMUMPARAMNUM(1);
		GETORRETURNVOID(sp, MessageLayer);
		sp->yinterval = PARAM(0);
		RETURNDEFAULT;
	}*/

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS TextSprite
#endif

	void TextSprite::mark(){}

	NATIVECLASS_INIT()
	{
//		NATIVECLASS_SUPERINIT();
		PREPARECLASS();

		REG_GET(color);
		REG_GET(text);
		REG_GET(fontName);
		REG_GET(fontStyle);
		REG_GET(fontSize);

		REG_SET(color);
		REG_SET(text);
		REG_SET(fontName);
		REG_SET(fontStyle);
		REG_SET(fontSize);
	}

	NATIVECLASS_GET(color)
	{
//		GETORRETURNVOID(sp, TextSprite);
/*//		return varFromFontColor(sp->info.getFontColor());*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(text)
	{
//		GETORRETURNVOID(sp, TextSprite);
/*//		return sp->text;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontName)
	{
//		GETORRETURNVOID(sp, TextSprite);
/*//		return sp->info.getFontName();*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontStyle)
	{
//		GETORRETURNVOID(sp, TextSprite);
/*//		return sp->info.getFontStyle();*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontSize)
	{
//		GETORRETURNVOID(sp, TextSprite);
/*//		return sp->info.getFontSize();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(color)
	{
//		MINIMUMPARAMNUM(1);
//		GETORRETURNVOID(sp, TextSprite);
//		BKE_FontColor c;
//		if (varToFontColor(c, PARAM(0)))
//		{
//			sp->info.setFontColor(c);
//			sp->updateTexture();
//			RETURNDEFAULT;
//		}
//		throw(Var_Except(L"错误的FontColor类型。应该是一个整数或者含有两个整数的数组。"));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(text)
	{
//		MINIMUMPARAMNUM(1);
//		GETORRETURNVOID(sp, TextSprite);
//		sp->text = (wstring)PARAM(0);
//		sp->updateTexture();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontName)
	{
//		MINIMUMPARAMNUM(1);
//		GETORRETURNVOID(sp, TextSprite);
//		sp->info.setFontName(PARAM(0).asBKEStr());
//		sp->updateTexture();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontStyle)
	{
//		MINIMUMPARAMNUM(1);
//		GETORRETURNVOID(sp, TextSprite);
//		sp->info.setFontStyle((bkulong)PARAM(0));
//		sp->updateTexture();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontSize)
	{
//		MINIMUMPARAMNUM(1);
//		GETORRETURNVOID(sp, TextSprite);
//		sp->info.setFontSize(PARAM(0));
//		sp->updateTexture();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Button
#endif

	void Button::mark(){}

	NATIVECLASS_INIT()
	{
//		NATIVECLASS_SUPERINIT();
		PREPARECLASS();
		//same as parent class
		//_class->cannotcreate = false;
		//this->graphic = BKE_Graphic::sharedGraphic();

		REG_GET(idle);
		REG_GET(hover);
		REG_GET(click);
		//REG_SET(idle);
		//REG_SET(hover);
		//REG_SET(click);
	}

	NATIVECLASS_GET(idle)
	{
//		GETORRETURNVOID(sp, Button);
//		CREATECLASSINSTANCE(in, n, Sprite);
//		n->index = sp->m_psIdle->getID();
/*//		return in;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(hover)
	{
//		GETORRETURNVOID(sp, Button);
//		CREATECLASSINSTANCE(in, n, Sprite);
//		n->index = sp->m_psHover->getID();
/*//		return in;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(click)
	{
//		GETORRETURNVOID(sp, Button);
//		CREATECLASSINSTANCE(in, n, Sprite);
//		n->index = sp->m_psClick->getID();
/*//		return in;*/	RETURNDEFAULT;
	}

	//NATIVECLASS_SET(idle)
	//{
	//	RETURNDEFAULT;
	//}

	//NATIVECLASS_SET(hover)
	//{
	//	RETURNDEFAULT;
	//}

	//NATIVECLASS_SET(click)
	//{
	//	RETURNDEFAULT;
	//}
#if USE_LIVE2D

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Live2DSprite
#endif

	void Live2DSprite::mark(){}

	NATIVECLASS_INIT()
	{
//		NATIVECLASS_SUPERINIT();
		PREPARECLASS();

		REG_FUNC(startMotion);
		REG_FUNC(startMotionFile);
		REG_FUNC(startRandomMotion);
		REG_FUNC(stopMotion);
		REG_FUNC(getMotionNum);

		REG_FUNC(setExpression);
		REG_FUNC(setExpressionFile);
		REG_FUNC(setRandomExpression);

		REG_GET(onFlick);
		REG_SET(onFlick);
		REG_GET(onTap);
		REG_SET(onTap);
	}

//	static int varToPriorityType(const BKE_Variable &v)
//	{
//		if (v.getType()==VAR_STR)
//		{
//			wstring s = v.asString();
//			if (s == L"none")
//				return PRIORITY_NONE;
//			else if (s == L"idle")
//				return PRIORITY_IDLE;
//			else if (s == L"normal")
//				return PRIORITY_NORMAL;
//			else if (s == L"force")
//				return PRIORITY_FORCE;
//			else
//				throw(Var_Except(L"错误的priority字符串。应该是none、idle、normal、force其中之一。"));
//		}
//		else if (v.getType() == VAR_NUM)
//		{
//			bklong i = v.asInteger();
//			if (i < 0 || i > PRIORITY_FORCE)
//			{
//				throw(Var_Except(L"错误的priority值。应该在0-3之间。"));
//			}
//			return i;
//		}
//		else
//			throw(Var_Except(L"错误的priority类型。需要字符串或者数值。"));
//	}

	NATIVECLASS_FUNC(startMotion)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
//		MINIMUMPARAMNUM(1);
/*//		return sp->startMotion(PARAM(0).asString(), PARAMDEFAULT(1, 0), PARAMEXIST(2) ? varToPriorityType(PARAM(2)) : PRIORITY_NORMAL);*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(startRandomMotion)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
//		MINIMUMPARAMNUM(1);
/*//		return sp->startRandomMotion(PARAM(0).asString(), PARAMEXIST(1)?varToPriorityType(PARAM(1)):PRIORITY_NORMAL);*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(startMotionFile)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
//		MINIMUMPARAMNUM(1);
/*//		return sp->startMotionFile(PARAM(0).asString(), PARAMDEFAULT(1, L""), PARAMDEFAULT(2, true), PARAMEXIST(3) ? varToPriorityType(PARAM(3)) : PRIORITY_NORMAL, PARAMDEFAULT(4, 0), PARAMDEFAULT(5, 1000), PARAMDEFAULT(6, 1000));*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(stopMotion)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
//		sp->stopMotion();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getMotionNum)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
/*//		return sp->getMotionNum(PARAM(0));*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(setExpression)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
//		MINIMUMPARAMNUM(1);
/*//		return sp->setExpression(PARAM(0).asString());*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(setRandomExpression)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
//		sp->setRandomExpression();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(setExpressionFile)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
//		MINIMUMPARAMNUM(1);
/*//		return sp->setExpressionFile(PARAM(0).asString(), PARAMDEFAULT(1, true));*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(onTap)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
//		BKE_Event event = sp->getTapEvent();
//		CREATECLASSINSTANCE(ins, nat, Event, event);
/*//		return ins;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(onTap)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
//		GETCLASSINSTANCE(PARAM(0).obj, Event, event);
//		sp->setTapEvent(event->getEvent());
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(onFlick)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
//		BKE_Event event = sp->getFlickEvent();
//		CREATECLASSINSTANCE(ins, nat, Event, event);
/*//		return ins;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(onFlick)
	{
//		GETORRETURNVOID(sp, Live2DSprite);
//		GETCLASSINSTANCE(PARAM(0).obj, Event, event);
//		sp->setFlickEvent(event->getEvent());
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}
#endif

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Event
#endif

	void Event::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();

		REG_SET(name);
		REG_SET(enable);
		REG_SET(label);
		REG_SET(file);
		REG_SET(type);
		REG_SET(ignore);
		REG_SET(exp);
		REG_SET(param);

		REG_GET(name);
		REG_GET(enable);
		REG_GET(label);
		REG_GET(file);
		REG_GET(type);
		REG_GET(ignore);
		REG_GET(exp);
		REG_GET(param);

		REG_FUNC(call);
		REG_FUNC(toVar);
	}

	NATIVECLASS_CREATENEW()
	{
		//if (!paramarray)
//		{
			return new Event;
//		}
		BKE_Event e;
		//varToEvent(e, PARAM(0), BKE_Script::sharedScript()->curscript);
		return new Event(e);
	}

	NATIVECLASS_SAVE()
	{
//		BKE_Variable v;
//		varFromEvent(v, event);
/*//		return v;*/	RETURNDEFAULT;
	}

	NATIVECLASS_LOAD()
	{
/*//		varToEvent(event, var, BKE_Script::sharedScript()->curscript);*/
	}

	NATIVECLASS_SET(name)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		instance->event.name = PARAM(0).asBKEStr();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(enable)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		instance->event.enable = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(label)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		instance->event.label = PARAM(0).asBKEStr();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(file)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		instance->event.file = PARAM(0).asBKEStr();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(type)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		varToEventType(instance->event.type, PARAM(0));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(ignore)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		instance->event.ignore = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(exp)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		instance->event.exp = PARAM(0).asBKEStr();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(param)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		instance->event.setParam(PARAM(0).asDic());
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(func)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		instance->event.setFunc(PARAM(0).asFunc());
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(name)
	{
//		GETINSTANCE();
/*//		return instance->event.name;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(enable)
	{
//		GETINSTANCE();
/*//		return instance->event.enable;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(label)
	{
//		GETINSTANCE();
/*//		return instance->event.label;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(file)
	{
//		GETINSTANCE();
/*//		return instance->event.file;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(type)
	{
//		GETINSTANCE();
/*//		return instance->event.type;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(ignore)
	{
//		GETINSTANCE();
/*//		return instance->event.ignore;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(exp)
	{
//		GETINSTANCE();
/*//		return instance->event.exp;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(param)
	{
//		GETINSTANCE();
//		if (instance->event.param)
//		{
//			return BKE_VarObjectReferencer(instance->event.param);
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(func)
	{
//		GETINSTANCE();
//		if (instance->event.func)
//		{
//			return BKE_VarObjectReferencer(instance->event.func);
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(call)
	{
//		GETINSTANCE();
//		BKE_ObjectID index = BKE_INVALID_INDEX;
//		if (PARAMEXIST(0))
//		{
//			if (PARAM(0).getType()==VAR_CLASS)
//			{
//				GETCLASSINSTANCE(PARAM(0).obj, Sprite, n);
//				index = n->index;
//			}
//			index = PARAM(0);
//		}
//		BKE_EventParam p(PARAM(1).asDic());
//		return instance->event.call(index == BKE_INVALID_INDEX?NULL:BKE_Graphic::getInstance()->getSpriteByID(index),
/*//			PARAMEXIST(1)?&p : NULL);*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(toVar)
	{
//		GETINSTANCE();
//		BKE_Variable v;
//		varFromEvent(v, instance->event);
/*//		return v;*/	RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS EventCenter
#endif

//	BKE_EventCenter *EventCenter::ec = NULL;

	void EventCenter::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();

//		ec = BKE_EventCenter::getInstance();

		REG_FUNC(register);
		REG_FUNC(delete);
		REG_FUNC(systemCall);
		REG_FUNC(call);
		REG_FUNC(get);
	}

	NATIVECLASS_FUNC(register)
	{
//		MINIMUMPARAMNUM(1);
//		if (PARAMEXIST(1))
//		{
//			GETCLASSINSTANCE(PARAM(1).obj, Event, event);
//			ec->registerEvent(PARAM(0).asString(), event->getEvent());
//		}
//		else
//		{
//			ec->deleteEvent(PARAM(0).asString());
//		}
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(delete)
	{
//		MINIMUMPARAMNUM(1);
//		ec->deleteEvent(PARAM(0).asString());
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//systemCall(name, param);
	NATIVECLASS_FUNC(systemCall)
	{
//		MINIMUMPARAMNUM(1);
//		BKE_EventParam p;
//		if (PARAMEXIST(1) && PARAM(1).getType() == VAR_DIC)
//			p.dic = BKE_VarObjectReferencer((BKE_VarDic*)PARAM(1).obj);
//		ec->callSysEvent(PARAM(0).asString(), NULL, &p);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(call)
	{
//		MINIMUMPARAMNUM(1);
//		BKE_EventParam p;
//		if (PARAMEXIST(1) && PARAM(1).getType()==VAR_DIC)
//			p.dic = BKE_VarObjectReferencer((BKE_VarDic*)PARAM(1).obj);
//		ec->callEvent(PARAM(0).asString(), NULL, &p);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(get)
	{
//		MINIMUMPARAMNUM(1);
//		wstring name = PARAM(0).asString();
//		if (!ec->existEvent(name))
//		{
//			RETURNDEFAULT;
//		}
//		BKE_Event event = ec->getEvent(name);
//		CREATECLASSINSTANCE(i, n, Event, event);
/*//		return i;*/	RETURNDEFAULT;
	}

//	TimerHelper *TimerHelper::helper = NULL;

//	void TimerHelper::update(float dt)
//	{
//		for (auto it : todelete)
//		{
//			timerset.erase(it);
//		}
//		for (auto it : todelete2)
//		{
//			timerset2.erase(it);
//		}
//		todelete.clear();
//		todelete2.clear();
//		for (auto it : timerset)
//		{
			//if (todelete.find(it) != todelete.end())
			//	continue;
//			if (it->enabled && it->interval > 0 && (it->nexttime -= 1000 * dt) <= 0)
//			{
//				it->paramarr->vararray[0] = it->interval - it->nexttime;
//				it->nexttime = it->interval;
//				try
//				{
//					((BKE_VarFunction*)it->func.obj)->run(it->paramarr);
//				}
//				catch (Var_Except &e)
//				{
//					ERRORADD(TR_ERROR_TIMER_RUN, e.getMsg());
#if PARSER_DEBUG
//					ERRORDIRECTADD(L'\t', Parser::getInstance()->getTraceString(), L'\n');
#endif
//					it->enabled = false;
//					ERRORCOMMIT();
//				}
//			}
//		}
//		for (auto it : timerset2)
//		{
			//if (todelete2.find(it.first) != todelete2.end())
			//	continue;
//			if ((it.second.delay -= 1000 * dt) <= 0)
//			{
//				todelete2.insert(it.first);
//				it.second.paramarr->vararray[0] = it.second.rawdelay - it.second.delay;
//				try
//				{
//					it.second.func->run(it.second.paramarr);
//				}
//				catch (Var_Except &e)
//				{
//					ERRORADD(TR_ERROR_TIMER_RUN, e.getMsg());
#if PARSER_DEBUG
//					ERRORDIRECTADD(L'\t', Parser::getInstance()->getTraceString(), L'\n');
#endif
//					ERRORCOMMIT();
//				}
//			}
//		}
//	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Timer
#endif

	void Timer::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();

		REG_GET(enable);
		REG_SET(enable);
		REG_GET(interval);
		REG_SET(interval);

		REG_FUNC(setTimeout);
		REG_FUNC(clearTimeout);
	}

	NATIVECLASS_GET(enable)
	{
//		GETINSTANCE();
/*//		return instance->enabled;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(enable)
	{
//		MINIMUMPARAMNUM(1);
//		GETINSTANCE();
//		instance->enabled = (bool)PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(interval)
	{
//		GETINSTANCE();
/*//		return instance->interval;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(interval)
	{
//		MINIMUMPARAMNUM(1);
//		GETINSTANCE();
//		instance->interval = (float)PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//setTimeout(func, delay, params, ...)
	NATIVECLASS_FUNC(setTimeout)
	{
//		MINIMUMPARAMNUM(2);
//		if (PARAM(0).getType() != VAR_FUNC)
//			throw Var_Except(L"参数1必须为函数");
//		BKE_VarFunction *func = (BKE_VarFunction *)(PARAM(0).obj->addRef());
//		auto arr = new BKE_VarArray();
//		arr->setLength(paramarray->vararray.size() - 2);
//		int i = 1;
//		while (++i < paramarray->vararray.size())
//			arr->vararray[i - 1] = paramarray->vararray[i];
//		TimerHelper::getSharedObject()->addTempTimer(func, arr, (float)PARAM(1));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	//clearTimeout(func)
	NATIVECLASS_FUNC(clearTimeout)
	{
//		MINIMUMPARAMNUM(1);
//		if (PARAM(0).getType() != VAR_FUNC)
//			throw Var_Except(L"参数必须为函数");
//		TimerHelper::getSharedObject()->removeTempTimer((BKE_VarFunction *)(PARAM(0).obj));
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Json
#endif

	void Json::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_FUNC(toJson);
		REG_FUNC(fromJson);
	}

	NATIVECLASS_FUNC(toJson)
	{
//		MINIMUMPARAMNUM(1);
/*//		return support::varToJsonStringW(PARAM(0));*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(fromJson)
	{
//		MINIMUMPARAMNUM(1);
/*//		return support::varFromJsonStringW(PARAM(0));*/	RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS CSVParser
#endif

	void CSVParser::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();

		REG_FUNC(doLine);
		REG_FUNC(doDocument);
		REG_FUNC(doFile);

		REG_FUNC(fromDocument);
		REG_FUNC(fromFile);
	}

	NATIVECLASS_CREATENEW()
	{
		return new CSVParser;
	}

	NATIVECLASS_FUNC(doLine)
	{
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(doFile)
	{
//		MINIMUMPARAMNUM(1);
//		GETINSTANCE();
//		BKE_Variable *v;
//		if (_this->hasMember(L"doLine", &v) && v->getType() == VAR_FUNC)
//			instance->parser.setDoLineFunction((BKE_VarFunction *)v->obj->addRef());
//		else
//			instance->parser.setDoLineFunction(NULL);
/*//		return instance->parser.doFile(PARAM(0).asString());*/	RETURNDEFAULT;
	}
	
	NATIVECLASS_FUNC(doDocument)
	{
//		MINIMUMPARAMNUM(1);
//		GETINSTANCE();
//		BKE_Variable *v;
//		if (_this->hasMember(L"doLine", &v) && v->getType() == VAR_FUNC)
//			instance->parser.setDoLineFunction((BKE_VarFunction *)v->obj->addRef());
//		else
//			instance->parser.setDoLineFunction(NULL);
/*//		return instance->parser.doDocument(PARAM(0).asString());*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(fromDocument)
	{
//		MINIMUMPARAMNUM(1);
/*//		return ::CSVParser::fromDocument(PARAM(0).asString());*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(fromFile)
	{
//		MINIMUMPARAMNUM(1);
/*//		return ::CSVParser::fromFile(PARAM(0).asString());*/	RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS DownloadManager
#endif

	void DownloadManager::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_FUNC(begin);
		REG_FUNC(stop);

		REG_GET(packageUrl);
		REG_SET(packageUrl);
		REG_GET(outFileName);
		REG_SET(outFileName);
		REG_SET(onSuccess);
		REG_SET(onError); // error_string
		REG_SET(onProgress); // now,total
	}

	//url, filename
	NATIVECLASS_CREATENEW()
	{
		return new DownloadManager(PARAM(0).asString(), PARAM(1).asString());
	}

	NATIVECLASS_SAVE()
	{
//		if (!mgr)
//		{
//			return BKE_Variable();
//		}
/*//		return BKE_Variable::arrayWithObjects(mgr->getPackageUrl(), mgr->getOutFileName());*/	RETURNDEFAULT;
	}

	NATIVECLASS_LOAD()
	{
//		if (mgr)
//			delete mgr;
//		mgr = new BKE_DownloadManager(var[0].asString(), var[1].asString()); 
/*//		mgr->setDelegate(this);*/
	}

//	void DownloadManager::onError(BKE_DownloadManager::ErrorCode errorCode)
//	{
//		wstring err;
//		switch (errorCode)
//		{
//		case BKE_DownloadManager::kCreateFile:
//			err = L"createfile";
//			break;
//		case BKE_DownloadManager::kNetwork:
//			err = L"network";
//			break;
//		default:
//			err = L"stop";
//			break;
//		}
//		_onError.run(_class, err);
//	}

//	void DownloadManager::onProgress(bklong nowDownloaded, bklong totalToDownload)
//	{
//		_onProgress.run(_class, nowDownloaded, totalToDownload);
//	}

//	void DownloadManager::onSuccess()
//	{
//		_onSuccess.run(_class);
//	}

	NATIVECLASS_FUNC(begin)
	{
//		GETINSTANCE();
//		instance->mgr->begin();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(stop)
	{
//		GETINSTANCE();
//		instance->mgr->stop();
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(packageUrl)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		instance->mgr->setPackageUrl(PARAM(0).asString());
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(outFileName)
	{
//		GETINSTANCE();
//		MINIMUMPARAMNUM(1);
//		instance->mgr->setOutFileName(PARAM(0).asString());
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(packageUrl)
	{
//		GETINSTANCE();
/*//		return instance->mgr->getPackageUrl();*/	RETURNDEFAULT;
	}

	NATIVECLASS_GET(outFileName)
	{
//		GETINSTANCE();
/*//		return instance->mgr->getOutFileName();*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(onError)
	{
//		GETINSTANCE();
//		if (PARAM(0).getType()!=VAR_FUNC)
//		{
//			throw(Var_Except(L"需要一个function作为参数。"));
//		}
//		instance->_onError = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(onProgress)
	{
//		GETINSTANCE();
//		if (PARAM(0).getType() != VAR_FUNC)
//		{
//			throw(Var_Except(L"需要一个function作为参数。"));
//		}
//		instance->_onProgress = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

	NATIVECLASS_SET(onSuccess)
	{
//		GETINSTANCE();
//		if (PARAM(0).getType() != VAR_FUNC)
//		{
//			throw(Var_Except(L"需要一个function作为参数。"));
//		}
//		instance->_onSuccess = PARAM(0);
/*//		RETURNDEFAULT;*/	RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Network
#endif

	void Network::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
		REG_FUNC(download);
		REG_FUNC(getString);
		REG_FUNC(request);
	}
	
	NATIVECLASS_FUNC(download)
	{
//		MINIMUMPARAMNUM(2);
//		CREATECLASSINSTANCE(rtn, clazz, DownloadManager, PARAM(0).asString(), PARAM(1).asString());
/*//		return rtn;*/	RETURNDEFAULT;
	}
	
	NATIVECLASS_FUNC(getString)
	{
//		MINIMUMPARAMNUM(1);
/*//		return BKE_DownloadManager::getString(PARAM(0));*/	RETURNDEFAULT;
	}

//	enum bkeHttpType{
//		bkeHttpGet,
//		bkeHttpPost,
//		bkeHttpPut,
//		bkeHttpDelete,
//		bkeHttpUnknown
	//};

//	bkeHttpType _getHttpType(const wstring &s)
	//{
//		static unordered_map<wstring, bkeHttpType> _e2h = {
//			{ L"put", bkeHttpPut } ,
//			{ L"PUT", bkeHttpPut },
//			{ L"get", bkeHttpGet },
//			{ L"GET", bkeHttpGet },
//			{ L"post", bkeHttpPost },
//			{ L"POST", bkeHttpPost },
//			{ L"delete", bkeHttpDelete },
//			{ L"DELETE", bkeHttpDelete },
//		};
//		auto it = _e2h.find(s);
//		if (it==_e2h.end())
//		{
//			return bkeHttpUnknown;
//		}
//		return it->second;
//	}

//	class NetworkPrivate : public CCObject
//	{
//		BKE_Variable _success;
//		BKE_Variable _error;

//	public:
//		NetworkPrivate(const BKE_Variable &success, const BKE_Variable &error)
//			:_success(success), _error(error)
//		{
//		}
//		void call(CCHttpClient* client, CCHttpResponse* response)
//		{
//			if (!response->isSucceed())
//			{
//				_error.run(NULL, response->getResponseCode(), UniFromUTF8(response->getErrorBuffer()));
//				return;
//			}
//			bkeHttpContentType type = support::getContentTypeByString(response->getContentType());
//			if (type == bkeHttpContentUnknown)
//			{
//				_error.run(NULL, -1, wstring(L"不支持的Content-Type：") + UniFromUTF8(response->getContentType()));
//				return;
//			}
//			_success.run(NULL, support::varFromStringWithContentType(*response->getResponseData(), type),UniFromUTF8(response->getCookies()));
//		}
//	};

	NATIVECLASS_FUNC(request)
	{
//		MINIMUMPARAMNUM(1);
//		auto &v = PARAM(0);
//		if (v.getType() != VAR_DIC)
//		{
//			throw(Var_Except(L"request方法需要一个字典参数。"));
//		}
//		bool async = v[L"async"].isVoid() ? true : v[L"async"].asBoolean();
//		wstring url = v[L"url"].asString();
//		bkeHttpType type = _getHttpType(v[L"type"].asString());
//		wstring cookies = v[L"cookies"].asString();
//		BKE_Variable success = v[L"success"];
//		BKE_Variable error = v[L"error"];
//		switch (type)
//		{
//		case Parser_Util::bkeHttpGet:
//		{
//			BKE_VarDic *data;
//			{
//				BKE_Variable &_data = v[L"data"];
//				if (_data.getType() == VAR_NONE)
//				{
//					data = NULL;
//				}
//				else if (_data.getType() == VAR_DIC)
//				{
//					data = (BKE_VarDic *)_data.obj;
//				}
//				else
//				{
//					throw(Var_Except(L"data参数需要一个字典。"));
//				}
//			}
//			support::HttpGet(url, data, (new NetworkPrivate(success, error))->autorelease(), httpresponse_selector(NetworkPrivate::call), async, UniToUTF8(cookies));
//		}
//			break;
//		case Parser_Util::bkeHttpPost:
//		case Parser_Util::bkeHttpPut:
//		{
//			bkeHttpContentType dataType;
//			{
//				BKE_Variable &type = v[L"dataType"];
//				if (type.getType() == VAR_NONE)
//				{
//					dataType = bkeHttpContentUrlencoded;
//				}
//				else if (type.getType() != VAR_STR)
//				{
//					throw(Var_Except(L"dataType参数需要一个字符串。"));
//				}
//				wstring _dataType = type.asString();
//				if (_dataType == L"text")
//				{
//					dataType = bkeHttpContentText;
//				}
//				else if (_dataType == L"json")
//				{
//					dataType = bkeHttpContentJson;
//				}
//				else if (_dataType == L"urlencoded")
//				{
//					dataType = bkeHttpContentUrlencoded;
//				}
//				else
//				{
//					throw(Var_Except(L"错误的dataType：" + _dataType));
//				}
//			}
//			switch (dataType)
//			{
//			case bkeHttpContentText:
//			{
//				BKE_Variable &data = v[L"data"];
//				if (data.getType() != VAR_NONE && data.getType() != VAR_STR)
//					throw(Var_Except(L"data需要一个字符串，因为dataType类型是\"text\""));
//				type == bkeHttpPost ?
//					support::HttpPost(url, &data, bkeHttpContentText, (new NetworkPrivate(success, error))->autorelease(), httpresponse_selector(NetworkPrivate::call), async, UniToUTF8(cookies)) :
//					support::HttpPut(url, &data, bkeHttpContentText, (new NetworkPrivate(success, error))->autorelease(), httpresponse_selector(NetworkPrivate::call), async, UniToUTF8(cookies));
//			}
//				break;
//			case bkeHttpContentJson:
//			{
//				BKE_Variable &data = v[L"data"];
//				type == bkeHttpPost ?
//					support::HttpPost(url, &data, bkeHttpContentText, (new NetworkPrivate(success, error))->autorelease(), httpresponse_selector(NetworkPrivate::call), async, UniToUTF8(cookies)) :
//					support::HttpPut(url, &data, bkeHttpContentText, (new NetworkPrivate(success, error))->autorelease(), httpresponse_selector(NetworkPrivate::call), async, UniToUTF8(cookies));
//			}
//				break;
//			case bkeHttpContentUrlencoded:
//			{
//				BKE_Variable &data = v[L"data"];
//				if (data.getType() != VAR_NONE && data.getType() != VAR_DIC)
//				throw(Var_Except(L"data需要一个字典，因为dataType类型是\"urlencoded\""));
//				type == bkeHttpPost ?
//					support::HttpPost(url, &data, bkeHttpContentText, (new NetworkPrivate(success, error))->autorelease(), httpresponse_selector(NetworkPrivate::call), async, UniToUTF8(cookies)) :
//					support::HttpPut(url, &data, bkeHttpContentText, (new NetworkPrivate(success, error))->autorelease(), httpresponse_selector(NetworkPrivate::call), async, UniToUTF8(cookies));
//			}
//				break;
//			case bkeHttpContentBinary:
//				break;
//			case bkeHttpContentUnknown:
//				break;
//			default:
//				break;
//			}
//		}
//			break;
//		case Parser_Util::bkeHttpDelete:
//			support::HttpDelete(url, (new NetworkPrivate(success, error))->autorelease(), httpresponse_selector(NetworkPrivate::call), async, UniToUTF8(cookies));
//			break;
//		case Parser_Util::bkeHttpUnknown:
//			throw(Var_Except(L"未知的type类型"));
//			break;
//		default:
//			break;
//		}
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Debug
#endif

	void Debug::mark(){}

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
		REG_FUNC(printCallStack);
		REG_FUNC(memoryStatus);
	}

	NATIVECLASS_FUNC(printCallStack)
	{
//		auto script = BKE_Script::sharedScript();
//		FORCEADD(TR_FORCE_CALL_STACK);
//		for (int i = script->offsets.size() - 1; i >= 0; i--)
//		{
//			auto &o = script->offsets[i];
//			auto s = script->loadBuffer(o.filename);
//			auto l = o.label;
//			auto li = s->getLine(o.offset);
//			auto li2 = s->getLine(o.labeloffset);
//			FORCEADD(TR_FORCE_CALL_STACK_ELEMENT, o.filename, l, li - li2, li);
//		}
//		FORCECOMMIT();
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(memoryStatus)
	{
//#if CC_PLATFORM_WIN32 == CC_TARGET_PLATFORM
	//win上用API获得总内存量先
//		static struct PROCESS_MEMORY_COUNTERS {
//			DWORD cb;
//			DWORD PageFaultCount;
//			SIZE_T PeakWorkingSetSize;
//			SIZE_T WorkingSetSize;
//			SIZE_T QuotaPeakPagedPoolUsage;
//			SIZE_T QuotaPagedPoolUsage;
//			SIZE_T QuotaPeakNonPagedPoolUsage;
//			SIZE_T QuotaNonPagedPoolUsage;
//			SIZE_T PagefileUsage;
//			SIZE_T PeakPagefileUsage;
//		} pmc;
//		static HANDLE pID = GetCurrentProcess();
//		typedef BOOL(__stdcall *PGetProcessMemoryInfo)(
//			HANDLE Process,                          // handle to process
//			PROCESS_MEMORY_COUNTERS *ppsmemCounters, // buffer
//			DWORD cb                                 // size of buffer
//			);
//		static PGetProcessMemoryInfo GetProcessMemoryInfo = (PGetProcessMemoryInfo)GetProcAddress(LoadLibrary(L"psapi"), "GetProcessMemoryInfo");
//		if (GetProcessMemoryInfo)
//		{
//			GetProcessMemoryInfo(pID, &pmc, sizeof(pmc));
//			FORCEADD(TR_FORCE_MEMORY_STATUS, pmc.WorkingSetSize / 1024.0 / 1024.0);
//			FORCECOMMIT();
//		}
//#endif
		RETURNDEFAULT;
	}
};

void registerExtendFunction()
{
	Parser *p = Parser::getInstance();
	BKE_VarClosure *clo = BKE_VarClosure::global();
	BKE_VarClass *arr = (BKE_VarClass *)(clo->getMember(L"array").obj);
	BKE_VarClass *dic = (BKE_VarClass *)(clo->getMember(L"dictionary").obj);
	arr->addNativeFunction(QUICKFUNC(load));
	arr->addNativeFunction(QUICKFUNC(save));
	arr->addNativeFunction(QUICKFUNC(saveStruct));
	dic->addNativeFunction(QUICKFUNC(saveStruct));
	clo->addNativeFunction(QUICKFUNC(loadFile));
	clo->addNativeFunction(QUICKFUNC(saveFile));
	clo->addNativeFunction(QUICKFUNC(appendFile));
	clo->addNativeFunction(QUICKFUNC(evalFile));
	clo->addNativeFunction(QUICKFUNC(fileExist));
	clo->addNativeFunction(L"log", &Parser_Util::nativeFunc_log_ol);

	p->registerClass(QUICKCLASS(System));
	p->registerClass(QUICKCLASS(Scripts));
	p->registerClass(QUICKCLASS(Channel));
	p->registerClass(QUICKCLASS(SaveData));
	p->registerClass(QUICKCLASS(History));
	p->registerClass(QUICKCLASS(Sprite));
	p->registerClass(QUICKCLASS(MessageLayer));
	p->registerClass(QUICKCLASS(Button));
	p->registerClass(QUICKCLASS(TextSprite));
#if USE_LIVE2D
	p->registerClass(QUICKCLASS(Live2DSprite));
#endif
	p->registerClass(QUICKCLASS(Event));
	p->registerClass(QUICKCLASS(EventCenter));
	p->registerClass(QUICKCLASS(Timer));
	p->registerClass(QUICKCLASS(Json));
	p->registerClass(QUICKCLASS(CSVParser));
	p->registerClass(QUICKCLASS(DownloadManager));
	p->registerClass(QUICKCLASS(Network));
	p->registerClass(QUICKCLASS(Debug));
}
