#include "NewClient/pch.h"
#include <Lightbone/utils.h>
#include "Service/AntiCheatClient.h"

__declspec(dllimport) HMODULE dll_base;

#pragma warning(disable:4996)
#define GET_IMAGE_DOS_HEADER(image) ((PIMAGE_DOS_HEADER)(image))
#define GET_IMAGE_NT_HEADER(image) ((PIMAGE_NT_HEADERS)((ULONG_PTR)(GET_IMAGE_DOS_HEADER(image)->e_lfanew) + (ULONG_PTR)(image)))
#define ALIGN_LEFT_SIZE(virtual_size,align_size) ((virtual_size%align_size)?(align_size - (virtual_size%align_size)):0)
#define ALIGN(virtual_size,align_size) (virtual_size+ALIGN_LEFT_SIZE(virtual_size,align_size))
#define GET_DATA_DIRECTORY(nt_header,dir) (nt_header->OptionalHeader.DataDirectory[dir])
#define GET_DATA_DIRECTORY_VA(image_base, nt_header, dir) RVA2VA(image_base, GET_DATA_DIRECTORY(nt_header, dir).VirtualAddress)
#define GET_RELOC_DESC_TYPEOFFSET_SIZE(reloc_desc) ((reloc_desc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(TYPEOFFSET))
#define RVA2VA(image_base,rva) ((BYTE*)image_base+rva)

class CFile
{
public:
    CFile()
    {
        file_handle_ = INVALID_HANDLE_VALUE;
        CreateFileA = IMPORT(L"kernel32.dll", CreateFileA);
        GetFileSize = IMPORT(L"kernel32.dll", GetFileSize);
        FlushFileBuffers = IMPORT(L"kernel32.dll", FlushFileBuffers);
        ReadFile = IMPORT(L"kernel32.dll", ReadFile);
        WriteFile = IMPORT(L"kernel32.dll", WriteFile);
        CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
        SetFilePointer = IMPORT(L"kernel32.dll", SetFilePointer);
    }

    ~CFile()
    {
        if (file_handle_ != INVALID_HANDLE_VALUE)
            CloseHandle(file_handle_);
    }

    bool create(const std::string& path)
    {
        file_handle_ = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (file_handle_ != INVALID_HANDLE_VALUE)
        {
            if (CloseHandle(file_handle_))
                return true;
            else
                return false;
        }
        else
        {
            return false;
        }
        return false;
    }

