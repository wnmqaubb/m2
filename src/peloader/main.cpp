#include <iostream>
#include <assert.h>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <map>

bool build_shellcode(std::string obj_file_path, std::string bind_dll_path, std::string pdb_path, std::string output_path, std::string entrypoint);
bool build_packfile(std::string target_file_path, std::string dll_path, std::string output_file_path);
bool build_reflective_symbol(std::string dll_path, std::string pdb_path, std::string config, std::string output_path);
bool change_config(std::string dll_path, std::string config, std::string output_path);
bool extract_pe_main_icon(std::string path, std::string output_path);

int main(int argc, char *argv[])
{
    if (argc == 2 && std::string(argv[1]) == "-help")
    {
        std::cout << "========shellcode生成=========" << std::endl
                << "-shellcode [bin生成]" << std::endl
                << "-obj [obj file path]" << std::endl
                << "-dll [bind dll file path] " << std::endl
                << "-pdb [bind dll pdb file path]" << std::endl
                << "-output [output file path]" << std::endl
                << "-entrypoint [entrypoint symbol name]" << std::endl;
        std::cout << "========登录器封装=========" << std::endl
            << "-pack" << std::endl
            <<"-exe [origin exe path]" << std::endl
            <<"-dll [bind dll file path]" << std::endl
            <<"-output [output file path]" << std::endl;
        return 0;
    }
    std::map<std::string, std::string> args;
    for (int n = 0; n < argc; n++)
    {
        if (argv[n][0] == '-')
        {
            if (!(n + 1 < argc))
            {
                std::cout << "params invaild" << std::endl;
                return 1;
            }
            args.emplace(std::make_pair(argv[n], argv[n + 1]));
        }
    }
    if (args.find("-shellcode") != args.end())
    {
        if (!std::filesystem::exists(args["-obj"]))
        {
            std::cout << "obj file not found" << std::endl;
            return 1;
        }
        if (!std::filesystem::exists(args["-dll"]))
        {
            std::cout << "dll file not found" << std::endl;
            return 1;
        }
        if (!std::filesystem::exists(args["-pdb"]))
        {
            std::cout << "pdb file not found" << std::endl;
            return 1;
        }
        if (args["-entrypoint"] == "")
        {
            std::cout << "entrypoint invaild" << std::endl;
            return 1;
        }
        if (std::filesystem::is_directory(args["-obj"]))
        {
            if (!std::filesystem::is_directory(args["-output"]))
            {
                std::cout << "output file path shoule be a directory" << std::endl;
                return 1;
            }
            for (auto& file : std::filesystem::directory_iterator(args["-obj"]))
            {
                if (file.path().extension() == ".obj")
                {
                    if (build_shellcode(
                        file.path().string(),
                        args["-dll"],
                        args["-pdb"],
                        (std::filesystem::path(args["-output"]) / (file.path().filename().string() + ".bin")).string(),
                        args["-entrypoint"]
                    ))
                    {
                        std::cout << file.path().filename().string() << " build success" << std::endl;
                    }
                    else
                    {
                        std::cout << file.path().filename().string() << " build failed" << std::endl;
                        return 1;
                    }
                }
            }
            return 0;
        }
        
        if (build_shellcode(
            args["-obj"],
            args["-dll"],
            args["-pdb"],
            args["-output"],
            args["-entrypoint"]
        ))
        {
            std::cout << args["-output"] << " build success" << std::endl;
            return 0;
        }
        std::cout << args["-output"] << " build failed" << std::endl;
        return 1;
    }
    else if (args.find("-pack") != args.end())
    {
        if (!std::filesystem::exists(args["-exe"]))
        {
            std::cout << "exe file not found" << std::endl;
            return 1;
        }
        if (!std::filesystem::exists(args["-dll"]))
        {
            std::cout << "dll file not found" << std::endl;
            return 1;
        }
        std::filesystem::path config_dll(std::filesystem::current_path() / "temp" / std::filesystem::path(args["-dll"]).filename());

        if (change_config(args["-dll"], args["-config"], config_dll.parent_path().string()))
        {
            if (build_packfile(args["-exe"], config_dll.string(), args["-output"]))
            {
                std::cout << "input file:" << args["-exe"] << std::endl;
                std::cout << "input file size:" << std::filesystem::file_size(args["-exe"]) << std::endl;
                std::cout << "output file:" << (std::filesystem::current_path() / args["-output"]).string() << std::endl;
                std::cout << "output file size:" << std::filesystem::file_size(args["-output"]) << std::endl;
                std::cout << "封装成功" << std::endl;
                return 0;
            }
        }

        std::cout << "封装失败" << std::endl;
        return 1;
    }
    else if (args.find("-symbol") != args.end())
    {
        if (!std::filesystem::exists(args["-dll"]))
        {
            std::cout << "dll file not found" << std::endl;
            return 1;
        }
        if (!std::filesystem::exists(args["-pdb"]))
        {
            std::cout << "pdb file not found" << std::endl;
            return 1;
        }
        if (build_reflective_symbol(args["-dll"], args["-pdb"], args["-config"], args["-output"]))
        {
            std::cout << "build reflective symbol success" << std::endl;
            return 0;
        }
        std::cout << "build reflective symbol failed" << std::endl;
        return 1;
    }
    else if (args.find("-config") != args.end())
    {
        if (!std::filesystem::exists(args["-dll"]))
        {
            std::cout << "dll file not found" << std::endl;
            return 1;
        }
        if (change_config(args["-dll"], args["-config"], args["-output"]))
        {
            std::cout << "config dll success" << std::endl;
            return 0;
        }
        std::cout << "config dll failed" << std::endl;
        return 1;
    }
    else if (args.find("-static_detect") != args.end())
    {
        if (!std::filesystem::exists(args["-exe"]))
        {
            std::cout << "exe file not found" << std::endl;
            return 1;
        }
        if (std::filesystem::is_directory(args["-exe"]))
        {
            for (auto& file : std::filesystem::directory_iterator(args["-exe"]))
            {
                if (file.path().extension() == ".exe")
                {
                    extract_pe_main_icon(file.path().string(), args["-output"]);
                }
            }
        }
        else
        {
            extract_pe_main_icon(args["-exe"], args["-output"]);
        }
        return 0;
    }
    else
    {
        std::cout << "-help 显示帮助" << std::endl;
        return 1;
    }
	return 0;
}

