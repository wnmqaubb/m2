#pragma once
#include <atomic>
#include <filesystem>
#include <thread>
#include <fstream>
#include <cassert>
#include "lf_plug_sdk.h"
#include <unordered_map>
#include <WinDef.h>

#ifdef _DEBUG
#define VMP_VIRTUALIZATION_BEGIN()
#define VMP_VIRTUALIZATION_END()
#else
#include "../../yk/3rdparty/vmprotect/VMProtectSDK.h"
#define VMP_VIRTUALIZATION_BEGIN() VMProtectBeginVirtualization("");
#define VMP_VIRTUALIZATION_END() VMProtectEnd();
#endif
namespace fs = std::filesystem;
using namespace lfengine::rungate;
namespace Utils {
    VOID DbgPrint(const char* fmt, ...);

    std::string load_file(fs::path path);

    void PrintFunctionBytes(void* funcPtr, size_t bytesToRead = 32);

    BOOL WriteStructToFile(const TRunGatePlugClientInfo& data, const char* filename);

    void PrintClientInfo(const TRunGatePlugClientInfo& info);

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> readIniFile(const std::filesystem::path& path);

    std::string read_config_txt(const std::filesystem::path& path, const std::string& section, const std::string& key);
};

// DLL文件更新监控
class FileWatcher {
public:
    FileWatcher(const fs::path& filepath,
                std::function<void()> callback,
                std::chrono::milliseconds interval = std::chrono::seconds(5))
        : filepath_(filepath),
        callback_(callback),
        interval_(interval),
        running_(false) {
    }

    ~FileWatcher() {
        stop();
    }

    void start();

    void stop();

private:
    fs::path filepath_;
    std::function<void()> callback_;
    std::chrono::milliseconds interval_;
    std::thread watcher_thread_;
    std::atomic<bool> running_;
};