#include "../pch.h"
#include <Lightbone/utils.h>
#include "Service/AntiCheatClient.h"
#include "anti_monitor_directory/ReadDirectoryChanges.h"
#include "ClientImpl.h"
extern std::shared_ptr<asio::io_service> g_game_io;
extern std::shared_ptr<CClientImpl> client_;

void report_monitor_directory(bool is_cheat, const std::string& reason)
{
    VMP_VIRTUALIZATION_BEGIN();
    static uint32_t last_tick_count = NULL;
    if (GetTickCount() - last_tick_count >= 200)
    {
        ProtocolC2STaskEcho resp;
        resp.task_id = 689066;
        resp.is_cheat = true;
        resp.text = xorstr("�������:") + reason;
        client_->send(&resp);
        last_tick_count = GetTickCount();
    }
    VMP_VIRTUALIZATION_END();
}

void InitDirectoryChangsDetect()
{
	wchar_t* user_profile = nullptr;
	size_t len = 0;
	extern std::unique_ptr<FileChangeNotifier> notifier;
	notifier = std::make_unique<FileChangeNotifier>();
	notifier->add_filter_role(L"wpe", L"WPE.INI");
	notifier->add_filter_role(L"CE", L"ADDRESSES.FIRST");
	notifier->add_filter_role(L"CE", L"MEMORY.FIRST");

	if (_wdupenv_s(&user_profile, &len, L"USERPROFILE") == 0 && user_profile != nullptr) {
		notifier->start_directory_and_monitor(user_profile, true, [](const std::wstring& filter_role, int action, const std::wstring& file_path) {

			report_monitor_directory(true, Utils::String::w2c(filter_role + L"|" + explain_action(action) + L"|" + file_path));

		});
		free(user_profile);  // �ͷ��ڴ�
	}
	
	/*notifier->start_directory_and_monitor(L"D:\\work\\temp\\2024", true, [client_](const std::wstring& filter_role, int action, const std::wstring& file_path) {

		report_monitor_directory(client_, true, Utils::String::w2c(filter_role + L"|" + explain_action(action) + L"|" + file_path,CP_UTF8));

		});*/

	



    
}