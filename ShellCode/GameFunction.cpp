#include "NewClient/pch.h"
#include "GameFunction.h"
#include "BasicUtils.h"

__declspec(dllimport) asio::io_service g_game_io;
LightHook::HookMgr hook_mgr;

uint32_t base_handle_offset_;
uint32_t sendmsg_call_param_offset;
uint32_t send_msg_call_addr_;
uint32_t messagebox_call_addr_;
uint32_t public_call_addr_;

GameLocalFuntion::GameLocalFuntion()
    : back_game_timer_(g_game_io),
    gee_x64(false),
    is_continue_(true),
    can_back_exit_game_(true),
    actor_base_addr_(0),
    actor_speed_interval_base_addr_(0)
{
}

void GameLocalFuntion::call_sig_pattern()
{
    VMP_VIRTUALIZATION_BEGIN();
    auto GetModuleHandleW = IMPORT(L"kernel32.dll", GetModuleHandleW);
    auto module = GetModuleHandleW(NULL);
    auto module_size = ApiResolver::get_image_nt_header(module)->OptionalHeader.SizeOfImage;
    std::vector<ptr> result;

    if (!base_handle_offset_ || !sendmsg_call_param_offset)
    {
        if (edit_handle_key.search(module, module_size, result))
        {
            base_handle_offset_ = result.front();
            sendmsg_call_param_offset = *(uint8_t*)(base_handle_offset_ + 4);
            base_handle_offset_ = *(uint32_t*)(base_handle_offset_ - 4);
        }
    }

    if(!send_msg_call_addr_)
    { 
        result.clear();
        if (send_msg_call_key.search(module, module_size, result))
        {
            send_msg_call_addr_ = (uint32_t)result.front();
            send_msg_call_addr_ -= 8;
        }
        else if (send_msg_call_key1.search(module, module_size, result))
        {
            send_msg_call_addr_ = (uint32_t)result.front();
            send_msg_call_addr_ -= 8;
        }
    }

    if (!messagebox_call_addr_)
    {
        if (messagebox_call_key.search(module, module_size, result))
        {
            messagebox_call_addr_ = (uint32_t)result.front();
        }
    }

    if (!public_call_addr_)
    {
        result.clear();
        if (public_call_key.search(module, module_size, result))
        {
            public_call_addr_ = (uint32_t)result.front();
        }
        else if (public_call_key1.search(module, module_size, result))
        {
            gee_x64 = true;
            public_call_addr_ = (uint32_t)result.front();
        }
    }

    if (!exit_game_call_addr_)
    {
        result.clear();
        if (exit_game_call_key.search(module, module_size, result))
        {
            exit_game_call_addr_ = result.front() + 0x19;
        }
    }


    if (!action_call_addr_)
    {
        result.clear();
        if (action_call_key.search(module, module_size, result))
        {
            action_call_addr_ = result.front();
        }
    }

    if (!ttimer_wndproc_addr_)
    {
        result.clear();
        if (ttimer_wndproc_key.search(module, module_size, result))
        {
            ttimer_wndproc_addr_ = (uint32_t)result.front();
        }
    }

    if (ttimer_wndproc_addr_)
    {
        result.clear();
        if (actor_key.search(module, module_size, result))
        {
            actor_addr_ = result.front();
            actor_addr_ = *(uint32_t*)(actor_addr_ + 1);

            // 移动速度的偏移
            actor_move_speed_offset_ = result.front();
            actor_move_speed_offset_ = *(uint32_t*)(actor_move_speed_offset_ + 0xA);

            actor_speed_interval_addr_ = result.front();
            actor_speed_interval_addr_ = *(uint32_t*)(actor_speed_interval_addr_ + 0x24);

            // 移动速度间隔的偏移
            actor_move_speed_interval_offset_ = result.front();
            actor_move_speed_interval_offset_ = *(uint32_t*)(actor_move_speed_interval_offset_ + 0x2A);
        }
    }

    VMP_VIRTUALIZATION_END();
}

GameLocalFuntion::~GameLocalFuntion()
{

}

