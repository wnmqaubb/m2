
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
#include "NewClient/lf_plug_sdk.h"
//#include "NewClient/loader.h"

extern void __stdcall client_entry(lfengine::client::PAppFuncDef AppFunc) noexcept;
extern void __stdcall DoUnInit();
using client_entry_t = decltype(&client_entry);
using uninit_t = decltype(&DoUnInit);
namespace fs = std::filesystem;
//extern void LoadPlugin(CAntiCheatClient* client);
extern uint32_t peload(void* buffer, size_t size, HINSTANCE* instance, void* params);
extern void execute_tls_callback(HINSTANCE instance, uint32_t reason, void* param);
extern void execute_entrypoint(HINSTANCE instance, uint32_t reason, void* param);
extern void enable_seh_on_shellcode();
struct IniSection {
	std::string name;
	std::unordered_map<std::string, std::string> keyValuePairs;
};

IniSection parseSection(std::ifstream& file) {
	IniSection section;

	std::string line;
	std::getline(file, line); // 读取节名
	if (line.empty() || line[0] != '[') {
		return section; // 如果不是有效的节名，则返回空节
	}

	section.name = line.substr(1, line.size() - 2); // 去掉方括号

	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '[') {
			break; // 如果遇到下一个节或空行，则结束当前节
		}

		size_t pos = line.find('=');
		if (pos == std::string::npos) {
			continue; // 忽略无效的行
		}

		std::string key = line.substr(0, pos);
		std::string value = line.substr(pos + 1);

		// 去除首尾空白字符
		key.erase(0, key.find_first_not_of(" \t"));
		key.erase(key.find_last_not_of(" \t") + 1);
		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t") + 1);

		section.keyValuePairs[key] = value;
	}

	return section;
}

std::unordered_map<std::string, IniSection> parseIniFile(const std::string& filename) {
	std::ifstream configFile(filename);
	if (!configFile.is_open()) {
		std::cerr << "Failed to open config file." << std::endl;
		return {};
	}

	std::unordered_map<std::string, IniSection> sections;
	IniSection currentSection;

	while (!configFile.eof()) {
		currentSection = parseSection(configFile);
		if (!currentSection.name.empty()) {
			sections[currentSection.name] = currentSection;
		}
	}

	configFile.close();
	return sections;
}
int main(int argc, char** argv)
{
#ifndef _DEBUG
	auto hmodule = LoadLibraryA("NewClient.dll");
	client_entry_t entry = (client_entry_t)ApiResolver::get_proc_address(hmodule, CT_HASH("client_entry"));
	uninit_t uninit = (uninit_t)ApiResolver::get_proc_address(hmodule, CT_HASH("DoUnInit"));
	///*share_data_ptr_t param = new share_data_t();
	//param->stage = 1;
	//ProtocolCFGLoader cfg;
	//cfg.set_field(ip_field_id, kDefaultLocalhost);
	//cfg.set_field(port_field_id, kDefaultServicePort);
	//cfg.set_field(test_mode_field_id, true);
	//auto cfg_bin = cfg.dump();
	//param->cfg_size = cfg_bin.size();
	//memcpy(param->cfg, cfg_bin.data(), std::min(cfg_bin.size(), sizeof(param->cfg)));*/
	entry(nullptr);
	//Sleep(1000 * 15);
	//uninit();
	// 卸载DLL
	/*BOOL result = FreeLibrary(hmodule);
	if (!result) {
		result = FreeLibrary(hmodule);
		std::cerr << "无法卸载DLL: 再次尝试" << GetLastError() << std::endl;
	}
	else {
		std::cout << "卸载DLL成功\n";
	}*/
	//std::string_view url = R"(43.139.236.115:5178)";

	//const std::string filename = "D:\\work\\Repos\\M2\\build\\bin\\Debug\\Win32\\Config.ini";
	//auto iniSections = parseIniFile(filename);

	//// 查找 ServerAddr 的值
	//auto gameGateSection = iniSections.find("GameGate");
	//if (gameGateSection != iniSections.end()) {
	//	auto serverAddr = gameGateSection->second.keyValuePairs.find("ServerAddr");
	//	if (serverAddr != gameGateSection->second.keyValuePairs.end()) {
	//		std::cout << "Server Address: " << serverAddr->second << std::endl;
	//	}
	//	else {
	//		std::cerr << "ServerAddr not found in GameGate section." << std::endl;
	//	}
	//}
	//else {
	//	std::cerr << "GameGate section not found in INI file." << std::endl;
	//}


	
	
#else
    fs::path path(argv[0]);
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
#endif
    getchar();
    std::cout << "Hello World!\n";
}
