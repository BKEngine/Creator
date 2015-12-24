#if BKE_CREATOR
#include "parser.h"
#include "parserextend.h"
#else
#include "parser/parser.h"
#include "Script.h"
#include "Font/FontManager.h"
#include "Live2D/Live2DSprite.h"
#include "../spine/SkeletonAnimation.h"
#include "parserextend.h"
#include "Archive/Archive.h"
#include "Audio/Audio.h"
#include "version.h"
#include "Graphic.h"
#include "Node/Button.h"
#include "Node/Checkbox.h"
#include "Node/Image.h"
#include "Node/Layer.h"
#include "Node/Slider.h"
#include "Node/Sprite.h"
#include "Node/MessageLayer.h"
#include "Node/TextureCache.h"
#include "Node/TextSprite.h"
#include "ConfigExtension/JsonSupport.h"
#include "Node/InputBox.h"
#include "Network/HttpSupport.h"
#include "BTL/ParserHelper.h"
#include "../Effect/Effect.h"
#include "Platform/PlatformWrapper.h"
#endif

#define GETORRETURNVOID0(name, __class) GETINSTANCE();auto name=graphic->getSpriteByID(instance->index);if(!name)RETURNDEFAULT;
#define GETORRETURNVOID(name, __class) GETINSTANCE();auto name=dynamic_cast<BKE_##__class*>(graphic->getSpriteByID(instance->index));if(!name)RETURNDEFAULT;
#define GETORRETURNVOID2(name, __class) GETINSTANCE();auto name=dynamic_cast<__class*>(graphic->getSpriteByID(instance->index));if(!name)RETURNDEFAULT;

#if BKE_SYS_ANDROID
#include <jnifunc.h>
#endif

BKE_Variable BKE_EvalFile(const wstring &filename, BKE_VarClosure *clo)
{
	return BKE_Variable();
}

namespace ParserUtils
{
	//array.load(filename)
	NATIVE_FUNC(load)
	{
		RETURNDEFAULT;
	}

	//array.save(filename, append)
	NATIVE_FUNC(save)
	{
		RETURNDEFAULT;
	}

	//array.savestruct(filename, mode)
	//dic.savestruct(filename, mode)
	NATIVE_FUNC(saveStruct)
	{
		RETURNDEFAULT;
	}

	//loadFile(filename) return string
	NATIVE_FUNC(loadFile)
	{
		wstring res;
		return res;
	}

	//bool saveFile(filename, string, pos)
	//pos==-1, append
	NATIVE_FUNC(saveFile)
	{
		RETURNDEFAULT;
	}

	//appendFile(filename, string)
	NATIVE_FUNC(appendFile)
	{
		RETURNDEFAULT;
	}

	//xxx = evalFile(filename)
	NATIVE_FUNC(evalFile)
	{
		BKE_Variable res;
		return res;
	}

	//bool fileExist(filename)
	NATIVE_FUNC(fileExist)
	{
		return true;
	}

	//array.load(filename)
	NATIVE_FUNC(log_ol)
	{
		RETURNDEFAULT;
	}

	//System object
#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS System
#endif

	void System::mark(){}

	REG_GET_BEGIN(System)
	{
		GET_INFO(appName),
		GET_INFO(appPath),
		GET_INFO(appFullName),
		GET_INFO(showingText),
		GET_INFO(transCount),
		GET_INFO(actionCount),
		GET_INFO(gameName),
		GET_INFO(gameTitle),
		GET_INFO(saveHistory),
		GET_INFO(autoMode),
		GET_INFO(autoModeTime),
		GET_INFO(skipMode),
		GET_INFO(skipAll),
		GET_INFO(maxFrameSpeed),
		GET_INFO(textSpeed),
		GET_INFO(transitionEnabled),
		GET_INFO(fontSize),
		GET_INFO(fontColor),
		GET_INFO(fontStyle),
		GET_INFO(fullScreen),
		GET_INFO(FPSEnabled),
		GET_INFO(fontSizeFactor),
		GET_INFO(touchPointEnabled),
		GET_INFO(windowSize),
		GET_INFO(mouseCursorAutoHideTime),
		GET_INFO(mouseWheel),
		GET_INFO(mouseStatus),
		GET_INFO(resolutionSize),
		GET_INFO(displayedResolutionSize),
		GET_INFO(stable),
		GET_INFO(screenSize),
#if DEVELOP_VERSION
		GET_INFO(debugSkipMode),
#endif
		GET_INFO(version),
		GET_INFO(platform),
	}
	REG_GET_END;

