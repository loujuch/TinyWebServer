#include "timer_set.hpp"

#include "channel.hpp"
#include "event_loop.hpp"
#include "timer_heap.hpp"
#include "timer.hpp"
#include "timer_stamp.hpp"
#include "timer_id.hpp"
#include "log.hpp"

#include <sys/timerfd.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

static void set_timer_from_timestamp(int timer_fd, const ev::TimerStamp &timerstamp) {
	if(timerstamp.sec() == 0 && timerstamp.msec() == 0) {
		logger::log_info << "timerfd " << timer_fd << " stop";
		::timerfd_settime(timer_fd, TFD_TIMER_ABSTIME, nullptr, nullptr);
		return;
	}
	logger::log_info << "timer " << timer_fd << " reset";
	struct itimerspec next = { {0, 0}, {static_cast<__time_t>(timerstamp.sec()),
		static_cast<__syscall_slong_t>(1000000 * timerstamp.msec())} };
	// std::cout << timerstamp.sec() << "s" << timerstamp.msec() << "ms" << std::endl;
	::timerfd_settime(timer_fd, TFD_TIMER_ABSTIME, &next, nullptr);
}

static uint64_t read_timerfd(int timer_fd) {
	uint64_t timeout_number;
	if(sizeof(timeout_number) != ::read(timer_fd, &timeout_number, sizeof(timeout_number))) {
		if(errno == EAGAIN) {
			logger::log_warn << "Read timerfd " << timer_fd << " fail";
			return 0;
		} else {
			logger::log_error << "Read timerfd " << timer_fd << " Error: " << strerror(errno);
			assert(false);
		}
	}
	return timeout_number;
}

ev::TimerSet::TimerSet(EventLoop *event_loop) :
	timer_fd_(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC)),
	owner_event_loop_(event_loop),
	timer_channel_(nullptr),
	timer_heap_(new TimerHeap) {
	assert(timer_fd_ >= 0);
	assert(owner_event_loop_ != nullptr);
	assert(timer_heap_ != nullptr);
	logger::log_info << "timer fd " << timer_fd_ << " create";
	::timerfd_settime(timer_fd_, TFD_TIMER_ABSTIME, nullptr, nullptr);
	timer_channel_.reset(new Channel(owner_event_loop_, timer_fd_));
	logger::log_info << "timer fd " << timer_fd_ << " bind channel " << timer_channel_.get();
	timer_channel_->setReadCallback(std::bind(&TimerSet::exec, this));
	logger::log_info << "timer fd " << timer_fd_ << " channel "
		<< timer_channel_.get() << " add event_loop " << owner_event_loop_;
	owner_event_loop_->setChannel(timer_channel_.get());
}

ev::TimerSet::~TimerSet() {
	logger::log_info << "timer fd " << timer_fd_ << " close";
	::close(timer_fd_);
}

void ev::TimerSet::exec() {
	if(read_timerfd(timer_fd_) == 0) {
		logger::log_warn << "Read timerfd " << timer_fd_ << " get zero";
		return;
	}
	TimerStamp now_time = TimerStamp::now();
	std::vector<std::shared_ptr<ev::Timer>> tmp;
	timer_mutex_.lock();
	while(!timer_heap_->empty() && timer_heap_->top()->timer_stamp() <= now_time) {
		auto heap_top = timer_heap_->top();
		timer_heap_->pop();
		logger::log_trace << "timer " << heap_top.get() << " pop";
		tmp.emplace_back(heap_top);
		if(heap_top->has_next()) {
			heap_top->next();
			logger::log_trace << "timer " << heap_top.get() << " push";
			timer_heap_->push(heap_top);
		}
	}
	if(!timer_heap_->empty()) {
		set_timer_from_timestamp(timer_fd_, timer_heap_->top()->timer_stamp());
	} else {
		set_timer_from_timestamp(timer_fd_, TimerStamp(0));
	}
	timer_mutex_.unlock();
	for(auto &&p : tmp) {
		logger::log_trace << "timer " << p.get() << " exec";
		p->exec();
	}
}

ev::TimerId ev::TimerSet::addTimer(const TimerStamp &when, uint64_t interval,
	TimerCallback timer_callback) {
	timer_mutex_.lock();
	bool change = timer_heap_->empty();
	if(!change) {
		change = timer_heap_->top()->timer_stamp() > when;
	}
	TimerId out = timer_heap_->emplace(when, interval, timer_callback);
	logger::log_info << "timer " << out.own_timer_ << " add to set";
	if(change) {
		set_timer_from_timestamp(timer_fd_, when);
	}
	timer_mutex_.unlock();
	return out;
}

bool ev::TimerSet::closeTimer(TimerId &id) {
	auto p = id.own_timer_;
	if(p == nullptr) {
		logger::log_info << "timer " << p << " isn't in set";
		return false;
	}
	logger::log_info << "timer " << p << " del from set";
	auto seq = p->seq();
	p->close();
	id.own_timer_ = nullptr;
	timer_mutex_.lock();
	bool out = timer_heap_->erase(p);
	if(!timer_heap_->empty()) {
		if(seq == 0) {
			set_timer_from_timestamp(timer_fd_, timer_heap_->top()->timer_stamp());
		}
	} else {
		set_timer_from_timestamp(timer_fd_, TimerStamp(0));
	}
	timer_mutex_.unlock();
	return out;
}

bool ev::TimerSet::updateTimer(TimerId &timer, uint64_t delay) {
	auto p = timer.own_timer_;
	if(p == nullptr) {
		return false;
	}
	if(delay == 0) {
		return true;
	}
	auto seq = p->seq();
	timer_mutex_.lock();
	bool out = timer_heap_->updateTimer(p, delay);
	if(!timer_heap_->empty()) {
		if(seq == 0) {
			set_timer_from_timestamp(timer_fd_, timer_heap_->top()->timer_stamp());
		}
	} else {
		set_timer_from_timestamp(timer_fd_, TimerStamp(0));
	}
	timer_mutex_.unlock();
	return out;
}