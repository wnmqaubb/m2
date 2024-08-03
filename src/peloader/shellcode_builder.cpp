#include "pdb_reader.hpp"
#include "coff_reader.h"
#include <pe_bliss.h>
#include <string>
#include <fstream>
#include <assert.h>
#include <vector>
#include <filesystem>
#include <memory>
#include <iostream>
#include <json\json.hpp>
#include <pe_bliss_resources.h>
#include "peloader.h"

using namespace pe_bliss;
using namespace nlohmann;

unsigned int aphash(const char *str)
{
    unsigned int hash = 0;
    for (int i = 0; *str; i++)
    {
        if ((i & 1) == 0)
        {
            hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
        }
        else
        {
            hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
        }
    }
    return (hash & 0x7FFFFFFF);
};

bool change_config(std::string dll_path, std::string config, std::string output_path)
{
    std::ifstream pe_file(dll_path, std::ios::in | std::ios::binary);
    pe_base image(pe_factory::create_pe(pe_file));
    pe_file.close();

    if (image.has_resources() == false)
    {
        std::cout << "there has no .rsrc" << std::endl;
        return false;
    }
    if (image.sections_.back().get_name() != ".rsrc")
    {
        std::cout << ".rsrc is not the last section" << std::endl;
        return false;
    }

    std::string text = "01:360Gsds";
    resource_directory root(get_resources(image));
    pe_resource_manager res(root);
    res.add_resource(text, L"字符串", L"0015", 0);
    res.add_resource(config, L"字符串", L"0010", 0);
    
    rebuild_resources(image, root, image.sections_.back());
    if (!std::filesystem::exists(output_path))
    {
        std::filesystem::create_directory(output_path);
    }
    std::ofstream output(std::filesystem::path(output_path) / std::filesystem::path(dll_path).filename(), std::ios::out | std::ios::binary);
    rebuild_pe(image, output);
    return true;
}

