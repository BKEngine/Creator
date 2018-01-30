#include "BKE_string.h"
#include "BKE_number.h"

//GlobalStringMap StringMap;

#undef new

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
	BKE_WrapperMutexLocker ml(mu);
#endif
	int32_t ha = BKE_hash(str);
	int32_t h = ha & (hashsize - 1);
	auto t = buf[h];
	if (t == NULL)
	{
		//buf[h] = al.op_new(std::forward<T>(key));
		inserttick++;
		t = buf[h] = al.allocate();
		new (buf[h]) BKE_HashNode(str);
		buf[h]->index = h;
		buf[h]->hashvalue = ha;
		buf[h]->next = start.next;
		buf[h]->last = &start;
		start.next->last = buf[h];
		start.next = buf[h];
		count++;
		if (count > hashsize)
		{
			resizeTableSize(hashsize << 1);
		}
	}
	else
	{
		auto node = buf[h];
		while (node && node->index == h)
		{
			if (node->ct.first == str)
				return const_cast<StringType*>(&node->ct.first);
			node = node->next;
		}
		//insert before buf[h]
		//auto newnode = al.op_new(std::forward<T>(key));
		inserttick++;
		auto newnode = al.allocate();
		new (newnode) BKE_HashNode(str);
		t = newnode;
		newnode->index = h;
		newnode->hashvalue = ha;
		newnode->last = buf[h]->last;
		newnode->next = buf[h];
		buf[h]->last->next = newnode;
		buf[h]->last = newnode;
		buf[h] = newnode;
		count++;
		if (count > hashsize)
		{
			resizeTableSize(hashsize << 1);
		}
	}
	t->ct.first.hashed = true;
	t->ct.first.hash = t->hashvalue;
	t->ct.first.ref++;
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
	BKE_WrapperMutexLocker ml(mu);
#endif
	int32_t ha = BKE_hash(str);
	int32_t h = ha & (hashsize - 1);
	auto t = buf[h];
	if (t == NULL)
	{
		//buf[h] = al.op_new(std::forward<T>(key));
		inserttick++;
		t = buf[h] = al.allocate();
		new (buf[h]) BKE_HashNode(std::move(str));
		buf[h]->index = h;
		buf[h]->hashvalue = ha;
		buf[h]->next = start.next;
		buf[h]->last = &start;
		start.next->last = buf[h];
		start.next = buf[h];
		count++;
		if (count > hashsize)
		{
			auto res = buf[h];
			resizeTableSize(hashsize << 1);
		}
	}
	else
	{
		auto node = buf[h];
		while (node && node->index == h)
		{
			if (node->ct.first == str)
				return const_cast<StringType*>(&node->ct.first);
			node = node->next;
		}
		//insert before buf[h]
		//auto newnode = al.op_new(std::forward<T>(key));
		inserttick++;
		auto newnode = al.allocate();
		new (newnode) BKE_HashNode(std::move(str));
		t = newnode;
		newnode->index = h;
		newnode->hashvalue = ha;
		newnode->last = buf[h]->last;
		newnode->next = buf[h];
		buf[h]->last->next = newnode;
		buf[h]->last = newnode;
		buf[h] = newnode;
		count++;
		if (count > hashsize)
		{
			resizeTableSize(hashsize << 1);
		}
	}
	t->ct.first.hashed = true;
	t->ct.first.hash = t->hashvalue;
	t->ct.first.ref++;
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
	BKE_WrapperMutexLocker ml(mu);
#endif
	auto t = _getNode(std::move(s));
	t->ct.first.hashed = true;
	t->ct.first.hash = t->hashvalue;
	t->ct.first.ref++;
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
	BKE_WrapperMutexLocker ml(mu);
#endif
	auto it = begin();
	while (it != end())
	{
		if (it->first.ref <= 0)
			it = erase(it);
		else
			++it;
	}
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
		if (*c<32 || *c=='\"')
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