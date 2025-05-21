
#define WIN32_LEAN_AND_MEAN
#define __cplusplus
#define _MSC_EXTENSIONS
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "Lightbone/utils.h"
#include "Service/SubServicePackage.h"
#include "Service/NetUtils.h"
#include "Tools/Packer/loader.h"
#include "asio2/util/sha1.hpp"
#include "asio2/util/base64.hpp"
#include "Gate/cmdline.h"
#include "Service/Ini_tool.h"
#include <vector>

using client_entry_t = decltype(&client_entry);
namespace fs = std::filesystem;
void on_recv_pkg_policy(ProtocolS2CPolicy& req);
void get_public_ip();
std::wstring GetProcessPath(DWORD pid);
bool decode_plugin(std::string filename);
int main(int argc, char** argv)
{
#ifndef _DEBUG
    //decode_plugin("D:\\work\\mir\\竞品\\裁决网关\\plugin\\TaskBasic.dll");
    //decode_plugin("D:\\work\\Repos\\yk\\build\\bin\\Release\\Win32\\plugin\\TaskBasic.dll");
    //decode_plugin("D:\\work\\temp\\2\\cache\\2A6E4A2B");
	//get_public_ip();
	//while (true)
	//{
	//	ProtocolS2CPolicy req;
	//	std::ifstream file("D:\\work\\Repos\\yk\\build\\bin\\及时雨封装内部版\\config.cfg", std::ios::in | std::ios::binary);
	//	if (file.is_open())
	//	{
	//		std::stringstream ss;
	//		ss << file.rdbuf();
	//		auto str = ss.str();
	//		req = *ProtocolS2CPolicy::load(str.data(), str.size());
	//		file.close();
	//	}
	//	std::cout << "开始运行" << std::endl;
	//	auto start = std::chrono::steady_clock::now(); // 记录开始时间
	//	on_recv_pkg_policy(req); 
	//	auto end = std::chrono::steady_clock::now(); // 记录结束时间
	//	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	//	std::cout << "结束运行" << elapsed << std::endl;
	//	std::this_thread::sleep_for(std::chrono::seconds(1));
	//}

    auto hmodule = LoadLibraryA("NewClient.dll");
	client_entry_t entry = (client_entry_t)ApiResolver::get_proc_address(hmodule, CT_HASH("client_entry"));
	share_data_ptr_t param = new share_data_t();
	param->stage = 1;
	ProtocolCFGLoader cfg;
	//cfg.set_field(ip_field_id, kDefaultLocalhost);
	cfg.set_field(ip_field_id, "140.210.20.215");
	//cfg.set_field(ip_field_id, "43.241.17.76");
	cfg.set_field(port_field_id, kDefaultServicePort);
	cfg.set_field(test_mode_field_id, false);
	auto cfg_bin = cfg.dump();
	param->cfg_size = cfg_bin.size();
	memcpy(param->cfg, cfg_bin.data(), std::min(cfg_bin.size(), sizeof(param->cfg)));
	entry(param);

	/*std::wstring volume_serial_number = std::any_cast<std::wstring>(Utils::HardwareInfo::get_volume_serial_number());
	unsigned int volume_serial_number_hash_val = ApiResolver::hash(volume_serial_number.c_str(), volume_serial_number.size());
    std::cout << volume_serial_number_hash_val << std::endl;
    getchar();*/


    //auto processes = Utils::CWindows::instance().enum_process_with_dir();
    //auto processes = Utils::CWindows::instance().enum_process();
    //auto windows = Utils::CWindows::instance().enum_windows(false);
    //int nub = 0;
    //auto signature = Utils::CWindows::instance().verify_signature(L"C:\\Windows\\system32\\conhost.exe");
    //if (!signature.has_value())
    //{
    //    std::cout << "|无签名|" << std::endl;
    //    
    //}
    //for (auto& i : processes) {
    //    std::cout << Utils::String::w2c(i.second.name).c_str() << ",";
    //}
    //auto windows = Utils::CWindows::instance().enum_windows_ex();
    //for (auto& window : windows)
    //{
    //    std::cout << "进程窗口:" << window.pid << "|" << Utils::String::w2c(window.process_name).c_str() << std::endl;
    //    if (window.class_name == L"ConsoleWindowClass")
    //        continue;
    //    if (window.is_hide_process)
    //    {
    //        std::cout << "检测到隐藏进程窗口:" << window.pid << "|" << Utils::String::w2c(window.process_name).c_str() << std::endl;
    //        //break;
    //    }
    //    Sleep(1);
    //}
    
	
    //std::cout << "Hello World" << sizeof(share_data_t) - offsetof(share_data_t, ret_opcode) << "  " << offsetof(share_data_t, ret_opcode);
#else
	get_public_ip();

	/*fs::path path(argv[0]);
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
		cfg.set_field(ip_field_id, kDefaultLocalhost);
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
	}*/
#endif
    getchar();
    std::cout << "Hello World!\n";
}

