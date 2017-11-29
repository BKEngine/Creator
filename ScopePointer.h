#pragma once

#include <functional>

template<class T>
class ScopePointer
{
	T *p;
	std::function<void(T*)> deleter;
	ScopePointer(const ScopePointer &) = delete;
public:
	ScopePointer(T *p, std::function<void(T*)> &&deleter) :p(p), deleter(std::move(deleter)){}
        ScopePointer(ScopePointer &&scp): p(scp.p), deleter(std::move(scp.deleter)) { scp.deleter = nullptr; }
	void release() { if (deleter) deleter(p); deleter = nullptr; }
	~ScopePointer() { if(deleter) deleter(p); }
	T *operator ->() const { return p; }
	const T &operator *() const { return *p; }
	operator bool() const { return !!p; }
};
