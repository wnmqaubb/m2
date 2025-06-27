#include "pch.h"
#include "ClientImpl.h"
#include "version.build"
#include "TaskBasic.h"

using namespace Utils;
extern std::shared_ptr<asio2::timer> g_timer;

const std::wstring main_window_class = L"TFrmMain"; // 替换为实际类名
std::wstring g_lastKnownTitle;
#ifdef LOG_SHOW
#define log(LOG_TYPE,x,...) log(LOG_TYPE, x, __VA_ARGS__ )
#else
#define log(LOG_TYPE,x,...)
#endif
CClientImpl::CClientImpl() : super()
{
	char path[MAX_PATH] = { 0 };
	GetModuleFileNameA(GetModuleHandleA(NULL), path, sizeof(path));
	exe_path_ = path;
	cache_dir_ = exe_path_.parent_path() / "cache";
	std::error_code ec;
	if (fs::is_directory(cache_dir_, ec) == false)
	{
		fs::create_directory(cache_dir_, ec);
	}
	notify_mgr().register_handler(CLIENT_DISCONNECT_NOTIFY_ID, [this]() {
		LOG("失去连接");
	});
	notify_mgr().register_handler(CLIENT_CONNECT_FAILED_NOTIFY_ID, [this]() {
		auto ec = asio2::get_last_error();
		LOG("连接失败: %s ", ec.message().c_str());
	});
	notify_mgr().register_handler(CLIENT_CONNECT_SUCCESS_NOTIFY_ID, [this]() {
		LOG("握手");
		ProtocolC2SHandShake handshake;
		memcpy(&handshake.uuid, uuid().data, sizeof(handshake.uuid));
		handshake.system_version = std::any_cast<int>(user_data().get_field(sysver_field_id));
		handshake.is_64bit_system = std::any_cast<bool>(user_data().get_field(is_64bits_field_id));
		handshake.cpuid = std::any_cast<std::wstring>(user_data().get_field(cpuid_field_id));
		handshake.mac = std::any_cast<std::wstring>(user_data().get_field(mac_field_id));
		handshake.volume_serial_number = std::any_cast<std::wstring>(user_data().get_field(vol_field_id));
		handshake.rev_version = std::any_cast<int>(user_data().get_field(rev_version_field_id));
		handshake.commited_hash = std::any_cast<std::string>(user_data().get_field(commited_hash_field_id));
		handshake.pid = GetCurrentProcessId();
		this->save_uuid(handshake);
		send(&handshake);
		g_timer->start_timer<unsigned int>(CLIENT_HEARTBEAT_TIMER_ID, heartbeat_duration(), [this]() {
            __try {
			    ProtocolC2SHeartBeat heartbeat;
			    heartbeat.tick = time(0);
			    send(&heartbeat);
			    LOG("发送心跳");
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                LOG("线程异常: %s|%s|%d|0x%X", __FILE__, __FUNCTION__, __LINE__, GetExceptionCode());
            }
		});
		post([this]() {
			if (!is_loaded_plugin())
			{
				LoadPlugin();
			}
			}, std::chrono::milliseconds(2000));
		// 发送用户名 防止断开后重连时网关用户名为空
		notify_mgr().dispatch(CLIENT_RECONNECT_SUCCESS_NOTIFY_ID);
	});

    notify_mgr().register_handler(CLIENT_RECONNECT_SUCCESS_NOTIFY_ID, []() {
        //防止重启服务器的时候过于集中发包
        client_->post([]() {
            ProtocolC2SUpdateUsername req;
            req.username = client_->cfg()->get_field<std::wstring>(usrname_field_id);
            client_->send(&req);
        }, std::chrono::seconds(std::rand() % 10 + 1));
    });

	notify_mgr().register_handler(ON_RECV_HEARTBEAT_NOTIFY_ID, [this]() {
		LOG("接收心跳");
	});

	notify_mgr().register_handler(CLIENT_START_NOTIFY_ID, [this]() {
		LOG("客户端初始化成功");
		user_data().set_field(sysver_field_id, (int)CWindows::instance().get_system_version());
		user_data().set_field(is_64bits_field_id, CWindows::instance().is_64bits_system());
		user_data().set_field(cpuid_field_id, Utils::HardwareInfo::get_cpuid());
		user_data().set_field(mac_field_id, Utils::HardwareInfo::get_mac_address());
		user_data().set_field(vol_field_id, Utils::HardwareInfo::get_volume_serial_number());
		user_data().set_field(rev_version_field_id, (int)REV_VERSION);
		user_data().set_field(commited_hash_field_id, std::string(VER2STR(COMMITED_HASH)));
		this->load_uuid();
        init_role_monitor();
	});
	package_mgr().register_handler(SPKG_ID_S2C_PUNISH, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
		//switch (msg.get().as<ProtocolS2CPunish>().type) 
		//{
		//	case PunishType::ENM_PUNISH_TYPE_KICK:
		//		Utils::CWindows::instance().exit_process();
		//		break;
		//	default:
		//		break;
		//}
	}); 
}

void CClientImpl::on_recv(unsigned int package_id, const RawProtocolImpl& package, const msgpack::v1::object_handle&)
{
	LOG("收到:%d", package_id);
}

