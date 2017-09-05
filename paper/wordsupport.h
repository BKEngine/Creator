#ifndef WORDSUPPORT_H
#define WORDSUPPORT_H
#include <QString>

class WordSupport
{
public:
	static bool IsNumber(const QString &t);
	static bool IsColor(const QString &t);
	static bool IsFontColor(const QString &t);
};

#endif // WORDSUPPORT_H
