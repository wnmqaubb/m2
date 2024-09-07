#include "pch.h"
#include <iostream>
#include "Lightbone/lighthook.h"
#include "Lightbone/pointer.hpp"
#include "Lightbone/utils.h"
#include "NewClient/ClientImpl.h"
#include "Gate/cmdline.h"
#include "PEScan.h"

void async_execute_javascript(const std::string& sv, uint32_t script_id);

void __stdcall client_entry(const std::string& ip);

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
    client_entry("127.0.0.1");
}
//
void test_javascript()
{
    client_entry("127.0.0.1");
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
    //hook_calc_pe_ico_hash();
}

int main(int argc, char** argv)
{
    setlocale(LC_CTYPE, "");

    InitUnitTest();
    
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
                test_javascript();
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

    cmd_handler(args);

    std::string cmd;
    while (std::getline(std::cin, cmd))
    {
        cmd_handler(split(cmd, " "));
    }
    getchar();
    return 0;
}
