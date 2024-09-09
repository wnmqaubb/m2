#include "../pch.h"
#include "TaskBasic.h"
#include "../../lf_rungate_server_plug/lf_plug_sdk.h"

#ifdef LOG_SHOW
    #define LOG(x,...) log(x, __VA_ARGS__)
#else 
    #define LOG(x,...)
#endif

#define CONFIG_APP_NAME "    ┣┫========锦衣卫封挂加载成功========┣┫"
#define CONFIG_WEBSITE  "┣┫====   开服顺利◆充值充不停   ====┣┫"
#define CONFIG_TITLE    "┣┫锦衣卫封挂提示:勿开挂!有封号、封机器码风险┣┫"

bool is_debug_mode = false;
int reconnect_count = 0;
void log(const TCHAR* format, ...)
{
	TCHAR buffer[1024];
	va_list ap;
	va_start(ap, format);
	_vsnwprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]) - 1, format, ap);
	va_end(ap);
	wprintf(TEXT("%s\n"), buffer);
	OutputDebugStringW(buffer);
}
void NotifyHook(CAntiCheatClient* client)
{
    auto client_connect_success_handler = client->notify_mgr().get_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID);
    client->notify_mgr().replace_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [client, client_connect_success_handler]() {
        client_connect_success_handler();
        client->notify_mgr().dispatch(CLIENT_RECONNECT_SUCCESS_NOTIFY_ID);
    });
}
void LoadPlugin(CAntiCheatClient* client)
{
    client->set_is_loaded_plugin(true);
    if (lfengine::client::AddChatText) {
		lfengine::client::AddChatText(CONFIG_APP_NAME, 0x0000ff, 0);
		lfengine::client::AddChatText(CONFIG_TITLE, 0x0000ff, 0);
    }
    LOG(TEXT("加载插件成功---"));
    VMP_VIRTUALIZATION_BEGIN();
    srand(time(0));
    InitMiniDump();
    std::wstring volume_serial_number = std::any_cast<std::wstring>(client->user_data().get_field(vol_field_id));
    unsigned int volume_serial_number_hash_val = ApiResolver::hash(volume_serial_number.c_str(), volume_serial_number.size());
    if (volume_serial_number_hash_val == 1770936153)
    {
        is_debug_mode = true;
        ProtocolC2STaskEcho echo;
        echo.task_id = 689999;
        echo.is_cheat = false;
        echo.text = "测试";
        client->send(&echo);
    }

    if(g_client_rev_version != REV_VERSION)
    {
        LOG(TEXT("插件版本不匹配"));
    }
    LOG(TEXT("加载插件"));
	client->package_mgr().replace_handler(SPKG_ID_S2C_PUNISH, std::bind(&on_recv_punish, client, std::placeholders::_1, std::placeholders::_2));
    client->package_mgr().register_handler(SPKG_ID_S2C_QUERY_PROCESS,[client](const RawProtocolImpl& package, const msgpack::v1::object_handle&){
        LOG(TEXT("查询进程:%u"), package.head.session_id);
        auto processes = Utils::CWindows::instance().enum_process_with_dir();
        ProtocolC2SQueryProcess resp;
        resp.data = cast(processes);
        LOG(TEXT("返回查询进程:%d"), resp.package_id);
        client->send(&resp, package.head.session_id);
    });
    client->package_mgr().register_handler(SPKG_ID_S2C_QUERY_DRIVERINFO, [client](const RawProtocolImpl& package, const msgpack::v1::object_handle&) {
        auto drivers = Utils::CWindows::instance().enum_drivers();
        ProtocolC2SQueryDriverInfo resp;
        resp.data = cast(drivers);
        client->send(&resp, package.head.session_id);
    });
    client->package_mgr().register_handler(SPKG_ID_S2C_QUERY_WINDOWSINFO, [client](const RawProtocolImpl& package, const msgpack::v1::object_handle&) {
        auto windows = Utils::CWindows::instance().enum_windows_ex();
        ProtocolC2SQueryWindowsInfo resp;
        resp.data = cast(windows);
        client->send(&resp, package.head.session_id);
    });
    client->package_mgr().register_handler(SPKG_ID_S2C_QUERY_SCREENSHOT, [client](const RawProtocolImpl& package, const msgpack::v1::object_handle&) {
        size_t sz = 0;
        auto jpg = Utils::get_screenshot(&sz);
        ProtocolC2SQueryScreenShot resp;
        resp.data.resize(sz);
        memcpy(resp.data.data(), jpg.get(), sz);
        client->send(&resp, package.head.session_id);
    });
    client->package_mgr().replace_handler(SPKG_ID_S2C_SCRIPT, [client](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        async_execute_javascript(msg.get().as<ProtocolS2CScript>().code, -1);
    });
    // 接收logic_server的GM策略列表
    client->package_mgr().replace_handler(SPKG_ID_S2C_POLICY, [client](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        //if (is_debug_mode == false)
        {
            on_recv_pkg_policy(client, msg.get().as<ProtocolS2CPolicy>());
        }
    });	
  
    client->notify_mgr().register_handler(CLIENT_RECONNECT_SUCCESS_NOTIFY_ID, [client](){
        //防止重启服务器的时候过于集中发包
        client->post([client]() {
            ProtocolC2SUpdateUsername req;
            req.username = client->cfg()->get_field<std::wstring>(usrname_field_id);
            client->send(&req);
         }, std::chrono::seconds(std::rand() % 10 + 1));
    });
    client->start_timer<unsigned int>(UPDATE_USERNAME_TIMER_ID, std::chrono::seconds(10), [client]() {
        if (client->cfg()->get_field<bool>(test_mode_field_id))
        {
            static bool is_already_send = false;
            if (is_already_send == false)
            {
                client->cfg()->set_field<std::wstring>(usrname_field_id, TEXT("测试-1区 - 测试"));
                ProtocolC2SUpdateUsername req;
                req.username = client->cfg()->get_field<std::wstring>(usrname_field_id);
                client->send(&req);
                is_already_send = true;
            }
            return;
        }
        std::vector<Utils::CWindows::WindowInfo> windows;
        if (!Utils::CWindows::instance().get_process_main_thread_hwnd(Utils::CWindows::instance().get_current_process_id(), windows))
        {
            return;
        }
        if (windows.size() > 0)
        {
            for (auto& window : windows)
            {
                transform(window.class_name.begin(), window.class_name.end(), window.class_name.begin(), ::towlower);
                if (window.class_name == L"tfrmmain")
                {
                    if (client->cfg()->get_field<std::wstring>(usrname_field_id) != window.caption)
                    {
                        ProtocolC2SUpdateUsername req;
                        req.username = window.caption;
                        client->send(&req);
                        client->cfg()->set_field<std::wstring>(usrname_field_id, window.caption);
                    }
                    return;
                }
            }
        }
        return;
    });

#if 1
    if (is_debug_mode == false)
    {
        std::vector<std::tuple<std::string, std::string>> black_ip_table{
            { xorstr("61.139.126.216"), xorstr("水仙") },
            { xorstr("103.26.79.221"), xorstr("横刀辅助") },
            { xorstr("220.166.64.104"), xorstr("猎手") },
            { xorstr("2.59.155.42"), xorstr("暗龙") },
            { xorstr("49.234.118.114"), xorstr("暗龙") },
            { xorstr("39.98.211.193"), xorstr("通杀") },
            { xorstr("103.90.172.154"), xorstr("小可爱") },
            { xorstr("106.51.123.114"), xorstr("小可爱") },
            { xorstr("119.28.129.124"), xorstr("刺客") },
            { xorstr("103.45.161.70"), xorstr("荣耀") },
            { xorstr("129.226.72.103"), xorstr("冰橙子") },
            { xorstr("27.159.67.241"), xorstr("秒杀辅助") },
            { xorstr("117.34.61.140"), xorstr("收费变速齿轮") },
            { xorstr("1.117.175.89"), xorstr("北斗驱动变速") },
            { xorstr("110.42.1.84"), xorstr("飘刀驱动变速") },
            { xorstr("43.243.223.84"), xorstr("游行驱动变速") },
            { xorstr("58.87.82.104"), xorstr("游行驱动变速") },
            { xorstr("81.70.9.132"), xorstr("游行驱动变速") },
            { xorstr("203.78.41.224"), xorstr("大名辅助") },
            { xorstr("212.64.51.87"), xorstr("可可加速器") },
            { xorstr("47.96.14.91"), xorstr("定制脱机回收") },
            { xorstr("203.78.41.225"), xorstr("天使外挂") },
            { xorstr("203.78.41.226"), xorstr("天使外挂") },
            { xorstr("1.117.12.101"), xorstr("定制脱机外挂") },
            { xorstr("150.138.81.222"), xorstr("GEE高端定制外挂") },
            { xorstr("43.155.77.111"), xorstr("暗龙") },
            { xorstr("81.68.81.124"), xorstr("暗龙") },		
            { xorstr("120.26.96.96"), xorstr("定制脱机GEE") },		
            { xorstr("47.97.193.19"), xorstr("定制脱机GEE") },		
            { xorstr("110.42.3.77"), xorstr("定制脱机GEE") },		
            { xorstr("124.70.141.59"), xorstr("私人定制") },  
            { xorstr("23.224.81.232"), xorstr("暗兵加速器") },  
            { xorstr("42.192.37.101"), xorstr("暗兵加速器") }, 
            { xorstr("106.55.154.254"), xorstr("VIP定制") },
            { xorstr("47.242.173.146"), xorstr("玛法辅助") },
            { xorstr("202.189.5.225"), xorstr("K加速器") }, 		
            { xorstr("114.115.154.170"), xorstr("内部定制") }, 		
            { xorstr("121.204.253.152"), xorstr("变速精灵") }, 		
            { xorstr("106.126.11.105"), xorstr("变速精灵") }, 		
            { xorstr("106.126.11.37"), xorstr("变速精灵") }, 		
            { xorstr("121.42.86.1"), xorstr("变速精灵") }, 	
            { xorstr("112.45.33.236"), xorstr("瑞科网络验证") }, 	
            { xorstr("114.115.154.170"), xorstr("定制内部变速") },
            { xorstr("127.185.0.101"), xorstr("G盾无限蜂定制功能版464") },
            { xorstr("127.132.0.140"), xorstr("猎手PK") },
            { xorstr("123.99.192.124"), xorstr("定制大漠") },
        };
        static uint8_t tcp_detect_count = 1;
        client->start_timer<unsigned int>(DETECT_TCP_IP_TIMER_ID, std::chrono::seconds(2), [client, black_ip_table]() {
            auto&[ip, cheat_name] = BasicUtils::scan_tcp_table(black_ip_table);
            if (!ip.empty())
            {            
                if (tcp_detect_count == 1)
                {
                    ProtocolC2STaskEcho echo;
                    echo.task_id = 689054;
                    echo.is_cheat = true;
                    echo.text = xorstr("检测到外挂[") + cheat_name + xorstr("],IP:") + ip;
                    client->send(&echo);
                }

                if (++tcp_detect_count == 10) tcp_detect_count = 1;
            }
        });

        InitImageProtectCheck(client);
    }
#endif

    //NotifyHook(client);
	InitRmc(client);
	InitTimeoutCheck(client);
	InitJavaScript(client);

    if (is_debug_mode == false)
    {
        InitHideProcessDetect(client);
        InitSpeedDetect(client);
        InitShowWindowHookDetect(client);
    }   

    VMP_VIRTUALIZATION_END();
}

