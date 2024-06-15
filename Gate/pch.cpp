<<<<<<< HEAD
﻿// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。

namespace Utils
{
    std::vector<std::wstring> split(const std::wstring& input,
        const std::wstring& regex)
    {
        std::wregex re(regex);
        std::wsregex_token_iterator first{ input.begin(), input.end(), re, -1 }, last;
        return { first, last };
    }
    std::wstring c2w(const std::string& in, unsigned int cp)
    {
        int iBuffSize = ::MultiByteToWideChar(cp, 0, in.c_str(), -1, NULL, 0);
        if (iBuffSize > 0)
        {
            std::unique_ptr<wchar_t[]> wszString = std::make_unique<wchar_t[]>(iBuffSize + 1);
            int nChars = ::MultiByteToWideChar(cp, 0, in.c_str(), -1, wszString.get(), iBuffSize);
            nChars = nChars < iBuffSize ? nChars : iBuffSize;
            wszString[nChars] = 0;
            return wszString.get();
        }
        return L"";
    }

    std::string w2c(const std::wstring& in, unsigned int cp)
    {
        int iBuffSize = ::WideCharToMultiByte(cp, 0, in.c_str(), -1, NULL, 0, NULL, false);
        if (iBuffSize > 0)
        {
            std::unique_ptr<char[]> str = std::make_unique<char[]>(iBuffSize + 1);
            ::WideCharToMultiByte(cp, 0, in.c_str(), -1, str.get(), iBuffSize, NULL, false);
            return str.get();
        }
        return "";
    }
=======
﻿// pch.cpp: 与预编译标头对应的源文件

#include "pch.h"

// 当使用预编译的头时，需要使用此源文件，编译才能成功。

namespace Utils
{
    std::vector<std::wstring> split(const std::wstring& input,
        const std::wstring& regex)
    {
        std::wregex re(regex);
        std::wsregex_token_iterator first{ input.begin(), input.end(), re, -1 }, last;
        return { first, last };
    }
    std::wstring c2w(const std::string& in, unsigned int cp)
    {
        int iBuffSize = ::MultiByteToWideChar(cp, 0, in.c_str(), -1, NULL, 0);
        if (iBuffSize > 0)
        {
            std::unique_ptr<wchar_t[]> wszString = std::make_unique<wchar_t[]>(iBuffSize + 1);
            int nChars = ::MultiByteToWideChar(cp, 0, in.c_str(), -1, wszString.get(), iBuffSize);
            nChars = nChars < iBuffSize ? nChars : iBuffSize;
            wszString[nChars] = 0;
            return wszString.get();
        }
        return L"";
    }

    std::string w2c(const std::wstring& in, unsigned int cp)
    {
        int iBuffSize = ::WideCharToMultiByte(cp, 0, in.c_str(), -1, NULL, 0, NULL, false);
        if (iBuffSize > 0)
        {
            std::unique_ptr<char[]> str = std::make_unique<char[]>(iBuffSize + 1);
            ::WideCharToMultiByte(cp, 0, in.c_str(), -1, str.get(), iBuffSize, NULL, false);
            return str.get();
        }
        return "";
    }
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
}