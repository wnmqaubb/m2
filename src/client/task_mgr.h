#pragma once
#include <map>
#include "protocol.h"

class Task
{
public:
    Task();
    Task(const Protocol::PackageId package_id);
    ~Task();

    virtual bool start() { return true; };
    virtual bool stop() { return true; };
    virtual void on_time_proc(uint32_t curtime) { return; }
    virtual void on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) { return; };
    virtual void trigger() { return; };
    bool get_last_error();
    bool set_last_error();
    Protocol::PackageId get_package_id() const;
    uint32_t get_last_tickcount() const;
    uint32_t get_interval() const;
    void set_interval(uint32_t interval);
    void set_last_tickcount(uint32_t tickcount);
    void set_package_id(const Protocol::PackageId package_id);
private:
    Protocol::PackageId package_id_;
    uint32_t last_tickcount_;
    uint32_t interval_;
};

class TaskMgr
{
public:
    using TaskMap = std::map<Protocol::PackageId, Task*>;
    TaskMgr();
    ~TaskMgr();
    void get_error_task();
    bool add_task(Task* task);
    uint32_t remove_task(Task* task);
    void time_proc_dispatcher(uint32_t curtime);
    TaskMap task_map_;
};