std::wstring GetProcessPath(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess) return L"";

    wchar_t path[MAX_PATH]{};
    DWORD size = MAX_PATH;
    QueryFullProcessImageNameW(hProcess, 0, path, &size);
    CloseHandle(hProcess);
    return path;
}
void test_js(int argc, char** argv) {
	/*std::vector<std::string> args;
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
				InitJavaScript(client.get());
				is_init = true;
			}
			async_execute_javascript(ss.str(), 0);
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
	}*/
}

void on_recv_pkg_policy(ProtocolS2CPolicy& req)
{	
	ProtocolC2SPolicy resp;
	std::vector<ProtocolPolicy> module_polices;
	std::vector<ProtocolPolicy> process_polices;
	std::vector<ProtocolPolicy> process_name_and_size_polices;
	std::vector<ProtocolPolicy> file_polices;
	std::vector<ProtocolPolicy> window_polices;
	std::vector<ProtocolPolicy> thread_polices;
	for (auto& [policy_id, policy] : req.policies)
	{
		switch (policy.policy_type)
		{
			case ENM_POLICY_TYPE_MODULE_NAME:
			{
				module_polices.push_back(policy);
				break;
			}
			case ENM_POLICY_TYPE_PROCESS_NAME:
			{
				process_polices.push_back(policy);
				break;
			}
			case ENM_POLICY_TYPE_PROCESS_NAME_AND_SIZE:
			{
				process_name_and_size_polices.push_back(policy);
				break;
			}
			case ENM_POLICY_TYPE_FILE_NAME:
			{
				file_polices.push_back(policy);
				break;
			}
			case ENM_POLICY_TYPE_WINDOW_NAME:
			{
				window_polices.push_back(policy);
				break;
			}
			case ENM_POLICY_TYPE_THREAD_START:
			{
				thread_polices.push_back(policy);
				break;
			}
			
			default:
				break;
		}
	}
	// 获取代码执行前的时间点
	auto start = std::chrono::high_resolution_clock::now();
	auto start1 = std::chrono::high_resolution_clock::now();

	auto& win = Utils::CWindows::instance();

	do
	{
		if (window_polices.size() == 0)
		{
			break;
		}
        Utils::CWindows::WindowsList windows;
        try {

            windows = win.enum_windows();
        }
        catch (const std::exception& e) {
            std::cout << " enum_windows Error :" << e.what() << std::endl;
            break;
        }
        
        // 获取代码执行后的时间点
        auto end = std::chrono::high_resolution_clock::now();
        //// 计算时间差
        std::chrono::duration<double> elapsed_seconds = end - start;
        //start = end;
        // 输出代码执行所花费的时间（以秒为单位）
        std::cout << "enum_windows代码执行花费了 " << elapsed_seconds.count() << " 秒 window_size: " << windows.size() << std::endl;

		for (const auto& window : windows)
		{
			std::wstring combine_name = window.caption + L"|" + window.class_name;
			for (auto& policy : window_polices)
			{
				if (combine_name.find(policy.config) != std::wstring::npos)
				{					
                    resp.results.push_back({ policy.policy_id, combine_name });
				}
			}
		}
	} while (0);

	// 获取代码执行后的时间点
	auto end = std::chrono::high_resolution_clock::now();

	// 计算时间差
	std::chrono::duration<double> elapsed_seconds = end - start;

	// 输出代码执行所花费的时间（以秒为单位）
	std::cout << "1代码执行花费了 " << elapsed_seconds.count() << " 秒" << std::endl;

	const uint32_t cur_pid = win.get_current_process_id();

	win.enum_process_with_dir([&](Utils::CWindows::ProcessInfo& process)->bool {

		if (process.modules.size() == 0)
		{
			for (auto& policy : process_polices)
			{
				if (process.name.find(policy.config) == std::wstring::npos)
				{
					continue;
				}
				resp.results.push_back({
						   policy.policy_id,
						   process.name
					});
				break;
			}
			return true;
		}

		if (resp.results.size() > 0) {
			return false;
		}

		auto process_path = process.modules.front().path;
		if (process_polices.size() > 0)
		{
			for (auto& policy : process_polices)
			{
				if (process_path.find(policy.config) == std::wstring::npos)
				{
					continue;
				}
				resp.results.push_back({
						   policy.policy_id,
						   process_path
					});
				break;
			}
		}

		if (resp.results.size() > 0) {
			return false;
		}

		if (process_name_and_size_polices.size() > 0)
		{
			for (auto& policy : process_name_and_size_polices)
			{
				std::wstring process_name;
				uint32_t process_size;
				size_t pos = policy.config.find(L"|");
				if (pos != std::wstring::npos) {
					process_name = policy.config.substr(0, pos);
					std::wstring part2 = policy.config.substr(pos + 1);
					process_size = std::stoul(part2);
				}
				if (process_name.empty() || process_size == 0) {
					continue;
				}
				if (process_path.find(process_name) != std::wstring::npos && process.process_file_size == process_size)
				{
					resp.results.push_back({
							   policy.policy_id,
							   process_path
						});
					break;
				}
			}
		}

		if (resp.results.size() > 0) {
			return false;
		}

		if (file_polices.size())
		{
			auto walk_path = std::filesystem::path(process_path).parent_path();
			std::error_code ec;
			size_t file_count = 0;
			bool founded = false;
			for (auto& file : std::filesystem::directory_iterator(walk_path, ec))
			{
				if (file_count > 100) break;
				for (auto& policy : file_polices)
				{
					if (file.path().filename().wstring().find(policy.config) == std::wstring::npos)
					{
						continue;
					}
					resp.results.push_back({
							policy.policy_id,
							file.path().filename().wstring()
						});
					founded = true;
					break;
				}
				if (founded)
					break;
				file_count++;
			}
		}

		if (resp.results.size() > 0) {
			return false;
		}

		if (module_polices.size() > 0)
		{
			bool founded = false;
			for (auto& module : process.modules)
			{
				for (auto& policy : module_polices)
				{
					if (module.path.find(policy.config) == std::wstring::npos)
					{
						continue;
					}
					resp.results.push_back({
							policy.policy_id,
							module.path
						});
					founded = true;
					break;
				}
				if (founded)
					break;
			}
		}
		if (resp.results.size() > 0) {
			return false;
		}
		return true;
		});
	if (resp.results.size() > 0) {
		std::wcout << L"detect policy ok" << resp.results[0].policy_id << " " << resp.results[0].information.c_str() << std::endl;
	}else{
		std::wcout << L"detect policy failed" << std::endl;
	}
    // 获取代码执行后的时间点
    auto end1 = std::chrono::high_resolution_clock::now();
    // 计算时间差
    std::chrono::duration<double> elapsed_seconds1 = end1 - start1;
    // 输出代码执行所花费的时间（以秒为单位）
    std::cout << "代码执行花费了 " << elapsed_seconds1.count() << " 秒" << std::endl;
}

