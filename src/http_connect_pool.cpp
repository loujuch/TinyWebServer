#include "http_connect_pool.hpp"

#include <iostream>

#include "http_connect.hpp"
#include "web_server.hpp"
#include "socket_connect.hpp"
#include "callback.hpp"

#include "log.hpp"
#include "event_loop.hpp"
#include "event_loop_thread.hpp"

web::HttpConnectPool::HttpConnectPool(uint32_t thread_number) :
	now(0),
	event_loop_thread_set_(thread_number),
	event_loop_set_(thread_number, nullptr) {
	for(int i = 0;i < thread_number;++i) {
		logger::log_info << "create thread " << i;
		event_loop_thread_set_[i].reset(new ev::EventLoopThread([&](
			ev::EventLoop &loop, ev::ChannelArgs &args) {
				loop.setTimeOut(timeout);
				loop.setFreeCallback([&]() {
					logger::log_trace << "Pool: " << this << " free "
						<< timeout << "ms event_loop is " << &loop;
					});
			}));
		event_loop_set_[i] = event_loop_thread_set_[i]->run();
	}
}

web::HttpConnectPool::~HttpConnectPool() {
	for(auto &t : event_loop_thread_set_) {
		t->join();
	}
}

void web::HttpConnectPool::add_connect(SocketConnect &&new_sock,
	const sockaddr_in &new_addr, WebServer *web) {
	int target = now;
	now = now + 1 >= event_loop_set_.size() ? 0 : now + 1;
	HttpConnect *http_connect = new HttpConnect(
		event_loop_set_[target], std::move(new_sock), new_addr, web);
	logger::log_info << "socket connect: " << new_sock.socket()
		<< " create belong " << target << " in " << http_connect;
}