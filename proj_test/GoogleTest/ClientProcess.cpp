#include <Windows.h>
#include <iostream>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "../Lightbone/utils.h"
#include "../Service/SubServicePackage.h"
#include "../Service/NetUtils.h"
#include "../Tools/Packer/loader.h"
#include "asio2/util/sha1.hpp"
#include "asio2/util/base64.hpp"
#include "../Gate/cmdline.h"
#include "../Service/Ini_tool.h"
#include <vector>

using client_entry_t = decltype(&client_entry);

int main(int argc, char* argv[]) {

    // 实际应使用命令行解析库，此处简化处理
    if (argc > 1) {
        // 解析--player-id等参数
    }

    // 加载DLL
    HMODULE hDll = LoadLibraryA("NewClient.dll");
    if (!hDll) {
        std::cerr << "Failed to load NewClient.dll: " << GetLastError() << std::endl;
        return 1;
    }

    // 获取入口函数
    auto entry = reinterpret_cast<client_entry_t>(GetProcAddress(hDll, "client_entry"));
    if (!entry) {
        std::cerr << "Failed to find ClientEntry: " << GetLastError() << std::endl;
        FreeLibrary(hDll);
        return 1;
    }

    // 执行客户端逻辑
    try {
        share_data_ptr_t param = new share_data_t();
        param->stage = 1;
        ProtocolCFGLoader cfg;
        //cfg.set_field(ip_field_id, kDefaultLocalhost);
        cfg.set_field(ip_field_id, "140.210.20.215");
        cfg.set_field(port_field_id, kDefaultServicePort);
        cfg.set_field(test_mode_field_id, false);
        auto cfg_bin = cfg.dump();
        param->cfg_size = cfg_bin.size();
        memcpy(param->cfg, cfg_bin.data(), std::min(cfg_bin.size(), sizeof(param->cfg)));
        entry(param);
    }
    catch (...) {
        std::cerr << "Client execution failed" << std::endl;
    }

    // 清理
    FreeLibrary(hDll);
    return 0;
}
