#include "pch.h"
#include "TaskBasic.h"

std::shared_ptr<bool> is_debug_mode = std::make_shared<bool>(false);
std::shared_ptr<bool> is_detect_finish = std::make_shared<bool>(true);
std::shared_ptr<int> reconnect_count = std::make_shared<int>(0);

// 兼容XP的时间常量定义
#ifdef _WIN32_WINNT_WINXP
constexpr auto POLICY_TIMEOUT = std::chrono::seconds(10);
#else
using namespace std::chrono_literals;
constexpr auto POLICY_TIMEOUT = 10s;
#endif

void NotifyHook()
{
    auto client_connect_success_handler = client_->notify_mgr().get_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID);
    client_->notify_mgr().replace_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [client_connect_success_handler]() {
        client_connect_success_handler();
        client_->notify_mgr().dispatch(CLIENT_RECONNECT_SUCCESS_NOTIFY_ID);
    });
}

void LoadPlugin()
{
    client_->set_is_loaded_plugin(true);
    LOG("加载插件成功---");
    VMP_VIRTUALIZATION_BEGIN();
    srand(time(0));
    //InitMiniDump();
    std::wstring volume_serial_number = std::any_cast<std::wstring>(client_->user_data().get_field(vol_field_id));
    unsigned int volume_serial_number_hash_val = ApiResolver::hash(volume_serial_number.c_str(), volume_serial_number.size());
    if (volume_serial_number_hash_val == 1770936153)
    {
        *is_debug_mode = true;
        ProtocolC2STaskEcho echo;
        echo.task_id = 689999;
        echo.is_cheat = false;
        echo.text = "测试";
        client_->send(&echo);
    }

    if(*g_client_rev_version != REV_VERSION)
    {
        LOG("插件版本不匹配");
    }
    LOG("加载插件");
	client_->package_mgr().replace_handler(SPKG_ID_S2C_PUNISH, std::bind(&on_recv_punish, std::placeholders::_1, std::placeholders::_2));
    client_->package_mgr().register_handler(SPKG_ID_S2C_QUERY_PROCESS,[](const RawProtocolImpl& package, const msgpack::v1::object_handle&){
        client_->post([pkg_ptr = std::make_shared<RawProtocolImpl>(package)]() {
            LOG("%s|%d 当前线程:%u===========", __FUNCTION__, __LINE__, GetCurrentThreadId());
            LOG("查询进程:%u", pkg_ptr->head.session_id);
            auto processes = Utils::CWindows::instance().enum_process_with_dir();
            ProtocolC2SQueryProcess resp;
            resp.data = cast(processes);
            LOG("返回查询进程:%d", resp.package_id);
            client_->send(&resp, pkg_ptr->head.session_id);
        });
    });
    client_->package_mgr().register_handler(SPKG_ID_S2C_QUERY_DRIVERINFO, [](const RawProtocolImpl& package, const msgpack::v1::object_handle&) {
        client_->post([pkg_ptr = std::make_shared<RawProtocolImpl>(package)]() {
            auto drivers = Utils::CWindows::instance().enum_drivers();
            ProtocolC2SQueryDriverInfo resp;
            resp.data = cast(drivers);
            client_->send(&resp, pkg_ptr->head.session_id);
        });
    });
    client_->package_mgr().register_handler(SPKG_ID_S2C_QUERY_WINDOWSINFO, [](const RawProtocolImpl& package, const msgpack::v1::object_handle&) {
        client_->post([pkg_ptr = std::make_shared<RawProtocolImpl>(package)]() {
            auto windows = Utils::CWindows::instance().enum_windows_ex();
            ProtocolC2SQueryWindowsInfo resp;
            resp.data = cast(windows);
            client_->send(&resp, pkg_ptr->head.session_id);
        });
    });
    client_->package_mgr().register_handler(SPKG_ID_S2C_QUERY_SCREENSHOT, [](const RawProtocolImpl& package, const msgpack::v1::object_handle&) {
        client_->post([pkg_ptr = std::make_shared<RawProtocolImpl>(package)]() {
            size_t sz = 0;
            auto jpg = Utils::get_screenshot(&sz);
            ProtocolC2SQueryScreenShot resp;
            resp.data.resize(sz);
            memcpy(resp.data.data(), jpg.get(), sz);
            client_->send(&resp, pkg_ptr->head.session_id);
        });
    });
    client_->package_mgr().replace_handler(SPKG_ID_S2C_SCRIPT, [](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        async_execute_javascript(msg.get().as<ProtocolS2CScript>().code, -1);
    });
    // 接收logic_server的GM策略列表
    client_->package_mgr().replace_handler(SPKG_ID_S2C_POLICY, [](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto policy = msg.get().as<ProtocolS2CPolicy>();
        client_->post([policy_ptr = std::make_shared<ProtocolS2CPolicy>(policy)]() {
#ifdef LOG_SHOW
			auto start = std::chrono::high_resolution_clock::now();
#endif
            on_recv_pkg_policy(*policy_ptr);
        #ifdef LOG_SHOW
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
			LOG("on_recv_pkg_policy cost %lld us", duration.count());
#endif
        });
    });	
    LOG(__FUNCTION__);

    //NotifyHook();
	InitRmc();
    //InitTimeoutCheck();
	InitJavaScript();
    //InitDirectoryChangsDetect();
    if (*is_debug_mode == false)
    {
        // InitHideProcessDetect停用,因为枚举窗口设置的是3分钟一次,导致与枚举进程的频率不一致,
        // 如果进程已经退出,但枚举窗口因为还没有刷新会导致检测不准确,产生误报
		//InitHideProcessDetect();
		InitSpeedDetect();
		//InitShowWindowHookDetect();
    }   

    VMP_VIRTUALIZATION_END();
}

