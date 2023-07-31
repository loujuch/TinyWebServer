#ifndef _HTTP_CONNECT_POOL__
#define _HTTP_CONNECT_POOL__

#include <vector>
#include <memory>
#include <unordered_set>
#include <mutex>

#include <stdint.h>

struct sockaddr_in;

namespace ev {
class EventLoop;
class EventLoopThread;
}

namespace web {

class HttpConnect;
class WebServer;
class SocketConnect;

// 一个服务器中只包含一个，用于分发连接事件
class HttpConnectPool {
	static constexpr uint16_t timeout = 1000 * 30;
	int now;
	std::vector<std::mutex> connect_set_mutex_;
	std::vector<std::unordered_set<HttpConnect *>> connect_set_;
	std::vector<std::unique_ptr<ev::EventLoopThread>> event_loop_thread_set_;
	std::vector<ev::EventLoop *> event_loop_set_;
public:
	explicit HttpConnectPool(uint32_t connect_number);
	~HttpConnectPool();

	// 仅被用于Accepter中，因此实际上不涉及多线程调用，无需保证线程安全
	void add_connect(SocketConnect &&new_sock, const sockaddr_in &new_addr);
	void del_connect(HttpConnect *http_connect);

	HttpConnectPool(const HttpConnectPool &) = delete;
	HttpConnectPool &operator=(const HttpConnectPool &) = delete;
}; // class HttpConnectPool

} // namespace web

#endif // _HTTP_CONNECT_POOL__