#include "QPinyin.h"
#include "PinyinHelper.h"

QStringList QPinyin::ExtractPinyin(const QStringList &qs, QHash<QString, QString> &map)
{
	QStringList pinyins;
	for (auto &&s : qs)
	{
		map[s] = s;
		QStringList tmp = ConvertToPinyin(s);
		pinyins << tmp;
		for (auto &&ss : tmp)
		{
			map[ss] = s;
		}
	}
	return pinyins;
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

