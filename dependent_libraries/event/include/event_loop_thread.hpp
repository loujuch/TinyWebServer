#ifndef _EVENT_LOOP_THREAD__
#define _EVENT_LOOP_THREAD__

#include <thread>
#include <memory>
#include <condition_variable>
#include <vector>

#include "channel.hpp"
#include "callback.hpp"

namespace ev {

class EventLoopThread {
	bool run_;
	EventLoop *event_loop_;
	ThreadInitCallback init_callback_;
	std::thread loop_thread_;
	std::condition_variable cond_;
	ChannelArgs args_;

	void func();
public:
	explicit EventLoopThread(ThreadInitCallback callback = ThreadInitCallback());

	EventLoop *run();

	inline bool joinable() {
		return loop_thread_.joinable();
	}

	inline void join() {
		loop_thread_.join();
	}

	inline void detach() {
		loop_thread_.detach();
	}

	EventLoopThread(const EventLoopThread &) = delete;
	EventLoopThread &operator=(const EventLoopThread &) = delete;
}; // class EventLoopThread

} // namespace ev

#endif // _EVENT_LOOP_THREAD__