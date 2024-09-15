#pragma once
#include <windows.h>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <Shlwapi.h>
#include <shared_mutex>
#include <thread>
LPCWSTR explain_action(DWORD dwAction);
class FileChangeNotifier {
protected:
	using dectect_handler_t = std::function<void(const std::wstring& filter_role, int action, const std::wstring& file_path)>;
public:
	FileChangeNotifier();
	FileChangeNotifier(const std::wstring& file_path, bool is_recursive, dectect_handler_t callback);
	~FileChangeNotifier();

	void start_directory_and_monitor(const std::wstring& path, bool is_recursive = false, dectect_handler_t callback = nullptr);
	void add_filter_role(const std::wstring& dectect_target_name, const std::wstring& filter_role);

private:
	void start_watching();
	void stop_watching();
	void watch_files();

	std::wstring file_path;
	std::thread watching_thread;
	bool stop_requested = false;
	std::shared_mutex mutex;
	const int buffer_size = 4096;

	HANDLE file_handle;
	OVERLAPPED overlapped;
	BYTE* buffer;
	std::unordered_map<std::wstring, FileChangeNotifier*> notifier_map_;
	bool is_recursive_ = false;
	dectect_handler_t callback_ = nullptr;
};