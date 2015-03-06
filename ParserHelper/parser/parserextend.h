#pragma once

//NULL template for all classes

#include "extend.h"

#include <unordered_set>
#include <unordered_map>

//BKE_Variable BKE_EvalFile(const wstring &filename, BKE_VarClosure *clo = BKE_VarClosure::global());

extern void registerExtendFunction();

typedef int BKE_ObjectID;
typedef int BKE_Event;	//fake
//class BKE_Script;
//class BKE_Mutex;
//class BKE_Audio;
//class BKE_Graphic;

namespace Parser_Util
{
	/**
	*	@class array
	*	@static
	*	@param string filename 文件名
	*	@return array
	*	@brief 读取文件并按照换行切分为数组。
	*	@example 以下将演示一个从内容为
	*		a
	*		b
	*		c
	*		的1.txt文件载入为数组的示例：
	*	@example_code [].load("1.txt")
	*	@example_result ["a","b","c"]
	*/
	NATIVE_FUNC(load);

	/**
	 *	@class array
	 *	@param string filename 文件名
	 *	@param string mode="" 模式。若首字母为'o'则字符串的其余部分代表一个偏移，文件将从该偏移处进行覆盖保存，负值将从文件末尾进行计算。若首字母为'z'则与'o'类似，只不过将启用压缩。
	 *	将字符串数组按照换行合并，并保存到文件
	 */
	NATIVE_FUNC(save);

	/**
	 *	@class array|dictionary
	 *	@param string filename 文件名
	 *	@param string mode="" 模式。若首字母为'o'则字符串的其余部分代表一个偏移，文件将从该偏移处进行覆盖保存，负值将从文件末尾进行计算。若首字母为'z'则与'o'类似，只不过将启用压缩。
	 *	将本结构（数组或者字典）进行序列化，并保存到文件。
	 *	文件的内容可以直接使用{@link evalFile}进行反序列化，读取至变量。
	 */
	NATIVE_FUNC(saveStruct);

	/**
	 *	@param string filename 文件名
	 *	@return string
	 *	读取一个文件，返回里面的原始内容（字符串）
	 */
	NATIVE_FUNC(loadFile);

	/**
	 *	@param string filename 文件名
	 *	@param string str 将要保存的字符串
	 *	@param int pos=0 保存的偏移。若为负，则从文件尾进行计算。
	 *	@return bool 是否成功
	 *	将一个字符串写入文件。
	 */
	NATIVE_FUNC(saveFile);


	/**
	 *	@param string filename 文件名
	 *	@param string str 将要保存的字符串
	 *	@return bool 是否成功
	 *	将一个字符串添加到文件末尾。等同于{@link saveFile}(filename, str, -1);
	 */
	NATIVE_FUNC(appendFile);

	/**
	 *	@param string filename 文件名
	 *	@param bool krmode=false 若为真，表示双引号字符串可以像单引号字符串一样使用转义（此时不使用原有的""表示"的功能）
	 *	@param bool rawstr=false 若为真，表示字符串可以跨行
	 *	@return var 反序列化的结果
	 *  对文件读取并且解析（反序列化），将结果返回。
	 *  等同于{@link eval}({@link loadFile}(filename, krmode, rawstr));
	 */
	NATIVE_FUNC(evalFile);

	/**
	 *	@param filename 文件名
	 *	@return bool
	 *	判断文件是否存在且可以被读取（即文件是否存在于文件系统或者封包中）
	 */
	NATIVE_FUNC(fileExist);

	/**
	 *	@function log
	 *	@param var
	 *	序列化一个变量并且将结果打印到控制台。
	 */
	NATIVE_FUNC(log_ol);

	/**
	 *	系统类。
	 *	{@b 不可实例化。}
	 */
	class System :public BKE_NativeClass
	{
	private:
		//static BKE_SystemConfig *cfg;

	public:
		System() :BKE_NativeClass(L"System"){};

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }

		/**
		 *	主程序的文件名（不含路径）
		 *  @type string
		 */
		NATIVE_GET(appName);

		/**
		 *  主程序的路径
		 *  @type string
		 */
		NATIVE_GET(appPath);

		/**
		 *  主程序的全路径（包含文件名）
		 *  @type string
		 */
		NATIVE_GET(appFullName);

		/**
		 *  当前是否正在显示文字
		 *  @type bool
		 */
		NATIVE_GET(showingText);

