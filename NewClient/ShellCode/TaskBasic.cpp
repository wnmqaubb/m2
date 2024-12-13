#include "../pch.h"
#include "TaskBasic.h"

std::shared_ptr<bool> is_debug_mode = std::make_shared<bool>(false);
std::shared_ptr<bool> is_detect_finish = std::make_shared<bool>(true);
std::shared_ptr<int> reconnect_count = std::make_shared<int>(0);
std::shared_ptr<HWND> g_main_window_hwnd;
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
    LOG("���ز���ɹ�---");
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
        echo.text = "����";
        client_->send(&echo);
    }

    if(*g_client_rev_version != REV_VERSION)
    {
        LOG("����汾��ƥ��");
    }
    LOG("���ز��");
	client_->package_mgr().replace_handler(SPKG_ID_S2C_PUNISH, std::bind(&on_recv_punish, std::placeholders::_1, std::placeholders::_2));
    client_->package_mgr().register_handler(SPKG_ID_S2C_QUERY_PROCESS,[](const RawProtocolImpl& package, const msgpack::v1::object_handle&){
        LOG("��ѯ����:%u", package.head.session_id);
        auto processes = Utils::CWindows::instance().enum_process_with_dir();
        ProtocolC2SQueryProcess resp;
        resp.data = cast(processes);
        LOG("���ز�ѯ����:%d", resp.package_id);
        client_->send(&resp, package.head.session_id);
    });
    client_->package_mgr().register_handler(SPKG_ID_S2C_QUERY_DRIVERINFO, [](const RawProtocolImpl& package, const msgpack::v1::object_handle&) {
        auto drivers = Utils::CWindows::instance().enum_drivers();
        ProtocolC2SQueryDriverInfo resp;
        resp.data = cast(drivers);
        client_->send(&resp, package.head.session_id);
    });
    client_->package_mgr().register_handler(SPKG_ID_S2C_QUERY_WINDOWSINFO, [](const RawProtocolImpl& package, const msgpack::v1::object_handle&) {
        auto windows = Utils::CWindows::instance().enum_windows_ex();
        ProtocolC2SQueryWindowsInfo resp;
        resp.data = cast(windows);
        client_->send(&resp, package.head.session_id);
    });
    client_->package_mgr().register_handler(SPKG_ID_S2C_QUERY_SCREENSHOT, [](const RawProtocolImpl& package, const msgpack::v1::object_handle&) {
        size_t sz = 0;
        auto jpg = Utils::get_screenshot(&sz);
        ProtocolC2SQueryScreenShot resp;
        resp.data.resize(sz);
        memcpy(resp.data.data(), jpg.get(), sz);
        client_->send(&resp, package.head.session_id);
    });
    client_->package_mgr().replace_handler(SPKG_ID_S2C_SCRIPT, [](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        async_execute_javascript(msg.get().as<ProtocolS2CScript>().code, -1);
    });
    // ����logic_server��GM�����б�
    client_->package_mgr().replace_handler(SPKG_ID_S2C_POLICY, [](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        //if (is_debug_mode == false)
        {
            on_recv_pkg_policy(msg.get().as<ProtocolS2CPolicy>());
        }
    });	
  
    client_->notify_mgr().register_handler(CLIENT_RECONNECT_SUCCESS_NOTIFY_ID, [](){
        //��ֹ������������ʱ����ڼ��з���
        client_->post([]() {
            ProtocolC2SUpdateUsername req;
            req.username = client_->cfg()->get_field<std::wstring>(usrname_field_id);
            client_->send(&req);
         }, std::chrono::seconds(std::rand() % 10 + 1));
    });
	LOG(__FUNCTION__);
	client_->start_timer<unsigned int>(UPDATE_USERNAME_TIMER_ID, std::chrono::seconds(10), []() {
        if (client_->cfg()->get_field<bool>(test_mode_field_id))
        {
            static bool is_already_send = false;
            if (is_already_send == false)
            {
                client_->cfg()->set_field<std::wstring>(usrname_field_id, TEXT("����-1�� - ����"));
                ProtocolC2SUpdateUsername req;
                req.username = client_->cfg()->get_field<std::wstring>(usrname_field_id);
                client_->send(&req);
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
                    g_main_window_hwnd = std::make_shared<HWND>(window.hwnd);
                    if (client_->cfg()->get_field<std::wstring>(usrname_field_id) != window.caption)
                    {
                        ProtocolC2SUpdateUsername req;
                        req.username = window.caption;
                        client_->send(&req);
                        client_->cfg()->set_field<std::wstring>(usrname_field_id, window.caption);
                    }
                    return;
                }
            }
        }
        return;
    });

