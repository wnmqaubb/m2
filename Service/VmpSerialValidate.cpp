#include "pch.h"
#include "VmpSerialValidate.h"
#include "3rdparty/vmprotect/VMProtectSDK.h"
#include <asio2/util/base64.hpp>
#include <asio2/util/sha1.hpp>

VmpSerialValidator::VmpSerialValidator(CObserverServer* ac_server)
{
    server_ = ac_server;
    write_hwid();
}
bool VmpSerialValidator::validate_timer(bool slience)
{
    VMProtectBeginVirtualization(__FUNCTION__);
    bool status = false;
#ifdef _DEBUG
    server()->set_auth_ticket("");
    server()->auth_success();
    server_->get_vmp_expire() = TEXT("测试");;
#else
    is_multi_serial_ = false;
    server_->get_vmp_expire() = L"";
    std::filesystem::path file(std::filesystem::current_path() / "serial.txt");
    if (!std::filesystem::exists(file))
    {
        MessageBox(NULL, TEXT("授权文件不存在,请联系客服!"), TEXT("提示"), MB_ICONERROR | MB_SYSTEMMODAL);
        server()->logic_client_stop();
        server()->stop();
        return false;
    }
    auto sn = read_license(file.string());
    std::string ticket;
    status = validate(sn, slience, ticket);
    // 识别多个KEY~就是转发的那个功能,应用场景:多个服的网关专门放到一个服务器上,共用一个网关
    std::filesystem::path file1(std::filesystem::current_path() / "serial1.txt");
    uint8_t i = 2;
    bool multi_serial_status = false;
    while (std::filesystem::exists(file1))
    {
        is_multi_serial_ = true;
        if (validate(read_license(file1.string()), slience, ticket))
        {
            multi_serial_status = true;
        }
        else
        {
            multi_serial_status = false;
            break;
        }
        file1 = std::filesystem::current_path() / ("serial" + std::to_string(i++) + ".txt");
    }

    //多个serial--必须所有serial都验证通过
    status = is_multi_serial_ ? (multi_serial_status && status) : status;

    if (status)
    {
        server()->set_auth_ticket(ticket);
        server()->auth_success();
    }
    else
    {
        server()->auth_fail();
    }
#endif;
    VMProtectEnd();
    return status;
}


bool VmpSerialValidator::validate(const std::string& sn, bool slience, std::string& ticket)
{
    VMProtectBeginVirtualization(__FUNCTION__);
    int res = VMProtectSetSerialNumber(sn.c_str());
    if (res)
    {
        std::wstring vmp_text = L"";
        switch (res)
        {
        case SERIAL_STATE_FLAG_CORRUPTED:
            vmp_text = L"许可系统已损坏。可能的原因是：被恶意破解。";
            break;
        case SERIAL_STATE_FLAG_INVALID:
            vmp_text = L"请输入有效的序列号";
            break;
        case SERIAL_STATE_FLAG_BLACKLISTED:
            vmp_text = L"序列号与产品匹配,但已冻结";
            break;
        case SERIAL_STATE_FLAG_DATE_EXPIRED:
            vmp_text = L"序列号已过期。";
            break;
        case SERIAL_STATE_FLAG_RUNNING_TIME_OVER:
            vmp_text = L"该程序的运行时间已用完。";
            break;
        case SERIAL_STATE_FLAG_BAD_HWID:
            vmp_text = L"硬件标识符与密钥中指定的硬件标识符不匹配。";
            break;
        case SERIAL_STATE_FLAG_MAX_BUILD_EXPIRED:
            vmp_text = L"序列号与受保护程序的当前版本不匹配。";
            break;
        default:
            vmp_text = L"序列号错误，请联系客服。";
            break;
        }
        if (slience)
            server()->log(LOG_TYPE_ERROR, vmp_text.c_str());
        else
            MessageBox(NULL, vmp_text.c_str(), TEXT("提示"), MB_OK);
            server()->log(LOG_TYPE_ERROR, vmp_text.c_str());
        return false;
    }

    // 验证绑定的IP是否与序列号的IP一致
    VMProtectSerialNumberData sd = { 0 };
    VMProtectGetSerialNumberData(&sd, sizeof(sd));
    asio2::base64 base64;
    auto sn_sha1 = asio2::sha1(base64.decode(sn));
    ticket = base64.encode((unsigned char*)&sn_sha1, sizeof(sn_sha1));
    server()->log(LOG_TYPE_DEBUG, TEXT("sn_hash:%s"), Utils::c2w(ticket).c_str());

    // 验证序列号是否已冻结
    int status = http_query_sn_status(sn);
    if (status == -1)
    {
        server()->log(LOG_TYPE_ERROR, TEXT("网络异常,验证序列号失败!"));
        return false;
    }
    if (status > 0)
    {
        server()->log(LOG_TYPE_ERROR, TEXT("序列号已冻结"));
        return false;
    }

    wchar_t vmp_expire_t[50] = { 0 };
    swprintf_s(vmp_expire_t, TEXT("%d-%d-%d"), sd.dtExpire.wYear, sd.dtExpire.bMonth, sd.dtExpire.bDay);
    if (server_->get_vmp_expire().empty())
    {
        server_->get_vmp_expire() = vmp_expire_t;
    }
    server_->log(LOG_TYPE_EVENT, (TEXT("验证成功，到期时间:") + server_->get_vmp_expire()).c_str());
    VMProtectEnd();
    return true;
}

std::string VmpSerialValidator::read_license(const std::string& path)
{
    std::string serial(
        std::istreambuf_iterator<char>(std::ifstream(path) >> std::skipws),
        std::istreambuf_iterator<char>());
    std::find_if(serial.begin(), serial.end(), [&serial](char f)->bool {
        if (f == '\r' || f == '\n')
        {
            serial.erase(serial.find(f), 1);
        }
        return false;
    });
    return serial;
}

int VmpSerialValidator::http_query_sn_status(const std::string& sn)
{
	VMProtectBeginVirtualization(__FUNCTION__);
	try
	{
		const std::string host = "43.139.236.115";
		const int port = 5178;
		asio2::base64 base64;
        asio2::http_client http_client;
		auto base64_sn = base64.encode((const unsigned char*)sn.data(), sn.size());
		http::web_request req;
		req.method(http::verb::post);
		req.target("/serialstatus.php");
		req.set(http::field::user_agent, "Chrome");
		req.set(http::field::content_type, "application/x-www-form-urlencoded");
		req.body() = http::url_encode("serial=" + base64_sn);
		req.prepare_payload();
		auto rep = asio2::http_client::execute(host, port, req);
		if (asio2::get_last_error()) {
			return -1;
		}
		else {
			return atoi(rep.body().c_str());
		}
	}
	catch (...)
	{
		return -1;
	}
	VMProtectEnd();
}
//#pragma optimize( "", on )

void VmpSerialValidator::write_hwid()
{
    VMProtectBeginVirtualization(__FUNCTION__);
    int sz = VMProtectGetCurrentHWID(NULL, 0);
    std::string buf;
    buf.resize(sz);
    VMProtectGetCurrentHWID((char*)buf.c_str(), sz);
    std::filesystem::path file(std::filesystem::current_path() / "HWID.txt");
    std::ofstream output(file, std::ios::trunc);
    output << buf.c_str();
    output.flush();
    output.close();
    VMProtectEnd();
}