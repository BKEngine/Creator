#pragma once

#include <QtCore/QtCore>
#include <set>

class QPinyin
{
public:
	static void ExtractPinyinToMap(const std::set<QString> &qs, QHash<QString, QString> &map);
	static void ExtractPinyinToMap(const QStringList &qs, QHash<QString, QString> &map);
	static void ExtractPinyinToMap(const QString &s, QHash<QString, QString> &map);
	static QStringList ConvertToPinyin(const QString &s);
};