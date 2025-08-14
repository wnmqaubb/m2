#include "pch.h"
#include "ClientImpl.h"
#include "version.build"
#include "TaskBasic.h"
#include "../../yk/NewClient/window_role_name.h"
#include <functional>

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
        user_data().set_field(sysver_field_id, (int)CWindows::instance().get_system_version());
        user_data().set_field(is_64bits_field_id, CWindows::instance().is_64bits_system());
        user_data().set_field(cpuid_field_id, Utils::HardwareInfo::get_cpuid());
        user_data().set_field(mac_field_id, Utils::HardwareInfo::get_mac_address());
        user_data().set_field(vol_field_id, Utils::HardwareInfo::get_volume_serial_number());
        user_data().set_field(rev_version_field_id, (int)REV_VERSION);
        user_data().set_field(commited_hash_field_id, std::string(FILE_VERSION_STR));
        user_data().set_field(is_client_field_id, true);
        this->load_uuid();

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
        handshake.is_client = true;
        this->save_uuid(handshake);
		send(&handshake);
		start_timer<unsigned int>(CLIENT_HEARTBEAT_TIMER_ID, heartbeat_duration(), [this]() {
            __try {
			    ProtocolC2SHeartBeat heartbeat;
			    heartbeat.tick = time(nullptr);
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
		}, std::chrono::seconds(2));
		// 发送用户名 防止断开后重连时网关用户名为空
		notify_mgr().dispatch(CLIENT_RECONNECT_SUCCESS_NOTIFY_ID);
	});

	notify_mgr().register_handler(ON_RECV_HEARTBEAT_NOTIFY_ID, [this]() {
		LOG("接收心跳");
	});

	notify_mgr().register_handler(CLIENT_START_NOTIFY_ID, [this]() {
		LOG("客户端初始化成功");
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

    notify_mgr().register_handler(ON_RECV_HANDSHAKE_NOTIFY_ID, [this]() {
        // 用于后台重启后更新用户名
        if (cfg()->get_field<std::wstring>(usrname_field_id) != L"未登录用户") {
            ProtocolC2SUpdateUsername req;
            req.username = cfg()->get_field<std::wstring>(usrname_field_id);
            async_send(&req);
        }
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

// 主窗口检测函数
void CClientImpl::monitor_main_window() {
    std::lock_guard<std::mutex> lock(g_windowMutex);
    HWND hCurrentWindow = g_main_window_hwnd.load();

    auto& wrn = window_role_name_hook::get_instance();

    // 双重检查窗口有效性
    if (hCurrentWindow && IsWindow(hCurrentWindow)) {
        if (!wrn.get_is_hook_installed()) {
            wrn.init(hCurrentWindow);
        }
        return;
    }

    // 枚举查找主窗口
    struct WindowSearchContext {
        DWORD pid;
        HWND result = nullptr;
    } ctx{ g_currentPid };

    ::EnumDesktopWindows(nullptr, [](HWND hwnd, LPARAM lParam) -> BOOL {
        auto& ctx = *reinterpret_cast<WindowSearchContext*>(lParam);

        DWORD windowPid = 0;
        ::GetWindowThreadProcessId(hwnd, &windowPid);
        if (windowPid != ctx.pid) return TRUE;


        wchar_t classBuf[256] = {};
        ::GetClassName(hwnd, classBuf, ARRAYSIZE(classBuf));
        if (_wcsicmp(classBuf, main_window_class.c_str()) != 0) return TRUE;

        if (!::IsWindowVisible(hwnd)) return TRUE;

        ctx.result = hwnd;

        return FALSE;
    }, reinterpret_cast<LPARAM>(&ctx));

    // 更新缓存
    if (ctx.result) {
        g_main_window_hwnd.store(ctx.result);

        if (window_role_name_hook::get_instance().init(ctx.result)) {
            window_role_name_hook::get_instance().request_title();
        }
    }
}

// 处理消息队列中的标题更新
void CClientImpl::process_title_messages() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_USER + 100) {
            std::unique_ptr<std::wstring> pTitle(reinterpret_cast<std::wstring*>(msg.lParam));

            if (!pTitle->empty() && *pTitle != g_lastKnownTitle) {
                g_lastKnownTitle = *pTitle;
                if (g_roleNameCallback) {
                    g_roleNameCallback(*pTitle);
                }
            }
        }
    }
}

// 窗口监控线程（优化版）
void CClientImpl::window_monitor_thread() {
    auto& wrn = window_role_name_hook::get_instance();
    wrn.set_rev_thread_id(GetCurrentThreadId());

    // 1. 检测主窗口
    monitor_main_window();

    // 2. 请求标题更新
    if (g_main_window_hwnd.load() && IsWindow(g_main_window_hwnd.load()))
    {
        wrn.request_title();
    }

    // 3. 处理消息队列
    process_title_messages();
}

// 初始化窗口监控
void CClientImpl::init_role_monitor() {
    // 一次性初始化（移出定时器循环）
    g_currentPid = ::GetCurrentProcessId();

    // 启动定时器
    start_timer("window_monitor_thread", std::chrono::seconds(1), [this]() {
        window_monitor_thread();
    });

    // 设置角色名变化回调
    set_role_name_callback([this](const std::wstring& roleName) {

        if (roleName.find(L" - ") != std::wstring::npos) {

            /*if (get_timer_interval("window_monitor_thread") != std::chrono::seconds(30))
            {
                set_timer_interval("window_monitor_thread", std::chrono::seconds(30));
            }*/
            cfg()->set_field<std::wstring>(usrname_field_id, roleName);
            ProtocolC2SUpdateUsername req;
            req.username = roleName;
            async_send(&req);
        }
    });
}