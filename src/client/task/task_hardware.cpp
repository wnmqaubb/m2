#include "pch.h"
#include "task.h"
#include "windows.hpp"
#include "singleton.hpp"
#include <array>
#include "utils/utils.h"
#include <Psapi.h>

TaskHardware::TaskHardware() 
{
	set_interval(0);
	set_package_id(Protocol::PackageId::PACKAGE_ID_HARDWARE);
}

TaskHardware::~TaskHardware() 
{

}

void TaskHardware::on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel) 
{
#ifdef SERVER    
    ProtocolHardWare proto;
    proto.from_json(package);
    Utils::log_to_file(conn_id, Utils::CHANNEL_WARNING, GlobalString::LOG::FORMAT_S.c_str(), proto.dump().c_str());
#endif
#ifdef CLIENT
	on_time_proc(0);
#endif
}

void TaskHardware::on_time_proc(uint32_t curtime) 
{
#ifdef CLIENT
	AntiCheat& anticheat = AntiCheat::instance();	
    ProtocolHardWare proto;
    proto.cpuid = Utils::HardwareInfo::get_cpuid();
    proto.volume_serial_number = Utils::HardwareInfo::get_volume_serial_number();
    proto.mac_address = Utils::HardwareInfo::get_mac_address();
    anticheat.send(proto);
#endif
}

void TaskHardware::trigger()
{
#ifdef SERVER 
    this->send_to_all();
#endif
}


void TaskHardware::send_to_all()
{
#ifdef SERVER 
    ProtocolHardWare proto;
    MainWnd->SendToAll(proto);
#endif
}