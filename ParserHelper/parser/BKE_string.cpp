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
	t->key.hashed = true;
	t->key.hash = t->hashvalue;
	t->key.ref++;
#if PARSER_MULTITHREAD
	mu.unlock();
#endif
	return &t->key;
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
	//rewrite hash to accept move param
	bkplong ha = BKE_hash(str);
	bkplong h = ha & (hashsize - 1);
	if (buf[h] == NULL)
	{
		buf[h] = al.op_new();
		buf[h]->key.hash = ha;
		buf[h]->key.hashed = true;
		buf[h]->key.str = std::move(str);
		buf[h]->key.ref = 1;
		buf[h]->index = h;
		buf[h]->last = NULL;
		buf[h]->next = NULL;
		buf[h]->hashvalue = ha;
		count++;
#if PARSER_MULTITHREAD
		mu.unlock();
#endif
		return &buf[h]->key;
	}
	else
	{
		BKE_HashNode *node = buf[h];
		if (node->key.str == str)
		{
			node->key.ref++;
#if PARSER_MULTITHREAD
			mu.unlock();
#endif
			return &node->key;
		}
		while (node->next != NULL)
		{
			node = node->next;
			if (node->key.str == str)
			{
				node->key.ref++;
#if PARSER_MULTITHREAD
				mu.unlock();
#endif
				return &node->key;
			}
		}
		node->next = al.op_new();
		node->next->key.hash = ha;
		node->next->key.hashed = true;
		node->next->key.str = std::move(str);
		node->next->key.ref = 1;
		node->next->index = h;
		node->next->last = node;
		node->next->next = NULL;
		node->next->hashvalue = ha;
		count++;
#if PARSER_MULTITHREAD
		mu.unlock();
#endif
		return &node->next->key;
	}
}

StringType *GlobalStringMap::hashString(StringType &&s)
{
	if (s.str[0]==L'\0')
	{
		//nullString.ref++;
		return &nullString;
	}
#if PARSER_MULTITHREAD
	mu.lock();
#endif
	//rewrite hash to accept move param
	bkplong ha = BKE_hash(s.str);
	bkplong h = ha & (hashsize - 1);
	if (buf[h] == NULL)
	{
		buf[h] = al.op_new();
		buf[h]->key = std::move(s);
		buf[h]->key.hash = ha;
		buf[h]->index = h;
		buf[h]->last = NULL;
		buf[h]->next = NULL;
		buf[h]->hashvalue = ha;
		count++;
#if PARSER_MULTITHREAD
		mu.unlock();
#endif
		return &buf[h]->key;
	}
	else
	{
		BKE_HashNode *node = buf[h];
		if (node->key.str == s.str)
		{
			node->key.ref++;
#if PARSER_MULTITHREAD
			mu.unlock();
#endif
			return &node->key;
		}
		while (node->next != NULL)
		{
			node = node->next;
			if (node->key.str == s.str)
			{
				node->key.ref++;
#if PARSER_MULTITHREAD
				mu.unlock();
#endif
				return &node->key;
			}
		}
		node->next = al.op_new();
		node->next->key = std::move(s);
		node->next->key.hash = ha;
		node->next->index = h;
		node->next->last = node;
		node->next->next = NULL;
		node->next->hashvalue = ha;
		count++;
#if PARSER_MULTITHREAD
		mu.unlock();
#endif
		return &node->next->key;
	}
}

void GlobalStringMap::forceGC()
{
#if PARSER_MULTITHREAD
	mu.lock();
#endif
	for (int i = 0; i<hashsize; i++)
	{
		auto p = buf[i];
		while (p != NULL)
		{
			BKE_HashNode *node = p;
			p = node->next;
			if (node->key.ref <= 0)
			{
				if (node->next)
					node->next->last =node->last;
				if (node->last)
					node->last->next = node->next;
				else
					buf[node->index] = node->next;
				al.op_delete(node);
				count--;
			}
		}
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
	wchar_t buf[10];
	while (*c)
	{
		if (*c == L'\'')
			var->printStr += L"\\\'";
		else if (*c < 32)
		{
			swprintf(buf, 10, L"\\x%02X", *c);
			var->printStr += buf;
		}
		else
			var->printStr += *c;
		c++;
	}
	var->printStr += L"\'";
	return var->printStr;
}