#include "BKE_string.h"
#include "BKE_number.h"

//GlobalStringMap StringMap;

BKE_String::BKE_String(const BKE_Number &num)
{
	var = StringMap().allocString(num.tostring());
}

StringType *GlobalStringMap::allocString(const wchar_t *str)
{
	if (str[0] == L'\0')
	{
		//nullString.ref++;
		return &nullString;
	}
	//ref is no use
	auto r = new StringType(str);
	r->hashed=false;
	r->ref = 1;
	return r;
}

StringType *GlobalStringMap::allocHashString(const wchar_t *str)
{
	if (str[0] == L'\0')
	{
		//nullString.ref++;
		return &nullString;
	}
#if PARSER_MULTITHREAD
	mu.lock();
#endif
	auto t = _getNode(str);
	t->ct.first.hashed = true;
	t->ct.first.hash = t->hashvalue;
	t->ct.first.ref++;
#if PARSER_MULTITHREAD
	mu.unlock();
#endif
	return const_cast<StringType*>(&t->ct.first);
}

StringType *GlobalStringMap::allocString(wstring &&str)
{
	if (str[0] == L'\0')
	{
		//nullString.ref++;
		return &nullString;
	}
	auto r = new StringType(std::move(str));
	r->hashed = false;
	r->ref = 1;
	return r;
}

StringType *GlobalStringMap::allocHashString(wstring &&str)
{
	if (str[0] == L'\0')
	{
		//nullString.ref++;
		return &nullString;
	}
#if PARSER_MULTITHREAD
	mu.lock();
#endif
	auto t = _getNode(StringType(std::move(str)));
	t->ct.first.hashed = true;
	t->ct.first.hash = t->hashvalue;
	t->ct.first.ref++;
#if PARSER_MULTITHREAD
	mu.unlock();
#endif
	return const_cast<StringType*>(&t->ct.first);
}

StringType *GlobalStringMap::hashString(StringType &&s)
{
	if (s.str[0]==L'\0')
	{
		//nullString.ref++;
		return &nullString;
	}
	if(s.hashed)
		return &s;
#if PARSER_MULTITHREAD
	mu.lock();
#endif
	auto t = _getNode(std::move(s));
	t->ct.first.hashed = true;
	t->ct.first.hash = t->hashvalue;
	t->ct.first.ref++;
#if PARSER_MULTITHREAD
	mu.unlock();
#endif
	return const_cast<StringType*>(&t->ct.first);
}

bool GlobalStringMap::stripStr(const wstring &s)
{
	StringType st(s.c_str());
	auto it = this->find(st);
	if (it == this->end())
		return true;
	if (it->first.ref > 0)
		return false;
	this->erase(it);
	return true;
}

void GlobalStringMap::forceGC()
{
#if PARSER_MULTITHREAD
	mu.lock();
#endif
	auto it = begin();
	while (it != end())
	{
		if (it->first.ref <= 0)
			it = erase(it);
		else
			++it;
	}
#if PARSER_MULTITHREAD
	mu.unlock();
#endif
}

const wstring& BKE_String::printStr() const
{
	if (var->printStrAvailable)
		return var->printStr;
	bool sim = true;
	wstring s;
	const wchar_t *c = getConstStr().c_str();
	while (*c)
	{
		if (*c<32)
		{
			sim = false;
			break;
		}
		s += *c++;
	}
	if (sim)
	{
		var->printStrAvailable = true;
		var->printStr = L"\"" + s + L"\"";
		return var->printStr;
	}
	c = *var;
	var->printStrAvailable = true;
	var->printStr = L"\'";
#ifdef WIN32
	wchar_t buf[10];
#else
	char buf[10];
#endif
	while (*c)
	{
		if (*c == L'\'')
			var->printStr += L"\\\'";
		else if (*c < 32)
		{
#ifdef WIN32
			swprintf(buf, 10, L"\\x%02X", *c);
			var->printStr += buf;
#else
			snprintf(buf, 10, "\\x%02X", *c);
			char *ptr = buf;
			while (*ptr)
				var->printStr.push_back(*ptr++);
#endif
		}
		else
			var->printStr += *c;
		c++;
	}
	var->printStr += L"\'";
	return var->printStr;
}