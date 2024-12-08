#pragma once
#include <toml.hpp>

namespace IniTool {
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
	inline void write_ini(const std::string& path, const std::string& section_name, const std::string& key, const T& default_val)
	{
		try
		{
			auto jishiyu_ini = toml::parse_file(path);
			auto& section = jishiyu_ini[section_name];
			if (section.is_table())
			{
				section.as_table()->insert_or_assign(std::move(key), std::move(default_val));
			}

			std::ofstream output(path, std::ios::out | std::ios::binary);
			output << jishiyu_ini;
			output.close();
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