	REG_SET_BEGIN(System)
	{
		SET_INFO(gameTitle),

		SET_INFO(saveHistory),
		SET_INFO(autoMode),
		SET_INFO(autoModeTime),

		SET_INFO(skipMode),
		SET_INFO(skipAll),
		SET_INFO(maxFrameSpeed),
		SET_INFO(textSpeed),
		SET_INFO(transitionEnabled),
		SET_INFO(fontSize),
		SET_INFO(fontColor),
		SET_INFO(fontStyle),
		SET_INFO(fullScreen),
		SET_INFO(FPSEnabled),
		SET_INFO(fontSizeFactor),
		SET_INFO(touchPointEnabled),
		SET_INFO(windowSize),
		SET_INFO(mouseCursorAutoHideTime),
		SET_INFO(displayedResolutionSize),
#if DEVELOP_VERSION		
		SET_INFO(debugSkipMode),
#endif
	};
	REG_SET_END;

	REG_FUNC_BEGIN(System)
	{
		FUNC_INFO(getKeyState),
		FUNC_INFO(getKeyboardState),
		FUNC_INFO(getEnv),
		FUNC_INFO(setEnv),
		FUNC_INFO(exit),
		FUNC_INFO(run),
		FUNC_INFO(openUrl),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
		REG_GET(System);
		REG_SET(System);
		REG_FUNC(System);
	}

