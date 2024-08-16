#include "pch.h"
#include "Gate.h"
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

CObserverClientImpl::CObserverClientImpl(asio::io_service& io_, const std::string& auth_key) : super(io_, auth_key)
, user_count_(0)
{
    std::error_code ec;
    cache_dir_ = theApp.m_ExeDir;
    cache_dir_ = cache_dir_ / "log";
    if (fs::is_directory(cache_dir_, ec) == false)
    {
        fs::create_directory(cache_dir_);
    }
    notify_mgr_.register_handler(CLIENT_AUTH_FAILED_NOTIFY_ID, [this]() {
        log(LOG_TYPE_ERROR, TEXT("obclient auth failed"));
    });
#ifndef GATE_ADMIN
    package_mgr_.register_handler(OBPKG_ID_S2C_QUERY_VMP_EXPIRE, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        auto msg = raw_msg.get().as<ProtocolOBS2OBCQueryVmpExpire>();
        theApp.m_WorkIo.post([vmp_expire = msg.vmp_expire]() {
            ((CStatic*)theApp.GetMainFrame()->GetClientView().GetMainBar()->GetDlgItem(IDC_EXPDATE_STATIC))->SetWindowText(vmp_expire.c_str());
        });
    });
#endif
    package_mgr_.register_handler(OBPKG_ID_S2C_SEND, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto req = msg.get().as<ProtocolOBS2OBCSend>().package;
        auto raw_msg = msgpack::unpack((char*)req.body.buffer.data(), req.body.buffer.size());
        if (raw_msg.get().type != msgpack::type::ARRAY) throw msgpack::type_error();
        if (raw_msg.get().via.array.size < 1) throw msgpack::type_error();
        if (raw_msg.get().via.array.ptr[0].type != msgpack::type::POSITIVE_INTEGER) throw msgpack::type_error();
        const auto package_id = raw_msg.get().via.array.ptr[0].as<unsigned int>();
        if (package_id == OBPKG_ID_S2C_SEND)
        {
            log(LOG_TYPE_ERROR, TEXT("嵌套转发"));
            return;
        }
        client_pkg_mgr_.dispatch(package_id, req.head.session_id, req, raw_msg);
    });
    package_mgr_.register_handler(LSPKG_ID_S2C_LOG, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        try
        {
            auto req = raw_msg.get().as<ProtocolLSLCLogPrint>();
#ifdef GATE_ADMIN
            if (!req.silence)
            {
                LogPrint(LogicServerLog, TEXT("%s"), req.text.c_str());
            }
            if (!req.identify.empty())
            {
                log_to_file(req.identify, Utils::w2c(req.text));
            }
#else
            if (req.gm_show)
            {
                if (!req.silence)
                {
                    LogPrint(LogicServerLog, TEXT("%s"), req.text.c_str());
                }
                if (!req.identify.empty())
                {
                    log_to_file(req.identify, Utils::w2c(req.text));
                }
            }
#endif // GATE_ADMIN
        }
        catch (msgpack::v1::type_error)
        {
            TRACE("解析Logic日志失败");
        }
    });
    package_mgr_.register_handler(OBPKG_ID_S2C_LOG, [this](const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        try
        {
            auto req = raw_msg.get().as<ProtocolOBS2OBCLogPrint>();
            if (!req.silence)
            {
                LogPrint(ServiceLog, TEXT("%s"), req.text.c_str());
            }
            if (!req.identify.empty())
            {
                log_to_file(req.identify, Utils::w2c(req.text));
            }
        }
        catch (msgpack::v1::type_error)
        {
            TRACE("解析Service日志失败");
        }
    });
    //client_pkg_mgr_.register_handler(SPKG_ID_C2S_CHECK_PLUGIN, [this](unsigned int sid, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
    //    auto msg = raw_msg.get().as<ProtocolC2SCheckPlugin>();
    //    if (msg.plugin_list.empty())
    //    {
    //        log(LOG_TYPE_DEBUG, _T("未拉起任何云代码"));
    //        return;
    //    }
    //    for (auto&[plugin_hash, module_info] : msg.plugin_list)
    //    {
    //        log(LOG_TYPE_DEBUG, _T("云代码:%08X %s 0x%llX 0x%08X"), plugin_hash,
    //            module_info.module_name.c_str(),
    //            module_info.base,
    //            module_info.size_of_image);
    //    }
    //});
    //client_pkg_mgr_.register_handler(SPKG_ID_C2S_CHECK_PLUGIN, [this](unsigned int sid, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
    //    auto msg = raw_msg.get().as<ProtocolC2SCheckPlugin>();
    //    if (msg.plugin_list.empty())
    //    {
    //        log(LOG_TYPE_DEBUG, _T("未拉起任何云代码"));
    //        return;
    //    }
    //    for (auto&[plugin_hash, module_info] : msg.plugin_list)
    //    {
    //        log(LOG_TYPE_DEBUG, _T("云代码:%08X %s 0x%llX 0x%08X"), plugin_hash,
    //            module_info.module_name.c_str(),
    //            module_info.base,
    //            module_info.size_of_image);
    //    }
    //});

    client_pkg_mgr_.register_handler(SPKG_ID_C2S_QUERY_PROCESS, [this](unsigned int sid, const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg) {
        wchar_t file_name[255];
        _snwprintf_s(file_name, sizeof(file_name) / sizeof(file_name[0]) - 1, TEXT("%s_%d.process"), get_current_time_str().GetBuffer(), SPKG_ID_C2S_QUERY_PROCESS);
        std::ofstream file(cache_dir_ / file_name, std::ios::out | std::ios::binary);
        auto buffer = package.release();
        file.write(buffer.data(), buffer.size());
        file.close();
        OpenDocument(cache_dir_ / file_name);
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
        if (std::filesystem::path(path).filename().extension() == ".jpg")
        {
            theApp.OpenFolderAndSelectFile(path.c_str());
        }
        else
        {
            theApp.OpenDocumentFile(path.c_str());
        }
    });
}
