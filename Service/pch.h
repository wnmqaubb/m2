#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define _HAS_SHARED_MUTEX 1
#include <asio2\asio2.hpp>
#include <asio2\util\uuid.hpp>
#include <msgpack.hpp>
#include <json/json.hpp>
using json = nlohmann::json;

namespace Utils
{
    inline std::wstring c2w(const std::string& in, unsigned int cp = CP_ACP)
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

    inline std::string w2c(const std::wstring& in, unsigned int cp = CP_ACP)
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

    inline std::string to_utf8(const std::string& in)
    {
        return w2c(c2w(in, CP_ACP), CP_UTF8);
    }

    inline std::wstring to_utf8(const std::wstring& in)
    {
        return c2w(w2c(in, CP_ACP), CP_UTF8);
    }

    inline std::string from_utf8(const std::string& in)
    {
        return w2c(c2w(in, CP_UTF8), CP_ACP);
    }

    inline std::wstring from_utf8(const std::wstring& in)
    {
        return c2w(w2c(in, CP_UTF8), CP_ACP);
    }
    inline std::wstring get_current_date_str()
    {
        std::time_t now_time = std::time(nullptr);
        std::tm tm_;
        localtime_s(&tm_, &now_time);
        std::wostringstream oss;
        oss << std::put_time(&tm_, L"%Y_%m_%d");
        return oss.str();
    }

    inline std::wstring get_current_time_str()
    {
        std::time_t now_time = std::time(nullptr);
        std::tm tm_;
        localtime_s(&tm_, &now_time);
        std::wostringstream oss;
        oss << std::put_time(&tm_, L"%H_%M_%S");
        return oss.str();
    }
}


