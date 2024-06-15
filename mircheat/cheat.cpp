<<<<<<< HEAD
#include "cheat.h"

const uint32_t hook_addr[26] = {
    0x00A7FD4C,
    0x00CA5D20,
    0x00D1BA4F,
    0x00C5806F,
    0x00C5814B,
    0x00C5821D,
    0x00C31FE2,
    0x00CF4013,
    0x00C3287B,
    0x00C3293C,
    0x00C329E6,
    0x00C32A82,
    0x00C32B4A,
    0x00C52E62,
    0x00C52FA9,
    0x00C53000,
    0x00C53057,
    0x00C59E33,
    0x00D07388,
    0x00ABF3D4,
    0x00C49A7F,
    0x00C49C96,
    0x00C4A336,
    0x00C4AA69,
    0x00CFAA67,
    0x00CA518F
};
uint32_t xxxx_base_addr = 0;//06446DC8
CRITICAL_SECTION get_tick_count_mutex;
DWORD start_time = 0, new_time=0;

Cheat::Cheat()
{

}

Cheat::~Cheat()
{

}

Cheat& Cheat::instance()
{
    static Cheat instance_;
    return instance_;
}

bool Cheat::hook_init()
{
    auto module = GetModuleHandleW(NULL);
    auto module_size = Singleton<Windows>::getInstance().get_module_size(module);
    std::vector<ptr> result;
    Pattern xxxx_base_addr_sign = {0xA1, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0x00, 0x8B, 0x40, 0xCC, 0x2B, 0xC6, 0x99, 0x33, 0xC2, 0x2B, 0xC2, 0x48, 0x7F};
    size_t result_flag = 0;
    result_flag = xxxx_base_addr_sign.search(module, module_size, result);
    if(result_flag)
    {
        xxxx_base_addr = result.front();
        xxxx_base_addr = *(uint32_t*)(xxxx_base_addr + 1);
    }
    InitializeCriticalSection(&get_tick_count_mutex);
    HookMgr& hookMgr = HookMgr::instance();
    // 变速
    hookMgr.add_inline_hook((uint32_t*)hook_addr[0], speed_handler, nullptr, false); 
    // 移动速度
    hookMgr.add_context_hook((uint32_t*)hook_addr[1], [](LightHook::Context& ctx) {
        general_move_speed_handler(ctx);
    }, &general_move_speed_type);

    // 攻击速度
    hookMgr.add_context_hook((uint32_t*)hook_addr[2], [](Context& ctx)
    {
        general_attack_speed_handler(ctx);
    }, NULL);

    // 移动间隔1
    hookMgr.add_context_hook((uint32_t*)hook_addr[3], [](Context& ctx)
    {
        general_move_interval_handler(ctx);
    }, NULL);

    // 移动间隔2
    hookMgr.add_context_hook((uint32_t*)hook_addr[4], [](Context& ctx) {
        general_move_interval_handler(ctx);
    }, NULL);
    // 攻击间隔
    hookMgr.add_context_hook((uint32_t*)hook_addr[5], [](Context& ctx)
    {
        general_attack_interval_handler(ctx);
    }, &general_attack_interval_type); 

    // 魔法间隔
    hookMgr.add_context_hook((uint32_t*)hook_addr[6], [](Context& ctx) {
        general_magic_interval_handler(ctx);
    }, &general_magic_interval_type);

    // 超级不卡
    /*hookMgr.add_context_hook((uint32_t*)hook_addr[7], [](Context& ctx) {
        general_super_smooth_handler(ctx);
    }, &general_super_smooth_type);*/

    /*if(hookMgr.add_context_hook((uint32_t*)hook_addr[7], [](Context& ctx)
    {
        general_super_smooth_handler(ctx);
    }, &general_super_smooth_type))
    {
        OutputDebugStringA("hook ok");
    }
    else
    {
        OutputDebugStringA("hook fail");

        return false;
    }*/
    return true;
}
// 基本_变速
void Cheat::general_move_speed_handler(Context& ctx)
{
    //LOG_EVENT("ctx.custom_param = 0x%08X  %d", ctx.custom_param, *(uint32_t*)ctx.custom_param);
    uint32_t type = *(uint32_t*)ctx.custom_param;
    uint32_t result = 0;

    // type可能是不同版本的兼容
    switch(type)
    {
    case 1:
        //dword_644633C = 1;
        original_general_move_speed = ctx.esi;
        if(general_speed_checkbox)
            ctx.esi = general_move_speed;
        break;
    case 2:
        //LOG_EVENT("original_general_move_speed =  %d", ctx.ebx);
        original_general_move_speed = ctx.ebx;
        if(general_speed_checkbox)
            ctx.ebx = general_move_speed;
        break;
    case 3:
        original_general_move_speed = *(uint32_t*)(ctx.ebp + 8);
        if(general_speed_checkbox)
        {
            result = ctx.ebp;
            *(uint32_t*)(result + 8) = general_move_speed;
        }
        break;
    case 4:
        original_general_move_speed = *(uint32_t *)ctx.ebp;
        if(general_speed_checkbox)
        {
            result = general_move_speed;
            *(uint32_t *)ctx.ebp = general_move_speed;
        }
        break;
    case 5:
        original_general_move_speed = *(uint32_t *)ctx.edi;
        if(general_speed_checkbox)
        {
            result = ctx.edi;
            *(uint32_t *)result = general_move_speed;
        }
        break;
    case 6:
        original_general_move_speed = *(uint32_t *)ctx.esi;
        if(general_speed_checkbox)
        {
            result = general_move_speed;
            *(uint32_t *)ctx.esi = general_move_speed;
        }
        break;
    default:
        return ;
    }
}

