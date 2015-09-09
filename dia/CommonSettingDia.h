#pragma once

class CommonSettingDia
{
public:
	virtual void save() = 0;

	virtual void load() = 0;

	virtual void reset() = 0;
};