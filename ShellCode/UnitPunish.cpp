#include "NewClient/pch.h"
#include "TaskBasic.h"

void __declspec(noinline) UnitPunishBsod(std::error_code ec)
{
    BasicUtils::manual_load_ntdll_and_bsod();
    BasicUtils::bsod();
    BasicUtils::infinite_exit();
    g_game_io.post([]() {
        GameLocalFuntion::instance().messagebox_call("������������ؾ��棺���𿪹ҽ�����Ϸ��������������Ŵ���");
    });
    std::this_thread::sleep_for(std::chrono::seconds(std::rand() % 5));
    g_game_io.post([]() {
        while (true)
        {
            Sleep(-1);
        }
    });
}


void __declspec(noinline) UnitPunishKick(std::error_code ec)
{
    g_game_io.post([](){
        std::this_thread::sleep_for(std::chrono::seconds(std::rand() % 5));
        auto GetModuleHandleA = IMPORT(L"kernel32.dll", GetModuleHandleA);
        char ntdll_name[] = { 'n', 't', 'd', 'l', 'l', '.', 'd', 'l', 'l' ,0 };
        Utils::ImageProtect::instance().unmap_image(GetModuleHandleA(ntdll_name));
    });
    GameLocalFuntion::instance().messagebox_call("�����������ʾ�����𿪹ҽ�����Ϸ��������������Ŵ���");
}