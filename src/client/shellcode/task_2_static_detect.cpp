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
                0x36836402,/*QQ,工具箱9*/
                0x73AE920E,/*2018吉吉辅助*/
                0x65B26960,/*2018暗龙辅助,暗龙2020*/
                0x2C0A0991,/*32位,64位,PCHunter32,PCHunter64*/
                0x3287C659,/*9*/
                0x319739C5,/*A版挂机脚本编辑器,A版本脚本编辑器,A版脚本编辑器,gee-gom脚本编辑器*/
                0x216BC56A,/*esp-ZW1205*/
                0x36C8E71B,/*FSCrossHair*/
                0x0EFB8F3F,/*FSRecorder*/
                0x7573059C,/*g5ZlwYJe0k*/
                0x30C25D57,/*geem2会员版【官方正版】,一刀插件免费版,会员通杀版【官方正版】*/
                0x1DD85266,/*GEE通用版本*/
                0x31C2A2DC,/*K-2,PK加速*/
                0x02479776,/*PK*/
                0x568BFD60,/*PK专用插件2*/
                0x0CD3D104,/*tQqU*/
                0x561B25FC,/*Update,刺客7*/
                0x2A615ACE,/*VAR乱地图账号密码界面处理*/
                0x5B2E53F9,/*vip-旗舰版,秒杀辅助2*/
                0x4ADE709C,/*WinSrvManager*/
                0x1D8C50E6,/*一刀插件收费版*/
                0x658F46D1,/*七剑辅助2020通杀版*/
                0x3A1622A2,/*万千辅助免费版,万千辅助收费版*/
                0x0874F3E7,/*传奇易大师*/
                0x163BDD02,/*传奇通用变速器免费版,传奇通用变速器收费版*/
                0x781DB673,/*刀速B版*/
                0x4B4B255B,/*刺客6*/
                0x429F463F,/*十c-vip官方会员版*/
                0x3E18CC07,/*单挑王加速器2*/
                0x74ED714B,/*及时雨v1*/
                0x74B40E08,/*吕布2020*/
                0x5B97A6C6,/*处理器*/
                0x387D9B76,/*守护者收费版*/
                0x4CA2F0FA,/*小可爱官方会员版*/
                0x693A68F1,/*战舞客户端0627*/
                0x4F77C34F,/*新脚本编辑器*/
                0x7C8AAB17,/*晴天收费版*/
                0x106E4805,/*晴天至尊-2019-4-VIP版,晴天辅助会员版【官方正版】*/
                0x429D5895,/*暗魂免费版1*/
                0x2D3EE3C0,/*横刀*/
                0x6FE9E52D,/*永恒狂刀5*/
                0x32A5B1DD,/*永恒狂刀获取NPC命令工具*/
                0x00620091,/*灭魂辅助免费版*/
                0x05CC0284,/*点此打开*/
                0x6AB81BD9,/*烈日辅助官方会员版*/
                0x005B2B89,/*狩猎3,狩猎3,狩猎加速器,狩猎者3*/
                0x36ED51F5,/*王中王免费版,王中王收费版*/
                0x398C93E3,/*简单挂免费版*/
                0x00D78C2F,/*简单挂收费版*/
                0x20C8FEBA,/*脚本编辑器2019V1*/
                0x3BB7AFC2,/*脱机版*/
                0x6A591680,/*自动更新*/
                0x17BD85A0,/*简单类*/
				0x72827FF9,/*刺客辅助*/
				0x7AF3013E,/*360急救箱_32位,360急救箱_64位*/
				0x41E7D96A,/*AliApp*/
				0x2C4A0BC8,/*GEE,启动器*/
				0x094945A1,/*GH处理,YDS工具,巡航工具*/
				0x0AE5D4C4,/*HW辉煌处理器,易大师定制版处理器*/
				0x6AEBCFC0,/*LeiGodSetup*/
				0x07985495,/*NetSpeed*/
				0x3A1A34B8,/*点击启动*/
				0x000BAD8A,/*移动宽带用户登录,移动宽带登录线路*/
				0x36836402,/*明月*/
				0x43074645,/*前锋加速*/
				0x41E7D96A,/*AliApp*/
				0x6C085F40,/*GEE大师*/
				0x075091E7,/*GEE猎手*/
				0x482A2F05,/*GEE管家会员版*/
				0x568BFD60,/*K PK外挂*/
				0x31C2A2DC,/*K加速*/
				0x02479776,/*PK辅助,天魔辅助*/
				0x0082E27A,/*七星辅助*/
				0x29D2DE39,/*乾坤加速器*/
				0x32BF3E3E,/*传奇私服扫号器V3*/
				0x022459C8,/*刺客辅助*/
				0x3E18CC07,/*单挑王加速器*/
				0x0C5CB728,/*大师多开器*/
				0x6585A6FD,/*天使辅助*/
				0x6C37B144,/*天逸辅助*/
				0x3A1A34B8,/*奇刃辅助*/
				0x48BEFD21,/*小手、飘逸*/
				0x693A68F1,/*战舞辅助*/
				0x106E4805,/*晴天至尊*/
				0x65B26960,/*暗龙辅助*/
				0x6FE9E52D,/*永恒狂刀*/
				0x32A5B1DD,/*永恒狂刀获取NPC命令工具*/
				0x00620091,/*灭魂辅助*/
				0x005B2B89,/*狩猎者加速器*/
				0x3287C659,/*老冰狼*/
				0x769CEE7A,/*老好人辅助*/
				0x2C4A0BC8,/*荣耀辅助*/
				0x7573059C,/*超级变速器*/
				0x4DDE8C1B,/*速战辅助*/
				0x70FB35E3,/*飘刀加速*/
				0x7D3EBBF4,/*晴天至尊*/
				0x3C271E7B,/*晴天至尊1*/
                0x303E4C29,/*剑魂辅助*/
                0x5B881226,/*无敌辅助*/
                0x00D78C2F,/*简单辅助*/
                0x0A4C0BD9,/*大名外挂*/
                0x592D0452,/*简单外挂*/
                0x17BD85A0,/*简单外挂*/
                0x0A6C741E,/*秒杀*/
                0x49630BA3,/*AliIM.exe*/
                0x1443563C,/*2020小可爱.exe_*/
                0x0E32E1A5,/*简尚自动按键_*/
                0x73C4E5F7,/*永恒狂刀*/
                0x422EBE34,/*不死传说.exe*/
                0x122891F6,/*主宰辅助官方免费版.exe*/
                0x65D8B6C9,/*WPE.exe*/
                0x749F70F9,/*CE*/
                0x43F18F71,/*cheatengine*/
                0x2DD47CBD,/*VE封包工具*/
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