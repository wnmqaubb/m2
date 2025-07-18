#pragma once
#include <asio2\asio2.hpp>
#include "NetUtils.h"


template <class MatchCondition>
class CTcpClient : public std::enable_shared_from_this<CTcpClient<MatchCondition>>,
    public NetUtils::CTimerMgr
{
	using resolver_t = asio::ip::tcp::resolver;
	using socket_t = asio::ip::tcp::socket;
public:
	CTcpClient(asio::io_service& io) : io_(io), socket_(io), is_connect_(false), CTimerMgr(io) {};
    ~CTcpClient() { stop(); }
    inline asio::io_service& io() { return io_; }
    inline bool is_connected() const { return is_connect_; }
    virtual void start(const std::string& ip, unsigned short port)
	{
        if (is_connect_ == false)
        {
            ip_ = ip;
            port_ = port;
            do_connect();
        }
	}
	virtual void stop()
	{
        if (is_connect_)
        {
            is_connect_ = false; 
			if (socket_.is_open()) {
				socket_.cancel();
				socket_.shutdown(socket_t::shutdown_both);
				socket_.close();
			}
			//stop_all_timer();
        }
	}
	virtual void send(const char* buf, std::size_t size)
	{
        std::lock_guard<std::mutex> lck(send_mtx_);
		if (is_connect_)
		{
            size_t length = 0;
            try
            {
                length = asio::write(socket_, asio::buffer(buf, size));
                asio::error_code ec;
                on_send(ec, length);
            }
            catch (asio::error_code ec)
            {
                on_send(ec, length);
            }
		}
	}
    
	void send(const std::string& text) { send(text.data(), text.size()); }
	const std::string& get_address() { return ip_; }
	unsigned short get_port() { return port_; }
private:
    virtual void on_connect([[maybe_unused]] const std::error_code& ec) {}
	virtual void on_disconnect([[maybe_unused]] const std::error_code& ec) {}
	virtual void on_recv([[maybe_unused]] asio::streambuf& buf, [[maybe_unused]] const std::error_code& ec, [[maybe_unused]] std::size_t length) {}
	virtual void on_send([[maybe_unused]] const std::error_code& ec, [[maybe_unused]] std::size_t length) {}
protected:
	virtual void do_receive()
	{
		auto self(shared_from_this());
		asio::async_read_until(socket_, buf_, MatchCondition(), [self, this](const std::error_code& ec, std::size_t length) {
            if (ec)
            {
                on_disconnect(ec);
                CTcpClient::stop();
                return;
            }
            on_recv(buf_, ec, length);
            buf_.consume(length);
			do_receive();
		});
	}
	void do_connect()
	{
		auto self(shared_from_this());
		asio::ip::tcp::endpoint ep(asio::ip::address::from_string(ip_), port_);
		socket_.async_connect(ep, [self, this](const std::error_code& ec) {
			if (!ec)
			{
                is_connect_ = true;
                on_connect(ec);
				do_receive();
				return;
			}
			else
			{
                is_connect_ = false;
                on_connect(ec);
			}
		});
	}

protected:
	asio::io_service& io_;
	socket_t socket_;
	std::string ip_;
	unsigned short port_;
	bool is_connect_;
	asio::streambuf buf_;
    std::mutex send_mtx_;
};
