#include "NewClient/pch.h"
#include <Lightbone/utils.h>
#include "Service/AntiCheatClient.h"

std::set<uint32_t> cheat_exe_set_pt = {
    0x9D6B,/*GEE��ʦ*/
};
std::set<uint32_t> cheat_set_pt = {
    0x40B76F,/*�������*/
    0x44FD84,/*�������*/
    0x401214,/*�������*/
    0xBAE089,/*�̿����*/
    0x14413D2,/*��ţ����*/
    0x548290,/*������2.03*/
    0x49D9FA,
    0x409E00,
    0x4A3AA5,
    0x405DF0,
    0x46CBC3,
    0x1FBB35E,
    0x418101,
    0x449C71,
    0x405810,
    0x44EA94,
    0x410780,/*CE5410*/
    0x464FD7,
    0x44C4A2,
    0x466D95,
    0x131E022,
    0x1010DD1F,
    0x455580,
    0x455188,
    0x468881,
    0xE87B43,
    0xD07A0B,
    0x421456,
    0x40BC0E,
    0x4012DC,/*�����*/
    0x4D4960,/*������࿪��*/
    0xEBCF00,/*BRCTR*/
    0xEBC2D1,/*BRCTR*/
    0x44E076,/*WPE*/
    0x44EA94,/*WPE*/
    0x44533D,/*WPE*/
    0x445CD4,/*WPE*/
    0x7D6B38,/*WPE*/
    0xC78F17,/*GEE����*/
    0xDF3E94,/*GEE����*/
    0x806D24,/*С����*/
    0x4151F0,/*CE6.5*/
    0x404FE0,/*������CE*/
    0x63D336,/*�������CE*/
    0x413900,/*����CE*/
    0x404DFC,/*����ר��CE*/
    0x419800,/*CE6.5*/
    0x415A40,/*CE6.6*/
    0x41A380,/*CE6.7*/
    0x415C50,/*CE6.7*/
    0x40B3C1,/*ȫ��CE*/
    0x414260,/*CEαװQQ*/
    0x418400,/*CEαװQQ*/
    0x44C740,/*���ӷ��*/
    0x4A9BB4,/*���ӷ��*/
    0x5073FC,/*������CE*/
    0x12C6EFE,/*QE*/
    0x495CA4,/*���*/
    0x12DE787,/*��ҫ*/
    0x44C9B8,/*��ҫ*/
    0x40D5AF,/*�������¼��*/
    0x18D3209,/*��ҫ*/
    0x40110C,/*δ֪*/
    0x4165C1,/*BOIS���㻰*/
    0x233F3386,/*һ��*/
    0x622FFD,/*�Ϸ����*/
    0x114FC7A,/*Gee��ҫ*/
    0x4AA37E,/*���ƽ�*/
    0x594017,/*GEE��ҫ*/
    0x4298F6,/*GEE����*/
    0x4FD970,/*GEE����*/
    0x47B172,/*GEE����*/
    0x6B6EB40,/*GEE��ҫ*/
    0x25F0EF2,/*�������*/
    0x40A735,/*��������*/
    0x0209F0,/*GEE��ʦ*/
    0x2C800A,/*�Ϲǻ�С��*/
    0x10070B75,/*GEE��ҫ*/
    0x4A3C45,/*�����*/
    0x40148C,/*�����*/
    0x545C69,
};
std::set<uint32_t> cheat_gee_set_pt = {
    0x03A72,/*GEE����*/
    0x608E7,/*GEE����*/
    0xF7B63,/*GEE����*/
    0xE30BD,/*GEE����*/
    0x23C5,/*GEE����*/
    0x3DBD,/*GEE����*/
    0x30C1,/*GEE����*/
    0x60D9,/*GEE����*/
    0x60A7,/*GEE����*/
    0x60D1,/*GEE����*/
    0x7AD7,/*GEE����*/
    0x5417,/*GEE����*/
    0x0363,/*GEE����*/
    0xE18C,/*GEE����*/
    0xEBAE,/*���ƹһ�*/
    0x94DD,/*GEE����*/
    0x4AC4,/*GEE����*/
    0xECCA,/*GEEPK*/
    0x1006FB55,
    0x4B45EF,
    0x402644,
    0x192FB9A,
    0xE09808,
    0xE65E94,
    0xE60ACF,
    0xD0E9,
    0x5247,
    0x33ED,
    0x115D7F,
    0xEBAE,
    0x6F659,
    0x2256,
    0x3C3A,
    0xD36B,
    0x443090,
    0xF33A,
    0x03A72,
    0x23C5,
    0x2644,
    0x8063,
    0x545D39,
    0x545B89,/*������*/
    0x475DA8,/*GEE����*/
    0x406393,/*GEE����*/
    0x40AFC8,/*GEE����*/
    0x4106F9,/*GEE����*/
    0x406345,/*GEE����*/
    0x3C7EB40,/*GEE��ʦ*/
    0x6841F0,/*GEE����*/
    0x17A4
};
void ProcessThread(CAntiCheatClient* client)
{
    ProtocolC2STaskEcho resp;
    resp.task_id = TASK_PKG_ID_PROCESS_THREAD_DETECT;
    resp.is_cheat = false;
    auto windows = Utils::CWindows::instance().enum_windows();
    auto process_maps = Utils::CWindows::instance().enum_process();
    auto Sleep = IMPORT(L"kernel32.dll", Sleep);
    for (auto& window : windows)
    {
        if ((window.class_name.find(L"MSCTFIME") == std::wstring::npos
            && window.class_name.find(L"tooltips_class32") == std::wstring::npos
            && window.class_name.find(L"FORM_PROJECT1_FORM1_CLASS:0") == std::wstring::npos
            && window.class_name.find(L"QQPinyinImageCandWndTSF") == std::wstring::npos
            && window.class_name.length() != 10) ||
            process_maps.find(window.pid) == process_maps.end())
        {
            continue;
        }
        auto& process = process_maps[window.pid];
        std::string process_name = Utils::String::w2c(process.name);
        if (!process.threads.size())
            continue;
        for (auto& thread : process.threads)
        {
            if ((window.class_name.find(L"tooltips_class32") != std::wstring::npos || window.class_name.find(L"MSCTFIME"))
                && (cheat_gee_set_pt.find(thread.second.start_address & 0xFFFFFFFF) != cheat_gee_set_pt.end() ||
                    cheat_gee_set_pt.find(thread.second.start_address & 0xFFFFFF) != cheat_gee_set_pt.end()))
            {
                resp.is_cheat = true;
                resp.text = "����GEE���ֻ�����ҫ������Ŵ���1��������������Ϊ:" + process_name;
                break;
            }
            Utils::CWindows::ModuleInfo module;
            if (!process.modules.size() || !Utils::CWindows::instance().get_module_from_address(process.pid, thread.second.start_address, process.modules, module))
            {
                // ***************��ģ��ļ�����***************
                if (cheat_gee_set_pt.find(thread.second.start_address & 0xFFFF) != cheat_gee_set_pt.end() ||
                    cheat_gee_set_pt.find(thread.second.start_address & 0xFFFFF) != cheat_gee_set_pt.end())
                {
                    resp.is_cheat = true;
                    resp.text = "����GEE���ֻ�����ҫ������Ŵ���1��������������Ϊ:" + process_name;
                    break;
                }
                continue;
            }

            if (module.module_name.find(L".exe") != std::wstring::npos)
            {
                if (window.class_name.find(L"QQPinyinImageCandWndTSF") != std::wstring::npos
                    && (thread.second.start_address & 0xFFFF) == 0x3020)
                {
                    resp.is_cheat = true;
                    resp.text = "GEE��ʦ������Ŵ���4��������������Ϊ:" + process_name;
                    break;
                }
                else if (window.class_name.find(L"FORM_PROJECT1_FORM1_CLASS:0") != std::wstring::npos
                    && (thread.second.start_address & 0xFFFF) == 0x2256)
                {
                    resp.is_cheat = true;
                    resp.text = "����GEE���ֻ�����ҫ������Ŵ���5��������������Ϊ:" + process_name;
                    break;
                }
            }
            else if (module.module_name.find(L".dll") != std::wstring::npos)
            {
                if (window.class_name.length() == 10
                    && ((thread.second.start_address & 0xFFFF) == 0xE070
                        || (thread.second.start_address & 0xFFFF) == 0xE480
                        || (thread.second.start_address & 0xFFFF) == 0x8B86
                        || (thread.second.start_address & 0xFFFF) == 0xE750
                        || (thread.second.start_address & 0xFFFFFF) == 0x4012DC))
                {
                    resp.is_cheat = true;
                    resp.text = "���ּ�����ң�����Ϊ:" + process_name;
                    break;
                }
            }
            else if (module.module_name.find(L".tap") != std::wstring::npos
                && ((thread.second.start_address & 0xFFFF) == 0xDD1F
                    || (thread.second.start_address & 0xFFFF) == 0x58F0))
            {
                resp.is_cheat = true;
                resp.text = "������ҫ������Ŵ�������Ϊ��7��������:" + process_name;
                break;
            }
        }
        if (resp.is_cheat)
        {
            break;
        }
        Sleep(1);
    }


    if (!resp.is_cheat)
    {
        for (auto &process : process_maps)
        {
            auto& threads = process.second.threads;
            if (!threads.size())
                continue;
            for (auto &thread : threads)
            {
                Utils::CWindows::ModuleInfo module;
                uint32_t pid = process.second.pid;
                uint64_t thread_start_address = thread.second.start_address;
                std::vector<Utils::CWindows::ModuleInfo>& modules = process.second.modules;

                if (cheat_set_pt.find(thread_start_address) != cheat_set_pt.end())
                {
                    resp.is_cheat = true;
                    resp.text = "���ַ�����߻�����ң�������Ϊ:" + Utils::String::w2c(process.second.name);
                    break;
                }

                if (!modules.size() || !Utils::CWindows::instance().get_module_from_address(pid, thread_start_address, modules, module))
                {
                    continue;
                }


                if (module.module_name.find(L".exe") != std::wstring::npos)
                {
                    if (cheat_exe_set_pt.find(thread.second.start_address & 0xFFFF) != cheat_exe_set_pt.end())
                    {
                        resp.is_cheat = true;
                        resp.text = "������ң�����Ϊ:" + Utils::String::w2c(process.second.name);
                        break;
                    }
                }
                else if (module.module_name.find(L".dll") != std::wstring::npos)
                {
                }
            }
            if (resp.is_cheat)
            {
                break;
            }
            Sleep(1);
        }
    }
    if (resp.is_cheat)
    {
        client->send(&resp);
    }
};

const unsigned int DEFINE_TIMER_ID(kProcessThreadTimerId);
void InitProcessThreadDetect(CAntiCheatClient* client)
{
    client->start_timer(kProcessThreadTimerId, std::chrono::seconds(30), [client]() {
        ProcessThread(client);
    });
}