	NATIVECLASS_GET(appName)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(appPath)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(appFullName)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(showingText)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(transCount)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(actionCount)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(gameName)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(gameTitle)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(gameTitle)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(saveHistory)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(saveHistory)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(autoMode)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(autoMode)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(autoModeTime)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(autoModeTime)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(skipMode)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(skipMode)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(skipAll)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(skipAll)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(maxFrameSpeed)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(maxFrameSpeed)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(textSpeed)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(textSpeed)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(windowSize)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(windowSize)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(mouseCursorAutoHideTime)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(mouseCursorAutoHideTime)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(transitionEnabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(transitionEnabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontSize)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontSize)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontColor)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontColor)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontStyle)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontStyle)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(fullScreen)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(fullScreen)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(FPSEnabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(FPSEnabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontSizeFactor)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontSizeFactor)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(touchPointEnabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(touchPointEnabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(mouseWheel)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(mouseStatus)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(resolutionSize)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(displayedResolutionSize)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(displayedResolutionSize)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(stable)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(screenSize)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(version)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(platform)
	{
		RETURNDEFAULT;
	}


#if DEVELOP_VERSION
	NATIVECLASS_GET(debugSkipMode)
	{
		return cfg->debugSkipMode;
	}

	NATIVECLASS_SET(debugSkipMode)
	{
		MINIMUMPARAMNUM(1);
		cfg->debugSkipMode = (bool)PARAM(0);
		RETURNDEFAULT;
	}
#endif

	NATIVECLASS_FUNC(getKeyState)
	{
#if BKE_SYS_WINDOWS
		MINIMUMPARAMNUM(1);
		return (int)(GetKeyState(PARAM(0)) & 0xFF);
#else
		RETURNDEFAULT;
#endif
	}

	NATIVECLASS_FUNC(getKeyboardState)
	{
#if BKE_SYS_WINDOWS
		BYTE status[256];
		GetKeyboardState(status);
		auto arr = new BKE_VarArrayTemplate<8>();
		arr->setLength(256);
		for(int i=0;i<256;i++)
			arr->quickSetMember(i, (int)(status[i] & 0xFF));
		return arr;
#else
		RETURNDEFAULT;
#endif
	}

	//getEnv(name)
	NATIVECLASS_FUNC(getEnv)
	{
		MINIMUMPARAMNUM(1);
// 		auto it = cfg->env.find(PARAM(0).asBKEStr());
// 		if (it != cfg->env.end())
// 			return it->second;
		RETURNDEFAULT;
	}

	//setEnv(name, value)
	NATIVECLASS_FUNC(setEnv)
	{
		MINIMUMPARAMNUM(2);
		/*auto it = cfg->env[PARAM(0).asBKEStr()] = PARAM(1).asBKEStr();*/
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(exit)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(run)
	{
		MINIMUMPARAMNUM(1);
#if BKE_SYS_WINDOWS
		ShellExecuteW(NULL, NULL, PARAM(0).asBKEStr().c_str(), PARAMEXIST(1) ? PARAM(1).asBKEStr().c_str() : NULL, L"", SW_SHOWNORMAL);
#elif !BKE_SYS_WP8 && !BKE_SYS_IOS
		//WP8没有system命令。IOS有，但是为了不被苹果审核干掉，还是禁用了好
		string cmd = UniToUTF8(PARAM(0).asBKEStr().getConstStr());
		return system(cmd.c_str());
#else 
#endif
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(openUrl)
	{
		RETURNDEFAULT;
	}
	
	BKE_Script *Scripts::script = NULL;

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Scripts
#endif

	void Scripts::mark(){}

	REG_FUNC_BEGIN(Scripts)
	{
		FUNC_INFO(showText),
		FUNC_INFO(runCmd),
		FUNC_INFO(goto),
		FUNC_INFO(go),
		FUNC_ALIAS_INFO(jump, goto),
		FUNC_INFO(call),
		FUNC_INFO(callback),
		FUNC_INFO(return),
		FUNC_INFO(returnTo),
		FUNC_INFO(getCurrentRead),
		FUNC_INFO(isLabelReaded),
	}
	REG_FUNC_END;
	REG_GET_BEGIN(Scripts)
	{
		GET_INFO(curLabel),
		GET_INFO(curScript),
		GET_INFO(actualCurLabel),
		GET_INFO(actualCurScript),
		GET_INFO(skipModeDisabled),
		GET_INFO(autoModeDisabled),
	}
	REG_GET_END;
	REG_SET_BEGIN(Scripts)
	{
		SET_INFO(autoModeDisabled),
		SET_INFO(skipModeDisabled),
	}
	REG_SET_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
		REG_GET(Scripts);
		REG_SET(Scripts);
		REG_FUNC(Scripts);
	}

	//showText(text)
	NATIVECLASS_FUNC(showText)
	{
		MINIMUMPARAMNUM(1);
		RETURNDEFAULT;
	}

	//runCmd(name, param)
	NATIVECLASS_FUNC(runCmd)
	{
		MINIMUMPARAMNUM(1);
		return true;
	}

	//Goto(label, script)
	NATIVECLASS_FUNC(goto)
	{
		MINIMUMPARAMNUM(2);
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(go)
	{
		RETURNDEFAULT;
	}

	//Call(label, script)
	NATIVECLASS_FUNC(call)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(callback)
	{
		RETURNDEFAULT;
	}

	//Return()
	NATIVECLASS_FUNC(return)
	{
		RETURNDEFAULT;
	}

	//ReturnTo(label, script)
	NATIVECLASS_FUNC(returnTo)
	{
		MINIMUMPARAMNUM(1);
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getCurrentRead)
	{
		RETURNDEFAULT;
	}

	//IsLabelReaded(label, script)
	NATIVECLASS_FUNC(isLabelReaded)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(curLabel)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(curScript)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(actualCurLabel)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(actualCurScript)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(skipModeDisabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(skipModeDisabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(autoModeDisabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(autoModeDisabled)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS SaveData
#endif

	void SaveData::mark(){}

	REG_FUNC_BEGIN(SaveData)
	{
		FUNC_INFO(exist),
		FUNC_INFO(getTime),
		FUNC_INFO(getText),
		FUNC_INFO(remove),
		FUNC_INFO(stashSave),
		FUNC_INFO(stashApply),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
		REG_FUNC(SaveData);
	}

	//exist(num)
	NATIVECLASS_FUNC(exist)
	{
		RETURNDEFAULT;
	}

	//getTime(num)
	NATIVECLASS_FUNC(getTime)
	{
		RETURNDEFAULT;
	}

	//getText(num)
	NATIVECLASS_FUNC(getText)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(remove)
	{
		RETURNDEFAULT;
	}

	class StashSaveItem : public BKE_NativeClass
	{
		//Retainer<BKE_VarDic> saveData;
	public:
		StashSaveItem(BKE_VarDic *dic):BKE_NativeClass(L"StashSaveItem") {};

		NATIVE_CREATENEW()
		{
			return NULL;
		}

		NATIVE_INIT()
		{
			PREPARECLASS();
			DISABLECREATE();
		}

		NATIVE_SAVE()
		{
			RETURNDEFAULT;
		}

		NATIVE_LOAD()
		{
		}
	};

	NATIVECLASS_FUNC(stashSave)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(stashApply)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Channel
#endif

	BKE_Mutex *Channel::mu = NULL;
	BKE_Audio *Channel::audio = NULL;

	void Channel::mark(){}

	REG_GET_BEGIN(Channel)
	{
		GET_INFO(isPlaying),
		GET_INFO(isPaused),
		GET_INFO(loopTimes),
		GET_INFO(loopTo),
		GET_INFO(volume),
		GET_INFO(fileName),
		GET_INFO(musicVolume),
		GET_INFO(soundVolume),
		GET_INFO(voiceVolume),
	}
	REG_GET_END;
	REG_SET_BEGIN(Channel)
	{
		SET_INFO(loopTimes),
		SET_INFO(loopTo),
		SET_INFO(volume),
		SET_INFO(musicVolume),
		SET_INFO(soundVolume),
		SET_INFO(voiceVolume),
	}
	REG_SET_END;
	REG_FUNC_BEGIN(Channel)
	{
		FUNC_INFO(load),
		FUNC_INFO(play),
		FUNC_INFO(pause),
		FUNC_INFO(resume),
		FUNC_INFO(stop),
		FUNC_INFO(tell),
		FUNC_INFO(seek),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_GET(Channel);
		REG_SET(Channel);
		REG_FUNC(Channel);
	}

	NATIVECLASS_CREATENEW()
	{
		if (paramarray == NULL)
			return new Channel(0);
		return new Channel(paramarray->getMember(0));
	};

	NATIVECLASS_GET(isPlaying)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(isPaused)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(loopTimes)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(loopTimes)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(loopTo)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(loopTo)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(volume)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(volume)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(fileName)
	{
		RETURNDEFAULT;
	}

	//for static function
	NATIVECLASS_GET(musicVolume)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(musicVolume)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(soundVolume)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(soundVolume)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(voiceVolume)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(voiceVolume)
	{
		RETURNDEFAULT;
	}

	//load(file)
	NATIVECLASS_FUNC(load)
	{
		RETURNDEFAULT;
	}

	//play([file], [volume], [looptime])
	NATIVECLASS_FUNC(play)
	{
		RETURNDEFAULT;
	}

	//pause()
	NATIVECLASS_FUNC(pause)
	{
		RETURNDEFAULT;
	}

	//resume()
	NATIVECLASS_FUNC(resume)
	{
		RETURNDEFAULT;
	}

	//stop(fadetime)
	NATIVECLASS_FUNC(stop)
	{
		RETURNDEFAULT;
	}

	//seek(time)
	NATIVECLASS_FUNC(seek)
	{
		RETURNDEFAULT;
	}

	//double tell()
	NATIVECLASS_FUNC(tell)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS History
#endif

	void History::mark(){}

	REG_GET_BEGIN(History)
	{
		GET_INFO(data),
		GET_INFO(startIndex),
		GET_INFO(curIndex),
		GET_INFO(nextIndex),
		GET_INFO(count),
		GET_INFO(enabled),
		GET_INFO(maxPage),
		GET_INFO(recordMode),
		GET_INFO(maxChars),
	}
	REG_GET_END;
	REG_SET_BEGIN(History)
	{
		SET_INFO(maxChars),
		SET_INFO(enabled),
		SET_INFO(maxPage),
		SET_INFO(recordMode),
	}
	REG_SET_END;
	REG_FUNC_BEGIN(History)
	{
		FUNC_INFO(getDataAtLine),
		FUNC_INFO(reline),
		FUNC_INFO(repage),
		FUNC_INFO(add),
		FUNC_INFO(clear),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
		REG_GET(History);
		REG_SET(History);
		REG_FUNC(History);
	}

	NATIVECLASS_GET(data)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(enabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(enabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(maxPage)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(maxPage)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(startIndex)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(curIndex)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(nextIndex)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(count)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(recordMode)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(recordMode)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(maxChars)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(maxChars)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getDataAtLine)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(reline)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(repage)
	{
		RETURNDEFAULT;
	}

	//add(text, reline=false)
	NATIVECLASS_FUNC(add)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(clear)
	{
		RETURNDEFAULT;
	}

	BKE_Graphic *Sprite::graphic = NULL;

	/**
	 * @class Sprite
	 * @brief 精灵类
	 */
#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Sprite
#endif

	void Sprite::mark(){}

	REG_GET_BEGIN(Sprite)
	{
		GET_INFO(type),
		GET_INFO(index),
		GET_INFO(value),
		GET_INFO(x),
		GET_INFO(y),
		GET_INFO(width),
		GET_INFO(height),
		GET_INFO(imageWidth),
		GET_INFO(imageHeight),
		GET_INFO(opacity),
		GET_INFO(zorder),
		GET_INFO(rect),
		GET_INFO(filename),
		GET_INFO(angle),
		GET_INFO(visible),

		GET_INFO(disabled),
		GET_INFO(focusable),

		GET_INFO(onEnter),
		GET_INFO(onHover),
		GET_INFO(onLeave),
		GET_INFO(onTouchBegan),
		GET_INFO(onTouchMoved),
		GET_INFO(onTouchEnded),
		GET_INFO(onClick),
		GET_INFO(onRClick),
		GET_INFO(onMouseWheel),
	}
	REG_GET_END;
	REG_SET_BEGIN(Sprite)
	{
		SET_INFO(value),
		SET_INFO(x),
		SET_INFO(y),
		SET_INFO(width),
		SET_INFO(height),
		SET_INFO(opacity),
		SET_INFO(zorder),
		SET_INFO(rect),
		SET_INFO(angle),
		SET_INFO(visible),

		SET_INFO(disabled),
		SET_INFO(disabledRecursive),
		SET_INFO(focusable),

		SET_INFO(onEnter),
		SET_INFO(onHover),
		SET_INFO(onLeave),
		SET_INFO(onTouchBegan),
		SET_INFO(onTouchMoved),
		SET_INFO(onTouchEnded),
		SET_INFO(onClick),
		SET_INFO(onRClick),
		SET_INFO(onMouseWheel),
	}
	REG_SET_END;
	REG_FUNC_BEGIN(Sprite)
	{
		FUNC_INFO(getParent),
		FUNC_INFO(removeFromParent),
		FUNC_INFO(removeChild),
		FUNC_INFO(hasChild),
		FUNC_INFO(saveImage),
		FUNC_INFO(getChildren),

		FUNC_INFO(idlesp),
		FUNC_INFO(create),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_GET(Sprite);
		REG_SET(Sprite);
		REG_FUNC(Sprite);
	}

	/**
	 * @property_getter type
	 * @brief 获取类型
	 */
	NATIVECLASS_GET(type)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(filename)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(index)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(value)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(value)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(angle)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(angle)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(visible)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(visible)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(x)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(x)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(rect)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(rect)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(width)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(width)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(height)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(height)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(imageWidth)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(imageHeight)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(y)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(y)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(opacity)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(opacity)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(zorder)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(zorder)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(disabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(disabled)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(disabledRecursive)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(focusable)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(focusable)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onEnter)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onHover)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onLeave)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onTouchBegan)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onTouchMoved)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onTouchEnded)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onClick)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(onEnter)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(onHover)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(onLeave)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(onTouchBegan)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(onTouchMoved)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(onTouchEnded)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(onClick)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onRClick)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(onRClick)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onMouseWheel)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(onMouseWheel)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(removeFromParent)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(removeChild)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(hasChild)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getParent)
	{
		RETURNDEFAULT;
	}

	//saveImage(filename, hasChildren=false)	//save as png
	NATIVECLASS_FUNC(saveImage)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getChildren)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(idlesp)
	{
		RETURNDEFAULT;
	}

	//create(index, file, rect)
	NATIVECLASS_FUNC(create)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS MessageLayer
