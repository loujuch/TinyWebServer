#ifndef _HTTP_CONNECT_HPP__
#define _HTTP_CONNECT_HPP__

#include <memory>
#include <string>

#include "timer_id.hpp"
#include "http_head_parser.hpp"

#include <netinet/in.h>

namespace ev {
class EventLoop;
class Channel;
}

namespace web {

class HttpFileSender;
class WebServer;

// 只能位于堆中，自身释放内存空间
class HttpConnect {
	int status;
	bool close_;
	int conn_sockfd_;
	WebServer *own_web_server_;
	HttpHeadParser head_parser_;
	ev::TimerId timer_id_;
	std::unique_ptr<ev::Channel> conn_channel_;
	std::unique_ptr<HttpFileSender> conn_sender_;
	ev::EventLoop *own_event_loop_;

	/**
	 * 以下事件只会被注册在一个EventLoop中
	 * 不涉及线程安全
	 * 但有可能会被乱序、重复调用
	 */
	void conn_read();
	void conn_write();
	void conn_close();
	void conn_error();
	void conn_timer();
	void conn_delete();

	void close();
	int receive(char *buffer, int size);
	void response();
public:
	HttpConnect(ev::EventLoop *loop, int sock,
		const sockaddr_in new_addr, WebServer *web);
	~HttpConnect();

	inline int socket() const {
		return conn_sockfd_;
	}
}; // class HttpConnect

} // namespace web

#endif // _HTTP_CONNECT_HPP__