#ifndef _HTTP_CONNECT_HPP__
#define _HTTP_CONNECT_HPP__

#include <memory>
#include <string>

#include "timer_id.hpp"
#include "http_head_parser.hpp"
#include "socket_connect.hpp"

#include <netinet/in.h>

namespace ev {
class EventLoop;
class Channel;
}

namespace web {

class HttpFileSender;
class HttpCGI;
class HttpConnectPool;

// 只能位于堆中，自身释放内存空间
class HttpConnect {
	static constexpr int timeout = 1000000;

	int group_;
	bool close_;
	HttpConnectPool *pool_;
	SocketConnect conn_sockfd_;
	HttpHeadParser head_parser_;
	ev::TimerId timer_id_;
	std::unique_ptr<ev::Channel> conn_channel_;
	std::unique_ptr<HttpFileSender> conn_sender_;
	std::shared_ptr<HttpCGI> conn_cgi_;
	ev::EventLoop *own_event_loop_;

	/**
	 * 以下事件只会被注册在一个EventLoop中
	 * 不涉及线程安全
	 * 但有可能会被乱序、重复调用
	 */
	bool conn_read();
	bool conn_write();
	bool conn_close();
	bool conn_error();
	void conn_timer();

	bool cgi_read();
	bool cgi_close();
	bool cgi_error();

	bool close();
	int set_cgi(const std::string &file_name);
	bool send_error_short_file(int error);
public:
	explicit HttpConnect(ev::EventLoop *loop, SocketConnect &&sock,
		const sockaddr_in new_addr, HttpConnectPool *pool, int group);
	~HttpConnect();

	void exec();

	inline int socket() const {
		return conn_sockfd_.socket();
	}

	inline int group() const {
		return group_;
	}

	HttpConnect(const HttpConnect &) = delete;
	HttpConnect &operator=(const HttpConnect &) = delete;
}; // class HttpConnect

} // namespace web

#endif // _HTTP_CONNECT_HPP__