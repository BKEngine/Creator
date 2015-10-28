#include <weh.h>
#include "cmdlist_wrapper.h"

#define REG_CMD(a, b) do{ auto &_v = CmdList[#a]; _v.name=#a; _v.detail = b;
//#define REG_CMD(a) do{ auto &_v = CmdList[#a]; _v.name=#a; _v.detail = "说明暂缺";
#define REG_CMD_VERSION(a, v, b) REG_CMD(a, b);
#define REG_END() }while(0)
#define ADD_PARAM(b, c, ...) _v.argNames.push_back(#b);_v.argFlags.push_back(c);_v.argAutoList.push_back("");
#define ADD_LIST(a) _v.argAutoList.back()=a;
#define PROPERTY(a, b, c) 
#define REG_SPE_CMD(a, b, c, d) { auto &_p = SpecialCmdList[#a].modes[#b]; _p.first = c; auto &_v = _p.second; _v.name=#b;_v.detail = d; /*_v.argNames.push_back("mode"); _v.argFlags.push_back(ptString);*/
//#define REG_SPE_CMD(a, b, c) { auto &_p = SpecialCmdList[#a].modes[#b]; _p.first = c; auto &_v = _p.second; _v.name=#b; _v.argNames.push_back("mode"); _v.argFlags.push_back(ptString);

#define AVALIABLE_AFTER(a) DynamicVersionInfo::availableAfter(a)
#define AVALIABLE_BEFORE(a) DynamicVersionInfo::availableBefore(a)

QHash<QString, BKECmdInfo> CmdList;
QHash<QString, BKESpecialCmdInfo> SpecialCmdList;

bool cmd_inited = false;

std::unordered_map<std::wstring, int> allEnums=
{
	{ L"crossfade", TRANS_CROSSFADE },
	{ L"universal", TRANS_UNIVERSAL },
	{ L"scroll", TRANS_SCROLL },
	{ L"shutter", TRANS_SHUTTER },
	{ L"ripple", TRANS_RIPPLE },
	{ L"rotateswap", TRANS_ROTATESWAP },
	{ L"rotatevanish", TRANS_ROTATEVANISH },
	{ L"rotatezoom", TRANS_ROTATEZOOM },
	{ L"wave", TRANS_WAVE },
	{ L"turn", TRANS_TURN },
	{ L"mosaic", TRANS_MOSAIC },
	{ L"normal", TRANS_NORMAL },
	{ L"start", ACTION_START },
	{ L"parallel", ACTION_PARALLEL },
	{ L"queue", ACTION_QUEUE },
	{ L"moveby", ACTION_MOVEBY },
	{ L"moveto", ACTION_MOVETO },
	{ L"fadeto", ACTION_FADETO },
	{ L"scaleby", ACTION_SCALEBY },
	{ L"scaleto", ACTION_SCALETO },
	{ L"rotateby", ACTION_ROTATEBY },
	{ L"rotateto", ACTION_ROTATETO },
	{ L"tintby", ACTION_TINTBY },
	{ L"tintto", ACTION_TINTTO },
	{ L"jump", ACTION_JUMP },
	{ L"shake", ACTION_SHAKE },
	{ L"quake", ACTION_QUAKE },
	{ L"visible", ACTION_VISIBLE },
	{ L"remove", ACTION_REMOVE },
	{ L"flipx", ACTION_FLIPX },
	{ L"flipy", ACTION_FLIPY },
	{ L"scissorby", ACTION_SCISSORBY },
	{ L"scissorto", ACTION_SCISSORTO },
	{ L"delay", ACTION_DELAY },
	{ L"callback", ACTION_CALLBACK },
	{ L"shaky3d", ACTION_SHAKY3D },
	{ L"end", ACTION_END },
	{ L"start", ANIMATE_START },
	{ L"horizontal", ANIMATE_HORIZONTAL },
	{ L"vertical", ANIMATE_VERTICAL },
	{ L"multifiles", ANIMATE_MULTIFILES },
	{ L"cell", ANIMATE_CELL },
	{ L"stop", ANIMATE_STOP },
	{ L"none", ANIMATE_LOOP_NONE },
	{ L"forward", ANIMATE_LOOP_FORWARD },
	{ L"bouncing", ANIMATE_LOOP_BOUNCING },
	{ L"delete", EFFECT_DELETE },
	{ L"clear", EFFECT_CLEAR },
	{ L"scissor", EFFECT_SCISSOR },
	{ L"mono", EFFECT_MONO },
	{ L"gray", EFFECT_GRAY },
	{ L"mask", EFFECT_MASK },
	{ L"stencil", EFFECT_STENCIL },
	{ L"gamma", EFFECT_GAMMA },
	{ L"invert", EFFECT_INVERT },
	{ L"BC", EFFECT_BC },
	{ L"add", EFFECT_ADD },
	{ L"multiply", EFFECT_MULTIPLY },
	{ L"blur", EFFECT_BLUR },
	{ L"sharpen", EFFECT_SHARPEN },
	{ L"edge", EFFECT_EDGE },
};