		/**
		 *  当前正在执行trans的层的数量
		 *  @type int
		 */
		NATIVE_GET(transCount);

		/**
		 *  当前正在执行action的层的数量
		 *  @type int
		 */
		NATIVE_GET(actionCount);

		/**
		 *  游戏名称
		 *  @type string
		 */
		NATIVE_GET(gameName);
		
		/**
		 *  游戏标题
		 *  @type string
		 */
		NATIVE_GET(gameTitle);

		NATIVE_SET(gameTitle);

		/**
		 *	是否将历史记录保存进存档
		 *	@type bool
		 */
		NATIVE_GET(saveHistory);
		NATIVE_SET(saveHistory);

		/**
		 *	是否开启自动模式
		 *	@type bool
		 */
		NATIVE_GET(autoMode);
		NATIVE_SET(autoMode);

		/**
		 *	自动模式时间，单位ms
		 *	@type int
		 */
		NATIVE_GET(autoModeTime);
		NATIVE_SET(autoModeTime);

		/**
		 *	是否开启跳过模式
		 *	@type bool
		 */
		NATIVE_GET(skipMode);
		NATIVE_SET(skipMode);

		/**
		 *	是否允许跳过未读文本
		 *	@type bool
		 */
		NATIVE_GET(skipAll);
		NATIVE_SET(skipAll);

		/**
		 *	最高帧速
		 *	{@b 除非你知道你在做什么，否则请不要动这个变量。}
		 *	@type int
		 */
		NATIVE_GET(maxFrameSpeed);
		NATIVE_SET(maxFrameSpeed);

		/**
		 *	文字速度
		 *	值从0到100，代表每显示一个文字耗时100ms到0ms（瞬间显示）。
		 *	@type int
		 */
		NATIVE_GET(textSpeed);
		NATIVE_SET(textSpeed);

		/**
		 *	是否允许渐变特效
		 *	若为否，则所有trans特效皆适用trans的normal模式。
		 *	@type bool
		 */
		NATIVE_SET(transitionEnabled);
		NATIVE_GET(transitionEnabled);

		/**
		 *	{@b 当前消息层}的文字大小
		 *	@type int
		 */
		NATIVE_SET(fontSize);
		NATIVE_GET(fontSize);

		/**
		 *	{@b 当前消息层}的文字颜色
		 *	@type int
		 */
		NATIVE_SET(fontColor);
		NATIVE_GET(fontColor);

		/**
		 *	{@b 当前消息层}的文字样式
		 *	@type int
		 */
		NATIVE_SET(fontStyle);
		NATIVE_GET(fontStyle);

		/**
		 *	是否全屏显示
		 *	{@b 在移动端此参数无效。}
		 *	@type bool
		 */
		NATIVE_GET(fullScreen);
		NATIVE_SET(fullScreen);

		/**
		 *	是否开启FPS显示
		 *	@type bool
		 */
		NATIVE_GET(FPSEnabled);
		NATIVE_SET(FPSEnabled);

		/**
		 *	文字全局缩放比例
		 *	用于外部调整文字大小。默认1.0
		 *	@type number
		 */
		NATIVE_GET(fontSizeFactor);
		NATIVE_SET(fontSizeFactor);

		/**
		 *	是否显示触摸点
		 *	@type bool
		 */
		NATIVE_GET(touchPointEnabled);
		NATIVE_SET(touchPointEnabled);

		/**
		 *	窗口大小
		 *	{@b 注意，窗口大小并不等于游戏分辨率大小。你不应该将此变量用于游戏界面相关的计算中。}
		 *	@type vec2
		 */
		NATIVE_GET(windowSize);
		NATIVE_SET(windowSize);

		/**
		 *	鼠标指针的文件名
		 *	若为""，则不存在鼠标指针。
		 *	@type string
		 */
		NATIVE_GET(mouseCursor);
		NATIVE_SET(mouseCursor);

		/**
		 *	鼠标自动消失的时间
		 *	{@b 仅启用自定义鼠标指针时有效。}
		 *	单位为ms，默认为2000
		 *	@type int
		 */
		NATIVE_GET(mouseCursorAutoHideTime);
		NATIVE_SET(mouseCursorAutoHideTime);

		/**
		 *	鼠标滚轮的旋转量
		 *	正数是向上。
		 *	@type int
		 */
		NATIVE_GET(mouseWheel);

