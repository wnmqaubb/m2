#include "pch.h"
#include "task.h"

TaskExample::TaskExample()
{
    //set_interval����λ���룬���óɷ�0���֣�����on_time_procִ�е�ʱ���������ó�0�򲻶�ʱִ�У�on_time_procֻ�пͻ�����Ч
    //set_package_id�����ð�id�����շ���ʱ�õ�
    set_interval(0);
    set_package_id(Protocol::PackageId::PACKAGE_ID_EXAMPLE);
}

TaskExample::~TaskExample()
{

}

void TaskExample::on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
{
#ifdef SERVER
    GLOG(TEXT("�յ��ͻ��˷����Ĳ��԰�"));
    //��һЩ����
#endif
#ifdef CLIENT
    //�յ�����˷����İ�
    //��һЩ����
#endif
}

void TaskExample::on_time_proc(uint32_t curtime)
{
#ifdef CLIENT
    //��ʱִ��
    Protocol proto;
    proto.copy({ {"id",this->get_package_id()} });
    size_t buffer_size;
    BufferPtr buffer(proto.pak(&buffer_size));
    AntiCheat::instance().m_client->Send(buffer.get(), buffer_size);
#endif
}
