#include "EnumsComponent.h"

BKS_EnumsComponent::BKS_EnumsComponent()
	: _wcs2TransEnum({
			{ "crossfade", TRANS_CROSSFADE },
			{ "universal", TRANS_UNIVERSAL },
			{ "scroll", TRANS_SCROLL },
			{ "shutter", TRANS_SHUTTER },
			{ "ripple", TRANS_RIPPLE },
			{ "rotateswap", TRANS_ROTATESWAP },
			{ "rotatevanish", TRANS_ROTATEVANISH },
			{ "rotatezoom", TRANS_ROTATEZOOM },
			{ "wave", TRANS_WAVE },
			{ "turn", TRANS_TURN },
			{ "mosaic", TRANS_MOSAIC },
			{ "normal", TRANS_NORMAL },
			{ "line", TRANS_LINE },
			{ "pageturn", TRANS_PAGETURN },
			{ "burn", TRANS_BURN },
			{ "burn", TRANS_BURN },
	})
	, _wcs2ActionEnum({
			{ "start", ACTION_START },
			{ "parallel", ACTION_PARALLEL },
			{ "queue", ACTION_QUEUE },
			{ "moveby", ACTION_MOVEBY },
			{ "moveto", ACTION_MOVETO },
			{ "fadeto", ACTION_FADETO },
			{ "scaleby", ACTION_SCALEBY },
			{ "scaleto", ACTION_SCALETO },
			{ "rotateby", ACTION_ROTATEBY },
			{ "rotateto", ACTION_ROTATETO },
			{ "tintby", ACTION_TINTBY },
			{ "tintto", ACTION_TINTTO },
			{ "jump", ACTION_JUMP },
			{ "shake", ACTION_SHAKE },
			{ "quake", ACTION_QUAKE },
			{ "visible", ACTION_VISIBLE },
			{ "remove", ACTION_REMOVE },
			{ "flipx", ACTION_FLIPX },
			{ "flipy", ACTION_FLIPY },
			{ "scissorby", ACTION_SCISSORBY },
			{ "scissorto", ACTION_SCISSORTO },
			{ "delay", ACTION_DELAY },
			{ "callback", ACTION_CALLBACK },
			{ "shaky3d", ACTION_SHAKY3D },
			{ "end", ACTION_END },
			{ "scissorcatmullromby", ACTION_SCISSORCATMULLROMBY },
			{ "catmullromby", ACTION_CATMULLROMBY },
			{ "catmullromto", ACTION_CATMULLROMTO },
			{ "timer", ACTION_TIMER },
			{ "delayuntil", ACTION_DELAYUNTIL },
			{ "stretch", ACTION_STRETCH },
			{ "rotatezby", ACTION_ROTATEZBY },
			{ "rotatezto", ACTION_ROTATEZTO },
	})
	, _wcs2AnimateEnum({
			{ "start", ANIMATE_START },
			{ "horizontal", ANIMATE_HORIZONTAL },
			{ "vertical", ANIMATE_VERTICAL },
			{ "multifiles", ANIMATE_MULTIFILES },
			{ "video", ANIMATE_VIDEO },
			{ "cell", ANIMATE_CELL },
			{ "stop", ANIMATE_STOP }
	})
	, _wcs2AnimateLoopEnum({
			{ "none", ANIMATE_LOOP_NONE },
			{ "forward", ANIMATE_LOOP_FORWARD },
			{ "bouncing", ANIMATE_LOOP_BOUNCING }
	})
	, _wcs2EffectEnum({
			{ "delete", EFFECT_DELETE },
			{ "clear", EFFECT_CLEAR },
			{ "scissor", EFFECT_SCISSOR },
			{ "mono", EFFECT_MONO },
			{ "gray", EFFECT_GRAY },
			{ "mask", EFFECT_MASK },
			{ "stencil", EFFECT_STENCIL },
			{ "gamma", EFFECT_GAMMA },
			{ "invert", EFFECT_INVERT },
			{ "BC", EFFECT_BC },
			{ "add", EFFECT_ADD },
			{ "multiply", EFFECT_MULTIPLY },
			{ "blur", EFFECT_BLUR },
			{ "sharpen", EFFECT_SHARPEN },
			{ "edge", EFFECT_EDGE },
			{ "darken", EFFECT_DARKEN },
			{ "colorburn", EFFECT_COLORBURN },
			{ "linearburn", EFFECT_LINEARBURN },
			{ "lighten", EFFECT_LIGHTEN },
			{ "screen", EFFECT_SCREEN },
			{ "colordodge", EFFECT_COLORDODGE },
			{ "lineardodge", EFFECT_LINEARDODGE },
			{ "overlay", EFFECT_OVERLAY },
			{ "Overlay", EFFECT_OVERLAY },
			{ "softlight", EFFECT_SOFTLIGHT },
			{ "hardlight", EFFECT_HARDLIGHT },
			{ "vividlight", EFFECT_VIVIDLIGHT },
			{ "linearlight", EFFECT_LINEARLIGHT },
			{ "pinlight", EFFECT_PINLIGHT },
			{ "hardmix", EFFECT_HARDMIX },
			{ "difference", EFFECT_DIFFERENCE },
			{ "exclusion", EFFECT_EXCLUSION },
			{ "normal", EFFECT_NORMAL },
			{ "fill", EFFECT_FILL },
			{ "oldfilm", EFFECT_OLDFILM },
	})
	, _wcs2TextAlignmentEnum({
		{"left",TA_LEFT},
		{"center",TA_CENTER},
		{"right",TA_RIGHT},
	})
	, _wcs2VerticalTextAlignmentEnum({
		{"top",VTA_TOP},
		{"center", VTA_CENTER},
		{"bottom", VTA_BOTTOM},
	})
	, _wcs2ShapeEnum({
		{"circle", SHAPE_CIRCLE},
		{"rectangle", SHAPE_RECTANGLE},
		{"polygon", SHAPE_POLYGON}
	})
{
}