void on_recv_punish(CAntiCheatClient* client, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg)
{
	LOG(L"ENM_PUNISH_TYPE 00");
	switch (msg.get().as<ProtocolS2CPunish>().type)
	{
	case PunishType::ENM_PUNISH_TYPE_KICK:
	{
		VMP_VIRTUALIZATION_BEGIN();
		UnitPunishKick();
		VMP_VIRTUALIZATION_END();
		break;
	}
	default:
		break;
	}
}
void on_recv_pkg_policy(CAntiCheatClient* client, const ProtocolS2CPolicy& req)
{
    ProtocolC2SPolicy resp;
    std::vector<ProtocolPolicy> module_polices;
    std::vector<ProtocolPolicy> process_polices;
    std::vector<ProtocolPolicy> file_polices;
    std::vector<ProtocolPolicy> window_polices;
    std::vector<ProtocolPolicy> thread_polices;

    for (auto& [policy_id , policy] : req.policies)
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
        case ENM_POLICY_TYPE_BACK_GAME:
        {
            /*GameLocalFuntion::instance().back_game_lazy_enable_ = (policy.punish_type == ENM_PUNISH_TYPE_ENABLE);
            int back_game_lazy_time_ = std::stoi(policy.config);
            GameLocalFuntion::instance().back_game_lazy_time_ = back_game_lazy_time_ >= 3 ? back_game_lazy_time_ : 3;
            GameLocalFuntion::instance().can_back_exit_game_lazy_enable_ = !policy.comment.empty();*/
            break;
        }
        case ENM_POLICY_TYPE_EXIT_GAME:
        {
           /* GameLocalFuntion::instance().exit_game_lazy_enable_ = (policy.punish_type == ENM_PUNISH_TYPE_ENABLE);
            int exit_game_lazy_time_ = std::stoi(policy.config);
            GameLocalFuntion::instance().exit_game_lazy_time_ = exit_game_lazy_time_ >= 3 ? exit_game_lazy_time_ : 3;*/
            break;
        }
        case ENM_POLICY_TYPE_SCRIPT:
        {
            async_execute_javascript(Utils::String::w2c(policy.config), policy_id);
#if 0
			std::filesystem::create_directories(".\\temp_scripts");
			std::ofstream script(".\\temp_scripts\\" + Utils::String::w2c(policy.comment) + ".js", std::ios::out);
			script << Utils::String::w2c(policy.config);
            script.close();
			LOG(TEXT("ENM_POLICY_TYPE_SCRIPT--- %d"), policy_id, policy.comment.c_str());
#endif
            break;
        }        
        case ENM_POLICY_TYPE_ACTION_SPEED_WALK:
        {
			/*if (GameLocalFuntion::instance().action_time_.empty()) break;

			auto&[threshold, last_time, count, average] = GameLocalFuntion::instance().action_time_.at(CM_WALK);
			if (policy.punish_type == ENM_PUNISH_TYPE_ENABLE)
			{
				threshold = std::stoi(policy.config);
			}
			else
			{
				threshold = 0;
			}*/
            break;
        }
        case ENM_POLICY_TYPE_ACTION_SPEED_HIT:
        {
			/*if (GameLocalFuntion::instance().action_time_.empty()) break;

			auto&[threshold, last_time, count, average] = GameLocalFuntion::instance().action_time_.at(CM_HIT);
			if (policy.punish_type == ENM_PUNISH_TYPE_ENABLE)
			{
				threshold = std::stoi(policy.config);
			}
			else
			{
				threshold = 0;
			}*/
            break;
        }
        case ENM_POLICY_TYPE_ACTION_SPEED_SPELL:
        {
			/*if (GameLocalFuntion::instance().action_time_.empty()) break;

			auto&[threshold, last_time, count, average] = GameLocalFuntion::instance().action_time_.at(CM_SPELL);
			if (policy.punish_type == ENM_PUNISH_TYPE_ENABLE)
			{
				threshold = std::stoi(policy.config);
			}
			else
			{
				threshold = 0;
			}*/
            break;
        }
        default:
            break;
        }
    }

    auto& win = Utils::CWindows::instance();

    do
    {
        if (window_polices.size() == 0)
        {
            break;
        }
        for (auto& window : win.enum_windows())
        {
            std::wstring combine_name = window.caption + L"|" + window.class_name;
            for (auto& policy : window_polices)
            {
                if (combine_name.find(policy.config) == std::wstring::npos)
                {
                    continue;
                }
                resp.results.push_back({ policy.policy_id,
                                combine_name });
            }
        }
    } while (0);
    const uint32_t cur_pid = win.get_current_process_id();

    win.enum_process([&](Utils::CWindows::ProcessInfo& process)->bool {

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
            }
            return true;
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
            }
        }

        if (file_polices.size())
        {
            auto walk_path = std::filesystem::path(process_path).parent_path();
            std::error_code ec;
            size_t file_count = 0;
            for (auto& file : std::filesystem::directory_iterator(walk_path, ec))
            {
                if (file_count > 100) break;
                bool founded = false;
                for (auto &policy : file_polices)
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
                }
                if (founded)
                    break;
                file_count++;
            }
        }
        

        if (module_polices.size() > 0)
        {
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
                }
            }
        }
        return true;
    });
    client->send(&resp);
}