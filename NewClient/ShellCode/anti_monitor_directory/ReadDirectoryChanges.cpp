#include "ReadDirectoryChanges.h"
#include <iostream>
#include <Shlwapi.h> 

// 要监控的程序名字和路径
static std::unordered_map <std::wstring, std::unordered_set<std::wstring>> filtered_role_map_;
extern HANDLE dll_exit_event_handle_;
LPCWSTR explain_action(DWORD dwAction)
{
	switch (dwAction)
	{
		case FILE_ACTION_ADDED:
			return L"Added";
		case FILE_ACTION_REMOVED:
			return L"Deleted";
		case FILE_ACTION_MODIFIED:
			return L"Modified";
		case FILE_ACTION_RENAMED_OLD_NAME:
			return L"Renamed From";
		case FILE_ACTION_RENAMED_NEW_NAME:
			return L"Renamed To";
		default:
			return L"BAD DATA";
	}
}

FileChangeNotifier::FileChangeNotifier() {}

FileChangeNotifier::FileChangeNotifier(const std::wstring& file_path, bool is_recursive, dectect_handler_t callback)
	: file_path(file_path), is_recursive_(is_recursive), callback_(callback), file_handle(INVALID_HANDLE_VALUE) {
	start_watching();
}

FileChangeNotifier::~FileChangeNotifier() {
	stop_watching();
	if (file_handle && file_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(file_handle);
	}	
}

void FileChangeNotifier::start_watching() {
	watching_thread = std::thread(&FileChangeNotifier::watch_files, this);
}

void FileChangeNotifier::start_directory_and_monitor(const std::wstring& path, bool is_recursive, dectect_handler_t callback) {
	std::unique_lock <std::shared_mutex> lck(mutex);
	if (notifier_map_.find(path) == notifier_map_.end()) {
		notifier_map_.emplace(std::make_pair(path, new FileChangeNotifier(path, is_recursive, callback)));
	}
}

void FileChangeNotifier::add_filter_role(const std::wstring& dectect_target_name, const std::wstring& filter_role) {
	std::unique_lock <std::shared_mutex> lck(mutex);
	if (filtered_role_map_.find(dectect_target_name) == filtered_role_map_.end()) {
		filtered_role_map_.emplace(std::make_pair(dectect_target_name, std::unordered_set<std::wstring>({ filter_role })));
	}
	else {
		filtered_role_map_[dectect_target_name].insert(filter_role);
	}
}

void FileChangeNotifier::stop_watching() {
	stop_requested = true;
	if (watching_thread.joinable()) {
		watching_thread.join();
	}
}

void FileChangeNotifier::join_all() {
	for (auto& fcn : notifier_map_) {
		fcn.second->stop_watching();
	}
}

void FileChangeNotifier::watch_files() {
	file_handle = CreateFileW(file_path.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		nullptr);
	if (file_handle == INVALID_HANDLE_VALUE) {
		std::wcerr << L"Error opening file: " << file_path.c_str() << std::endl;
	}

	buffer = new BYTE[buffer_size];
	ZeroMemory(&overlapped, sizeof(OVERLAPPED));
	overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	std::unique_lock<std::shared_mutex> lock(mutex);
	while (!stop_requested/* && WaitForSingleObject(dll_exit_event_handle_, 0) == WAIT_TIMEOUT*/) {
		DWORD bytes_returned;
		BOOL success;
		
		success = ReadDirectoryChangesW(file_handle, buffer, buffer_size, is_recursive_,
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME,
			&bytes_returned, &overlapped, nullptr);

		if (!success && GetLastError() != ERROR_IO_PENDING) {
			std::wcerr << L"Error in ReadDirectoryChangesW for file: " << file_path.c_str() << std::endl;
			continue;
		}

		DWORD wait_result = WaitForSingleObject(overlapped.hEvent, INFINITE);
		if (wait_result == WAIT_OBJECT_0) {
			BYTE* current_ptr = buffer;
			while (true) {
				FILE_NOTIFY_INFORMATION* notify_info = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(current_ptr);
				std::wstring file_name(notify_info->FileName, notify_info->FileNameLength / sizeof(wchar_t));
				std::wstring full_file_path = file_path + L"\\" + file_name;
				file_name = PathFindFileNameW(full_file_path.c_str());
				for (const auto& pair : filtered_role_map_) {
					if (pair.second.find(file_name) != pair.second.end()) {
#ifdef LOG_SHOW
						wprintf(L"role [%s] %s file: %s\n", pair.first.c_str(), explain_action(notify_info->Action), full_file_path.c_str());
#endif
						if (callback_) {
							callback_(pair.first, notify_info->Action, full_file_path);
						}
						break;
					}
				}
				//wprintf(L"%s file: %s\n", explain_action(notify_info->Action), full_file_path.c_str());
				if (notify_info->NextEntryOffset == 0) {
					break;
				}
				current_ptr += notify_info->NextEntryOffset;
			}
		}
	}
	CloseHandle(file_handle);
	CloseHandle(overlapped.hEvent);
	delete[] buffer;
}