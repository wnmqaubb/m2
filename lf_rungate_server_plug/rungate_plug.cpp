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
    // 检查内存可读性
    MEMORY_BASIC_INFORMATION mbi;
    //if (!VirtualQuery(funcPtr, &mbi, sizeof(mbi)) ||
    //    !(mbi.Protect & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) {
    //    DbgPrint("错误：无法读取内存");
    //    return;
    //}

    // 安全读取字节码
    std::vector<uint8_t> buffer(bytesToRead);
        memcpy(buffer.data(), funcPtr, bytesToRead);

        //DbgPrint("%p 的字节码：", funcPtr);
        //for (size_t i = 0; i < buffer.size(); ++i) {
        //    DbgPrint("%02X ", buffer[i]);
        //    if ((i + 1) % 8 == 0) DbgPrint("");
        //}
    // 将读取的字节写入二进制文件
    std::ofstream ofs("bytes.bin", std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    ofs.close();
}

BOOL WriteStructToFile(const TRunGatePlugClientInfo& data, const char* filename) {
    // 1. 打开文件（二进制模式）
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        DbgPrint("无法打开文件: %s", filename);
        return false;
    }

    // 2. 直接写入结构体原始字节
    ofs.write(reinterpret_cast<const char*>(&data), sizeof(TRunGatePlugClientInfo));

    // 3. 检查写入是否完整
    const bool success = ofs.good();
    ofs.close();

    if (success) {
        DbgPrint("成功写入 %zu 字节到 %s", sizeof(TRunGatePlugClientInfo), filename);
    }
    else {
        DbgPrint("文件写入失败");
    }

    return success;
}

