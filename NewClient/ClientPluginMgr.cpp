#include "pch.h"
#include "ClientPluginMgr.h"

extern std::shared_ptr<HINSTANCE> dll_base;
extern void LoadPlugin(CAntiCheatClient* client);
extern uint32_t peload(void* buffer, size_t size, HINSTANCE* instance, void* params);
extern void execute_tls_callback(HINSTANCE instance, uint32_t reason, void* param);
extern void execute_entrypoint(HINSTANCE instance, uint32_t reason, void* param);
extern void enable_seh_on_shellcode();

CClientPluginMgr::CClientPluginMgr(fs::path cache_dir_) : cache_dir_(cache_dir_)
{
    instance_ = nullptr;
}

void CClientPluginMgr::set_client_instance(CAntiCheatClient* instance)
{
    std::unique_lock<std::shared_mutex> lck(mtx_);
    instance_ = instance;
}

bool CClientPluginMgr::load_plugin(plugin_hash_t plugin_hash, const std::string& filename)
{
    try {
        // 加锁
        std::unique_lock<std::shared_mutex> lck(mtx_);

        std::error_code ec;
        HMODULE plugin_handle = NULL;

        // 获取插件缓存路径
        auto plugin_cache_path = cache_dir_ / get_plugin_cache_str(plugin_hash);

    #ifdef _DEBUG
        // 调试模式下，尝试获取模块句柄
        plugin_handle = GetModuleHandleA(filename.c_str());
        if (plugin_handle)
            return true;
        // 加载库
        plugin_handle = LoadLibraryA(filename.c_str());
    #else
        // 非调试模式下，检查插件缓存路径是否存在
        if (!fs::exists(plugin_cache_path, ec))
        {
            return false;
        }

        // 打开插件缓存文件
        std::ifstream file(plugin_cache_path, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }

        // 读取文件内容到字符串流
        std::stringstream ss;
        ss << file.rdbuf();
        file.close();
        auto buffer = ss.str();

        // 解码协议包
        RawProtocolImpl package;
        if (!package.decode(std::string_view(buffer.data(), buffer.size())))
        {
            fs::remove_all(plugin_cache_path, ec);
            return false;
        }

        // 解包并检查插件信息
        auto msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
        auto plugin = msg.get().as<ProtocolS2CDownloadPlugin>();
        if (!plugin.is_crypted || plugin.plugin_hash != plugin_hash)
        {
            fs::remove_all(plugin_cache_path, ec);
            return false;
        }

        // 解密插件数据
        xor_buffer(plugin.data.data(), plugin.data.size(), kProtocolXorKey);
        plugin_handle = *dll_base;
        // 加载插件
        if (peload(plugin.data.data(), sizeof(IMAGE_DOS_HEADER), &plugin_handle, NULL) != ERROR_SUCCESS || !plugin_handle)
        {
            instance_->log(0, TEXT("加载插件失败"));
            fs::remove_all(plugin_cache_path, ec);
            return false;
        }

        // 启用SEH
        enable_seh_on_shellcode();
        // 执行TLS回调函数
        execute_tls_callback(plugin_handle, DLL_PROCESS_ATTACH, 0);
        // 执行入口点
        execute_entrypoint(plugin_handle, DLL_PROCESS_ATTACH, 0);
    #endif

        // 检查是否成功加载插件
        if (!plugin_handle)
        {
            fs::remove_all(plugin_cache_path, ec);
            return false;
        }

        // 获取插件入口点
        decltype(&LoadPlugin) plugin_entry = (decltype(&LoadPlugin))ApiResolver::get_proc_address(plugin_handle, ApiResolver::hash("LoadPlugin"));
        if (plugin_entry)
        {
    #if LOG_SHOW
            // 输出插件入口点地址
            char path[MAX_PATH];
            sprintf_s(path, MAX_PATH, "plugin_entry %08X", plugin_entry);
            OutputDebugStringA(path);
    #endif
            // 调用插件入口点
            plugin_entry(instance_);
        }

        // 填充模块信息
        ProtocolModuleInfo module_info;
        module_info.base = (uintptr_t)plugin_handle;
        module_info.module_name = Utils::String::c2w(filename);
        module_info.size_of_image = ApiResolver::get_image_nt_header(plugin_handle)->OptionalHeader.SizeOfImage;
        // 将模块信息添加到插件列表中
        plugin_list_.emplace(plugin_hash, module_info);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

std::string CClientPluginMgr::get_plugin_cache_str(plugin_hash_t plugin_hash)
{
    char plugin_hash_str[20] = { 0 };
    snprintf(plugin_hash_str, sizeof(plugin_hash_str) - 1, "%08X", plugin_hash);
    return plugin_hash_str;
}

bool CClientPluginMgr::is_plugin_cache_exist(plugin_hash_t plugin_hash)
{
    std::error_code ec;
    return fs::exists(cache_dir_ / get_plugin_cache_str(plugin_hash), ec);
}

bool CClientPluginMgr::is_plugin_loaded(plugin_hash_t plugin_hash)
{
    std::shared_lock<std::shared_mutex> lck(mtx_);
    return plugin_list_.find(plugin_hash) != plugin_list_.end();
}

void CClientPluginMgr::save_plugin(const RawProtocolImpl& package, const ProtocolS2CDownloadPlugin& msg)
{
    if (package.body.buffer.empty())
        return;
    std::error_code ec;
    if (!fs::is_directory(cache_dir_, ec))
    {
        fs::create_directory(cache_dir_);
    }
    std::ofstream file(cache_dir_ / get_plugin_cache_str(msg.plugin_hash), std::ios::out | std::ios::binary);
    if (file.is_open())
    {
        auto buf = package.release();
        file.write(buf.data(), buf.size());
        file.close();
    }
}
