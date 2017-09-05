#include "wordsupport.h"
#include <QDebug>

bool WordSupport::IsNumber(const QString &t)
{
	bool ok;
	t.toInt(&ok, 0);
	if (!ok)
	{
		t.toDouble(&ok);
	}
	return ok;
}

bool WordSupport::IsColor(const QString &t)
{
	int start = 0;
	if (t.startsWith("#"))
	{
		int len = t.length();
		if (len == 3 || len == 5 || len == 7 || len == 9)
		{
			QString s = t.mid(1);
			bool ok;
			t.toUInt(&ok, 16);
			return ok;
		}
		return false;
	}
	else
	{
		bool ok;
		t.toUInt(&ok, 0);
		return ok;
	}
}

bool WordSupport::IsFontColor(const QString &t)
{
	QString s = t.toLower();
	if(s.size()>=2 && s.left(1)==QChar('"') && s.right(1)==QChar('"'))
		s = s.mid(1, s.size()-2);
	if(s.isEmpty())
		return true;
	if (IsColor(s))
		return true;

	int pos = s.indexOf(',');
	if (pos != -1)
	{
		if (!IsColor(s.mid(0, pos).trimmed()))
			return false;
		if (!IsColor(s.mid(pos + 1).trimmed()))
			return false;
		return true;
	}
	return false;
}