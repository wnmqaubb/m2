
#define WIN32_LEAN_AND_MEAN
#include "pch.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Lightbone/utils.h"
#include "Service/SubServicePackage.h"
#include "Service/NetUtils.h"
#include "asio2/http/http_client.hpp"
#include "asio2/util/base64.hpp"
#include "Service/AntiCheatClient.h"
#include "lf_rungate_server_plug/lf_plug_sdk.h"
#include "NewClient/ShellCode/TaskBasic.h"
//#include "NewClient/loader.h"

extern void __stdcall client_entry(const std::string& guard_gate_ip) noexcept;
extern void __stdcall DoUnInit();
using client_entry_t = decltype(&client_entry);
using uninit_t = decltype(&DoUnInit);
namespace fs = std::filesystem;
//extern void LoadPlugin(CAntiCheatClient* client);
extern uint32_t peload(void* buffer, size_t size, HINSTANCE* instance, void* params);
extern void execute_tls_callback(HINSTANCE instance, uint32_t reason, void* param);
extern void execute_entrypoint(HINSTANCE instance, uint32_t reason, void* param);
extern void enable_seh_on_shellcode();
extern asio::io_service g_game_io;
void init_client_entry();
std::string read_config_txt(const std::filesystem::path& path, const std::string& section, const std::string& key);
void test_task_basic_dll(fs::path path);

int main(int argc, char** argv)
{	
	init_client_entry();

	//Utils::ImageProtect::instance().unmap_image(GetModuleHandleA(nullptr));
	/*char m_ExeDir[MAX_PATH];
	GetModuleFileNameA(NULL, m_ExeDir, sizeof(m_ExeDir));
	auto ini_path = std::filesystem::path(m_ExeDir).parent_path() / "Config.ini";
	std::string value = read_config_txt(ini_path, "GuardGate", "GateIP");
	if (!value.empty()) {
		std::cout << "ServerAddr 的值为：" << value << std::endl;
	}
	else {
		std::cout << "未找到指定键的值。" << std::endl;
	}*/
	
	//fs::path path("d:\\");
	//test_task_basic_dll(path);
    getchar();
    std::cout << "Hello World!\n";
}

void init_client_entry() {
	auto hmodule = LoadLibraryA("NewClient.dll");
	client_entry_t entry = (client_entry_t)ApiResolver::get_proc_address(hmodule, CT_HASH("client_entry"));
	uninit_t uninit = (uninit_t)ApiResolver::get_proc_address(hmodule, CT_HASH("DoUnInit"));
	//entry("43.139.236.115");
	entry("");
	//Sleep(15000);
	//uninit();
	/*if(FreeLibrary(hmodule))
		std::cout << "FreeLibrary ok!\n";*/
}

void test_task_basic_dll(fs::path path) {
	std::ifstream file(path.parent_path() / "taskbasic.dll", std::ios::in | std::ios::binary);
	if (file.is_open())
	{
		std::cout << "start running\n";
		std::stringstream ss;
		ss << file.rdbuf();
		xor_buffer(ss.str().data(), ss.str().size(), kProtocolXorKey);

		/*std::ofstream file1("d:\\work\\Repos\\M2\\taskbasic1.dll", std::ios::out | std::ios::binary);
		file1.write(ss.str().data(), ss.str().size());
		file1.close();*/

		//plugin_handle = dll_base;
		auto a = peload(ss.str().data(), sizeof(IMAGE_DOS_HEADER), nullptr, NULL);
		if (a == ERROR_SUCCESS)
		{
			std::cout << "start running 11\n";
			enable_seh_on_shellcode();
			//execute_tls_callback(plugin_handle, DLL_PROCESS_ATTACH, 0);
			//execute_entrypoint(plugin_handle, DLL_PROCESS_ATTACH, 0);
			//instance_->log(0, TEXT("CClientPluginMgr::load_plugin 加载插件>>> 3"));
			//decltype(&LoadPlugin) plugin_entry = (decltype(&LoadPlugin))ApiResolver::get_proc_address(plugin_handle, ApiResolver::hash("LoadPlugin"));
			//OutputDebugStringA((std::to_string(*(int*)plugin_entry) + "<<< LoadPlugin").c_str());
			std::cout << "start running 22\n";
		}
		std::cout << "start running ok\n";
	}
	else
	{
		std::cout << "client.bin not exist\n";
	}
}

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> readIniFile(const std::filesystem::path& path) {
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> iniData;
	std::ifstream file(path);
	if (file.is_open()) {
		std::string section;
		std::string line;
		while (std::getline(file, line)) {
			// 去除行首尾的空白字符
			line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
			if (line.empty() || line[0] == ';') continue;
			if (line[0] == '[' && line.back() == ']') {
				section = line.substr(1, line.length() - 2);
			}
			else {
				auto pos = line.find('=');
				if (pos != std::string::npos) {
					std::string key = line.substr(0, pos);
					std::string value = line.substr(pos + 1);
					iniData[section][key] = value;
				}
			}
		}
		file.close();
	}
	else {
		std::cerr << "无法打开文件 " << path << std::endl;
	}
	return iniData;
}

std::string read_config_txt(const std::filesystem::path& path, const std::string& section, const std::string& key) {
	setlocale(LC_CTYPE, "");
	auto iniData = readIniFile(path);
	if (iniData.find(section) != iniData.end() && iniData[section].find(key) != iniData[section].end()) {
		return iniData[section][key];
	}
	else {
		return "";
	}
}