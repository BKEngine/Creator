#pragma once

#include <string>

int ConvertSingleUTF8Character(wchar_t &dst, const unsigned char *src, unsigned int size, unsigned int &bytes_used);

//获取UTF8转换为Unicode的字符数量（注意不是内存大小）。不包含terminate字符（'\0'）。
unsigned int UniFromUTF8_Size(const char *str, unsigned int len);
inline unsigned int UniFromUTF8_Size(const std::string &str){return UniFromUTF8_Size(str.c_str(), (unsigned int)str.size());}
//获取UTF8转换为Unicode的std字符串。
std::wstring UniFromUTF8(const char *str, unsigned int len);
inline std::wstring UniFromUTF8(const std::string &str){return UniFromUTF8(str.c_str(), (unsigned int)str.size());}
//将UTF8转换为Unicode。不会自动添加terminate字符（'\0'）。
void UniFromUTF8(wchar_t *dst, const char *str, unsigned int srclen);

bool IsValidUTF8(const char *str, unsigned int len);
inline bool IsValidUTF8(const std::string &str){ return IsValidUTF8(str.c_str(), (unsigned int)str.size()); }

//获取Unicode转换为UTF8的字符数量（注意不是内存大小）。不包含terminate字符（'\0'）。
unsigned int UniToUTF8_Size(const wchar_t *str, unsigned int len);
inline unsigned int UniToUTF8_Size(const std::wstring &str){return UniToUTF8_Size(str.c_str(), (unsigned int)str.size());}
//获取Unicode转换为UTF8的std字符串。
std::string UniToUTF8(const wchar_t *str, unsigned int len, bool addBOM = 0);
inline std::string UniToUTF8(const std::wstring &str, bool addBOM = 0){return UniToUTF8(str.c_str(), (unsigned int)str.size());}
//将Unicode转换为UTF8。不会自动添加terminate字符（'\0'）。
void UniToUTF8(char *dst, const wchar_t *str, unsigned int len);

//获取UTF16转换为Unicode的字符数量（注意不是内存大小）。不包含terminate字符（'\0'）。
unsigned int UniFromUTF16_Size(const unsigned short *str, unsigned int len);
//获取UTF16转换为Unicode的std字符串。
std::wstring UniFromUTF16(const unsigned short *str, unsigned int len);
//将UTF16转换为Unicode。不会自动添加terminate字符（'\0'）。
bool UniFromUTF16(wchar_t *dst, const unsigned short *str, unsigned int len);

//获取Unicode转换为UTF16的字符数量（注意不是内存大小）。不包含terminate字符（'\0'）。
unsigned int UniToUTF16_Size(const wchar_t *str, unsigned int len);
inline unsigned int UniToUTF16_Size(const std::wstring &str){return UniToUTF16_Size(str.c_str(), (unsigned int)str.size());}
//将Unicode转换为UTF16。不会自动添加terminate字符（'\0'）。
void UniToUTF16(unsigned short *dst, const wchar_t *str, unsigned int len);
inline void UniToUTF16(unsigned short *dst, const std::wstring &str){return UniToUTF16(dst, str.c_str(), (unsigned int)str.size());}

//获取Unicode转换为UTF7的字符数量（注意不是内存大小）。不包含terminate字符（'\0'）。
inline unsigned int UniToUTF7_Size(const wchar_t *str, unsigned int len){ (void)str; return len; }
inline unsigned int UniToUTF7_Size(const std::wstring &str){ return UniToUTF7_Size(str.c_str(), (unsigned int)str.size()); }
//将Unicode转换为UTF7。不会自动添加terminate字符（'\0'）。
inline void UniToUTF7(char *dst, const wchar_t *str, unsigned int len)
{
	while (len--)
	{
		*dst++ = (char)*str++;
	}
}
inline std::string UniToUTF7(const wchar_t *str, unsigned int len)
{
	std::string res;
	res.resize(UniToUTF8_Size(str, len));
	UniToUTF7(&res[0], str, len);
	return std::move(res);
}
inline std::string UniToUTF7(const std::wstring &str, bool addBOM = 0){ return UniToUTF7(str.c_str(), (unsigned int)str.size()); }


//获取UTF7转换为Unicode的字符数量（注意不是内存大小）。不包含terminate字符（'\0'）。
inline unsigned int UniFromUTF7_Size(const char *str, unsigned int len){ (void)str; return len; }
inline unsigned int UniFromUTF7_Size(const std::string &str){ return UniFromUTF7_Size(str.c_str(), (unsigned int)str.size()); }
//获取UTF7转换为Unicode的std字符串。
inline void UniFromUTF7(wchar_t *dst, const char *str, unsigned int len)
{
	while (len--)
	{
		*dst++ = (wchar_t)*str++;
	}
}
inline std::wstring UniFromUTF7(const char *str, unsigned int len)
{
	std::wstring res;
	res.resize(UniFromUTF7_Size(str, len));
	UniFromUTF7(&res[0], str, len);
	return std::move(res);
}
inline std::wstring UniFromUTF7(const std::string &str){ return UniFromUTF7(str.c_str(), (unsigned int)str.size()); }
//将UTF8转换为Unicode。不会自动添加terminate字符（'\0'）。



