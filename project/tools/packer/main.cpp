#include "packer.h"
#include "cmdline.h"
using namespace Packer;

int main(int argc, char** argv)
{
    cmdline::parser a;
    a.add("generate_reflective_loader", '\x0', "[--obj][--dll][--output]");
    a.add("obfuse_dll_export", '\x0', "[--dll]");
    a.add("obfuse_dll_import", '\x0', "[--dll]");
    a.add("make_loader_payload", '\x0', "[--obj]");
    a.add("pack_exe", '\x0', "[--exe][--dll][--output]");
    a.add<std::string>("obj", 'o', "obj path", false);
    a.add<std::string>("dll", 'd', "dll path", false);
    a.add<std::string>("exe", 'e', "exe path", false);
    a.add<std::string>("config", 'c', "config", false);
    a.add<std::string>("output", '\x0', "output path", false);
    a.parse_check(argc, argv);
    if (a.exist("generate_reflective_loader"))
    {
        if (!(a.exist("obj") && a.exist("output")) && a.exist("dll"))
            return 1;
        std::string payload;
        build_raw_shellcode(a.get<std::string>("obj"), 
            "_reflective_loader_entry", 
            load_file(a.get<std::string>("dll")), 
            a.get<std::string>("output"), 
            payload);
        return 0;
    }
    else if (a.exist("obfuse_dll_export"))
    {
        obfuse_dll_export(a.get<std::string>("dll"));
        return 0;
    }
    else if (a.exist("obfuse_dll_import"))
    {
        obfuse_dll_import(a.get<std::string>("dll"), a.get<std::string>("output"));
        return 0;
    }
    else if (a.exist("make_loader_payload"))
    {
        fs::path output_dir(a.get<std::string>("output"));

        std::string stage_first_payload;
        std::string stage_second_payload;
        build_raw_shellcode(a.get<std::string>("obj"),
            "_loader_entry@4",
            "",
            (output_dir / "stage_1_payload.bin").string(),
            stage_first_payload);
        build_raw_shellcode(a.get<std::string>("obj"),
            "_stub_entry",
            "",
            (output_dir / "stage_2_payload.bin").string(),
            stage_second_payload);
        return 0;
    }
    else if (a.exist("pack_exe"))
    {
        if (!(a.exist("dll") && a.exist("output")) && a.exist("exe"))
            return 1;
        
        if (build_packfile(a.get<std::string>("exe"),
            a.get<std::string>("dll"),
            a.get<std::string>("output"),
            load_file(get_parent_dir() / "stage_1_payload.bin"),
            load_file(get_parent_dir() / "stage_2_payload.bin"),
            a.get<std::string>("config")))
        {
            std::cout << "加壳前大小:" << fs::file_size(a.get<std::string>("dll")) << std::endl;
            std::cout << "加壳后大小:" << fs::file_size(a.get<std::string>("output")) << std::endl;
            std::cout << "加壳成功" << std::endl;
            return 0;
        }
        std::cout << "加壳失败" << std::endl;
        return 1;
    }
    std::cerr << a.usage() << std::endl;
    return 1;
}
