#include <weh.h>
#include "cmdlist_wrapper.h"

#define REG_CMD(a, b) do{ auto &_v = CmdList[#a]; _v.name=#a; _v.detail = b;
//#define REG_CMD(a) do{ auto &_v = CmdList[#a]; _v.name=#a; _v.detail = "˵����ȱ";
#define REG_CMD_VERSION(a, v, b) REG_CMD(a, b);
#define REG_END() }while(0)
#define ADD_PARAM(b, c, ...) _v.argNames.push_back(#b);_v.argFlags.push_back(c);
#define PROPERTY(a, b, c) 
#define REG_SPE_CMD(a, b, c, d) { auto &_p = SpecialCmdList[#a].modes[#b]; _p.first = c; auto &_v = _p.second; _v.name=#b;_v.detail = d; _v.argNames.push_back("mode"); _v.argFlags.push_back(ptString);
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

	//�û���Ϣ���
	REG_CMD(check, "���ָ�����½��Ƿ��ѹ���");
	ADD_PARAM(chapter, ptString | ptStringArray);
	ADD_PARAM(onsuccess, ptEvent | ptOptional);
	ADD_PARAM(onfailure, ptEvent | ptOptional);
	REG_END();

	REG_CMD(queryitem, "����������ѯ����ָ���½ڵ���Ʒ");
	ADD_PARAM(chapter, ptString | ptStringArray);
	ADD_PARAM(all, ptBool);
	ADD_PARAM(onsuccess, ptEvent | ptOptional);
	ADD_PARAM(onfailure, ptEvent | ptOptional);
	REG_END();

	REG_CMD(buyitem, "������Ʒ");
	ADD_PARAM(item, ptString);
	ADD_PARAM(onsuccess, ptEvent | ptOptional);
	ADD_PARAM(onfailure, ptEvent | ptOptional);
	REG_END();

	REG_CMD(cursor, "�������ָ��\nfile��index����������һ������������Ϊ�������ָ�����ϵͳĬ��");
	ADD_PARAM(file, ptString | ptOptional);
	ADD_PARAM(index, ptInteger | ptOptional);
	PROPERTY(image, index, atmostOne);
	REG_END();

	REG_CMD(l2dsprite, "����һ��live2dģ�ͣ���Ҫ��live2d key��");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	REG_END();

	REG_CMD(sprite, "��index��ָΪһ�����飬���޸�����״̬�����򽫴���һ�����顣");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	ADD_PARAM(rect, ptRect | ptOptional);
	REG_END();

	REG_CMD(addto, "���һ�����飨Դ���飩����һ������飨Ŀ�꾫�飩�ϣ������͸���Ƚ��ᴫ��");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(zorder, ptInteger | ptOptional);
	ADD_PARAM(pos, ptPos | ptOptional);
	ADD_PARAM(opacity, ptByte | ptOptional);
	ADD_PARAM(modal, ptBool | ptOptional, AVALIABLE_AFTER(2027));
	REG_END();

	REG_CMD(layer, "����һ��ָ����С����ɫ���飬�����͸������������鱾���͸����");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(width, ptInteger);
	ADD_PARAM(height, ptInteger);
	ADD_PARAM(color, ptInteger | ptOptional);
	ADD_PARAM(opacity, ptByte | ptOptional);
	REG_END();

	REG_CMD(click, "�ȴ���������������");
	REG_END();

	REG_CMD(remove, "��һ�������parent��ȡ���Լ����þ����Ϊ���ɼ�");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(delete, ptBool | ptOptional);
	REG_END();

	REG_CMD(removeall, "��һ��������ȡ�������Ӿ��飬�����Ӿ����Ϊ���ɼ�");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(delete, ptBool | ptOptional);
	ADD_PARAM(recursive, ptBool | ptOptional);
	REG_END();

	REG_CMD(info, "��ȡһ��ͼƬ����Ϣ����ȡһ���ֵ䱣����get�ַ�����Ӧ�ı�����\n�ֵ��Ա��filename��width��height");
	ADD_PARAM(file, ptString);
	ADD_PARAM(get, ptVariable);
	REG_END();

	REG_CMD(infoex, "��ȡһ���������Ϣ����ȡһ���ֵ䱣����get�ַ�����Ӧ�ı�����\n�ֵ��Ա��filename��width��height��type��pos��index��parent��children");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(get, ptVariable);
	REG_END();

	REG_CMD(bgm, "���ű�������");
	ADD_PARAM(file, ptString);
	ADD_PARAM(loop, ptBool | ptOptional);
	ADD_PARAM(vol, ptVol | ptOptional);
	ADD_PARAM(fadein, ptTime | ptOptional);
	ADD_PARAM(loopto, ptTime | ptOptional);
	REG_END();

	REG_CMD(se, "������Ч");
	ADD_PARAM(file, ptString);
	ADD_PARAM(channel, ptInteger);
	ADD_PARAM(loop, ptBool | ptOptional);
	ADD_PARAM(vol, ptVol | ptOptional);
	ADD_PARAM(fadein, ptTime | ptOptional);
	REG_END();

	REG_CMD(voice, "��������");
	ADD_PARAM(file, ptString);
	ADD_PARAM(vol, ptVol | ptOptional);
	REG_END();

	REG_CMD(stop, "ֹͣ����");
	ADD_PARAM(channel, ptInteger | ptOptional);
	ADD_PARAM(fadeout, ptTime | ptOptional);
	REG_END();

	REG_CMD(volume, "��ȡ���߸�������������Ա��β�����Ч����set��get���������ұ������һ����");
	ADD_PARAM(channel, ptInteger);
	ADD_PARAM(set, ptVol | ptOptional);
	ADD_PARAM(get, ptVariable | ptOptional);
	PROPERTY(set, get, onlyOne);
	REG_END();

	REG_CMD(pause, "��ͣ����");
	ADD_PARAM(channel, ptInteger);
	REG_END();

	REG_CMD(resume, "�ָ�����");
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
	REG_CMD(text, "��ʾһ�仰");
	ADD_PARAM(text, ptString);
	REG_END();

	REG_CMD(messagelayer, "��index��ָ�ľ������һ����Ϣ�㣬��ı�����active״̬�����򴴽�һ����Ϣ�㡣");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(active, ptBool | ptInteger);;
	REG_END();

	REG_CMD(textcursor, "�������ֵȴ����");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(follow, ptBool);
	ADD_PARAM(pos, ptPos | ptOptional);
	REG_END();

	REG_CMD(textwindow, "�����ı�������,����ѡ����û���趨��û������˵���ģ���Ĭ��ʹ�õ�ǰ���ı������á�\n����opacity��ʼ��Ϊ255��pos��ʼ��Ϊ[0,0]��xinterval��ʼ��Ϊ0��yinterval��ʼ��Ϊ12��file��color�����������һ����");
	ADD_PARAM(file, ptString | ptOptional);
	ADD_PARAM(color, ptInteger | ptOptional);
	PROPERTY(file, color, atmostOne);
	ADD_PARAM(opacity, ptByte | ptOptional);
	ADD_PARAM(pos, ptPos | ptOptional);
	ADD_PARAM(rect, ptRect | ptOptional);
	ADD_PARAM(xinterval, ptInteger | ptOptional);
	ADD_PARAM(yinterval, ptInteger | ptOptional);
	REG_END();

	REG_CMD(textoff, "�����ı���");
	ADD_PARAM(time, ptTime | ptOptional, AVALIABLE_AFTER(2307));
	REG_END();

	REG_CMD(texton, "��ʾ�ı���");
	ADD_PARAM(time, ptTime | ptOptional, AVALIABLE_AFTER(2307));
	REG_END();

	REG_CMD(locate, "����һ�����ֵ����λ�ø�Ϊx��y��");
	ADD_PARAM(x, ptInteger | ptOptional);
	ADD_PARAM(y, ptInteger | ptOptional);
	REG_END();

	//REG_CMD(textshadow, "");
	//	ADD_PARAM(shadow, VT_BOOL);
	//	ADD_PARAM(color, VT_INTEGER|VT_OPTIONAL);
	//REG_END();

	REG_CMD(textspeed, "����������ʾ�ٶȣ���Χ0-100��Խ��Խ��");
	ADD_PARAM(interval, ptInteger);
	REG_END();

	REG_CMD(textsprite, "����һ�����־��顣����ѡ����û���趨��û������˵���ģ���Ĭ��ʹ�õ�ǰ���������á�\n���ö�����ʱ�ģ�ֻ�Ը����־�����Ч��");
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

	REG_CMD(textstyle, "���õ�ǰ���ִ�С����ɫ�����ԡ�����ѡ����û���趨��û������˵���ģ���Ĭ��ʹ�õ�ǰ���ı�������á�\n��������Ĭ��ʹ��ϵͳ���壬size��ʼ��Ϊ24��color��ʼ��Ϊ0xFFFFFF��б��ȳ�ʼ��Ϊfalse��\nenableshadow��enablestroke��ʼ��Ϊfalse��shadow��stroke��ʼ��Ϊ0x000000");
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

	REG_CMD(storefont, "�ݴ�������Ϣ��������storefont��ֻ�ᱣ�����һ�εĽ����\n��restorefont������Խ�������Ϊ���һ��storefontʱ��״̬��");
	REG_END();

	REG_CMD(restorefont, "��ԭ������Ϣ");
	REG_END();

	REG_CMD(texttag, "�趨�����ı�����ʱ�Ĵ����൱��callһ��macro��������Ϊ�ַ������ڱ���tag�С�");
	ADD_PARAM(label, ptLabel);
	ADD_PARAM(file, ptScript | ptOptional);
	ADD_PARAM(autoclean, ptBool | ptOptional, AVALIABLE_AFTER(2327));
	REG_END();

	REG_CMD(history, "�����Ƿ�����ʷ��¼");
	ADD_PARAM(output, ptBool);
	REG_END();

	REG_CMD(i, "����б��");
	REG_END();

	REG_CMD(b, "���ش���");
	REG_END();

	REG_CMD(s, "����ɾ����");
	REG_END();

	REG_CMD(u, "�����»���");
	REG_END();

	REG_CMD(r, "�ֶ�����");
	REG_END();

	REG_CMD(l, "�ȴ����");
	REG_END();

	REG_CMD(p, "��ҳ�ȴ��������ҳ");
	REG_END();

	REG_CMD(eval, "ִ��һ�����ʽ����ֵ�ȵȣ�");
	ADD_PARAM(exp, ptString);
	REG_END();

	REG_CMD(jump, "��ת��ָ����ǩ");
	ADD_PARAM(label, ptLabel);
	ADD_PARAM(file, ptScript | ptOptional);
	REG_END();

	REG_CMD(call, "����(call)ָ����ǩ������return");
	ADD_PARAM(label, ptLabel);
	ADD_PARAM(file, ptScript | ptOptional);
	REG_END();

	REG_CMD(return, "���أ�����ֵ���浽����macroʱreturn�������ı�����");
	ADD_PARAM(value, ptEverything | ptOptional);
	REG_END();

	REG_CMD(returnto, "����һ��call�з��أ�Ȼ��jump��ָ����block��file");
	ADD_PARAM(label, ptLabel);
	ADD_PARAM(file, ptScript | ptOptional);
	ADD_PARAM(value, ptEverything | ptOptional);
	REG_END();

	REG_CMD(if, "����������ʽ�ж�ִ���������仹������else��elseif����ִ�С�");
	ADD_PARAM(exp, ptString | ptConst);
	REG_END();

	REG_CMD(else, "��if���ʹ��");
	REG_END();

	REG_CMD(elseif, "����������ʽ�ж�ִ���������仹��������һ��else��elseif����ִ�С�");
	ADD_PARAM(exp, ptString | ptConst);
	REG_END();

	REG_CMD(endif, "��־��if��Ľ���");
	REG_END();

	REG_CMD(tablegoto, "������ʽ�����ݷ��ص�ֵ��ת����ǩ�����Ӧ��Ԫ��ָʾ�ı�ǩ��ֻ��ͬһ�ļ�����ת��");
	ADD_PARAM(exp, ptTime);
	ADD_PARAM(target, ptLabelArray);
	REG_END();

	REG_CMD(for, "varָ���ı�������range�����ÿһ��ֵ��ÿ����һ��ִ��һ��for�顣");
	ADD_PARAM(var, ptVariable);
	ADD_PARAM(range, ptArray);
	REG_END();

	REG_CMD(next, "forѭ�������ֹ���");
	REG_END();

	REG_CMD(continue, "��ֹ����forѭ����ʼ�´�ѭ��");
	REG_END();

	REG_CMD(break, "����forѭ��");
	REG_END();

	REG_CMD(wait, "�ȴ�һ��ʱ��");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitaction, "�ȴ�ĳ������Ķ���ִ����ϣ�������������������ʱ��ͬʱֹͣaction");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitallaction, "�ȴ����о���Ķ���ִ����ϣ�������������������ʱ��ͬʱֹͣ���о����action");
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitbutton, "�ȴ���ť��ת�¼�����");
	ADD_PARAM(time, ptTime | ptOptional);
	REG_END();

	REG_CMD(waitbgm, "�ȴ�bgm������ϣ�������������������ʱ��ͬʱֹͣbgm");
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitfade, "�ȴ���Ƶ������ϣ�������������������ʱ�����������Ƶ����");
	ADD_PARAM(channel, ptInteger);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waittrans, "�ȴ�trans��ϣ�������������������ʱ��ͬʱֹͣtrans");
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitalltrans, "�ȴ����о����trans��ϣ�������������������ʱ��ͬʱֹͣ���о����trans");
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitvoice, "�ȴ�����������ϣ�������������������ʱ��ͬʱֹͣ����");
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(waitse, "�ȴ���Ч������ϣ�������������������ʱ��ͬʱֹͣ��Ч");
	ADD_PARAM(channel, ptInteger);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(stoptrans, "ǿ����ֹĳ������Ľ���");
	ADD_PARAM(index, ptInteger);
	REG_END();

	REG_CMD(stopaction, "ǿ����ֹĳ������Ķ���");
	ADD_PARAM(index, ptInteger);
	REG_END();

	REG_CMD(cancelaction, "ǿ��ȡ��ĳ������Ķ���");
	ADD_PARAM(index, ptInteger);
	REG_END();

	REG_CMD(stopalltrans, "ǿ����ֹ���о���Ľ���");
	REG_END();

	REG_CMD(stopallaction, "ǿ����ֹ���о���Ķ���");
	REG_END();

	REG_CMD(cancelallaction, "ǿ��ȡ�����о���Ķ���");
	REG_END();

	REG_CMD(pretrans, "ָ��һ������׼�����䣬�������������Ļ���״̬");
	ADD_PARAM(index, ptInteger | ptOptional);
	REG_END();

	REG_CMD(button, "����һ����ť");
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

	REG_CMD(video, "������Ƶ");
	ADD_PARAM(file, ptString);
	ADD_PARAM(canskip, ptBool | ptOptional);
	REG_END();

	REG_CMD(fileexist, "�ж�һ���ļ��Ƿ����");
	ADD_PARAM(file, ptString);
	ADD_PARAM(result, ptVariable);
	REG_END();

	REG_CMD(fade, "��һ��ͨ�����������䵽Ŀ��ֵ");
	ADD_PARAM(channel, ptInteger);
	ADD_PARAM(time, ptInteger);
	ADD_PARAM(to, ptInteger);
	ADD_PARAM(stop, ptBool | ptOptional);
	REG_END();

	REG_CMD(save, "�浵��");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(str, ptString | ptOptional);
	REG_END();

	REG_CMD(savepoint, "ָ��һ���浵�㡣�浵��������µĴ浵���״̬��");
	REG_END();

	REG_CMD(load, "������");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(onauthfailed, ptEvent | ptOptional, AVALIABLE_AFTER(2183));
	REG_END();

	REG_CMD(ruby, "��ʾС��ע�͡�");
	ADD_PARAM(text, ptString);
	ADD_PARAM(size, ptInteger | ptOptional);
	ADD_PARAM(char, ptInteger | ptOptional);
	REG_END();

	REG_CMD(event, "ע���¼��Լ����Ĵ���Ŀ�꣬���¼���ϵͳ�¼������򸲸�ϵͳĬ���¼������ӹ�ϵͳ�����ĸ��¼���");
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

	REG_CMD(deleteevent, "ɾ���Զ����¼��еĴ��¼������б����ǵ�ϵͳĬ���¼���ϵͳĬ���¼�����������");
	ADD_PARAM(name, ptString);
	REG_END();

	REG_CMD(sendevent, "�ֶ�����һ���Ѿ���������¼�");
	ADD_PARAM(name, ptString);
	ADD_PARAM(param, ptDictionary | ptOptional);
	REG_END();

	REG_CMD(sendsystemevent, "�ֶ�����һ��ϵͳ�¼�");
	ADD_PARAM(name, ptString);
	ADD_PARAM(param, ptDictionary | ptOptional);
	REG_END();

	REG_CMD(anchor, "����һ�������ê�㡣set��get�����������һ����\nset������ַ����Ļ�����Ԥ���ַ����У�center��topleft��topright��topcenter��leftcenter��rightcenter��bottomleft��bottomcenter��bottomright");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(set, ptString | ptPos | ptOptional);
	ADD_PARAM(get, ptVariable | ptOptional);
	ADD_PARAM(keep, ptBool | ptOptional);
	PROPERTY(set, get, onlyOne);
	REG_END();

	REG_CMD(er, "�����Ϣ��������");
	REG_END();

	REG_CMD(quit, "�˳���Ϸ");
	ADD_PARAM(force, ptBool | ptOptional);
	REG_END();

	REG_CMD(buttonex, "����һ����ť");
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

	REG_CMD(idlesp, "���һ����ǰû�б�ռ�õ�����ֵ��index���������������֤��ȫ�����������ȥ��ײ������\n��numȱʡ��Ϊvoid��ʱ��get�ı������ص���һ����ֵ������get�ı������ص���һ�����飬������num����ֵ��");
	ADD_PARAM(get, ptVariable);
	ADD_PARAM(num, ptInteger | ptOptional);
	REG_END();

	REG_CMD(savetoimage, "��ָ�������ͼ�񱣴���ָ�����ļ�");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	REG_END();

	REG_CMD(screenshot, "��Ŀ�꾫���ͼ��������ָ�����飬������ͼ�ᱻ������width*height�Ĵ�С����δָ��width��height�����Ǹ�ά�Ȳ������ţ�����ԭĿ�꾫��Ĵ�С��");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(width, ptInteger | ptOptional);
	ADD_PARAM(height, ptInteger | ptOptional);
	REG_END();

	//20140402
	REG_CMD(slider, "����һ������");
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

	REG_CMD(sliderex, "����һ������");
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
	REG_CMD(checkbox, "����һ����ѡ��");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(idle, ptString);
	ADD_PARAM(checked, ptString);
	ADD_PARAM(onchanged, ptArray | ptEvent | ptOptional);
	ADD_PARAM(state, ptBool | ptOptional);
	REG_END();

	REG_CMD(checkboxex, "����һ����ѡ��");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(idle, ptInteger);
	ADD_PARAM(checked, ptInteger);
	ADD_PARAM(onchanged, ptArray | ptEvent | ptOptional);
	ADD_PARAM(state, ptBool | ptOptional);
	REG_END();

	REG_CMD(inputbox, "����һ���ı������");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(back, ptString);
	ADD_PARAM(text, ptString | ptOptional);
	ADD_PARAM(maxchars, ptNumber | ptOptional);
	ADD_PARAM(showcursor, ptBool | ptOptional);
	ADD_PARAM(cursor, ptString | ptOptional);
	ADD_PARAM(align, ptString | ptOptional);
	ADD_PARAM(multiline, ptBool | ptOptional, AVALIABLE_AFTER(2023));
	ADD_PARAM(rect, ptRect | ptOptional, AVALIABLE_AFTER(2023));	//��array����rect��Ϊ��֧��ÿ��Ԫ�ص�Ĭ��ֵ
	ADD_PARAM(wrap, ptBool | ptOptional, AVALIABLE_AFTER(2023));
	ADD_PARAM(onchanged, ptEvent | ptOptional);
	ADD_PARAM(onok, ptEvent | ptOptional);
	REG_END();

	REG_CMD(inputboxex, "����һ���ı������");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(back, ptInteger);
	ADD_PARAM(text, ptString | ptOptional);
	ADD_PARAM(maxchars, ptNumber | ptOptional);
	ADD_PARAM(showcursor, ptBool | ptOptional);
	ADD_PARAM(cursor, ptInteger | ptOptional);
	ADD_PARAM(align, ptString | ptOptional);
	ADD_PARAM(multiline, ptBool | ptOptional, AVALIABLE_AFTER(2023));
	ADD_PARAM(rect, ptRect | ptOptional, AVALIABLE_AFTER(2023));	//��array����rect��Ϊ��֧��ÿ��Ԫ�ص�Ĭ��ֵ
	ADD_PARAM(wrap, ptBool | ptOptional, AVALIABLE_AFTER(2023));
	ADD_PARAM(onchanged, ptEvent | ptOptional);
	ADD_PARAM(onok, ptEvent | ptOptional);
	REG_END();

	//20140829
	REG_CMD(spriteopt, "����һ�������ĳЩ���ԡ����һ�����������ڣ������ı�ԭ�е�״̬��");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(disable, ptBool | ptOptional);
	ADD_PARAM(recursive, ptBool | ptOptional);
	REG_END();

	REG_CMD(particle, "����һ������ϵͳ");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	REG_END();

	REG_CMD(particleopt, "�޸�һ������ϵͳ����ͣ/����״̬����������");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(paused, ptBool | ptOptional);
	ADD_PARAM(updateparam, ptDictionary | ptOptional);
	REG_END();

	REG_CMD(spine, "����һ��spine����");
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
	REG_CMD(link, "��link��ʼ����endlinkΪֹ���м�����ֱ��һ����ť��");
	ADD_PARAM(onclick, ptEvent);
	ADD_PARAM(hover, ptColor | ptColorArray | ptOptional);
	ADD_PARAM(click, ptColor | ptColorArray | ptOptional);
	ADD_PARAM(style, ptInteger | ptOptional);
	REG_END();

	REG_CMD(endlink, "��link��ʼ����endlinkΪֹ���м�����ֱ��һ����ť��");
	REG_END();

	//20150516
	REG_CMD_VERSION(group, AVALIABLE_AFTER(2020), "����һ����ť�飬��ҪΪ�Ǵ������������������Ҫ�������˼��̵ķ������α�����ť�Ⱦ��顣");
	ADD_PARAM(id, ptTime);
	ADD_PARAM(lr, ptBool | ptOptional);
	ADD_PARAM(loop, ptBool | ptOptional);
	REG_END();

	REG_CMD_VERSION(endgroup, AVALIABLE_AFTER(2020), "����һ����ť��Ķ��塣");
	REG_END();

	REG_CMD_VERSION(preload, AVALIABLE_AFTER(2326), "Ԥ��һ��ͼƬ�ļ�");
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
	REG_SPE_CMD(animate, horizontal, ANIMATE_HORIZONTAL, "��һ��ͼƬ�д���һ��������ˮƽģʽ������֡ˮƽ���У�������\nһ��frameΪ3��rowΪ2�Ķ�����֡����˳��Ϊ��\n\t\t1 2 3\n\t\t4 5 6");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	ADD_PARAM(frame, ptTime | ptOptional);
	ADD_PARAM(row, ptInteger | ptOptional);
	ADD_PARAM(interval, ptInteger | ptIntegerArray | ptOptional);
	ADD_PARAM(delay, ptInteger | ptOptional);
	ADD_PARAM(loop, ptInteger | ptString | ptOptional);
	REG_END();

	REG_SPE_CMD(animate, vertical, ANIMATE_VERTICAL, "��һ��ͼƬ�д���һ����������ֱģʽ������֡��ֱ���У�������\nһ��frameΪ3��columnΪ2�Ķ�����֡����˳��Ϊ��\n\t\t1 4\n\t\t2 5\n\t\t3 6");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptString);
	ADD_PARAM(frame, ptTime | ptOptional);
	ADD_PARAM(column, ptInteger | ptOptional);
	ADD_PARAM(interval, ptInteger | ptIntegerArray | ptOptional);
	ADD_PARAM(delay, ptInteger | ptOptional);
	ADD_PARAM(loop, ptInteger | ptString | ptOptional);
	REG_END();

	REG_SPE_CMD(animate, multifiles, ANIMATE_MULTIFILES, "�Ӷ���ļ�����һ������");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(file, ptStringArray);
	ADD_PARAM(interval, ptInteger | ptIntegerArray | ptOptional);
	ADD_PARAM(delay, ptInteger | ptOptional);
	ADD_PARAM(loop, ptInteger | ptString | ptOptional);
	REG_END();

	REG_SPE_CMD(animate, start, ANIMATE_START, "��ʼһ������");
	ADD_PARAM(index, ptInteger);
	REG_END();
	REG_SPE_CMD(animate, cell, ANIMATE_CELL, "�л�������ĳһ֡����ֹͣ������");
	ADD_PARAM(index, ptInteger);
	ADD_PARAM(frame, ptInteger);
	REG_END();
	REG_SPE_CMD(animate, stop, ANIMATE_STOP, "ֹͣһ������");
	ADD_PARAM(index, ptInteger);
	REG_END();
	//action
	REG_SPE_CMD(action, start, ACTION_START, "���ڴ���queue��parallel�Ĺ����У����̽������в㶯���Ĵ������൱��ִ�����ɸ�end���������̿�ʼ�����õĶ�����");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(times, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, queue, ACTION_QUEUE, "����һ��������У����ڰ�˳��ִ������");
	REG_END();
	REG_SPE_CMD(action, moveby, ACTION_MOVEBY, "���ƾ�������ƶ��Ķ���");
	ADD_PARAM(pos, ptPos);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, moveto, ACTION_MOVETO, "���ƾ�������ƶ��Ķ���");
	ADD_PARAM(pos, ptPos);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, fadeto, ACTION_FADETO, "���ƾ���͸���Ƚ���Ķ���");
	ADD_PARAM(opacity, ptByte);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, scaleto, ACTION_SCALETO, "���ƾ���������ŵ�ĳһ�����Ķ���");
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(x, ptNumber | ptOptional);
	ADD_PARAM(y, ptNumber | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, scaleby, ACTION_SCALEBY, "���ƾ����������ĳһ�����Ķ���");
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(x, ptNumber | ptOptional);
	ADD_PARAM(y, ptNumber | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, rotateto, ACTION_ROTATETO, "���ƾ��������ת��ĳһ�ǶȵĶ���");
	ADD_PARAM(rotate, ptInteger);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, rotateby, ACTION_ROTATEBY, "���ƾ��������תĳһ�ǶȵĶ���");
	ADD_PARAM(rotate, ptInteger);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, tintto, ACTION_TINTTO, "���ƾ���ɫ������ĳһ��ɫ�Ķ���");
	ADD_PARAM(color, ptInteger);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, tintby, ACTION_TINTBY, "���ƾ���ɫ���ı�ĳһ��ɫ�Ķ���");
	ADD_PARAM(r, ptInteger);
	ADD_PARAM(g, ptInteger);
	ADD_PARAM(b, ptInteger);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, jump, ACTION_JUMP, "����ִ�����������Ķ���");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(height, ptInteger);
	ADD_PARAM(x, ptInteger | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(times, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(action, shake, ACTION_SHAKE, "����ִ��ҡ���Ķ���");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(range, ptInteger);
	ADD_PARAM(vertical, ptBool | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(times, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, quake, ACTION_QUAKE, "��");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(rangex, ptInteger | ptOptional);
	ADD_PARAM(rangey, ptInteger | ptOptional);
	ADD_PARAM(speed, ptInteger | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, visible, ACTION_VISIBLE, "���������Ƿ�ɼ��������Ƴ���");
	ADD_PARAM(visible, ptBool);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, remove, ACTION_REMOVE, "�Ӹþ���ĸ��������Ƴ���");
	ADD_PARAM(delete, ptBool | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, flipx, ACTION_FLIPX, "ˮƽ��ת");
	ADD_PARAM(force, ptBool | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, flipy, ACTION_FLIPY, "��ֱ��ת");
	ADD_PARAM(force, ptBool | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, scissorby, ACTION_SCISSORBY, "��Ըı�ĳ��������ʾͼ��ķ�Χ");
	ADD_PARAM(rect, ptRect);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, scissorto, ACTION_SCISSORTO, "���Ըı�ĳ��������ʾͼ��ķ�Χ");
	ADD_PARAM(rect, ptRect);
	ADD_PARAM(time, ptTime | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(ease, ptString | ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, shaky3d, ACTION_SHAKY3D, "�������Ч��");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(hnum, ptTime | ptOptional);
	ADD_PARAM(vnum, ptTime | ptOptional);
	ADD_PARAM(offset, ptTime | ptOptional);
	ADD_PARAM(speed, ptInteger | ptOptional);
	ADD_PARAM(target, ptInteger | ptOptional);
	REG_END();

	REG_SPE_CMD(action, delay, ACTION_DELAY, "ֻ������queue����parallel�У����������������ʱ����");
	ADD_PARAM(time, ptTime);
	REG_END();
	REG_SPE_CMD(action, callback, ACTION_CALLBACK, "ֻ������queue����parallel�У�����ĳ��ʱ������ָ���ű�");
	ADD_PARAM(label, ptString | ptOptional);
	ADD_PARAM(file, ptString | ptOptional);
	ADD_PARAM(exp, ptString | ptOptional);
	ADD_PARAM(param, ptDictionary | ptOptional);
	ADD_PARAM(wait, ptBool | ptOptional);
	REG_END();
	REG_SPE_CMD(action, end, ACTION_END, "ֻ������queue����parallel�У�������ǰqueue����parallel��������ӵ���һ��queue����parallel�����û����һ������ʲô��������");
	ADD_PARAM(target, ptInteger | ptOptional);
	ADD_PARAM(times, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(action, parallel, ACTION_PARALLEL, "����һ����������ϣ�����ͬʱִ���������ʱ�䰴����Ķ���ʱ����㡣");
	REG_END();
	//trans
	REG_SPE_CMD(trans, normal, TRANS_NORMAL, "˲���л���Ŀ��״̬�����޽��䣩");
	ADD_PARAM(index, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, crossfade, TRANS_CROSSFADE, "͸���Ƚ���");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, universal, TRANS_UNIVERSAL, "���������ļ����н���");
	ADD_PARAM(rule, ptString);
	ADD_PARAM(time, ptTime);
	ADD_PARAM(vague, ptTime | ptOptional);
	ADD_PARAM(index, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, scroll, TRANS_SCROLL, "���任��\"nostay\"Ϊ��ͼ�񽫾�ͼ�񼷳���\"stayfore\"Ϊ��ͼ��������һ��������\"stayback\"Ϊ��ͼ�񱣳��ڵײ�����ͼ��������");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(from, ptString | ptOptional);
	ADD_PARAM(stay, ptString | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, shutter, TRANS_SHUTTER, "��Ҷ�����䣬����ʱnumĬ��ֵΪ16������ʱnumĬ��ֵΪ12");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(from, ptString | ptOptional);
	ADD_PARAM(num, ptInteger | ptOptional);	//default=12
	REG_END();
	REG_SPE_CMD(trans, ripple, TRANS_RIPPLE, "ˮ���ƽ���");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(center, ptPos | ptOptional);
	ADD_PARAM(speed, ptInteger | ptOptional);
	ADD_PARAM(rwidth, ptInteger | ptOptional);
	ADD_PARAM(maxdrift, ptInteger | ptOptional);
	ADD_PARAM(roundness, ptVol | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, rotateswap, TRANS_ROTATESWAP, "��ת�л�");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(bgcolor, ptColor | ptOptional);
	ADD_PARAM(twist, ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, rotatevanish, TRANS_ROTATEVANISH, "ԭͼ��ת��ʧ");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(center, ptPos | ptOptional);
	ADD_PARAM(accel, ptNumber | ptOptional);
	ADD_PARAM(twist, ptNumber | ptOptional);
	ADD_PARAM(twistaccel, ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, rotatezoom, TRANS_ROTATEZOOM, "��ͼ��ת�Ŵ�");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(center, ptPos | ptOptional);
	ADD_PARAM(accel, ptNumber | ptOptional);
	ADD_PARAM(twist, ptNumber | ptOptional);
	ADD_PARAM(twistaccel, ptNumber | ptOptional);
	ADD_PARAM(factor, ptNumber | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, wave, TRANS_WAVE, "���˽���");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(maxh, ptInteger | ptOptional);
	ADD_PARAM(maxomega, ptNumber | ptOptional);
	ADD_PARAM(bgcolor1, ptColor | ptOptional);
	ADD_PARAM(bgcolor2, ptColor | ptOptional);
	ADD_PARAM(wavetype, ptTime | ptOptional);
	REG_END();
	REG_SPE_CMD(trans, turn, TRANS_TURN, "�ضԽ��߷ַ��鷭תΪ��ͼ");
	ADD_PARAM(time, ptTime);
	ADD_PARAM(index, ptInteger | ptOptional);
	ADD_PARAM(step, ptNumber | ptOptional);
	ADD_PARAM(bgcolor, ptColor | ptOptional);
	REG_END();
	//effect
	REG_SPE_CMD(effect, delete, EFFECT_DELETE, "ɾ��Ŀ�꾫���ϵ��ض�Ч����name��indexΪ��ѡһ��ѡ�\n���ʹ��name����ô����ͬ����Ч�������Ƴ���\n���ʹ��index����ָ�������Ч�����Ƴ���Ч���Ĵ���ΪĬ�ϵ���1�������ȼӵ�effect�Ĵ���Ϊ0����һ��Ϊ1���Դ����ơ�");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(name, ptString | ptOptional);
	ADD_PARAM(index, ptInteger | ptOptional);
	PROPERTY(name, index, onlyOne);
	REG_END();
	REG_SPE_CMD(effect, clear, EFFECT_CLEAR, "���Ŀ�꾫���ϵ�����Ч��");
	ADD_PARAM(target, ptInteger);
	REG_END();
	REG_SPE_CMD(effect, scissor, EFFECT_SCISSOR, "���ڷ�ͼƬ���飬��Ч��ʹ�øþ����ü��Ӿ��鳬������Χ�����򡣶�ͼƬ���������κ�Ч��");
	ADD_PARAM(target, ptInteger);
	REG_END();
	REG_SPE_CMD(effect, mono, EFFECT_MONO, "ʹĿ�꾫����ɫ��ָ����ɫ");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(mono, ptColor);
	REG_END();
	REG_SPE_CMD(effect, gray, EFFECT_GRAY, "ʹĿ�꾫��ҶȻ�");
	ADD_PARAM(target, ptInteger);
	REG_END();
	REG_SPE_CMD(effect, mask, EFFECT_MASK, "��Ŀ�꾫������֡�mask��maskspΪ��ѡһ��ѡ�\n���ʹ��mask�����ṩһ������ͼƬ�ļ��������ʹ��masksp������ָ���ľ�����Ϊ���֡�\nmask����Ĵ�Сλ����ת�������ԣ�����ƥ��Ŀ��sp��");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(mask, ptString | ptOptional);
	ADD_PARAM(masksp, ptInteger | ptOptional);
	PROPERTY(mask, masksp, onlyOne);
	REG_END();
	REG_SPE_CMD(effect, stencil, EFFECT_STENCIL, "��Ŀ�꾫����ɰ档�ɰ澫���λ��Ϊ���Ŀ�꾫���λ�ã������ɰ澫�鲻͸��������Ŀ�꾫�齫����Щ�����Ϊ͸�������οգ�����Ŀ�꾫������������Ӱ�졣");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(rule, ptInteger);
	REG_END();
	REG_SPE_CMD(effect, gamma, EFFECT_GAMMA, "����Ŀ�꾫���gamma��٤��ֵ��");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(down, ptColor);
	ADD_PARAM(up, ptColor);
	REG_END();
	REG_SPE_CMD(effect, invert, EFFECT_INVERT, "ʹĿ�꾫�鷴ɫ��");
	ADD_PARAM(target, ptInteger);
	REG_END();
	REG_SPE_CMD(effect, BC, EFFECT_BC, "����Ŀ�꾫���������Աȶȡ�");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(bright, ptInteger | ptOptional);
	ADD_PARAM(contrast, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(effect, add, EFFECT_ADD, "ָ��һ�������ͼƬ����Ŀ�꾫��Ļ�Ϸ�ʽΪ�򵥵���ɫ���ӣ�������ͨ��͸���ȵ��ӡ�\nmask��maskspΪ��ѡһ��ѡ�mask��alphaͨ���ᱻ���ԡ�");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(mask, ptString | ptOptional);
	ADD_PARAM(masksp, ptInteger | ptOptional);
	PROPERTY(mask, masksp, onlyOne);
	ADD_PARAM(stretch, ptBool | ptOptional);
	REG_END();
	REG_SPE_CMD(effect, multiply, EFFECT_MULTIPLY, "ָ��һ�������ͼƬ����Ŀ�꾫��Ļ�Ϸ�ʽΪ��Ƭ���ף�������ͨ��͸���ȵ��ӡ�mask��maskspΪ��ѡһ��ѡ�\nmask��alphaͨ���ᱻ����ԭͼƬ����Ƭ���׺�ͼƬ��alpha��ϡ�\n��Ƭ���ײ��ı�ԭͼƬ��͸���ȡ�");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(mask, ptString | ptOptional);
	ADD_PARAM(masksp, ptInteger | ptOptional);
	PROPERTY(mask, masksp, onlyOne);
	ADD_PARAM(stretch, ptBool | ptOptional);
	REG_END();
	REG_SPE_CMD(effect, blur, EFFECT_BLUR, "ʹĿ�꾫��ģ��");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(radius, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(effect, sharpen, EFFECT_SHARPEN, "ʹĿ�꾫������");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(radius, ptInteger | ptOptional);
	REG_END();
	REG_SPE_CMD(effect, edge, EFFECT_EDGE, "ʹĿ�꾫���Եͻ��");
	ADD_PARAM(target, ptInteger);
	ADD_PARAM(radius, ptInteger | ptOptional);
	REG_END();

	cmd_inited = true;
};
