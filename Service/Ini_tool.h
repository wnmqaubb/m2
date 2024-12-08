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
					//log(LOG_TYPE_ERROR, TEXT("�����ļ��ж�Ӧ�������ڻ������Ͳ�ƥ��!"));
					return default_val;
				}
			}
			else
			{
				//log(LOG_TYPE_ERROR, TEXT("�����ļ���δ�ҵ���Ӧ�ı�!"));
				return default_val;
			}
		}
		catch (const toml::parse_error& e)
		{
			//log(LOG_TYPE_ERROR, TEXT("����jishiyu.ini����: "), e.what());
			return default_val;
		}
		catch (const std::exception& e)
		{
			// �����������ܵı�׼������쳣
			//log(LOG_TYPE_ERROR, TEXT("��������: "), e.what());
			return default_val;
		}
		catch (...)
		{
			// ���״�������δ֪�쳣���
			//log(LOG_TYPE_ERROR, TEXT("δ֪������!"));
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
			//log(LOG_TYPE_ERROR, TEXT("����jishiyu.ini����: "), e.what());
			
		}
		catch (const std::exception& e)
		{
			// �����������ܵı�׼������쳣
			//log(LOG_TYPE_ERROR, TEXT("��������: "), e.what());
			
		}
		catch (...)
		{
			// ���״�������δ֪�쳣���
			//log(LOG_TYPE_ERROR, TEXT("δ֪������!"));
			
		}
	}
}