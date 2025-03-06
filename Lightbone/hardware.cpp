#include "pch.h"
#include "utils.h"
#include <WinNT.h>
#include <WinBase.h>
#include "api_resolver.h"
#include <stddef.h>
#include <Nb30.h>
#include <intrin.h>
#include <array>
#include "singleton.hpp"
#include <setupapi.h>
#include <devguid.h>
#include <regex>
#include <tchar.h>
#include <sstream>

#pragma comment(lib,"setupapi.lib")
#pragma comment(lib,"netapi32.lib")  
#define NAME_SIZE 128
const GUID GUID_CLASS_MONITOR = { 0x4d36e96e, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 };

namespace Utils {
    using namespace String;
	namespace HardwareInfo {
		std::wstring get_cpuid() {
            std::array<int, 4> cpui;
            __cpuid(cpui.data(), 0x0);
            int nIds_ = cpui[0];
            std::vector<std::array<int, 4>> data_;
            for (int i = 0; i <= nIds_; ++i) {
                __cpuidex(cpui.data(), i, 0);
                data_.push_back(cpui);
            }
            char vendor[0x20] = { 0 };
            *reinterpret_cast<int*>(vendor) = data_[0][1];
            *reinterpret_cast<int*>(vendor + 4) = data_[0][3];
            *reinterpret_cast<int*>(vendor + 8) = data_[0][2];
            std::string vendor_ = vendor;
            bool isIntel_ = false;
            bool isAMD = false;
            if ("GenuineIntel" == vendor_) {
                isIntel_ = true;    //厂商为INTEL
            }
            else if ("AuthenticAMD" == vendor_) {
                isAMD = true;       //厂商为AMD
            }
            char vendor_serialnumber[0x14] = { 0 };
            sprintf_s(vendor_serialnumber, sizeof(vendor_serialnumber), "%08X%08X", data_[1][3], data_[1][0]);		
            return c2w(vendor_serialnumber);
		}

		std::wstring get_mac_address() {
            char mac[100] = { 0 };
            NCB ncb;
            typedef struct _ASTAT_ {
                ADAPTER_STATUS   adapt;
                NAME_BUFFER   NameBuff[30];
            }ASTAT, *PASTAT;

            ASTAT Adapter;

            typedef struct _LANA_ENUM {
                UCHAR   length;
                UCHAR   lana[MAX_LANA];
            }LANA_ENUM;

            LANA_ENUM lana_enum;
            UCHAR uRetCode;
            memset(&ncb, 0, sizeof(ncb));
            memset(&lana_enum, 0, sizeof(lana_enum));
            ncb.ncb_command = NCBENUM;
            ncb.ncb_buffer = (unsigned char *)&lana_enum;
            ncb.ncb_length = sizeof(LANA_ENUM);
            uRetCode = Netbios(&ncb);

            if (uRetCode != NRC_GOODRET)
                return L"";

            for (int lana = 0; lana < lana_enum.length; lana++) {
                ncb.ncb_command = NCBRESET;
                ncb.ncb_lana_num = lana_enum.lana[lana];
                uRetCode = Netbios(&ncb);
                if (uRetCode == NRC_GOODRET)
                    break;
            }

            if (uRetCode != NRC_GOODRET)
                return L"";

            memset(&ncb, 0, sizeof(ncb));
            ncb.ncb_command = NCBASTAT;
            ncb.ncb_lana_num = lana_enum.lana[0];

            UCHAR ncb_callname[NCBNAMSZ];

            strcpy_s((char*)ncb.ncb_callname, sizeof(ncb_callname), "*");
            ncb.ncb_buffer = (unsigned char *)&Adapter;
            ncb.ncb_length = sizeof(Adapter);
            uRetCode = Netbios(&ncb);

            if (uRetCode != NRC_GOODRET)
                return L"";

            sprintf_s(mac, 100, "%02X-%02X-%02X-%02X-%02X-%02X",
                Adapter.adapt.adapter_address[0],
                Adapter.adapt.adapter_address[1],
                Adapter.adapt.adapter_address[2],
                Adapter.adapt.adapter_address[3],
                Adapter.adapt.adapter_address[4],
                Adapter.adapt.adapter_address[5]);

            return c2w(std::string(mac));
		}