#endif

	void MessageLayer::mark(){}

	REG_GET_BEGIN(MessageLayer)
	{
		GET_INFO(curposX),
		GET_INFO(curposY),
	}
	REG_GET_END;

	REG_SET_BEGIN(MessageLayer)
	{
		SET_INFO(curposX),
		SET_INFO(curposY),
	}
	REG_SET_END;

	NATIVECLASS_INIT()
	{
		NATIVECLASS_SUPERINIT();
		PREPARECLASS();
		//same as parent class
		//_class->cannotcreate = false;
		//this->graphic = BKE_Graphic::sharedGraphic();
		REG_GET(MessageLayer);
		REG_SET(MessageLayer);
	}

	NATIVECLASS_GET(curposX)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(curposX)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(curposY)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(curposY)
	{
		RETURNDEFAULT;
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

	REG_GET_BEGIN(TextSprite)
	{
		GET_INFO(color),
		GET_INFO(text),
		GET_INFO(fontName),
		GET_INFO(fontStyle),
		GET_INFO(fontSize),
	}
	REG_GET_END;
	REG_SET_BEGIN(TextSprite)
	{
		SET_INFO(color),
		SET_INFO(text),
		SET_INFO(fontName),
		SET_INFO(fontStyle),
		SET_INFO(fontSize),
	}
	REG_SET_END;

	NATIVECLASS_INIT()
	{
		NATIVECLASS_SUPERINIT();
		PREPARECLASS();
		REG_GET(TextSprite);
		REG_SET(TextSprite);
	}

	NATIVECLASS_GET(color)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(text)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontName)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontStyle)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(fontSize)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(color)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(text)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontName)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontStyle)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(fontSize)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Button
