#ifndef _TIMER_HPP__
#define _TIMER_HPP__

#include "timer_stamp.hpp"
#include "callback.hpp"

namespace ev {

class Timer {
	bool run_;
	int64_t seq_;
	TimerStamp timer_stamp_;
	uint64_t interval_;
	TimerCallback timer_callback_;
public:
	explicit Timer(const TimerStamp &when, uint64_t interval, TimerCallback timer_callback);
	Timer(const Timer &) = default;

	friend class TimerHeap;

	inline int64_t seq() const {
		return seq_;
	}

	inline void close() {
		run_ = false;
	}

	void exec();

	inline bool has_next() const {
		return interval_ != 0 && run_;
	}

	inline void next() {
		timer_stamp_.delay_time(interval_);
	}

	inline uint64_t interval() {
		return interval_;
	}

	inline const TimerStamp &timer_stamp() const {
		return timer_stamp_;
	}

	inline bool operator<(const Timer &time) const {
		return this->timer_stamp_ < time.timer_stamp_;
	}

	inline bool operator>(const Timer &time) const {
		return this->timer_stamp_ > time.timer_stamp_;
	}

	bool operator==(const Timer &time) const {
		return this->timer_stamp_ == time.timer_stamp_;
	}
}; // class Timer

} // namespace ev

#endif // _TIMER_HPP__