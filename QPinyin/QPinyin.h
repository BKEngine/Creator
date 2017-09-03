#pragma once

#include <QtCore/QtCore>

class QPinyin
{
public:
	static QStringList ExtractPinyin(const QStringList &qs, QHash<QString, QString> &map);
	static QStringList ConvertToPinyin(const QString &s);
};