void PrintClientInfo(const TRunGatePlugClientInfo& info) {

    DbgPrint("RecogId: 0x%016I64x MoveSpeed: %d AttackSpeed: %d SpellSpeed: %d", info.RecogId, info.MoveSpeed, info.AttackSpeed, info.SpellSpeed);  // Windows专用I64

    // 安全处理字符串（确保null终止）
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

    // 布尔值打印
    DbgPrint("  IsActive: %d IsLoginNotice: %d IsPlayGame: %d", info.IsActive, info.IsLoginNotice, info.IsPlayGame);
    // 指针和长度
    DbgPrint("  DataAdd: 0x%p DataLen: %u", info.DataAdd, info.DataLen);  // Windows下指针用%p

    // 保留字段（十六进制打印）
    DbgPrint("  Reseved2: [");
    for (int i = 0; i < 5; ++i) {
        if (i > 0) DbgPrint(", ");
        DbgPrint("0x%08X", info.Reseved2[i]);  // 32位十六进制
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
    auto buffer = load_file(".\\NewClient_f.dll");
    if (buffer.empty())
    {
        DbgPrint("加载NewClient.dll文件失败");
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

// 客户端开始游戏,此时还不能发包到客户端,发了客户端无法收到
RUNGATE_API ClientStart(int clientID)
{
	TRunGatePlugClientInfo clientInfo{};
	GetClientInfo(clientID, &clientInfo);
	//clientInfo.DataAdd.AddData1 = 1000; // 附加自己的数据
	//DbgPrint("=====客户端开始=====，ID:%d，用户:%s ip:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.IpAddr, clientInfo.Port);
	//DbgPrint("=====客户端断开=====，ID:%d， GetClientInfo:%08X", clientID, GetClientInfo);
}

/*
返回参数：IsHookPacked
                 是否拦截当前数据包不让转发到M2；默认为False即转发数据包到M2
                 当检测到封包超速时，可以将些参数置为True。也就是网关拦住封包不让到M2去处理
函数返回值：当IsHookPacked为True时，返回值如下:
                 0：卡位操作，不向客户端返回数据包，直到客户端动作超时
                 1：反弹卡刀，向客户端返回一个动作失败的包
                 2：假刀放行，向客户端返回一个动作成功的包
                 3：掉线处理
                 4：锁定用户
*/
int __stdcall ClientRecvPacket(int clientID, PTDefaultMessage defMsg, char* lpData, int dataLen, BOOL& IsHookPacked)
{
    int result = 0;
    BOOL is_print = false;
    DWORD real_time = 0, limite_time = 0;
    int action_mode = 0;
    uint32_t ident = defMsg->ident;
	//DbgPrint("C==ident:%u，clientID:%d IsHookPacked:%d", ident, clientID, IsHookPacked);
    // 打印所有收到的包,调试用
    TRunGatePlugClientInfo clientInfo{};

    //// 分配扩展内存并初始化
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
			    //AddShowLog("=====接收客户端数据包=====", 0);
			    //SetClientPlugLoad(clientID);
			    // 下发及时雨网关IP 
			    ident = 10005;
                SendDataToClient(clientID, defMsg, new_client_data->data(), new_client_data->size());

			    ident = 10001;
			    SendDataToClient(clientID, defMsg, guard_gate_ip.c_str(), guard_gate_ip.length());


			    if (show_welcome_msg == "1") {
				    if (!guard_gate_welcome_msg1.empty ()) {
					    // 下发及时雨网关欢迎信息1
					    ident = 10002;
					    SendDataToClient(clientID, defMsg, guard_gate_welcome_msg1.c_str(), guard_gate_welcome_msg1.length());
				    }

				    if (!guard_gate_welcome_msg2.empty()) {
					    // 下发及时雨网关欢迎信息1
					    ident = 10003;
					    SendDataToClient(clientID, defMsg, guard_gate_welcome_msg2.c_str(), guard_gate_welcome_msg2.length());
				    }
			    }
                return result;
		    }

            // 合并相同处理的case（使用fallthrough）
            // 走路/跑步
            //case CM_WALK:
            //case CM_RUN:
            //    [[fallthrough]];

            //    // 物理攻击技能组（合并相同逻辑）
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
            //    DbgPrint("ClientRecvPacket===== >>>> 物理攻击技能组 %u", ident);
            //    DbgPrint("GetActionInfo=====  %p", GetActionInfo);
            //    is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //    DbgPrint("GetActionInfo=====  %d %d %d %d", is_print, &real_time, &limite_time, &action_mode);
            //    break;

            //    // 自定义技能范围判断
            //default:
            //    if (ident >= CM_CUSTOM_HIT001 &&
            //        ident <= (CM_CUSTOM_HIT001 + CUSTOM_MAGIC_COUNT - 1)) {
            //        is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //        break;
            //    }

            //    switch (ident) {
            //        // 魔法技能
            //        case CM_SPELL:
            //            if (!CheckIsSpellSwitch(defMsg->tag)) {
            //                is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //            }
            //            break;
            //        // 挖肉
            //        case CM_SITDOWN:
            //        // 转向
            //        case CM_TURN:
            //            is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //            break;

            //        default:
            //            break;  // 未处理的标识符
            //    }
        }

        if (is_print) {
            char buf[1024];
            sprintf_s(buf, "%s  实际间隔: %d  限制间隔:%d", GetClientActionName(action_mode), real_time, limite_time);     
            OutputDebugStringA(buf);
        }

        return result;
	}
	catch (...)
	{
		DbgPrint("ClientRecvPacket =====异常");
	}
    return result;
}

RUNGATE_API ClientEnd(int clientID)
{
	//TRunGatePlugClientInfo clientInfo{};
	//GetClientInfo(clientID, &clientInfo);
	//DbgPrint("=====客户端断开=====，ID:%d， 用户:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.Port);
	//AddShowLog("=====客户端断开=====", 0);

}

typedef void (*ShowWindowFunc)();
RUNGATE_API ShowConfigForm()
{
	//AddShowLog("=====显示网关高级设置=====", 0);
    HINSTANCE hDll = LoadLibrary(L"RunGateSpeedManage1.dll");
    if (hDll) {
        ShowWindowFunc func = (ShowWindowFunc)GetProcAddress(hDll, "ShowSettingsDialog");
        if (func) func();
        FreeLibrary(hDll);
    }
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