		/**
		 *	鼠标的状态
		 *	类型为数组，长度固定为5。5个数组成员分别为鼠标的X坐标，Y坐标（整数），左、中、右三个键的按下状态（bool）
		 *	@type array
		 */
		NATIVE_GET(mouseStatus);

		/**
		 *	游戏分辨率
		 *	可以用于UI相关的计算，长度固定为2，2个数组成员分别为宽和高。
		 *	@type array
		 */
		NATIVE_GET(resolutionSize);

		/**
		*	脚本静止状态
		*	当脚本中遇到l, p, click, waitbutton并等待的时候被标记为true。
		*	@type bool
		*/
		NATIVE_GET(stable);

		/**
		 *	BKEngine版本号
		 *	@type int
		 */
		NATIVE_GET(version);

		/**
		 *	当前运行的平台
		 *	值为"windows","macos","linux","ios","android","winphone"其中之一
		 *	@type string
		 */
		NATIVE_GET(platform);
		

		NATIVE_FUNC(getKeyState);
		NATIVE_FUNC(getKeyboardState);


		NATIVE_FUNC(getEnv);
		NATIVE_FUNC(setEnv);

		/**
		 *	退出程序
		 */
		NATIVE_FUNC(exit);


		NATIVE_FUNC(run);

		/**
		 *	打开一个网页
		 */
		NATIVE_FUNC(openUrl);
	};

	/**
	 *	脚本类
	 *	{@b 不可实例化。}
	 */
	/////////////////////////////////////////////////////////////////////		Scripts
	class Scripts :public BKE_NativeClass
	{
	private:
		//friend void handleError(bkeLogLevel err);
		//static BKE_Script *script;

	public:
		Scripts() :BKE_NativeClass(L"Scripts"){};

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }
		
		/**
		 *	@param string text
		 *	@param function callback=void 文字显示完毕时触发回调
		 *	在文本框打印一段文字。等同于bkscr中text命令。
		 */
		NATIVE_FUNC(showText);

		/**
		 *	@param string name 调用的bks指令名
		 *	@param dictionary param 调用的参数列表
		 *	@param function callback=void 函数真正执行完毕时触发的回调
		 *	调用一个bkscr指令。
		 *	如调用wait*系列函数，则在wait的条件被触发、等待结束时callback回调才被触发。正常指令则立刻触发回调。
		 *	text指令则等待文字打印完成才触发回调。
		 */
		NATIVE_FUNC(runCmd);

		
		/**
		*	让脚本从等待状态（等待按钮、等待点击等等）恢复，继续执行
		*/
		NATIVE_FUNC(go);

		/**
		 *	@param string label 标签
		 *	@param string file="" 文件
		 *	跳转到某个脚本，等同于bkscr中jump指令
		 *	和{@link jump}指令等同
		 */
		NATIVE_FUNC(goto);

		/**
		 *	@param string label 标签
		 *	@param string file="" 文件
		 *	跳转到某个脚本，等同于bkscr中jump指令
		 *	和{@link goto}指令等同
		 */
		NATIVE_FUNC(jump);

		/**
		 *	@param string label 标签
		 *	@param string file="" 文件
		 *	访问某个标签，等同于bkscr中jump指令
		 *	和{@link goto}指令等同
		 */
		NATIVE_FUNC(call);

		//Call(label, script, blocked, exp, param, ignored)
		NATIVE_FUNC(callback);

		//Return()
		NATIVE_FUNC(return);

		//ReturnTo(label, script)
		NATIVE_FUNC(returnTo);

		//GetCurrentRead()
		NATIVE_FUNC(getCurrentRead);

		//IsLabelReaded(label, script)
		NATIVE_FUNC(isLabelReaded);

		//curLabel
		NATIVE_GET(curLabel);

		//curScript
		NATIVE_GET(curScript);

		//actualCurLabel
		NATIVE_GET(actualCurLabel);

		//actualCurScript
		NATIVE_GET(actualCurScript);
	};

	/////////////////////////////////////////////////////////////////////		SaveData
	class SaveData :public BKE_NativeClass
	{
	private:
		//static BKE_Script *script;

	public:
		SaveData() :BKE_NativeClass(L"SaveData"){};

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }

		//exist(num)
		NATIVE_FUNC(exist);

		//getTime(num)
		NATIVE_FUNC(getTime);

		//getText(num)
		NATIVE_FUNC(getText);

		//GetCurretnSave()
		NATIVE_FUNC(getCurrentSave);
	};

	/////////////////////////////////////////////////////////////////////		Channel
	class Channel :public BKE_NativeClass
	{
	public:
		//int channel_index;

		//static BKE_Mutex *mu;

		//static BKE_Audio *audio;

		Channel(bkplong i=0) :BKE_NativeClass(L"Channel")/*, channel_index(i)*/{};

		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_SAVE()
		{
			//return channel_index;
			RETURNDEFAULT;
		}

		NATIVE_LOAD()
		{
			//channel_index = var;
		}
		
		NATIVE_GET(isPlaying);
		NATIVE_GET(isPaused);
		NATIVE_GET(loopTimes);
		NATIVE_SET(loopTimes);
		NATIVE_GET(loopTo);
		NATIVE_SET(loopTo);
		NATIVE_GET(volume);
		NATIVE_SET(volume);
		NATIVE_GET(fileName);

		//for static function
		NATIVE_GET(musicVolume);
		NATIVE_SET(musicVolume);
		NATIVE_GET(soundVolume);
		NATIVE_SET(soundVolume);
		NATIVE_GET(voiceVolume);
		NATIVE_SET(voiceVolume);

		//load(file)
		NATIVE_FUNC(load);

		//play([file], [looptime])
		NATIVE_FUNC(play);

		//pause()
		NATIVE_FUNC(pause);

		//resume()
		NATIVE_FUNC(resume);

		//stop(fadetime)
		NATIVE_FUNC(stop);

		//seek(time)
		NATIVE_FUNC(seek);

		//double tell()
		NATIVE_FUNC(tell);
	};

	/////////////////////////////////////////////////////////////////////		History
	class History :public BKE_NativeClass
	{
	public:
		//static BKE_History *his;

		History() :BKE_NativeClass(L"History"){};

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }

		NATIVE_GET(data);
		NATIVE_GET(enabled);
		NATIVE_SET(enabled);
		NATIVE_GET(maxPage);
		NATIVE_SET(maxPage);
		NATIVE_GET(startIndex);
		NATIVE_GET(curIndex);
		NATIVE_GET(nextIndex);
		NATIVE_GET(count);
		NATIVE_GET(recordMode);
		NATIVE_SET(recordMode);
		NATIVE_GET(maxChars);
		NATIVE_SET(maxChars);

		NATIVE_FUNC(getDataAtLine);
		NATIVE_FUNC(reline);
		NATIVE_FUNC(repage);
		NATIVE_FUNC(add);
	};

	/////////////////////////////////////////////////////////////////////		Sprite
	class Sprite :public BKE_NativeClass
	{
	public:
		//static BKE_Graphic *graphic;

		//BKE_ObjectID index;

		Sprite(BKE_ObjectID i = -1) :BKE_NativeClass(L"Sprite"){/* index = i; */ };

		NATIVE_INIT();

		NATIVE_CREATENEW()
		{
			if (!paramarray)
				return new Sprite(-1);
			return new Sprite(PARAM(0)); 
		}

		NATIVE_SAVE()
		{
			//return index;
			RETURNDEFAULT;
		}

		NATIVE_LOAD()
		{
			//index = var;
		}

		NATIVE_GET(type);
		NATIVE_GET(index);

		NATIVE_GET(value);
		NATIVE_GET(x);
		NATIVE_GET(y);
		NATIVE_GET(width);
		NATIVE_GET(height);
		NATIVE_GET(imageWidth);
		NATIVE_GET(imageHeight);
		NATIVE_GET(opacity);
		NATIVE_GET(zorder);
		NATIVE_GET(rect);

		NATIVE_SET(value);
		NATIVE_SET(x);
		NATIVE_SET(y);
		NATIVE_SET(width);
		NATIVE_SET(height);
		NATIVE_SET(opacity);
		NATIVE_SET(zorder);
		NATIVE_SET(rect);

		NATIVE_SET(disabled);
		NATIVE_GET(disabled);
		NATIVE_SET(disabledRecursive);
// 		NATIVE_SET(ignoreEvent);
// 		NATIVE_GET(ignoreEvent);
// 		NATIVE_SET(ignoreEventRecursive);

		NATIVE_SET(onEnter);
		NATIVE_SET(onHover);
		NATIVE_SET(onLeave);
		NATIVE_SET(onTouchBegan);
		NATIVE_SET(onTouchMoved);
		NATIVE_SET(onTouchEnded);
		NATIVE_SET(onClick);
		NATIVE_GET(onEnter);
		NATIVE_GET(onHover);
		NATIVE_GET(onLeave);
		NATIVE_GET(onTouchBegan);
		NATIVE_GET(onTouchMoved);
		NATIVE_GET(onTouchEnded);
		NATIVE_GET(onClick);

		NATIVE_FUNC(getParent);
		//saveImage(filename, hasChildren=false)	//保存为 png
		NATIVE_FUNC(saveImage);

		//static方法
		NATIVE_FUNC(idlesp);
		NATIVE_FUNC(create);
	};

	/////////////////////////////////////////////////////////////////////		MessageLayer
	class MessageLayer :public Sprite
	{
		NATIVE_SUPPERINIT(Sprite);
	public:
		MessageLayer(BKE_ObjectID i = -1) :Sprite(i){};

		NATIVE_INIT();

		NATIVE_CREATENEW()
		{
			if (!paramarray)
				return new MessageLayer(-1);
			return new MessageLayer(PARAM(0));
		}

		NATIVE_GET(curposX);
		NATIVE_GET(curposY);

		NATIVE_SET(curposX);
		NATIVE_SET(curposY);
	};

	/////////////////////////////////////////////////////////////////////		TextSprite
	class TextSprite :public Sprite
	{
		NATIVE_SUPPERINIT(Sprite);
	public:
		TextSprite(BKE_ObjectID i = -1) :Sprite(i){};

		NATIVE_INIT();

		NATIVE_CREATENEW()
		{
			if (!paramarray)
				return new TextSprite(-1);
			return new TextSprite(PARAM(0));
		}

		NATIVE_GET(color);
		NATIVE_GET(text);
		NATIVE_GET(fontName);
		NATIVE_GET(fontStyle);
		NATIVE_GET(fontSize);

		NATIVE_SET(color);
		NATIVE_SET(text);
		NATIVE_SET(fontName);
		NATIVE_SET(fontStyle);
		NATIVE_SET(fontSize);
	};

	/////////////////////////////////////////////////////////////////////		Button
	class Button : public Sprite
	{
		NATIVE_SUPPERINIT(Sprite);
	public:
		Button(BKE_ObjectID i = -1) :Sprite(i){};

		NATIVE_INIT();

		NATIVE_CREATENEW()
		{
			if (!paramarray)
				return new Button(-1);
			return new Button(PARAM(0));
		}

		NATIVE_GET(idle);
		NATIVE_GET(hover);
		NATIVE_GET(click);
		//NATIVE_SET(idle);
		//NATIVE_SET(hover);
		//NATIVE_SET(click);
	};
		
	/////////////////////////////////////////////////////////////////////		Live2DSprite
