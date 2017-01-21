#pragma once

#include "../ScriptEnums.h"
#include <unordered_map>
#include <map>
#include <string>

class BKS_EnumsComponent
{
	friend class BKE_ImageSprite;
protected:
	BKS_EnumsComponent();

	std::unordered_map<std::string, TransEnum> _wcs2TransEnum;

	std::unordered_map<std::string, ActionEnum>  _wcs2ActionEnum;

	std::unordered_map<std::string, AnimateEnum> _wcs2AnimateEnum;

	std::map<std::string, AnimateLoopEnum> _wcs2AnimateLoopEnum;

	std::unordered_map<std::string, EffectEnum> _wcs2EffectEnum;

	std::map<std::string, TextAlignmentEnum> _wcs2TextAlignmentEnum;

	std::map<std::string, VerticalTextAlignmentEnum> _wcs2VerticalTextAlignmentEnum;

	std::map<std::string, ShapeEnum> _wcs2ShapeEnum;
};
