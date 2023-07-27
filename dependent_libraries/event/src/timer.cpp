#include "timer.hpp"
#include "log.hpp"

ev::Timer::Timer(const TimerStamp &when, uint64_t interval, TimerCallback timer_callback) :
	run_(true),
	seq_(-1),
	timer_stamp_(when),
	interval_(interval),
	timer_callback_(timer_callback) {
}

void ev::Timer::exec() {
	logger::log_trace << "timer " << this << " exec";
	if(timer_callback_ && run_) {
		timer_callback_();
	} else {
		if(!run_) {
			logger::log_warn << "timer " << this << " had close";
		} else {
			logger::log_warn << "timer " << this << " callback is empty";
		}
	}
}