void on_recv_punish(const RawProtocolImpl& package, const msgpack::v1::object_handle& msg)
{
	switch (msg.get().as<ProtocolS2CPunish>().type)
	{
	case PunishType::ENM_PUNISH_TYPE_KICK:
	{
		VMP_VIRTUALIZATION_BEGIN();
        std::thread([]() {
            std::this_thread::sleep_for(std::chrono::seconds(std::rand() % 5 + 5));
            Utils::CWindows::instance().exit_process();
            exit(-1);
            abort();
            UnitPunishKick();
            Utils::CWindows::instance().exit_process();
            exit(-1);
            abort();
        }).detach();
        VMP_VIRTUALIZATION_END();
        if (auto main_window_hwnd = client_->g_main_window_hwnd.load()) {
		    MessageBoxA(main_window_hwnd, xorstr("请勿开挂进行游戏！否则有封号拉黑风险处罚"), xorstr("封挂提示"), MB_OK | MB_ICONWARNING);
        }
        else {
            MessageBoxA(nullptr, xorstr("请勿开挂进行游戏！否则有封号拉黑风险处罚"), xorstr("封挂提示"), MB_OK | MB_ICONWARNING);
        }
		break;
	}
	default:
		break;
	}
}
void on_recv_pkg_policy(const ProtocolS2CPolicy& req)
{
    // 确保检测状态原子操作
    static std::atomic<bool> is_detect_finish(true);

    if (!is_detect_finish.load(std::memory_order_acquire)) {
        return;
    }
    is_detect_finish.store(false, std::memory_order_release);

    g_thread_group->create_thread([req]() {
        try {
            // 使用值捕获避免引用失效
            auto policy_task = std::async(std::launch::async, [req] {
                // 实际策略执行代码
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
                    case ENM_POLICY_TYPE_SCRIPT:
		            {
                        try {
                            if (policy.config.empty()) break;
			                LOG("ENM_POLICY_TYPE_SCRIPT---1 policy_id = %d, comment = %s ", policy_id, Utils::String::w2c(policy.comment).c_str());
                            async_execute_javascript(Utils::String::w2c(policy.config), policy_id);
			                LOG("ENM_POLICY_TYPE_SCRIPT---2 policy_id = %d, comment = %s ", policy_id, Utils::String::w2c(policy.comment).c_str());
                        }
                        catch (...) {
                            LOG("async_execute_javascript error");
                        }
            #if 0
			            std::filesystem::create_directories(".\\temp_scripts");
			            std::ofstream script(".\\temp_scripts\\" + Utils::String::w2c(policy.comment) + ".js", std::ios::out);
			            script << Utils::String::w2c(policy.config);
                        script.close();
            #endif
                        break;
                    }  
                    default:
                        break;
                    }
                }

                auto& win = Utils::CWindows::instance();
                bool find_cheat = false;
                bool is_cheat = false;
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

                    for (const auto& window : windows)
                    {
                        std::wstring combine_name = window.caption + L"|" + window.class_name;
                        for (auto& policy : window_polices)
                        {
                            if (combine_name.find(policy.config) != std::wstring::npos)
                            {
                                resp.results.push_back({ policy.policy_id, combine_name });
                                is_cheat = (policy.punish_type == PunishType::ENM_PUNISH_TYPE_KICK);
                                if (is_cheat) {
                                    find_cheat = true;
                                    break;
                                }
                            }
                        }
                        if (find_cheat) {
                            break;
                        }
                    }
                    if (find_cheat) {
                        break;
                    }
                } while (0);

                const uint32_t cur_pid = win.get_current_process_id();

                if (resp.results.size() > 0) {
                    client_->send(&resp);
                    if (is_cheat) {
                        is_detect_finish.store(true, std::memory_order_release);
                        return;
                    }
                }

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
                            is_cheat = (policy.punish_type == PunishType::ENM_PUNISH_TYPE_KICK);
                            if (is_cheat) {
                                break;
                            }
                        }
                        return true;
                    }

                    if (resp.results.size() > 0 && is_cheat) {
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
                            is_cheat = (policy.punish_type == PunishType::ENM_PUNISH_TYPE_KICK);
                            if (is_cheat) {
                                break;
                            }
                        }
                    }

                    if (resp.results.size() > 0 && is_cheat) {
                        return false;
                    }

                    /*if (file_polices.size())
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
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
                    }*/

                    /*if (module_polices.size() > 0)
                    {
                        bool founded = false;
                        for (auto& module : process.modules)
                        {
                            for (auto& policy : module_polices)
                            {
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
			        }*/
                    return true;
                    });
                if (resp.results.size() > 0) {
                    client_->send(&resp);
                }
            });

            // 兼容XP的超时等待实现
            auto status = policy_task.wait_for(POLICY_TIMEOUT);
            if (status != std::future_status::ready) {
                LOG("策略执行超时");

                // 尝试终止卡住的任务
                try {
                    policy_task.get(); // 触发异常
                }
                catch (const std::future_error& e) {
                    LOG("任务终止错误: %s", e.what());
                }
            }
        }
        catch (const std::exception& e) {
            LOG("策略执行异常: %s", e.what());
        }
        catch (...) {
            LOG("未知策略执行异常");
        }

        // 确保状态重置
        is_detect_finish.store(true, std::memory_order_release);
    });
}