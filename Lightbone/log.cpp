#include "pch.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <filesystem>
#include <fstream>
#include <time.h>
#include <mutex>

#pragma warning(disable:4996)

namespace Utils
{
    void log_to_file(const std::string& identify, Enum type, const char* format, ...)
    {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lck(mtx);
        std::time_t now_time = time(0);
        char date_str[MAX_PATH] = { 0 };
        char time_str[MAX_PATH] = { 0 };
        auto local_time = std::localtime(&now_time);
        strftime(date_str, sizeof(date_str) - 1, "%Y-%m-%d", local_time);
        strftime(time_str, sizeof(time_str) - 1, "%I:%M:%S", local_time);

        std::filesystem::path file(std::filesystem::current_path() / "log");
        if(!std::filesystem::exists(file))
        {
            std::filesystem::create_directory(file);
        }
        file = file / date_str;

        if(!std::filesystem::exists(file))
        {
            std::filesystem::create_directory(file);
        }

        file = file / identify;

        std::ofstream output(file, std::ios::out | std::ios::app);
        char buffer[1024];
        std::string result;
        va_list ap;
        va_start(ap, format);
        _vsnprintf_s(buffer, sizeof(buffer) - 1, format, ap);
        va_end(ap);
        result = "";
        switch(type)
        {
        case Utils::CHANNEL_WARNING:
            result = result + "[WARNING]" + time_str + "|";
            break;
        case Utils::CHANNEL_ERROR:
            result = result + "[ERROR]" + time_str + "|";
            break;
        case Utils::CHANNEL_EVENT:
            result = result + "[EVENT]" + time_str + "|";
            break;
        default:
            break;
        }
        result = result + buffer + "\n";
        output << result;
        output.close();
    }

    void log(Enum type, const char *format, ...)
    {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lck(mtx);
        std::time_t now_time = time(0);
        char date_str[MAX_PATH] = { 0 };
        char time_str[MAX_PATH] = { 0 };
        auto local_time = std::localtime(&now_time);
        strftime(time_str, sizeof(time_str) - 1, "%I:%M:%S", local_time);

        char buffer[1024];
        std::string result;
        va_list ap;
        va_start(ap, format);
        _vsnprintf_s(buffer, sizeof(buffer) - 1, format, ap);
        va_end(ap);
        result = "";
        switch (type)
        {
        case Utils::CHANNEL_WARNING:
            result = result + "[WARNING]" + time_str + "|";
            break;
        case Utils::CHANNEL_ERROR:
            result = result + "[ERROR]" + time_str + "|";
            break;
        case Utils::CHANNEL_EVENT:
            result = result + "[EVENT]" + time_str + "|";
            break;
        default:
            break;
        }
        result = result + buffer + "\n";
        ::OutputDebugStringA(result.c_str());
    }
}
