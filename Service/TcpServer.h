#pragma once
#include <asio2/tcp/tcp_session.hpp>

class CTcpServer : public asio2::tcp_server
{
public:
	using tcp_session_t = session_type;
	using tcp_session_shared_ptr_t = std::shared_ptr<session_type>;
	using error_code_t = asio2::error_code;
    using super = asio2::tcp_server;

    CTcpServer() 
        : super(asio2::detail::tcp_frame_size)
    {
		bind_init(&CTcpServer::on_init, this);
		bind_recv(&CTcpServer::on_recv, this);
		bind_accept(&CTcpServer::on_accept, this);
		bind_connect(&CTcpServer::on_post_connect, this);
		bind_disconnect(&CTcpServer::on_post_disconnect, this);
		bind_start(&CTcpServer::on_start, this);
		bind_stop(&CTcpServer::on_stop, this);
    };
    virtual bool start(const std::string& listen_addr, int port) { return super::start(listen_addr, port); }
    virtual void stop() { super::stop(); }
    virtual void on_recv(tcp_session_shared_ptr_t& session, std::string_view sv) {};
    virtual void on_accept(tcp_session_shared_ptr_t& session) {};
    virtual void on_init() {};
    virtual void on_post_connect(tcp_session_shared_ptr_t& session) {};
    virtual void on_post_disconnect(tcp_session_shared_ptr_t& session) {};
    virtual void on_start() {};
    virtual void on_stop() {};
};

using CTcpServerImpl = CTcpServer;
using CTcpSessionSharedPtr = CTcpServerImpl::tcp_session_shared_ptr_t;