#endif

	void Button::mark(){}

	REG_GET_BEGIN(Button)
	{
		GET_INFO(idle),
		GET_INFO(hover),
		GET_INFO(click),
	}
	REG_GET_END;

	NATIVECLASS_INIT()
	{
		NATIVECLASS_SUPERINIT();
		PREPARECLASS();
		//same as parent class
		REG_GET(Button);
	}

	NATIVECLASS_GET(idle)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(hover)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(click)
	{
		RETURNDEFAULT;
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

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Live2DSprite
#endif

	void Live2DSprite::mark(){}

	REG_GET_BEGIN(Live2DSprite)
	{
		GET_INFO(onFlick),
		GET_INFO(onTap),
	}
	REG_GET_END;
	REG_SET_BEGIN(Live2DSprite)
	{
		SET_INFO(onFlick),
		SET_INFO(onTap),
	}
	REG_SET_END;
	REG_FUNC_BEGIN(Live2DSprite)
	{
		FUNC_INFO(startMotion),
		FUNC_INFO(startMotionFile),
		FUNC_INFO(startRandomMotion),
		FUNC_INFO(stopMotion),
		FUNC_INFO(getMotionNum),

		FUNC_INFO(setExpression),
		FUNC_INFO(setExpressionFile),
		FUNC_INFO(setRandomExpression),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		NATIVECLASS_SUPERINIT();
		PREPARECLASS();

		REG_GET(Live2DSprite);
		REG_SET(Live2DSprite);
		REG_FUNC(Live2DSprite);
	}

	NATIVECLASS_FUNC(startMotion)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(startRandomMotion)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(startMotionFile)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(stopMotion)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(getMotionNum)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(setExpression)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(setRandomExpression)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(setExpressionFile)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(onTap)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onTap)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(onFlick)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onFlick)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Spine
