#include "signal_set.hpp"

#include "signal_event.hpp"
#include "log.hpp"

std::vector<std::unique_ptr<ev::SignalEvent>> ev::SignalSet::signal_set_(SIGRTMAX + 1);

ev::SignalSet::SignalSet(EventLoop *event_loop) :
	own_event_loop_(event_loop) {
	if(own_event_loop_ == nullptr) {
		logger::log_fatal << "signal set " << this << " event_loop is nullptr";
		return;
	}
	sigemptyset(&loop_set_);
}

ev::SignalSet::~SignalSet() {
	for(int i = 0;i < signal_set_.size();++i) {
		if(hasSignal(i)) {
			logger::log_info << "signo = " << i << " close and remove set";
			signal_set_[i].reset();
		}
	}
}

ev::SignalCallback ev::SignalSet::setSignal(int signo, SignalCallback signal_callback) {
	signal_mutex_.lock();
	if(sigismember(&loop_set_, signo) == 1) {
		auto p = signal_set_[signo]->setCallback(signal_callback);
		signal_mutex_.unlock();
		logger::log_trace << "signo: " << signo << " reset";
		return p;
	}
	if(signal_set_[signo] != nullptr) {
		logger::log_warn << "signal_set " << this << " set signo " << signo << " has callback";
	}
	sigaddset(&loop_set_, signo);
	signal_set_[signo].reset(new SignalEvent(own_event_loop_, signo));
	signal_mutex_.unlock();
	logger::log_trace << "signo: " << signo << " add";
	return signal_set_[signo]->setCallback(signal_callback);
}

bool ev::SignalSet::delSignal(int signo) {
	logger::log_trace << "start";
	signal_mutex_.lock();
	if(!hasSignal(signo)) {
		signal_mutex_.unlock();
		logger::log_trace << "Signo: " << signo << " isn't in set: ";
		return false;
	}
	sigdelset(&loop_set_, signo);
	signal_set_[signo].reset();
	signal_mutex_.unlock();
	logger::log_trace << "end signo: " << signo;
	return true;
}