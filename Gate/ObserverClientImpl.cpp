#include "pch.h"
#ifdef GATEF
    #include "GateF.h"
#else
    #include "Gate.h"
#endif
#include "ObserverClientImpl.h"

namespace fs = std::filesystem;

CString get_current_time_str()
{
    CTime tm = CTime::GetCurrentTime();
    return tm.Format(_T("%H_%M_%S"));
}

CString get_current_date_str()
{
    CTime tm = CTime::GetCurrentTime();
    return tm.Format(_T("%Y-%m-%d"));
}

/**
 * @brief 构造函数，初始化观察者客户端
 * @param io_ 异步IO服务
 * @param auth_key 认证密钥
 */
CObserverClientImpl::CObserverClientImpl(asio::io_service& io_) : super(), user_count_(0)
{
    // 初始化日志缓存目录
    std::error_code ec;
    cache_dir_ = theApp.m_ExeDir;
    cache_dir_ = cache_dir_ / "log";
    if (fs::is_directory(cache_dir_, ec) == false)
    {
        // 如果目录不存在，则创建目录
        fs::create_directory(cache_dir_);
    }
    // 注册认证失败通知处理函数
    notify_mgr_.register_handler(CLIENT_AUTH_FAILED_NOTIFY_ID, [this]() {
        log(LOG_TYPE_ERROR, TEXT("obclient auth failed"));
    });
#ifndef GATE_ADMIN
    // 注册认证失败通知处理函数
    //notify_mgr_.register_handler(ON_RECV_HEARTBEAT_NOTIFY_ID, [this]() {
    //    theApp.is_service_stauts = true;
    //});
    // 注册处理包管理器的处理函数，用于处理OBPKG_ID_S2C_QUERY_VMP_EXPIRE包
    package_mgr_.register_handler(OBPKG_ID_S2C_QUERY_VMP_EXPIRE, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        // 从原始消息中解析出ProtocolOBS2OBCQueryVmpExpire对象
        auto msg = raw_msg.get().as<ProtocolOBS2OBCQueryVmpExpire>();
        // 将处理逻辑投递到工作线程中执行
        theApp.m_WorkIo.post([vmp_expire = msg.vmp_expire]() {
#ifdef GATEF
            // 在GateF模式下，更新界面上的过期时间显示        
            ((CStatic*)theApp.GetMainFrame()->m_games_dlg->GetDlgItem(IDC_EXPDATE_STATIC))->SetWindowText(vmp_expire.c_str());
#else
            // 在非GateF模式下，更新界面上的过期时间显示
            ((CStatic*)theApp.GetMainFrame()->GetClientView().GetMainBar()->GetDlgItem(IDC_EXPDATE_STATIC))->SetWindowText(vmp_expire.c_str());
#endif
        });
    });