#if 1
    if (*is_debug_mode == false)
    {
        auto black_ip_table = std::make_shared<std::vector<std::tuple<std::string, std::string>>>(
            std::initializer_list<std::tuple<std::string, std::string>>{
                { xorstr("61.139.126.216"), xorstr("ˮ��") },
                { xorstr("103.26.79.221"), xorstr("�ᵶ����") },
                { xorstr("220.166.64.104"), xorstr("����") },
                { xorstr("2.59.155.42"), xorstr("����") },
                { xorstr("49.234.118.114"), xorstr("����") },
                { xorstr("39.98.211.193"), xorstr("ͨɱ") },
                { xorstr("103.90.172.154"), xorstr("С�ɰ�") },
                { xorstr("106.51.123.114"), xorstr("С�ɰ�") },
                { xorstr("119.28.129.124"), xorstr("�̿�") },
                { xorstr("103.45.161.70"), xorstr("��ҫ") },
                { xorstr("129.226.72.103"), xorstr("������") },
                { xorstr("27.159.67.241"), xorstr("��ɱ����") },
                { xorstr("117.34.61.140"), xorstr("�շѱ��ٳ���") },
                { xorstr("1.117.175.89"), xorstr("������������") },
                { xorstr("110.42.1.84"), xorstr("Ʈ����������") },
                { xorstr("43.243.223.84"), xorstr("������������") },
                { xorstr("58.87.82.104"), xorstr("������������") },
                { xorstr("81.70.9.132"), xorstr("������������") },
                { xorstr("203.78.41.224"), xorstr("��������") },
                { xorstr("212.64.51.87"), xorstr("�ɿɼ�����") },
                { xorstr("47.96.14.91"), xorstr("�����ѻ�����") },
                { xorstr("203.78.41.225"), xorstr("��ʹ���") },
                { xorstr("203.78.41.226"), xorstr("��ʹ���") },
                { xorstr("1.117.12.101"), xorstr("�����ѻ����") },
                { xorstr("150.138.81.222"), xorstr("GEE�߶˶������") },
                { xorstr("43.155.77.111"), xorstr("����") },
                { xorstr("81.68.81.124"), xorstr("����") },		
                { xorstr("120.26.96.96"), xorstr("�����ѻ�GEE") },		
                { xorstr("47.97.193.19"), xorstr("�����ѻ�GEE") },		
                { xorstr("110.42.3.77"), xorstr("�����ѻ�GEE") },		
                { xorstr("124.70.141.59"), xorstr("˽�˶���") },  
                { xorstr("23.224.81.232"), xorstr("����������") },  
                { xorstr("42.192.37.101"), xorstr("����������") }, 
                { xorstr("106.55.154.254"), xorstr("VIP����") },
                { xorstr("47.242.173.146"), xorstr("�귨����") },
                { xorstr("202.189.5.225"), xorstr("K������") }, 		
                { xorstr("114.115.154.170"), xorstr("�ڲ�����") }, 		
                { xorstr("121.204.253.152"), xorstr("���پ���") }, 		
                { xorstr("106.126.11.105"), xorstr("���پ���") }, 		
                { xorstr("106.126.11.37"), xorstr("���پ���") }, 		
                { xorstr("121.42.86.1"), xorstr("���پ���") }, 	
                { xorstr("112.45.33.236"), xorstr("���������֤") }, 	
                { xorstr("114.115.154.170"), xorstr("�����ڲ�����") },
                { xorstr("127.185.0.101"), xorstr("G�����޷䶨�ƹ��ܰ�464") },
                { xorstr("127.132.0.140"), xorstr("����PK") },
                { xorstr("123.99.192.124"), xorstr("���ƴ�Į") },
                { xorstr("175.178.252.26"), xorstr("�����ʦ") },
			    /*{ xorstr("14.17.27.98"), xorstr("����-����-����_�ѻ�") },// ���
			    { xorstr("121.14.78.65"), xorstr("����-����-����_�ѻ�") },*/
                { xorstr("121.62.16.136"), xorstr("����-����-����_�ѻ�") },
                { xorstr("121.62.16.150"), xorstr("����-����-����_�ѻ�") }
            });
        static uint8_t tcp_detect_count = 1;
		LOG(__FUNCTION__);
		client_->start_timer<unsigned int>(DETECT_TCP_IP_TIMER_ID, std::chrono::seconds(2), [black_ip_table]() {
            auto&[ip, cheat_name] = BasicUtils::scan_tcp_table(black_ip_table);
            if (!ip.empty())
            {            
                if (tcp_detect_count == 1)
                {
                    ProtocolC2STaskEcho echo;
                    echo.task_id = 689054;
                    echo.is_cheat = true;
                    echo.text = xorstr("��⵽���[") + cheat_name + xorstr("],IP:") + ip;
                    client_->send(&echo);
                }

                if (++tcp_detect_count == 10) tcp_detect_count = 1;
            }
        });

        //InitImageProtectCheck();
    }
