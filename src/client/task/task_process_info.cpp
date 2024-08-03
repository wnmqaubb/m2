#include "pch.h"
#include "task.h"
#include "utils/utils.h"
#include "protocol.h"

TaskProcessInfo::TaskProcessInfo() 
{
    set_interval(0);
    set_package_id(Protocol::PackageId::PACKAGE_ID_PROCESSINFO);
    handler_.emplace(std::make_pair("get_directories", &TaskProcessInfo::on_subid_get_directories));
    handler_.emplace(std::make_pair("get_threads", &TaskProcessInfo::on_subid_get_threads));
    handler_.emplace(std::make_pair("get_modules", &TaskProcessInfo::on_subid_get_modules));
    handler_.emplace(std::make_pair("get_windows", &TaskProcessInfo::on_subid_get_windows));
    handler_.emplace(std::make_pair("get_processes", &TaskProcessInfo::on_subid_get_processes));
#ifdef SERVER
    send_get_directories = std::bind(&TaskProcessInfo::send_command, this, std::placeholders::_1, "get_directories", std::placeholders::_2);
    send_get_threads = std::bind(&TaskProcessInfo::send_command, this, std::placeholders::_1, "get_threads", std::placeholders::_2);
    send_get_modules = std::bind(&TaskProcessInfo::send_command, this, std::placeholders::_1, "get_modules", std::placeholders::_2);
    send_get_windows = std::bind(&TaskProcessInfo::send_command, this, std::placeholders::_1, "get_windows", -1);
    send_get_process = std::bind(&TaskProcessInfo::send_command, this, std::placeholders::_1, "get_processes", -1);
#endif
}

TaskProcessInfo::~TaskProcessInfo()
{

}

void TaskProcessInfo::on_subid_get_processes(void* sender, uintptr_t conn_id, ProtocolProcessInfo& proto)
{
#ifdef SERVER
    MainWnd->ShowProcessInfo(proto);
#endif
#ifdef CLIENT
    Utils::CWindows::ProcessMap processes = Utils::CWindows::instance().enum_process();
    for (auto& process : processes)
    {
        uint32_t pid = process.second.pid;
        proto.processes.push_back({
                pid,
                Utils::CWindows::instance().get_process_path(pid, processes),
            });
    }

    AntiCheat::instance().send(proto);
#endif
}
void TaskProcessInfo::on_subid_get_modules(void* sender, uintptr_t conn_id, ProtocolProcessInfo& proto)
{
#ifdef SERVER
    MainWnd->m_processInfoDlg.ShowProcessModuleInfo(proto);
#endif 
#ifdef CLIENT
    Utils::CWindows::ProcessInfo process;
    Utils::CWindows::instance().get_process(proto.pid, process);
    for (auto& module : process.modules)
    {
        proto.modules.push_back({ module.path });
    }
    AntiCheat::instance().send(proto);
#endif
}

void TaskProcessInfo::on_subid_get_threads(void* sender, uintptr_t conn_id, ProtocolProcessInfo& proto)
{
#ifdef SERVER
    MainWnd->m_processInfoDlg.ShowProcessThreadInfo(proto);
#endif
#ifdef CLIENT
    Utils::CWindows::ProcessInfo process;
    if (!Utils::CWindows::instance().get_process(proto.pid, process))
        return;

    for (auto &thread : process.threads) 
    {
        uint32_t tid = thread.second.tid;
        uint64_t startaddr = 0;
        Utils::CWindows::ModuleInfo module;
        Utils::CWindows::instance().get_module_from_address(proto.pid, thread.second.start_address, process.modules, module);
        std::wstring owner_module = module.path;
        startaddr = thread.second.start_address;
        proto.threads.push_back({
            tid,
            startaddr,
            owner_module
            });
    }
    AntiCheat::instance().send(proto);
#endif
}
void TaskProcessInfo::on_subid_get_directories(void* sender, uintptr_t conn_id, ProtocolProcessInfo& proto)
{
#ifdef SERVER
    MainWnd->m_processInfoDlg.ShowProcessDirectoryInfo(proto);
#endif
#ifdef CLIENT
    std::filesystem::path walk_path(Utils::CWindows::instance().get_process_path(proto.pid));
    walk_path = walk_path.parent_path();
    for (auto& file: std::filesystem::directory_iterator(walk_path))
    {
        proto.dirs.push_back({
            file.path().filename(),
            file.is_directory()
            });
    }
    AntiCheat::instance().send(proto);
#endif
}
void TaskProcessInfo::on_subid_get_windows(void* sender, uintptr_t conn_id, ProtocolProcessInfo& proto)
{
#ifdef SERVER
    MainWnd->ShowClientWindowDlg(proto);
#endif 
#ifdef CLIENT
    Utils::CWindows::WindowsList windows = Utils::CWindows::instance().enum_windows_ex();
    for (auto& window : windows) 
    {
        proto.windows.push_back({
            window.caption,
            window.class_name,
            window.is_hide_process ? -1 : window.pid,
            window.process_name,
            window.tid
            });
    }
    AntiCheat::instance().send(proto);
#endif
}

#ifdef SERVER
void TaskProcessInfo::send_command(uintptr_t conn_id, const char* command, uint32_t pid)
{
    ProtocolProcessInfo proto;
    proto.subid = command;
    proto.pid = pid;
    MainWnd->Send(conn_id, proto);
}
#endif

void TaskProcessInfo::on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) 
{
    ProtocolProcessInfo proto;
    proto.from_json(package);
    proto.m_type = channel;
#ifdef SERVER
    Utils::log_to_file(conn_id, Utils::CHANNEL_EVENT, GlobalString::LOG::FORMAT_S.c_str(), proto.dump().c_str());
#endif
    if (handler_.find(proto.subid) != handler_.end())
    {
        Handler callback = handler_[proto.subid];
        (this->*callback)(sender, conn_id, proto);
    }
}

void TaskProcessInfo::on_time_proc(uint32_t curtime) {
#ifdef CLIENT
   

#endif
}