bool build_reflective_symbol(std::string dll_path, std::string pdb_path, std::string config, std::string output_path)
{
    blink::pdb_reader pdb(pdb_path);
    std::unordered_map<std::string, void*> symbols;
    pdb.read_symbol_table(nullptr, symbols);
    
    json rfs_json;

    struct ReflectiveSymbol
    {
        uint32_t hash;
        uint32_t offset;
    };
    
    for (auto &symbol : symbols)
    {
        char hex[20] = { 0 };
        snprintf(hex, sizeof(hex) - 1, "%08X", aphash(symbol.first.c_str()));
        rfs_json[hex] = (uint32_t)symbol.second;
    }
    rfs_json["GOOGLE"] = 
        "Operating System   :32-bit Windows"
        "File Type          :Application"
        "File Sub-Type      :unknown"
        "File Version       :78,0,3904,87"
        "Product Version    :78,0,3904,87"
        ""
        "Product Name       :Google Chrome"
        "File Description   :Google Chrome"
        "Company Name       :Google LLC"
        "Internal Name      :chrome_dll"
        "Legal Copyright    :Copyright 2019 Google LLC. All rights reserved."
        "Original FileName  :chrome.dll"
        ;
    std::vector<uint8_t> data_ubjson = json::to_msgpack(rfs_json);
    std::ifstream pe_file(dll_path, std::ios::in | std::ios::binary);
    pe_base image(pe_factory::create_pe(pe_file));
    pe_file.close();

    if (image.has_resources())
    {
        std::cout << "image have had rsrc" << std::endl;
        return false;
    }

    std::string rfs;
    rfs.resize(data_ubjson.size());
    memcpy(rfs.data(), data_ubjson.data(), data_ubjson.size());
    resource_directory root(get_resources(image));
    pe_resource_manager res(root);
    res.add_resource(rfs, L"字符串", L"0011", 0);
    section rsrc;
    rsrc.get_raw_data().resize(1);
    rsrc.set_name(".rsrc");
    rsrc.readable(true);
    rsrc.set_flag(pe_win::image_scn_cnt_initialized_data, true);
    section& attach_rsrc = image.add_section(rsrc);
    rebuild_resources(image, root, attach_rsrc);
    if (!std::filesystem::exists(output_path))
    {
        std::filesystem::create_directory(output_path);
    }
    std::ofstream output(std::filesystem::path(output_path) / std::filesystem::path(dll_path).filename() ,std::ios::out | std::ios::binary);
    rebuild_pe(image, output);
    output.close();
    if (!change_config(dll_path, config, output_path))
    {
        return false;
    }

    std::cout << "rebuild " << std::filesystem::path(output_path) / std::filesystem::path(dll_path).filename() << std::endl;

    return true;
}
bool build_shellcode(std::string obj_file_path, std::string bind_dll_path, std::string pdb_path, std::string output_path, std::string entrypoint)
{
    CoffReader coff(obj_file_path);
    CoffReader::SectionVector& sections = coff.get_section_header();
    CoffReader::FullSymbolSet all_needed_functions;
    CoffReader::FullSymbolSet import_functions;

    //导入表初始化
    imported_functions_list imports;
    import_library import_lib;
    std::filesystem::path bind_dll(bind_dll_path);
    import_lib.set_name(bind_dll.filename().string());
    //从入口点开始遍历所有需要的函数
    coff.get_symbol_link_func(entrypoint, all_needed_functions);
    std::unique_ptr<pe_base> image;
    image.reset(new pe_base(pe_properties_32()));
    pe_bliss::section temp_code;
    temp_code.get_raw_data().resize(0);

    for (auto current_function : all_needed_functions)
    {
        if (current_function.is_import_symbol() == false)
        {
            const int index = current_function.section_number;
            if (coff.link_sections_.find(index) == coff.link_sections_.end())
            {
                IMAGE_SECTION_HEADER section = coff.get_section_from_symbol(current_function);
                coff.link_sections_.insert(std::make_pair(index,
                    CoffReader::LinkSection(temp_code.get_raw_data().size(),&section)));
                temp_code.get_raw_data() += coff.get_section_raw_data(section);
            }
        }
    }
    
    pe_bliss::section& code = image->add_section(temp_code);
    code.set_name(".CODE");
    size_t reloc_tables_size = code.get_virtual_size() / image->get_section_alignment() + 1;
    std::vector<pe_bliss::relocation_table> reloc_tables(reloc_tables_size);
    for (int n = 0; n < reloc_tables_size; n++)
    {
        reloc_tables[n].set_rva(code.get_virtual_address() + image->get_section_alignment()*n);
    }
    CoffReader::RelocVector import_relocs;

    blink::pdb_reader bind_dll_pdb(pdb_path);
    std::unordered_map<std::string, void*> symbols;
    bind_dll_pdb.read_symbol_table(nullptr, symbols);

    for (auto &link_section : coff.link_sections_)
    {
        CoffReader::RelocVector relocs = coff.get_section_reloc(link_section.second.section);
        for (auto& reloc : relocs)
        {
            auto reloc_symbol = coff.get_symbols()[reloc.SymbolTableIndex];
            if (reloc_symbol.is_import_symbol())
            {
                //需要导入的函数单独记录
                IMAGE_RELOCATION import_reloc;
                import_reloc.VirtualAddress = link_section.second.offset + reloc.VirtualAddress;
                import_reloc.SymbolTableIndex = reloc.SymbolTableIndex;
                import_reloc.Type = reloc.Type;
                import_relocs.push_back(import_reloc);
				if (reloc_symbol.name.size() > 5 && reloc_symbol.name.substr(0, 4) == "??_E")
				{
					if (reloc_symbol.name.find("@std@") == std::string::npos)
					{
						std::cout << reloc_symbol.name.c_str() << "replace to ??_Ebad_alloc@std@@UAEPAXI@Z" << std::endl;
						reloc_symbol.name = "??_Ebad_alloc@std@@UAEPAXI@Z";
					}
				}
                if (import_functions.find(reloc_symbol) == import_functions.end())
                {
                    static uint64_t va = 0;
                    import_functions.emplace(reloc_symbol);
                    imported_function import_func;
                    import_func.set_name(reloc_symbol.name);
                    if (symbols.find(reloc_symbol.name) == symbols.end())
                    {
                        std::cout << reloc_symbol.name << "not found in bind dll" << std::endl;
                        exit(1);
                    }
                    import_func.set_iat_va(reinterpret_cast<uint64_t>(symbols[reloc_symbol.name]));
                    import_lib.add_import(import_func);
                }
                continue;
            }
            uintptr_t target_address = coff.link_sections_[reloc_symbol.section_number].offset + reloc_symbol.value;
            //内部函数重定位修正
            coff.fix_reloc(reloc.Type,
                (uint8_t*)&code.get_raw_data()[link_section.second.offset + reloc.VirtualAddress],
                image->get_image_base_32() + code.get_virtual_address() + link_section.second.offset + reloc.VirtualAddress,
                image->get_image_base_32() + code.get_virtual_address() + target_address,
                image->get_image_base_32());
            if (reloc.Type == IMAGE_REL_I386_DIR32)
            {
                const uint32_t rva = link_section.second.offset + reloc.VirtualAddress;
                uint16_t rva_low = rva & 0xFFF;
                reloc_tables[rva / image->get_section_alignment()].add_relocation(pe_bliss::relocation_entry(rva_low, IMAGE_REL_BASED_HIGHLOW));
            }
        }
    }

    //写入代码段
    code.readable(true).executable(true);
    
    std::filesystem::path path(obj_file_path);
    std::stringstream temp_pe(std::ios::out | std::ios::in | std::ios::binary);
    std::ofstream new_pe_file(output_path,
        std::ios::out | std::ios::binary | std::ios::trunc);

    /*导入表建立*/
    imports.push_back(import_lib);
    pe_bliss::section new_imports;
    new_imports.get_raw_data().resize(1);
    new_imports.set_name(".IAT");
    new_imports.readable(true).writeable(true);
    pe_bliss::section& attached_section = image->add_section(new_imports);
    import_rebuilder_settings settings(true, false);
    pe_bliss::rebuild_imports(*image, imports, attached_section, settings);

    /*导入表重定位修正*/
    for (auto &reloc : import_relocs)
    {
        imports = get_imported_functions(*image);
        auto &xclient = std::find_if(imports.begin(), imports.end(), [&](import_library lib) -> bool {
            return lib.get_name() == bind_dll.filename().string();
        });
        auto funcs = xclient->get_imported_functions();
        auto func = std::find_if(funcs.begin(), funcs.end(), [&](imported_function func) ->bool {
            return func.get_name() == coff.get_symbols()[reloc.SymbolTableIndex].name;
        });
        if (func->get_name() == "__tls_array")
        {
            *(uint32_t*)&(image->sections_.front().get_raw_data()[reloc.VirtualAddress])
                = func->get_iat_va();
            continue;
        }
        assert(func != funcs.end());
        size_t index = func - funcs.begin();
        coff.fix_reloc(reloc.Type,
            (uint8_t*)&(image->sections_.front().get_raw_data()[reloc.VirtualAddress]),
            image->get_image_base_32() + image->sections_.front().get_virtual_address() + reloc.VirtualAddress,
            image->get_image_base_32() + xclient->get_rva_to_iat() + index * sizeof(uintptr_t),
            image->get_image_base_32());
        {
            const uint32_t rva = reloc.VirtualAddress;
            uint16_t rva_low = rva & 0xFFF;
            reloc_tables[rva / image->get_section_alignment()].add_relocation(pe_bliss::relocation_entry(rva_low, reloc.Type));
        }
    }
    
    {
        pe_bliss::section new_relocs;
        new_relocs.get_raw_data().resize(1);
        new_relocs.set_name(".reloc");
        new_relocs.readable(true);
        pe_bliss::section& attached_section = image->add_section(new_relocs);
        pe_bliss::rebuild_relocations(*image, reloc_tables, attached_section);
    }
    
    pe_bliss::rebuild_pe(*image, temp_pe);
    image->set_characteristics_flags(pe_bliss::pe_win::image_file_dll);
    image->set_ep(image->sections_.front().get_virtual_address() + coff.link_sections_[coff.find_symbol(entrypoint).section_number].offset);
    pe_bliss::rebuild_pe(*image, new_pe_file);
    new_pe_file.close();

	// xor加密
	std::string sbuf;
	std::ifstream shell(output_path,std::ios::binary);
	std::stringstream sb;
	sb << shell.rdbuf();
	sbuf = sb.str();
	PELoader::xor_buffer(sbuf.data(), sbuf.size(), 0x3F275A8C);
	std::ofstream xor_pe_file(output_path,std::ios::out | std::ios::binary | std::ios::trunc);
	xor_pe_file << sbuf;
	xor_pe_file.close();
    return true;
}