bool GameLocalFuntion::hook_init(CAntiCheatClient* client)
{

#ifdef _RTEST
    LOG_ERROR("public_call_addr:%08X", public_call_addr_);
    LOG_ERROR("exit_game_call_addr:%08X", exit_game_call_addr_);
#endif
    client_ = client;
    // 公共功能函数HOOK
    if (public_call_addr_)
    {
        hook_mgr.add_context_hook((uint32_t*)public_call_addr_, [](LightHook::Context& ctx) {
            if (ctx.edx == 0x3F1)
            {
                if (GameLocalFuntion::instance().back_game_lazy_enable_)
                {
                    GameLocalFuntion::instance().can_back_exit_game_ = true;
                    GameLocalFuntion::instance().back_game_notice_countdown(GameLocalFuntion::instance().back_game_lazy_time_, [ctx = ctx]() {
                        hook_mgr.restore();
                        GameLocalFuntion::instance().back_game_call(ctx);
                    });
                    ctx.edx = -1;
                }
            }
        });
    }

    if (exit_game_call_addr_)
    {
        // 大退延时
        hook_mgr.add_context_hook((uint32_t*)exit_game_call_addr_, [](LightHook::Context& ctx) {
            if (ctx.eax == 2)
            {
                return;
            }
            if (GameLocalFuntion::instance().exit_game_lazy_enable_)
            {
                ctx.eax = 2;
                GameLocalFuntion::instance().can_back_exit_game_ = true;
                GameLocalFuntion::instance().back_game_notice_countdown(GameLocalFuntion::instance().exit_game_lazy_time_, []() {
                    Utils::CWindows::instance().exit_process();
                });
            }
        });
    }

    // 动作ID hook
    if (action_call_addr_)
    {
        hook_mgr.add_context_hook((uint32_t*)action_call_addr_, [](LightHook::Context& ctx) {
            GameLocalFuntion::instance().action_id_hook(ctx);
        });
    }

    // 速度检测
    if (ttimer_wndproc_addr_ && actor_addr_)
    {
        set_old_move_speed(-1);
        set_old_hit_speed(-1);
        set_old_spell_speed(-1);
        set_old_move_speed_interval(-1);
        set_old_hit_speed_interval(-1);
        set_old_spell_speed_interval(-1);

        hook_mgr.add_context_hook((uint32_t*)(ttimer_wndproc_addr_+0x16), [](LightHook::Context& ctx) {
            GameLocalFuntion::instance().decode_message_packet_before(ctx);
        });

        hook_mgr.add_context_hook((uint32_t*)(ttimer_wndproc_addr_+0x30), [](LightHook::Context& ctx) {
            GameLocalFuntion::instance().decode_message_packet_after(ctx);
        });

    }

    return true;
}

GameLocalFuntion& GameLocalFuntion::instance()
{
    static GameLocalFuntion instance_;
    return instance_;
}

void GameLocalFuntion::messagebox_call(const std::string text, uint32_t mb_type)
{
    VMP_VIRTUALIZATION_BEGIN();
    if (base_handle_offset_ && messagebox_call_addr_)
    {
        strings msg;
        msg.top = -1;
        msg.length = text.length();
        strcpy_s(msg.text, text.c_str());
        __asm {
            pushad
            push 0
            push 0
            mov eax, base_handle_offset_
            mov eax, dword ptr ds : [eax]
            mov eax, dword ptr ds : [eax]
            mov ecx, mb_type
            lea edx, msg.text
            mov ebx, dword ptr ds : [eax]
            mov esi, messagebox_call_addr_
            call esi
            popad
        }
    }
    else
    {
        auto msgbox = IMPORT(L"user32.dll", MessageBoxA);
        msgbox(NULL, text.c_str(), "封挂提示", MB_OK);
    }

    VMP_VIRTUALIZATION_END();
}

void GameLocalFuntion::notice(const strings msg, uint32_t bgcolor, uint32_t font_color)
{
    VMP_VIRTUALIZATION_BEGIN();
    if (send_msg_call_addr_ && base_handle_offset_ && sendmsg_call_param_offset)
    {
        __asm {
            pushad
            push bgcolor//fcf8fc cbf400 背景色
            mov ecx, font_color //字体颜色
            lea edx, msg.text
            mov eax, base_handle_offset_
            mov eax, dword ptr ds : [eax]
            mov eax, dword ptr ds : [eax]
            mov esi, sendmsg_call_param_offset
            add eax, esi
            mov eax, dword ptr ds : [eax]//2017 + 0x2C
            mov esi, send_msg_call_addr_
            call esi
            popad
        }
    }
    VMP_VIRTUALIZATION_END();
}

void GameLocalFuntion::back_game_notice_countdown(unsigned char tick, std::function<void()> cb)
{
    back_game_timer_.expires_after(std::chrono::seconds(1));
    back_game_timer_.async_wait([this, tick, cb = std::move(cb)](std::error_code ec) {
        if (ec == asio::error::operation_aborted || !can_back_exit_game_)
        {
            return;
        }
        strings msg = { -1, 27, "" };
        snprintf(msg.text, sizeof(msg.text) - 1, "审判封挂提示游戏即将退出%d", tick);
        notice(msg);
        if (tick - 1 == 0)
        {
            if (cb) cb();
            return;
        }
        back_game_notice_countdown(tick - 1, std::move(cb));
    });
}

