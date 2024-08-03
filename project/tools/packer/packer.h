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
}

bool build_packfile(const std::string& target_file_path,
    const std::string& dll_path,
    const std::string& output_file_path,
    const std::string& stage_1_payload,
    const std::string& stage_2_payload,
    const std::string& config);