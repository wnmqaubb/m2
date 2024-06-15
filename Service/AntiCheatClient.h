#pragma once

#include "TcpClient.h"
#include "Protocol.h"
#include "ServicePackage.h"
#include "SubServicePackage.h"
#include <filesystem>
#include <fstream>

using CTcpClientImpl = CTcpClient<RawProtocolImpl>;

class CAntiCheatClient : public CTcpClientImpl
{
public:
	using super = CTcpClientImpl;
	using timer_mgr_t = NetUtils::CTimerMgr;
    using package_handler_t = std::function<void(const RawProtocolImpl& package, const msgpack::v1::object_handle&)>;
    using notify_handler_t = std::function<void()>;
    CAntiCheatClient(asio::io_service& io) : super(io) 
    {
        heartbeat_duration_ = std::chrono::seconds(10);
        reconnect_duration_ = std::chrono::seconds(5);
        package_mgr_.register_handler(PKG_ID_S2C_HANDSHAKE, std::bind(&CAntiCheatClient::on_recv_handshake, this, std::placeholders::_1, std::placeholders::_2));
        package_mgr_.register_handler(PKG_ID_S2C_HEARTBEAT, std::bind(&CAntiCheatClient::on_recv_heartbeat, this, std::placeholders::_1, std::placeholders::_2));
    }
    virtual void start(const std::string& ip, unsigned short port)
    {
        is_stop_ = false;
        notify_mgr_.dispatch(CLIENT_START_NOTIFY_ID);
        super::start(ip, port);
    }
    virtual void stop()
    {
        super::stop();
        stop_timer(CLIENT_RECONNECT_TIMER_ID);
        is_stop_ = true;
    }
	virtual void on_connect(const std::error_code& ec)
    {
        if (!ec)
        {
            stop_timer(CLIENT_RECONNECT_TIMER_ID);
            notify_mgr_.dispatch(CLIENT_CONNECT_SUCCESS_NOTIFY_ID);
        }
        else
        {
            notify_mgr_.dispatch(CLIENT_CONNECT_FAILED_NOTIFY_ID);
            auto self(shared_from_this());
            start_timer(CLIENT_RECONNECT_TIMER_ID, reconnect_duration_, [self, this]() {
                super::start(get_address(), get_port());
            });
        }
    }
	virtual void on_disconnect(const std::error_code& ec)
    {
        auto self(shared_from_this());
        if (is_stop_ == false)
        {
            stop_timer(CLIENT_HEARTBEAT_TIMER_ID);
            start_timer(CLIENT_RECONNECT_TIMER_ID, reconnect_duration_, [self, this]() {
                super::start(get_address(), get_port());
            });
        }
        notify_mgr_.dispatch(CLIENT_DISCONNECT_NOTIFY_ID);
    }
	virtual void on_recv(asio::streambuf& buf, const std::error_code& ec, std::size_t length)
    {
        notify_mgr_.dispatch(CLIENT_ON_RECV_PACKAGE_NOTIFY_ID);
        RawProtocolImpl package;
        std::string_view sv((const char*)buf.data().data(), length);
        if (package.decode(sv) == false)
        {
            notify_mgr_.dispatch(PACKAGE_DECODE_ERROR_NOTIFY_ID);
            return;
        }
        auto raw_msg = msgpack::unpack((char*)package.body.buffer.data(), package.body.buffer.size());
        if (raw_msg.get().type != msgpack::type::ARRAY) throw msgpack::type_error();
        if (raw_msg.get().via.array.size < 1) throw msgpack::type_error();
        if (raw_msg.get().via.array.ptr[0].type != msgpack::type::POSITIVE_INTEGER) throw msgpack::type_error();
        const auto package_id = raw_msg.get().via.array.ptr[0].as<unsigned int>();
        if (!package_mgr_.dispatch(package_id, package, raw_msg))
            on_recv(package_id, package, raw_msg);
    }
    virtual void on_recv(unsigned int package_id, const RawProtocolImpl& package, const msgpack::v1::object_handle&) {};
    virtual void on_send(const std::error_code& ec, std::size_t length)
    {
        notify_mgr_.dispatch(CLIENT_ON_SEND_PACKAGE_NOTIFY_ID);
        if (ec)
        {
            notify_mgr_.dispatch(SEND_PACKAGE_ERROR_NOTIFY_ID);
        }
    }
    virtual void send(const std::string& text)
    {
        super::send(text);
    }
    virtual void send(msgpack::sbuffer& buffer, unsigned int session_id)
    {
        RawProtocolImpl raw_package;
        raw_package.encode(buffer.data(), buffer.size());
        raw_package.head.session_id = session_id;
        step_++;
        raw_package.head.step = step_;
        super::send(raw_package.release());
    }

