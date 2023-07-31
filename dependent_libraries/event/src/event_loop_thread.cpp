#include "event_loop_thread.hpp"

#include "event_loop.hpp"
#include "log.hpp"

ev::EventLoopThread::EventLoopThread(ThreadInitCallback callback) :
	run_(false),
	event_loop_(nullptr),
	init_callback_(callback) {
}

void ev::EventLoopThread::func() {
	run_ = true;
	EventLoop event_loop;
	event_loop_ = &event_loop;
	if(init_callback_) {
		logger::log_trace << "epoll thread init";
		init_callback_(event_loop);
	}
	logger::log_trace << "epoll thread notify run";
	cond_.notify_one();
	event_loop.run();
	run_ = false;
	event_loop_ = nullptr;
}

ev::EventLoop *ev::EventLoopThread::run() {
	loop_thread_ = std::thread(std::bind(&EventLoopThread::func, this));
	std::mutex tmp;
	std::unique_lock<std::mutex> locker(tmp);
	while(event_loop_ == nullptr) {
		logger::log_trace << "wait event_loop_ init";
		cond_.wait(locker);
	}
	locker.unlock();
	return event_loop_;
}