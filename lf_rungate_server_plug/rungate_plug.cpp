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
		AddShowLog("����Config.ini���ü�ʱ������IP [GuardGate]-->GateIP", 0);
	}
	else {
		Utils::DbgPrint("                       =====��ʱ������IP:%s=====", guard_gate_ip.c_str());
	}
    auto buffer = Utils::load_file(new_client_filename);
    if (buffer.empty())
    {
        Utils::DbgPrint("����NewClient.dll�ļ�ʧ��");
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
            Utils::DbgPrint("����NewClient.dll�ļ�ʧ��");
            return;
        }
        new_client_data = std::shared_ptr<std::vector<char>>(new std::vector<char>(buffer.data(), buffer.data() + buffer.size()));
        Utils::DbgPrint("NewClient_f.dll�ļ������¼���");
    }, std::chrono::seconds(5)); // ÿ5����һ��

    // ����������
    watcher->start();
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
    VMP_VIRTUALIZATION_END()
}

// �ͻ��˿�ʼ��Ϸ,��ʱ�����ܷ������ͻ���,���˿ͻ����޷��յ�
RUNGATE_API ClientStart(int clientID)
{
	TRunGatePlugClientInfo clientInfo{};
	GetClientInfo(clientID, &clientInfo);
	//clientInfo.DataAdd.AddData1 = 1000; // �����Լ�������
	//Utils::DbgPrint("=====�ͻ��˿�ʼ=====��ID:%d���û�:%s ip:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.IpAddr, clientInfo.Port);
	//Utils::DbgPrint("=====�ͻ��˶Ͽ�=====��ID:%d�� GetClientInfo:%08X", clientID, GetClientInfo);
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
    VMP_VIRTUALIZATION_BEGIN()
    int result = 0;
    BOOL is_print = false;
    DWORD real_time = 0, limite_time = 0;
    int action_mode = 0;
	//Utils::DbgPrint("==ident:%u��clientID:%d IsHookPacked:%d", defMsg->ident, clientID, IsHookPacked);
    // ��ӡ�����յ��İ�,������
    TRunGatePlugClientInfo clientInfo{};

    //// ������չ�ڴ沢��ʼ��
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
			    //Utils::DbgPrint("=====���տͻ������ݰ�=====");
			    // �·���ʱ������IP 
                if (new_client_data->empty()) {
                    Utils::DbgPrint("NewClient_f.dll�ļ���ȡʧ��,�����ļ��Ƿ���ڲ����¼��ز��");
                    return result;
                }
                defMsg->ident = 10005;
                SendDataToClient(clientID, defMsg, new_client_data->data(), new_client_data->size());

                defMsg->ident = 10001;
			    SendDataToClient(clientID, defMsg, guard_gate_ip.c_str(), guard_gate_ip.length());


			    if (show_welcome_msg == "1") {
				    if (!guard_gate_welcome_msg1.empty()) {
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
            //    Utils::DbgPrint("ClientRecvPacket===== >>>> ������������ %u", defMsg->ident);
            //    Utils::DbgPrint("GetActionInfo=====  %p", GetActionInfo);
            //    is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //    Utils::DbgPrint("GetActionInfo=====  %d %d %d %d", is_print, &real_time, &limite_time, &action_mode);
            //    break;

            //    // �Զ��弼�ܷ�Χ�ж�
            //default:
            //    if (defMsg->ident >= CM_CUSTOM_HIT001 &&
            //        defMsg->ident <= (CM_CUSTOM_HIT001 + CUSTOM_MAGIC_COUNT - 1)) {
            //        is_print = GetActionInfo(clientID, &real_time, &limite_time, &action_mode);
            //        break;
            //    }

            //    switch (defMsg->ident) {
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
		Utils::DbgPrint("ClientRecvPacket =====�쳣");
	}
    return result;
    VMP_VIRTUALIZATION_END()
}

RUNGATE_API ClientEnd(int clientID)
{
	//TRunGatePlugClientInfo clientInfo{};
	//GetClientInfo(clientID, &clientInfo);
	//Utils::DbgPrint("=====�ͻ��˶Ͽ�=====��ID:%d�� �û�:%s Port:%d", clientID, clientInfo.ChrName, clientInfo.Port);
	//AddShowLog("=====�ͻ��˶Ͽ�=====", 0);

}

typedef void (*ShowWindowFunc)();
RUNGATE_API ShowConfigForm()
{
    VMP_VIRTUALIZATION_BEGIN()
    //AddShowLog("=====��ʾ���ظ߼�����=====", 0);
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
    //AddShowLog("=====ж�����ز�����=====", 0);
    watcher->stop();
    VMP_VIRTUALIZATION_END()
}