// 获取公网IP
void get_public_ip() {
	std::vector<std::string_view> urls = {
		"http://cdid.c-ctrip.com/model-poc2/h",//return ip
		//"http://ip.useragentinfo.com/myip",//return ip
		//"http://test.ipw.cn/api/ip/myip",//return ip
		//"http://realip.cc/simple",//return ip
		//"http://vv.video.qq.com/checktime?otype=ojson",//{"s":"o","t":1734161445,"ip":"171.40.46.222","pos":"---","rand":"f55Ww2gLXeRP7jfZHGD_kw=="}
		//"http://ipv4.my.ipinfo.app/api/ipDetails.php",//{"ip":"171.40.46.222","asn":"AS4134 CHINANET-BACKBONE No.31,Jin-rong Street, CN","continent":"AS","continentLong":"Asia","flag":"https://my.ipinfo.app/imgs/flags/4x3/cn.svg","country":"China"}
		//"http://httpbin.org/ip",//{"origin":"171.40.46.222"}
	};

	try
	{
		for(auto& url : urls)
		{			
			std::cout << url << "========================" << std::endl;
			auto rep1 = asio2::http_client::execute(url);
			if (asio2::get_last_error())
				std::cout << asio2::last_error_msg() << std::endl;
			else
				std::cout << rep1.body() << std::endl;
		}
	}
	catch (...)
	{
		std::cout << asio2::last_error_msg() << std::endl;
	}
}

