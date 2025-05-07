
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
int main(int argc, char** argv)
{
#ifndef _DEBUG
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

 //   auto hmodule = LoadLibraryA("NewClient.dll");
	//client_entry_t entry = (client_entry_t)ApiResolver::get_proc_address(hmodule, CT_HASH("client_entry"));
	//share_data_ptr_t param = new share_data_t();
	//param->stage = 1;
	//ProtocolCFGLoader cfg;
	////cfg.set_field(ip_field_id, kDefaultLocalhost);
	////cfg.set_field(ip_field_id, "140.210.20.215");
	//cfg.set_field(ip_field_id, "43.241.17.76");
	//cfg.set_field(port_field_id, kDefaultServicePort);
	//cfg.set_field(test_mode_field_id, false);
	//auto cfg_bin = cfg.dump();
	//param->cfg_size = cfg_bin.size();
	//memcpy(param->cfg, cfg_bin.data(), std::min(cfg_bin.size(), sizeof(param->cfg)));
	//entry(param);

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
    std::string_view host = "0.0.0.0";
    std::string_view port = "8028";

    // 由于tcp_server默认会启动cpu*2个数量的线程，假定cpu核数为4，那就是8个线程，
    // 假定为“线程0，线程1，...到...线程7”
    // 这里main函数的线程假定为“线程main”，（新版本asio2没有任何事件会在main线程中触发）

    static std::size_t constexpr  tcp_frame_size = 1536;
    static std::size_t constexpr  udp_frame_size = 1024;
    static std::size_t constexpr http_frame_size = 1536;

    static std::size_t constexpr max_buffer_size = (std::numeric_limits<std::size_t>::max)();
    asio2::tcp_server server(tcp_frame_size, max_buffer_size,1);

    // 针对server对象启动一个定时器
    server.start_timer(123, std::chrono::seconds(1), []() {
        // 这个定时器的回调函数固定在“线程0”中触发
        printf("server.start_timer被执行了\n");
    });

    // 针对这个session_ptr投递一个异步事件
    server.post([]() {
        // 对这个server对象的投递的异步事件固定在“线程0”中触发
        printf("投递的异步事件被执行了\n");
    });


    server.bind_init([]() {
        // 固定在“线程0”中触发
        printf("bind_init\n");
    }).bind_start([&]() {
        // 固定在“线程0”中触发
        printf("bind_start\n");
    }).bind_accept([](std::shared_ptr<asio2::tcp_session>& session_ptr) {
        // 固定在“线程0”中触发
        printf("bind_accept\n");

    }).bind_connect([&](auto& session_ptr) {
        // 固定在“线程0”中触发

        // 连接成功以后，可以给这个连接(即session_ptr)启动一个定时器
        session_ptr->start_timer("123", std::chrono::seconds(1), []() {
            // 这个session_ptr的定时器的回调函数和bind_recv的触发线程是同一个线程
            // (见下方bind_recv中的说明)
        });

        // 针对这个session_ptr投递一个异步事件
        session_ptr->post([]() {
            // 对这个session_ptr投递的异步事件的回调函数和bind_recv的触发线程是
            // 同一个线程(见下方bind_recv中的说明)
            printf("投递的异步事件被执行了\n");
        });
        printf("bind_connect\n");

    }).bind_recv([&](std::shared_ptr<asio2::tcp_session>& session_ptr, std::string_view data) {
        // 平均分布在“线程0，线程1，...到...线程7”中触发

        // 假如session A的bind_recv在“线程2”中触发，session B的bind_recv在“线程3”中触发，
        // 那么session A的bind_recv将永远只在“线程2”中触发，不会出现一会儿在“线程2”中触发
        // 一会儿在“线程3”中触发这种情况，session B同理。

        // 对于async_send发送数据函数来说，真正发送数据时所在的线
        // 程和bind_recv的触发线程是同一个线程。

        // 假定session A的bind_recv的触发线程是“线程2”，那么session A的async_send函数也是在“线程2”
        // 中发送的数据的(不管async_send函数在哪里调用，不管在哪个线程调用，最终都是投递到“线程2”中
        // 去发送的)。
        session_ptr->async_send(data, [](std::size_t bytes_sent) {
            // async_send函数可以设置一个回调函数，当发送数据结束以后(不管发送成功还是发送失败)，这个
            // 回调函数就会被调用。
            // 这个回调函数也是在“线程2”中触发的。
            if (asio2::get_last_error())
                printf("发送数据失败,失败原因:%s\n", asio2::last_error_msg().c_str());
            else
                printf("发送数据成功,共发送了%d个字节\n", int(bytes_sent));
        });

    }).bind_disconnect([&](auto& session_ptr) {
        // 固定在“线程0”中触发
        printf("bind_disconnect\n");

    }).bind_stop([&]() {
        // 固定在“线程0”中触发
        printf("bind_stop\n");

    });

    printf("server.start1\n");
    server.start(host, port);
    printf("server.start2\n");

    while (std::getchar() != '\n');  // press enter to exit this program

    server.stop();

    return 0;
	
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
