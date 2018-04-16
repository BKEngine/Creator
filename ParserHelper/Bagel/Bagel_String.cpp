#include "Bagel_String.h"
#include "Bagel_Number.h"
#include "Bagel_Utils.h"
#include "Bagel_Var.h"

Bagel_String::~Bagel_String()
{
	if (pool)
		_globalStructures.stringMap->removeString(this);
}

StringVal Bagel_String::printStr() const noexcept
{
	StringVal printStr;
	bool sim = true;
	StringVal s;
	const char16_t *c = getConstStr().c_str();
	while (*c)
	{
		if (*c<32 || *c=='\"')
		{
			sim = false;
			break;
		}
		s += *c++;
	}
	if (sim)
	{
		printStr = W("\"") + s + W("\"");
		return printStr;
	}
	c = getConstStr().c_str();
	printStr = W("\'");
#ifdef WIN32
	wchar_t buf[10];
#else
	char buf[10];
#endif
	while (*c)
	{
		if (*c == '\'')
			printStr += W("\\\'");
		else if (*c < 32)
		{
#ifdef WIN32
			swprintf(buf, 10, L"\\x%02X", *c);
			printStr += (char16_t*)buf;
#else
			snprintf(buf, 10, "\\x%02X", *c);
			char *ptr = buf;
			while (*ptr)
				printStr.push_back(*ptr++);
#endif
		}
		else
			printStr += *c;
		c++;
	}
	printStr += W("\'");
	return printStr;
}

Bagel_StringHolder::Bagel_StringHolder() noexcept : s(_globalStructures.stringMap->nullString)
{
}

Bagel_StringHolder::Bagel_StringHolder(nullptr_t) noexcept : s(_globalStructures.stringMap->nullString)
{
}

Bagel_StringHolder::Bagel_StringHolder(const Bagel_Var & v) noexcept : Bagel_StringHolder(v.asBKEStr())
{
}

Bagel_StringHolder::Bagel_StringHolder(const wchar_t * str) noexcept : s(_globalStructures.stringMap->allocateString(str))
{
}

Bagel_StringHolder::Bagel_StringHolder(const std::wstring & str) noexcept : s(_globalStructures.stringMap->allocateString(str))
{
}

Bagel_StringHolder::Bagel_StringHolder(const char16_t * str) noexcept : s(_globalStructures.stringMap->allocateString(str))
{
}

Bagel_StringHolder::Bagel_StringHolder(const std::u16string & str) noexcept : s(_globalStructures.stringMap->allocateString(str))
{
}

Bagel_StringHolder::Bagel_StringHolder(std::u16string && str) noexcept : s(_globalStructures.stringMap->allocateString(std::move(str)))
{
}

Bagel_StringHolder::Bagel_StringHolder(double num) noexcept : s(_globalStructures.stringMap->allocateString(Bagel_Number::toString(num)))
{
}

Bagel_StringHolder::Bagel_StringHolder(int num) noexcept : s(_globalStructures.stringMap->allocateString(bkpInt2Str(num)))
{
}

