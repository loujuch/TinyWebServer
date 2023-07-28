#ifndef _ACCEPTER_HPP__
#define _ACCEPTER_HPP__

#include <memory>
#include <functional>

struct sockaddr_in;

namespace ev {
class Channel;
class EventLoop;
}

namespace web {

using AcceptCallback = std::function<void(int, const sockaddr_in &)>;

// 一个服务器中只包含一个，用于响应连接事件
class Accepter {
	static constexpr uint16_t timeout = 1000 * 30;
	int listen_fd_;
	std::unique_ptr<ev::EventLoop> listen_event_loop_;
	std::unique_ptr<ev::Channel> listen_channel_;
	AcceptCallback accept_callback_;

	void free_callback();
	void listen_error();
	void listen_accept();
public:
	explicit Accepter(uint16_t port);
	~Accepter();

	inline ev::EventLoop *event_loop() {
		return listen_event_loop_.get();
	}

	int run(AcceptCallback callback);

	Accepter(const Accepter &) = delete;
	Accepter &operator=(const Accepter &) = delete;
};

} // namespace web

#endif // _ACCEPTER_HPP__