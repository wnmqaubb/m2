#include "anticheat.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include "lightbone/lighthook.h"

using namespace nlohmann;
DWORD WINAPI check_thread_routine(LPVOID param);
#if (CONFIG_GET_HOST_FROM_CONNECT_HOOK == 1)
LightHook::InlineHook ws2_connect_hook;
decltype(&::connect) connect_ptr = nullptr;
#endif

AntiCheat& AntiCheat::instance()
{
    static AntiCheat instance_;
    return instance_;
}
#if (CONFIG_GET_HOST_FROM_CONNECT_HOOK == 1)
int __stdcall connect_handler(SOCKET s, sockaddr_in *name, int namelen)
{
    auto GetModuleHandleExW = IMPORT(L"kernel32.dll", GetModuleHandleExW);
    auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
    HMODULE module = NULL;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)_ReturnAddress(), &module);
    if (module == GetModuleHandleA(NULL) && AntiCheat::instance().ip_ == L"*")
    {
        AntiCheat::instance().ip_ = Utils::string2wstring(inet_ntoa(name->sin_addr));
        ws2_connect_hook.restore();
    }
    return connect_ptr(s, (sockaddr*)name, namelen);
}
#endif
void AntiCheat::async_call(PAPCFUNC apcfunc,ULONG_PTR data)
{
    VMP_VIRTUALIZATION_BEGIN();
    auto QueueUserAPC = IMPORT(L"kernel32.dll", QueueUserAPC);
    QueueUserAPC(apcfunc, main_thread_, data);
    VMP_VIRTUALIZATION_END();
}

AntiCheat::AntiCheat()
    :NetworkMgr(), TaskMgr()
{
    auto OpenThread = IMPORT(L"kernel32.dll", OpenThread);
    auto GetCurrentThreadId = IMPORT(L"kernel32.dll", GetCurrentThreadId);
    main_thread_ = OpenThread(THREAD_ALL_ACCESS, FALSE, GetCurrentThreadId());
    auto QueueUserWorkItem = IMPORT(L"kernel32.dll", QueueUserWorkItem);
    QueueUserWorkItem(&check_thread_routine, 0, WT_EXECUTEDEFAULT);
}

AntiCheat::~AntiCheat()
{
    auto CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
    CloseHandle(main_thread_);
}

void AntiCheat::load_rsrc(HMODULE module_instance)
{
    AntiCheat& anticheat = AntiCheat::instance();
    auto find_resource = IMPORT(L"kernel32.dll", FindResourceA);
    auto size_of_resource = IMPORT(L"kernel32.dll", SizeofResource);
    auto load_resource = IMPORT(L"kernel32.dll", LoadResource);
    auto msgbox = IMPORT(L"user32.dll", MessageBoxA);

    HRSRC rfs_handle = find_resource(module_instance, "0011", "×Ö·û´®");
    DWORD size_of_rfs = size_of_resource(module_instance, rfs_handle);
    HGLOBAL rfs = load_resource(module_instance, rfs_handle);
    
    json symbols = json::from_msgpack((uint8_t*)rfs, (uint8_t*)rfs + size_of_rfs);
    for (json::iterator itor = symbols.begin(); itor != symbols.end(); itor++)
    {
        if(itor.key() == "GOOGLE")
        {
            continue;
        }
        uint32_t hash = 0;
        _snscanf_s(itor.key().c_str(), itor.key().size(), "%08X", &hash);
        anticheat.symbols_[hash] = itor.value();
    }
    const uint32_t current_function_rva = (uint32_t)&AntiCheat::load_rsrc - (uint32_t)module_instance;
    const uint32_t rfs_current_function_rva = anticheat.symbols_[CT_APHASH("?load_rsrc@AntiCheat@@SAXPAUHINSTANCE__@@@Z")];
    if (rfs_current_function_rva != current_function_rva)
    {
        msgbox(NULL, "RFS load error", "Error", MB_OK);
        return;
    }

    HRSRC ini_handle = find_resource(module_instance, "0010", "×Ö·û´®"); 
    DWORD size_of_ini = size_of_resource(module_instance, ini_handle);
    HGLOBAL ini = load_resource(module_instance, ini_handle);
    std::string ini_str;
    ini_str.resize(size_of_ini);
    memcpy((void*)ini_str.data(), ini, size_of_ini);
    
    std::string ip(ini_str.substr(0, ini_str.find(":")));
    std::string port_str(ini_str.substr(ini_str.find(":")+1));
    uint16_t port = 0;
    _snscanf_s(port_str.c_str(), port_str.size(), "%hu", &port);

    anticheat.ip_ = Utils::string2wstring(ip);
    anticheat.port_ = port;
#if (CONFIG_GET_HOST_FROM_CONNECT_HOOK == 1)
    if (anticheat.ip_ == L"*")
    {
        ws2_connect_hook.install(&::connect, &connect_handler, &connect_ptr);
    }
#endif
}

DWORD WINAPI check_thread_routine(LPVOID param)
{
    VMP_VIRTUALIZATION_BEGIN();
    while(true)
    {
        auto GetTickCount = IMPORT(L"kernel32.dll", GetTickCount);
        auto ExitProcess = IMPORT(L"kernel32.dll", ExitProcess);
        auto Sleep = IMPORT(L"kernel32.dll", Sleep);
        if((GetTickCount() - Singleton<TaskHeartBeat>::getInstance().get_last_tickcount())
                 > 5 * 59.9999 * 1000)
        {
            *(void**)_AddressOfReturnAddress() = ExitProcess;
            break;
        }
        Sleep(59.9998 * 1000);
    }
    return 0;
    VMP_VIRTUALIZATION_END();
}

EnHandleResult AntiCheat::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{
    VMP_VIRTUALIZATION_BEGIN();
    Protocol proto;
    do
    {
        if (proto.unpak(pData, iLength))
        {
            json package;
            proto.get_json(package);
            if (package.find("id") == package.end())
            {
                break;
            }
            Protocol::PackageId id = package["id"];
            if (task_map_.find(id) != task_map_.end())
            {
                try {
                    task_map_[id]->on_recv_package(pSender, dwConnID, package, proto.m_type);
                }
                catch (...)
                {
                    LOG_ERROR("dispatcher error");
                }
            }
            else
            {
                LOG_ERROR("undefined error");
            }
        }
        else
        {
            LOG_ERROR("unpack error");
        }
    } while (0);
    return NetworkMgr::OnReceive(pSender, dwConnID, pData, iLength);
    VMP_VIRTUALIZATION_END();
}
