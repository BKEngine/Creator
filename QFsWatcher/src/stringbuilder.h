#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

#include <string>
#include <type_traits>
#include <charconv>
#include <array>

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
	template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
	stringbuilder &operator <<(T t) {
		std::array<char, 32> str;
		if (auto[p, ec] = std::to_chars(str.data(), str.data() + str.size(), 42);
		ec == std::errc())
			buf.append(str.data(), p - str.data());
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
