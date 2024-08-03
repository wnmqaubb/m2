#pragma once
#include <string>

std::wstring string2wstring(std::string str, unsigned int CodePage = 0);
std::string wstring2string(std::wstring wstr, unsigned int CodePage = 0);
std::string from_utf8(std::string str);
std::string from_utf8(std::wstring str);
std::string to_utf8(std::string str);
std::string to_utf8(std::wstring str);
void debug_log(const char *format, ...);

constexpr unsigned int aphash(const char *str)
{
	unsigned int hash = 0;
	for (int i = 0; *str; i++)
	{
		if ((i & 1) == 0)
		{
			hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
		}
		else
		{
			hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
		}
	}
	return (hash & 0x7FFFFFFF);
}
constexpr unsigned int aphash(const unsigned char *str, uint32_t len)
{
	unsigned int hash = 0;
	for (uint32_t i = 0; i < len; i++)
	{
		if ((i & 1) == 0)
		{
			hash ^= ((hash << 7) ^ (str[i]) ^ (hash >> 3));
		}
		else
		{
			hash ^= (~((hash << 11) ^ (str[i]) ^ (hash >> 5)));
		}
	}
	return (hash & 0x7FFFFFFF);
}