void initCmd()
{
	// 	REG_CMD(bg, "");
	// 		ADD_PARAM(file, VT_STRING);
	// 	REG_END();

	//用户信息相关
	REG_CMD(check, "检查指定的章节是否已购买");
	ADD_PARAM(chapter, ptString | ptStringArray);
	ADD_PARAM(onsuccess, ptEvent | ptOptional);
	ADD_PARAM(onfailure, ptEvent | ptOptional);
	REG_END();

	REG_CMD(queryitem, "根据条件查询包含指定章节的商品");
	ADD_PARAM(chapter, ptString | ptStringArray);
	ADD_PARAM(all, ptBool);
	ADD_PARAM(onsuccess, ptEvent | ptOptional);
	ADD_PARAM(onfailure, ptEvent | ptOptional);
	REG_END();

	REG_CMD(buyitem, "购买商品");
	ADD_PARAM(item, ptString);
	ADD_PARAM(onsuccess, ptEvent | ptOptional);
	ADD_PARAM(onfailure, ptEvent | ptOptional);
	REG_END();

	REG_CMD(cursor, "设置鼠标指针\nfile和index参数最多存在一个，若两个都为空则将鼠标指针设回系统默认");
	ADD_PARAM(file, ptString | ptOptional);
	ADD_PARAM(index, ptInteger | ptOptional);
	PROPERTY(image, index, atmostOne);
	REG_END();

	REG_CMD(l2dsprite, "载入一个live2d模型（需要有live2d key）");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	REG_END();

	REG_CMD(sprite, "若index所指为一个精灵，则修改它的状态。否则将创建一个精灵。");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	ADD_PARAM(rect, ptRect | ptOptional);
	REG_END();

	REG_CMD(addto, "添加一个精灵（源精灵）到另一个层或精灵（目标精灵）上，这里的透明度将会传递");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(zorder, ptInteger | ptOptional);
	ADD_PARAM(pos, ptPos | ptOptional);
	ADD_PARAM(opacity, ptByte | ptOptional);
	ADD_PARAM(modal, ptBool | ptOptional, AVALIABLE_AFTER(2027));
	REG_END();

	REG_CMD(layer, "创建一个指定大小的颜色精灵，这里的透明度是这个精灵本身的透明度");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(width, ptInteger);
	ADD_PARAM(height, ptInteger);
	ADD_PARAM(color, ptInteger | ptOptional);
	ADD_PARAM(opacity, ptByte | ptOptional);
	REG_END();

	REG_CMD(click, "等待鼠标左键点击后继续");
	REG_END();

	REG_CMD(remove, "从一个精灵的parent上取下自己，该精灵变为不可见");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(delete, ptBool | ptOptional);
	REG_END();

	REG_CMD(removeall, "从一个精灵上取下所有子精灵，所有子精灵变为不可见");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(delete, ptBool | ptOptional);
	ADD_PARAM(recursive, ptBool | ptOptional);
	REG_END();

	REG_CMD(info, "获取一个图片的信息，获取一个字典保存至get字符串对应的变量。\n字典成员有filename、width、height");
	ADD_PARAM(file, ptString);
	ADD_PARAM(get, ptVariable);
	REG_END();

	REG_CMD(infoex, "获取一个精灵的信息，获取一个字典保存至get字符串对应的变量。\n字典成员有filename、width、height、type、pos、index、parent、children");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(get, ptVariable);
	REG_END();

	REG_CMD(bgm, "播放背景音乐");
	ADD_PARAM(file, ptString);
	ADD_PARAM(loop, ptBool | ptOptional);
	ADD_PARAM(vol, ptVol | ptOptional);
	ADD_PARAM(fadein, ptTime | ptOptional);
	ADD_PARAM(loopto, ptTime | ptOptional);
	REG_END();

	REG_CMD(se, "播放音效");
	ADD_PARAM(file, ptString);
	ADD_PARAM(channel, ptInteger);
	ADD_PARAM(loop, ptBool | ptOptional);
	ADD_PARAM(vol, ptVol | ptOptional);
	ADD_PARAM(fadein, ptTime | ptOptional);
	REG_END();

	REG_CMD(voice, "播放语音");
	ADD_PARAM(file, ptString);
	ADD_PARAM(vol, ptVol | ptOptional);
	REG_END();

	REG_CMD(stop, "停止播放");
	ADD_PARAM(channel, ptInteger | ptOptional);
	ADD_PARAM(fadeout, ptTime | ptOptional);
	REG_END();

	REG_CMD(volume, "获取或者更改音量（仅针对本次播放有效）。set和get参数至多且必须存在一个。");
	ADD_PARAM(channel, ptInteger);
	ADD_PARAM(set, ptVol | ptOptional);
	ADD_PARAM(get, ptVariable | ptOptional);
	PROPERTY(set, get, onlyOne);
	REG_END();

	REG_CMD(pause, "暂停播放");
	ADD_PARAM(channel, ptInteger);
	REG_END();

	REG_CMD(resume, "恢复播放");
	ADD_PARAM(channel, ptInteger);
	REG_END();

	//	REG_CMD(mute, "");
	//		ADD_PARAM(channel, VT_INTEGER);
	//	REG_END();
	// 	REG_CMD(defaultstyle, "");
	// 		ADD_PARAM(name, VT_STRING|VT_OPTIONAL);
	// 		ADD_PARAM(size, VT_INTEGER|VT_OPTIONAL);
	// 		ADD_PARAM(color, VT_INTEGER|VT_OPTIONAL);
	// 		ADD_PARAM(bold, VT_BOOL|VT_OPTIONAL);
	// 		ADD_PARAM(italic, VT_BOOL|VT_OPTIONAL);
	// 		ADD_PARAM(strike, VT_BOOL|VT_OPTIONAL);
	// 		ADD_PARAM(under, VT_BOOL|VT_OPTIONAL);
	// 	REG_CMD(resetstyle, "");
	REG_CMD(text, "显示一句话");
	ADD_PARAM(text, ptString);
	REG_END();

	REG_CMD(messagelayer, "若index所指的精灵就是一个消息层，则改变它的active状态，否则创建一个消息层。");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(active, ptBool | ptInteger);;
	REG_END();

	REG_CMD(textcursor, "设置文字等待光标");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(follow, ptBool);
	ADD_PARAM(pos, ptPos | ptOptional);
	REG_END();

	REG_CMD(textwindow, "设置文本框属性,若可选参数没有设定且没有特殊说明的，将默认使用当前的文本框设置。\n其中opacity初始化为255，pos初始化为[0,0]，xinterval初始化为0，yinterval初始化为12。file和color参数至多存在一个。");
	ADD_PARAM(file, ptString | ptOptional);
	ADD_PARAM(color, ptInteger | ptOptional);
	PROPERTY(file, color, atmostOne);
	ADD_PARAM(opacity, ptByte | ptOptional);
	ADD_PARAM(pos, ptPos | ptOptional);
	ADD_PARAM(rect, ptRect | ptOptional);
	ADD_PARAM(xinterval, ptInteger | ptOptional);
	ADD_PARAM(yinterval, ptInteger | ptOptional);
	REG_END();

	REG_CMD(textoff, "隐藏文本框");
	ADD_PARAM(time, ptTime | ptOptional, AVALIABLE_AFTER(2307));
	REG_END();

	REG_CMD(texton, "显示文本框");
	ADD_PARAM(time, ptTime | ptOptional, AVALIABLE_AFTER(2307));
	REG_END();

	REG_CMD(locate, "将下一个文字的输出位置改为x，y。");
	ADD_PARAM(x, ptInteger | ptOptional);
	ADD_PARAM(y, ptInteger | ptOptional);
	REG_END();

	//REG_CMD(textshadow, "");
	//	ADD_PARAM(shadow, VT_BOOL);
	//	ADD_PARAM(color, VT_INTEGER|VT_OPTIONAL);
	//REG_END();

	REG_CMD(textspeed, "设置文字显示速度，范围0-100，越大越快");
	ADD_PARAM(interval, ptInteger);
	REG_END();

	REG_CMD(textsprite, "创建一个文字精灵。若可选参数没有设定且没有特殊说明的，将默认使用当前的字体设置。\n设置都是临时的，只对该文字精灵有效。");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(text, ptString);
	ADD_PARAM(color, ptColor | ptColorArray | ptOptional);
	ADD_PARAM(size, ptInteger | ptOptional);
	ADD_PARAM(font, ptString | ptOptional);
	ADD_PARAM(width, ptInteger | ptOptional);
	ADD_PARAM(height, ptInteger | ptOptional);
	ADD_PARAM(xinterval, ptInteger | ptOptional);
	ADD_PARAM(yinterval, ptInteger | ptOptional);
	ADD_PARAM(extrachar, ptString | ptOptional);
	ADD_PARAM(bold, ptBool | ptOptional);
	ADD_PARAM(italic, ptBool | ptOptional);
	ADD_PARAM(strike, ptBool | ptOptional);
	ADD_PARAM(under, ptBool | ptOptional);
	ADD_PARAM(shadow, ptBool | ptOptional);
	ADD_PARAM(shadowcolor, ptColor | ptOptional);
	ADD_PARAM(stroke, ptBool | ptOptional);
	ADD_PARAM(strokecolor, ptColor | ptOptional);
	REG_END();

	REG_CMD(textstyle, "设置当前文字大小，颜色等属性。若可选参数没有设定且没有特殊说明的，将默认使用当前的文本风格设置。\n其中字体默认使用系统字体，size初始化为24，color初始化为0xFFFFFF，斜体等初始化为false，\nenableshadow和enablestroke初始化为false，shadow和stroke初始化为0x000000");
	ADD_PARAM(name, ptString | ptOptional);
	ADD_PARAM(size, ptInteger | ptOptional);
	ADD_PARAM(color, ptColor | ptColorArray | ptOptional);
	ADD_PARAM(bold, ptBool | ptOptional);
	ADD_PARAM(italic, ptBool | ptOptional);
	ADD_PARAM(strike, ptBool | ptOptional);
	ADD_PARAM(under, ptBool | ptOptional);
	ADD_PARAM(shadow, ptBool | ptOptional);
	ADD_PARAM(shadowcolor, ptColor | ptOptional);
	ADD_PARAM(stroke, ptBool | ptOptional);
	ADD_PARAM(strokecolor, ptColor | ptOptional);
	REG_END();

	REG_CMD(storefont, "暂存字体信息，如果多次storefont，只会保存最后一次的结果。\n用restorefont命令可以将字体设为最后一次storefont时的状态。");
	REG_END();

	REG_CMD(restorefont, "复原字体信息");
	REG_END();

	REG_CMD(texttag, "设定遇到文本【】时的处理，相当于call一个macro，参数作为字符串存在变量tag中。");
	ADD_PARAM(label, ptLabel);
	ADD_PARAM(file, ptScript | ptOptional);
	ADD_PARAM(autoclean, ptBool | ptOptional, AVALIABLE_AFTER(2327));
	REG_END();

	REG_CMD(history, "设置是否开启历史记录");
	ADD_PARAM(output, ptBool);
	REG_END();

	REG_CMD(i, "开关斜体");
	REG_END();

	REG_CMD(b, "开关粗体");
	REG_END();

	REG_CMD(s, "开关删除线");
	REG_END();

	REG_CMD(u, "开关下划线");
	REG_END();

	REG_CMD(r, "手动换行");
	REG_END();

	REG_CMD(l, "等待点击");
	REG_END();

	REG_CMD(p, "换页等待，点击后换页");
	REG_END();

	REG_CMD(eval, "执行一个表达式（赋值等等）");
	ADD_PARAM(exp, ptString);
	REG_END();

	REG_CMD(jump, "跳转到指定标签");
	ADD_PARAM(label, ptLabel);
	ADD_PARAM(file, ptScript | ptOptional);
	REG_END();

	REG_CMD(call, "跳入(call)指定标签，遇到return");
	ADD_PARAM(label, ptLabel);
	ADD_PARAM(file, ptScript | ptOptional);
	REG_END();

	REG_CMD(return, "返回，返回值被存到调用macro时return参数给的变量中");
	ADD_PARAM(value, ptEverything | ptOptional);
	REG_END();

	REG_CMD(returnto, "从上一个call中返回，然后jump到指定的block和file");
	ADD_PARAM(label, ptLabel);
	ADD_PARAM(file, ptScript | ptOptional);
	ADD_PARAM(value, ptEverything | ptOptional);
	REG_END();

	REG_CMD(if, "根据这个表达式判断执行下面的语句还是跳到else或elseif后面执行。");
	ADD_PARAM(exp, ptString | ptConst);
	REG_END();

	REG_CMD(else, "跟if配合使用");
	REG_END();

	REG_CMD(elseif, "根据这个表达式判断执行下面的语句还是跳到下一个else或elseif后面执行。");
	ADD_PARAM(exp, ptString | ptConst);
	REG_END();

	REG_CMD(endif, "标志着if块的结束");
	REG_END();

	REG_CMD(tablegoto, "计算表达式，根据返回的值跳转到标签数组对应的元素指示的标签，只能同一文件中跳转。");
	ADD_PARAM(exp, ptTime);
	ADD_PARAM(target, ptLabelArray);
	REG_END();

	REG_CMD(for, "var指定的变量遍历range数组的每一个值，每遍历一次执行一次for块。");
	ADD_PARAM(var, ptVariable);
	ADD_PARAM(range, ptArray);
	REG_END();

	REG_CMD(next, "for循环块的终止标记");
	REG_END();

	REG_CMD(continue, "终止本次for循环开始下次循环");
	REG_END();

	REG_CMD(break, "跳出for循环");
	REG_END();

	REG_CMD(wait, "等待一段时间");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitaction, "等待某个精灵的动作执行完毕，若可以跳过，点击鼠标时将同时停止action");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitallaction, "等待所有精灵的动作执行完毕，若可以跳过，点击鼠标时将同时停止所有精灵的action");
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitbutton, "等待按钮跳转事件触发");
	ADD_PARAM(time, ptTime | ptOptional);
	REG_END();

	REG_CMD(waitbgm, "等待bgm播放完毕，若可以跳过，点击鼠标时将同时停止bgm");
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitfade, "等待音频渐变完毕，若可以跳过，点击鼠标时将立刻完成音频渐变");
	ADD_PARAM(channel, ptInteger);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waittrans, "等待trans完毕，若可以跳过，点击鼠标时将同时停止trans");
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitalltrans, "等待所有精灵的trans完毕，若可以跳过，点击鼠标时将同时停止所有精灵的trans");
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitvoice, "等待语音播放完毕，若可以跳过，点击鼠标时将同时停止语音");
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitse, "等待音效播放完毕，若可以跳过，点击鼠标时将同时停止音效");
	ADD_PARAM(channel, ptInteger);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(stoptrans, "强制终止某个精灵的渐变");
	ADD_PARAM(index, ptInteger);
	REG_END();

	REG_CMD(stopaction, "强制终止某个精灵的动作");
	ADD_PARAM(index, ptInteger);
	REG_END();

	REG_CMD(cancelaction, "强制取消某个精灵的动作");
	ADD_PARAM(index, ptInteger);
	REG_END();

	REG_CMD(stopalltrans, "强制终止所有精灵的渐变");
	REG_END();

	REG_CMD(stopallaction, "强制终止所有精灵的动作");
	REG_END();

	REG_CMD(cancelallaction, "强制取消所有精灵的动作");
	REG_END();

	REG_CMD(pretrans, "指定一个精灵准备渐变，将冻结这个精灵的画面状态");
	ADD_PARAM(index, ptInteger | ptOptional);
	REG_END();

	REG_CMD(button, "创建一个按钮");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(idle, ptString);
	ADD_PARAM(hover, ptString | ptOptional);
	ADD_PARAM(click, ptString | ptOptional);
	ADD_PARAM(disable, ptString | ptOptional);
	ADD_PARAM(onenter, ptEvent | ptOptional);
	ADD_PARAM(onleave, ptEvent | ptOptional);
	ADD_PARAM(onclick, ptEvent | ptOptional);
	//20140903
	ADD_PARAM(enterse, ptString | ptOptional);
	ADD_PARAM(clickse, ptString | ptOptional);
	ADD_PARAM(threshold, ptInteger | ptOptional);
	REG_END();

	REG_CMD(video, "播放视频");
	ADD_PARAM(file, ptString);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(fileexist, "判断一个文件是否存在");
	ADD_PARAM(file, ptString);
	ADD_PARAM(result, ptVariable);
	REG_END();

	REG_CMD(fade, "将一个通道的音量渐变到目标值");
	ADD_PARAM(channel, ptInteger);
	ADD_PARAM(time, ptInteger);
	ADD_PARAM(to, ptInteger);
	ADD_PARAM(stop, ptBool | ptOptional);
	REG_END();

	REG_CMD(save, "存档。");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(str, ptString | ptOptional);
	REG_END();

	REG_CMD(savepoint, "指定一个存档点。存档会存下最新的存档点的状态。");
	REG_END();

	REG_CMD(load, "读档。");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(onauthfailed, ptEvent | ptOptional, AVALIABLE_AFTER(2183));
	REG_END();

	REG_CMD(ruby, "显示小字注释。");
	ADD_PARAM(text, ptString);
	ADD_PARAM(size, ptInteger | ptOptional);
	ADD_PARAM(char, ptInteger | ptOptional);
	REG_END();

	REG_CMD(event, "注册事件以及它的处理目标，若事件与系统事件重名则覆盖系统默认事件（即接管系统触发的该事件）");
	ADD_PARAM(name, ptString);
	ADD_PARAM(label, ptLabel | ptOptional);
	ADD_PARAM(file, ptScript | ptOptional);
	ADD_PARAM(type, ptString | ptInteger | ptOptional);
	ADD_PARAM(exp, ptString | ptOptional);
	ADD_PARAM(param, ptDictionary | ptOptional);
	ADD_PARAM(ignore, ptBool | ptOptional);
	ADD_PARAM(enable, ptBool | ptOptional);
	ADD_PARAM(condition, ptString | ptOptional);
	REG_END();

	REG_CMD(deleteevent, "删除自定义事件中的此事件，若有被覆盖的系统默认事件则系统默认事件被重新启用");
	ADD_PARAM(name, ptString);
	REG_END();

	REG_CMD(sendevent, "手动触发一个已经定义过的事件");
	ADD_PARAM(name, ptString);
	ADD_PARAM(param, ptDictionary | ptOptional);
	REG_END();

	REG_CMD(sendsystemevent, "手动触发一个系统事件");
	ADD_PARAM(name, ptString);
	ADD_PARAM(param, ptDictionary | ptOptional);
	REG_END();

	REG_CMD(anchor, "调整一个精灵的锚点。set和get参数至多存在一个。\nset如果是字符串的话，其预设字符串有：center、topleft、topright、topcenter、leftcenter、rightcenter、bottomleft、bottomcenter、bottomright");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(set, ptString | ptPos | ptOptional);
	ADD_LIST("\"center\" \"topleft\" \"topright\" \"topcenter\" \"leftcenter\" \"rightcenter\" \"bottomleft\" \"bottomcenter\" \"bottomright\"");
	ADD_PARAM(get, ptVariable | ptOptional);
	ADD_PARAM(keep, ptBool | ptOptional);
	PROPERTY(set, get, onlyOne);
	REG_END();

	REG_CMD(er, "清除消息层上文字");
	REG_END();

	REG_CMD(quit, "退出游戏");
	ADD_PARAM(force, ptBool | ptOptional);
	REG_END();

	REG_CMD(buttonex, "创建一个按钮");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(idle, ptInteger);
	ADD_PARAM(hover, ptInteger | ptOptional);
	ADD_PARAM(click, ptInteger | ptOptional);
	ADD_PARAM(disable, ptInteger | ptOptional);
	ADD_PARAM(onenter, ptEvent | ptOptional);
	ADD_PARAM(onleave, ptEvent | ptOptional);
	ADD_PARAM(onclick, ptEvent | ptOptional);
	//20140903
	ADD_PARAM(enterse, ptString | ptOptional);
	ADD_PARAM(clickse, ptString | ptOptional);
	ADD_PARAM(threshold, ptInteger | ptOptional);
	REG_END();

	REG_CMD(idlesp, "获得一个当前没有被占用的索引值（index），这个索引将保证安全（除非你刻意去冲撞它）。\n当num缺省（为void）时，get的变量返回的是一个数值，否则，get的变量返回的是一个数组，包含了num个数值。");
	ADD_PARAM(get, ptVariable);
	ADD_PARAM(num, ptInteger | ptOptional);
	REG_END();

	REG_CMD(savetoimage, "将指定精灵的图像保存至指定的文件");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	REG_END();

	REG_CMD(screenshot, "将目标精灵截图并保存至指定精灵，截完后的图会被缩放至width*height的大小。若未指定width或height，则那个维度不会缩放，保留原目标精灵的大小。");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(width, ptInteger | ptOptional);
	ADD_PARAM(height, ptInteger | ptOptional);
	REG_END();

	//20140402
	REG_CMD(slider, "创建一个滑条");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(back, ptString);
	ADD_PARAM(block, ptString);
	ADD_PARAM(fore, ptString | ptOptional);
	ADD_PARAM(onchanged, ptEvent | ptOptional);
	ADD_PARAM(length, ptInteger | ptOptional);
	ADD_PARAM(isblockabove, ptBool | ptOptional);
	ADD_PARAM(isblockinside, ptBool | ptOptional);
	ADD_PARAM(min, ptInteger | ptOptional);
	ADD_PARAM(max, ptInteger | ptOptional);
	ADD_PARAM(value, ptInteger | ptOptional);
	ADD_PARAM(vertical, ptBool | ptOptional);
	REG_END();

	REG_CMD(sliderex, "创建一个滑条");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(back, ptInteger);
	ADD_PARAM(block, ptInteger);
	ADD_PARAM(fore, ptInteger | ptOptional);
	ADD_PARAM(onchanged, ptEvent | ptOptional);
	ADD_PARAM(length, ptInteger | ptOptional);
	ADD_PARAM(isblockabove, ptBool | ptOptional);
	ADD_PARAM(isblockinside, ptBool | ptOptional);
	ADD_PARAM(min, ptInteger | ptOptional);
	ADD_PARAM(max, ptInteger | ptOptional);
	ADD_PARAM(value, ptInteger | ptOptional);
	ADD_PARAM(vertical, ptBool | ptOptional);
	REG_END();

	REG_CMD(zorder, "");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(set, ptInteger | ptOptional);
	ADD_PARAM(get, ptVariable | ptOptional);
	PROPERTY(set, get, onlyOne);
	REG_END();


	//20140705
	REG_CMD(checkbox, "创建一个复选框");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(idle, ptString);
	ADD_PARAM(checked, ptString);
	ADD_PARAM(onchanged, ptArray | ptEvent | ptOptional);
	ADD_PARAM(state, ptBool | ptOptional);
	REG_END();

	REG_CMD(checkboxex, "创建一个复选框");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(idle, ptInteger);
	ADD_PARAM(checked, ptInteger);
	ADD_PARAM(onchanged, ptArray | ptEvent | ptOptional);
	ADD_PARAM(state, ptBool | ptOptional);
	REG_END();

	REG_CMD(inputbox, "创建一个文本输入框");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(back, ptString);
	ADD_PARAM(text, ptString | ptOptional);
	ADD_PARAM(maxchars, ptNumber | ptOptional);
	ADD_PARAM(showcursor, ptBool | ptOptional);
	ADD_PARAM(cursor, ptString | ptOptional);
	ADD_PARAM(align, ptString | ptOptional);
	ADD_PARAM(multiline, ptBool | ptOptional, AVALIABLE_AFTER(2023));
	ADD_PARAM(rect, ptRect | ptOptional, AVALIABLE_AFTER(2023));	//用array不用rect因为想支持每个元素的默认值
	ADD_PARAM(wrap, ptBool | ptOptional, AVALIABLE_AFTER(2023));
	ADD_PARAM(onchanged, ptEvent | ptOptional);
	ADD_PARAM(onok, ptEvent | ptOptional);
	REG_END();

	REG_CMD(inputboxex, "创建一个文本输入框");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(back, ptInteger);
	ADD_PARAM(text, ptString | ptOptional);
	ADD_PARAM(maxchars, ptNumber | ptOptional);
	ADD_PARAM(showcursor, ptBool | ptOptional);
	ADD_PARAM(cursor, ptInteger | ptOptional);
	ADD_PARAM(align, ptString | ptOptional);
	ADD_PARAM(multiline, ptBool | ptOptional, AVALIABLE_AFTER(2023));
	ADD_PARAM(rect, ptRect | ptOptional, AVALIABLE_AFTER(2023));	//用array不用rect因为想支持每个元素的默认值
	ADD_PARAM(wrap, ptBool | ptOptional, AVALIABLE_AFTER(2023));
	ADD_PARAM(onchanged, ptEvent | ptOptional);
	ADD_PARAM(onok, ptEvent | ptOptional);
	REG_END();

	//20140829
	REG_CMD(spriteopt, "设置一个精灵的某些属性。如果一个参数不存在，将不改变原有的状态。");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(disable, ptBool | ptOptional);
	ADD_PARAM(recursive, ptBool | ptOptional);
	REG_END();

	REG_CMD(particle, "载入一个粒子系统");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	REG_END();

	REG_CMD(particleopt, "修改一个例子系统的暂停/运行状态或其他参数");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(paused, ptBool | ptOptional);
	ADD_PARAM(updateparam, ptDictionary | ptOptional);
	REG_END();

	REG_CMD(spine, "载入一个spine动画");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(skeleton, ptString);
	ADD_PARAM(atlas, ptString);
	ADD_PARAM(zoom, ptNumber | ptOptional);
	REG_END();

	//20140924
	// 	REG_CMD(bgmvol, "");
	// 	ADD_PARAM(get, VT_VARIABLE | VT_OPTIONAL);
	// 	ADD_PARAM(set, VT_VOL | VT_OPTIONAL);
	// 	PROPERTY(set, get, onlyOne);
	// 	REG_END();
	// 
	// 	REG_CMD(sevol, "");
	// 	ADD_PARAM(get, VT_VARIABLE | VT_OPTIONAL);
	// 	ADD_PARAM(set, VT_VOL | VT_OPTIONAL);
	// 	PROPERTY(set, get, onlyOne);
	// 	REG_END();
	// 
	// 	REG_CMD(voicevol, "");
	// 	ADD_PARAM(get, VT_VARIABLE | VT_OPTIONAL);
	// 	ADD_PARAM(set, VT_VOL | VT_OPTIONAL);
	// 	PROPERTY(set, get, onlyOne);
	// 	REG_END();

	//20150208
	REG_CMD(link, "从link开始，到endlink为止，中间的文字变成一个按钮。");
	ADD_PARAM(onclick, ptEvent);
	ADD_PARAM(hover, ptColor | ptColorArray | ptOptional);
	ADD_PARAM(click, ptColor | ptColorArray | ptOptional);
	ADD_PARAM(style, ptInteger | ptOptional);
	REG_END();

	REG_CMD(endlink, "从link开始，到endlink为止，中间的文字变成一个按钮。");
	REG_END();

	//20150516
	REG_CMD_VERSION(group, AVALIABLE_AFTER(2020), "建立一个按钮组，主要为非触屏且无鼠标的情况下需要。定义了键盘的方向键如何遍历按钮等精灵。");
	ADD_PARAM(id, ptTime);
	ADD_PARAM(lr, ptBool | ptOptional);
	ADD_PARAM(loop, ptBool | ptOptional);
	REG_END();

	REG_CMD_VERSION(endgroup, AVALIABLE_AFTER(2020), "结束一个按钮组的定义。");
	REG_END();

	REG_CMD_VERSION(preload, AVALIABLE_AFTER(2326), "预载一个图片文件");
	ADD_PARAM(file, ptString);
	REG_END();

	//20150518
	// 	REG_CMD_VERSION(modallayer, AVALIABLE_AFTER(2020));
	// 		ADD_PARAM(index, ptInteger);
	// 		ADD_PARAM(width, ptInteger);
	// 		ADD_PARAM(height, ptInteger);
	// 	REG_END();

	//special cmd
	//animate
	REG_SPE_CMD(animate, horizontal, ANIMATE_HORIZONTAL, "从一个图片中创建一个动画，水平模式（动画帧水平排列），即：\n一个frame为3，row为2的动画的帧播放顺序为：\n\t\t1 2 3\n\t\t4 5 6");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	ADD_PARAM(frame, ptTime | ptOptional);
	ADD_PARAM(row, ptInteger | ptOptional);
	ADD_PARAM(interval, ptInteger | ptIntegerArray | ptOptional);
	ADD_PARAM(delay, ptInteger | ptOptional);
	ADD_PARAM(loop, ptInteger | ptString | ptOptional);
	REG_END();

	REG_SPE_CMD(animate, vertical, ANIMATE_VERTICAL, "从一个图片中创建一个动画，垂直模式（动画帧垂直排列），即：\n一个frame为3，column为2的动画的帧播放顺序为：\n\t\t1 4\n\t\t2 5\n\t\t3 6");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	ADD_PARAM(frame, ptTime | ptOptional);
	ADD_PARAM(column, ptInteger | ptOptional);
	ADD_PARAM(interval, ptInteger | ptIntegerArray | ptOptional);
	ADD_PARAM(delay, ptInteger | ptOptional);
	ADD_PARAM(loop, ptInteger | ptString | ptOptional);
	REG_END();

	REG_SPE_CMD(animate, multifiles, ANIMATE_MULTIFILES, "从多个文件创建一个动画");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptStringArray);
	ADD_PARAM(interval, ptInteger | ptIntegerArray | ptOptional);
	ADD_PARAM(delay, ptInteger | ptOptional);
	ADD_PARAM(loop, ptInteger | ptString | ptOptional);
	REG_END();

	REG_SPE_CMD(animate, start, ANIMATE_START, "开始一个动画");
	ADD_PARAM(index, ptInteger);
	REG_END();
	REG_SPE_CMD(animate, cell, ANIMATE_CELL, "切换动画到某一帧（会停止动画）");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(frame, ptInteger);
	REG_END();
	REG_SPE_CMD(animate, stop, ANIMATE_STOP, "停止一个动画");
	ADD_PARAM(index, ptInteger);
	REG_END();
	//action
	REG_SPE_CMD(action, start, ACTION_START, "若在创建queue或parallel的过程中，立刻结束所有层动作的创建（相当于执行若干个end），并立刻开始创建好的动作。");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(times, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, queue, ACTION_QUEUE, "创建一个命令队列，用于按顺序执行命令");
	REG_END();
	REG_SPE_CMD(action, moveby, ACTION_MOVEBY, "控制精灵相对移动的动作");
	ADD_PARAM(pos, ptPos);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, moveto, ACTION_MOVETO, "控制精灵绝对移动的动作");
	ADD_PARAM(pos, ptPos);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, fadeto, ACTION_FADETO, "控制精灵透明度渐变的动作");
	ADD_PARAM(opacity, ptByte);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, scaleto, ACTION_SCALETO, "控制精灵绝对缩放到某一倍数的动作");
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(x, ptNumber | ptOptional);
	ADD_PARAM(y, ptNumber | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, scaleby, ACTION_SCALEBY, "控制精灵相对缩放某一倍数的动作");
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(x, ptNumber | ptOptional);
	ADD_PARAM(y, ptNumber | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, rotateto, ACTION_ROTATETO, "控制精灵绝对旋转至某一角度的动作");
	ADD_PARAM(rotate, ptInteger);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, rotateby, ACTION_ROTATEBY, "控制精灵相对旋转某一角度的动作");
	ADD_PARAM(rotate, ptInteger);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, tintto, ACTION_TINTTO, "控制精灵色调变至某一颜色的动作");
	ADD_PARAM(color, ptInteger);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, tintby, ACTION_TINTBY, "控制精灵色调改变某一颜色的动作");
	ADD_PARAM(r, ptInteger);
	ADD_PARAM(g, ptInteger);
	ADD_PARAM(b, ptInteger);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, jump, ACTION_JUMP, "精灵执行向上跳动的动作");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(height, ptInteger);
	ADD_PARAM(x, ptInteger | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(times, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, shake, ACTION_SHAKE, "精灵执行摇动的动作");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(range, ptInteger);
	ADD_PARAM(vertical, ptBool | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(times, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, quake, ACTION_QUAKE, "震动");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(rangex, ptInteger | ptOptional);
	ADD_PARAM(rangey, ptInteger | ptOptional);
	ADD_PARAM(speed, ptInteger | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, visible, ACTION_VISIBLE, "调整精灵是否可见（但不移除）");
	ADD_PARAM(visible, ptBool);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, remove, ACTION_REMOVE, "从该精灵的父精灵上移除它");
	ADD_PARAM(delete, ptBool | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, flipx, ACTION_FLIPX, "水平翻转");
	ADD_PARAM(force, ptBool | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, flipy, ACTION_FLIPY, "垂直翻转");
	ADD_PARAM(force, ptBool | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, scissorby, ACTION_SCISSORBY, "相对改变某个精灵显示图像的范围");
	ADD_PARAM(rect, ptRect);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, scissorto, ACTION_SCISSORTO, "绝对改变某个精灵显示图像的范围");
	ADD_PARAM(rect, ptRect);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, shaky3d, ACTION_SHAKY3D, "网格的震动效果");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(hnum, ptTime | ptOptional);
	ADD_PARAM(vnum, ptTime | ptOptional);
	ADD_PARAM(offset, ptTime | ptOptional);
	ADD_PARAM(speed, ptInteger | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();

	REG_SPE_CMD(action, delay, ACTION_DELAY, "只能用于queue或者parallel中，用于两个动作间的时间间隔");
	ADD_PARAM(time, ptTime);
	REG_END();
	REG_SPE_CMD(action, callback, ACTION_CALLBACK, "只能用于queue或者parallel中，用于某个时间点访问指定脚本");
	ADD_PARAM(label, ptString | ptOptional);
	ADD_PARAM(file, ptString | ptOptional);
	ADD_PARAM(exp, ptString | ptOptional);
	ADD_PARAM(param, ptDictionary | ptOptional);
	ADD_PARAM(wait, ptBool | ptOptional);
	REG_END();
	REG_SPE_CMD(action, end, ACTION_END, "只能用于queue或者parallel中，结束当前queue或者parallel创建并添加到上一级queue或者parallel最后。若没有上一级，则什么都不做。");
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(times, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, parallel, ACTION_PARALLEL, "创建一个并行命令集合，用于同时执行命令。结束时间按照最长的动作时间计算。");
	REG_END();
	//trans
	REG_SPE_CMD(trans, normal, TRANS_NORMAL, "瞬间切换到目标状态（即无渐变）");
	ADD_PARAM(index, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, crossfade, TRANS_CROSSFADE, "透明度渐变");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, universal, TRANS_UNIVERSAL, "根据遮罩文件进行渐变");
	ADD_PARAM(rule, ptString);
	ADD_PARAM(time, ptTime);
	ADD_PARAM(vague, ptTime | ptOptional);
	ADD_PARAM(index, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, scroll, TRANS_SCROLL, "卷动变换。\"nostay\"为新图像将旧图像挤出，\"stayfore\"为旧图像逐渐向另一方向抽出，\"stayback\"为旧图像保持在底部，新图像逐渐移入");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(from, ptString | ptOptional);
	ADD_PARAM(stay, ptString | ptOptional);
	ADD_LIST("\"nostay\" \"stayfore\" \"stayback\"");
	REG_END();
	REG_SPE_CMD(trans, shutter, TRANS_SHUTTER, "百叶窗渐变，横向时num默认值为16，纵向时num默认值为12");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(from, ptString | ptOptional);
	ADD_PARAM(num, ptInteger | ptOptional);	//default=12
	REG_END();
	REG_SPE_CMD(trans, ripple, TRANS_RIPPLE, "水波纹渐变");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(center, ptPos | ptOptional);
	ADD_PARAM(speed, ptInteger | ptOptional);
	ADD_PARAM(rwidth, ptInteger | ptOptional);
	ADD_PARAM(maxdrift, ptInteger | ptOptional);
	ADD_PARAM(roundness, ptVol | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, rotateswap, TRANS_ROTATESWAP, "旋转切换");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(bgcolor, ptColor | ptOptional);
	ADD_PARAM(twist, ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, rotatevanish, TRANS_ROTATEVANISH, "原图旋转消失");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(center, ptPos | ptOptional);
	ADD_PARAM(accel, ptNumber | ptOptional);
	ADD_PARAM(twist, ptNumber | ptOptional);
	ADD_PARAM(twistaccel, ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, rotatezoom, TRANS_ROTATEZOOM, "新图旋转放大");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(center, ptPos | ptOptional);
	ADD_PARAM(accel, ptNumber | ptOptional);
	ADD_PARAM(twist, ptNumber | ptOptional);
	ADD_PARAM(twistaccel, ptNumber | ptOptional);
	ADD_PARAM(factor, ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, wave, TRANS_WAVE, "波浪渐变");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(maxh, ptInteger | ptOptional);
	ADD_PARAM(maxomega, ptNumber | ptOptional);
	ADD_PARAM(bgcolor1, ptColor | ptOptional);
	ADD_PARAM(bgcolor2, ptColor | ptOptional);
	ADD_PARAM(wavetype, ptTime | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, turn, TRANS_TURN, "沿对角线分方块翻转为新图");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(step, ptNumber | ptOptional);
	ADD_PARAM(bgcolor, ptColor | ptOptional);
	REG_END();
	//effect
	REG_SPE_CMD(effect, delete, EFFECT_DELETE, "删除目标精灵上的特定效果。name和index为二选一的选项。\n如果使用name，那么所有同名的效果都被移除，\n如果使用index，则指定次序的效果被移除。效果的次序为默认递增1，即最先加的effect的次序为0，下一个为1，以此类推。");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(name, ptString | ptOptional);
	ADD_PARAM(index, ptInteger | ptOptional);
	PROPERTY(name, index, onlyOne);
	REG_END();
	REG_SPE_CMD(effect, clear, EFFECT_CLEAR, "清空目标精灵上的所有效果");
	ADD_PARAM(target, ptInteger);
	REG_END();
	REG_SPE_CMD(effect, scissor, EFFECT_SCISSOR, "对于非图片精灵，该效果使得该精灵会裁剪子精灵超出自身范围的区域。对图片精灵则无任何效果");
	ADD_PARAM(target, ptInteger);
	REG_END();
	REG_SPE_CMD(effect, mono, EFFECT_MONO, "使目标精灵褪色到指定颜色");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(mono, ptColor);
	REG_END();
	REG_SPE_CMD(effect, gray, EFFECT_GRAY, "使目标精灵灰度化");
	ADD_PARAM(target, ptInteger);
	REG_END();
	REG_SPE_CMD(effect, mask, EFFECT_MASK, "给目标精灵加遮罩。mask和masksp为二选一的选项。\n如果使用mask，则提供一个遮罩图片文件名，如果使用masksp，则用指定的精灵作为遮罩。\nmask精灵的大小位置旋转将被忽略，缩放匹配目标sp。");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(mask, ptString | ptOptional);
	ADD_PARAM(masksp, ptInteger | ptOptional);
	PROPERTY(mask, masksp, onlyOne);
	REG_END();
	REG_SPE_CMD(effect, stencil, EFFECT_STENCIL, "给目标精灵加蒙版。蒙版精灵的位置为相对目标精灵的位置，对于蒙版精灵不透明的区域，目标精灵将在这些区域成为透明（被镂空），而目标精灵其他区域不受影响。");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(rule, ptInteger);
	REG_END();
	REG_SPE_CMD(effect, gamma, EFFECT_GAMMA, "调整目标精灵的gamma（伽马）值。");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(down, ptColor);
	ADD_PARAM(up, ptColor);
	REG_END();
	REG_SPE_CMD(effect, invert, EFFECT_INVERT, "使目标精灵反色。");
	ADD_PARAM(target, ptInteger);
	REG_END();
	REG_SPE_CMD(effect, BC, EFFECT_BC, "调整目标精灵的亮度与对比度。");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(bright, ptInteger | ptOptional);
	ADD_PARAM(contrast, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(effect, add, EFFECT_ADD, "指定一个精灵或图片，和目标精灵的混合方式为简单的颜色叠加，而非普通的透明度叠加。\nmask和masksp为二选一的选项。mask的alpha通道会被忽略。");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(mask, ptString | ptOptional);
	ADD_PARAM(masksp, ptInteger | ptOptional);
	PROPERTY(mask, masksp, onlyOne);
	ADD_PARAM(stretch, ptBool | ptOptional);
	REG_END();
	REG_SPE_CMD(effect, multiply, EFFECT_MULTIPLY, "指定一个精灵或图片，和目标精灵的混合方式为正片叠底，而非普通的透明度叠加。mask和masksp为二选一的选项。\nmask的alpha通道会被用于原图片和正片叠底后图片的alpha混合。\n正片叠底不改变原图片的透明度。");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(mask, ptString | ptOptional);
	ADD_PARAM(masksp, ptInteger | ptOptional);
	PROPERTY(mask, masksp, onlyOne);
	ADD_PARAM(stretch, ptBool | ptOptional);
	REG_END();
	REG_SPE_CMD(effect, blur, EFFECT_BLUR, "使目标精灵模糊");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(radius, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(effect, sharpen, EFFECT_SHARPEN, "使目标精灵锐利");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(radius, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(effect, edge, EFFECT_EDGE, "使目标精灵边缘突出");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(radius, ptInteger | ptOptional);
	REG_END();

	cmd_inited = true;
};
