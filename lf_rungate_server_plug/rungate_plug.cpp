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
	AddShowLog("                          ====���ؼ�ʱ�����ز�����====", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                                QQ����:331101339", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                           ������  ����������ʱ  ������", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                           ������  ������Ϸģʽ  ������", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                           ������  �����ʵ�ʹ��  ������", 0);
	AddShowLog("                                                                                       ", 0);
	AddShowLog("                           ������  ���Ч������  ������", 0);
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
		AddShowLog("����Config.ini���ü�ʱ������IP [GuardGate]-->GateIP", 0);
	}
	else {
		DbgPrint("                       =====��ʱ������IP:%s=====", guard_gate_ip.c_str());
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

// �ͻ��˿�ʼ��Ϸ,��ʱ�����ܷ������ͻ���,���˿ͻ����޷��յ�
RUNGATE_API ClientStart(int clientID)
{
	TRunGatePlugClientInfo clientInfo{};
	GetClientInfo(clientID, &clientInfo);
	//clientInfo.DataAdd.AddData1 = 1000; // �����Լ�������
	DbgPrint("=====�ͻ��˿�ʼ=====��ID:%d���û�:%s ip:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.IpAddr, clientInfo.Port);
	//DbgPrint("=====�ͻ��˶Ͽ�=====��ID:%d�� GetClientInfo:%08X", clientID, GetClientInfo);

}

RUNGATE_API ClientRecvPacket(int clientID, PTDefaultMessage defMsg, char* lpData, int dataLen, bool isSendToM2)
{
	SetClientPlugLoad(clientID);
	//DbgPrint("ClientRecvPacket=====ident:%lu��clientID:%d ", defMsg->ident, clientID);
	try
	{
		if (defMsg->ident == 10000)
		{
			//AddShowLog("=====���տͻ������ݰ�=====", 0);
			//SetClientPlugLoad(clientID);
			// �·���ʱ������IP
			defMsg->ident = 10001;
			SendDataToClient(clientID, defMsg, guard_gate_ip.c_str(), guard_gate_ip.length());


			if (show_welcome_msg == "1") {
				if (!guard_gate_welcome_msg1.empty ()) {
					// �·���ʱ�����ػ�ӭ��Ϣ1
					defMsg->ident = 10002;
					SendDataToClient(clientID, defMsg, guard_gate_welcome_msg1.c_str(), guard_gate_welcome_msg1.length());
				}

				if (!guard_gate_welcome_msg2.empty()) {
					// �·���ʱ�����ػ�ӭ��Ϣ1
					defMsg->ident = 10003;
					SendDataToClient(clientID, defMsg, guard_gate_welcome_msg2.c_str(), guard_gate_welcome_msg2.length());
				}
			}
		}
	}
	catch (...)
	{
		DbgPrint("ClientRecvPacket =====�쳣");
	}
}

RUNGATE_API ClientEnd(int clientID)
{
	//TRunGatePlugClientInfo clientInfo{};
	//GetClientInfo(clientID, &clientInfo);
	//DbgPrint("=====�ͻ��˶Ͽ�=====��ID:%d�� �û�:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.Port);
	//AddShowLog("=====�ͻ��˶Ͽ�=====", 0);

}

RUNGATE_API ShowConfigForm()
{
	//AddShowLog("=====��ʾ���ظ߼�����=====", 0);
}

RUNGATE_API Uninit()
{
	//AddShowLog("=====ж�����ز�����=====", 0);
}

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> readIniFile(const std::filesystem::path& path) {
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> iniData;
	std::ifstream file(path);
	if (file.is_open()) {
		std::string section;
		std::string line;
		while (std::getline(file, line)) {
			// ȥ������β�Ŀհ��ַ�
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
		AddShowLog("=====�޷���Config.ini�ļ�=====", 0);
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