#endif
    // 注册处理包管理器的处理函数，用于处理OBPKG_ID_S2C_SEND包
    package_mgr_.register_handler(OBPKG_ID_S2C_SEND, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        // 从原始消息中解析出ProtocolOBS2OBCSend对象
        auto req = msg.get().as<ProtocolOBS2OBCSend>().package;
        // 从消息体中解包出原始消息对象
        auto raw_msg = msgpack::unpack((char*)req.body.buffer.data(), req.body.buffer.size());
        // 检查解包后的消息类型是否为数组
        if (raw_msg.get().type != msgpack::type::ARRAY) throw msgpack::type_error();
        // 检查数组大小是否小于1
        if (raw_msg.get().via.array.size < 1) throw msgpack::type_error();
        // 检查数组第一个元素的类型是否为正整数
        if (raw_msg.get().via.array.ptr[0].type != msgpack::type::POSITIVE_INTEGER) throw msgpack::type_error();
        // 从数组中解析出包ID
        const auto package_id = raw_msg.get().via.array.ptr[0].as<unsigned int>();
        // 如果包ID是OBPKG_ID_S2C_SEND，表示嵌套转发，记录错误日志并返回
        if (package_id == OBPKG_ID_S2C_SEND)
        {
            log(LOG_TYPE_ERROR, TEXT("嵌套转发"));
            return;
        }
        // 调用client_pkg_mgr_的dispatch方法处理包
        client_pkg_mgr_.dispatch(package_id, req.head.session_id, req, raw_msg);
    });
    // 注册处理包管理器的处理函数，用于处理LSPKG_ID_S2C_LOG包
    package_mgr_.register_handler(LSPKG_ID_S2C_LOG, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        try
        {
            // 从原始消息中解析出ProtocolLSLCLogPrint对象
            auto req = raw_msg.get().as<ProtocolLSLCLogPrint>();
            // 获取日志消息内容
            auto msg = req.text;
            // 如果需要惩罚
            if (req.punish_flag)
            {
#ifndef GATE_ADMIN
                // 获取当前时间并格式化为字符串
                std::time_t now_time = time(0);
                char time_str[MAX_PATH] = { 0 };
                tm tm_;
                localtime_s(&tm_, &now_time);
                strftime(time_str, sizeof(time_str) / sizeof(time_str[0]) - 1, "%m-%d %H:%M:%S", &tm_);
                // 构造日志消息
                std::string result;
                result = result + "[事件]" + time_str + "|";
#endif
                // 记录日志到惩罚文件
                log_to_punish_file(Utils::w2c(msg));
            }
#ifdef GATE_ADMIN
            // 如果不需要静默
            if (!req.silence)
            {
                // 记录日志到LogicServerLog
                LogPrint(LogicServerLog, TEXT("%s"), msg.c_str());
            }
            // 如果标识不为空
            if (!req.identify.empty())
            {
                // 记录日志到指定文件
                log_to_file(req.identify, Utils::w2c(msg));
            }
#else
            // 如果需要GM显示
            if (req.gm_show)
            {
                // 如果不需要静默且不需要惩罚
                if (!req.silence && !req.punish_flag)
                {
                    // 记录日志到LogicServerLog
                    LogPrint(LogicServerLog, TEXT("%s"), msg.c_str());
                }
                // 如果标识不为空
                if (!req.identify.empty())
                {
                    // 记录日志到指定文件
                    log_to_file(req.identify, Utils::w2c(msg));
                }
            }
#endif // GATE_ADMIN
        }
        catch (msgpack::v1::type_error)
        {
            // 解析Logic日志失败，记录错误信息
            TRACE("解析Logic日志失败");
        }
    });
    // 注册处理包管理器的处理函数，用于处理OBPKG_ID_S2C_LOG包
    package_mgr_.register_handler(OBPKG_ID_S2C_LOG, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        try
        {
            // 从原始消息中解析出ProtocolOBS2OBCLogPrint对象
            auto req = raw_msg.get().as<ProtocolOBS2OBCLogPrint>();
            // 如果不需要静默
            if (!req.silence)
            {
                // 记录日志到ServiceLog
                LogPrint(ServiceLog, TEXT("%s"), req.text.c_str());
            }
            // 如果标识不为空
            if (!req.identify.empty())
            {
                // 记录日志到指定文件
                log_to_file(req.identify, Utils::w2c(req.text));
            }
        }
        catch (msgpack::v1::type_error)
        {
            // 解析Service日志失败，记录错误信息
            TRACE("解析Service日志失败");
        }
    });
