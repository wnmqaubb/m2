#include "pch.h"
#include "task.h"
#include "utils/utils.h"
#include <corecrt_io.h>
#include <direct.h>
#include <time.h>
#include <filesystem>

#pragma warning(disable:4996)
TaskScreenShot::TaskScreenShot()
{
	set_interval(0);
	set_package_id(Protocol::PackageId::PACKAGE_ID_SCREENSHOT);
}

TaskScreenShot::~TaskScreenShot()
{

}

void TaskScreenShot::on_recv_package(void* sender, uintptr_t conn_id, json& package, Protocol::PackageType channel)
{
#ifdef SERVER
    ProtocolScreenShot proto;
    proto.from_json(package);
    Utils::log_to_file(conn_id, Utils::CHANNEL_EVENT, GlobalString::LOG::FORMAT_S.c_str(), proto.dump().c_str());
	if(proto.status != 0)
	{
		return;
	}
    std::vector<uint8_t> bitmap(proto.uncompress_size);
    DWORD bitmap_size = bitmap.size();
	int status = SYS_Uncompress(proto.data.data(), proto.data.size(), bitmap.data(), bitmap_size);
	if(status != 0)
	{
		return;
	}

	char guid_str[50];
	memset(guid_str, 0, sizeof(guid_str));
	GUID guid;
	if (S_OK == ::CoCreateGuid(&guid))
	{
		sprintf_s(guid_str, 50, "%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X"
			, guid.Data1
			, guid.Data2
			, guid.Data3
			, guid.Data4[0], guid.Data4[1]
			, guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5]
			, guid.Data4[6], guid.Data4[7]);
	}
	std::string file_name(guid_str);
	file_name.append(".jpg");

	std::time_t now_time;
	std::time(&now_time);
	char time_str[MAX_PATH];
	strftime(time_str, sizeof(time_str) - 1, "%Y-%m-%d", std::localtime(&now_time));
	std::filesystem::path file(std::filesystem::current_path() / "Íæ¼Ò½ØÍ¼");
	if(!std::filesystem::exists(file))
	{
		std::filesystem::create_directory(file);
	}
	file /= time_str;
	if(!std::filesystem::exists(file))
	{
		std::filesystem::create_directory(file);
	}
	file /= file_name;

	std::fstream output(file, std::ios::out | std::ios::binary | std::ios::trunc);
	output.write((char*)bitmap.data(), bitmap_size);
	output.close();
    if (!proto.silent)
    {
        MainWnd->m_screenshotdlg.m_screenshot_path = file.c_str();
        MainWnd->ShowScreenshotDlg();
    }
#endif
#ifdef CLIENT
    AntiCheat& anticheat = AntiCheat::instance();
    ProtocolScreenShot proto;
    proto.from_json(package);
    proto.m_type = channel;
    size_t bitmap_size;
    std::unique_ptr<unsigned char[]> screenshot(Utils::get_screenshot(&bitmap_size));
    if (!bitmap_size)
    {
        return;
    }

    //JPEG
    


    DWORD compress_buffer_size = SYS_GuessCompressBound(bitmap_size);
    std::unique_ptr<unsigned char[]> compress_buffer = std::make_unique<unsigned char[]>(compress_buffer_size);
    int status = SYS_Compress(screenshot.get(),
        bitmap_size,
        compress_buffer.get(),
        compress_buffer_size);
    if (status != 0)
    {
        proto.status = status;
        anticheat.send(proto);
        return;
    }

    proto.status = status;
    proto.uncompress_size = bitmap_size;
    std::vector<uint8_t> data(compress_buffer_size);
    memcpy(data.data(), compress_buffer.get(), compress_buffer_size);
    proto.data = data;
    anticheat.send(proto);
#endif
}
void TaskScreenShot::send(uintptr_t conn_id, bool silent)
{
#ifdef SERVER
    ProtocolScreenShot proto;
    proto.silent = silent;
    MainWnd->Send(conn_id, proto);
#endif
}
void TaskScreenShot::on_time_proc(uint32_t curtime)
{
#ifdef CLIENT

#endif
}

void TaskScreenShot::trigger()
{
#ifdef SERVER
	this->send_to_all();
#endif
}

void TaskScreenShot::send_to_all()
{
#ifdef SERVER
    ProtocolScreenShot proto;
	MainWnd->SendToAll(proto);
#endif
}