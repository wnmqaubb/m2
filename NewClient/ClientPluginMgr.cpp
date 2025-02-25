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
        // ����
        std::unique_lock<std::shared_mutex> lck(mtx_);

        std::error_code ec;
        HMODULE plugin_handle = NULL;

        // ��ȡ�������·��
        auto plugin_cache_path = cache_dir_ / get_plugin_cache_str(plugin_hash);

    #ifdef _DEBUG
        // ����ģʽ�£����Ի�ȡģ����
        plugin_handle = GetModuleHandleA(filename.c_str());
        if (plugin_handle)
            return true;
        // ���ؿ�
        plugin_handle = LoadLibraryA(filename.c_str());
    #else
        // �ǵ���ģʽ�£����������·���Ƿ����
        if (!fs::exists(plugin_cache_path, ec))
        {
            return false;
        }

        // �򿪲�������ļ�
        std::ifstream file(plugin_cache_path, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }

        // ��ȡ�ļ����ݵ��ַ�����
        std::stringstream ss;
        ss << file.rdbuf();
        file.close();
        auto buffer = ss.str();

        // ����Э���
        RawProtocolImpl package;
        if (!package.decode(std::string_view(buffer.data(), buffer.size())))
        {
            fs::remove_all(plugin_cache_path, ec);
            return false;
        }

        // ������������Ϣ
        auto msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
        auto plugin = msg.get().as<ProtocolS2CDownloadPlugin>();
        if (!plugin.is_crypted || plugin.plugin_hash != plugin_hash)
        {
            fs::remove_all(plugin_cache_path, ec);
            return false;
        }

        // ���ܲ������
        xor_buffer(plugin.data.data(), plugin.data.size(), kProtocolXorKey);
        plugin_handle = *dll_base;
        // ���ز��
        if (peload(plugin.data.data(), sizeof(IMAGE_DOS_HEADER), &plugin_handle, NULL) != ERROR_SUCCESS || !plugin_handle)
        {
            instance_->log(0, TEXT("���ز��ʧ��"));
            fs::remove_all(plugin_cache_path, ec);
            return false;
        }

        // ����SEH
        enable_seh_on_shellcode();
        // ִ��TLS�ص�����
        execute_tls_callback(plugin_handle, DLL_PROCESS_ATTACH, 0);
        // ִ����ڵ�
        execute_entrypoint(plugin_handle, DLL_PROCESS_ATTACH, 0);
    #endif

        // ����Ƿ�ɹ����ز��
        if (!plugin_handle)
        {
            fs::remove_all(plugin_cache_path, ec);
            return false;
        }

        // ��ȡ�����ڵ�
        decltype(&LoadPlugin) plugin_entry = (decltype(&LoadPlugin))ApiResolver::get_proc_address(plugin_handle, ApiResolver::hash("LoadPlugin"));
        if (plugin_entry)
        {
    #if LOG_SHOW
            // ��������ڵ��ַ
            char path[MAX_PATH];
            sprintf_s(path, MAX_PATH, "plugin_entry %08X", plugin_entry);
            OutputDebugStringA(path);
    #endif
            // ���ò����ڵ�
            plugin_entry(instance_);
        }

        // ���ģ����Ϣ
        ProtocolModuleInfo module_info;
        module_info.base = (uintptr_t)plugin_handle;
        module_info.module_name = Utils::String::c2w(filename);
        module_info.size_of_image = ApiResolver::get_image_nt_header(plugin_handle)->OptionalHeader.SizeOfImage;
        // ��ģ����Ϣ��ӵ�����б���
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
