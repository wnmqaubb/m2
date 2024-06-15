#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "packer.h"
#include "coff_reader.h"
#include "Lightbone/utils.h"
#include "Lightbone/api_resolver.h"
#include <iostream>
#include "Service/SubServicePackage.h"
#include "ShellCode/BasicUtils.h"

using namespace PE;
namespace Packer
{

    void execute_shellcode(const std::string& payload)
    {
        void* buffer = VirtualAlloc(NULL, payload.size(), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        memcpy(buffer, payload.data(), payload.size());
        typedef void(*entrypoint)();
        ((entrypoint)buffer)();
    }

    bool build_raw_shellcode(
        const std::string& obj_file_path,
        const std::string& entrypoint,
        const std::string& bind_data,
        const std::string& output_path,
        std::string& payload
    )
    {
        coff_reader_t coff(obj_file_path);
        section_vector_t& sections = coff.get_section_header();
        full_symbol_set_t all_needed_functions;
        full_symbol_set_t import_functions;
        std::ofstream output(output_path, std::ios::out | std::ios::binary);
        //从入口点开始遍历所有需要的函数
        std::map<full_symbol_t, std::pair<uint32_t, uint32_t>> link_functions;
        full_symbol_t symbol;
        if (!coff.find_symbol(entrypoint, symbol))
        {
            printf("entry point not found\n");
            return false;
        }

        coff.get_symbol_link_func(entrypoint, all_needed_functions);

        std::string raw_data;
        for (auto current_function : all_needed_functions)
        {
            if (current_function.is_import_symbol())
            {
                printf("not support %s import outside \n", current_function.name.c_str());
                assert(!current_function.is_import_symbol());
            }
            const size_t func_size = coff.get_symbol_size(current_function);
            printf("func %s sz:%d\n", current_function.name.c_str(), func_size);
            size_t func_head = raw_data.size();
            raw_data += coff.get_symbol_raw_data(current_function);
            link_functions.insert(std::make_pair(current_function, std::make_pair(func_head, func_size)));
        }

        for (auto current_function : all_needed_functions)
        {
            auto relocs = coff.get_symbol_reloc(current_function, link_functions[current_function].second);
            for (auto& reloc : relocs)
            {
                if (reloc.Type != IMAGE_REL_I386_REL32 && reloc.Type != IMAGE_REL_AMD64_REL32)
                {
                    printf("not support reloc %X in %s+%X\n", reloc.Type, current_function.name.c_str(), reloc.VirtualAddress - current_function.value);
                }
                else
                {
                    printf("fix reloc %X in %s+%X\n", reloc.Type, current_function.name.c_str(), reloc.VirtualAddress - current_function.value);
                }
                assert(reloc.Type == IMAGE_REL_I386_REL32 || reloc.Type == IMAGE_REL_AMD64_REL32);
                auto target_func = coff.get_symbols()[reloc.SymbolTableIndex];
                coff.fix_reloc(reloc.Type,
                    (uint8_t*)&raw_data[link_functions[current_function].first + reloc.VirtualAddress - current_function.value],
                    link_functions[current_function].first + reloc.VirtualAddress - current_function.value,
                    link_functions[target_func].first,
                    0);
            }
        }
        raw_data += "\xEF\xBE\xAD\xDE";
        raw_data += bind_data;
        raw_data += "\xBE\xBA\xFE\xCA";
        output.write(raw_data.data(), raw_data.size());
        output.close();
        payload = raw_data;
        return true;
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

    void obfuse_dll_export(fs::path dll)
    {
        auto buffer = load_file(dll);
        size_t sz = buffer.size();
        unsigned char* image = (unsigned char*)buffer.data();
        size_t image_export_dir_va = (size_t)ApiResolver::get_image_export_directory(image);
        size_t image_export_dir_fa = ApiResolver::va2fa(image, image_export_dir_va);
        auto image_export_dir = ApiResolver::fa2ptr<PIMAGE_EXPORT_DIRECTORY>(image, image_export_dir_fa);
        auto address_of_names = ApiResolver::rva2va<uint32_t*>(image, ApiResolver::rva2fa(image, image_export_dir->AddressOfNames));
        for (int i = 0; i < image_export_dir->NumberOfNames; i++)
        {
            char* name = ApiResolver::fa2ptr<char*>(image, ApiResolver::rva2fa(image, address_of_names[i]));
            if (std::string(name).find("client_entry") != std::string::npos)
            {
                continue;
            }
            size_t len = strlen(name);
            if (name[0] == '_' && len == 8)
            {
                continue;
            }

            if (len < 9)
            {
                continue;
            }

            printf("obfuse export %s\n", name);

            uint32_t hash_val = ApiResolver::hash(name);
            memset(name, 0, len);
            char temp[10] = { 0 };
            snprintf(temp, sizeof(temp) - 1, "_%08X", hash_val);
            strcpy(name, temp);
        }

        std::error_code ec;
        fs::remove(dll.string() + ".bak", ec);
        fs::copy_file(dll, dll.string() + ".bak", ec);
        std::ofstream output(dll, std::ios::out | std::ios::binary);
        output.write(buffer.data(), buffer.size());
        output.close();
    }
    void obfuse_dll_import(fs::path dll, fs::path output_dir)
    {
        auto buffer = load_file(dll);
        size_t sz = buffer.size();
        unsigned char* image = (unsigned char*)buffer.data();
        size_t image_import_dir_va = (size_t)ApiResolver::get_image_import_directory(image);
        size_t image_import_dir_fa = ApiResolver::va2fa(image, image_import_dir_va);
        auto image_import_dir = ApiResolver::fa2ptr<PIMAGE_IMPORT_DESCRIPTOR>(image, image_import_dir_fa);

        for (auto import_desc = image_import_dir;
            import_desc->Name;
            import_desc++)
        {
            char* import_module_name = ApiResolver::fa2ptr<char*>(image, ApiResolver::rva2fa(image, import_desc->Name));
            PIMAGE_THUNK_DATA iat = ApiResolver::fa2ptr<PIMAGE_THUNK_DATA>(image, ApiResolver::rva2fa(image, import_desc->FirstThunk));
            PIMAGE_THUNK_DATA thunk = import_desc->OriginalFirstThunk ?
                ApiResolver::fa2ptr<PIMAGE_THUNK_DATA>(image, ApiResolver::rva2fa(image, import_desc->OriginalFirstThunk)) :
                ApiResolver::fa2ptr<PIMAGE_THUNK_DATA>(image, ApiResolver::rva2fa(image, import_desc->FirstThunk));
            
            std::filesystem::path full_path = dll.parent_path() / import_module_name;
            if (full_path.filename() == "VMProtectSDK32.dll")
                continue;
            if (!fs::exists(full_path))
            {
                continue;
            }
            printf("obfuse import %s\n", import_module_name);
            for (;
                iat && thunk && thunk->u1.AddressOfData;
                iat++, thunk++)
            {
                if (IMAGE_SNAP_BY_ORDINAL(thunk->u1.Ordinal))
                {
                    printf("pass ordinal %s:%d\n", import_module_name, IMAGE_ORDINAL(thunk->u1.Ordinal));
                    continue;
                }
                char* import_function_name = (char*)ApiResolver::fa2ptr<PIMAGE_IMPORT_BY_NAME>(image, ApiResolver::rva2fa(image, thunk->u1.AddressOfData))->Name;

                size_t len = strlen(import_function_name);

                if (import_function_name[0] == '_' && len == 8)
                {
                    continue;
                }
                if (len < 9)
                {
                    continue;
                }
                printf("obfuse import %s\n", import_function_name);
                uint32_t hash_val = ApiResolver::hash(import_function_name);
                memset(import_function_name, 0, len);
                char temp[10] = { 0 };
                snprintf(temp, sizeof(temp) - 1, "_%08X", hash_val);
                strcpy(import_function_name, temp);
            }
        }

        {
            std::error_code ec;
            fs::remove(dll.string() + ".bak", ec);
            fs::copy_file(dll, dll.string() + ".bak", ec);
            std::ofstream output(dll, std::ios::out | std::ios::binary);
            output.write(buffer.data(), buffer.size());
            output.close();
        }

        if (!output_dir.empty())
        {
            ProtocolS2CDownloadPlugin package;
            package.data.resize(buffer.size());
            std::copy(buffer.begin(), buffer.end(), package.data.begin());
            xor_buffer(package.data.data(), package.data.size(), kProtocolXorKey);
            package.is_crypted = 1;
            package.plugin_hash = Utils::Crypto::aphash((unsigned char*)buffer.data(), buffer.size());
            package.plugin_name = dll.filename().string();
            RawProtocolImpl proto;
            msgpack::sbuffer sbuf;
            msgpack::pack(sbuf, package);
            proto.encode(sbuf.data(), sbuf.size());
            std::ofstream output(output_dir / dll.filename(), std::ios::out | std::ios::binary);
            auto res = proto.release();
            output.write(res.data(), res.size());
            output.close();
            std::cout << "write to " << output_dir / dll.filename();
        }
    }

    fs::path get_parent_dir()
    {
        char path[MAX_PATH] = { 0 };
        GetModuleFileNameA(GetModuleHandleA(NULL), path, sizeof(path));
        return fs::path(path).parent_path();
    }

    bool extract_pe_main_icon(std::string& path, std::string& output_path)
    {        
        try
        {
            uint32_t hash_val = 0;
            Utils::PEScan::calc_pe_ico_hash(Utils::String::c2w(path), &hash_val);
            std::filesystem::path input(path);
            std::filesystem::path output(output_path);
            if (std::filesystem::is_directory(output))
            {
                char name[MAX_PATH] = { 0 };
                _snprintf(name, sizeof(name) - 1, "%s_%08X.txt", input.filename().string().c_str(), hash_val, MAX_PATH);
                output = output / std::string(name);
            }

            std::ofstream output_file(output, std::ios::out | std::ios::binary);
            char hash[20] = { 0 };
            sprintf_s(hash, "0x%08X", hash_val);
            output_file.write(hash, strlen(hash));
            return true;
        }
        catch (...)
        {
            std::cout << "unknown error" << std::endl;
            return false;
        }
    }
}
