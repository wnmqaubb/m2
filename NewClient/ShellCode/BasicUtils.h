#pragma once

namespace BasicUtils
{
    void __declspec(noinline) init_heartbeat_check(HWND hwnd);
    std::tuple<std::string, std::string> scan_tcp_table(std::vector<std::tuple<std::string, std::string>> black_ip_table);
    WNDPROC set_old_wnd_proc(WNDPROC routine);
    void __declspec(noinline) infinite_exit();
    uint32_t read_virtual_memory(
        HANDLE handle,
        uint64_t base_address,
        PVOID buffer,
        uint64_t buffer_size,
        uint32_t* bytes_of_read);
    bool calc_pe_ico_hash(std::wstring path, uint32_t* hash_val);
    std::string get_mac_by_ip(std::string &gateway_ip);
    std::vector<std::pair<std::string, std::string>> get_gateway_ip_macs();
	uint32_t get_parent_process_id();
    std::map<uint32_t, std::tuple<std::wstring, uint32_t, uint32_t>> enum_memory(uint32_t phandle);
}

