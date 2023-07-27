#ifndef _TIMER_ID_HPP__
#define _TIMER_ID_HPP__

#include <stdint.h>

namespace ev {

class Timer;

class TimerId {
	Timer *own_timer_;
public:
	explicit TimerId(Timer *timer = nullptr);
	TimerId(const TimerId &) = default;

	friend class TimerSet;
}; // class TimerId

} // namespace ev

#endif // _TIMER_ID_HPP__