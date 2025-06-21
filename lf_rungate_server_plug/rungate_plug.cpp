#include "pch.h"
#include "lf_plug_sdk.h"
#include <string>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include <cassert>

#define RUNGATE_API void __stdcall

using namespace lfengine;
using namespace lfengine::rungate;
namespace fs = std::filesystem;
TInitRecord g_InitRecord;
#pragma comment(lib, "user32.lib")

std::string read_config_txt(const std::filesystem::path& path, const std::string& section, const std::string& key);
VOID DbgPrint(const char* fmt, ...);
std::string guard_gate_ip;
std::string show_welcome_msg;
std::string guard_gate_welcome_msg1;
std::string guard_gate_welcome_msg2;
std::shared_ptr<const std::vector<char>> new_client_data;
std::string load_file(fs::path path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    assert(file.is_open());
    file.seekg(0, file.end);
    size_t sz = file.tellg();
    file.seekg(0);
    std::string buffer;
    buffer.resize(sz);
    file.read(buffer.data(), sz);
    file.close();
    return buffer;
}
void PrintFunctionBytes(void* funcPtr, size_t bytesToRead = 32) {
    // ����ڴ�ɶ���
    MEMORY_BASIC_INFORMATION mbi;
    //if (!VirtualQuery(funcPtr, &mbi, sizeof(mbi)) ||
    //    !(mbi.Protect & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) {
    //    DbgPrint("�����޷���ȡ�ڴ�");
    //    return;
    //}

    // ��ȫ��ȡ�ֽ���
    std::vector<uint8_t> buffer(bytesToRead);
        memcpy(buffer.data(), funcPtr, bytesToRead);

        //DbgPrint("%p ���ֽ��룺", funcPtr);
        //for (size_t i = 0; i < buffer.size(); ++i) {
        //    DbgPrint("%02X ", buffer[i]);
        //    if ((i + 1) % 8 == 0) DbgPrint("");
        //}
    // ����ȡ���ֽ�д��������ļ�
    std::ofstream ofs("bytes.bin", std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    ofs.close();
}

BOOL WriteStructToFile(const TRunGatePlugClientInfo& data, const char* filename) {
    // 1. ���ļ���������ģʽ��
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        DbgPrint("�޷����ļ�: %s", filename);
        return false;
    }

    // 2. ֱ��д��ṹ��ԭʼ�ֽ�
    ofs.write(reinterpret_cast<const char*>(&data), sizeof(TRunGatePlugClientInfo));

    // 3. ���д���Ƿ�����
    const bool success = ofs.good();
    ofs.close();

    if (success) {
        DbgPrint("�ɹ�д�� %zu �ֽڵ� %s", sizeof(TRunGatePlugClientInfo), filename);
    }
    else {
        DbgPrint("�ļ�д��ʧ��");
    }

    return success;
}

void PrintClientInfo(const TRunGatePlugClientInfo& info) {

    DbgPrint("RecogId: 0x%016I64x MoveSpeed: %d AttackSpeed: %d SpellSpeed: %d", info.RecogId, info.MoveSpeed, info.AttackSpeed, info.SpellSpeed);  // Windowsר��I64

    // ��ȫ�����ַ�����ȷ��null��ֹ��
    //char account[sizeof(info.Account) + 1] = { 0 };
    //char chrName[sizeof(info.ChrName) + 1] = { 0 };
    //char ipAddr[sizeof(info.IpAddr) + 1] = { 0 };
    //char macID[sizeof(info.MacID) + 1] = { 0 };

    //memcpy(account, info.Account, sizeof(info.Account));
    //memcpy(chrName, info.ChrName, sizeof(info.ChrName));
    //memcpy(ipAddr, info.IpAddr, sizeof(info.IpAddr));
    //memcpy(macID, info.MacID, sizeof(info.MacID));

    //DbgPrint("  Account: \"%s\"", account);
    //DbgPrint("  ChrName: \"%s\"", chrName);
    //DbgPrint("  IPValue: %d (0x%08x)", info.IPValue, info.IPValue);
    //DbgPrint("  IpAddr: \"%s\"", ipAddr);
    //DbgPrint("  Port: %d", info.Port);
    //DbgPrint("  MacID: \"%s\"", macID);

    // ����ֵ��ӡ
    DbgPrint("  IsActive: %d IsLoginNotice: %d IsPlayGame: %d", info.IsActive, info.IsLoginNotice, info.IsPlayGame);
    // ָ��ͳ���
    DbgPrint("  DataAdd: 0x%p DataLen: %u", info.DataAdd, info.DataLen);  // Windows��ָ����%p

    // �����ֶΣ�ʮ�����ƴ�ӡ��
    DbgPrint("  Reseved2: [");
    for (int i = 0; i < 5; ++i) {
        if (i > 0) DbgPrint(", ");
        DbgPrint("0x%08X", info.Reseved2[i]);  // 32λʮ������
    }
    DbgPrint("]");
}

