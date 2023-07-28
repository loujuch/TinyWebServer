#include "channel.hpp"

#include "event_loop.hpp"
#include "log.hpp"

#include <string.h>

ev::Channel::Channel(EventLoop *event_loop, int fd) :
	close_(false),
	fd_(fd),
	events_(EV_POOLER_NONE),
	revents_(EV_POOLER_NONE),
	owner_event_loop_(event_loop) {
	if(owner_event_loop_ == nullptr) {
		logger::log_fatal << "channel " << this << " event_loop is nullptr";
		return;
	}
	if(fd_ < 0) {
		logger::log_fatal << "channel " << this << " event_loop "
			<< owner_event_loop_ << " fd is not exist";
		return;
	}
	logger::log_info << "channel " << this << " fd = " << fd_ << ": create";
}

ev::Channel::~Channel() {
	if(close_) {
		logger::log_info << "channel " << this << " fd = " << fd_ << ": had close";
		return;
	}
	if(owner_event_loop_ != EventLoop::this_thread_event_loop()) {
		logger::log_warn << "channel " << this << " fd = " << fd_ << ": this_thread_event_loop has close";
		return;
	}
	if(owner_event_loop_->hasChannel(this)) {
		close();
		logger::log_info << "channel " << this << " fd = " << fd_ << ": close";
	} else {
		logger::log_warn << "channel " << this << " fd = " << fd_ << ": isn't in loop";
	}
}

void ev::Channel::close() {
	close_ = true;
	owner_event_loop_->delChannel(this);
}

void ev::Channel::exec() {
	if(close_) {
		logger::log_trace << "channel " << this << " fd = " << fd_ << " has close";
		return;
	}

	pooler_event_t revent = revents_;

	logger::log_info << "channel " << this << " fd = " << fd_ << " has event " << revent;

	if(revent == EV_POOLER_NONE) {
		logger::log_warn << "channel " << this << " fd = " << fd_ << ": revent == EV_POOLER_NONE";
		return;
	}

	if((revent & EV_POOLER_HUP) && !(revents_ & EV_POOLER_READ)) {
		logger::log_trace << "channel " << this << " fd = " << fd_ << ": hup";
		if(close_callback_) {
			close_callback_();
		} else {
			logger::log_warn << "channel " << this << " fd = " << fd_ << ": close callback is empty";
		}
	}

	if(revent & (EV_POOLER_ERROR | EV_POOLER_NVAL)) {
		logger::log_trace << "channel " << this << " fd = " << fd_ << ": error";
		logger::log_error << "channel " << this << " errno = " << errno << ' ' << strerror(errno);
		if(error_callback_) {
			error_callback_();
		} else {
			logger::log_warn << "channel " << this << " fd = " << fd_ << ": error callback is empty";
		}
	}

	if(revent & (EV_POOLER_READ | EV_POOLER_PRI | EV_POOLER_RDHUP)) {
		logger::log_trace << "channel " << this << " fd = " << fd_ << ": read";
		if(read_callback_) {
			read_callback_();
		} else {
			logger::log_warn << "channel " << this << " fd = " << fd_ << ": read callback is empty";
		}
	}

	if(revent & EV_POOLER_WRITE) {
		logger::log_trace << "channel " << this << " fd = " << fd_ << ": write";
		if(write_callback_) {
			write_callback_();
		} else {
			logger::log_warn << "channel " << this << " fd = " << fd_ << ": write callback is empty";
		}
	}

	if(close_) {
		revents_ = EV_POOLER_NONE;
		if(delete_callback_) {
			delete_callback_();
		}
	}
}