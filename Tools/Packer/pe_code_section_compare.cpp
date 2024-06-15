#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Lightbone/utils.h"
#include "Lightbone/api_resolver.h"
#include <string>
#include <iostream>
#include <vector>

#define GET_IMAGE_DOS_HEADER(image) ((PIMAGE_DOS_HEADER)(image))
#define GET_IMAGE_NT_HEADER(image) ((PIMAGE_NT_HEADERS)((ULONG_PTR)(GET_IMAGE_DOS_HEADER(image)->e_lfanew) + (ULONG_PTR)(image)))
#define GET_DATA_DIRECTORY(nt_header,dir) (nt_header->OptionalHeader.DataDirectory[dir])
#define GET_RELOC_DESC_TYPEOFFSET_SIZE(reloc_desc) ((reloc_desc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(TYPEOFFSET))
bool client_code_verify(const std::string& src_path, const std::string& desc_path, std::string& result);
class CFile
{
public:
    CFile()
    {
        file_handle_ = INVALID_HANDLE_VALUE;
    }
    ~CFile()
    {
        if (file_handle_ != INVALID_HANDLE_VALUE)
            CloseHandle(file_handle_);
    }
    bool create(std::string src_path, std::string desc_path, const std::string& path)
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

    HANDLE file_handle_;
    std::size_t size_;
};
ULONG rva2foa(PIMAGE_NT_HEADERS ntheader, ULONG rva, ULONG file_size)
{
    PIMAGE_SECTION_HEADER section_header = IMAGE_FIRST_SECTION(ntheader);
    USHORT number_of_sections = ntheader->FileHeader.NumberOfSections;
    ULONG foa = 0;
    for (int i = 0; i < number_of_sections; i++)
    {
        if (rva >= section_header->VirtualAddress && rva < (section_header->VirtualAddress + section_header->Misc.VirtualSize))
        {
            foa = rva - section_header->VirtualAddress + section_header->PointerToRawData;//FOA=RVA-VA+RAW
            if (foa <= file_size)
                return foa;
        }
        section_header++;
    }
    return 0;
}
bool get_pe_image_code(const std::string& pe_path, std::vector<uint8_t>& code_buffer, PIMAGE_SECTION_HEADER code_section)
{
    typedef struct _TYPEOFFSET
    {
        WORD offset : 12;			//偏移值
        WORD Type : 4;			//重定位属性(方式)
    }TYPEOFFSET, *PTYPEOFFSET;
    CFile file;
    if (!file.open_file(pe_path, 'r'))
        return false;
    std::vector<uint8_t> buffer;
    buffer.resize(file.get_file_size());
    file.read(buffer.data(), buffer.size());
    uint32_t image_base = (uint32_t)buffer.data();
    auto nt_header = GET_IMAGE_NT_HEADER(image_base);
    auto sections = IMAGE_FIRST_SECTION(nt_header);
    *code_section = sections[0];
    auto dir_entry_basereloc = GET_DATA_DIRECTORY(nt_header, IMAGE_DIRECTORY_ENTRY_BASERELOC);
    if (dir_entry_basereloc.Size)
    {
        ULONG_PTR dist = (ULONG_PTR)0;
        uint32_t code_section_foa = rva2foa(nt_header, sections[0].VirtualAddress, buffer.size());
        uint32_t code_section_base = (image_base + rva2foa(nt_header, sections[0].VirtualAddress, buffer.size()));
        uint32_t code_section_size = code_section_base + sections[0].SizeOfRawData;
        for (PIMAGE_BASE_RELOCATION reloc_desc = (PIMAGE_BASE_RELOCATION)(image_base + rva2foa(nt_header, dir_entry_basereloc.VirtualAddress, buffer.size()));
            reloc_desc->SizeOfBlock;
            reloc_desc = (PIMAGE_BASE_RELOCATION)((BYTE*)reloc_desc + reloc_desc->SizeOfBlock))
        {
            PTYPEOFFSET offset = (PTYPEOFFSET)&reloc_desc[1];
            uint32_t reloc_desc_foa = 0;
            reloc_desc_foa = (image_base + rva2foa(nt_header, reloc_desc->VirtualAddress, buffer.size()));
            for (size_t n = 0; n < GET_RELOC_DESC_TYPEOFFSET_SIZE(reloc_desc); n++)
            {
                if (reloc_desc_foa + offset[n].offset > code_section_base + code_section_size)
                    continue;
                ULONG_PTR* fix = (ULONG_PTR*)(reloc_desc_foa + offset[n].offset);
                if (offset[n].Type == IMAGE_REL_BASED_DIR64 ||
                    offset[n].Type == IMAGE_REL_BASED_HIGHLOW)
                {
                    *fix = dist;
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
        code_buffer.resize(sections[0].Misc.VirtualSize);
        memcpy(code_buffer.data(), buffer.data() + code_section_foa, code_buffer.size());
    }
    return true;
}
bool client_code_verify(const std::string& src_path, const std::string& desc_path, std::string& result)
{
    bool ret = false;
    std::vector<uint8_t> src_buffer, desc_buffer;
    IMAGE_SECTION_HEADER src_code_section, desc_code_section;
    if (!get_pe_image_code(src_path, src_buffer, &src_code_section) ||
        !get_pe_image_code(desc_path, desc_buffer, &desc_code_section))
    {
        return false;
    }
    int seq = 1;
    for (uint32_t n = 0; n < src_buffer.size(); n += 4)
    {
        if (*(DWORD*)&src_buffer.data()[n] != *(DWORD*)&desc_buffer.data()[n])
        {
            if (result.size() > 512)
                return ret;
            ret = true;
            char temp[255];
            snprintf(temp, sizeof(temp) - 1, "%02d: FOA:%08X %08X %08X |\n",
                seq++,
                src_code_section.PointerToRawData + n,
                *(DWORD*)&src_buffer.data()[n],
                *(DWORD*)&desc_buffer.data()[n]);
            result += temp;
        }
    }
    return ret;
}