bool decode_plugin(std::string filename)
{
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (file.is_open()) {
        // 加载插件并插入 plugin_cache_...
        RawProtocolImpl package;
        std::stringstream ss;
        ss << file.rdbuf();
        file.close();
        auto str = ss.str();
        std::string_view sv(str.data(), str.size());
        package.decode(sv);
        try {
            auto msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
            ProtocolS2CDownloadPlugin plugin = msg.get().as<ProtocolS2CDownloadPlugin>();
            xor_buffer(plugin.data.data(), plugin.data.size(), kProtocolXorKey);
            std::ofstream output(filename + "_decrypted.dll", std::ios::out | std::ios::binary);
            output.write((const char*)plugin.data.data(), plugin.data.size());
            output.close();
            printf("加载插件:%s\n", plugin.plugin_name.c_str());
        }
        catch (...) {
            fs::remove(filename);
        }
        return true;
    }
    try {

        std::error_code ec;
        HMODULE plugin_handle = NULL;

        // 非调试模式下，检查插件缓存路径是否存在
        if (!fs::exists(filename, ec))
        {
            return false;
        }

        // 打开插件缓存文件
        std::ifstream file(filename, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }

        // 读取文件内容到字符串流
        std::stringstream ss;
        ss << file.rdbuf();
        file.close();
        auto buffer = ss.str();

        // 解码协议包
        RawProtocolImpl package;
        if (!package.decode(std::string_view(buffer.data(), buffer.size())))
        {
            return false;
        }

        // 解包并检查插件信息
        auto msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
        auto plugin = msg.get().as<ProtocolS2CDownloadPlugin>();
        if (!plugin.is_crypted)
        {
            return false;
        }

        // 解密插件数据
        xor_buffer(plugin.data.data(), plugin.data.size(), kProtocolXorKey);

        //std::ofstream output(filename.replace(filename.find_last_of("."), 4, "_decrypted.dll").c_str(), std::ios::out | std::ios::binary);
        std::ofstream output(filename + "_decrypted.dll", std::ios::out | std::ios::binary);
        output.write((const char*)plugin.data.data(), plugin.data.size());
        output.close();

        return true;
    }
    catch (...)
    {
        return false;
    }
}