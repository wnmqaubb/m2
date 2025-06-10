#pragma once
#include <toml.hpp>

namespace IniTool {
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
	template <typename T>
	inline T read_ini(const std::string& path, const std::string& section_name, const std::string& key, const T& default_val = T())
	{
		try
		{
			auto jishiyu_ini = toml::parse_file(path);
			auto& section = jishiyu_ini[section_name];
			if (section.is_table())
			{
				if (section[key]/* && section[key].is<T>()*/)
				{
					return section[key].value<T>().value();
				}
				else
				{
					//log(LOG_TYPE_ERROR, TEXT("配置文件中对应键不存在或者类型不匹配!"));
					return default_val;
				}
			}
			else
			{
				//log(LOG_TYPE_ERROR, TEXT("配置文件中未找到对应的表!"));
				return default_val;
			}
		}
		catch (const toml::parse_error& e)
		{
			//log(LOG_TYPE_ERROR, TEXT("解析jishiyu.ini出错: "), e.what());
			return default_val;
		}
		catch (const std::exception& e)
		{
			// 捕获其他可能的标准库相关异常
			//log(LOG_TYPE_ERROR, TEXT("其他错误: "), e.what());
			return default_val;
		}
		catch (...)
		{
			// 兜底处理其他未知异常情况
			//log(LOG_TYPE_ERROR, TEXT("未知错误发生!"));
			return default_val;
		}
	}

	template <typename T>
	inline void write_ini(const std::string& path, const std::string& section_name, const std::string& key, const T& value)
	{
		try
		{
            toml::table tbl;

            // 如果文件已存在，先读取现有内容
            if (std::filesystem::exists(path)) {
                tbl = toml::parse_file(path);
            }

            // 确保 section 存在
            if (!tbl.contains(section_name)) {
                tbl.insert(section_name, toml::table{});
            }

            // 处理不同类型的数据
            if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*>) {
                std::string str = to_utf8(value);
                tbl[section_name].as_table()->insert_or_assign(key, str);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                std::string str = to_utf8(value);
                tbl[section_name].as_table()->insert_or_assign(key, str);
            }
            else if constexpr (std::is_same_v<T, std::wstring>) {
                std::string str = to_utf8(value);
                tbl[section_name].as_table()->insert_or_assign(key, str);
            }
            else if constexpr (std::is_same_v<T, CString>) {
                std::string str = to_utf8(value);
                tbl[section_name].as_table()->insert_or_assign(key, str);
            }
            else {
                // 其他类型（如 int, float, bool）直接写入
                tbl[section_name].as_table()->insert_or_assign(key, value);
            }

            // 写入文件
            std::ofstream out(path, std::ios::binary);
            if (out) {
                out << tbl;
                out.close();
            }
		}
		catch (const toml::parse_error& e)
		{
			//log(LOG_TYPE_ERROR, TEXT("解析jishiyu.ini出错: "), e.what());
			
		}
		catch (const std::exception& e)
		{
			// 捕获其他可能的标准库相关异常
			//log(LOG_TYPE_ERROR, TEXT("其他错误: "), e.what());
			
		}
		catch (...)
		{
			// 兜底处理其他未知异常情况
			//log(LOG_TYPE_ERROR, TEXT("未知错误发生!"));
			
		}
	}
}