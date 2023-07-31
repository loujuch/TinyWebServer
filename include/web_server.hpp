#ifndef _WEB_SERVER_HPP__
#define _WEB_SERVER_HPP__

#include <memory>
#include <atomic>

#include "log.hpp"
#include "http_connect_pool.hpp"

#include <stdint.h>

struct sockaddr_in;

namespace web {

class Accepter;

class WebServer {
	bool run_;
	uint16_t port_;
	std::atomic<uint32_t> max_conn_;
	std::atomic<uint32_t> now_conn_;
	std::unique_ptr<Accepter> accepter_;
	HttpConnectPool http_connect_pool_;

	bool accept_callback(int sock, const sockaddr_in &addr);
public:
	explicit WebServer(uint16_t port, uint32_t max_conn,
		const std::string &web_root, const std::string &server_name,
		const logger::LogLevel level);
	~WebServer();

	int run();

	inline void close_conn() {
		--now_conn_;
	}

	WebServer(const WebServer &) = delete;
	WebServer &operator=(const WebServer &) = delete;
}; // class WebServer

} // namespace web

#endif // _WEB_SERVER_HPP__