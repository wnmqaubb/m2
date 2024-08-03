#include <string>
#include <fstream>
#include <assert.h>
#include <vector>
#include <filesystem>
#include <iostream>
#include <pe_bliss_resources.h>
#include <pe_bliss.h>
#include <stdio.h>
#include <windows.h>

constexpr unsigned int aphash(const unsigned char *str, uint32_t len)
{
    unsigned int hash = 0;
    for (uint32_t i = 0; i < len; i++)
    {
        if ((i & 1) == 0)
        {
            hash ^= ((hash << 7) ^ (str[i]) ^ (hash >> 3));
        }
        else
        {
            hash ^= (~((hash << 11) ^ (str[i]) ^ (hash >> 5)));
        }
    }
    return (hash & 0x7FFFFFFF);
}

using namespace pe_bliss;
bool extract_pe_main_icon(std::string path, std::string output_path)
{
    std::string main_icon;
    std::ifstream pe_file(path, std::ios::in | std::ios::binary);
    pe_base image(pe_factory::create_pe(pe_file));
    if (!image.has_resources())
    {
        std::cout << "Image has no resources" << std::endl;
        return false;
    }
    try {
        const resource_directory root(get_resources(image));
        pe_resource_viewer res(root);
        pe_resource_viewer::resource_id_list icon_id_list(res.list_resource_ids(pe_resource_viewer::resource_icon_group));
        pe_resource_viewer::resource_name_list icon_name_list(res.list_resource_names(pe_resource_viewer::resource_icon_group));
        if (!icon_name_list.empty())
        {
            main_icon = resource_cursor_icon_reader(res).get_icon_by_name(icon_name_list[0]);
        }
        else if (!icon_id_list.empty()) //§¦§ã§Ý§Ú §ß§Ö§ä §Ú§Þ§Ö§ß§à§Ó§Ñ§ß§ß§í§ç §Ô§â§å§á§á §Ú§Ü§à§ß§à§Ü, §ß§à §Ö§ã§ä§î §Ô§â§å§á§á§í §ã ID
        {
            main_icon = resource_cursor_icon_reader(res).get_icon_by_id(icon_id_list[0]);
        }

        std::filesystem::path input(path);
        std::filesystem::path output(output_path);
        if (std::filesystem::is_directory(output))
        {
            char name[MAX_PATH] = { 0 };
            _snprintf(name, sizeof(name) - 1, "%s_%08X.ico", input.filename().string().c_str(), aphash((unsigned char*)main_icon.data(), main_icon.size()));
            output = output / std::string(name);
        }
        std::ofstream output_file(output, std::ios::out | std::ios::binary);
        output_file.write(main_icon.data(), main_icon.size());
        return true;
    }
    catch (...)
    {
        std::cout << "unknown error" << std::endl;
        return false;
    }
    
    
}