#if USE_LIVE2D
	class Live2DSprite : public Sprite
	{
		NATIVE_SUPPERINIT(Sprite);
	public:
		Live2DSprite(BKE_ObjectID i = -1) :Sprite(i){}
		
		NATIVE_INIT();

		NATIVE_CREATENEW()
		{
			if (!paramarray)
				return new Live2DSprite(-1);
			return new Live2DSprite(PARAM(0));
		}

		//int startMotion(name, no=0, priority="normal")
		//返回值0成功，-1表示priority被占用
		NATIVE_FUNC(startMotion);
		//int startRandomMotion(name, priority="normal")
		//返回值0成功，-1表示priority被占用，-2表示motion组为空
		NATIVE_FUNC(startRandomMotion);
		//int startMotionFile(name, sound="", relative=true, priority="normal", lipsync=false, fadein=1000, fadeout=1000)
		//返回值0成功，-1表示priority被占用，-2文件未找到
		NATIVE_FUNC(startMotionFile);

		NATIVE_FUNC(stopMotion);

		//int getMotionNum(name)
		NATIVE_FUNC(getMotionNum);

		//int setExpression(name)
		//返回值0成功，-1名称未找到
		NATIVE_FUNC(setExpression);
		//void setRandomExpression()
		NATIVE_FUNC(setRandomExpression);
		//int setExpression(name, relative=true);
		//返回值0成功，-1名称未找到
		NATIVE_FUNC(setExpressionFile);
		
		NATIVE_SET(onFlick);

		NATIVE_GET(onFlick);

		NATIVE_SET(onTap);

		NATIVE_GET(onTap);
	};