		std::wstring get_volume_serial_number() {
            DWORD  VolumeSerialNumber;
            TCHAR  VolumeName[256];
            GetVolumeInformation(NULL, VolumeName, 12, &VolumeSerialNumber, NULL, NULL, NULL, 10);
            return std::to_wstring((uint32_t)VolumeSerialNumber);
		}

		uint32_t get_all_device_ids_hash() {
            HDEVINFO hDevInfo;
            SP_DEVINFO_DATA DeviceInfoData = { sizeof(DeviceInfoData) };
            uint32_t device_id = 0;
            hDevInfo = SetupDiGetClassDevs(NULL, 0, 0, DIGCF_ALLCLASSES | DIGCF_PRESENT);
            if (hDevInfo == INVALID_HANDLE_VALUE)
            {
                return device_id;
            }

            std::vector<int> device_ids;
            for (int i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)
            {
                DWORD DataT;
                const DWORD buffersize = 2046;
                char friendly_name[buffersize] = { 0 };
                DWORD req_bufsize = 0;

                if (!SetupDiGetDeviceRegistryPropertyA(hDevInfo, &DeviceInfoData, SPDRP_DEVICEDESC, &DataT, (LPBYTE)friendly_name, buffersize, &req_bufsize))
                {
                    continue;
                }

                char wDeviceInstanceId[MAX_PATH] = { 0 };

                if (!SetupDiGetDeviceInstanceIdA(hDevInfo, &DeviceInfoData, wDeviceInstanceId, MAX_PATH, 0))
                {
                    continue;
                }

                std::string text = wDeviceInstanceId;
                if (std::regex_search(text, std::regex("USB|AUDIO|PNP")))
                    continue;
                if (std::regex_search(text, std::regex("PID|VID|VEN|DEV")))
                {
                    std::string pattern = "[_&](\\d{4})[^0-9]";
                    std::regex express(pattern);
                    std::string text = wDeviceInstanceId;
                    std::regex_token_iterator<std::string::const_iterator> begin2_1(text.cbegin(), text.cend(), express, 1);
                    for (auto iter = begin2_1; iter != std::sregex_token_iterator(); iter++)
                    {
                        device_ids.push_back(atoi(iter->str().c_str()));
                    }
                }
            }

            uint16_t width, height;
            uint32_t serial_number;
            if (get_monitor_info(width, height, serial_number))
            {
                device_ids.push_back(width);
                device_ids.push_back(height);
                device_ids.push_back(serial_number);
            }

            std::string ids;
            std::sort(device_ids.begin(), device_ids.end());
            for (int id : device_ids)
            {
                ids += id;
            }
            device_id = Utils::Crypto::aphash((unsigned char *)ids.data(), ids.size());
            return device_id;
		}

        bool get_monitor_info_from_edid(const HKEY dev_reg_key, uint16_t& width, uint16_t& hieght, uint32_t &serial_number)
        {
            DWORD type, acutal_value_name_length = NAME_SIZE;
            TCHAR value_name[NAME_SIZE];

            BYTE edid_data[1024];
            DWORD edid_size = sizeof(edid_data);
            for (LONG i = 0, retValue = ERROR_SUCCESS; retValue != ERROR_NO_MORE_ITEMS; ++i)
            {
                retValue = RegEnumValue(dev_reg_key, i, &value_name[0],
                    &acutal_value_name_length, NULL, &type,
                    edid_data,
                    &edid_size);

                if (retValue != ERROR_SUCCESS || 0 != _tcscmp(value_name, TEXT("EDID")))
                    continue;

                width = ((edid_data[68] & 0xF0) << 4) + edid_data[66];
                hieght = ((edid_data[68] & 0x0F) << 8) + edid_data[67];
                serial_number = *(uint32_t*)&edid_data[0xc];
                return true;
            }
            return false;
        }

