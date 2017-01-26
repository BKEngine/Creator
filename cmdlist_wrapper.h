#pragma once

#include <weh.h>
#include <unordered_map>

enum TransEnum : int {
	TRANS_CROSSFADE,
	TRANS_UNIVERSAL,
	TRANS_SCROLL,
	TRANS_SHUTTER,
	TRANS_RIPPLE,
	TRANS_ROTATESWAP,
	TRANS_ROTATEVANISH,
	TRANS_ROTATEZOOM,
	TRANS_WAVE,
	TRANS_TURN,
	TRANS_MOSAIC,
	TRANS_NORMAL,
	TRANS_LINE,
	TRANS_MAX
};

enum ActionEnum : int {
	ACTION_QUEUE,
	ACTION_PARALLEL,
	ACTION_MOVEBY,
	ACTION_MOVETO,
	ACTION_FADETO,
	ACTION_SCALEBY,
	ACTION_SCALETO,
	ACTION_ROTATEBY,
	ACTION_ROTATETO,
	ACTION_TINTBY,
	ACTION_TINTTO,
	ACTION_JUMP,
	ACTION_SHAKE,
	ACTION_QUAKE,
	ACTION_VISIBLE,
	ACTION_REMOVE,
	ACTION_FLIPX,
	ACTION_FLIPY,
	ACTION_SCISSORBY,
	ACTION_SCISSORTO,
	ACTION_DELAY,
	ACTION_CALLBACK,
	ACTION_SHAKY3D,
	ACTION_END,
	ACTION_START,

	ACTION_CATMULLROMBY,
	ACTION_CATMULLROMTO,
	ACTION_SCISSORCATMULLROMBY,
	ACTION_TIMER,
	ACTION_DELAYUNTIL,

	ACTION_MAX
};

enum AnimateLoopEnum : int {
	ANIMATE_LOOP_NONE,
	ANIMATE_LOOP_FORWARD,
	ANIMATE_LOOP_BOUNCING,

	ANIMATE_LOOP_MAX
};

enum EffectEnum : int {
	EFFECT_DELETE,
	EFFECT_CLEAR,

	EFFECT_SCISSOR,
	EFFECT_MONO,
	EFFECT_GRAY,
	EFFECT_MASK,
	EFFECT_STENCIL,
	EFFECT_GAMMA,
	EFFECT_INVERT,
	EFFECT_BC,
	EFFECT_ADD,
	EFFECT_MULTIPLY,
	EFFECT_BLUR,
	EFFECT_BLURX,
	EFFECT_BLURY,
	EFFECT_SHARPEN,
	EFFECT_EDGE,

	EFFECT_DARKEN,
	EFFECT_COLORBURN,
	EFFECT_LINEARBURN,
	EFFECT_LIGHTEN,
	EFFECT_SCREEN,
	EFFECT_COLORDODGE,
	EFFECT_OVERLAY,
	EFFECT_SOFTLIGHT,
	EFFECT_HARDLIGHT,
	EFFECT_VIVIDLIGHT,
	EFFECT_LINEARLIGHT,
	EFFECT_PINLIGHT,
	EFFECT_HARDMIX,
	EFFECT_DIFFERENCE,
	EFFECT_EXCLUSION,
	EFFECT_LINEARDODGE,
	EFFECT_NORMAL,

	EFFECT_MAX
};

enum AnimateEnum : int {
	ANIMATE_START,
	ANIMATE_HORIZONTAL,
	ANIMATE_VERTICAL,
	ANIMATE_MULTIFILES,
	ANIMATE_CELL,
	ANIMATE_STOP,
	ANIMATE_VIDEO,

	ANIMATE_MAX
};

enum bkCmdParamType
{
	ptEverything = 0,
	ptVoid = 1,
	ptInteger = 2,
	ptByte = 4,	//0-255
	ptString = 8,
	ptPos = 0x10,
	ptRect = 0x20,
	ptBool = 0x40,	//non-void
	ptVariable = 0x80,
	ptVol = 0x100,	//0-100
	ptTime = 0x200,	//>=0
	ptIntegerArray = 0x400,
	ptLabelArray = 0x800,
	ptLabel = 0x1000,
	ptExpression = 0x2000,	//expression which has any effect, i.e., not a constant
	ptArray = 0x4000,	//array
	ptScript = 0x8000,	//script filename string, ended with L".bkscr"
	ptDictionary = 0x10000,
	ptColor = 0x20000,	//integer
	ptColorArray = 0x40000,	//array2
	ptStringArray = 0x80000,
	ptNumber = 0x100000,
	ptEvent = 0x200000,
	ptDeprecated = 0x20000000,
	ptOptional = 0x40000000,
	ptConst = 0x80000000,
};

struct BKECmdInfo
{
	QString name;
	QString detail;//描述信息
	QStringList argAutoList;	//对于有限字符串取值的参数，其自动完成列表
	QStringList argNames;	//参数名称
	vector<bkpulong> argFlags;
	//almostOne之类的以后再说……
};

struct BKESpecialCmdInfo
{
	map<QString, pair<int, BKECmdInfo>> modes;
};

extern QHash<QString, BKECmdInfo> CmdList;
extern QHash<QString, BKESpecialCmdInfo> SpecialCmdList;
extern bool cmd_inited;

extern std::unordered_map<std::wstring, int> allEnums;

void initCmd();
