#include <pe_bliss.h>
#include <pe_bliss_resources.h>
#include <fstream>
#include <string>
#include "peloader.h"
using namespace pe_bliss;

extern PELoader::ShareData share;

pe_types_class_32::NtHeaders& get_nt_headers_32_ptr(pe_base* base)
{
    return *(pe_types_class_32::NtHeaders*)base->get_nt_headers_ptr();
}
bool copy_append_data(std::string target_file_path, std::string output_file_path)
{
    std::ifstream pe_file(target_file_path, std::ios::in | std::ios::binary);
    pe_base image(pe_factory::create_pe(pe_file));
    const section_list& pe_sections = image.get_image_sections();
    pe_sections.back().get_pointer_to_raw_data();
    pe_file.seekg(0, pe_file.end);
    size_t pe_file_size = pe_file.tellg();
    size_t pe_file_image_end = pe_sections.back().get_size_of_raw_data() + pe_sections.back().get_pointer_to_raw_data();
    if (pe_file_size == pe_file_image_end)
        return false;
    pe_file.seekg(pe_file_image_end);
    size_t append_data_size = pe_file_size - pe_file_image_end;
    std::unique_ptr<unsigned char[]> append_data(new unsigned char[append_data_size]);
    pe_file.read((char*)append_data.get(), append_data_size);
    std::ofstream output(output_file_path, std::ios::out | std::ios::binary | std::ios::app);
    output.seekp(0, output.end);
    output.write((char*)append_data.get(), append_data_size);
    pe_file.close();
    output.close();
}
void copy_option_headers(pe_base* source, pe_base* dest)
{
    get_nt_headers_32_ptr(dest).OptionalHeader.BaseOfCode
        = get_nt_headers_32_ptr(source).OptionalHeader.BaseOfCode;
    get_nt_headers_32_ptr(dest).OptionalHeader.BaseOfData
        = get_nt_headers_32_ptr(source).OptionalHeader.BaseOfData;
    get_nt_headers_32_ptr(dest).OptionalHeader.SizeOfCode
        = get_nt_headers_32_ptr(source).OptionalHeader.SizeOfCode;
    get_nt_headers_32_ptr(dest).OptionalHeader.SizeOfInitializedData
        = get_nt_headers_32_ptr(source).OptionalHeader.SizeOfInitializedData;
    get_nt_headers_32_ptr(dest).OptionalHeader.SizeOfUninitializedData
        = get_nt_headers_32_ptr(source).OptionalHeader.SizeOfUninitializedData;
    get_nt_headers_32_ptr(dest).OptionalHeader.MajorLinkerVersion
        = get_nt_headers_32_ptr(source).OptionalHeader.MajorLinkerVersion;
    get_nt_headers_32_ptr(dest).OptionalHeader.MajorImageVersion
        = get_nt_headers_32_ptr(source).OptionalHeader.MajorImageVersion;
    get_nt_headers_32_ptr(dest).OptionalHeader.MajorOperatingSystemVersion
        = get_nt_headers_32_ptr(source).OptionalHeader.MajorOperatingSystemVersion;
    get_nt_headers_32_ptr(dest).OptionalHeader.MajorSubsystemVersion
        = get_nt_headers_32_ptr(source).OptionalHeader.MajorSubsystemVersion;

    get_nt_headers_32_ptr(dest).OptionalHeader.MinorLinkerVersion
        = get_nt_headers_32_ptr(source).OptionalHeader.MinorLinkerVersion;
    get_nt_headers_32_ptr(dest).OptionalHeader.MinorImageVersion
        = get_nt_headers_32_ptr(source).OptionalHeader.MinorImageVersion;
    get_nt_headers_32_ptr(dest).OptionalHeader.MinorOperatingSystemVersion
        = get_nt_headers_32_ptr(source).OptionalHeader.MinorOperatingSystemVersion;
    get_nt_headers_32_ptr(dest).OptionalHeader.MinorSubsystemVersion
        = get_nt_headers_32_ptr(source).OptionalHeader.MinorSubsystemVersion;
}

