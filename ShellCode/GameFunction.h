#pragma once
#include <windows.h>
#include <stdint.h>
#include "lightbone/pattern.hpp"
#include "lightbone/lighthook.h"
#include "lightbone/utils.h"
#include "Service/AntiCheatClient.h"

typedef struct _Strings
{
    DWORD top;
    DWORD length;
    char text[1024];
}strings, *pstrings;

enum Action_Id
{
    CM_TURN = 3010,//转身(方向改变)                           
    CM_WALK = 3011,//走                                       
    CM_SITDOWN = 3012,//挖(蹲下)                              
    CM_RUN = 3013,//跑                                        
    CM_HIT = 3014,//砍 普通物理近身攻击                       
    CM_HEAVYHIT = 3015,// 跳起来打的动作                      
    CM_BIGHIT = 3016,
    CM_SPELL = 3017,//使用魔法                                
    CM_FIREHIT = 3025,//烈火攻击                              
    CM_POWERHIT = 3018,//攻杀 被动技能                        
    CM_TWINHIT = 3038,//开天斩重击                            
    CM_WIDEHIT = 3024,//半月                                  
    CM_CRSHIT = 3036, //抱月刀                                
    CM_LONGHIT = 3019,//刺杀                                  
    CM_DIGUP = 3020, //挖是一"起"一"坐",这里是挖动作的"起" 
    CM_DIGDOWN = 3021, //挖动作的"坐"                        
};

class GameLocalFuntion
{

public:
    GameLocalFuntion();
    ~GameLocalFuntion();
    static GameLocalFuntion& instance();
    bool hook_init();
    void messagebox_call(std::string msg, uint32_t mb_type = 0);
    void notice(const strings msg, uint32_t bgcolor = 0xFFFFFF, uint32_t font_color = 0x0000ff);
    void back_game_notice_countdown(unsigned char tick, std::function<void()> cb);
    void back_game_call(LightHook::Context ctx);
    void action_id_hook(LightHook::Context ctx);
    void decode_message_packet_before(LightHook::Context& ctx);
    void decode_message_packet_after(LightHook::Context& ctx);
    void center_messagebox(HWND hParent, const char* lpText, const char* lpCaption, UINT uType);
    void call_sig_pattern();

    int32_t get_old_move_speed()
    {
        return old_move_speed_ ^ old_speed_xor_key;
    }
    int32_t get_old_hit_speed()
    {
        return old_hit_speed_ ^ old_speed_xor_key;
    }
    int32_t get_old_spell_speed()
    {
        return old_spell_speed_ ^ old_speed_xor_key;
    }
    void set_old_move_speed(int32_t old_move_speed)
    {
        old_move_speed_ = old_move_speed ^ old_speed_xor_key;
    }
    void set_old_hit_speed(int32_t old_hit_speed)
    {
        old_hit_speed_ = old_hit_speed ^ old_speed_xor_key;
    }
    void set_old_spell_speed(int32_t old_spell_speed)
    {
        old_spell_speed_ = old_spell_speed ^ old_speed_xor_key;
    }
    int32_t get_old_move_speed_interval()
    {
        return old_move_speed_interval_ ^ old_speed_xor_key;
    }
    int32_t get_old_hit_speed_interval()
    {
        return old_hit_speed_interval_ ^ old_speed_xor_key;
    }
    int32_t get_old_spell_speed_interval()
    {
        return old_spell_speed_interval_ ^ old_speed_xor_key;
    }
    void set_old_move_speed_interval(int32_t old_move_speed_interval)
    {
        old_move_speed_interval_ = old_move_speed_interval ^ old_speed_xor_key;
    }
    void set_old_hit_speed_interval(int32_t old_hit_speed_interval)
    {
        old_hit_speed_interval_ = old_hit_speed_interval ^ old_speed_xor_key;
    }
    void set_old_spell_speed_interval(int32_t old_spell_speed_interval)
    {
        old_spell_speed_interval_ = old_spell_speed_interval ^ old_speed_xor_key;
    }

    bool back_game_lazy_enable_;
    bool exit_game_lazy_enable_;
    bool can_back_exit_game_lazy_enable_;
    uint32_t back_game_lazy_time_;
    uint32_t exit_game_lazy_time_;
    asio::steady_timer back_game_timer_;
    // std::unordered_map<Action_Id, std::tuple<threshold, last_time, count, average>> 
    std::unordered_map<uint16_t, std::tuple<uint32_t, uint32_t, uint8_t, uint32_t>> action_time_;

private:
    bool gee_x64;
    bool is_continue_;
    bool can_back_exit_game_;// 在大小退的倒计时内，如果玩家有移动，攻击，魔法的行为就取消退出
    uint32_t actor_base_addr_;
    uint32_t actor_speed_interval_base_addr_;
    int32_t old_move_speed_;
    int32_t old_hit_speed_;
    int32_t old_spell_speed_;
    int32_t old_move_speed_interval_;
    int32_t old_hit_speed_interval_;
    int32_t old_spell_speed_interval_;
    const uint32_t old_speed_xor_key = 0x3FA795C;