RUNGATE_API Init(PInitRecord pInitRecord, BOOL isReload)
{
    PrintFunctionBytes(pInitRecord, 300);
	g_InitRecord = *pInitRecord;
    AddShowLog = g_InitRecord.AddShowLog;
    EncodeBuffer = g_InitRecord.EncodeBuffer;
    DecodeBuffer = g_InitRecord.DecodeBuffer;
    GetClientInfo = g_InitRecord.GetClientInfo;
    CloseClient = g_InitRecord.CloseClient;
    SetClientPlugLoad = g_InitRecord.SetClientPlugLoad;
    SendDataToClient = g_InitRecord.SendDataToClient;
    LockClient = g_InitRecord.LockClient;
    GetActionInfo = g_InitRecord.GetActionInfo;
    CheckIsSpellSwitch = g_InitRecord.CheckIsSpellSwitch;
    //g_InitRecord.Reseved2 = pInitRecord->Reseved2;
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
    auto buffer = load_file(".\\NewClient_f.dll");
    if (buffer.empty())
    {
        DbgPrint("����NewClient.dll�ļ�ʧ��");
        return;
    }
    new_client_data = std::shared_ptr<std::vector<char>>(new std::vector<char>(buffer.data(), buffer.data() + buffer.size()));

    DbgPrint("0x%p", pInitRecord);
    //DbgPrint("GetActionInfo=====  %p %x", g_InitRecord.GetActionInfo, *g_InitRecord.GetActionInfo);
    //DbgPrint("GetActionInfo=====  %p %x", g_InitRecord.CheckIsSpellSwitch, *g_InitRecord.CheckIsSpellSwitch);
    //PrintFunctionBytes(pInitRecord->LockClient);
    //PrintFunctionBytes(pInitRecord->GetActionInfo);
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
	//DbgPrint("=====�ͻ��˿�ʼ=====��ID:%d���û�:%s ip:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.IpAddr, clientInfo.Port);
	//DbgPrint("=====�ͻ��˶Ͽ�=====��ID:%d�� GetClientInfo:%08X", clientID, GetClientInfo);
}