#ifdef GATE_ADMIN
    // 注册处理包管理器的处理函数，用于处理SPKG_ID_C2S_CHECK_PLUGIN包
    client_pkg_mgr_.register_handler(SPKG_ID_C2S_CHECK_PLUGIN, [this](unsigned int sid, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        // 从原始消息中解析出ProtocolC2SCheckPlugin对象
        auto msg = raw_msg.get().as<ProtocolC2SCheckPlugin>();
        // 如果插件列表为空
        if (msg.plugin_list.empty())
        {
            // 记录调试日志
            log(LOG_TYPE_DEBUG, _T("未拉起任何云代码"));
            return;
        }
        // 遍历插件列表，记录每个插件的信息
        for (auto&[plugin_hash, module_info] : msg.plugin_list)
        {
            log(LOG_TYPE_DEBUG, _T("云代码:%08X %s 0x%llX 0x%08X"), plugin_hash,
                module_info.module_name.c_str(),
                module_info.base,
                module_info.size_of_image);
        }
    });
    client_pkg_mgr_.register_handler(SPKG_ID_C2S_QUERY_DRIVERINFO, [this](unsigned int sid, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        wchar_t file_name[255];
        _snwprintf_s(file_name, sizeof(file_name) / sizeof(file_name[0]) - 1, TEXT("%s_%d.driver"), get_current_time_str().GetBuffer(), SPKG_ID_C2S_QUERY_DRIVERINFO);
        std::ofstream file(cache_dir_ / file_name, std::ios::out | std::ios::binary);
        auto buffer = package.release();
        file.write(buffer.data(), buffer.size());
        file.close();
        OpenDocument(cache_dir_ / file_name);
    });
    client_pkg_mgr_.register_handler(SPKG_ID_C2S_QUERY_WINDOWSINFO, [this](unsigned int sid, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        wchar_t file_name[255];
        _snwprintf_s(file_name, sizeof(file_name) / sizeof(file_name[0]) - 1, TEXT("%s_%d.window"), get_current_time_str().GetBuffer(), SPKG_ID_C2S_QUERY_WINDOWSINFO);
        std::ofstream file(cache_dir_ / file_name, std::ios::out | std::ios::binary);
        auto buffer = package.release();
        file.write(buffer.data(), buffer.size());
        file.close();
        OpenDocument(cache_dir_ / file_name);
    });
#endif // GATE_ADMIN   

    client_pkg_mgr_.register_handler(SPKG_ID_C2S_QUERY_PROCESS, [this](unsigned int sid, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        wchar_t file_name[255];
        _snwprintf_s(file_name, sizeof(file_name) / sizeof(file_name[0]) - 1, TEXT("%s_%d.process"), get_current_time_str().GetBuffer(), SPKG_ID_C2S_QUERY_PROCESS);
        std::ofstream file(cache_dir_ / file_name, std::ios::out | std::ios::binary);
        auto buffer = package.release();
        file.write(buffer.data(), buffer.size());
        file.close();
        OpenDocument(cache_dir_ / file_name);
        });

    client_pkg_mgr_.register_handler(SPKG_ID_C2S_QUERY_SCREENSHOT, [this](unsigned int sid, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        wchar_t file_name[255];
        _snwprintf_s(file_name, sizeof(file_name) / sizeof(file_name[0]) - 1, TEXT("%s_%d.jpg"), get_current_time_str().GetBuffer(), SPKG_ID_C2S_QUERY_SCREENSHOT);
        std::filesystem::path filepath(theApp.m_ExeDir);
        filepath /= "玩家截图";
        if (!std::filesystem::exists(filepath))
        {
            std::filesystem::create_directory(filepath);
        }
        filepath /= get_current_date_str().GetBuffer();
        if (!std::filesystem::exists(filepath))
        {
            std::filesystem::create_directory(filepath);
        }
        std::ofstream file(filepath / file_name, std::ios::out | std::ios::binary);
        auto req = raw_msg.get().as<ProtocolC2SQueryScreenShot>();
        file.write((char*)req.data.data(), req.data.size());
        file.close();
        OpenDocument(filepath / file_name);
        });
}

void CObserverClientImpl::log(int type, LPCTSTR format, ...)
{
    CString buf;
    va_list ap;
    va_start(ap, format);
    buf.FormatV(format, ap);
    va_end(ap);
    LogPrint(ObserverClientLog, _T("%s"), buf);
}

void CObserverClientImpl::OpenDocument(const std::wstring& path)
{
    theApp.m_WorkIo.post([path]() {
        auto& ext = std::filesystem::path(path).filename().extension();
        if (ext == ".jpg")
        {
            theApp.OpenFolderAndSelectFile(path.c_str());
		}
#ifdef GATEF
		else if (ext == ".process")
		{
            theApp.GetMainFrame()->SwitchToTab(3);
			//theApp.GetMainFrame()->m_polices_dlg->RefreshViewList();
            //theApp.GetMainFrame()->m_process_info_dlg->ShowWindow(SW_SHOW);
			theApp.GetMainFrame()->m_process_info_dlg->LoadFile(path);
		}
#else
        else
        {
            theApp.OpenDocumentFile(path.c_str());
        }
#endif
    });
}
