#include "anticheat.h"
#include "game_local_funtion.h"
#include "protocol.h"
#include "VMProtectSDK.h"
#include "utils\utils.h"
#include "utils\api_resolver.h"
#include <process.h>
#include "memory.h"

uint32_t base_handle_offset_;//8B 00 8B 40 ? 8B CB 8B 55 FC E8
uint32_t sendmsg_call_param_offset;
uint32_t send_msg_call_addr_;//57 33 DB 89 5D E0 89 5D F4 89 4D F8 8B F2 89 45
uint32_t messagebox_call_addr_;//55 8B EC 83 C4 E8 53 56 57 66 89 4D FA 89 55 FC
// 小退CALL 6A 00 6A 00 6A 00 6A 00 33 C9 BA F1 03 00 00 8B
//uint32_t back_game_call_addr_;
// 大退CALL 48 75 ? 6A 00 6A 00 6A 00 6A 00 33 C9 BA BA 14 00 00 8B C3 E8//不兼容2017版本的
// 大退CALL 8B 55 FC A1 ? ? ? ? 8B 00 66 8B 0D ? ? ? ? 8B 38 FF 97 ? ? ? ? 48
uint32_t exit_game_call_addr_;
// 公用功能CALL (小退,大退,进入游戏,等) 55 8B EC 83 C4 E8 53 56 57 33 DB 89 5D E8 89 5D F0 89 5D EC
//call 00C4EA14(eax, edx(功能号), ecx, int, int, int, int)
bool gee_x64 = false;
uint32_t public_call_addr_;
//void (WINAPI *back_game_call)(uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7);

LightHook::Context context;
HANDLE current_process_handle = ::GetCurrentProcess();
GameLocalFuntion::GameLocalFuntion()
{
    VMP_VIRTUALIZATION_BEGIN();
    auto GetModuleHandleW = IMPORT(L"kernel32.dll", GetModuleHandleW);
    auto module = GetModuleHandleW(NULL);
    auto module_size = Singleton<Windows>::getInstance().get_module_size(module);
    std::vector<ptr> result;

    Pattern send_msg_call_key = {0x57, 0x33, 0xDB, 0x89, 0x5D, 0xE0, 0x89, 0x5D, 0xF4, 0x89, 0x4D, 0xF8, 0x8B, 0xF2, 0x89, 0x45};
    Pattern messagebox_call_key = {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xE8, 0x53, 0x56, 0x57, 0x66, 0x89, 0x4D, 0xFA, 0x89, 0x55, 0xFC};
    Pattern edit_handle_key = {0x8B, 0x00, 0x8B, 0x40, 0xCC, 0x8B, 0xCB, 0x8B, 0x55, 0xFC, 0xE8};
    // 小退延时3秒
    //Pattern back_game_call_key = {0x55, 0x8B, 0xEC, 0x51, 0x53, 0x89, 0x45, 0xFC, 0x8B, 0x45, 0xFC, 0x83, 0xC0, 0x08, 0x50, 0xE8};
    //Pattern back_game_call_key = {0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x33, 0xC9, 0xBA, 0xF1, 0x03, 0x00, 0x00, 0x8B};
    //Pattern exit_game_call_key = {0x48, 0x75, 0xCC, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0x33, 0xC9, 0xBA, 0xBA, 0x14, 0x00, 0x00, 0x8B, 0xC3, 0xE8};
    Pattern exit_game_call_key = {0x8B, 0x55, 0xFC, 0xA1, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0x00, 0x66, 0x8B, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0x8B, 0x38, 0xFF, 0x97, 0xCC, 0xCC, 0xCC, 0xCC, 0x48};
    Pattern public_call_key = {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xE8, 0x53, 0x56, 0x57, 0x33, 0xDB, 0x89, 0x5D, 0xE8, 0x89, 0x5D, 0xF0, 0x89, 0x5D, 0xEC};
    Pattern public_call_key1 = {0x55, 0x8B, 0xEC, 0x83, 0xC4, 0xE4, 0x53, 0x56, 0x57, 0x33, 0xDB, 0x89, 0x5D, 0xE4, 0x89, 0x5D, 0xEC, 0x89, 0x5D, 0xE8,
        0x8B, 0xF9, 0x8B, 0xF2, 0x8B, 0xD8, 0x8B, 0x45, 0x08, 0xE8};
   


    if(edit_handle_key.search(module, module_size, result))
    {
        base_handle_offset_ = result.front();
        sendmsg_call_param_offset = *(uint8_t*)(base_handle_offset_ + 4);
        base_handle_offset_ = *(uint32_t*)(base_handle_offset_ - 4);
    }

    result.clear();
    if(send_msg_call_key.search(module, module_size, result))
    {
        send_msg_call_addr_ = (uint32_t)result.front();
        send_msg_call_addr_ -= 8;
    }

    result.clear();
    if(messagebox_call_key.search(module, module_size, result))
    {
        messagebox_call_addr_ = (uint32_t)result.front();
    }

    result.clear();
    if(public_call_key.search(module, module_size, result))
    {
        public_call_addr_ = (uint32_t)result.front();
    }
    else if(public_call_key1.search(module, module_size, result))
    {
        gee_x64 = true;
        public_call_addr_ = (uint32_t)result.front();
    }

    result.clear();
    if(exit_game_call_key.search(module, module_size, result))
    {
        exit_game_call_addr_ = result.front() + 0x19;
    }

    VMP_VIRTUALIZATION_END();
}