bool build_packfile(std::string target_file_path, std::string dll_path, std::string output_file_path)
{
    std::ifstream pe_file(target_file_path, std::ios::in | std::ios::binary);
    if (!pe_file)
    {
        return false;
    }
    pe_base image(pe_factory::create_pe(pe_file));
    if (image.get_pe_type() == pe_type_64)
    {
        return false;
    }
    pe_base new_image(pe_properties_32(), image.get_section_alignment());

    new_image.set_characteristics(image.get_characteristics());
    new_image.set_dll_characteristics(image.get_dll_characteristics());
    new_image.set_file_alignment(image.get_file_alignment());
    new_image.set_heap_size_commit(image.get_heap_size_commit_64());
    new_image.set_heap_size_reserve(image.get_heap_size_reserve_64());
    new_image.set_stack_size_commit(image.get_stack_size_commit_64());
    new_image.set_stack_size_reserve(image.get_stack_size_reserve_64());
    new_image.set_image_base_64(image.get_image_base_64());
    new_image.set_ep(image.get_ep());
    new_image.set_number_of_rvas_and_sizes(new_image.pe_base::get_number_of_rvas_and_sizes());
    new_image.set_subsystem(image.get_subsystem());
    new_image.set_time_date_stamp(image.get_time_date_stamp());

    copy_option_headers(&image, &new_image);

    for (unsigned long i = 0; i < image.get_number_of_rvas_and_sizes(); ++i)
    {
        if (i == pe_win::image_directory_entry_bound_import)
        {
            new_image.set_directory_rva(i, 0);
            new_image.set_directory_size(i, 0);
        }
        else
        {
            new_image.set_directory_rva(i, image.get_directory_rva(i));
            new_image.set_directory_size(i, image.get_directory_size(i));
        }
    }

    {
        const section_list& pe_sections = image.get_image_sections();
        for (section_list::const_iterator it = pe_sections.begin(); it != pe_sections.end(); ++it)
        {
            if (it->get_name() == ".reloc")
            {
                continue;
            }
            if (it->get_name() == ".rsrc")
            {
                continue;
            }
            new_image.set_section_virtual_size(new_image.add_section(*it), (*it).get_virtual_size());
        }
    }


        srand(::GetTickCount());
        section crypt_code_section;
        crypt_code_section.get_raw_data().resize(1);
        crypt_code_section.set_name(".mapo");
        crypt_code_section.readable(true);

        //shellcode初始化及填充
        IMAGE_SECTION_HEADER shell_section = *PELoader::get_shell_section();
        std::string shellcode;
        shellcode.resize(shell_section.SizeOfRawData);
        share.oep = image.get_ep();
        share.xor_key = rand();
        share.origin_image_base = image.get_image_base_64();

        /* 重定位信息记载 */
        relocation_table_list relocs_tables(get_relocations(image));
        std::map<uint32_t, relocation_table> new_relocs_tables;
        std::vector<PELoader::RelocInfo> relocs;

        section code_section = new_image.get_image_sections().front();

        for (auto& table : relocs_tables)
        {
            for (auto& reloc : table.get_relocations())
            {
                if (code_section.get_virtual_address() <= table.get_rva() && table.get_rva() <= (code_section.get_virtual_address() + code_section.get_virtual_size()))
                {
                    relocs.push_back({
                        reloc.get_type(),
                        table.get_rva() + reloc.get_rva()
                        });
                }
                else
                {
                    new_relocs_tables[table.get_rva()].set_rva(table.get_rva());
                    new_relocs_tables[table.get_rva()].add_relocation(reloc);
                }
            }
        }

        relocs_tables.clear();
        for (auto& new_relocs_tables_itor : new_relocs_tables)
        {
            relocs_tables.push_back(new_relocs_tables_itor.second);
        }
        
        section new_relocs;
        new_relocs.get_raw_data().resize(1);
        new_relocs.set_name(".reloc");
        new_relocs.readable(true);
        section& attached_section = new_image.add_section(new_relocs);
        rebuild_relocations(new_image, relocs_tables, attached_section);
        resource_directory root(get_resources(image));
        section rsrc;
        rsrc.set_name(".rsrc");
        rsrc.get_raw_data().resize(1);
        rsrc.readable(true);
        rsrc.set_flag(pe_win::image_scn_cnt_initialized_data, true);
        section& attach_rsrc = new_image.add_section(rsrc);
        rebuild_resources(new_image, root, attach_rsrc);
        
        std::ifstream dll_file(dll_path, std::ios::in | std::ios::binary);
        dll_file.seekg(0, dll_file.end);
        const size_t bind_dll_size = dll_file.tellg();
        dll_file.seekg(0);

        const size_t text_code_size = code_section.get_raw_data().size();
        
        const size_t crypt_code_relocs_size = offsetof(PELoader::CryptCode, reloc) + relocs.size() * sizeof(PELoader::RelocInfo) + text_code_size + bind_dll_size;
        std::unique_ptr<unsigned char[]> crypt_code(new unsigned char[crypt_code_relocs_size]);
        crypt_code_section.get_raw_data().resize(crypt_code_relocs_size);
        ((PELoader::CryptCodePtr)crypt_code.get())->text_code_size = text_code_size;
        ((PELoader::CryptCodePtr)crypt_code.get())->bind_dll_size = bind_dll_size;
        ((PELoader::CryptCodePtr)crypt_code.get())->reloc_size = relocs.size();
        int n = 0;
        for (auto& reloc : relocs)
        {
            ((PELoader::CryptCodePtr)crypt_code.get())->reloc[n].rva = reloc.rva;
            ((PELoader::CryptCodePtr)crypt_code.get())->reloc[n].type = reloc.type;
            n++;
        }
        const size_t text_code_offset = offsetof(PELoader::CryptCode, reloc) + relocs.size() * sizeof(PELoader::RelocInfo);
        memcpy(crypt_code.get() + text_code_offset,
            code_section.get_raw_data().data(),
            text_code_size);

        if (bind_dll_size)
        {
            dll_file.read((char*)(crypt_code.get() + text_code_offset + text_code_size), bind_dll_size);
        }

        memcpy(crypt_code_section.get_raw_data().data(), crypt_code.get(), crypt_code_relocs_size);
        

        PELoader::xor_buffer(crypt_code_section.get_raw_data().data(), crypt_code_section.get_raw_data().size(), share.xor_key);
        section& bind_crypt_code_section = new_image.add_section(crypt_code_section);
        share.crypt_code_rva = bind_crypt_code_section.get_virtual_address();
        share.crypt_code_size = bind_crypt_code_section.get_raw_data().size();

        int random = shellcode.size() + (rand() % (code_section.get_raw_data().size() - shellcode.size()));

        share.stub_rva = random;
        share.stub_size = shellcode.size();
        share.stub_entry_rva = (DWORD)&PELoader::stub_entry - (DWORD)RVA2VA(GetModuleHandleA(NULL), shell_section.VirtualAddress);

        memcpy((void*)shellcode.data(),
            RVA2VA(GetModuleHandleA(NULL), shell_section.VirtualAddress),
            shell_section.SizeOfRawData);

        //清空代码段
        //memset((void*)new_image.get_image_sections().front().get_raw_data().data(), 0xCC, code_section.get_raw_data().size());

        if (image.get_image_sections().front().get_virtual_address() <= image.get_directory_rva(pe_win::image_directory_entry_import)
            && image.get_directory_rva(pe_win::image_directory_entry_import) <= image.get_image_sections().front().get_virtual_size())
        {
            section& old_text = image.get_image_sections().front();
            section& text = new_image.get_image_sections().front();
            memcpy(text.get_raw_data().data() + image.get_directory_rva(pe_win::image_directory_entry_import) - text.get_virtual_address(),
                old_text.get_raw_data().data() + image.get_directory_rva(pe_win::image_directory_entry_import) - old_text.get_virtual_address(),
                text.get_raw_data().size() - (image.get_directory_rva(pe_win::image_directory_entry_import) - text.get_virtual_address()));
        }
        
        //写入目标文件代码段
        memcpy((void*)(new_image.get_image_sections().front().get_raw_data().data() + random),
            shellcode.data(),
            shellcode.size());
        new_image.set_ep(new_image.get_image_sections().front().get_virtual_address() + random + (DWORD)&share.eip_opcode - (DWORD)RVA2VA(GetModuleHandleA(NULL), shell_section.VirtualAddress));

    //关闭重定位
    new_image.set_characteristics_flags(pe_bliss::pe_win::image_file_relocs_stripped);

    std::stringstream temp_pe(std::ios::out | std::ios::in | std::ios::binary);
    rebuild_pe(new_image, temp_pe);
    //new_image->set_checksum(calculate_checksum(temp_pe));
    
    std::ofstream new_pe_file(output_file_path.c_str(), 
        std::ios::out | std::ios::binary | std::ios::trunc);
    rebuild_pe(new_image, new_pe_file);
    new_pe_file.close();

    copy_append_data(target_file_path, output_file_path);
    return true;
}