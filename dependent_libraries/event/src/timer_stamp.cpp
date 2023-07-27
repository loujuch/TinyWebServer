#include "timer_stamp.hpp"

#include <time.h>

ev::TimerStamp::TimerStamp(uint64_t ms) :
	time_(ms) {
}

ev::TimerStamp::TimerStamp(const TimerStamp &time) :
	time_(time.time_) {
}

ev::TimerStamp ev::TimerStamp::now() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return TimerStamp(ts.tv_sec * s_to_ms + ts.tv_nsec / ms_to_ns);
}