GameLocalFuntion::~GameLocalFuntion()
{

}

uint32_t count = 1;
bool GameLocalFuntion::hook_init()
{
    LightHook::HookMgr& hookMgr = LightHook::HookMgr::instance();
    
#ifdef _RTEST
	LOG_ERROR("public_call_addr:%08X", public_call_addr_);
	LOG_ERROR("exit_game_call_addr:%08X", exit_game_call_addr_);
#endif

	// 公共功能函数HOOK
	if (public_call_addr_)
	{
		hookMgr.add_context_hook((uint32_t*)public_call_addr_, [](LightHook::Context& ctx) {
			game_lazy_action(ctx);
		}, NULL);
	}

	if (exit_game_call_addr_)
	{
		// 大退延时
		hookMgr.add_context_hook((uint32_t*)exit_game_call_addr_, [](LightHook::Context& ctx) {
			exit_game_lazy(ctx);
		}, NULL);
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
    if(base_handle_offset_ && messagebox_call_addr_)
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

void GameLocalFuntion::sendmsg_call(const strings msg, uint32_t bgcolor, uint32_t font_color)
{
    VMP_VIRTUALIZATION_BEGIN();
    if(!send_msg_call_addr_)
    {
        return;
    }
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

    VMP_VIRTUALIZATION_END();
}

void GameLocalFuntion::back_game_msg_countdown(BOOL is_back_game)
{
    DWORD top = -1;
    std::vector<strings> welcome_msg_vector = {
        {top, 27, "审判封挂提示游戏即将退出10"},
        {top, 27, "审判封挂提示游戏即将退出09"},
        {top, 27, "审判封挂提示游戏即将退出08"},
        {top, 27, "审判封挂提示游戏即将退出07"},
        {top, 27, "审判封挂提示游戏即将退出06"},
        {top, 27, "审判封挂提示游戏即将退出05"},
        {top, 27, "审判封挂提示游戏即将退出04"},
        {top, 27, "审判封挂提示游戏即将退出03"},
        {top, 27, "审判封挂提示游戏即将退出02"},
        {top, 27, "审判封挂提示游戏即将退出01"}
    };
    auto back_lazy_time = is_back_game ? GameLocalFuntion::instance().back_game_lazy_time_ : GameLocalFuntion::instance().exit_game_lazy_time_;
    for(int i = 0; i < back_lazy_time; i++)
    {
        auto welcomemsg = welcome_msg_vector[10 - back_lazy_time + i];
        GameLocalFuntion::instance().sendmsg_call(welcomemsg);
        Sleep(1015);
    }
}

std::string GameLocalFuntion::encode_buffer_call(std::string& string_to_encode)
{
    if(string_to_encode.empty()) return "";

    auto encode_str = encode_buffer_call((unsigned char const*)string_to_encode.data(), string_to_encode.length());

    return std::move(encode_str.data());
}

std::vector<char> GameLocalFuntion::encode_buffer_call(unsigned char const* bytes_to_encode, unsigned int in_len)
{
    std::vector<char> buf(in_len * 2, 0);
    PCHAR pbuf = buf.data();
    __asm {
        pushad
        pushfd
        mov ecx, in_len
        shl ecx, 1
        push ecx
        mov eax, bytes_to_encode
        mov ecx, in_len
        mov edx, pbuf
        mov esi, 0x4aa3e8
        call esi
        popfd
        popad
    }
    return buf;
}

std::string GameLocalFuntion::decode_buffer_call(std::string& string_to_decode)
{
    if(string_to_decode.empty()) return "";

    auto decode_str = decode_buffer_call((unsigned char const*)string_to_decode.data(), string_to_decode.length());

    return std::move(decode_str.data());
}

std::vector<char> GameLocalFuntion::decode_buffer_call(unsigned char const* bytes_to_decode, unsigned int in_len)
{
    std::vector<char> buf(in_len, 0);
    PCHAR pbuf = buf.data();
    __asm {
        pushad
        pushfd
        push in_len
        mov eax, bytes_to_decode
        mov ecx, in_len
        mov edx, pbuf
        mov esi, 0x4aa4a0
        call esi
        popfd
        popad
    }
    return buf;
}

void GameLocalFuntion::back_game_call(void* param)
{
    LightHook::Context ctx = *(LightHook::Context*)param;
    back_game_msg_countdown(true);
    if(gee_x64)
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

    hook_init();
}

void GameLocalFuntion::game_lazy_action(LightHook::Context& ctx)
{
    // 功能号
    switch(ctx.edx)
    {
        //小退
        case 0x3F1:
            back_game_lazy(ctx);
            break;
        default:
            break;
    }
}

void GameLocalFuntion::back_game_lazy(LightHook::Context& ctx)
{
    if(GameLocalFuntion::instance().back_game_lazy_enable_)
    {
        LightHook::HookMgr::instance().restore();
        memset(&context, 0, sizeof(LightHook::Context));
        context = ctx; 
        ctx.edx = -1;
        _beginthread(&back_game_call, NULL, &context);
    }
}

void GameLocalFuntion::exit_game_call(void* param)
{
    back_game_msg_countdown(FALSE);
    Utils::CWindows::instance().exit_process();
}

// 大退不能放在公共功能函数里,他是在功能函数外面执行的退出
void GameLocalFuntion::exit_game_lazy(LightHook::Context& ctx)
{
    //弹窗:你想退出游戏吗 ? 1.yes 2.no
    if(ctx.eax == 2)
    {
        return;
    }
    if(GameLocalFuntion::instance().exit_game_lazy_enable_)
    {
        ctx.eax = 2;
        _beginthread(&exit_game_call, NULL, NULL);
    }
}

int GameLocalFuntion::guess_encode_bound(int size)
{
    return (4 * size + 2) / 3;
}

int GameLocalFuntion::sub_ABACC8(int a1)
{
    long double v1;
    v1 = (long double)(3 * a1) / 4.0;
    return trunc(*(double *)&v1);
}

// 游戏内部通用加密
int GameLocalFuntion::encode_buffer_sub(unsigned char const*src_to_encode, BYTE *dest_to_encode, int src_len, int desc_len)
{
    char rest, rest_count, made;
    int dest_pos, encode_len;

    encode_len = src_len;
    if(desc_len < guess_encode_bound(src_len))
        encode_len = sub_ABACC8(desc_len);
    rest = 0;
    rest_count = 0;
    if(encode_len - 1 >= 0)
    {
        dest_pos = encode_len;
        do
        {
            made = ((*src_to_encode >> (rest_count + 2)) | rest) & 0x3F;
            rest = ((unsigned int)(*src_to_encode << (8 - (rest_count + 2))) >> 2) & 0x3F;
            rest_count += 2;
            *dest_to_encode++ = made + 60;
            if(rest_count == 6)
            {
                *dest_to_encode++ = rest + 60;
                rest = 0;
                rest_count = 0;
            }
            ++src_to_encode;
            --dest_pos;
        }
        while(dest_pos);
    }

    if(rest_count)
    {
        *dest_to_encode = rest + 60;
    }

    return guess_encode_bound(encode_len);
}

// 游戏内部通用解密
int GameLocalFuntion::decode_buffer_sub(BYTE *src_to_decode, BYTE *dest_to_decode, int src_len, int desc_len)
{
    char rest;
    char rest_count;
    int dest_pos;
    unsigned __int8 made;
    int bytes_to_encode;
    int decode_len;

    decode_len = src_len;
    bytes_to_encode = 0;
    if(sub_ABACC8(src_len) > desc_len)
        decode_len = guess_encode_bound(desc_len);
    rest = 0;
    rest_count = 0;
    if(decode_len - 1 < 0)
        return sub_ABACC8(decode_len);
    dest_pos = decode_len;
    while(*src_to_decode >= 0x3Cu && *src_to_decode < 0x7Cu)
    {
        made = *src_to_decode - 60;
        if(rest_count)
        {
            *dest_to_decode++ = rest | (made >> (rest_count - 2));
            rest = made << (8 - (rest_count - 2));
            rest_count -= 2;
        }
        else
        {
            rest_count = 6;
            rest = 4 * made;
        }
        ++src_to_decode;
        if(!--dest_pos)
            return sub_ABACC8(decode_len);
    }
    return bytes_to_encode;
}

// 命令行参数1二次解密
// 原加密串 JR]ULbaEO`uqQS`r
// 一次解密 9hYBiINNuUy6
// 二次解密 F6 16 01 88 83 4D B9 4C BA
int GameLocalFuntion::decode_buffer_sub2(BYTE *src_to_decode, BYTE *dest_to_decode, int src_len)
{
    int v3;
    int v4;
    int result;
    int v6;
    int v7 = 0;
    char *v8;
    unsigned int v9;
    unsigned int v10;
    unsigned int v11;
    unsigned int v12;
    bool v13;
    unsigned int v14;
    char v15[4] = {0};
    unsigned __int8 v16 = 0;
    unsigned __int8 v17 = 0;
    char v18 = 0;

    v3 = 0;
    v4 = 0;
    result = 0;
    v6 = src_len / 4;
    if(src_len / 4 > 0)
    {
        do
        {
            v7 = 4;
            v8 = v15;
            do
            {
                v9 = *(unsigned __int8 *)(src_to_decode + v3);
                if(v9 > '=')
                {
                    v12 = v9 - 'A';
                    v13 = v12 < 0x1A;
                    v14 = v12 - 0x1A;
                    if(v13)
                    {
                        *v8 = *(BYTE *)(src_to_decode + v3) - 'A';
                    }
                    else if(v14 - 6 < 0x1A)
                    {
                        *v8 = *(BYTE *)(src_to_decode + v3) - 'a' + 0x1A;
                    }
                }
                else if(v9 == '=')
                {
                    *v8 = -1;
                }
                else
                {
                    v10 = v9 - '+';
                    if(v10)
                    {
                        v11 = v10 - 4;
                        if(v11)
                        {
                            if(v11 - 1 < '\n')
                                *v8 = *(BYTE *)(src_to_decode + v3) - '0' + 52;
                        }
                        else
                        {
                            *v8 = '?';
                        }
                    }
                    else
                    {
                        *v8 = '>';
                    }
                }
                ++v3;
                ++v8;
                --v7;
            }
            while(v7);

            v16 = v15[1];
            v17 = v15[2];
            v18 = v15[3];

            *(BYTE *)(dest_to_decode + v4) = (v16 >> 4) | (4 * v15[0]);
            result = v4 + 1;
            if(v17 == 0xFF || v18 != -1)
            {
                if(v17 != 0xFF)
                {
                    *(BYTE *)(dest_to_decode + v4 + 1) = (v17 >> 2) | (16 * v16);
                    *(BYTE *)(dest_to_decode + v4 + 2) = v18 | (v17 << 6);
                    result = v4 + 3;
                    v4 += 2;
                }
            }
            else
            {
                *(BYTE *)(dest_to_decode + v4 + 1) = (v17 >> 2) | (16 * v16);
                result = v4 + 2;
                ++v4;
            }
            ++v4;
            --v6;
        }
        while(v6);
    }
    return result;
}