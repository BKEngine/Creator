#include "BreakpointManager.h"

void BreakpointManager::Load()
{
}

void BreakpointManager::Save()
{
}

void BreakpointManager::Clear()
{
	breakpoints.clear();
	Resend();
}

void BreakpointManager::Update(const QString & file, const QMap<int, BreakpointInfo>& infos)
{
	breakpoints[file] = infos;
	Resend();
}

void BreakpointManager::Resend()
{
	emit OnBreakpointsChanged(breakpoints);
}