#endif

	void Spine::mark(){}

	REG_GET_BEGIN(Spine)
	{
		GET_INFO(boundingBox),
		GET_INFO(timeScale),
	}
	REG_GET_END;
	REG_SET_BEGIN(Spine)
	{
		SET_INFO(timeScale),
	}
	REG_SET_END;

	REG_FUNC_BEGIN(Spine)
	{
		FUNC_INFO(setAnimation),
		FUNC_INFO(addAnimation),
		FUNC_INFO(mixAnimation),
		FUNC_INFO(clearTrack),
		FUNC_INFO(clearAllTrack),

		FUNC_INFO(setStartListener),
		FUNC_INFO(setEndListener),
		FUNC_INFO(setCompleteListener),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		NATIVECLASS_SUPERINIT();
		PREPARECLASS();
		REG_GET(Spine);
		REG_SET(Spine);
		REG_FUNC(Spine);
	}

	NATIVECLASS_GET(boundingBox)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(setAnimation)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(addAnimation)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(mixAnimation)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(clearTrack)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(clearAllTrack)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(setStartListener)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(setEndListener)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(setCompleteListener)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(timeScale)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(timeScale)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Event
#endif

	void Event::mark(){}

	REG_GET_BEGIN(Event)
	{
		GET_INFO(name),
		GET_INFO(enable),
		GET_INFO(label),
		GET_INFO(file),
		GET_INFO(type),
		GET_INFO(ignore),
		GET_INFO(exp),
		GET_INFO(param),
	}
	REG_GET_END;
	REG_SET_BEGIN(Event)
	{
		SET_INFO(name),
		SET_INFO(enable),
		SET_INFO(label),
		SET_INFO(file),
		SET_INFO(type),
		SET_INFO(ignore),
		SET_INFO(exp),
		SET_INFO(param),
	}
	REG_SET_END;
	REG_FUNC_BEGIN(Event)
	{
		FUNC_INFO(call),
		FUNC_INFO(toVar),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_GET(Event);
		REG_SET(Event);
		REG_FUNC(Event);
	}

	NATIVECLASS_CREATENEW()
	{
		if (!paramarray)
		{
			return new Event;
		}
		return new Event();
	}

	NATIVECLASS_SAVE()
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_LOAD()
	{
	}

	NATIVECLASS_SET(name)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(enable)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(label)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(file)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(type)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(ignore)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(exp)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(param)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(func)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(name)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(enable)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(label)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(file)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(type)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(ignore)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(exp)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(param)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(func)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(call)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(toVar)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS EventCenter