void GameLocalFuntion::back_game_call(LightHook::Context ctx)
{
    if (!public_call_addr_) return;
    if (gee_x64)
    {
        __asm {
            pushad
            pushfd
            push 0
            push 0
            push 0
            push 0
            push 0
            mov eax, ctx.eax
            mov ecx, 0
            mov edx, ctx.edx
            mov esi, public_call_addr_
            call esi
            popfd
            popad
        }
    }
    else
    {
        __asm {
            pushad
            pushfd
            push 0
            push 0
            push 0
            push 0
            mov eax, ctx.eax
            mov ecx, 0
            mov edx, ctx.edx
            mov esi, public_call_addr_
            call esi
            popfd
            popad
        }
    }
    hook_init(client_);
}

/************************************************************************/
/* CM_TURN = 3010;//转身(方向改变)                                      */
/* CM_WALK = 3011;//走                                                  */
/* CM_SITDOWN = 3012;//挖(蹲下)                                         */
/* CM_RUN = 3013;//跑                                                   */
/* CM_HIT = 3014;//砍 普通物理近身攻击                                  */
/* CM_HEAVYHIT = 3015;// 跳起来打的动作                                 */
/* CM_BIGHIT = 3016;                                                    */
/* CM_SPELL = 3017;//使用魔法                                           */
/* CM_FIREHIT = 3025;//烈火攻击                                         */
/* CM_POWERHIT = 3018;//攻杀 被动技能                                   */
/* CM_TWINHIT = 3038;//开天斩重击                                       */
/* CM_WIDEHIT = 3024;//半月                                             */
/* CM_CRSHIT = 3036; //抱月刀                                           */
/* CM_LONGHIT = 3019;//刺杀                                             */
/* CM_DIGUP    = 3020; //挖是一"起"一"坐",这里是挖动作的"起"            */
/* CM_DIGDOWN  = 3021; //挖动作的"坐"                                   */
/************************************************************************/
void GameLocalFuntion::action_id_hook(LightHook::Context ctx)
{
    VMP_VIRTUALIZATION_BEGIN();

    switch (ctx.edx)
    {
    case CM_WALK:
    case CM_RUN:
    case CM_HIT:
    case CM_HEAVYHIT:
    case CM_BIGHIT:
    case CM_POWERHIT:
    case CM_LONGHIT:
    case CM_WIDEHIT:
    case CM_FIREHIT:
    case 3036:
    case 3037:
    case 3056:// 逐日剑法
    case 3043:
    case 3066:// 开天斩
    case 3166:
    case 3101:
    case 3102:
    case 3103:
    case 3113:
    case 3115:
    case CM_SPELL:
        if (can_back_exit_game_lazy_enable_)
        {
            can_back_exit_game_ = false;
        }
        break;
    default:    break;
    }    

    VMP_VIRTUALIZATION_END();
}