    uint32_t exit_game_call_addr_;
    uint32_t action_call_addr_;
    //TTimer::WndProc(Messages::TMessage &)
    uint32_t ttimer_wndproc_addr_;
    // 人物角色属性基址
    uint32_t actor_addr_;
    // 人物属性速度间隔基址
    uint32_t actor_speed_interval_addr_;
    // 人物角色属性_移动速度偏移 攻击速度= 移动速度偏移 + 2 魔法速度= 移动速度偏移 + 4
    uint32_t actor_move_speed_offset_;
    // 人物属性_攻击速度间隔偏移 移动速度= 攻击速度偏移 + 4 魔法速度= 攻击速度偏移 + 8
    uint32_t actor_move_speed_interval_offset_;
    int16_t move_speed_;
    int16_t hit_speed_;
    int16_t spell_speed_;
    int16_t move_speed_interval_;
    int16_t hit_speed_interval_;
    int16_t spell_speed_interval_;
    Pattern send_msg_call_key = {
        0x57, 0x33, 0xDB, 0x89, 0x5D, 0xE0, 0x89, 0x5D, 0xF4, 0x89, 0x4D, 0xF8, 0x8B, 0xF2, 0x89, 0x45
    };
    Pattern send_msg_call_key1 = {
        0x57, 0x33, 0xDB, 0x89, 0x5D, 0xE4, 0x89, 0x5D, 0xFC, 0x89, 0x4D, 0xF4, 0x8B, 0xDA, 0x89, 0x45
    };
    Pattern edit_handle_key = {
        0x8B, 0x00, 0x8B, 0x40, 0xCC, 0x8B, 0xCB, 0x8B, 0x55, 0xFC, 0xE8
    };

    Pattern messagebox_call_key = {
        0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xE8, 0x53, 0x56, 0x57, 0x66, 0x89, 0x4D, 0xFA, 0x89, 0x55, 0xFC
    };
    Pattern exit_game_call_key = {
        0x8B, 0x55, 0xFC, 0xA1, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0x00, 0xCC, 0xCC, 0x0D, 0xCC, 0xCC, 0xCC,
        0xCC, 0x8B, 0x38, 0xFF, 0x97, 0xCC, 0xCC, 0xCC, 0xCC, 0x48
    };
    Pattern public_call_key = {
        0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xE8, 0x53, 0x56, 0x57, 0x33, 0xDB, 0x89, 0x5D, 0xE8, 0x89, 0x5D,
        0xF0, 0x89, 0x5D, 0xEC
    };
    Pattern public_call_key1 = {
        0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xE4, 0x53, 0x56, 0x57, 0x33, 0xDB, 0x89, 0x5D, 0xE4, 0x89, 0x5D,
        0xEC, 0x89, 0x5D, 0xE8, 0x8B, 0xF9, 0x8B, 0xF2, 0x8B, 0xD8, 0x8B, 0x45, 0x08, 0xE8
    };
    Pattern action_call_key = {
        0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xCC, 0x53, 0x56, 0x57, 0x33, 0xDB, 0x89, 0x5D, 0xCC, 0x89, 0x5D,
        0xCC, 0x8B, 0xF9, 0x8B, 0xDA, 0x89, 0x45, 0xFC, 0x8B, 0x75, 0xCC, 0x8B, 0x45, 0x08, 0xE8
    };
    // 20181004
    Pattern ttimer_wndproc_key = {
        0x55, 0x8B, 0xEC, 0x51, 0x53, 0x56, 0x57, 0x8B, 0xDA, 0x89, 0x45, 0xFC, 0x8B, 0x33, 0x81, 0xFE,
        0x13, 0x01, 0x00, 0x00, 0x75, 0x3F, 0x33, 0xC0
    };
    Pattern actor_key = {
        0xA1, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0x00, 0x66, 0x8B, 0xB0, 0xCC, 0xCC, 0xCC, 0xCC, 0x66, 0x85,
        0xF6, 0x74, 0x38, 0x8B, 0xC3, 0x99, 0x52, 0x50, 0xA1
    };
};