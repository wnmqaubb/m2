#include "pch.h"
#include "FileWatcher.h"
#include <functional>
#include <thread>
#include "lf_plug_sdk.h"

namespace Utils {
inline VOID DbgPrint(const char* fmt, ...)
{
    char    buf[1024];
    va_list args;
    va_start(args, fmt);
    vsprintf_s(buf, fmt, args);
    va_end(args);
    AddShowLog(buf, 0);
}

std::string load_file(fs::path path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    assert(file.is_open());
    file.seekg(0, file.end);
    size_t sz = file.tellg();
    file.seekg(0);
    std::string buffer;
    buffer.resize(sz);
    file.read(buffer.data(), sz);
    file.close();
    return buffer;
}
void PrintFunctionBytes(void* funcPtr, size_t bytesToRead) {
    // ����ڴ�ɶ���
    MEMORY_BASIC_INFORMATION mbi;
    //if (!VirtualQuery(funcPtr, &mbi, sizeof(mbi)) ||
    //    !(mbi.Protect & (PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) {
    //    Utils::DbgPrint("�����޷���ȡ�ڴ�");
    //    return;
    //}

    // ��ȫ��ȡ�ֽ���
    std::vector<uint8_t> buffer(bytesToRead);
    memcpy(buffer.data(), funcPtr, bytesToRead);

    //Utils::DbgPrint("%p ���ֽ��룺", funcPtr);
    //for (size_t i = 0; i < buffer.size(); ++i) {
    //    Utils::DbgPrint("%02X ", buffer[i]);
    //    if ((i + 1) % 8 == 0) Utils::DbgPrint("");
    //}
// ����ȡ���ֽ�д��������ļ�
    std::ofstream ofs("bytes.bin", std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    ofs.close();
}

BOOL WriteStructToFile(const TRunGatePlugClientInfo& data, const char* filename) {
    // 1. ���ļ���������ģʽ��
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        Utils::DbgPrint("�޷����ļ�: %s", filename);
        return false;
    }

    // 2. ֱ��д��ṹ��ԭʼ�ֽ�
    ofs.write(reinterpret_cast<const char*>(&data), sizeof(TRunGatePlugClientInfo));

    // 3. ���д���Ƿ�����
    const bool success = ofs.good();
    ofs.close();

    if (success) {
        Utils::DbgPrint("�ɹ�д�� %zu �ֽڵ� %s", sizeof(TRunGatePlugClientInfo), filename);
    }
    else {
        Utils::DbgPrint("�ļ�д��ʧ��");
    }

    return success;
}

void PrintClientInfo(const TRunGatePlugClientInfo& info) {

    Utils::DbgPrint("RecogId: 0x%016I64x MoveSpeed: %d AttackSpeed: %d SpellSpeed: %d", info.RecogId, info.MoveSpeed, info.AttackSpeed, info.SpellSpeed);  // Windowsר��I64

    // ��ȫ�����ַ�����ȷ��null��ֹ��
    //char account[sizeof(info.Account) + 1] = { 0 };
    //char chrName[sizeof(info.ChrName) + 1] = { 0 };
    //char ipAddr[sizeof(info.IpAddr) + 1] = { 0 };
    //char macID[sizeof(info.MacID) + 1] = { 0 };

    //memcpy(account, info.Account, sizeof(info.Account));
    //memcpy(chrName, info.ChrName, sizeof(info.ChrName));
    //memcpy(ipAddr, info.IpAddr, sizeof(info.IpAddr));
    //memcpy(macID, info.MacID, sizeof(info.MacID));

    //Utils::DbgPrint("  Account: \"%s\"", account);
    //Utils::DbgPrint("  ChrName: \"%s\"", chrName);
    //Utils::DbgPrint("  IPValue: %d (0x%08x)", info.IPValue, info.IPValue);
    //Utils::DbgPrint("  IpAddr: \"%s\"", ipAddr);
    //Utils::DbgPrint("  Port: %d", info.Port);
    //Utils::DbgPrint("  MacID: \"%s\"", macID);

    // ����ֵ��ӡ
    Utils::DbgPrint("  IsActive: %d IsLoginNotice: %d IsPlayGame: %d", info.IsActive, info.IsLoginNotice, info.IsPlayGame);
    // ָ��ͳ���
    Utils::DbgPrint("  DataAdd: 0x%p DataLen: %u", info.DataAdd, info.DataLen);  // Windows��ָ����%p

    // �����ֶΣ�ʮ�����ƴ�ӡ��
    Utils::DbgPrint("  Reseved2: [");
    for (int i = 0; i < 5; ++i) {
        if (i > 0) Utils::DbgPrint(", ");
        Utils::DbgPrint("0x%08X", info.Reseved2[i]);  // 32λʮ������
    }
    Utils::DbgPrint("]");
}

std::unordered_map<std::string, std::unordered_map<std::string, std::string>> readIniFile(const std::filesystem::path& path) {
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> iniData;
    std::ifstream file(path);
    if (file.is_open()) {
        std::string section;
        std::string line;
        while (std::getline(file, line)) {
            // ȥ������β�Ŀհ��ַ�
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
            if (line.empty() || line[0] == ';') continue;
            if (line[0] == '[' && line.back() == ']') {
                section = line.substr(1, line.length() - 2);
            }
            else {
                auto pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    iniData[section][key] = value;
                }
            }
        }
        file.close();
    }
    else {
        AddShowLog("=====�޷���Config.ini�ļ�=====", 0);
    }
    return iniData;
}

std::string read_config_txt(const std::filesystem::path& path, const std::string& section, const std::string& key) {
    setlocale(LC_CTYPE, "");
    auto iniData = readIniFile(path);
    if (iniData.find(section) != iniData.end() && iniData[section].find(key) != iniData[section].end()) {
        return iniData[section][key];
    }
    else {
        return "";
    }
}
};
void FileWatcher::start() {
    if (running_) return;

    running_ = true;
    watcher_thread_ = std::thread([this]() {
        std::error_code ec;
        auto last_write_time = fs::last_write_time(filepath_, ec);

        if (ec) {
            AddShowLog("=====��ʼ������=====", 0);
            return;
        }

        while (running_) {
            std::this_thread::sleep_for(interval_);

            if (!fs::exists(filepath_)) {
                continue;
            }

            auto current_write_time = fs::last_write_time(filepath_, ec);
            if (ec) {
                ec.clear();
                continue;
            }

            if (current_write_time != last_write_time) {
                last_write_time = current_write_time;
                callback_();
            }
        }
    });
}

void FileWatcher::stop() {
    running_ = false;
    if (watcher_thread_.joinable()) {
        watcher_thread_.join();
    }
}