void GameLocalFuntion::decode_message_packet_before(LightHook::Context& ctx)
{
    VMP_VIRTUALIZATION_BEGIN(); 

    auto GetCurrentProcess = IMPORT(L"kernel32.dll", GetCurrentProcess);
       
    if (!actor_addr_ || !actor_speed_interval_addr_)
    {
        return;
    }

    uint32_t bytes_of_read = 0;

    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_addr_, &actor_base_addr_, sizeof(uint32_t), &bytes_of_read);
    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_speed_interval_addr_, &actor_speed_interval_base_addr_, sizeof(uint32_t), &bytes_of_read);
    if (!actor_base_addr_ || !actor_speed_interval_base_addr_) return;


    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_base_addr_, &actor_base_addr_, sizeof(uint32_t), &bytes_of_read);
    if (!actor_base_addr_) return;


    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_base_addr_ + actor_move_speed_offset_, &move_speed_, 2, &bytes_of_read);
    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_base_addr_ + actor_move_speed_offset_ + 2, &hit_speed_, 2, &bytes_of_read);
    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_base_addr_ + actor_move_speed_offset_ + 4, &spell_speed_, 2, &bytes_of_read);

    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_speed_interval_base_addr_ + actor_move_speed_interval_offset_, &move_speed_interval_, 4, &bytes_of_read);
    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_speed_interval_base_addr_ + actor_move_speed_interval_offset_ - 4, &hit_speed_interval_, 4, &bytes_of_read);
    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_speed_interval_base_addr_ + actor_move_speed_interval_offset_ + 4, &spell_speed_interval_, 4, &bytes_of_read);

    std::string resp_text;
    int16_t action_speed = 0;
    int16_t old_action_speed = 0;
    int32_t old_move_speed = get_old_move_speed();
    int32_t old_hit_speed = get_old_hit_speed();
    int32_t old_spell_speed = get_old_spell_speed();
    int32_t old_move_speed_interval = get_old_move_speed_interval();
    int32_t old_hit_speed_interval = get_old_hit_speed_interval();
    int32_t old_spell_speed_interval = get_old_spell_speed_interval();

    bool is_cheat_ = false;

    if (old_move_speed && old_move_speed != -1 && move_speed_ != old_move_speed)
    {
        is_cheat_ = true;
        action_speed = move_speed_;
        old_action_speed = old_move_speed;
        resp_text = xorstr("检测到移动加速作弊");
    }

    if (!is_cheat_ && old_hit_speed  && old_hit_speed != -1 && hit_speed_ != old_hit_speed)
    {
        is_cheat_ = true;
        action_speed = hit_speed_;
        old_action_speed = old_hit_speed;
        resp_text = xorstr("检测到攻击加速作弊");
    }

    if (!is_cheat_ && old_spell_speed && old_spell_speed != -1 && spell_speed_ != old_spell_speed)
    {
        is_cheat_ = true;
        action_speed = spell_speed_;
        old_action_speed = old_spell_speed;
        resp_text = xorstr("检测到魔法加速作弊");
    }

    if (!is_cheat_ && old_move_speed_interval && old_move_speed_interval != -1 && move_speed_interval_ != old_move_speed_interval)
    {
        is_cheat_ = true;
        action_speed = move_speed_interval_;
        old_action_speed = get_old_move_speed_interval();
        resp_text = xorstr("检测到移动加速间隔作弊");
    }

    if (!is_cheat_ && old_hit_speed_interval && old_hit_speed_interval != -1 && hit_speed_interval_ != old_hit_speed_interval)
    {
        is_cheat_ = true;
        action_speed = hit_speed_interval_;
        old_action_speed = old_hit_speed_interval;
        resp_text = xorstr("检测到攻击加速间隔作弊");
    }

    if (!is_cheat_ && old_spell_speed_interval && old_spell_speed_interval != -1 && spell_speed_interval_ != old_spell_speed_interval)
    {
        is_cheat_ = true;
        action_speed = spell_speed_interval_;
        old_action_speed = old_spell_speed_interval;
        resp_text = xorstr("检测到魔法加速间隔作弊");
    }

    // action_speed == 0的不报,切换角色时会误报
    if (is_cheat_ && action_speed)
    {
        ProtocolC2STaskEcho resp;
        resp.task_id = 9024;
        resp.text = resp_text + xorstr(",速度:[") + std::to_string(old_action_speed) + " >> " + std::to_string(action_speed) + "]";
        resp.is_cheat = true;
        client_->send(&resp);
    }
    VMP_VIRTUALIZATION_END();
}

void GameLocalFuntion::decode_message_packet_after(LightHook::Context& ctx)
{
    VMP_VIRTUALIZATION_BEGIN(); 

    int16_t t1 = 0, t2 = 0, t3 = 0;
    int32_t t4 = 0, t5 = 0, t6 = 0;
    uint32_t bytes_of_read = 0;

    auto GetCurrentProcess = IMPORT(L"kernel32.dll", GetCurrentProcess);
       
    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_base_addr_ + actor_move_speed_offset_, &t1, 2, &bytes_of_read);
    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_base_addr_ + actor_move_speed_offset_ + 2, &t2, 2, &bytes_of_read);
    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_base_addr_ + actor_move_speed_offset_ + 4, &t3, 2, &bytes_of_read);        

    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_speed_interval_base_addr_ + actor_move_speed_interval_offset_, &t4, 4, &bytes_of_read);
    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_speed_interval_base_addr_ + actor_move_speed_interval_offset_ - 4, &t5, 4, &bytes_of_read);
    BasicUtils::read_virtual_memory(GetCurrentProcess(), actor_speed_interval_base_addr_ + actor_move_speed_interval_offset_ + 4, &t6, 4, &bytes_of_read);

    set_old_move_speed(t1);
    set_old_hit_speed(t2);
    set_old_spell_speed(t3);

    set_old_move_speed_interval(t4);
    set_old_hit_speed_interval(t5);
    set_old_spell_speed_interval(t6);
    VMP_VIRTUALIZATION_END();
}