    bool open_file(const std::string& path, unsigned int mode)
    {
        DWORD access = 0;
        bool r = false;
        switch (mode)
        {
        case 'r':
        {
            access = GENERIC_READ;
            break;
        }
        case 'w':
        {
            access = GENERIC_WRITE;
            break;
        }
        default:
            return r;
        }
        file_handle_ = CreateFileA(path.c_str(), access, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        r = file_handle_ != INVALID_HANDLE_VALUE;
        if (r)
        {
            size_ = get_file_size();
        }
        return r;
    }
    std::size_t read(void* buffer, unsigned num_count)
    {
        if (file_handle_ == INVALID_HANDLE_VALUE || buffer == 0)
        {
            return false;
        }

        if (num_count == 0)
        {
            return false;
        }

        unsigned long bytes_of_read = 0;
        if (ReadFile(file_handle_, buffer, num_count, &bytes_of_read, 0))
        {
            return bytes_of_read;
        }
        else
        {
            return 0;
        }
    }

    bool write(const void* buffer, unsigned num_size)
    {
        if (file_handle_ == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        if (num_size == 0)
        {
            return false;
        }
        unsigned long bytes_of_write = 0;
        if (WriteFile(file_handle_, buffer, num_size, &bytes_of_write, 0))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    void flush()
    {
        if (file_handle_ == INVALID_HANDLE_VALUE)
            return;
        FlushFileBuffers(file_handle_);
    }

    long seek(unsigned off_set, unsigned from)
    {
        long pos = SetFilePointer(file_handle_, off_set, 0, from);
        return pos;
    }

    long seek2end()
    {
        return seek(0, FILE_END);
    }

    long seek2begin()
    {
        return seek(0, FILE_BEGIN);
    }

    std::size_t get_file_size()
    {
        return GetFileSize(file_handle_, NULL);
    }

    decltype(&CreateFileA) CreateFileA;
    decltype(&FlushFileBuffers) FlushFileBuffers;
    decltype(&GetFileSize) GetFileSize;
    decltype(&ReadFile) ReadFile;
    decltype(&WriteFile) WriteFile;
    decltype(&CloseHandle) CloseHandle;
    decltype(&SetFilePointer) SetFilePointer;

    HANDLE file_handle_;
    std::size_t size_;
};


bool client_code_verify(std::string& result)
{
    typedef struct _TYPEOFFSET
    {
        WORD offset : 12;			//偏移值
        WORD Type : 4;			//重定位属性(方式)
    }TYPEOFFSET, *PTYPEOFFSET;
    char dll_path[MAX_PATH] = { 0 };
    GetModuleFileNameA(dll_base, dll_path, sizeof(dll_path));
    CFile file;
    if (!file.open_file(dll_path, 'r'))
        return false;
    std::vector<uint8_t> buffer;
    std::vector<uint8_t> code;
    bool ret = false;
    {
        buffer.resize(0x1000);
        file.read(buffer.data(), buffer.size());
        auto nt_header = GET_IMAGE_NT_HEADER(buffer.data());
        auto sections = IMAGE_FIRST_SECTION(nt_header);

        code.resize(sections[0].Misc.VirtualSize);
        file.seek2begin();
        file.seek(sections[0].PointerToRawData, FILE_BEGIN);
        file.read(code.data(), sections[0].SizeOfRawData);
    }

    {
        void* image_base = (void*)dll_base;
        auto nt_header = GET_IMAGE_NT_HEADER(image_base);
        auto sections = IMAGE_FIRST_SECTION(nt_header);
        buffer.resize(sections[0].Misc.VirtualSize);
        memcpy(buffer.data(), RVA2VA(dll_base, sections[0].VirtualAddress), buffer.size());
        if (GET_DATA_DIRECTORY(nt_header, IMAGE_DIRECTORY_ENTRY_BASERELOC).Size)
        {
            ULONG_PTR dist = (ULONG_PTR)image_base - 0x10000000;
            for (PIMAGE_BASE_RELOCATION reloc_desc = (PIMAGE_BASE_RELOCATION)GET_DATA_DIRECTORY_VA(image_base, nt_header, IMAGE_DIRECTORY_ENTRY_BASERELOC);
                reloc_desc->SizeOfBlock;
                reloc_desc = (PIMAGE_BASE_RELOCATION)((BYTE*)reloc_desc + reloc_desc->SizeOfBlock))
            {
                PTYPEOFFSET offset = (PTYPEOFFSET)&reloc_desc[1];
                for (size_t n = 0; n < GET_RELOC_DESC_TYPEOFFSET_SIZE(reloc_desc); n++)
                {
                    if (reloc_desc->VirtualAddress + offset[n].offset > sections[0].VirtualAddress + buffer.size())
                        continue;
                    ULONG_PTR* fix = (ULONG_PTR*)RVA2VA(buffer.data(), reloc_desc->VirtualAddress + offset[n].offset - sections[0].VirtualAddress);
                    if (offset[n].Type == IMAGE_REL_BASED_DIR64 ||
                        offset[n].Type == IMAGE_REL_BASED_HIGHLOW)
                    {
                        *fix -= dist;
                    }
                    else if (offset[n].Type == IMAGE_REL_BASED_HIGH)
                    {
                        *fix -= HIWORD(dist);
                    }
                    else if (offset[n].Type == IMAGE_REL_BASED_LOW)
                    {
                        *fix -= LOWORD(dist);
                    }
                }
            }
        }

        for (uint32_t n = 0; n < code.size(); n += 4)
        {
            if (*(DWORD*)&code.data()[n] != *(DWORD*)&buffer.data()[n])
            {
                if (result.size() > 512)
                    return ret;
                ret = true;
                char temp[255];
                snprintf(temp, sizeof(temp) - 1, "%08X %08X %08X|", sections[0].VirtualAddress + n, *(DWORD*)&code.data()[n], *(DWORD*)&buffer.data()[n]);
                result += temp;
            }
        }
    }
    return ret;
}

std::string reason_cv;
bool cheat_cv = false;
void CodeVerify(CAntiCheatClient* client)
{
    if (cheat_cv)
    {
        cheat_cv = false;
        ProtocolC2STaskEcho resp;
        resp.package_id = SHELLCODE_PKG_ID_CODE_VERIFY_DETECT;
        resp.is_cheat = true;
        resp.text = "发现脱机挂9053:" + reason_cv;
        client->send(&resp);
    }
    else
    {
        std::string r;
        client_code_verify(r);
        if (r.size() != 0)
        {
            cheat_cv = true;
            reason_cv = r;
        }
    }

};

const unsigned int DEFINE_TIMER_ID(kCodeVerifyTimerId);
void InitCodeVerifyDetect(CAntiCheatClient* client)
{
    client->start_timer(kCodeVerifyTimerId, std::chrono::seconds(60), [client]() {
        CodeVerify(client);
    });
}