#endif

	void EventCenter::mark(){}

	REG_FUNC_BEGIN(EventCenter)
	{
		FUNC_INFO(register),
		FUNC_INFO(delete),
		FUNC_INFO(sendSystem),
		FUNC_INFO(send),
		FUNC_ALIAS_INFO(trigger, send),
		FUNC_ALIAS_INFO(triggerSystem, sendSystem),
		FUNC_INFO(get),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();

		REG_FUNC(EventCenter);
	}

	NATIVECLASS_FUNC(register)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(delete)
	{
		RETURNDEFAULT;
	}

	//systemCall(name, param);
	NATIVECLASS_FUNC(sendSystem)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(send)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(get)
	{
		RETURNDEFAULT;
	}
	
#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Timer
#endif

	void Timer::mark(){}

	REG_GET_BEGIN(Timer)
	{
		GET_INFO(enable),
		GET_INFO(interval),
	}
	REG_GET_END;
	REG_SET_BEGIN(Timer)
	{
		SET_INFO(enable),
		SET_INFO(interval),
	}
	REG_SET_END;
	REG_FUNC_BEGIN(Timer)
	{
		FUNC_INFO(setTimeout),
		FUNC_INFO(clearTimeout),
		FUNC_INFO(forceTrigger),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_GET(Timer);
		REG_SET(Timer);
		REG_FUNC(Timer);
	}

	NATIVECLASS_GET(enable)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(enable)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(interval)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(interval)
	{
		RETURNDEFAULT;
	}

	//setTimeout(func, delay, params, ...)
	NATIVECLASS_FUNC(setTimeout)
	{
		RETURNDEFAULT;
	}

	//clearTimeout(func)
	NATIVECLASS_FUNC(clearTimeout)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(forceTrigger)
	{
		RETURNDEFAULT;
	}


