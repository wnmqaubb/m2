#include "pch.h"
#include "task.h"
#include "utils/utils.h"
#include <fstream>
#include <atlconv.h>

extern std::vector<std::unique_ptr<Protocol>> protocol_shell_code_list;

TaskShellcode::TaskShellcode()
{
    set_interval(0);
    set_package_id(Protocol::PackageId::PACKAGE_ID_SHELLCODE);
}

TaskShellcode::~TaskShellcode()
{

}

void TaskShellcode::on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
{
#ifdef SERVER
	const int package_id = package[GlobalString::JsonProperty::ID];
    if (Protocol::PackageId::PACKAGE_ID_SHELLCODE_START < package_id && package_id < Protocol::PackageId::PACKAGE_ID_RMC)
    {
        std::vector<Policy> policies;
        FindPolicy(ENM_POLICY_TYPE_SHELLCODE, policies);
        ProtocolShellCodeInstance proto;
        proto.from_json(package);
        if (proto.is_cheat)
        {
            auto shellcode_policy_itor = std::find_if(policies.begin(), policies.end(), [&proto](Policy& policy)->bool {
                return StrToInt(policy.config.c_str()) == proto.id;
            });
            if (shellcode_policy_itor != policies.end())
            {
                Singleton<TaskPolicy>::getInstance().punish_player(conn_id, *shellcode_policy_itor, proto.reason);
            }
        }
        Utils::log_to_file(conn_id, Utils::CHANNEL_EVENT, GlobalString::LOG::FORMAT_S.c_str(), proto.dump().c_str());
        return;
    }
    if(package_id == Protocol::PackageId::PACKAGE_ID_SHELLCODE)
    {
        ProtocolShellCode proto;
        proto.from_json(package);
        Utils::log_to_file(conn_id, Utils::CHANNEL_EVENT, GlobalString::LOG::FORMAT_S.c_str(), proto.dump().c_str());
    }
#endif
#ifdef CLIENT
    VMP_VIRTUALIZATION_BEGIN();
    ProtocolShellCode proto;
    proto.from_json(package);
    if (proto.status != 0)
    {
        return;
    }
    
    DWORD bitmap_size = proto.uncompress_size;
    std::string shellcode_uncompress;
    shellcode_uncompress.resize(bitmap_size);
    int status = SYS_Uncompress(proto.data.data(), proto.data.size(), (BYTE*)shellcode_uncompress.data(), bitmap_size);
    if (status != 0)
    {
        return;
    }
	Utils::Crypto::xor_buffer(shellcode_uncompress.data(), shellcode_uncompress.size(), 0x3F275A8C);
    proto.status = Utils::execute_shellcode(shellcode_uncompress);
    proto.uncompress_size = 0;
    proto.data.clear();    
    AntiCheat::instance().send(proto);
    VMP_VIRTUALIZATION_END();
#endif
}

void TaskShellcode::shellcode_init()
{
#ifdef SERVER
    protocol_shell_code_list.clear();
    protocol_shell_code_list.shrink_to_fit();
    MainWnd->m_shellManagerDlg.EnumShellDirectory([&](std::filesystem::path file, bool checked, std::string description) {
        if(checked)
        {
            std::string shell;
            ProtocolShellCode proto;
            json temp;
            read_shell_file(file.string(), shell);
            compress_shellcode(shell, proto);
            proto.to_json(temp);
			auto package(std::make_unique<Protocol>(temp));
            protocol_shell_code_list.emplace_back(std::move(package));
        }
        });
#endif
}

void TaskShellcode::compress_shellcode(const std::string& shellcode, ProtocolShellCode& proto)
{
#ifdef SERVER
    DWORD compress_buffer_size = SYS_GuessCompressBound(shellcode.size());
    std::unique_ptr<unsigned char[]> compress_buffer = std::make_unique<unsigned char[]>(compress_buffer_size);
    int status = SYS_Compress((uint8_t*)shellcode.data(),
        shellcode.size(),
        compress_buffer.get(),
        compress_buffer_size);
    if (status != 0)
    {
        proto.status = status;        
        return;
    }
    proto.status = 0;
    proto.uncompress_size = shellcode.size();
    proto.data.resize(compress_buffer_size);
	memcpy(proto.data.data(), compress_buffer.get(), compress_buffer_size);
#endif
	return;
}

void TaskShellcode::send_to_all(const std::string& shellcode)
{
#ifdef SERVER
	ProtocolShellCode proto;
    compress_shellcode(shellcode, proto);
    MainWnd->SendToAll(proto);
#endif
}

void TaskShellcode::send(CONNID connid,const std::string& shellcode)
{
#ifdef SERVER
	ProtocolShellCode proto;
    compress_shellcode(shellcode, proto);
    json temp;
    proto.to_json(temp);
    Protocol package(temp);
	MainWnd->SendShellCode(connid, package);
#endif
}

void TaskShellcode::send_all_shell(CONNID connid)
{
#ifdef SERVER
    for(auto& proto : protocol_shell_code_list)
    {
		MainWnd->SendShellCode(connid, *proto);
    }
#endif
}


void TaskShellcode::trigger()
{
#ifdef SERVER
    CONNID m_current_connect_id = MainWnd->GetSelectedConnID();
    if(m_current_connect_id == -1) return;

    TCHAR path[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, path);
    CFileDialog file_dialog(TRUE, _T("bin"), path, 0, _T("云代码文件(*.bin)|*.bin|所有文件(*.*)|*.*||"), MainWnd);
    if (file_dialog.DoModal() != IDOK)
    {
        return;
    }
    std::string file = CT2A(file_dialog.GetPathName());
    std::string shellcode;
    read_shell_file(file, shellcode);
    if(shellcode.empty())
    {
        return;
    }
    send(m_current_connect_id, shellcode);
    /*Singleton<TaskPolicy>::getInstance().async_.delay_execute_shellcode([shellcode]() {
        Singleton<TaskShellcode>::getInstance().send_to_all(shellcode);
    }, 300);*/
    return;
#endif
}

// 读取Shell文件
void TaskShellcode::read_shell_file(const std::string& file,std::string& shellcode)
{
	std::ifstream bin(file, std::ios::in | std::ios::binary);
	if(bin.is_open() == false)
	{
		return;
	}
	bin.seekg(0, bin.end);
	size_t bin_size = (size_t)bin.tellg();
	bin.seekg(0);
	shellcode.resize(bin_size);
	bin.read((char*)shellcode.data(), bin_size);
	bin.close();
	return;
}


void TaskShellcode::on_time_proc(uint32_t curtime)
{

}
