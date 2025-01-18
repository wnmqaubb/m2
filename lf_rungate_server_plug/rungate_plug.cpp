#include "pch.h"
#include "lf_plug_sdk.h"
#include <string>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <filesystem>

#define RUNGATE_API void __stdcall

using namespace lfengine;
using namespace lfengine::rungate;
TInitRecord g_InitRecord;
#pragma comment(lib, "user32.lib")

std::string read_config_txt(const std::filesystem::path& path, const std::string& section, const std::string& key);
VOID DbgPrint(const char* fmt, ...);
std::string guard_gate_ip;
std::string show_welcome_msg;
std::string guard_gate_welcome_msg1;
std::string guard_gate_welcome_msg2;
RUNGATE_API Init(PInitRecord pInitRecord, bool isReload)
{
	g_InitRecord = *pInitRecord;
	AddShowLog = g_InitRecord.AddShowLog;
	EncodeBuffer = g_InitRecord.EncodeBuffer;
	DecodeBuffer = g_InitRecord.DecodeBuffer;
	GetClientInfo = g_InitRecord.GetClientInfo;
	CloseClient = g_InitRecord.CloseClient;
	SetClientPlugLoad = g_InitRecord.SetClientPlugLoad;
	SendDataToClient = g_InitRecord.SendDataToClient;
	LockClient = g_InitRecord.LockClient;
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                          ====加载及时雨网关插件完成====", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                                QQ交流:331101339", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                           ※※※  优雅永不过时  ※※※", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                           ※※※  沉静游戏模式  ※※※", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                           ※※※  最舒适的使用  ※※※", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                           ※※※  最高效的运行  ※※※", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                                                                                          ", 0);
	char m_ExeDir[MAX_PATH];
	GetModuleFileNameA(NULL, m_ExeDir, sizeof(m_ExeDir));
	auto ini_path = std::filesystem::path(m_ExeDir).parent_path() / "Config.ini";
	guard_gate_ip = read_config_txt(ini_path, "GuardGate", "GateIP");
	show_welcome_msg = read_config_txt(ini_path, "GuardGate", "ShowWelcomeMsg");
	guard_gate_welcome_msg1 = read_config_txt(ini_path, "GuardGate", "WelcomeMsg1");
	guard_gate_welcome_msg2 = read_config_txt(ini_path, "GuardGate", "WelcomeMsg2");
	if (guard_gate_ip.empty()) {
		AddShowLog("请在Config.ini配置及时雨网关IP [GuardGate]-->GateIP", 0);
	}
	else {
		DbgPrint("                       =====及时雨网关IP:%s=====", guard_gate_ip.c_str());
	}
}

VOID DbgPrint(const char* fmt, ...)
{
	char    buf[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, fmt, args);
	va_end(args);
	AddShowLog(buf, 0);
}

// 客户端开始游戏,此时还不能发包到客户端,发了客户端无法收到
RUNGATE_API ClientStart(int clientID)
{
	TRunGatePlugClientInfo clientInfo{};
	GetClientInfo(clientID, &clientInfo);
	//clientInfo.DataAdd.AddData1 = 1000; // 附加自己的数据
	DbgPrint("=====客户端开始=====，ID:%d，用户:%s ip:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.IpAddr, clientInfo.Port);
	//DbgPrint("=====客户端断开=====，ID:%d， GetClientInfo:%08X", clientID, GetClientInfo);

}

RUNGATE_API ClientRecvPacket(int clientID, PTDefaultMessage defMsg, char* lpData, int dataLen, bool isSendToM2)
{
	SetClientPlugLoad(clientID);
	//DbgPrint("ClientRecvPacket=====ident:%lu，clientID:%d ", defMsg->ident, clientID);
	try
	{
		if (defMsg->ident == 10000)
		{
			//AddShowLog("=====接收客户端数据包=====", 0);
			//SetClientPlugLoad(clientID);
			// 下发及时雨网关IP
			defMsg->ident = 10001;
			SendDataToClient(clientID, defMsg, guard_gate_ip.c_str(), guard_gate_ip.length());


			if (show_welcome_msg == "1") {
				if (!guard_gate_welcome_msg1.empty ()) {
					// 下发及时雨网关欢迎信息1
					defMsg->ident = 10002;
					SendDataToClient(clientID, defMsg, guard_gate_welcome_msg1.c_str(), guard_gate_welcome_msg1.length());
				}

				if (!guard_gate_welcome_msg2.empty()) {
					// 下发及时雨网关欢迎信息1
					defMsg->ident = 10003;
					SendDataToClient(clientID, defMsg, guard_gate_welcome_msg2.c_str(), guard_gate_welcome_msg2.length());
				}
			}
		}
	}
	catch (...)
	{
		DbgPrint("ClientRecvPacket =====异常");
	}
}

RUNGATE_API ClientEnd(int clientID)
{
	//TRunGatePlugClientInfo clientInfo{};
	//GetClientInfo(clientID, &clientInfo);
	//DbgPrint("=====客户端断开=====，ID:%d， 用户:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.Port);
	//AddShowLog("=====客户端断开=====", 0);

}

RUNGATE_API ShowConfigForm()
{
	//AddShowLog("=====显示网关高级设置=====", 0);
}

RUNGATE_API Uninit()
{
	//AddShowLog("=====卸载网关插件完成=====", 0);
}

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> readIniFile(const std::filesystem::path& path) {
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> iniData;
	std::ifstream file(path);
	if (file.is_open()) {
		std::string section;
		std::string line;
		while (std::getline(file, line)) {
			// 去除行首尾的空白字符
			line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
			if (line.empty() || line[0] == ';') continue;
			if (line[0] == '[' && line.back() == ']') {
				section = line.substr(1, line.length() - 2);
			}
			else {
				auto pos = line.find('=');
				if (pos != std::string::npos) {
					std::string key = line.substr(0, pos);
					std::string value = line.substr(pos + 1);
					iniData[section][key] = value;
				}
			}
		}
		file.close();
	}
	else {
		AddShowLog("=====无法打开Config.ini文件=====", 0);
	}
	return iniData;
}

std::string read_config_txt(const std::filesystem::path& path, const std::string& section, const std::string& key) {
	setlocale(LC_CTYPE, "");
	auto iniData = readIniFile(path);
	if (iniData.find(section) != iniData.end() && iniData[section].find(key) != iniData[section].end()) {
		return iniData[section][key];
	}
	else {
		return "";
	}
}