// 基本_变速
void Cheat::general_attack_speed_handler(Context& ctx)
{
    //LOG_EVENT("ctx.custom_param = 0x%08X  %d", ctx.custom_param, *(uint32_t*)ctx.custom_param);
    
    uint32_t result = 0;
    int v3; // ecx

    result = *(uint32_t *)(ctx.ebp - 4);
    v3 = xxxx_base_addr;
    if(xxxx_base_addr)
        v3 = **(uint32_t **)xxxx_base_addr;
    if(result != v3)
    {
        return;
    }
    if(result)
    {
        v3 = *(uint32_t *)(result + 0x620);
        if(v3 == 17)
        {
            original_general_magic_speed = ctx.ebx;
            if(general_speed_checkbox)
            {
                ctx.ebx = general_magic_speed;
            }
            if(fight_fadao_phantom_magic_checkbox)
            {

                *(BYTE *)(result + 0x480) = 0;
            }
            return;
        }
        if((unsigned int)(v3 - 0xE) <= 5)
        {
            result = ctx.ebx;
            original_general_attack_speed = ctx.ebx;
        }
    }
    if(general_speed_checkbox)
    {
        ctx.ebx = general_attack_speed;
    }
}

// 基本_移动间隔
void Cheat::general_move_interval_handler(Context& ctx)
{    
    if(general_speed_checkbox)
    {
        original_general_move_interval = ctx.eax;
        ctx.eax = general_move_interval;
    }
}

// 基本_移动间隔
void Cheat::general_attack_interval_handler(Context& ctx)
{
    uint32_t type = *(uint32_t*)ctx.custom_param;
    if(general_speed_checkbox)
    {
        if(type == 1)
        {
            original_general_attack_interval = ctx.ebx;
            ctx.ebx = general_attack_interval;
        }
        else
        {
            original_general_attack_interval = ctx.esi;
            ctx.esi = general_attack_interval;
        }
    }

}

// 基本_魔法间隔
void Cheat::general_magic_interval_handler(Context& ctx)
{
    if(general_speed_checkbox)
    {
        original_general_magic_interval = ctx.esi;
        ctx.esi = general_magic_interval;
    }
}

