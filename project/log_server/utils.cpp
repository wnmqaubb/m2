#include <windows.h>
#include <string>

std::wstring string2wstring(std::string str, unsigned int CodePage)
{
	int iBuffSize = ::MultiByteToWideChar(CodePage, 0, str.c_str(), -1, NULL, 0);
	if (iBuffSize > 0)
	{
		std::unique_ptr<wchar_t[]> wszString = std::make_unique<wchar_t[]>(iBuffSize + 1);
		int nChars = ::MultiByteToWideChar(CodePage, 0, str.c_str(), -1, wszString.get(), iBuffSize);
		nChars = nChars < iBuffSize ? nChars : iBuffSize;
		wszString[nChars] = 0;
		return wszString.get();
	}
	return L"";
}

std::string wstring2string(std::wstring wstr, unsigned int CodePage)
{
	int iBuffSize = ::WideCharToMultiByte(CodePage, 0, wstr.c_str(), -1, NULL, 0, NULL, false);
	if (iBuffSize > 0)
	{
		std::unique_ptr<char[]> wszString = std::make_unique<char[]>(iBuffSize + 1);
		::WideCharToMultiByte(CodePage, 0, wstr.c_str(), -1, wszString.get(), iBuffSize, NULL, false);
		return wszString.get();
	}
	return "";
}
std::string from_utf8(std::string str)
{
	return wstring2string(string2wstring(str, CP_UTF8), CP_ACP);
}
std::string to_utf8(std::string str)
{
	return wstring2string(string2wstring(str, CP_ACP), CP_UTF8);
}
std::string to_utf8(std::wstring str)
{
	return wstring2string(str, CP_UTF8);
}
std::string from_utf8(std::wstring str)
{
	return wstring2string(str, CP_ACP);
}

void debug_log(const char *format, ...)
{
	char buffer[1024];
	char buffer_new[1024];
	va_list ap;
	va_start(ap, format);
	_vsnprintf_s(buffer, 1024 - 1, format, ap);
	va_end(ap);
	snprintf(buffer_new, sizeof(buffer_new) - 1, "[LOG_SERVER]%s\n", buffer);
	::OutputDebugStringA(buffer_new);
}
