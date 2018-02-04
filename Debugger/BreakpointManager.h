#pragma once

#include <QtCore/QtCore>
#include "BreakpointInfo.h"

class BreakpointManager : public QObject
{
	Q_OBJECT

private:
	QMap<QString, QMap<int, BreakpointInfo>> breakpoints;

signals:
	void OnBreakpointsChanged(const QMap<QString, QMap<int, BreakpointInfo>> &);

public:
	void Load();
	void Save();
	void Clear();
	void Update(const QString &file, const QMap<int, BreakpointInfo> &infos);
	void Resend();
};