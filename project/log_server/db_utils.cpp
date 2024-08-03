#include <iostream>
#include <chrono>
#include <mutex>
#include "protocol.h"
#include <asio2/asio2.hpp>
#include "utils.h"
#include "db_utils.h"
#define STATUS_CODE_SUCCESS 200

DBUtils& DBUtils::instance()
{
	static DBUtils instance_;
	return instance_;
}

bool DBUtils::login()
{
    asio2::error_code ec;
    http::request_t<http::string_body> req;
    req.method(http::verb::post);
    req.target(db_login_api_);
    req.set(http::field::user_agent, "Chrome");
    req.set(http::field::content_type, "text/html");
    req.set(http::field::host, host_);
    req.body() = "{\"attributes\":{\"email\":\"" + db_login_mail_ + "\", \"password\" : \""+ db_login_pwd_ +"\"}}";
    req.prepare_payload();
    auto resp = asio2::http_client::execute(host_, port_, req, ec);
    if(ec)
    {
        debug_log("%s %s", __FUNCTION__, ec.message().c_str());
        return false;
    }
    else
    {
        if(!resp.body().empty())
        {
            json resp_data = json::parse(resp.body());
            authorization_ = resp_data[0]["Attributes"]["value"];
            return true;
        }
    }
    return false;
}

void DBUtils::save_log(std::wstring& body, const std::string& db_op)
{
    if(authorization_.empty())
    {
        debug_log("token cannot be empty");
        return;
    }
    std::lock_guard<std::mutex> lck(mtx_);
    http::request_t<http::string_body> req;
    req.method(http::verb::post);
    req.target(db_op);
    req.set(http::field::user_agent, "Chrome");
    req.set(http::field::content_type, "application/json;charset=UTF-8");
    req.set(http::field::authorization, "Bearer " + authorization_);
    req.set(http::field::host, host_);
    req.body() = to_utf8(body);
    req.prepare_payload();
    http_client_.send(std::move(req), []() {
        debug_log("sent result : %d %s\n",
            asio2::last_error_val(), asio2::last_error_msg().c_str());
        });
}

void DBUtils::save_heartbeat_log(const ProtocolHeartBeat& proto)
{
    std::wstring body = parse_protocol_heartbeat(proto);

    save_log(body, db_insert_player_api_);
}

void DBUtils::save_normal_log(const ProtocolShellCodeInstance& proto, const std::wstring& username)
{
    std::wstring body = parse_protocol_normal(proto, username);

    save_log(body, db_insert_normal_api_);
}

void DBUtils::query_heartbeat_log()
{
    if(authorization_.empty())
    {
        debug_log("token cannot be empty");
        return;
    }
    asio2::error_code ec;
    http::request_t<http::string_body> req;
    req.method(http::verb::post);
    req.target("/api/ac_player_log");
    req.set(http::field::user_agent, "Chrome");
    req.set(http::field::content_type, "application/json;charset=UTF-8");
    req.set(http::field::authorization, "Bearer " + authorization_);
    req.set(http::field::host, host_);
    req.prepare_payload();

    auto resp = asio2::http_client::execute(host_, port_, req, ec);
    if(ec)
    {
        debug_log("%s %s", __FUNCTION__, ec.message().c_str());
        return;
    }
    else
    {
        if(!resp.body().empty())
        {
            debug_log(resp.body().c_str());
            return;
        }
    }

}


std::wstring DBUtils::parse_protocol_heartbeat(const ProtocolHeartBeat& proto)
{
    wchar_t body[1024] = {0};
    const wchar_t *body1 = 
        L"{\"data\":{\
            \"type\": \"ac_player_log\",\
            \"attributes\": {\
                \"userid\": \"%d\",\
                \"username\": \"%s\",\
                \"cpuid\": \"%s\",\
                \"volume\": \"%s\",\
                \"mac\": \"%s\",\
                \"pack_ip\": \"%s\"\
            }}\
        }";
    swprintf_s(body, body1,proto.id,proto.gamer_username.c_str(),proto.cpuid.c_str(),
        proto.volume_serial_number.c_str(),proto.mac_address.c_str(),proto.pack_ip.c_str());
    return std::move(body);
}

std::wstring DBUtils::parse_protocol_normal(const ProtocolShellCodeInstance& proto, const std::wstring& username)
{
    const wchar_t *empty_str = L"";
    wchar_t body[1024] = {0};
    const wchar_t *body1 =
        L"{\"data\":{\
            \"type\": \"ac_normal_log\",\
            \"attributes\": {\
                \"username\": \"%s\",\
                \"packageid\": \"%d\",\
                \"L1\": \"%s\",\
                \"L2\": \"%s\",\
                \"L3\": \"%s\",\
                \"L4\": \"%s\",\
                \"L5\": \"%s\",\
                \"L6\": \"%s\",\
                \"L7\": \"%s\",\
                \"L8\": \"%s\"\
            }}\
        }";

    swprintf_s(body, body1, username.c_str(), proto.id,
        empty_str, // L1
        empty_str, // L2
        empty_str, // L3
        empty_str, // L4
        empty_str, // L5
        empty_str, // L6
        empty_str, // L7
        empty_str);// L8
    return std::move(body);
}

DBUtils::DBUtils()
{
	http_client_.bind_connect([&](asio::error_code ec){
		if (asio2::get_last_error())
            debug_log("connect failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
            debug_log("connect success : %s %u\n",
				http_client_.local_address().c_str(), http_client_.local_port());
	}).bind_disconnect([](asio::error_code ec){
        debug_log("disconnect : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

    if(http_client_.start(host_, port_) && !login())
    {
        debug_log("login failed");
        return;
    }
}

DBUtils::~DBUtils()
{

}