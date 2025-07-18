// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "framework.h"
#include <afxcontrolbars.h>

#define _HAS_SHARED_MUTEX 1
#include <asio2\asio2.hpp>
#include <asio2\util\uuid.hpp>
#include <msgpack.hpp>
#include <asio\detail\thread_group.hpp>
#include <nlohmann\json.hpp>
#include <iostream>
#include <afxext.h>
#include <afxext.h>
#include <afxcontrolbars.h>
#include <afxcontrolbars.h>
#include <afxcontrolbars.h>

using json = nlohmann::json;

namespace Utils
{
    std::vector<std::wstring> split(const std::wstring& input,
        const std::wstring& regex);

    std::wstring c2w(const std::string& in, unsigned int cp = CP_ACP);

    std::string w2c(const std::wstring& in, unsigned int cp = CP_ACP);

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
}


#endif //PCH_H
