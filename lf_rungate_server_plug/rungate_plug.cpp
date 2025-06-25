#include "pch.h"
#include <string>
#include <unordered_map>
#include <filesystem>
#include "lf_plug_sdk.h"
#include "FileWatcher.h"

using namespace lfengine;
using namespace lfengine::rungate;

#define RUNGATE_API void __stdcall

#pragma comment(lib, "user32.lib")
TInitRecord g_InitRecord;
std::string guard_gate_ip;
std::string show_welcome_msg;
std::string guard_gate_welcome_msg1;
std::string guard_gate_welcome_msg2;
const char* new_client_filename = ".\\NewClient_f.dll";
std::shared_ptr<const std::vector<char>> new_client_data;
std::shared_ptr<FileWatcher> watcher;

RUNGATE_API Init(PInitRecord pInitRecord, BOOL isReload)
{
    VMP_VIRTUALIZATION_BEGIN()
    //Utils::PrintFunctionBytes(pInitRecord, 300);
	g_InitRecord = *pInitRecord;
    AddShowLog = g_InitRecord.AddShowLog;
    EncodeBuffer = g_InitRecord.EncodeBuffer;
    DecodeBuffer = g_InitRecord.DecodeBuffer;
    GetClientInfo = g_InitRecord.GetClientInfo;
    CloseClient = g_InitRecord.CloseClient;
    SetClientPlugLoad = g_InitRecord.SetClientPlugLoad;
    SendDataToClient = g_InitRecord.SendDataToClient;
    LockClient = g_InitRecord.LockClient;
    //GetActionInfo = g_InitRecord.GetActionInfo;
    //CheckIsSpellSwitch = g_InitRecord.CheckIsSpellSwitch;
    //g_InitRecord.Reseved2 = pInitRecord->Reseved2;
	char m_ExeDir[MAX_PATH];
	GetModuleFileNameA(NULL, m_ExeDir, sizeof(m_ExeDir));
	auto ini_path = std::filesystem::path(m_ExeDir).parent_path() / "Config.ini";
	guard_gate_ip = Utils::read_config_txt(ini_path, "GuardGate", "GateIP");
	show_welcome_msg = Utils::read_config_txt(ini_path, "GuardGate", "ShowWelcomeMsg");
	guard_gate_welcome_msg1 = Utils::read_config_txt(ini_path, "GuardGate", "WelcomeMsg1");
	guard_gate_welcome_msg2 = Utils::read_config_txt(ini_path, "GuardGate", "WelcomeMsg2");
	if (guard_gate_ip.empty()) {
		AddShowLog("请在Config.ini配置及时雨网关IP [GuardGate]-->GateIP", 0);
	}
	else {
		Utils::DbgPrint("                       =====及时雨网关IP:%s=====", guard_gate_ip.c_str());
	}
    auto buffer = Utils::load_file(new_client_filename);
    if (buffer.empty())
    {
        Utils::DbgPrint("加载NewClient.dll文件失败");
        return;
    }
    new_client_data = std::shared_ptr<std::vector<char>>(new std::vector<char>(buffer.data(), buffer.data() + buffer.size()));

    //Utils::DbgPrint("0x%p", pInitRecord);
    //Utils::DbgPrint("GetActionInfo=====  %p %x", g_InitRecord.GetActionInfo, *g_InitRecord.GetActionInfo);
    //Utils::DbgPrint("GetActionInfo=====  %p %x", g_InitRecord.CheckIsSpellSwitch, *g_InitRecord.CheckIsSpellSwitch);
    //PrintFunctionBytes(pInitRecord->LockClient);
    //PrintFunctionBytes(pInitRecord->GetActionInfo);
    watcher = std::make_shared<FileWatcher>(new_client_filename, []() {
        auto buffer = Utils::load_file(new_client_filename);
        if (buffer.empty())
        {
            Utils::DbgPrint("加载NewClient.dll文件失败");
            return;
        }
        new_client_data = std::shared_ptr<std::vector<char>>(new std::vector<char>(buffer.data(), buffer.data() + buffer.size()));
        Utils::DbgPrint("NewClient_f.dll文件已重新加载");
    }, std::chrono::seconds(5)); // 每5秒检查一次

    // 启动监视器
    watcher->start();
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
    VMP_VIRTUALIZATION_END()
}

