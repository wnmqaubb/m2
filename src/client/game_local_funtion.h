#pragma once
#include <windows.h>
#include <stdint.h>
#include "pattern.hpp"
#include "lighthook.h"


typedef struct _Strings
{
    DWORD top;
    DWORD length;
    char text[1024];
}strings, *pstrings;


class GameLocalFuntion
{

public:
    GameLocalFuntion();
    ~GameLocalFuntion();
    static bool hook_init();
    static GameLocalFuntion& instance();
    void messagebox_call(const std::string msg, uint32_t mb_type = 0);
    void sendmsg_call(const strings msg, uint32_t bgcolor = 0xCBF400, uint32_t font_color = 0x205550);
    static void back_game_msg_countdown(BOOL is_back_game);
    std::string encode_buffer_call(std::string& string_to_encode);
    std::vector<char> encode_buffer_call(unsigned char const * bytes_to_encode, unsigned int in_len);
    std::string decode_buffer_call(std::string & string_to_decode);
    std::vector<char> decode_buffer_call(unsigned char const * bytes_to_decode, unsigned int in_len);
    static void back_game_call(void * param);
    static void game_lazy_action(LightHook::Context& ctx);
    static void back_game_lazy(LightHook::Context& ctx);
    static void exit_game_call(void * param);
    static void exit_game_lazy(LightHook::Context & ctx);
    int guess_encode_bound(int size);
    int sub_ABACC8(int a1);
    int encode_buffer_sub(unsigned char const*src_to_encode, BYTE *dest_to_encode, int src_len, int desc_len);
    int decode_buffer_sub(BYTE *src_to_decode, BYTE *dest_to_decode, int src_len, int desc_len);
    int decode_buffer_sub2(BYTE *src_to_decode, BYTE *dest_to_decode, int src_len);
    bool back_game_lazy_enable_;
    bool exit_game_lazy_enable_;
    uint32_t back_game_lazy_time_;
    uint32_t exit_game_lazy_time_;
};

