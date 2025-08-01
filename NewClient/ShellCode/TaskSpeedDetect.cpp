﻿#include "pch.h"
#include <../../yk/Lightbone/utils.h>
#include "Service/AntiCheatClient.h"
#include <MMSystem.h>
#include "ClientImpl.h"
extern std::shared_ptr<CClientImpl> client_;
extern std::shared_ptr<asio2::timer> g_timer;

void SpeedDetect()
{
    ProtocolC2STaskEcho resp;
    resp.task_id = TASK_PKG_ID_SPEED_DETECT;
    auto Sleep = IMPORT(L"kernel32.dll", Sleep);
    auto QueryPerformanceFrequency = IMPORT(L"kernel32.dll", QueryPerformanceFrequency);
    auto QueryPerformanceCounter = IMPORT(L"kernel32.dll", QueryPerformanceCounter);
    auto SetThreadAffinityMask = IMPORT(L"kernel32.dll", SetThreadAffinityMask);
    auto GetCurrentThread = IMPORT(L"kernel32.dll", GetCurrentThread);
    auto GetTickCount = IMPORT(L"kernel32.dll", GetTickCount);
    auto timeGetTime = IMPORT(L"winmm.dll", timeGetTime);
    std::vector<void*> api_check_list = { GetTickCount, QueryPerformanceCounter, timeGetTime, SetThreadAffinityMask, QueryPerformanceFrequency };
    for (auto api : api_check_list)
    {
        if (api)
        {
            if (*(uint8_t*)api == 0xE9)
            {
                resp.text = xorstr("检测到加速作弊");
                resp.is_cheat = true;
            }
        }
    }

    SetThreadAffinityMask(GetCurrentThread(), 1);
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    LARGE_INTEGER start, end;
    double seconds = 0.0L;
    int iterations = 0;
    for (int i = 0; i < 5; i++)
    {
        QueryPerformanceCounter(&start);
        Sleep(500);
        QueryPerformanceCounter(&end);

        seconds += double(end.QuadPart - start.QuadPart) / frequency.QuadPart;
        iterations++;
    }

    SetThreadAffinityMask(GetCurrentThread(), -1);
    seconds = seconds / iterations;
    if (seconds <= 0.5 * 0.99)
    {
        char buffer[255] = { 0 };
        snprintf(buffer, sizeof(buffer), xorstr("检测到驱动变速:%f"), seconds);
        resp.text = buffer;
        resp.is_cheat = true;
    }
    if (resp.is_cheat)
    {
        client_->send(&resp);
    }
};

const unsigned int DEFINE_TIMER_ID(kSpeedDetectTimerId);
void InitSpeedDetect()
{
	LOG(__FUNCTION__);
	g_timer->start_timer(kSpeedDetectTimerId, std::chrono::seconds(5), []() {
        __try {
            SpeedDetect();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            LOG("线程异常: %s|%s|%d|0x%X", __FILE__, __FUNCTION__, __LINE__, GetExceptionCode());
        }
    });
}