#include "pch.h"
#include <iostream>
#include "../../yk/Lightbone/lighthook.h"
#include "../../yk/Lightbone/pointer.hpp"
#include "../../yk/Lightbone/utils.h"
#include "NewClient/ClientImpl.h"
#include "NewClient/shellcode/TaskBasic.h"
#include "Gate/cmdline.h"
#include "PEScan.h"
#include <../../yk/Lightbone/api_resolver.h>
#include <NewClient/thread_manager.h>
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")
void async_execute_javascript(const std::string& sv, uint32_t script_id);

//extern void __stdcall InitExFunc(std::string guard_gate_ip) noexcept;
//typedef void(__stdcall* InitExFunc)(const void* AppFunc, const char* server_ip, const wchar_t* mutex_anti_cheat_module_name);
//extern void __stdcall DoUnInit();
//using client_entry_t = decltype(&InitEx);
//using uninit_t = decltype(&DoUnInit);
//client_entry_t pfnAntiCheatInit = nullptr;

void hook_calc_pe_ico_hash()
{
    LightHook::HookMgr::instance().add_inline_hook(&Utils::PEScan::calc_pe_ico_hash, &calc_pe_ico_hash, nullptr);
}


decltype(&async_execute_javascript) async_execute_javascript_trampoline = nullptr;
void async_execute_javascript_dump(const std::string& sv, uint32_t script_id)
{
    std::cout << "执行脚本:" << script_id << std::endl;
    std::filesystem::create_directories(".\\scripts");
    std::ofstream script(".\\scripts\\" + std::to_string(script_id) + ".js", std::ios::out);
    script << sv;
    async_execute_javascript_trampoline(sv, script_id);
}

void dump_all_scripts()
{
    LightHook::HookMgr::instance().add_inline_hook(&async_execute_javascript, &async_execute_javascript_dump, &async_execute_javascript_trampoline);
}

void test_connect()
{
    InitEx(nullptr, "127.0.0.1", L"mutex_anti_cheat_module_name");
}
//
void test_javascript()
{
    InitEx(nullptr, "127.0.0.1", L"mutex_anti_cheat_module_name");
}

std::vector<std::string> split(const std::string &str, const std::string &pattern)
{
    std::vector<std::string> res;
    if (str == "")
        return res;
    //在字符串末尾也加入分隔符，方便截取最后一段
    std::string strs = str + pattern;
    std::size_t pos = strs.find(pattern);

    while (pos != strs.npos)
    {
        std::string temp = strs.substr(0, pos);
        res.push_back(temp);
        //去掉已分割的字符串,在剩下的字符串中进行分割
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(pattern);
    }

    return res;
}

void InitUnitTest()
{
    InitEx(nullptr, "127.0.0.1", L"mutex_anti_cheat_module_name");
    //hook_calc_pe_ico_hash();
}

void init_client_entry_dll() {
    //auto hmodule = LoadLibraryA("NewClient_f.dll");
    //client_entry_t pfnAntiCheatInit = (client_entry_t)ApiResolver::get_proc_address(hmodule, CT_HASH("InitEx"));
    ////uninit_t uninit = (uninit_t)ApiResolver::get_proc_address(hmodule, CT_HASH("DoUnInit"));
    ////entry("43.139.236.115");
    //if  (!pfnAntiCheatInit)
    //    std::cout << "pfnAntiCheatInit is null\n";
    //else
    //    pfnAntiCheatInit(nullptr, "127.0.0.1", L"mutex_anti_cheat_module_name");
    ///*Sleep(30000);
    //uninit();
    //if (FreeLibrary(hmodule))
    //    std::cout << "FreeLibrary ok!\n";*/
}

int main(int argc, char** argv)
{
    setlocale(LC_CTYPE, "");
    //init_client_entry_dll();
    InitUnitTest();
   
    InitJavaScript();
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) args.push_back(argv[i]);
    
    auto cmd_handler = [](std::vector<std::string>& cmd) {
        cmdline::parser a;
        a.add("js");
        a.add("connect");
        a.parse(cmd);
        if (a.get_program_name() == "js")
        {
            a.add<std::string>("path", 'p');
            a.parse(cmd);
            auto path = a.get<std::string>("path");
            std::ifstream file(path, std::ios::in);
            std::stringstream ss;
            ss << file.rdbuf();
			static bool is_init = false;
			if (!is_init)
			{
                //test_javascript();
				is_init = true;
			}
            std::cout << "async_execute_javascript" << std::endl;
            async_execute_javascript(ss.str(), 0);
            std::cout << "async_execute_javascript end" << std::endl;
        }
        else if (a.get_program_name() == "connect")
        {
            test_connect();
        }
        else
        {
            std::cerr << a.usage() << std::endl;
        }
    };

    //cmd_handler(args);

    std::string cmd;
    while (std::getline(std::cin, cmd))
    {
        cmd_handler(split(cmd, " "));
    }
    getchar();
    return 0;
}