// 客户端开始游戏,此时还不能发包到客户端,发了客户端无法收到
RUNGATE_API ClientStart(int clientID)
{
	TRunGatePlugClientInfo clientInfo{};
	GetClientInfo(clientID, &clientInfo);
	//clientInfo.DataAdd.AddData1 = 1000; // 附加自己的数据
	//Utils::DbgPrint("=====客户端开始=====，ID:%d，用户:%s ip:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.IpAddr, clientInfo.Port);
	//Utils::DbgPrint("=====客户端断开=====，ID:%d， GetClientInfo:%08X", clientID, GetClientInfo);
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
    VMP_VIRTUALIZATION_BEGIN()
    int result = 0;
    BOOL is_print = false;
    DWORD real_time = 0, limite_time = 0;
    int action_mode = 0;
	//Utils::DbgPrint("==ident:%u，clientID:%d IsHookPacked:%d", defMsg->ident, clientID, IsHookPacked);
    // 打印所有收到的包,调试用
    TRunGatePlugClientInfo clientInfo{};

    //// 分配扩展内存并初始化
    //std::vector<uint8_t> debugBuffer(DEBUG_SIZE, 0xCC);
    GetClientInfo(clientID, &clientInfo);

	try
	{
        switch (defMsg->ident)
        {
            case 10000:
		    {
                IsHookPacked = true;
                SetClientPlugLoad(clientID);
			    //Utils::DbgPrint("=====接收客户端数据包=====");
			    // 下发及时雨网关IP 
                if (new_client_data->empty()) {
                    Utils::DbgPrint("NewClient_f.dll文件读取失败,请检查文件是否存在并重新加载插件");
                    return result;
                }
                defMsg->ident = 10005;
                SendDataToClient(clientID, defMsg, new_client_data->data(), new_client_data->size());

                defMsg->ident = 10001;
			    SendDataToClient(clientID, defMsg, guard_gate_ip.c_str(), guard_gate_ip.length());


			    if (show_welcome_msg == "1") {
				    if (!guard_gate_welcome_msg1.empty()) {
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
            //    Utils::DbgPrint("ClientRecvPacket===== >>>> 物理攻击技能组 %u", defMsg->ident);
            //    Utils::DbgPrint("GetActionInfo=====  %p", GetActionInfo);
            //    is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //    Utils::DbgPrint("GetActionInfo=====  %d %d %d %d", is_print, &real_time, &limite_time, &action_mode);
            //    break;

            //    // 自定义技能范围判断
            //default:
            //    if (defMsg->ident >= CM_CUSTOM_HIT001 &&
            //        defMsg->ident <= (CM_CUSTOM_HIT001 + CUSTOM_MAGIC_COUNT - 1)) {
            //        is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //        break;
            //    }

            //    switch (defMsg->ident) {
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
		Utils::DbgPrint("ClientRecvPacket =====异常");
	}
    return result;
    VMP_VIRTUALIZATION_END()
}

RUNGATE_API ClientEnd(int clientID)
{
	//TRunGatePlugClientInfo clientInfo{};
	//GetClientInfo(clientID, &clientInfo);
	//Utils::DbgPrint("=====客户端断开=====，ID:%d， 用户:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.Port);
	//AddShowLog("=====客户端断开=====", 0);

}

typedef void (*ShowWindowFunc)();
RUNGATE_API ShowConfigForm()
{
    VMP_VIRTUALIZATION_BEGIN()
    //AddShowLog("=====显示网关高级设置=====", 0);
    HINSTANCE hDll = LoadLibrary(L"RunGateSpeedManage1.dll");
    if (hDll) {
        ShowWindowFunc func = (ShowWindowFunc)GetProcAddress(hDll, "ShowSettingsDialog");
        if (func) func();
        FreeLibrary(hDll);
    }
    VMP_VIRTUALIZATION_END()
}
RUNGATE_API Uninit()
{
    VMP_VIRTUALIZATION_BEGIN()
    //AddShowLog("=====卸载网关插件完成=====", 0);
    watcher->stop();
    VMP_VIRTUALIZATION_END()
}