void CClientImpl::load_uuid()
{
	std::wstring volume_serial_number = std::any_cast<std::wstring>(user_data().get_field(vol_field_id));
	unsigned int volume_serial_number_hash_val = ApiResolver::hash(volume_serial_number.c_str(), volume_serial_number.size());
	std::ifstream file(cache_dir_ / std::to_string(volume_serial_number_hash_val).c_str(), std::ios::in | std::ios::binary);
	if (file.is_open())
	{
		std::stringstream ss;
		ss << file.rdbuf();
		auto str = ss.str();
		auto handshake = ProtocolC2SHandShake::load(str.data(), str.size());
		if (handshake)
		{
			memcpy(&uuid_, handshake->uuid, sizeof(handshake->uuid));
		}
		file.close();
	}
}

void CClientImpl::save_uuid(const ProtocolC2SHandShake& handshake)
{
	std::wstring volume_serial_number = std::any_cast<std::wstring>(user_data().get_field(vol_field_id));
	unsigned int volume_serial_number_hash_val = ApiResolver::hash(volume_serial_number.c_str(), volume_serial_number.size());
	std::ofstream file(cache_dir_ / std::to_string(volume_serial_number_hash_val).c_str(), std::ios::out | std::ios::binary);
	if (file.is_open())
	{
		auto str = handshake.dump();
		file.write(str.data(), str.size());
		file.close();
	}
}

// 注册角色名变更回调
void CClientImpl::set_role_name_callback(std::function<void(const std::wstring&)> callback) {
    g_roleNameCallback = callback;
}

// 安全的窗口标题获取
std::wstring GetWindowTextSafe(HWND hwnd, int max_retries = 2) {
    constexpr int timeoutMs = 50; // 每次尝试100ms超时
    wchar_t title[MAX_PATH] = {};
    for (int attempt = 0; attempt < max_retries; ++attempt) {
        // 使用SendMessageTimeout
        LRESULT res = ::SendMessageTimeoutW(
            hwnd,
            WM_GETTEXT,
            MAX_PATH - 1,
            reinterpret_cast<LPARAM>(title),
            SMTO_ABORTIFHUNG | SMTO_BLOCK | SMTO_ERRORONEXIT,
            timeoutMs,
            nullptr
        );

        if (res == 0 || title[0] == L'\0') {
            // 备选方案：使用GetWindowText避免阻塞
            int len = GetWindowTextW(hwnd, title, MAX_PATH);
            if (len <= 0) title[0] = L'\0';
        }

        return title;
    }

    // 重试后仍然失败
    return L"";
}

// 主窗口检测函数
void CClientImpl::monitor_main_window() {
    HWND hCurrentWindow = g_main_window_hwnd.load();

    // 1. 检查当前缓存的主窗口是否有效
    if (hCurrentWindow && IsWindow(hCurrentWindow)) {
        // 获取当前窗口标题
        std::wstring currentTitle = GetWindowTextSafe(hCurrentWindow);

        // 2. 检测标题变化（并忽略空标题）
        if (!currentTitle.empty() && currentTitle != g_lastKnownTitle) {
            g_lastKnownTitle = currentTitle;

            // 触发角色名变更回调
            if (g_roleNameCallback) {
                g_roleNameCallback(currentTitle);
            }
        }
        return;
    }

    // 3. 缓存无效时重新查找窗口
    std::lock_guard<std::mutex> lock(g_windowMutex);

    // 双重检查
    if (g_main_window_hwnd.load() && IsWindow(g_main_window_hwnd.load())) {
        return;
    }

    // 枚举查找主窗口
    struct WindowSearchContext {
        DWORD pid;
        HWND result = nullptr;
    } ctx{ g_currentPid };

    ::EnumDesktopWindows(nullptr, [](HWND hwnd, LPARAM lParam) -> BOOL {
        auto& ctx = *reinterpret_cast<WindowSearchContext*>(lParam);

        // 检查进程ID匹配
        DWORD windowPid = 0;
        ::GetWindowThreadProcessId(hwnd, &windowPid);
        if (windowPid != ctx.pid) return TRUE;

        // 检查类名匹配
        wchar_t classBuf[256] = {};
        ::GetClassName(hwnd, classBuf, ARRAYSIZE(classBuf));
        if (wcscmp(classBuf, main_window_class.c_str()) != 0) return TRUE;

        // 检查可见性（主窗口通常可见）
        if (!::IsWindowVisible(hwnd)) return TRUE;

        // 找到目标窗口
        ctx.result = hwnd;

        // 获取当前标题（可能包含角色名）        
        g_lastKnownTitle = GetWindowTextSafe(hwnd);

        return FALSE; // 找到后停止枚举
    }, reinterpret_cast<LPARAM>(&ctx));

    // 更新缓存
    if (ctx.result) {
        g_main_window_hwnd.store(ctx.result);

        // 首次发现窗口且标题非空时触发回调
        if (!g_lastKnownTitle.empty() && g_roleNameCallback) {
            g_roleNameCallback(g_lastKnownTitle);
        }
    }
}

// 窗口监控线程
void CClientImpl::window_monitor_thread() {
    // 初始化进程ID
    g_currentPid = ::GetCurrentProcessId();

    while (true) {
        // 每10秒检测一次
        monitor_main_window();

        if (is_stop_) return;
        Sleep(15000);
    }
}

// 在应用程序初始化时调用
void CClientImpl::init_role_monitor() {
    // 启动监控线程
    monitor_thread_ = std::thread([this]() {
        window_monitor_thread();
    });
    monitor_thread_.detach();

    // 设置角色名变化回调
    set_role_name_callback([this](const std::wstring& roleName) {
        // 处理角色名变更
        if (roleName.find(L" - ") != std::wstring::npos)
        {
            cfg()->set_field<std::wstring>(usrname_field_id, roleName);
            ProtocolC2SUpdateUsername req;
            req.username = roleName;
            send(&req);
            return true;
        }
    });
}