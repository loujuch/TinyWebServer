#ifndef _EVENT_LOOP_HPP__
#define _EVENT_LOOP_HPP__

#include <memory>
#include <unordered_set>
#include <mutex>

#include "timer_id.hpp"
#include "callback.hpp"
#include "event.hpp"

namespace ev {

class Channel;
class Poller;
class TimerSet;
class TimerStamp;
class SignalSet;

class EventLoop {
	bool loop_;
	int wait_timeout_;
	int loop_result_;
	FreeCallback free_callback_;
	ChannelLists channel_list_;
	mutable std::mutex channel_mutex_;
	std::unique_ptr<Poller> poller_;
	std::unique_ptr<TimerSet> timer_set_;
	std::unique_ptr<SignalSet> signal_set_;
	std::unordered_set<Channel *> channel_set_;
public:
	EventLoop();
	~EventLoop();

	/**
	 * 信号事件操作要求线程安全
	*/
	// 设置信号事件监听与回调（可能会改变信号阻塞掩码）；返回该信号原句柄
	SignalCallback setSignal(int signo, SignalCallback signal_callback);
	// 删除信号事件监听与回调（可能会改变信号阻塞掩码）
	bool delSignal(int signo);
	// 是否有信号事件监听
	bool hasSignal(int signo);

	/**
	 * 定时器操作要求线程安全
	*/
	// 设置定时器事件监听与回调
	TimerId addAfterTimer(uint64_t delay, TimerCallback timer_callback_);
	// 设置定时器事件监听与回调
	TimerId addWhenTimer(const TimerStamp &time, TimerCallback timer_callback_);
	// 设置定时器事件监听与回调
	TimerId addEveryTimer(uint64_t interval, TimerCallback timer_callback_);
	// 关闭指定定时器监听
	bool closeTimer(TimerId &timer);
	// 推迟定时器
	bool updateTimer(TimerId &timer, uint64_t delay);

	/**
	 * IO事件操作要求线程安全
	*/
	// 设置IO事件监听与回调
	void setChannel(Channel *channel);
	// 移除IO事件监听与回调
	void delChannel(Channel *channel);
	// 判断是否有IO事件监听与回调
	bool hasChannel(Channel *channel) const;

	// 运行循环
	int run();

	// 终止循环
	inline void quit(int result) {
		loop_ = false;
		loop_result_ = result;
	}

	inline bool loop() {
		return loop_;
	}

	inline void setFreeCallback(FreeCallback free_callback) {
		free_callback_ = free_callback;
	}

	inline void setTimeOut(int time) {
		wait_timeout_ = time;
	}

	static EventLoop *this_thread_event_loop();

	EventLoop(const EventLoop &) = delete;
	EventLoop &operator=(const EventLoop &) = delete;
};

} // namespace ev

#endif // _EVENT_LOOP_HPP__