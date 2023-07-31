#ifndef _HTTP_CGI_HPP__
#define _HTTP_CGI_HPP__

#include <string>
#include <memory>
#include <vector>
#include <utility>

#include "socket_connect.hpp"

#include <unistd.h>

namespace ev {
class Channel;
class EventLoop;
} // namespace ev

namespace web {

class SocketConnect;

class HttpCGI {
	static const std::vector<std::pair<std::string, std::string>> env_var;

	pid_t pid_;
	bool error_;
	bool close_;
	int this_send_fork_recv_[2];
	int this_recv_fork_send_[2];
	std::shared_ptr<ev::Channel> recv_channel_;
	ev::EventLoop *own_event_loop_;
public:
	explicit HttpCGI(const std::string &exec_file,
		const std::string &method,
		const std::vector<std::pair<std::string, std::string>> &env_bar,
		ev::EventLoop *loop);
	~HttpCGI();

	int recv(char *buffer, int size);
	int send(const char *buffer, int size);

	void exec();

	inline static const std::vector<std::pair<std::string, std::string>> &env() {
		return env_var;
	}

	inline ev::Channel *cgi_channel() {
		return recv_channel_.get();
	}

	inline bool error() const {
		return error_;
	}

	inline bool is_close() const {
		return close_;
	}

	void close();

	HttpCGI(const HttpCGI &) = delete;
	HttpCGI &operator=(const HttpCGI &) = delete;
}; // class HttpCGI

} // namespace web

#endif // _HTTP_CGI_HPP__