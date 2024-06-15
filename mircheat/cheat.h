
#pragma once
#include "cheat_global.h"

using namespace LightHook;

class Cheat
{
public:
    Cheat();
    ~Cheat();
    static Cheat& instance();
//private:
    bool hook_init();
    static void general_move_speed_handler(Context& ctx);
    static void general_attack_speed_handler(Context& ctx);
    static void general_move_interval_handler(Context& ctx);
    static void general_attack_interval_handler(Context & ctx);
    static void general_magic_interval_handler(Context & ctx);
    static void general_super_smooth_handler(Context & ctx);
    static uint32_t speed_handler();
};

