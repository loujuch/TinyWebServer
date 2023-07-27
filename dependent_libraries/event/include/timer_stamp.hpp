#ifndef _TIMER_STAMP_HPP__
#define _TIMER_STAMP_HPP__

#include <stdint.h>

namespace ev {

// 毫秒级事件戳(应用于计时器精度需求低的场合，过高的精度可能会导致进行排序时产生更多无意义的比较)
class TimerStamp {
	static constexpr uint64_t ms_to_ns = 1000000;
	static constexpr uint64_t s_to_ms = 1000;
	uint64_t time_;
public:
	explicit TimerStamp(uint64_t ms = 0);
	TimerStamp(const TimerStamp &time);

	static TimerStamp now();

	inline uint64_t sec() const {
		return time_ / s_to_ms;
	}

	inline uint64_t msec() const {
		return time_ % s_to_ms;
	}

	inline void delay_time(uint64_t ms) {
		time_ += ms;
	}

	inline void swap(TimerStamp &time) {
		uint64_t tmp = this->time_;
		this->time_ = time.time_;
		time.time_ = tmp;
	}

	inline TimerStamp &operator=(const TimerStamp &time) {
		this->time_ = time.time_;
		return *this;
	}

	inline bool operator<(const TimerStamp &time) const {
		return this->time_ < time.time_;
	}

	inline bool operator>(const TimerStamp &time) const {
		return this->time_ > time.time_;
	}

	bool operator==(const TimerStamp &time) const {
		return this->time_ == time.time_;
	}

	inline bool operator<=(const TimerStamp &time) const {
		return this->time_ <= time.time_;
	}
}; // class TimerStamp

} // namespace ev

#endif // _TIMER_STAMP_HPP__