/*
���ز�����IsHookPacked
                 �Ƿ����ص�ǰ���ݰ�����ת����M2��Ĭ��ΪFalse��ת�����ݰ���M2
                 ����⵽�������ʱ�����Խ�Щ������ΪTrue��Ҳ����������ס������õ�M2ȥ����
��������ֵ����IsHookPackedΪTrueʱ������ֵ����:
                 0����λ����������ͻ��˷������ݰ���ֱ���ͻ��˶�����ʱ
                 1��������������ͻ��˷���һ������ʧ�ܵİ�
                 2���ٵ����У���ͻ��˷���һ�������ɹ��İ�
                 3�����ߴ���
                 4�������û�
*/
int __stdcall ClientRecvPacket(int clientID, PTDefaultMessage defMsg, char* lpData, int dataLen, BOOL& IsHookPacked)
{
    int result = 0;
    BOOL is_print = false;
    DWORD real_time = 0, limite_time = 0;
    int action_mode = 0;
    uint32_t ident = defMsg->ident;
	//DbgPrint("C==ident:%u��clientID:%d IsHookPacked:%d", ident, clientID, IsHookPacked);
    // ��ӡ�����յ��İ�,������
    TRunGatePlugClientInfo clientInfo{};

    //// ������չ�ڴ沢��ʼ��
    //std::vector<uint8_t> debugBuffer(DEBUG_SIZE, 0xCC);
    GetClientInfo(clientID, &clientInfo);

    //PrintFunctionBytes(&clientInfo,200);
    //PrintClientInfo(clientInfo);
    //WriteStructToFile(clientInfo, "clientInfo.bin");
    //PrintFunctionBytes(clientInfo.DataAdd, clientInfo.DataLen);
	try
	{
        switch (ident)
        {
            case 10000:
		    {
                IsHookPacked = true;
                SetClientPlugLoad(clientID);
			    //AddShowLog("=====���տͻ������ݰ�=====", 0);
			    //SetClientPlugLoad(clientID);
			    // �·���ʱ������IP 
			    ident = 10005;
                SendDataToClient(clientID, defMsg, new_client_data->data(), new_client_data->size());

			    ident = 10001;
			    SendDataToClient(clientID, defMsg, guard_gate_ip.c_str(), guard_gate_ip.length());


			    if (show_welcome_msg == "1") {
				    if (!guard_gate_welcome_msg1.empty ()) {
					    // �·���ʱ�����ػ�ӭ��Ϣ1
					    ident = 10002;
					    SendDataToClient(clientID, defMsg, guard_gate_welcome_msg1.c_str(), guard_gate_welcome_msg1.length());
				    }

				    if (!guard_gate_welcome_msg2.empty()) {
					    // �·���ʱ�����ػ�ӭ��Ϣ1
					    ident = 10003;
					    SendDataToClient(clientID, defMsg, guard_gate_welcome_msg2.c_str(), guard_gate_welcome_msg2.length());
				    }
			    }
                return result;
		    }

            // �ϲ���ͬ�����case��ʹ��fallthrough��
            // ��·/�ܲ�
            //case CM_WALK:
            //case CM_RUN:
            //    [[fallthrough]];

            //    // �����������飨�ϲ���ͬ�߼���
            //case CM_HIT:
            //case CM_HEAVYHIT:
            //case CM_BIGHIT:
            //case CM_POWERHIT:
            //case CM_LONGHIT:
            //case CM_WIDEHIT:
            //case CM_FIREHIT:
            //case CM_CRSHIT:
            //case CM_TWNHIT:
            //case CM_SWORDHIT:
            //case CM_43HIT:
            //case CM_66HIT:
            //case CM_66HIT1:
            //case CM_101HIT:
            //case CM_100HIT:
            //case CM_102HIT:
            //case CM_103HIT:
            //case CM_113HIT:
            //case CM_115HIT:
            //    DbgPrint("ClientRecvPacket===== >>>> ������������ %u", ident);
            //    DbgPrint("GetActionInfo=====  %p", GetActionInfo);
            //    is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //    DbgPrint("GetActionInfo=====  %d %d %d %d", is_print, &real_time, &limite_time, &action_mode);
            //    break;

            //    // �Զ��弼�ܷ�Χ�ж�
            //default:
            //    if (ident >= CM_CUSTOM_HIT001 &&
            //        ident <= (CM_CUSTOM_HIT001 + CUSTOM_MAGIC_COUNT - 1)) {
            //        is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //        break;
            //    }

            //    switch (ident) {
            //        // ħ������
            //        case CM_SPELL:
            //            if (!CheckIsSpellSwitch(defMsg->tag)) {
            //                is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //            }
            //            break;
            //        // ����
            //        case CM_SITDOWN:
            //        // ת��
            //        case CM_TURN:
            //            is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //            break;

            //        default:
            //            break;  // δ����ı�ʶ��
            //    }
        }

        if (is_print) {
            char buf[1024];
            sprintf_s(buf, "%s  ʵ�ʼ��: %d  ���Ƽ��:%d", GetClientActionName(action_mode), real_time, limite_time);     
            OutputDebugStringA(buf);
        }

        return result;
	}
	catch (...)
	{
		DbgPrint("ClientRecvPacket =====�쳣");
	}
    return result;
}

RUNGATE_API ClientEnd(int clientID)
{
	//TRunGatePlugClientInfo clientInfo{};
	//GetClientInfo(clientID, &clientInfo);
	//DbgPrint("=====�ͻ��˶Ͽ�=====��ID:%d�� �û�:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.Port);
	//AddShowLog("=====�ͻ��˶Ͽ�=====", 0);

}

typedef void (*ShowWindowFunc)();
RUNGATE_API ShowConfigForm()
{
	//AddShowLog("=====��ʾ���ظ߼�����=====", 0);
    HINSTANCE hDll = LoadLibrary(L"RunGateSpeedManage1.dll");
    if (hDll) {
        ShowWindowFunc func = (ShowWindowFunc)GetProcAddress(hDll, "ShowSettingsDialog");
        if (func) func();
        FreeLibrary(hDll);
    }
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