#endif

	/////////////////////////////////////////////////////////////////////		Event

	class Event : public BKE_NativeClass
	{
	public:

		Event() :BKE_NativeClass(L"Event"){}
		Event(const wstring &name) :BKE_NativeClass(L"Event")/*, event(name)*/{}
		Event(const BKE_Event &e) :BKE_NativeClass(L"Event"), event(e){}

		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_SAVE();

		NATIVE_LOAD();

		NATIVE_SET(name);
		NATIVE_SET(enable);
		NATIVE_SET(label);
		NATIVE_SET(file);
		NATIVE_SET(type);
		NATIVE_SET(ignore);
		NATIVE_SET(exp);
		NATIVE_SET(param);
		NATIVE_SET(func);

		NATIVE_GET(name);
		NATIVE_GET(enable);
		NATIVE_GET(label);
		NATIVE_GET(file);
		NATIVE_GET(type);
		NATIVE_GET(ignore);
		NATIVE_GET(exp);
		NATIVE_GET(param);
		NATIVE_GET(func);

		NATIVE_FUNC(call);
		NATIVE_FUNC(toVar);

		const BKE_Event &getEvent() const{ return event; }
	protected:
		BKE_Event event;
	};


	class EventCenter : public BKE_NativeClass
	{
		//static BKE_EventCenter *ec;
	public:
		EventCenter() :BKE_NativeClass(L"EventCenter"){}

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }

		//register(name, event)
		NATIVE_FUNC(register);
		//delete(name)
		NATIVE_FUNC(delete);
		//callsysevent(name, param);
		NATIVE_FUNC(systemCall);
		//call(name, param);
		NATIVE_FUNC(call);
		//get(name)
		NATIVE_FUNC(get);
	};

	/////////////////////////////////////////////////////////////////////       Archive
