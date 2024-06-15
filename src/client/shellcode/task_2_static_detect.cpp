#include "anticheat.h"
#include <stdint.h>
#include <string>
#include <set>
#include "protocol.h"
#include "utils/utils.h"
#include "utils/api_resolver.h"

namespace ShellCode
{
    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            set_interval(30000);
            set_package_id(SHELLCODE_PACKAGE_ID(2));
            cheat_set = {
                0x22304296,/*GEEjkaaa*/
                0x33517512,/*FSCapture*/
                0x36836402,/*QQ,������9*/
                0x73AE920E,/*2018��������*/
                0x65B26960,/*2018��������,����2020*/
                0x2C0A0991,/*32λ,64λ,PCHunter32,PCHunter64*/
                0x3287C659,/*9*/
                0x319739C5,/*A��һ��ű��༭��,A�汾�ű��༭��,A��ű��༭��,gee-gom�ű��༭��*/
                0x216BC56A,/*esp-ZW1205*/
                0x36C8E71B,/*FSCrossHair*/
                0x0EFB8F3F,/*FSRecorder*/
                0x7573059C,/*g5ZlwYJe0k*/
                0x30C25D57,/*geem2��Ա�桾�ٷ����桿,һ�������Ѱ�,��Աͨɱ�桾�ٷ����桿*/
                0x1DD85266,/*GEEͨ�ð汾*/
                0x31C2A2DC,/*K-2,PK����*/
                0x02479776,/*PK*/
                0x568BFD60,/*PKר�ò��2*/
                0x0CD3D104,/*tQqU*/
                0x561B25FC,/*Update,�̿�7*/
                0x2A615ACE,/*VAR�ҵ�ͼ�˺�������洦��*/
                0x5B2E53F9,/*vip-�콢��,��ɱ����2*/
                0x4ADE709C,/*WinSrvManager*/
                0x1D8C50E6,/*һ������շѰ�*/
                0x658F46D1,/*�߽�����2020ͨɱ��*/
                0x3A1622A2,/*��ǧ������Ѱ�,��ǧ�����շѰ�*/
                0x0874F3E7,/*�����״�ʦ*/
                0x163BDD02,/*����ͨ�ñ�������Ѱ�,����ͨ�ñ������շѰ�*/
                0x781DB673,/*����B��*/
                0x4B4B255B,/*�̿�6*/
                0x429F463F,/*ʮc-vip�ٷ���Ա��*/
                0x3E18CC07,/*������������2*/
                0x74ED714B,/*��ʱ��v1*/
                0x74B40E08,/*����2020*/
                0x5B97A6C6,/*������*/
                0x387D9B76,/*�ػ����շѰ�*/
                0x4CA2F0FA,/*С�ɰ��ٷ���Ա��*/
                0x693A68F1,/*ս��ͻ���0627*/
                0x4F77C34F,/*�½ű��༭��*/
                0x7C8AAB17,/*�����շѰ�*/
                0x106E4805,/*��������-2019-4-VIP��,���츨����Ա�桾�ٷ����桿*/
                0x429D5895,/*������Ѱ�1*/
                0x2D3EE3C0,/*�ᵶ*/
                0x6FE9E52D,/*�����5*/
                0x32A5B1DD,/*����񵶻�ȡNPC�����*/
                0x00620091,/*��긨����Ѱ�*/
                0x05CC0284,/*��˴�*/
                0x6AB81BD9,/*���ո����ٷ���Ա��*/
                0x005B2B89,/*����3,����3,���Լ�����,������3*/
                0x36ED51F5,/*��������Ѱ�,�������շѰ�*/
                0x398C93E3,/*�򵥹���Ѱ�*/
                0x00D78C2F,/*�򵥹��շѰ�*/
                0x20C8FEBA,/*�ű��༭��2019V1*/
                0x3BB7AFC2,/*�ѻ���*/
                0x6A591680,/*�Զ�����*/
                0x17BD85A0,/*����*/
				0x72827FF9,/*�̿͸���*/
				0x7AF3013E,/*360������_32λ,360������_64λ*/
				0x41E7D96A,/*AliApp*/
				0x2C4A0BC8,/*GEE,������*/
				0x094945A1,/*GH����,YDS����,Ѳ������*/
				0x0AE5D4C4,/*HW�Իʹ�����,�״�ʦ���ư洦����*/
				0x6AEBCFC0,/*LeiGodSetup*/
				0x07985495,/*NetSpeed*/
				0x3A1A34B8,/*�������*/
				0x000BAD8A,/*�ƶ�����û���¼,�ƶ������¼��·*/
				0x36836402,/*����*/
				0x43074645,/*ǰ�����*/
				0x41E7D96A,/*AliApp*/
				0x6C085F40,/*GEE��ʦ*/
				0x075091E7,/*GEE����*/
				0x482A2F05,/*GEE�ܼһ�Ա��*/
				0x568BFD60,/*K PK���*/
				0x31C2A2DC,/*K����*/
				0x02479776,/*PK����,��ħ����*/
				0x0082E27A,/*���Ǹ���*/
				0x29D2DE39,/*Ǭ��������*/
				0x32BF3E3E,/*����˽��ɨ����V3*/
				0x022459C8,/*�̿͸���*/
				0x3E18CC07,/*������������*/
				0x0C5CB728,/*��ʦ�࿪��*/
				0x6585A6FD,/*��ʹ����*/
				0x6C37B144,/*���ݸ���*/
				0x3A1A34B8,/*���и���*/
				0x48BEFD21,/*С�֡�Ʈ��*/
				0x693A68F1,/*ս�踨��*/
				0x106E4805,/*��������*/
				0x65B26960,/*��������*/
				0x6FE9E52D,/*�����*/
				0x32A5B1DD,/*����񵶻�ȡNPC�����*/
				0x00620091,/*��긨��*/
				0x005B2B89,/*�����߼�����*/
				0x3287C659,/*�ϱ���*/
				0x769CEE7A,/*�Ϻ��˸���*/
				0x2C4A0BC8,/*��ҫ����*/
				0x7573059C,/*����������*/
				0x4DDE8C1B,/*��ս����*/
				0x70FB35E3,/*Ʈ������*/
				0x7D3EBBF4,/*��������*/
				0x3C271E7B,/*��������1*/
                0x303E4C29,/*���긨��*/
                0x5B881226,/*�޵и���*/
                0x00D78C2F,/*�򵥸���*/
                0x0A4C0BD9,/*�������*/
                0x592D0452,/*�����*/
                0x17BD85A0,/*�����*/
                0x0A6C741E,/*��ɱ*/
                0x49630BA3,/*AliIM.exe*/
                0x1443563C,/*2020С�ɰ�.exe_*/
                0x0E32E1A5,/*�����Զ�����_*/
                0x73C4E5F7,/*�����*/
                0x422EBE34,/*������˵.exe*/
                0x122891F6,/*���׸����ٷ���Ѱ�.exe*/
                0x65D8B6C9,/*WPE.exe*/
                0x749F70F9,/*CE*/
                0x43F18F71,/*cheatengine*/
                0x2DD47CBD,/*VE�������*/
            };
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
            ProtocolShellCodeInstance proto;
            proto.id = get_package_id();
            auto processes = Utils::CWindows::instance().enum_process();
            for (auto& process : processes)
            {
                if (process.second.modules.size() == 0)
                {
                    continue;
                }
                uint32_t hash_val = 0;
                if (!Utils::PEScan::calc_pe_ico_hash(process.second.modules.front().path, &hash_val))
                {
                    continue;
                }
                if (cheat_set.find(hash_val) != cheat_set.end())
                {
                    proto.is_cheat = true;
                    char buffer[30] = {0};
                    snprintf(buffer, sizeof(buffer), "%08X", hash_val);
                    proto.reason += process.second.modules.front().path + L"|"+ Utils::string2wstring(buffer) + L";";
                }
            }
            if(proto.is_cheat)
            {
                AntiCheat::instance().send(proto);
            }
        };
        std::set<uint32_t> cheat_set;
    };
    uint32_t main()
    {
        TaskStaticDetect* task = new TaskStaticDetect();
        if (AntiCheat::instance().add_task(task))
        {
            return 0;
        }
        else
        {
            delete task;
            return 1;
        }
    }
}