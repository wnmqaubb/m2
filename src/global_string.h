#pragma once
#include <string>
namespace GlobalString
{
    namespace JsonProperty
    {
        // PUBLIC
        static std::string SEPARATOR = "|";
        static std::string DATA = "data";
        // gmaer_json
        static std::string ID = "id";
        static std::string GAMER_USERNAME = "gamer_username";
        static std::wstring GAMER_USERNAME_TEST = L"测试__";
        static std::string CPUID = "cpuid";
        static std::string VOLUME_SERIAL_NUMBER = "volume_serial_number";
        static std::string MAC_ADDRESS = "mac_address";
        static std::string PACK_IP = "pack_ip";
        static std::string TIMESTAMP = "timestamp";
        static std::string LAST_RECV_TICK_COUNT = "last_recv_tick_count";
        // process_json
        static std::string SUBID = "subid";
        static std::string PID = "pid";
        static std::string PROCESSES = "processes";
        static std::string MODULES = "modules";
        static std::string THREADS = "threads";
        static std::string DIRS = "dirs";
        static std::string WINDOWS = "windows";
        // Policy
        static std::string POLICIES = "policies";
        static std::string RESULTS = "results";
        static std::string IS_CHEAT = "is_cheat";
        static std::string REASON = "reason";
        // SHELL
        static std::string STATUS = "status";
        static std::string UNCOMPRESS_SIZE = "uncompress_size";
        // SCREENSHOT
        static std::string SILENT = "silent";

        static std::string LAZY_TIME = "lazy_time";

        static std::wstring WSEPARATOR = L"|";
    }
    namespace ErrorMsg
    {
        static std::string REASON_EMSG = "打印错误原因:%s";
        static std::string UNPACK_EMSG = "解包失败";
        static std::string MISSID_EMSG = "找不到id字段";
    }
    namespace ANTICHEAT
    {
        static std::string SHELL_DIRECTORIES = "shell";
        static std::string FILE_EXTENSION_BIN = ".BIN";
        static std::string SHELL_README_TXT = "readme.txt";
    }
    namespace LOG
    {
        static std::string DATE_FORMAT = "%Y-%m-%d";
        static std::string DATETIME_FORMAT = "%Y-%m-%d %H:%M:%S";
        static std::string FORMAT_WARNING_S_S = "[WARNING]|%s|%s";
        static std::string FORMAT_ERROR_S_S = "[ERROR]|%s|%s";
        static std::string FORMAT_EVENT_S_S = "[EVENT]|%s|%s";
        static std::string FORMAT_WARNING_S = "[WARNING]%s";
        static std::string FORMAT_ERROR_S = "[ERROR]%s";
        static std::string FORMAT_EVENT_S = "[EVENT]%s";
        static std::string FORMAT_S = "%s";
        static std::string LOG_FILE = "log";
        static std::string LOG_DEFAULT_FILE = "default.txt";
        static std::string EXCEPTION_PACKAGE_ID = "exception|package_id:%d";
        static std::string UNKNOWN_PACKAGE_PACKAGE_ID = "unknown_package|package_id:%d";
        static std::string CLIENT_IP_CONN = "客户端IP:%ls:%u 连接";
        static std::string CLIENT_HANDSHAKE = "客户端IP:%ls:%u 握手";
        static std::string SHELLCODE_EXCEPTION_PACKAGE_ID = "shellcode_exception|package_id:%d";
        static std::wstring LOG_FILE_EXTENSION = L".txt";
        static std::wstring FORMAT_D_S_S = L"%d|%s|%s";
    }
}