#pragma once
#include "task_mgr.h"
#include <string>
#include <xstring>
#include <fstream>
#include <vector>
#include <map>
#include <HPSocket.h>
#include <singleton.hpp>
#ifdef CLIENT
#include "anticheat.h"
#endif
#ifdef SERVER
#include "framework.h"
#include "agent.h"
#include "agentDlg.h"
#include "afxdialogex.h"
#include "ProcessInfoDlg.h"
#include "utils/utils.h"
#define MainWnd ((CagentDlg*)theApp.m_pMainWnd)
using namespace Global;
using namespace SubProto;
#endif


class TaskPolicy : public Task
{
public:
    TaskPolicy();
    ~TaskPolicy();
    void multiclient_detect();
    void dectection_cheat(uintptr_t conn_id);
    void punish_player(uintptr_t conn_id, SubProto::Policy &policy, std::wstring reason);
    virtual void on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) override;
#ifdef SERVER
    Utils::AsyncExecute async_;
#endif
};


class TaskApiHook : public Task
{
public:
    TaskApiHook();
    ~TaskApiHook();
    void show_window_hook();
    virtual void on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) override;
};

class TaskHeartBeat : public Task
{
public:
    TaskHeartBeat();
    ~TaskHeartBeat();
    virtual void on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) override;
    virtual void on_time_proc(uint32_t curtime) override;
    void set_last_recv_tick_count(uint32_t recv_tick_count);
    uint32_t get_last_recv_tick_count();
private:
    uint32_t last_recv_tick_count;
};

class TaskScreenShot : public Task
{
public:
    TaskScreenShot();
    ~TaskScreenShot();
    virtual void on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) override;
    virtual void on_time_proc(uint32_t curtime) override;
    virtual void trigger() override;
    void send(uintptr_t conn_id, bool silent);
    void send_to_all();
};


class TaskProcessInfo : public Task 
{
public:
    TaskProcessInfo();
    ~TaskProcessInfo();
    virtual void on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) override;
    virtual void on_time_proc(uint32_t curtime) override;

    void on_subid_get_processes(void* sender, uintptr_t conn_id, ProtocolProcessInfo& package);
    void on_subid_get_modules(void* sender, uintptr_t conn_id, ProtocolProcessInfo& package);
    void on_subid_get_threads(void* sender, uintptr_t conn_id, ProtocolProcessInfo& package);
    void on_subid_get_directories(void* sender, uintptr_t conn_id, ProtocolProcessInfo& package);
	void on_subid_get_windows(void* sender, uintptr_t conn_id, ProtocolProcessInfo& package);
	void send_command(uintptr_t conn_id, const char* command, uint32_t pid);
	std::function<void(uintptr_t conn_id)> send_get_windows;
    std::function<void(uintptr_t conn_id, uint32_t pid)> send_get_directories;
    std::function<void(uintptr_t conn_id, uint32_t pid)> send_get_threads;
    std::function<void(uintptr_t conn_id, uint32_t pid)> send_get_modules;
    std::function<void(uintptr_t conn_id)> send_get_process;
    using Handler = decltype(&TaskProcessInfo::on_subid_get_processes);
    using HandlerMap = std::map<std::string, Handler>;
    HandlerMap handler_;
};

class TaskHardware : public Task 
{
public:
	TaskHardware();
	~TaskHardware();
	virtual void on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) override;
	virtual void on_time_proc(uint32_t curtime) override;
    virtual void trigger() override;
    void send_to_all();
};



class TaskShellcode : public Task 
{
public:
    TaskShellcode();
    ~TaskShellcode();
    virtual void on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) override;
    virtual void on_time_proc(uint32_t curtime) override;
	virtual void trigger() override;
    void read_shell_file(const std::string & file, std::string & shellcode);
    void shellcode_init();
    void compress_shellcode(const std::string& shellcode, ProtocolShellCode& proto);
	void send_to_all(const std::string& shellcode);
	void send(CONNID connid, const std::string& shellcode);
	void send_all_shell(CONNID connid);
};


class TaskExample : public Task 
{
public:
    TaskExample();
    ~TaskExample();
    virtual void on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) override;
    virtual void on_time_proc(uint32_t curtime) override;
};

class TaskJudgment : public Task
{
public:
    TaskJudgment();
    ~TaskJudgment();
    virtual void on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) override;
    virtual void on_time_proc(uint32_t curtime) override;
    void on_subid_msgbox(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel);
    void on_subid_bsod(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel);
    void on_subid_exit_game(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel);
    void on_subid_back_game_lazy(void * sender, uintptr_t conn_id, json & package, Protocol::PackageType channel);
    void on_subid_exit_game_lazy(void * sender, uintptr_t conn_id, json & package, Protocol::PackageType channel);
    void send_command(uintptr_t conn_id, const char* command, const wchar_t* data, uint32_t lazy_time = 0);
    void send_msgbox(uintptr_t conn_id, std::wstring data);
    std::function<void(uintptr_t conn_id)> send_bsod;
    std::function<void(uintptr_t conn_id)> send_exit_game;
    std::function<void(uintptr_t conn_id, const wchar_t* data, uint32_t lazy_time)> send_back_game_lazy;
    std::function<void(uintptr_t conn_id, const wchar_t* data, uint32_t lazy_time)> send_exit_game_lazy;
    using Handler = decltype(&TaskJudgment::on_recv_package);
    using HandlerMap = std::map<std::string, Handler>;
    HandlerMap handler_;
};

