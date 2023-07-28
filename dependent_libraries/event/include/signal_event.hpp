#ifndef _SIGNAL_EVENT_HPP__
#define _SIGNAL_EVENT_HPP__

#include <memory>
#include <vector>

#include "callback.hpp"

#include <signal.h>

namespace ev {

class Channel;
class EventLoop;

class SignalEvent {
	int signo_;
	int event_fd_;
	EventLoop *own_event_loop_;
	std::unique_ptr<Channel> event_channel_;
	SignalCallback callback_;

	static std::vector<SignalEvent *> signal_event_;
	static void call_sig(int sig);
	void exec();
public:
	explicit SignalEvent(EventLoop *event_loop, int signo);
	~SignalEvent();

	inline SignalCallback setCallback(SignalCallback callback) {
		auto out = callback_;
		callback_ = callback;
		return out;
	}

	inline int signo() const {
		return signo_;
	}

	SignalEvent(const SignalEvent &) = delete;
	SignalEvent &operator=(const SignalEvent &) = delete;
};

} // namespace ev

#endif // _SIGNAL_EVENT_HPP__