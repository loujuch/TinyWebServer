#ifndef _TIMER_HEAP_HPP__
#define _TIMER_HEAP_HPP__

#include <vector>
#include <memory>

#include "callback.hpp"
#include "timer.hpp"
#include "timer_stamp.hpp"

namespace ev {

class TimerId;

class TimerHeap {
	std::vector<std::shared_ptr<Timer>> timer_heap_;

	void from_to(int l, int r);
public:
	explicit TimerHeap() = default;

	bool push(std::shared_ptr<Timer> &timer);
	TimerId emplace(const TimerStamp &when, uint64_t interval, TimerCallback timer_callback);
	const std::shared_ptr<Timer> top() const;
	void pop();
	bool erase(Timer *timer);
	bool updateTimer(Timer *timer, uint64_t delay);

	inline bool has(const Timer *timer) const {
		return timer->seq() >= 0 && timer->seq() < timer_heap_.size();
	}

	inline uint64_t size() const {
		return timer_heap_.size();
	}

	inline bool empty() const {
		return timer_heap_.empty();
	}

	TimerHeap(const TimerHeap &) = delete;
	TimerHeap &operator=(const TimerHeap &) = delete;
}; // class TimerHeap

} // namespace ev

#endif // _TIMER_HEAP_HPP__