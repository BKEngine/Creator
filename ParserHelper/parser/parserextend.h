#pragma once

#if BKE_CREATOR
#include "extend.h"
#else
#include "parser/extend.h"
#include "System.h"
#include "FileSL.h"
#include "String.h"
#include "Script/History.h"
#include "Network/DownloadManager.h"
#include "Event/Event.h"
#include "ConfigExtension/CsvSupport.h"
#include "BTL/Selector.h"
#endif
#include <set>
#include <map>

BKE_Variable BKE_EvalFile(const wstring &filename, BKE_VarClosure *clo = BKE_VarClosure::global());

extern void registerExtendFunction();

class BKE_Script;
class BKE_Mutex;
class BKE_Audio;
class BKE_Graphic;

typedef int BKE_ObjectID;

namespace ParserUtils
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
	*	@param bool append=false 是否追加在文件末尾。若为否，则覆盖整个文件。
	*	@brief 将字符串数组按照换行合并，并保存到文件
	*	@example 以下将演示一个从内容为["a","b","c"]的数组写入文件的示例：
	*	@example_code [].save("1.txt")
	*/
	NATIVE_FUNC(save);

	/**
	*	@class array|dictionary
	*	@param string filename 文件名
	*	@param string mode=void 模式。若首字母为'o'则字符串的其余部分代表一个偏移，文件将从该偏移处进行覆盖保存，负值将从文件末尾进行计算。若首字母为'z'则与'o'类似，只不过将启用压缩。
	*	@brief 将本结构（数组或者字典）进行序列化，并保存到文件。
	*	@body 文件的内容可以直接使用{@link evalFile}进行反序列化，读取至变量。
	*	@example 以下将演示一个从内容为%[key:1] (%["abc"=>1]) 的字典写入文件的示例：
	*	@example_code a.saveStruct("1.txt")
	*/
	NATIVE_FUNC(saveStruct);

	/**
	*	@param string filename 文件名
	*	@return string
	*	@brief 读取一个文件，返回里面的原始内容（字符串）
	*	@example 以下将演示一个从内容为
	*		abc
	*		的1.txt文件载入为数组的示例：
	*	@example_code [].loadFile("1.txt")
	*	@example_result "abc"
	*/
	NATIVE_FUNC(loadFile);

	/**
	*	@param string filename 文件名
	*	@param string str 将要保存的字符串
	*	@param bool append=false 是否追加在文件末尾。若为否，则覆盖整个文件。
	*	@return bool 是否成功
	*	@brief 将一个字符串写入文件。
	*	@example
	*	@example_code
	*	@example_result
	*/
	NATIVE_FUNC(saveFile);


	/**
	*	@class
	*	@param string filename 文件名
	*	@param string str 将要保存的字符串
	*	@return bool 是否成功
	*	@brief 将一个字符串添加到文件末尾。等同于{@link saveFile}(filename, str, -1);
	*	@example
	*	@example_code
	*	@example_result
	*/
	NATIVE_FUNC(appendFile);

	/**
	*	@class
	*	@param string filename 文件名
	*	@param bool krmode=false 若为真，表示双引号字符串可以像单引号字符串一样使用转义（此时不使用原有的""表示"的功能）
	*	@param bool rawstr=false 若为真，表示字符串可以跨行
	*	@return var 反序列化的结果
	*	@brief 对文件读取并且解析（反序列化），将结果返回。
	*	@body 等同于{@link eval}({@link loadFile}(filename, krmode, rawstr));
	*	@example
	*	@example_code
	*	@example_result
	*/
	NATIVE_FUNC(evalFile);

	/**
	*	@class
	*	@param filename 文件名
	*	@return bool
	*	@brief 判断文件是否存在且可以被读取（即文件是否存在于文件系统或者封包中）
	*	@example
	*	@example_code
	*	@example_result
	*/
	NATIVE_FUNC(fileExist);

	/**
	*	@class
	*	@function log
	*	@param var
	*	@return
	*	@brief 序列化一个变量并且将结果打印到控制台。
	*	@example
	*	@example_code
	*	@example_result
	*/
	NATIVE_FUNC(log_ol);

	/**
	*	@brief 系统类。
	*	@sington
	*	@body 记录着一些系统相关的属性和类，用以获取预先定义的设定和控制引擎底层一些实现。
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
		*	@brief 主程序的文件名（不含路径）
		*  @type string
		*/
		NATIVE_GET(appName);

		/**
		*  @brief 主程序的路径
		*  @type string
		*/
		NATIVE_GET(appPath);

		/**
		*  @brief 主程序的全路径（包含文件名）
		*  @type string
		*/
		NATIVE_GET(appFullName);

		/**
		*  @brief 当前是否正在显示文字
		*  @type bool
		*/
		NATIVE_GET(showingText);

		/**
		*  @brief 当前正在执行trans的层的数量
		*  @type int
		*/
		NATIVE_GET(transCount);

		/**
		*  @brief 当前正在执行action的层的数量
		*  @type int
		*/
		NATIVE_GET(actionCount);

		/**
		*  @brief 游戏名称
		*  @type string
		*/
		NATIVE_GET(gameName);
		
		/**
		*  @brief 游戏标题
		*  @type string
		*/
		NATIVE_GET(gameTitle);
		NATIVE_SET(gameTitle);

		/**
		*	@brief 是否将历史记录保存进存档
		*	@type bool
		*/
		NATIVE_GET(saveHistory);
		NATIVE_SET(saveHistory);

		/**
		*	@brief 是否开启自动模式
		*	@type bool
		*/
		NATIVE_GET(autoMode);
		NATIVE_SET(autoMode);

		/**
		*	@brief 自动模式时间，单位ms
		*	@type int
		*/
		NATIVE_GET(autoModeTime);
		NATIVE_SET(autoModeTime);

		/**
		*	@brief 是否开启跳过模式
		*	@type bool
		*/
		NATIVE_GET(skipMode);
		NATIVE_SET(skipMode);

		/**
		*	@brief 是否允许跳过未读文本
		*	@type bool
		*/
		NATIVE_GET(skipAll);
		NATIVE_SET(skipAll);

		/**
		*	@brief 最高帧速
		*	@body {@b 除非你知道你在做什么，否则请不要动这个变量。}
		*	@type int
		*/
		NATIVE_GET(maxFrameSpeed);
		NATIVE_SET(maxFrameSpeed);

		/**
		*	@brief 文字速度
		*	@body 值从0到100，代表每显示一个文字耗时100ms到0ms（瞬间显示）。
		*	@type int
		*/
		NATIVE_GET(textSpeed);
		NATIVE_SET(textSpeed);

		/**
		*	@brief 是否允许渐变特效
		*	@body 若为否，则所有trans特效皆适用trans的normal模式。
		*	@type bool
		*/
		NATIVE_SET(transitionEnabled);
		NATIVE_GET(transitionEnabled);

		/**
		*	@brief {@b 当前消息层}的文字大小
		*	@type int
		*/
		NATIVE_SET(fontSize);
		NATIVE_GET(fontSize);

		/**
		*	@brief {@b 当前消息层}的文字颜色
		*	@type int
		*/
		NATIVE_SET(fontColor);
		NATIVE_GET(fontColor);

		/**
		*	@brief {@b 当前消息层}的文字样式
		*	@type int
		*/
		NATIVE_SET(fontStyle);
		NATIVE_GET(fontStyle);

		/**
		*	@brief 是否全屏显示
		*	@body {@b 在移动端此参数无效。}
		*	@type bool
		*/
		NATIVE_GET(fullScreen);
		NATIVE_SET(fullScreen);

		/**
		*	@brief 是否开启FPS显示
		*	@type bool
		*/
		NATIVE_GET(FPSEnabled);
		NATIVE_SET(FPSEnabled);

		/**
		*	@brief 文字全局缩放比例
		*	@body 用于外部调整文字大小。默认1.0
		*	@type number
		*/
		NATIVE_GET(fontSizeFactor);
		NATIVE_SET(fontSizeFactor);
		
		/**
		*	@brief 是否显示触摸点
		*	@type bool
		*/
		NATIVE_GET(touchPointEnabled);
		NATIVE_SET(touchPointEnabled);

		/**
		*	@brief 窗口大小
		*	@body {@b 注意，窗口大小并不等于游戏分辨率大小。你不应该将此变量用于游戏界面相关的计算中。}
		*	@type vec2
		*/
		NATIVE_GET(windowSize);
		NATIVE_SET(windowSize);

		/**
		*	@brief 鼠标自动消失的时间
		*	@body {@b 仅启用自定义鼠标指针时有效。}
		*	单位为ms，默认为2000
		*	@type int
		*/
		NATIVE_GET(mouseCursorAutoHideTime);
		NATIVE_SET(mouseCursorAutoHideTime);

		/**
		*	@brief 鼠标滚轮的旋转量
		*	@body 正数是向上。
		*	@type int
		*/
		NATIVE_GET(mouseWheel);

		/**
		*	@brief 鼠标的状态
		*	@body 类型为数组，长度固定为5。5个数组成员分别为鼠标的X坐标，Y坐标（整数），左、中、右三个键的按下状态（bool）
		*	@type array
		*/
		NATIVE_GET(mouseStatus);

		/**
		*	@brief 游戏分辨率
		*	@body 可以用于UI相关的计算，长度固定为2，2个数组成员分别为宽和高。
		*	@type array
		*/
		NATIVE_GET(resolutionSize);


		NATIVE_SET(displayedResolutionSize);
		NATIVE_GET(displayedResolutionSize);
		/**
		*	@brief 脚本静止状态
		*	@body 当脚本中遇到l, p, click, waitbutton并等待的时候被标记为true。
		*	@type bool
		*/
		NATIVE_GET(stable);

		/**
		*	@brief 已读文本信息
		*	@body readinfo[脚本名][标签名]=该标签已读过的偏移量
		*	可以用来检测标签是否被经过
		*	@type dictionary
		*/
		//NATIVE_GET(readinfo);
		
		NATIVE_GET(screenSize);

		/**
		*	@brief BKEngine版本号
		*	@type int
		*/
		NATIVE_GET(version);

		/**
		*	@brief 当前运行的平台
		*	@body 值为"windows","macos","linux","ios","android","winphone"其中之一
		*	@type string
		*/
		NATIVE_GET(platform);
		
