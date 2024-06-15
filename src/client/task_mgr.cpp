#include "pch.h"
#include "task_mgr.h"
#include "task\task.h"
#include <windows.h>

TaskMgr::TaskMgr()
{
    add_task(Singleton<TaskHeartBeat>::instance());
    add_task(Singleton<TaskPolicy>::instance());
    add_task(Singleton<TaskScreenShot>::instance());
    add_task(Singleton<TaskExample>::instance());
    add_task(Singleton<TaskProcessInfo>::instance());
    add_task(Singleton<TaskHardware>::instance());
    add_task(Singleton<TaskShellcode>::instance());
    add_task(Singleton<TaskJudgment>::instance());
    add_task(Singleton<TaskApiHook>::instance());
    add_task(Singleton<Task>::instance());
}

TaskMgr::~TaskMgr()
{

}

void TaskMgr::get_error_task()
{

}

bool TaskMgr::add_task(Task* task)
{
    if (task == nullptr)
    {
        return false;
    }
    if (task_map_.find(task->get_package_id()) != task_map_.end())
    {
        return false;
    }
    task_map_.insert(std::make_pair(task->get_package_id(), task));
    return true;
}


void TaskMgr::time_proc_dispatcher(uint32_t curtime)
{
    for (TaskMap::iterator itor = task_map_.begin(); itor != task_map_.end(); itor++)
    {
        Task& task = *(itor->second);
        if (task.get_interval() == 0)
        {
            continue;
        }
        if ( curtime - task.get_last_tickcount() > task.get_interval())
        {
            itor->second->on_time_proc(curtime);
            task.set_last_tickcount(curtime);
        }
    }
    ::SleepEx(100, TRUE);
}

Task::Task()
{
    set_last_tickcount(::GetTickCount());
    set_interval(0);
    set_package_id(Protocol::PackageId::PACKAGE_ID_UNDEFINED);
}

Task::~Task()
{
}

Task::Task(const Protocol::PackageId package_id)
    : Task()
{
    set_package_id(package_id);
}

Protocol::PackageId Task::get_package_id() const
{
    return this->package_id_;
}

uint32_t Task::get_last_tickcount() const
{
    return this->last_tickcount_;
}

uint32_t Task::get_interval() const
{
    return this->interval_;
}

void Task::set_interval(uint32_t interval)
{
    this->interval_ = interval;
}

void Task::set_last_tickcount(uint32_t tickcount)
{
    this->last_tickcount_ = tickcount;
}


void Task::set_package_id(const Protocol::PackageId package_id)
{
    this->package_id_ = package_id;
}

bool Task::set_last_error()
{
    return true;
}

