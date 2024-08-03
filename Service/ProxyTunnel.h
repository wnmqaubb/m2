#pragma once

class ProxyTunnel : public std::enable_shared_from_this<ProxyTunnel>
{
	using resolver_t = asio::ip::tcp::resolver;
	using socket_t = asio::ip::tcp::socket;
	const int kBufSize = 1024;
public:
	ProxyTunnel(asio::io_service& io) : io_(io), socket_(io), buf_(kBufSize), is_connect_(false){};
	void start(const std::string& ip, unsigned short port)
	{
		ip_ = ip;
		port_ = port;
		do_connect();
	}
	void stop()
	{
		if (is_connect_)
		{
			socket_.close();
		}
	}
	virtual void on_connect(const std::error_code& ec) {}
	virtual void on_disconnect(const std::error_code& ec) {}
	virtual void on_recv(const std::vector<char>& buf, const std::error_code& ec, std::size_t length) {}
	virtual void on_send(const std::error_code& ec, std::size_t length) {}
	void send(std::string_view& data)
	{
		if (is_connect_)
		{
			auto self(shared_from_this());
			asio::async_write(socket_, asio::buffer(data), [self, this](std::error_code ec, std::size_t length) {
				on_send(ec, length);
			});
		}
	}
	const std::string& get_address() { return ip_; }
	unsigned short get_port() { return port_; }
protected:
	void do_receive()
	{
		auto self(shared_from_this());
		socket_.async_read_some(asio::buffer(buf_), [self, this](std::error_code ec, std::size_t length) {
			if (ec)
			{
				is_connect_ = false;
				socket_.close();
				on_disconnect(ec);
				return;
			}
			on_recv(buf_, ec, length);
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
			on_connect(ec);
		});
	}
private:
	asio::io_service& io_;
	socket_t socket_;
	std::string ip_;
	unsigned short port_;
	bool is_connect_;
	std::vector<char> buf_;
};
