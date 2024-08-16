
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Lightbone/utils.h"
#include "Service/SubServicePackage.h"
#include "Service/NetUtils.h"
//#include "Service/AntiCheatClient.h"
//#include "NewClient/loader.h"

extern void __stdcall client_entry() noexcept;
extern void __stdcall UnInit();
using client_entry_t = decltype(&client_entry);
using uninit_t = decltype(&UnInit);
namespace fs = std::filesystem;
//extern void LoadPlugin(CAntiCheatClient* client);
extern uint32_t peload(void* buffer, size_t size, HINSTANCE* instance, void* params);
extern void execute_tls_callback(HINSTANCE instance, uint32_t reason, void* param);
extern void execute_entrypoint(HINSTANCE instance, uint32_t reason, void* param);
extern void enable_seh_on_shellcode();
int main(int argc, char** argv)
{
#ifdef _DEBUG
    auto hmodule = LoadLibraryA("NewClient.dll");
    client_entry_t entry = (client_entry_t)ApiResolver::get_proc_address(hmodule, CT_HASH("client_entry"));
	uninit_t uninit = (uninit_t)ApiResolver::get_proc_address(hmodule, CT_HASH("UnInit"));
	/*share_data_ptr_t param = new share_data_t();
	param->stage = 1;
	ProtocolCFGLoader cfg;
	cfg.set_field(ip_field_id, kDefaultLocalhost);
	cfg.set_field(port_field_id, kDefaultServicePort);
	cfg.set_field(test_mode_field_id, true);
	auto cfg_bin = cfg.dump();
	param->cfg_size = cfg_bin.size();
	memcpy(param->cfg, cfg_bin.data(), std::min(cfg_bin.size(), sizeof(param->cfg)));*/
    entry();
	Sleep(1000*20);
	uninit();
	// 卸载DLL
	BOOL result = FreeLibrary(hmodule);
	if (!result) {
		result = FreeLibrary(hmodule);
		std::cerr << "无法卸载DLL: 再次尝试" << GetLastError() << std::endl;
	}else if (!result) {
		std::cerr << "无法卸载DLL: " << GetLastError() << std::endl;
	}else{
		std::cout << "卸载DLL成功\n";
	}
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