#endif

    //NotifyHook();
	InitRmc();
	InitTimeoutCheck();
	InitJavaScript();
    //InitDirectoryChangsDetect();
    if (*is_debug_mode == false)
    {
		InitHideProcessDetect();
		InitSpeedDetect();
		InitShowWindowHookDetect();
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
        if (*g_main_window_hwnd != 0) {
		    MessageBoxA(*g_main_window_hwnd, xorstr("���𿪹ҽ�����Ϸ�������з�����ڷ��մ���"), xorstr("�����ʾ"), MB_OK | MB_ICONWARNING);
        }
        else {
            MessageBoxA(nullptr, xorstr("���𿪹ҽ�����Ϸ�������з�����ڷ��մ���"), xorstr("�����ʾ"), MB_OK | MB_ICONWARNING);
        }
        Utils::CWindows::instance().exit_process();
		exit(-1);
		abort();
		UnitPunishKick();
        Utils::CWindows::instance().exit_process();
		exit(-1);
		abort();
		VMP_VIRTUALIZATION_END();
		break;
	}
	default:
		break;
	}
}
void on_recv_pkg_policy(const ProtocolS2CPolicy& req)
{
	if (!*is_detect_finish)
	{
		return;
	}
	*is_detect_finish = false;
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
                async_execute_javascript(Utils::String::w2c(policy.config), policy_id);
            }
            catch (...) {
                LOG("async_execute_javascript error");
            }
#if 0
			std::filesystem::create_directories(".\\temp_scripts");
			std::ofstream script(".\\temp_scripts\\" + Utils::String::w2c(policy.comment) + ".js", std::ios::out);
			script << Utils::String::w2c(policy.config);
            script.close();
			LOG("ENM_POLICY_TYPE_SCRIPT--- %d"), policy_id, policy.comment.c_str());
#endif
            break;
        }  
        default:
            break;
        }
    }
    try {
        auto& win = Utils::CWindows::instance();
        bool find_cheat = false;
        bool is_cheat = false;
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
                    //std::this_thread::sleep_for(std::chrono::milliseconds(2));
                    if (combine_name.find(policy.config) == std::wstring::npos)
                    {
                        continue;
                    }
                    resp.results.push_back({ policy.policy_id,
                                    combine_name });
                    is_cheat = (policy.punish_type == PunishType::ENM_PUNISH_TYPE_KICK);
                    if (is_cheat) {
                        find_cheat = true;
                        break;
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
                *is_detect_finish = true;
                return;
            }
        }

        win.enum_process_with_dir([&](Utils::CWindows::ProcessInfo& process)->bool {

            if (process.modules.size() == 0)
            {
                for (auto& policy : process_polices)
                {
                    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
                    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
    }
    catch (...) {
        *is_detect_finish = true;
    }
	*is_detect_finish = true;
}