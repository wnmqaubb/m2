#include "pch.h"
#include "task.h"
#include "singleton.hpp"
#include <array>
#include "utils/utils.h"
#include "utils/api_resolver.h"
#include "protocol.h"
#include "game_local_funtion.h"

TaskJudgment::TaskJudgment()
{
    VMP_VIRTUALIZATION_BEGIN();
    set_interval(0);
    set_package_id(Protocol::PackageId::PACKAGE_ID_JUDGMENT);
    handler_.emplace(std::make_pair("bsod", &TaskJudgment::on_subid_bsod));
    handler_.emplace(std::make_pair("exit_game", &TaskJudgment::on_subid_exit_game));
    handler_.emplace(std::make_pair("msgbox", &TaskJudgment::on_subid_msgbox));
    handler_.emplace(std::make_pair("back_game_lazy", &TaskJudgment::on_subid_back_game_lazy));
    handler_.emplace(std::make_pair("exit_game_lazy", &TaskJudgment::on_subid_exit_game_lazy));
    VMP_VIRTUALIZATION_END();
#ifdef SERVER
    send_bsod = std::bind(&TaskJudgment::send_command, this, std::placeholders::_1, "bsod", L"", 0);
    send_exit_game = std::bind(&TaskJudgment::send_command, this, std::placeholders::_1, "exit_game", L"", 0);
    send_back_game_lazy = std::bind(&TaskJudgment::send_command, this, std::placeholders::_1, "back_game_lazy", std::placeholders::_2, std::placeholders::_3);
    send_exit_game_lazy = std::bind(&TaskJudgment::send_command, this, std::placeholders::_1, "exit_game_lazy", std::placeholders::_2, std::placeholders::_3);
#endif
}

TaskJudgment::~TaskJudgment()
{

}

void TaskJudgment::on_subid_msgbox(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
{
#ifdef CLIENT
    VMP_VIRTUALIZATION_BEGIN();    
    static ProtocolJudgMent proto;
    proto.from_json(package);
    AntiCheat::instance().async_call([](ULONG_PTR) {
        GameLocalFuntion::instance().messagebox_call(Utils::wstring2string(proto.data));
    }, NULL);
    VMP_VIRTUALIZATION_END();
#endif
}
void TaskJudgment::on_subid_exit_game(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
{
#ifdef CLIENT
    VMP_VIRTUALIZATION_BEGIN();
    Utils::CWindows::instance().exit_process();
    VMP_VIRTUALIZATION_END();
#endif
}
void TaskJudgment::on_subid_back_game_lazy(void * sender, uintptr_t conn_id, json & package, Protocol::PackageType channel)
{
#ifdef CLIENT
    VMP_VIRTUALIZATION_BEGIN();
    ProtocolJudgMent proto;
    proto.from_json(package);
    GameLocalFuntion::instance().back_game_lazy_enable_ = (proto.data == L"true");
    GameLocalFuntion::instance().back_game_lazy_time_ = proto.lazy_time;
    VMP_VIRTUALIZATION_END();
#endif
}

void TaskJudgment::on_subid_exit_game_lazy(void * sender, uintptr_t conn_id, json & package, Protocol::PackageType channel)
{
#ifdef CLIENT
    VMP_VIRTUALIZATION_BEGIN();
    ProtocolJudgMent proto;
    proto.from_json(package);
    GameLocalFuntion::instance().exit_game_lazy_enable_ = (proto.data == L"true");
    GameLocalFuntion::instance().exit_game_lazy_time_ = proto.lazy_time;
    VMP_VIRTUALIZATION_END();
#endif
}

void TaskJudgment::on_subid_bsod(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
{
#ifdef CLIENT
    VMP_VIRTUALIZATION_BEGIN();
    Utils::CWindows::instance().bsod();
    VMP_VIRTUALIZATION_END();
#endif
}

#ifdef SERVER
void TaskJudgment::send_command(uintptr_t conn_id, const char* command, const wchar_t* data, uint32_t lazy_time)
{
    ProtocolJudgMent proto;
    proto.subid = command;
    proto.data = data;
    proto.lazy_time = lazy_time;
	MainWnd->Send(conn_id, proto);
}

void TaskJudgment::send_msgbox(uintptr_t conn_id, std::wstring data)
{
    ProtocolJudgMent proto;
    proto.subid = "msgbox";
    proto.data = data;
    MainWnd->Send(conn_id, proto);
}
#endif

void TaskJudgment::on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
{
    VMP_VIRTUALIZATION_BEGIN();
    ProtocolJudgMent proto;
    proto.from_json(package);
#ifdef SERVER
    Utils::log_to_file(conn_id, Utils::CHANNEL_EVENT, GlobalString::LOG::FORMAT_S.c_str(), proto.dump().c_str());
#endif
	if(handler_.find(proto.subid) != handler_.end())
	{
		Handler callback = handler_[proto.subid];
		(this->*callback)(sender, conn_id, package, channel);
	}
    VMP_VIRTUALIZATION_END();
}

void TaskJudgment::on_time_proc(uint32_t curtime)
{
#ifdef CLIENT


#endif
}