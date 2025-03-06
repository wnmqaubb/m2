#pragma once
#include "NewClient/ClientImpl.h"
#include <Lightbone/utils.h>
#include <iostream>
#include "Service/AntiCheatClient.h"
#include "Service/SubServicePackage.h"
#include <filesystem>
#include "GameFunction.h"
#include "BasicUtils.h"
#include "version.build"

extern std::shared_ptr<asio::io_service> g_game_io;
extern std::shared_ptr<asio::detail::thread_group> g_thread_group;
extern std::shared_ptr<int> g_client_rev_version;
extern std::shared_ptr<CClientImpl> client_;
extern std::shared_ptr<asio2::timer> g_timer;

void LoadPlugin();
void InitRmc();
void InitJavaScript();
void InitTimeoutCheck();
void InitHideProcessDetect();
void InitShowWindowHookDetect();
void InitSpeedDetect();
void InitMiniDump();
void InitImageProtectCheck();
void InitDirectoryChangsDetect();
void async_execute_javascript(const std::string& code, uint32_t script_id);
void on_recv_punish(const RawProtocolImpl& package, const msgpack::v1::object_handle& msg);
void on_recv_pkg_policy(const ProtocolS2CPolicy& req);

void __declspec(noinline) UnitPunishKick();