// 基本_超级不卡
void Cheat::general_super_smooth_handler(Context& ctx)
{
//    uint32_t *v4,*v5,v2,**v6,**v7;
//    bool v3;
//    if((ctx.esi & 0xFFFFFF00) != 0xB00)
//    {
//        if((ctx.ebx & 0xFFFFFF00) != 0xB00)
//        {
//            if((ctx.eax & 0xFFFFFF00) == 0xB00)
//            {
//                /* LABEL_18:
//                     *(uint8_t*)v2 = sub_63EEC60(v5);
//                     **v6 = v2;
//                     return;*/
//            }
//            v4 = (uint32_t *)ctx.ecx;
//            if((ctx.ecx & 0xFFFFFF00) != 0xB00)
//            {
//                v5 = (char *)ctx.edx;
//                if((ctx.edx & 0xFFFFFF00) == 0xB00)
//                    goto LABEL_18;
//                v2 = ctx.edi;
//                if((v2 & 0xFFFFFF00) != 0xB00)
//                    return;
//                v4 = (uint32_t *)ctx.edi;
//            }
//        }
//        *(uint8_t*)v2 = sub_63EEC60(v4);
//        **v7 = v2;
//        return;
//    }
//    *(uint8_t*)v2 = 1;
//    if(ctx.esi == 0xBC3 || ctx.esi == 0xBC5)
//    {
//        if(general_super_smooth_checkbox)
//            goto LABEL_12;
//        v3 = true;
//    }
//    else
//    {
//        if(ctx.esi != 0xBC9)
//        {
//            if(!general_super_smooth_checkbox)
//            {
//                v3 = fight_warrior_attack_smooth_checkbox == false;
//                goto LABEL_11;
//            }
//        LABEL_12:
//            *(uint8_t*)v2 = 0;
//            goto LABEL_13;
//        }
//        if(general_super_smooth_checkbox)
//            goto LABEL_12;
//        v3 = fight_fadao_magic_smooth_checkbox == false;
//    }
//LABEL_11:
//    if(!v3)
//        goto LABEL_12;
//LABEL_13:
//    ***a1 = v2;
    return;
}

