#include "pch.h"
#include "task.h"

TaskExample::TaskExample()
{
    //set_interval，单位毫秒，设置成非0数字，代表on_time_proc执行的时间间隔，设置成0则不定时执行，on_time_proc只有客户端有效
    //set_package_id，设置包id用于收发包时用的
    set_interval(0);
    set_package_id(Protocol::PackageId::PACKAGE_ID_EXAMPLE);
}

TaskExample::~TaskExample()
{

}

void TaskExample::on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
{
#ifdef SERVER
    GLOG(TEXT("收到客户端发来的测试包"));
    //做一些事情
#endif
#ifdef CLIENT
    //收到服务端发来的包
    //做一些事情
#endif
}

void TaskExample::on_time_proc(uint32_t curtime)
{
#ifdef CLIENT
    //定时执行
    Protocol proto;
    proto.copy({ {"id",this->get_package_id()} });
    size_t buffer_size;
    BufferPtr buffer(proto.pak(&buffer_size));
    AntiCheat::instance().m_client->Send(buffer.get(), buffer_size);
#endif
}
