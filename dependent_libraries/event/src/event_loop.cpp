#include "event_loop.hpp"

#include <mutex>

#include "log.hpp"
#include "poller.hpp"
#include "channel.hpp"
#include "timer_set.hpp"
#include "timer_stamp.hpp"
#include "signal_set.hpp"

#include <sys/timerfd.h>
#include <string.h>
#include <sys/signalfd.h>
#include <unistd.h>
#include <assert.h>

namespace ev {
__thread EventLoop *thread_event_loop = nullptr;
} // namespace ev

ev::EventLoop *ev::EventLoop::this_thread_event_loop() {
	return thread_event_loop;
}

ev::EventLoop::EventLoop() :
	loop_(false),
	loop_result_(0),
	wait_timeout_(-1),
	poller_(nullptr),
	timer_set_(nullptr),
	signal_set_(nullptr) {
	assert(thread_event_loop == nullptr);
	thread_event_loop = this;
	logger::log_info << "thread loop set";
	poller_.reset(ev::Poller::new_default_poller());
	timer_set_.reset(new TimerSet(this));
	signal_set_.reset(new SignalSet(this));
}

ev::EventLoop::~EventLoop() {
	thread_event_loop = nullptr;
	logger::log_info << "thread loop unset";
}

void ev::EventLoop::setChannel(Channel *channel) {
	if(channel->event() == EV_POOLER_NONE) {
		logger::log_warn << "event_loop " << this
			<< " set channel " << channel << " not event";
	}
	channel_mutex_.lock();
	if(channel_set_.count(channel) == 0) {
		logger::log_info << "Channel " << channel << " set";
		channel_set_.emplace(channel);
	} else {
		logger::log_info << "Channel " << channel << " has reset";
	}
	poller_->setChannel(channel);
	channel_mutex_.unlock();
}

void ev::EventLoop::delChannel(Channel *channel) {
	channel_mutex_.lock();
	if(channel_set_.count(channel) == 0) {
		channel_mutex_.unlock();
		logger::log_warn << "Channel " << channel << " isn't in set";
	} else {
		logger::log_info << "Channel " << channel << " del from set";
		channel_set_.erase(channel);
		poller_->delChannel(channel);
		channel_mutex_.unlock();
	}
}

bool ev::EventLoop::hasChannel(Channel *channel) const {
	bool out;
	channel_mutex_.lock();
	out = poller_->hasChannel(channel);
	channel_mutex_.unlock();
	return out;
}

ev::TimerId ev::EventLoop::addAfterTimer(uint64_t delay, TimerCallback timer_callback_) {
	TimerStamp stamp = TimerStamp::now();
	stamp.delay_time(delay);
	return timer_set_->addTimer(stamp, 0, timer_callback_);
}

ev::TimerId ev::EventLoop::addWhenTimer(const TimerStamp &time, TimerCallback timer_callback_) {
	return timer_set_->addTimer(time, 0, timer_callback_);
}

ev::TimerId ev::EventLoop::addEveryTimer(uint64_t interval, TimerCallback timer_callback_) {
	TimerStamp stamp = TimerStamp::now();
	stamp.delay_time(interval);
	return timer_set_->addTimer(stamp, interval, timer_callback_);
}

bool ev::EventLoop::closeTimer(TimerId &id) {
	return timer_set_->closeTimer(id);
}

bool ev::EventLoop::updateTimer(TimerId &timer, uint64_t delay) {
	return timer_set_->updateTimer(timer, delay);
}

ev::SignalCallback ev::EventLoop::setSignal(int signo, SignalCallback signal_callback) {
	return signal_set_->setSignal(signo, signal_callback);
}


bool ev::EventLoop::delSignal(int signo) {
	return signal_set_->delSignal(signo);
}

bool ev::EventLoop::hasSignal(int signo) {
	return signal_set_->hasSignal(signo);
}

int ev::EventLoop::run() {
	assert(this == thread_event_loop);
	loop_ = true;
	while(loop_) {
		bool is_free = true;
		int nfds = poller_->wait(channel_list_, wait_timeout_);
		logger::log_info << "nfds = " << nfds;
		for(int i = 0;i < nfds && loop_;++i) {
			channel_mutex_.lock();
			if(channel_set_.count(channel_list_[i]) == 0) {
				channel_mutex_.unlock();
				logger::log_info << "fd = " << channel_list_[i]->fd() << " has close";
				continue;
			}
			channel_mutex_.unlock();
			logger::log_info << "fd = " << channel_list_[i]->fd() << " has event";
			channel_list_[i]->exec();
			is_free = false;
		}
		if(is_free && free_callback_ && loop_) {
			logger::log_info << "timeout: " << wait_timeout_ << " call free_callback_";
			free_callback_();
		}
	}
	loop_ = false;
	return loop_result_;
}