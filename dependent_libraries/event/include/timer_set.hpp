#ifndef _TIMER_SET_HPP__
#define _TIMER_SET_HPP__

#include <memory>
#include <vector>
#include <mutex>

#include "callback.hpp"

namespace ev {

class Channel;
class EventLoop;
class TimerHeap;
class TimerId;
class TimerStamp;

// 要求线程安全
class TimerSet {
	int timer_fd_;
	EventLoop *owner_event_loop_;
	mutable std::mutex timer_mutex_;
	std::unique_ptr<Channel> timer_channel_;
	std::unique_ptr<TimerHeap> timer_heap_;

	bool exec();
public:
	explicit TimerSet(EventLoop *event_loop);
	~TimerSet();

	TimerId addTimer(const TimerStamp &when, uint64_t interval, TimerCallback timer_callback);
	bool closeTimer(TimerId &id);
	bool updateTimer(TimerId &timer, uint64_t delay);

	TimerSet(const TimerSet &) = delete;
	TimerSet &operator=(const TimerSet &) = delete;
}; // class TimerSet

} // namespace ev

#endif // _TIMER_SET_HPP__