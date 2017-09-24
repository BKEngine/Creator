#include "QPinyin.h"
#include "PinyinHelper.h"

void QPinyin::ExtractPinyinToMap(const std::set<QString> &qs, QHash<QString, QString> &map)
{
	for (auto &&s : qs)
	{
		ExtractPinyinToMap(s, map);
	}
}

void QPinyin::ExtractPinyinToMap(const QStringList &qs, QHash<QString, QString> &map)
{
	for (auto &&s : qs)
	{
		ExtractPinyinToMap(s, map);
	}
}

void QPinyin::ExtractPinyinToMap(const QString &s, QHash<QString, QString> &map)
{
	if (!map.contains(s))
	{
		map.insert(s, s);
		QStringList tmp = ConvertToPinyin(s);
		for (auto &&ss : tmp)
		{
			map.insert(ss, s);
		}
	}
}

QStringList QPinyin::ConvertToPinyin(const QString &s)
{
	QStringList qs;
	qs << QString();
	for (QChar c : s)
	{
		QList<QString> pinyins;
		HanyuPinyinOutputFormat f;
		PinyinHelper::toHanyuPinyinStringArray(c, f, &pinyins);
		if (pinyins.empty())
		{
			for (auto &&ss : qs)
			{
				ss.push_back(c);
			}
		}
		else
		{
			QStringList tmp;
			for (auto &&ss : qs)
			{
				for (auto &&pinyin : pinyins)
				{
					tmp << ss + pinyin;
				}
			}
			qs = tmp;
		}
	}
	return qs;
}

