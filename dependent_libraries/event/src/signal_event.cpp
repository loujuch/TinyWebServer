#include "signal_event.hpp"

#include "channel.hpp"
#include "event_loop.hpp"
#include "log.hpp"

#include <string.h>
#include <sys/signalfd.h>
#include <assert.h>
#include <unistd.h>
#include <sys/eventfd.h>

std::vector<ev::SignalEvent *> ev::SignalEvent::signal_event_(SIGRTMAX + 1, nullptr);

static int safe_close(int fd) {
	int out = fd;
	if(fd >= 0) {
		out = ::close(fd);
		if(out) {
			logger::log_error << "signo event file " << fd << " close fail: " << "errno = " << errno
				<< ' ' << strerror(errno);
		} else {
			logger::log_info << "signo event file " << fd << " close";
		}
		fd = -1;
	} else {
		logger::log_warn << "signo event file fd = " << fd << " had closed";
	}
	return out;
}

static eventfd_t read_eventfd(int event_fd) {
	eventfd_t out;
	eventfd_read(event_fd, &out);
	return out;
}

void ev::SignalEvent::call_sig(int sig) {
	eventfd_write(signal_event_[sig]->event_fd_, 1);
}

static void set_signal_call(int signo, __sighandler_t handler) {
	struct sigaction act;
	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if(signo == SIGALRM) {
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	} else {
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}
	int res = sigaction(signo, &act, nullptr);
	if(res < 0) {
		logger::log_error << "set signo " << signo << " error: " << strerror(errno);
	}
	assert(res >= 0);
}

ev::SignalEvent::SignalEvent(EventLoop *event_loop, int signo) :
	signo_(signo),
	event_fd_(-1),
	own_event_loop_(event_loop),
	event_channel_(nullptr),
	callback_(std::bind(SIG_DFL, signo_)) {
	logger::log_trace << "try set signo: " << signo << " event";
	if(own_event_loop_ == nullptr) {
		logger::log_fatal << "signal event " << this << " with signo " << signo_
			<< " event_loop is nullptr";
		return;
	}
	if(signo_<0 || signo_>SIGRTMAX) {
		logger::log_fatal << "signal event " << this << " with signo " << signo_
			<< " in event_loop " << own_event_loop_ << " signo" << signo_ << "is error"
			<< strerror(errno);
		return;
	}
	event_fd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if(event_fd_ < 0) {
		logger::log_fatal << "signal event " << this << " with signo " << signo_
			<< " in event_loop " << own_event_loop_ << " event_fd open error"
			<< strerror(errno);
		return;
	}
	logger::log_trace << "signo " << signo_ << " eventfd " << event_fd_ << " create";
	event_channel_.reset(new Channel(own_event_loop_, event_fd_));
	event_channel_->setReadCallback(std::bind(&SignalEvent::exec, this));
	own_event_loop_->setChannel(event_channel_.get());
	signal_event_[signo_] = this;
	set_signal_call(signo_, call_sig);
	logger::log_trace << "set signo: " << signo_ << " event";
}

ev::SignalEvent::~SignalEvent() {
	logger::log_trace << "try unset signo: " << signo_ << " event";
	set_signal_call(signo_, SIG_DFL);
	safe_close(event_fd_);
	signal_event_[signo_] = nullptr;
	logger::log_trace << "unset signo: " << signo_ << " event";
}

bool ev::SignalEvent::exec() {
	logger::log_trace << "signo: " << signo_ << " exec";
	read_eventfd(event_fd_);
	if(callback_) {
		callback_();
	} else {
		logger::log_warn << "signo: " << signo_ << " callback is empty";
	}
	return true;
}