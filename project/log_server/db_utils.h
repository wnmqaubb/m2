#pragma once

class DBUtils
{
public:
	static DBUtils& instance();
    bool login();
    void save_heartbeat_log(const ProtocolHeartBeat& proto);
    void save_normal_log(const ProtocolShellCodeInstance& proto, const std::wstring& username);
    void query_heartbeat_log();
private:
    void save_log(std::wstring & body, const std::string & db_op);
    std::wstring parse_protocol_heartbeat(const ProtocolHeartBeat & proto);
    std::wstring parse_protocol_normal(const ProtocolShellCodeInstance & proto, const std::wstring& username);

    DBUtils();
    ~DBUtils();


    const std::string db_login_mail_ = "root@qq.com";
    const std::string db_login_pwd_ = "3d9EvDkeF23jLe";
    const std::string db_login_api_ = "/action/user_account/signin";
    const std::string db_insert_player_api_ = "/api/ac_player_log";
    const std::string db_insert_normal_api_ = "/api/ac_normal_log";
    const std::string_view host_ = "103.114.100.16";// "127.0.0.1";
    const std::string_view port_ = "6336";
    std::string authorization_;
	std::mutex mtx_;
    asio2::http_client http_client_;
};