// 	class Archive : public BKE_NativeClass
// 	{
// 	public:
// 		Archive
// 	};


	/////////////////////////////////////////////////////////////////////		Json
	class Json :public BKE_NativeClass
	{
	public:

		Json() :BKE_NativeClass(L"Json"){}

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }

		NATIVE_FUNC(toJson);
		NATIVE_FUNC(fromJson);
	};

	class CSVParserNI/* : public ::CSVParser*/
	{
		BKE_VarFunction *_doLine;
	public:
		virtual void doLine(BKE_VarArray *columns, bkpulong lineNo)
		{
			if (_doLine)
			{
				columns->addRef();
				BKE_VarArray *arr = new BKE_VarArray{ columns, lineNo };
				_doLine->run(arr);
				arr->release();
			}
		}
		CSVParserNI() :_doLine(NULL){}
		void setDoLineFunction(BKE_VarFunction *func){ if (_doLine) _doLine->release(); _doLine = func; }
		virtual ~CSVParserNI(){ if (_doLine) _doLine->release(); }
	};

	class CSVParser : public BKE_NativeClass
	{
		CSVParserNI parser;
	public:
		CSVParser() : BKE_NativeClass(L"CSVParser"){}

		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_FUNC(doFile);

		NATIVE_FUNC(doDocument);

		NATIVE_FUNC(doLine);

		NATIVE_FUNC(fromDocument);

		NATIVE_FUNC(fromFile);
	};

	/////////////////////////////////////////////////////////////////////		DownloadManager
	class DownloadManager : public BKE_NativeClass/*, public BKE_DownloadManagerDelegateProtocol*/
	{
	public:
		DownloadManager() :BKE_NativeClass(L"DownloadManager"){/* mgr = NULL;*/ }
		DownloadManager(const wstring &url, const wstring &file) :BKE_NativeClass(L"DownloadManager"){/* mgr = new BKE_DownloadManager(url, file); mgr->setDelegate(this); */}
		virtual ~DownloadManager(){ /*if(mgr) delete mgr;*/ }
		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_SAVE();

		NATIVE_LOAD();

		//BKE_DownloadManager *mgr;
		BKE_Variable _onSuccess;
		BKE_Variable _onError;
		BKE_Variable _onProgress;

		NATIVE_FUNC(begin);
		NATIVE_FUNC(stop);

		NATIVE_GET(packageUrl);
		NATIVE_SET(packageUrl);
		NATIVE_GET(outFileName);
		NATIVE_SET(outFileName);
		NATIVE_SET(onSuccess);
		NATIVE_SET(onError); // error_string
		NATIVE_SET(onProgress); // now,total

	private:
		//virtual void onError(/*BKE_DownloadManager::ErrorCode errorCode*/);
		//virtual void onProgress(/*bklong nowDownloaded, bklong totalToDownload*/);
		//virtual void onSuccess();
	};

	//class for Timer
	class Timer;

	struct TempTimer
	{
		BKE_VarFunction *func;
		BKE_VarArray *paramarr;
		float delay;
		float rawdelay;

		~TempTimer(){ paramarr->release(); func->release(); }
	};

	class TimerHelper /*: public CCLayer*/
	{
	private:
		TimerHelper()
		{
			//m_bRunning = true;
			//scheduleUpdate();
		}

	public:
		unordered_set<Timer *> timerset;
		unordered_set<Timer *> todelete;
		unordered_map<BKE_VarFunction *, TempTimer> timerset2;
		unordered_set<BKE_VarFunction *> todelete2;

		//static TimerHelper *helper;

		static void createSharedObject(){ /*if (!helper)helper = new TimerHelper();*/ }

		static void deleteSharedObject(){ /*if (helper){ helper->cleanup(); helper->release(); helper = NULL; }*/ }

		static TimerHelper *getSharedObject(){ return NULL/*helper*/; }

		void addTimer(Timer *t){ timerset.insert(t); }

		void removeTimer(Timer *t)
		{
			todelete.insert(t);
		}

		void addTempTimer(BKE_VarFunction *func, BKE_VarArray *paramarr, float delay)
		{ 
			todelete2.erase(func);
			timerset2[func] = {func, paramarr, delay, delay}; 
		}

		void removeTempTimer(BKE_VarFunction *func)
		{
			todelete2.insert(func);
		}

		void update(float dt);
	};

	/////////////////////////////////////////////////////////////////////		Timer
	class Timer :public BKE_NativeClass
	{
	public:
		BKE_Variable func;
		BKE_VarArray *paramarr;
		float interval;	//milisec
		bool enabled;
		float nexttime;	//下次还剩多久

		//作为类对象本身
		Timer() :BKE_NativeClass(L"Timer")
		{
			TimerHelper::createSharedObject();
			paramarr = NULL;
		}

		virtual ~Timer()
		{
			if (paramarr)
				paramarr->release();
			if(TimerHelper::getSharedObject())
				TimerHelper::getSharedObject()->removeTimer(this);
		}

		Timer(const BKE_Variable &func, float interval, BKE_VarArray *arr) :BKE_NativeClass(L"Timer")
		{
			this->func = func;
			this->paramarr = arr;
			this->enabled = false;
			this->interval = this->nexttime = interval;
			if (TimerHelper::getSharedObject())
				TimerHelper::getSharedObject()->addTimer(this);
		}

		NATIVE_INIT();

		NATIVE_CREATENEW()
		{
			PARAMEXIST(1);
			if (PARAM(0).getType() != VAR_FUNC)
				throw Var_Except(L"参数1必须为函数");
			auto arr = new BKE_VarArray();
			arr->setLength(paramarray->vararray.size() - 1);
			int i = 0;
			while (++i < paramarray->vararray.size())
				arr->vararray[i - 1] = paramarray->vararray[i];
			return new Timer(PARAM(0), (float)PARAM(1), arr);
		}

		NATIVE_SAVE()
		{
			return BKE_Variable::arrayWithObjects(func, enabled, interval);
		}

		NATIVE_LOAD()
		{
			this->func = var[0];
			this->enabled = var[1];
			this->interval = var[2];
			if (TimerHelper::getSharedObject())
				TimerHelper::getSharedObject()->addTimer(this);
		}

		NATIVE_GET(enable);
		NATIVE_SET(enable);
		NATIVE_GET(interval);
		NATIVE_SET(interval);

		NATIVE_FUNC(setTimeout);
		NATIVE_FUNC(clearTimeout);
	};
	
	/////////////////////////////////////////////////////////////////////		Network
	class Network :public BKE_NativeClass/*, public CCObject*/
	{
	public:
		Network():BKE_NativeClass(L"Network"){}
		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }
			
		NATIVE_FUNC(download);
		NATIVE_FUNC(getString);	
		NATIVE_FUNC(request);
		NATIVE_FUNC(option);
	};

	/////////////////////////////////////////////////////////////////////		Debug
	class Debug :public BKE_NativeClass
	{
	public:
		Debug() :BKE_NativeClass(L"Debug"){}
		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }
		
		NATIVE_FUNC(printCallStack);
		NATIVE_FUNC(memoryStatus);
	};
};