#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Json
#endif

	void Json::mark(){}

	REG_FUNC_BEGIN(Json)
	{
		FUNC_INFO(toJson),
		FUNC_INFO(fromJson),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_FUNC(Json);
	}

	NATIVECLASS_FUNC(toJson)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(fromJson)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS CSVParser
#endif

	void CSVParser::mark(){}

	REG_FUNC_BEGIN(CSVParser)
	{
		FUNC_INFO(doLine),
		FUNC_INFO(doDocument),
		FUNC_INFO(doFile),

		FUNC_INFO(fromDocument),
		FUNC_INFO(fromFile),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_FUNC(CSVParser);
	}

	NATIVECLASS_CREATENEW()
	{
		return new CSVParser;
	}

	NATIVECLASS_FUNC(doLine)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(doFile)
	{
		RETURNDEFAULT;
	}
	
	NATIVECLASS_FUNC(doDocument)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(fromDocument)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(fromFile)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS DownloadManager
#endif

	void DownloadManager::mark(){}

	REG_GET_BEGIN(DownloadManager)
	{
		GET_INFO(packageUrl),
		GET_INFO(outFileName),
	}
	REG_GET_END;
	REG_SET_BEGIN(DownloadManager)
	{
		SET_INFO(outFileName),
		SET_INFO(packageUrl),
		SET_INFO(onSuccess),
		SET_INFO(onError), // error_string
		SET_INFO(onProgress), // now,total
	}
	REG_SET_END;
	REG_FUNC_BEGIN(DownloadManager)
	{
		FUNC_INFO(begin),
		FUNC_INFO(stop),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		ENABLECREATE();
		REG_GET(DownloadManager);
		REG_SET(DownloadManager);
		REG_FUNC(DownloadManager);
	}

	//url, filename
	NATIVECLASS_CREATENEW()
	{
		return new DownloadManager();
	}

	NATIVECLASS_SAVE()
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_LOAD()
	{
	}

	NATIVECLASS_FUNC(begin)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(stop)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(packageUrl)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(outFileName)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(packageUrl)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_GET(outFileName)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onError)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onProgress)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_SET(onSuccess)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Network
#endif

	void Network::mark(){}

	REG_FUNC_BEGIN(Network)
	{
		FUNC_INFO(download),
		FUNC_INFO(getString),
		FUNC_INFO(request),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
		REG_FUNC(Network);
	}
	
	NATIVECLASS_FUNC(download)
	{
		RETURNDEFAULT;
	}
	
	NATIVECLASS_FUNC(getString)
	{
		RETURNDEFAULT;
	}

	enum bkeHttpType{
		bkeHttpGet,
		bkeHttpPost,
		bkeHttpPut,
		bkeHttpDelete,
		bkeHttpUnknown
	};

	map<wstring, bkeHttpType> _e2h = {
		{ L"put", bkeHttpPut } ,
		{ L"get", bkeHttpGet },
		{ L"post", bkeHttpPost },
		{ L"delete", bkeHttpDelete },
	};

	NATIVECLASS_FUNC(request)
	{
		RETURNDEFAULT;
	}

#ifdef CURRENTCLASS
#undef CURRENTCLASS
#define CURRENTCLASS Debug
#endif

	void Debug::mark(){}

	REG_FUNC_BEGIN(Debug)
	{
		FUNC_INFO(printCallStack),
		FUNC_INFO(memoryStatus),
	}
	REG_FUNC_END;

	NATIVECLASS_INIT()
	{
		PREPARECLASS();
		DISABLECREATE();
		REG_FUNC(Debug);
	}

	NATIVECLASS_FUNC(printCallStack)
	{
		RETURNDEFAULT;
	}

	NATIVECLASS_FUNC(memoryStatus)
	{
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
	clo->addNativeFunction(L"log", &ParserUtils::nativeFunc_log_ol);

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
#if USE_SPINE
	p->registerClass(QUICKCLASS(Spine));
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
