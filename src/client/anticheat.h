#pragma once
#include "network_mgr.h"
#include "protocol.h"
#include "task/task.h"
#include "task_mgr.h"
#include <map>
#include <xstring>

#define CONFIG_GET_HOST_FROM_CONNECT_HOOK 0

class AntiCheat : public NetworkMgr, public TaskMgr
{
public:
    AntiCheat();
    ~AntiCheat();
    using ReflectiveSymbolTable = std::map<uint32_t, uint32_t>;
    static AntiCheat& instance();
    void async_call(PAPCFUNC apcfunc, ULONG_PTR data);
    static void load_rsrc(HMODULE module_instance);
    virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) override;
    ReflectiveSymbolTable symbols_;
    HANDLE main_thread_;
    std::wstring ip_;
    uint16_t port_;
};