        bool get_size_for_devid(const std::wstring& target_devid, uint16_t& width, uint16_t& height, uint32_t &serial_number)
        {
            HDEVINFO dev_info = SetupDiGetClassDevsEx(
                &GUID_CLASS_MONITOR,
                NULL,
                NULL,
                DIGCF_PRESENT,
                NULL,
                NULL,
                NULL);
            if (NULL == dev_info)
                return false;
            bool result = false;
            for (ULONG i = 0; ERROR_NO_MORE_ITEMS != GetLastError(); ++i)
            {
                SP_DEVINFO_DATA dev_info_data;
                memset(&dev_info_data, 0, sizeof(dev_info_data));
                dev_info_data.cbSize = sizeof(dev_info_data);
                if (SetupDiEnumDeviceInfo(dev_info, i, &dev_info_data))
                {
                    HKEY dev_reg_key = SetupDiOpenDevRegKey(dev_info, &dev_info_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
                    if (!dev_reg_key || (dev_reg_key == INVALID_HANDLE_VALUE))
                        continue;
                    result = get_monitor_info_from_edid(dev_reg_key, width, height, serial_number);
                    RegCloseKey(dev_reg_key);
                }
            }
            SetupDiDestroyDeviceInfoList(dev_info);
            return result;
        }

        bool get_monitor_info(uint16_t& width, uint16_t& height, uint32_t &serial_number)
        {
            DISPLAY_DEVICE dd;
            dd.cb = sizeof(dd);
            uint32_t dev = 0;
            std::wstring device_id;
            bool found_device = false;
            while (EnumDisplayDevices(0, dev, &dd, 0) && !found_device)
            {
                DISPLAY_DEVICE display_devicew;
                ZeroMemory(&display_devicew, sizeof(display_devicew));
                display_devicew.cb = sizeof(display_devicew);
                uint32_t dev_count = 0;

                while (EnumDisplayDevices(dd.DeviceName, dev_count, &display_devicew, 0) && !found_device)
                {
                    if (display_devicew.StateFlags & DISPLAY_DEVICE_ACTIVE &&
                        !(display_devicew.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
                    {
                        device_id = display_devicew.DeviceID;
                        size_t position = device_id.find(L"\\", 9);
                        if (position != std::wstring::npos)
                        {
                            device_id = device_id.substr(8, position - 8);
                            found_device = get_size_for_devid(device_id, width, height, serial_number);
                        }
                    }
                    dev_count++;

                    ZeroMemory(&display_devicew, sizeof(display_devicew));
                    display_devicew.cb = sizeof(display_devicew);
                    if (found_device)
                    {
                        break;
                    }
                }

                ZeroMemory(&dd, sizeof(dd));
                dd.cb = sizeof(dd);
                dev++;

                if (found_device)
                {
                    break;
                }
            }
            return found_device;
        }

        std::vector<std::string> get_bcdinfo_by_keys(const std::string& output, const std::vector<std::string>& target_list) {
            std::vector<std::string> result;
            std::istringstream iss(output);
            std::string line;
            const std::string timeout = "timeout";
            //const std::string target = "description"; // 目标字段
            bool is_timeout = false;
            while (std::getline(iss, line)) {
                // 去除行首尾的空白字符
                size_t start = line.find_first_not_of(" \t\n\r");
                if (start == std::string::npos) continue;
                size_t end = line.find_last_not_of(" \t\n\r");
                std::string trimmed = line.substr(start, end - start + 1);
                // 跳过第一项
                if (!is_timeout && trimmed.find(timeout) == 0) {
                    is_timeout = true;
                    continue;
                }

                if (is_timeout)
                {
                    for (const auto& target : target_list) {

                        // 检查是否以 "description" 开头
                        if (trimmed.find(target) != 0) continue; // 不是以目标字段开头则跳过

                        // 提取值部分
                        size_t valueStart = target.length();
                        // 跳过冒号、空格等分隔符
                        valueStart = trimmed.find_first_not_of(": \t", valueStart);
                        if (valueStart == std::string::npos) continue;

                        std::string value = trimmed.substr(valueStart);
                        result.push_back(value);
                    }
                }
            }
            return std::move(result);
        }

        std::vector<std::string> GetBcdInfo() {
            // vista 以下不支持        
            if (Utils::CWindows::instance().get_system_version() < Utils::CWindows::SystemVersion::WINDOWS_VISTA) {
                return {};
            }
            BOOL isWow64 = FALSE;
            PVOID oldRedirection = nullptr;

            std::vector<std::string> descriptions;
            // 检查是否运行在64位系统的WOW64模式下
            if (IsWow64Process(GetCurrentProcess(), &isWow64) && isWow64) {
                // 动态加载函数（避免在32位系统上直接链接）
                auto Wow64DisableWow64FsRedirection = IMPORT(L"kernel32.dll", Wow64DisableWow64FsRedirection);
                auto Wow64RevertWow64FsRedirection = IMPORT(L"kernel32.dll", Wow64RevertWow64FsRedirection);

                if (Wow64DisableWow64FsRedirection && Wow64RevertWow64FsRedirection) {
                    Wow64DisableWow64FsRedirection(&oldRedirection); // 禁用重定向
                }
            }                

            SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE };
            HANDLE hReadPipe, hWritePipe;

            if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
                if (oldRedirection) {
                    auto Wow64RevertWow64FsRedirection = IMPORT(L"kernel32.dll", Wow64RevertWow64FsRedirection);
                    if (Wow64RevertWow64FsRedirection) Wow64RevertWow64FsRedirection(oldRedirection);
                }
                return descriptions;
            }

            // 获取标准输入、错误句柄（处理 GUI 程序无控制台的情况）
            HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
            HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
            HANDLE hStdInDup = INVALID_HANDLE_VALUE, hStdErrDup = INVALID_HANDLE_VALUE;

            // 仅复制有效句柄
            if (hStdIn != INVALID_HANDLE_VALUE) {
                DuplicateHandle(GetCurrentProcess(), hStdIn, GetCurrentProcess(), &hStdInDup, 0, TRUE, DUPLICATE_SAME_ACCESS);
            }
            if (hStdErr != INVALID_HANDLE_VALUE) {
                DuplicateHandle(GetCurrentProcess(), hStdErr, GetCurrentProcess(), &hStdErrDup, 0, TRUE, DUPLICATE_SAME_ACCESS);
            }

            STARTUPINFO si = { sizeof(STARTUPINFO) };
            si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
            si.hStdInput = hStdInDup;
            si.hStdOutput = hWritePipe;
            si.hStdError = (hStdErrDup != INVALID_HANDLE_VALUE) ? hStdErrDup : hWritePipe; // 若无错误句柄，重定向到输出管道
            si.wShowWindow = SW_HIDE;

            // 必须使用可修改的缓冲区
            wchar_t cmdLine[] = L"\"C:\\Windows\\System32\\bcdedit.exe\" /enum"; // 注意添加.exe

            PROCESS_INFORMATION pi = { 0 };
            BOOL bSuccess = CreateProcessW(
                nullptr,
                cmdLine,    // 命令行参数
                nullptr,
                nullptr,
                TRUE,       // 允许句柄继承
                CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,
                nullptr,
                nullptr,
                &si,
                &pi
            );

            // 立即关闭父进程不再需要的句柄
            CloseHandle(hWritePipe);
            if (hStdInDup != INVALID_HANDLE_VALUE) CloseHandle(hStdInDup);
            if (hStdErrDup != INVALID_HANDLE_VALUE) CloseHandle(hStdErrDup);

            if (!bSuccess) {
                // 恢复重定向
                if (oldRedirection) {
                    auto Wow64RevertWow64FsRedirection = IMPORT(L"kernel32.dll", Wow64RevertWow64FsRedirection);
                    if (Wow64RevertWow64FsRedirection) Wow64RevertWow64FsRedirection(oldRedirection);
                }
                //DWORD err = GetLastError();
                
                CloseHandle(hReadPipe);
                return descriptions;
            }

            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);


            // 读取输出
            char buffer[4096];
            DWORD bytesRead;
            std::string output;
            while (ReadFile(hReadPipe, buffer, sizeof(buffer), &bytesRead, nullptr) && bytesRead > 0) {
                output.append(buffer, bytesRead);
            }
            CloseHandle(hReadPipe);
            /*
                {current}
                Windows 11
                partition=C:
            */
            descriptions = get_bcdinfo_by_keys(output, { "description", "osdevice", "标识符" });

            // 恢复文件系统重定向
            if (oldRedirection) {
                auto Wow64RevertWow64FsRedirection = IMPORT(L"kernel32.dll", Wow64RevertWow64FsRedirection);
                if (Wow64RevertWow64FsRedirection) Wow64RevertWow64FsRedirection(oldRedirection);
            }
            return std::move(descriptions);
        }
	}
}