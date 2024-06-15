#pragma once
#include <filesystem>
#include <string>

namespace fs = std::filesystem;
namespace Packer
{
    void execute_shellcode(const std::string& payload);
    std::string load_file(fs::path path);
    bool build_raw_shellcode(
        const std::string& obj_file_path,
        const std::string& entrypoint,
        const std::string& bind_data,
        const std::string& output_path,
        std::string& payload
    );
    void obfuse_dll_export(fs::path dll);
    void obfuse_dll_import(fs::path dll, fs::path output_dir);
    fs::path get_parent_dir();
    bool extract_pe_main_icon(std::string& path, std::string& output_path);
}

bool build_packfile(const std::string& target_file_path,
    const std::string& dll_path,
    const std::string& output_file_path,
    const std::string& stage_1_payload,
    const std::string& stage_2_payload,
    const std::string& config);


bool copy_append_data(const std::string& target_file_path, const std::string& output_file_path);
void make_template_cfg(const std::string& output);
void expand_cache_dll(const std::string& src, const std::string& dst);
bool client_code_verify(const std::string& src_path, const std::string& desc_path, std::string& result);