    virtual void on_recv_handshake(const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg)
    {
        auto& msg = raw_msg.get().as<ProtocolS2CHandShake>();
        if (((asio2::uuid*)(msg.uuid))->hash() != uuid_.hash())
        {
            has_handshake_ = false;
            return;
        }
        has_handshake_ = true;
        notify_mgr_.dispatch(ON_RECV_HANDSHAKE_NOTIFY_ID);
    }
    virtual void on_recv_heartbeat(const RawProtocolImpl& package, const msgpack::v1::object_handle& raw_msg)
    {
        auto& msg = raw_msg.get().as<ProtocolS2CHeartBeat>();
        last_recv_hearbeat_time_ = std::chrono::system_clock::now();
        notify_mgr_.dispatch(ON_RECV_HEARTBEAT_NOTIFY_ID);
    }

    virtual void log(int type, LPCTSTR format, ...)
    {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lck(mtx);
        std::time_t now_time = time(0);
        TCHAR date_str[MAX_PATH] = { 0 };
        TCHAR time_str[MAX_PATH] = { 0 };
        tm tm_;
        localtime_s(&tm_, &now_time);
        wcsftime(time_str, sizeof(time_str) / sizeof(time_str[0]) - 1, TEXT("%H:%M:%S"), &tm_);
        wprintf(L"[%s]:", time_str);
        TCHAR buffer[1024];
        va_list ap;
        va_start(ap, format);
        _vsnwprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]) - 1, format, ap);
        va_end(ap);

        switch (type)
        {
        case LOG_TYPE_DEBUG:
            wprintf(L"[Debug]");
            break;
        case LOG_TYPE_EVENT:
            wprintf(L"[Event]");
            break;
        case LOG_TYPE_ERROR:
            wprintf(L"[Error]");
            break;
        default:
            break;
        }
        wprintf(TEXT("%s\n"), buffer);
        OutputDebugStringW(buffer);
    }

    void log_to_file(const std::string& identify, const std::string& text)
    {
        std::string log_file_name = identify.empty() ? "default.log" : identify + ".log";
        static std::mutex mtx;
        std::lock_guard<std::mutex> lck(mtx);
        std::time_t now_time = time(0);
        char date_str[MAX_PATH] = { 0 };
        char time_str[MAX_PATH] = { 0 };
        tm tm_;
        localtime_s(&tm_, &now_time);
        strftime(date_str, sizeof(date_str) / sizeof(date_str[0]) - 1, "%Y-%m-%d", &tm_);
        strftime(time_str, sizeof(time_str) / sizeof(time_str[0]) - 1, "%H:%M:%S", &tm_);

        std::filesystem::path file(std::filesystem::current_path() / "log" / date_str);
        if (!std::filesystem::exists(file))
        {
            std::filesystem::create_directories(file);
        }

        file = file / log_file_name;

        std::string result;
        result = result + "[Event]" + time_str + "|";
        std::ofstream output(file, std::ios::out | std::ios::app);

        result = result + text + "\n";

        output << result;
        output.close();
    }

	template <typename T>
	void send(T* package, unsigned int session_id = 0)
	{
		if (!package)
			__debugbreak();
		msgpack::sbuffer buffer;
		msgpack::pack(buffer, *package);
        send(buffer, session_id);
	}

    inline std::chrono::system_clock::duration& reconnect_duration() { return reconnect_duration_; }
    inline std::chrono::system_clock::duration& heartbeat_duration() { return heartbeat_duration_; }
    inline std::chrono::system_clock::time_point& last_recv_hearbeat_time() { return last_recv_hearbeat_time_; }
    inline bool has_handshake() { return has_handshake_; }
    inline asio2::uuid& uuid() { return uuid_; }
    inline bool is_stop() { return is_stop_; }
    inline NetUtils::UsersData& user_data() { return user_data_; }
    inline std::unique_ptr<ProtocolCFGLoader>& cfg() { return cfg_; }
    inline NetUtils::EventMgr<package_handler_t>& package_mgr() { return package_mgr_; }
    inline NetUtils::EventMgr<notify_handler_t>& notify_mgr() { return notify_mgr_; }
    inline unsigned char step() { return step_; }
protected:
    std::chrono::system_clock::duration reconnect_duration_;
	std::chrono::system_clock::duration heartbeat_duration_;
	std::chrono::system_clock::time_point last_recv_hearbeat_time_;
    bool has_handshake_ = false;
	asio2::uuid uuid_;
    bool is_stop_ = false;
    NetUtils::UsersData user_data_;
    NetUtils::EventMgr<package_handler_t> package_mgr_;
    NetUtils::EventMgr<notify_handler_t> notify_mgr_;
    std::unique_ptr<ProtocolCFGLoader> cfg_;
    unsigned char step_ = 0;
};
