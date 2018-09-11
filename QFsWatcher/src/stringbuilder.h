#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

#include <string>
#include <type_traits>
#include <array>
#include <cstdint>

class stringbuilder
{
	std::string buf;
public:
	stringbuilder(size_t capacity = 256) {
		buf.resize(capacity);
	}
	stringbuilder &operator <<(const std::string &s) {
		buf.append(s);
		return *this;
	}
	stringbuilder &operator <<(std::string &&s) {
		buf.append(std::move(s));
		return *this;
	}
	stringbuilder &operator <<(const char *s) {
		buf.append(s);
		return *this;
	}
	stringbuilder &operator <<(int32_t i) {
		char tmp[16];
		sprintf(tmp, "%d", i);
		buf.append(tmp);
		return *this;
	}
	stringbuilder &operator <<(int64_t i) {
		char tmp[24];
		sprintf(tmp, "%I64d", i);
		buf.append(tmp);
		return *this;
	}
	stringbuilder &operator <<(uint32_t i) {
		char tmp[16];
		sprintf(tmp, "%u", i);
		buf.append(tmp);
		return *this;
	}
	stringbuilder &operator <<(uint64_t i) {
		char tmp[24];
		sprintf(tmp, "%I64u", i);
		buf.append(tmp);
		return *this;
	}
	stringbuilder &operator <<(long i) {
		char tmp[24];
		sprintf(tmp, "%ld", i);
		buf.append(tmp);
		return *this;
	}
	stringbuilder &operator <<(unsigned long i) {
		char tmp[24];
		sprintf(tmp, "%lu", i);
		buf.append(tmp);
		return *this;
	}
	stringbuilder &operator <<(float f) {
		char tmp[24];
		sprintf(tmp, "%f", f);
		buf.append(tmp);
		return *this;
	}
	stringbuilder &operator <<(double f) {
		char tmp[24];
		sprintf(tmp, "%f", f);
		buf.append(tmp);
		return *this;
	}
	const std::string &str() const {
		return buf;
	}
	std::string mstr() {
		return std::move(buf);
	}
};

#endif
