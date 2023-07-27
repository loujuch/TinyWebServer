#ifndef _SIGNAL_SET_HPP__
#define _SIGNAL_SET_HPP__

#include <memory>
#include <mutex>
#include <vector>

#include "callback.hpp"

#include <signal.h>

namespace ev {

class Channel;
class EventLoop;
class SignalEvent;

class SignalSet {
	EventLoop *own_event_loop_;
	sigset_t loop_set_;
	mutable std::mutex signal_mutex_;

	static std::vector<std::unique_ptr<SignalEvent>> signal_set_;

	void exec();
public:
	SignalSet(EventLoop *event_loop);
	~SignalSet();

	SignalCallback setSignal(int signo, SignalCallback signal_callback);
	bool delSignal(int signo);

	inline bool hasSignal(int signo) {
		bool out;
		signal_mutex_.lock();
		out = (sigismember(&loop_set_, signo) == 1);
		signal_mutex_.unlock();
		return out;
	}

	SignalSet(const SignalSet &) = delete;
	SignalSet &operator=(const SignalSet &) = delete;
}; // namespace ev

} // namespace ev

#endif // _SIGNAL_SET_HPP__