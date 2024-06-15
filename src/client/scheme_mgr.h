#pragma once
#include <map>

class Scheme
{
public:
    typedef enum _Scheme
    {

    };
    Scheme();
    ~Scheme();
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool get_last_error();
};

class SchemeMgr
{
public:
    SchemeMgr();
    ~SchemeMgr();
    bool task_run_all();
    void get_error_task();
    std::map<uint32_t, Scheme*> m_task;
};