#pragma once
#include <Lightbone/utils.h>
#include <iostream>
#include "Service/AntiCheatClient.h"
#include "Service/SubServicePackage.h"
#include <filesystem>
#include "GameFunction.h"
#include "BasicUtils.h"
#include "version.build"

__declspec(dllimport) asio::io_service g_game_io;
__declspec(dllimport) asio::io_service g_io;
__declspec(dllimport) int g_client_rev_version;
__declspec(dllimport) asio::detail::thread_group g_thread_group;


void InitRmc(CAntiCheatClient* client);
void InitJavaScript(CAntiCheatClient* client);
void InitTimeoutCheck(CAntiCheatClient* client);
void InitHideProcessDetect(CAntiCheatClient* client);
void InitShowWindowHookDetect(CAntiCheatClient* client);
void InitSpeedDetect(CAntiCheatClient* client);
void InitMiniDump();
void InitImageProtectCheck(CAntiCheatClient* client);
void async_execute_javascript(const std::string& code, uint32_t script_id);
void on_recv_punish(CAntiCheatClient* client, const RawProtocolImpl& package, const msgpack::v1::object_handle& msg);
void on_recv_pkg_policy(CAntiCheatClient* client, const ProtocolS2CPolicy& req);

void __declspec(noinline) UnitPunishKick(std::error_code ec);