#if DEVELOP_VERSION
		NATIVE_GET(debugSkipMode);
		NATIVE_SET(debugSkipMode);
#endif

		NATIVE_FUNC(getKeyState);
		NATIVE_FUNC(getKeyboardState);


		NATIVE_FUNC(getEnv);
		NATIVE_FUNC(setEnv);

		/**
		 *	@brief 退出程序
		 */
		NATIVE_FUNC(exit);


		NATIVE_FUNC(run);

		/**
		*	@brief 打开一个网页
		*/
		NATIVE_FUNC(openUrl);
	};

	/**
	*	@brief 脚本类
	*	@sington
	*	@body {@b 不可实例化。}
	*/
	/////////////////////////////////////////////////////////////////////		Scripts
	class Scripts :public BKE_NativeClass
	{
	private:
		//friend void handleError(bkeLogLevel err);
		static BKE_Script *script;

	public:
		Scripts() :BKE_NativeClass(L"Scripts"){};

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }
		
		/**
		*	@param string text
		*	@param function callback=void 文字显示完毕时触发回调
		*	@brief 在文本框打印一段文字。等同于bkscr中text命令。
		*/
		NATIVE_FUNC(showText);

		/**
		*	@param string name 调用的bks指令名
		*	@param dictionary param 调用的参数列表
		*	@param function callback=void 函数真正执行完毕时触发的回调
		*	@brief 调用一个bkscr指令。
		*	@body 如调用wait*系列函数，则在wait的条件被触发、等待结束时callback回调才被触发。正常指令则立刻触发回调。
		*	text指令则等待文字打印完成才触发回调。
		*/
		NATIVE_FUNC(runCmd);

		
		/**
		*	@brief 让脚本从等待状态（等待按钮、等待点击等等）恢复，继续执行
		*/
		NATIVE_FUNC(go);

		/**
		*	@param string label 标签
		*	@param string file="" 文件
		*	@brief 跳转到某个脚本，等同于bkscr中jump指令
		*	@body 和{@link jump}指令等同
		*/
		NATIVE_FUNC(goto);

		/**
		*	@param string label 标签
		*	@param string file="" 文件
		*	@brief 跳转到某个脚本，等同于bkscr中jump指令
		*	@body 和{@link goto}指令等同
		*/
		NATIVE_FUNC(jump);

		/**
		*	@param string label 标签
		*	@param string file="" 文件
		*	@param function callback=void 从标签return时触发的回调
		*	@brief 访问某个标签，等同于bkscr中call指令
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

		NATIVE_SET(skipModeDisabled);
		NATIVE_GET(skipModeDisabled);

		NATIVE_SET(autoModeDisabled);
		NATIVE_GET(autoModeDisabled);
	};

	/////////////////////////////////////////////////////////////////////		SaveData

	/**
	* @brief 存储数据类
	* @sington
	* @body {@b 不能创建实例}
	*/
	class SaveData :public BKE_NativeClass
	{

	public:
		SaveData() :BKE_NativeClass(L"SaveData"){};

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }

		/**
		 *  @param int 存档编号
		 *  @return bool 是否存在
		 *  @brief 编号为n的存档是否存在
		 */
		NATIVE_FUNC(exist);

		/**
		 *  @param int 存档编号
		 *  @return Date & void
		 *  @brief 获取编号为n的存档时间，是一个Date类，存档不存在则返回void
		 */
		NATIVE_FUNC(getTime);

		/**
		 *  @param string 存档的额外字符串
		 *  @return string & void
		 *  @brief 获取存档时存入的额外字符串，若未保存额外字符串或存档不存在则返回空
		 */
		NATIVE_FUNC(getText);

		NATIVE_FUNC(remove);

		NATIVE_FUNC(stashSave);

		NATIVE_FUNC(stashApply);
	};

	/////////////////////////////////////////////////////////////////////		Channel
	
	/**
	*  @brief Channel类
	*  @body {@b 可创建实例}
	*/
	class Channel :public BKE_NativeClass
	{
	public:
		int channel_index;

		static BKE_Mutex *mu;

		static BKE_Audio *audio;

		Channel(bkplong i=0) :BKE_NativeClass(L"Channel"), channel_index(i){};

		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_CREATENULL(Channel);

		NATIVE_SAVE()
		{
			return channel_index;
		}

		NATIVE_LOAD()
		{
			channel_index = var;
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

		/**
		 *  @param string 文件的路径
		 *  @brief 加载路径为file的文件
		 */
		NATIVE_FUNC(load);

		/**
		 *  @param file，vol，looptimes
		 *  @brief 播放
		 */
		NATIVE_FUNC(play);

		/**
		 *  @brief 暂停播放
		 */
		NATIVE_FUNC(pause);

		/**
		 *  @brief 恢复播放
		 */
		NATIVE_FUNC(resume);

		/**
		 *  @param fadetime 毫秒时间
		 *  @brief 经过fadetime(毫秒)的淡出之后,停止播放
		 */
		NATIVE_FUNC(stop);

		/**
		 *  @param time 毫秒时间
		 *  @brief 跳转到音频第time毫秒
		 */
		NATIVE_FUNC(seek);

		/**
		 *  @brief 获得当前播放的时间进度(单位毫秒)
		 */
		NATIVE_FUNC(tell);
	};

	/////////////////////////////////////////////////////////////////////		History
	
	/**
	*  @brief History类
	*  @sington
	*  @body {@b 不能创建实例}
	*/
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

		/**
		 *  @param int
		 *  @return string 历史记录
		 *  @brief 返回第line行的历史记录
		 */
		NATIVE_FUNC(getDataAtLine);

		/**
		 *  @brief 换行
		 */
		NATIVE_FUNC(reline);

		/**
		 *  @brief 换页
		 */
		NATIVE_FUNC(repage);

		/**
		 *  @param string,bool
		 *  @brief 往历史记录里添加字符串text,reline决定之后是否要换行，默认为false
		 */
		NATIVE_FUNC(add);
		NATIVE_FUNC(clear);
	};

	/////////////////////////////////////////////////////////////////////		Sprite
	class Sprite :public BKE_NativeClass
	{
	public:
		static BKE_Graphic *graphic;

		BKE_ObjectID index;

		Sprite(BKE_ObjectID i = -1) :BKE_NativeClass(L"Sprite"){ index = i; };

		NATIVE_INIT();

		NATIVE_CREATENEW()
		{
			if (!paramarray)
				return new Sprite(-1);
			return new Sprite(PARAM(0)); 
		}

		NATIVE_CREATENULL(Sprite);

		NATIVE_SAVE()
		{
			return index;
		}

		NATIVE_LOAD()
		{
			index = var;
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
		NATIVE_GET(filename);
		NATIVE_GET(angle);
		NATIVE_GET(visible);

		NATIVE_SET(value);
		NATIVE_SET(x);
		NATIVE_SET(y);
		NATIVE_SET(width);
		NATIVE_SET(height);
		NATIVE_SET(opacity);
		NATIVE_SET(zorder);
		NATIVE_SET(rect);
		NATIVE_SET(angle);
		NATIVE_SET(visible);

		NATIVE_SET(disabled);
		NATIVE_GET(disabled);
		NATIVE_SET(disabledRecursive);

		NATIVE_SET(focusable);
		NATIVE_GET(focusable);

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

		NATIVE_SET(onRClick);
		NATIVE_GET(onRClick);

		NATIVE_SET(onMouseWheel);
		NATIVE_GET(onMouseWheel);

		/**
		 *  @return 父精灵
		 *  @brief 返回该精灵的父精灵
		 */
		NATIVE_FUNC(getParent);
		NATIVE_FUNC(removeFromParent);
		NATIVE_FUNC(removeChild);
		NATIVE_FUNC(hasChild);
		//saveImage(filename, hasChildren=false)	//保存为 png
		NATIVE_FUNC(saveImage);
		NATIVE_FUNC(getChildren);

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

		NATIVE_CREATENULL(MessageLayer);

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

		NATIVE_CREATENULL(TextSprite);

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

		NATIVE_CREATENULL(Button);

		NATIVE_GET(idle);
		NATIVE_GET(hover);
		NATIVE_GET(click);
		//NATIVE_SET(idle);
		//NATIVE_SET(hover);
		//NATIVE_SET(click);
	};
		
	/////////////////////////////////////////////////////////////////////		Live2DSprite
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

		NATIVE_CREATENULL(Live2DSprite);

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

	/////////////////////////////////////////////////////////////////////		Spine
	class Spine : public Sprite
	{
		NATIVE_SUPPERINIT(Sprite);
	public:
		Spine(BKE_ObjectID i = -1) :Sprite(i){}

		NATIVE_INIT();

		NATIVE_CREATENEW()
		{
			if (!paramarray)
				return new Spine(-1);
			return new Spine(PARAM(0));
		}

		NATIVE_CREATENULL(Spine);

		NATIVE_GET(boundingBox);

		//void setAnimation(uint idx, string name, loop=true, forcereset=false)
		//idx是动画编号，数组的下标，所以不要传奇怪的值→_→，以下同
		NATIVE_FUNC(setAnimation);

		//void addAnimation(uint idx, string name, loop=true, delay=0)
		NATIVE_FUNC(addAnimation);

		//void mixAnimation(string fromAnimation, string toAnimation, float duration = 0)
		NATIVE_FUNC(mixAnimation);

		//void clearTrack(uint idx = 0)
		NATIVE_FUNC(clearTrack);

		//void clearAllTrack()
		NATIVE_FUNC(clearAllTrack);

		//void setStartListener(uint idx = 0, func = void)
		//func(int idx)
		//func可以为空,为空时表示取消listener，下同
		NATIVE_FUNC(setStartListener);

		//void setEndListener(uint idx = 0, func = void)
		//func(int idx)
		NATIVE_FUNC(setEndListener);

		//void setCompleteListener(uint idx = 0, func = void)
		//func(int idx, int loopCount)
		NATIVE_FUNC(setCompleteListener);

		//void setEventListener(uint idx = 0, func = void)
		//func(int idx, string evname, evvalue)
		//NATIVE_FUNC(setEventListener);
		
		NATIVE_GET(timeScale);
		NATIVE_SET(timeScale);
	};

	/////////////////////////////////////////////////////////////////////		Event

	class Event : public BKE_NativeClass
	{
	public:

		Event() :BKE_NativeClass(L"Event"){}
		//Event(const wstring &name) :BKE_NativeClass(L"Event"), event(name){}
		//Event(const BKE_Event &e) :BKE_NativeClass(L"Event"), event(e){}

		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_CREATENULL(Event);

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

		//const BKE_Event &getEvent() const{ return event; }
	protected:
		//BKE_Event event;
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
		NATIVE_FUNC(sendSystem);
		//call(name, param);
		NATIVE_FUNC(send);
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

	/**
	*  @brief json类
	*  @sington
	*  @body {@b 不可创建实例}
	*/
	class Json :public BKE_NativeClass
	{
	public:

		Json() :BKE_NativeClass(L"Json"){}

		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }

		/**
		 *  @param var
		 *  @return string Json格式化字符串
		 *  @brief 讲一个parser变量转换成Json格式化字符串，返回字符串
		 */
		NATIVE_FUNC(toJson);

		/**
		 *  @param string Json格式化字符串
		 *  @return var
		 *  @brief 讲一个Json格式化字符串转换成parser变量，返回变量
		 */
		NATIVE_FUNC(fromJson);
	};

	//class CSVParserNI : public ::CSVParser
	//{
	//	BKE_VarFunction *_doLine;
	//public:
	//	virtual void doLine(BKE_VarArray *columns, bkpulong lineNo)
	//	{
	//		if (_doLine)
	//		{
	//			columns->addRef();
	//			BKE_VarArray *arr = new BKE_VarArray{ columns, lineNo };
	//			_doLine->run(arr);
	//			arr->release();
	//		}
	//	}
	//	CSVParserNI() :_doLine(NULL){}
	//	void setDoLineFunction(BKE_VarFunction *func){ if (_doLine) _doLine->release(); _doLine = func; }
	//	virtual ~CSVParserNI(){ if (_doLine) _doLine->release(); }
	//};

	class CSVParser : public BKE_NativeClass
	{
		//CSVParserNI parser;
	public:
		CSVParser() : BKE_NativeClass(L"CSVParser"){}

		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_CREATENULL(CSVParser);

		NATIVE_FUNC(doFile);

		NATIVE_FUNC(doDocument);

		NATIVE_FUNC(doLine);

		NATIVE_FUNC(fromDocument);

		NATIVE_FUNC(fromFile);
	};

	/////////////////////////////////////////////////////////////////////		DownloadManager
	/**
	*  @brief DownloadManager类
	*  @body {@b 必须创建实例才能使用}
	*  此类支持断网重连和断点续传
	*/
	class DownloadManager : public BKE_NativeClass/*, public BKE_DownloadManagerDelegate*/
	{
	public:
		DownloadManager() :BKE_NativeClass(L"DownloadManager"){ /*mgr = NULL;*/ }
		//DownloadManager(const wstring &url, const wstring &file) :BKE_NativeClass(L"DownloadManager"){ mgr = new BKE_DownloadManager(this, url, file); }
		//virtual ~DownloadManager(){ if(mgr) delete mgr; }
		NATIVE_INIT();

		NATIVE_CREATENEW();

		NATIVE_CREATENULL(DownloadManager);
		
		NATIVE_SAVE();

		NATIVE_LOAD();

		//BKE_DownloadManager *mgr;
		BKE_Variable _onSuccess;
		BKE_Variable _onError;
		BKE_Variable _onProgress;

		/**
		 *  @brief 开始下载
		 */
		NATIVE_FUNC(begin);

		/**
		 *  @brief 停止下载
		 */
		NATIVE_FUNC(stop);

		/**
		 *  @return string URL
		 *  @brief 获取将要下载文件的URL
		 */
		NATIVE_GET(packageUrl);

		/**
		 *  @brief 设置要下载文件的URL
		 */
		NATIVE_SET(packageUrl);

		/**
		 *  @return string
		 *  @brief 获取输出文件的文件名
		 */
		NATIVE_GET(outFileName);

		/**
		 *  @brief 设置输出文件的文件名
		 */
		NATIVE_SET(outFileName);

		/**
		 *  @brief 下载成功时所调用毁掉函数
		 */
		NATIVE_SET(onSuccess);

		/**
		 *  @brief 下载出错时所调用的回调函数
		 */
		NATIVE_SET(onError); // error_string

		/**
		 *  @brief 下载正在进行时所调用的回调函数
		 */
		NATIVE_SET(onProgress); // now,total

	private:
		//virtual void onError(BKE_DownloadManager::ErrorCode errorCode);
		//virtual void onProgress(bklong nowDownloaded, bklong totalToDownload);
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

		TempTimer(BKE_VarFunction *func, BKE_VarArray *paramarr, float delay)
			:func(func), paramarr(paramarr), delay(delay), rawdelay(delay) {}
		TempTimer(TempTimer &&r)
			:func(r.func), paramarr(r.paramarr), delay(r.delay), rawdelay(r.rawdelay) {
			r.func = NULL; r.paramarr = NULL;
		}
		TempTimer(const TempTimer &) = delete;
		~TempTimer(){ if(paramarr) paramarr->release(); if (func) func->release(); }
	};

	//class TimerHelper 
	//	: public CCObject
	//	, public Singleton<TimerHelper>
	//{
	//private:
	//	friend class Singleton<TimerHelper>;
	//	bool m_bRunning;
	//	TimerHelper()
	//		: m_bRunning(false)
	//	{
	//	}

	//	~TimerHelper()
	//	{
	//		if (m_bRunning)
	//		{
	//			removeFromScheduler();
	//		}
	//	}

	//	inline void addToScheduler()
	//	{
	//		CCDirector::sharedDirector()->getHighResolutionScheduler()->scheduleSelector(BKE_SELECTOR(TimerHelper::update), this, 0, false);
	//	}

	//	inline void removeFromScheduler()
	//	{
	//		CCDirector::sharedDirector()->getHighResolutionScheduler()->unscheduleAllForTarget(this);
	//	}
	//	
	//public:
	//	set<Timer *> timerset;
	//	set<Timer *> todelete;
	//	map<BKE_VarFunction *, TempTimer> timerset2;
	//	set<BKE_VarFunction *> todelete2;

	//	void addTimer(Timer *t)
	//	{
	//		timerset.insert(t); 
	//		if (!m_bRunning)
	//		{
	//			m_bRunning = true;
	//			addToScheduler();
	//		}
	//	}

	//	void removeTimer(Timer *t)
	//	{
	//		todelete.insert(t);
	//	}

	//	void addTempTimer(BKE_VarFunction *func, BKE_VarArray *paramarr, float delay)
	//	{ 
	//		todelete2.erase(func);
	//		timerset2.emplace(func, TempTimer{ func, paramarr, delay });
	//		if (!m_bRunning)
	//		{
	//			m_bRunning = true;
	//			addToScheduler();
	//		}
	//	}

	//	void removeTempTimer(BKE_VarFunction *func)
	//	{
	//		todelete2.insert(func);
	//	}

	//	void update(float dt);
	//};

	/////////////////////////////////////////////////////////////////////		Timer
	/**
	*  @brief 时间类
	*  @body {@b 可创建实例}
	*/
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
			//if (!TimerHelper::valid())
			//	TimerHelper::create();
			paramarr = NULL;
		}

		virtual ~Timer()
		{
			if (paramarr)
				paramarr->release();
			//if(TimerHelper::valid())
			//	TimerHelper::getInstance()->removeTimer(this);
		}

		Timer(const BKE_Variable &func, float interval, BKE_VarArray *arr) :BKE_NativeClass(L"Timer")
		{
			this->func = func;
			this->paramarr = arr;
			this->enabled = false;
			this->interval = this->nexttime = interval;
			//TimerHelper::getInstance()->addTimer(this);
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

		NATIVE_CREATENULL(Timer);

		NATIVE_SAVE()
		{
			return BKE_Variable::arrayWithObjects(func, enabled, interval);
		}

		NATIVE_LOAD()
		{
			this->func = var[0];
			this->enabled = var[1];
			this->interval = var[2];
			//TimerHelper::getInstance()->addTimer(this);
		}

		/**
		 *  @return bool
		 *  @brief 读取enable是否在计数
		 */
		NATIVE_GET(enable);

		/**
		 *  @brief 设置enable的状态
		 */
		NATIVE_SET(enable);

		/**
		 *  @return 数型
		 *  @brief 获取timer计数的间隔
		 */
		NATIVE_GET(interval);

		/**
		 *  @brief 设置timer计数的间隔
		 */
		NATIVE_SET(interval);

		/**
		 *  @static
		 *  @param func,delay,*args
		 *  @brief 延迟delay毫秒后执行func，传入的第一个参数是实际的延迟时间，之后将args依次传入。执行一次后自动删除.
		 */
		NATIVE_FUNC(setTimeout);

		/**
		 *  @param func
		 *  @brief 删除用setTimeout注册的函数。
		 */
		NATIVE_FUNC(clearTimeout);

		/**
		*  @param func
		*  @brief 强制触发一次，并更新下次触发的时间。
		*/
		NATIVE_FUNC(forceTrigger);
	};
	
	/////////////////////////////////////////////////////////////////////		Network
	/**
	*  @brief Network类
	*  @sington
	*  @body {@b 不可创建实例}
	*/
	class Network :public BKE_NativeClass//, public CCObject
	{
	public:
		Network():BKE_NativeClass(L"Network"){}
		NATIVE_INIT();

		NATIVE_CREATENEW(){ return NULL; }

		/**
		 *  @param packageURL,outFileName
		 *  @return DownloadManger
		 *  @brief 创建并返回一个packageUEL=URL,outFileName-outFileName的DownloadManger实例
		 */
		NATIVE_FUNC(download);

		/**
		 *  @param URL
		 *  @return string 网络连接失败会返回字符串
		 *  @brief 从URL处获取该文件的字符串,如果网络连接失败则会返回空字符.在返回字符串之前该线程的脚本会阻塞在此命令
		 */
		NATIVE_FUNC(getString);	
		/**
		 *	@param dic
		 *	@
		 */
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

		/**
		 *  @brief 打印当前脚本调用栈
		 */
		NATIVE_FUNC(printCallStack);

		/**
		 *  @brief 打印当前内存使用状态
		 */
		NATIVE_FUNC(memoryStatus);
	};
};