// 基本_变速
uint32_t Cheat::speed_handler()
{
    uint32_t now_time;
    now_time = GetTickCount();
    if(start_time == 0)
    {
        start_time = now_time;
        return now_time;
    }
    EnterCriticalSection(&get_tick_count_mutex);
    if(general_speed_change_checkbox/* && *MEMORY[0x7297FC]*/)
    {
        new_time = (now_time - start_time) * acceleration + new_time;
        start_time = now_time;
        now_time = new_time;        
    }
    LeaveCriticalSection(&get_tick_count_mutex);

    return now_time;
=======
#include "cheat.h"

const uint32_t hook_addr[26] = {
    0x00A7FD4C,
    0x00CA5D20,
    0x00D1BA4F,
    0x00C5806F,
    0x00C5814B,
    0x00C5821D,
    0x00C31FE2,
    0x00CF4013,
    0x00C3287B,
    0x00C3293C,
    0x00C329E6,
    0x00C32A82,
    0x00C32B4A,
    0x00C52E62,
    0x00C52FA9,
    0x00C53000,
    0x00C53057,
    0x00C59E33,
    0x00D07388,
    0x00ABF3D4,
    0x00C49A7F,
    0x00C49C96,
    0x00C4A336,
    0x00C4AA69,
    0x00CFAA67,
    0x00CA518F
};
uint32_t xxxx_base_addr = 0;//06446DC8
CRITICAL_SECTION get_tick_count_mutex;
DWORD start_time = 0, new_time=0;

Cheat::Cheat()
{

}

Cheat::~Cheat()
{

}

Cheat& Cheat::instance()
{
    static Cheat instance_;
    return instance_;
}

bool Cheat::hook_init()
{
    auto module = GetModuleHandleW(NULL);
    auto module_size = Singleton<Windows>::getInstance().get_module_size(module);
    std::vector<ptr> result;
    Pattern xxxx_base_addr_sign = {0xA1, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0x00, 0x8B, 0x40, 0xCC, 0x2B, 0xC6, 0x99, 0x33, 0xC2, 0x2B, 0xC2, 0x48, 0x7F};
    size_t result_flag = 0;
    result_flag = xxxx_base_addr_sign.search(module, module_size, result);
    if(result_flag)
    {
        xxxx_base_addr = result.front();
        xxxx_base_addr = *(uint32_t*)(xxxx_base_addr + 1);
    }
    InitializeCriticalSection(&get_tick_count_mutex);
    HookMgr& hookMgr = HookMgr::instance();
    // 变速
    hookMgr.add_inline_hook((uint32_t*)hook_addr[0], speed_handler, nullptr, false); 
    // 移动速度
    hookMgr.add_context_hook((uint32_t*)hook_addr[1], [](LightHook::Context& ctx) {
        general_move_speed_handler(ctx);
    }, &general_move_speed_type);

    // 攻击速度
    hookMgr.add_context_hook((uint32_t*)hook_addr[2], [](Context& ctx)
    {
        general_attack_speed_handler(ctx);
    }, NULL);

    // 移动间隔1
    hookMgr.add_context_hook((uint32_t*)hook_addr[3], [](Context& ctx)
    {
        general_move_interval_handler(ctx);
    }, NULL);

    // 移动间隔2
    hookMgr.add_context_hook((uint32_t*)hook_addr[4], [](Context& ctx) {
        general_move_interval_handler(ctx);
    }, NULL);
    // 攻击间隔
    hookMgr.add_context_hook((uint32_t*)hook_addr[5], [](Context& ctx)
    {
        general_attack_interval_handler(ctx);
    }, &general_attack_interval_type); 

    // 魔法间隔
    hookMgr.add_context_hook((uint32_t*)hook_addr[6], [](Context& ctx) {
        general_magic_interval_handler(ctx);
    }, &general_magic_interval_type);

    // 超级不卡
    /*hookMgr.add_context_hook((uint32_t*)hook_addr[7], [](Context& ctx) {
        general_super_smooth_handler(ctx);
    }, &general_super_smooth_type);*/

    /*if(hookMgr.add_context_hook((uint32_t*)hook_addr[7], [](Context& ctx)
    {
        general_super_smooth_handler(ctx);
    }, &general_super_smooth_type))
    {
        OutputDebugStringA("hook ok");
    }
    else
    {
        OutputDebugStringA("hook fail");

        return false;
    }*/
    return true;
}
// 基本_变速
void Cheat::general_move_speed_handler(Context& ctx)
{
    //LOG_EVENT("ctx.custom_param = 0x%08X  %d", ctx.custom_param, *(uint32_t*)ctx.custom_param);
    uint32_t type = *(uint32_t*)ctx.custom_param;
    uint32_t result = 0;

    // type可能是不同版本的兼容
    switch(type)
    {
    case 1:
        //dword_644633C = 1;
        original_general_move_speed = ctx.esi;
        if(general_speed_checkbox)
            ctx.esi = general_move_speed;
        break;
    case 2:
        //LOG_EVENT("original_general_move_speed =  %d", ctx.ebx);
        original_general_move_speed = ctx.ebx;
        if(general_speed_checkbox)
            ctx.ebx = general_move_speed;
        break;
    case 3:
        original_general_move_speed = *(uint32_t*)(ctx.ebp + 8);
        if(general_speed_checkbox)
        {
            result = ctx.ebp;
            *(uint32_t*)(result + 8) = general_move_speed;
        }
        break;
    case 4:
        original_general_move_speed = *(uint32_t *)ctx.ebp;
        if(general_speed_checkbox)
        {
            result = general_move_speed;
            *(uint32_t *)ctx.ebp = general_move_speed;
        }
        break;
    case 5:
        original_general_move_speed = *(uint32_t *)ctx.edi;
        if(general_speed_checkbox)
        {
            result = ctx.edi;
            *(uint32_t *)result = general_move_speed;
        }
        break;
    case 6:
        original_general_move_speed = *(uint32_t *)ctx.esi;
        if(general_speed_checkbox)
        {
            result = general_move_speed;
            *(uint32_t *)ctx.esi = general_move_speed;
        }
        break;
    default:
        return ;
    }
}

// 基本_变速
void Cheat::general_attack_speed_handler(Context& ctx)
{
    //LOG_EVENT("ctx.custom_param = 0x%08X  %d", ctx.custom_param, *(uint32_t*)ctx.custom_param);
    
    uint32_t result = 0;
    int v3; // ecx

    result = *(uint32_t *)(ctx.ebp - 4);
    v3 = xxxx_base_addr;
    if(xxxx_base_addr)
        v3 = **(uint32_t **)xxxx_base_addr;
    if(result != v3)
    {
        return;
    }
    if(result)
    {
        v3 = *(uint32_t *)(result + 0x620);
        if(v3 == 17)
        {
            original_general_magic_speed = ctx.ebx;
            if(general_speed_checkbox)
            {
                ctx.ebx = general_magic_speed;
            }
            if(fight_fadao_phantom_magic_checkbox)
            {

                *(BYTE *)(result + 0x480) = 0;
            }
            return;
        }
        if((unsigned int)(v3 - 0xE) <= 5)
        {
            result = ctx.ebx;
            original_general_attack_speed = ctx.ebx;
        }
    }
    if(general_speed_checkbox)
    {
        ctx.ebx = general_attack_speed;
    }
}

// 基本_移动间隔
void Cheat::general_move_interval_handler(Context& ctx)
{    
    if(general_speed_checkbox)
    {
        original_general_move_interval = ctx.eax;
        ctx.eax = general_move_interval;
    }
}

// 基本_移动间隔
void Cheat::general_attack_interval_handler(Context& ctx)
{
    uint32_t type = *(uint32_t*)ctx.custom_param;
    if(general_speed_checkbox)
    {
        if(type == 1)
        {
            original_general_attack_interval = ctx.ebx;
            ctx.ebx = general_attack_interval;
        }
        else
        {
            original_general_attack_interval = ctx.esi;
            ctx.esi = general_attack_interval;
        }
    }

}

// 基本_魔法间隔
void Cheat::general_magic_interval_handler(Context& ctx)
{
    if(general_speed_checkbox)
    {
        original_general_magic_interval = ctx.esi;
        ctx.esi = general_magic_interval;
    }
}

// 基本_超级不卡
void Cheat::general_super_smooth_handler(Context& ctx)
{
//    uint32_t *v4,*v5,v2,**v6,**v7;
//    bool v3;
//    if((ctx.esi & 0xFFFFFF00) != 0xB00)
//    {
//        if((ctx.ebx & 0xFFFFFF00) != 0xB00)
//        {
//            if((ctx.eax & 0xFFFFFF00) == 0xB00)
//            {
//                /* LABEL_18:
//                     *(uint8_t*)v2 = sub_63EEC60(v5);
//                     **v6 = v2;
//                     return;*/
//            }
//            v4 = (uint32_t *)ctx.ecx;
//            if((ctx.ecx & 0xFFFFFF00) != 0xB00)
//            {
//                v5 = (char *)ctx.edx;
//                if((ctx.edx & 0xFFFFFF00) == 0xB00)
//                    goto LABEL_18;
//                v2 = ctx.edi;
//                if((v2 & 0xFFFFFF00) != 0xB00)
//                    return;
//                v4 = (uint32_t *)ctx.edi;
//            }
//        }
//        *(uint8_t*)v2 = sub_63EEC60(v4);
//        **v7 = v2;
//        return;
//    }
//    *(uint8_t*)v2 = 1;
//    if(ctx.esi == 0xBC3 || ctx.esi == 0xBC5)
//    {
//        if(general_super_smooth_checkbox)
//            goto LABEL_12;
//        v3 = true;
//    }
//    else
//    {
//        if(ctx.esi != 0xBC9)
//        {
//            if(!general_super_smooth_checkbox)
//            {
//                v3 = fight_warrior_attack_smooth_checkbox == false;
//                goto LABEL_11;
//            }
//        LABEL_12:
//            *(uint8_t*)v2 = 0;
//            goto LABEL_13;
//        }
//        if(general_super_smooth_checkbox)
//            goto LABEL_12;
//        v3 = fight_fadao_magic_smooth_checkbox == false;
//    }
//LABEL_11:
//    if(!v3)
//        goto LABEL_12;
//LABEL_13:
//    ***a1 = v2;
    return;
}

// 基本_变速
uint32_t Cheat::speed_handler()
{
    uint32_t now_time;
    now_time = GetTickCount();
    if(start_time == 0)
    {
        start_time = now_time;
        return now_time;
    }
    EnterCriticalSection(&get_tick_count_mutex);
    if(general_speed_change_checkbox/* && *MEMORY[0x7297FC]*/)
    {
        new_time = (now_time - start_time) * acceleration + new_time;
        start_time = now_time;
        now_time = new_time;        
    }
    LeaveCriticalSection(&get_tick_count_mutex);

    return now_time;
>>>>>>> 31e167db06d1caa1061611bc914b44d6767746d9
}